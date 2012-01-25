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
 * @file SimMud.h
 * @author Stephan Krause
 */


#ifndef __SIMMUD_H_
#define __SIMMUD_H_

#include "BaseApp.h"
#include "SimMud_m.h"

#include "Vector2D.h"

class SimMud : public BaseApp
{
    private:
        int currentRegionX;
        int currentRegionY;
        OverlayKey currentRegionID;
        std::set<OverlayKey> subscribedRegions;

        int fieldSize;
        int numSubspaces;
        int regionSize;
        int AOIWidth;

        int receivedMovementLists;
        int lostMovementLists;
        simtime_t maxMoveDelay;

        int playerTimeout;
        cMessage* playerTimer;

        struct PlayerInfo {
            Vector2D pos;
            bool update;
        };

        std::map <NodeHandle, PlayerInfo> playerMap;

    public:
        SimMud( );
        ~SimMud( );

        /**
         * Initialize class attributes
         */
        virtual void initializeApp( int stage );

        virtual void handleTimerEvent( cMessage *msg );
        virtual void handleUpperMessage( cMessage *msg );
        virtual void handleLowerMessage( cMessage *msg );
        virtual void handleReadyMessage( CompReadyMessage *msg );

        virtual bool handleRpcCall( BaseCallMessage* msg );
        virtual void handleRpcResponse( BaseResponseMessage* msg,
                                        cPolymorphic* context,
                                        int rpcId, simtime_t rtt );

        /**
         * collect statistical data
         */
        virtual void finishApp( );

    protected:
        void handleMove( GameAPIPositionMessage* msg );
        void handleOtherPlayerMove( SimMudMoveMessage* msg );
};

#endif
