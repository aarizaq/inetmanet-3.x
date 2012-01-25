//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//

/**
* @file hotspotRoaming.cc
* @author Stephan Krause
*/

#include "hotspotRoaming.h"
#include "StringConvert.h"

hotspotRoaming::hotspotRoaming(double areaDimension, double speed, NeighborMap *Neighbors, GlobalCoordinator* coordinator, CollisionList* CollisionRect)
              :MovementGenerator(areaDimension, speed, Neighbors, coordinator, CollisionRect)
{
    double prob = 0;

    std::vector<std::string> hotspotvec = cStringTokenizer(coordinator->par("Hotspots"), ";").asVector();
    for( std::vector<std::string>::iterator it = hotspotvec.begin(); it != hotspotvec.end(); ++it ){
        std::vector<std::string> hstr = cStringTokenizer(it->c_str(), ",").asVector();
        if( hstr.size() != 4 ) {
            throw( cException("Error parsing Hotspots parameter") );
        }
        // parse string, convert to hotspot data
        Hotspot h;
        h.center.x = convertString<double>( hstr[0] );
        h.center.y = convertString<double>( hstr[1] );
        h.radius = convertString<double>( hstr[2] );
        h.probability = convertString<double>( hstr[3] );
        prob += h.probability;

        // check hotspot bounds
        if( h.center.x - h.radius < 0 || h.center.y - h.radius < 0 ||
                h.center.x + h.radius > areaDimension || h.center.y + h.radius > areaDimension ) {
         
            throw( cException("Error: Hotspot is outside the playground!") );
        }
        if( prob > 1 ){
            throw( cException("Error: Hotspot probabilities add up to > 1!") );
        }

        hotspots.push_back(h);
    }
    curHotspot = hotspots.end();
    if( (double) coordinator->par("HotspotStayTime") == (double) 0.0 ) {
        stayInHotspot = false;
    } else {
        stayInHotspot = true;
    }
    target.x = uniform(0.0, areaDimension);
    target.y = uniform(0.0, areaDimension);
}

double hotspotRoaming::getDistanceFromHotspot()
{
    double minDist=areaDimension;
    for( std::vector<Hotspot>::iterator it = hotspots.begin(); it != hotspots.end(); ++it) {
        double dist = sqrt(position.distanceSqr( it->center )) - it->radius;
        if( dist < minDist ) minDist = dist;
    }

    return minDist;
}

void hotspotRoaming::move()
{
    flock();
    position += direction * speed;
    if(testBounds()) {
        position += direction * speed * 2;
        testBounds();
    }

    if(target.distanceSqr(position) < speed * speed) {
        // arrived at current destination
        
        // if we are not inside a hotspot, or do not want to
        // stay inside the current hotspot (any more) ...
        if ( !stayInHotspot || curHotspot == hotspots.end() ||
                  ( hotspotStayTime > 0 && hotspotStayTime < simTime() ))
        {
            hotspotStayTime = 0;
            
            // ... select next target hotspot
            double rnd = uniform(0, 1);
            for( curHotspot = hotspots.begin(); curHotspot != hotspots.end(); ++curHotspot ){
                rnd -= curHotspot->probability;
                if( rnd <= 0 ) break;
            }

        } else {
            // stay in current hotspot
            // start stayTimer if not already done
            if ( hotspotStayTime == 0 ) {
                hotspotStayTime = simTime() + coordinator->par("HotspotStayTime");
            }
        }

        // chose target inside hotspot, or random target if no hotspot was selected
        if( curHotspot != hotspots.end() ){
            Vector2D dev;
            // randomly select point inside the hotspot
            double r = uniform( 0, 1 );
            double theta = uniform( 0, 2*M_PI );
            dev.x = sqrt( r ) * cos( theta );
            dev.y = sqrt( r ) * sin( theta );

            target = curHotspot->center + dev*curHotspot->radius;
        } else {
            target.x = uniform(0.0, areaDimension);
            target.y = uniform(0.0, areaDimension);
        }
    }
}
