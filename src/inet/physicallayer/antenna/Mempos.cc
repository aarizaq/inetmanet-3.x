//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "inet/physicallayer/antenna/Mempos.h"
#include <sstream>
#include <string>
#include <cmath>
#include <math.h>
#include <iomanip>
namespace inet {
namespace physicallayer {

using std::cout;
using std::cin;

Define_Module(Mempos);
Mempos::Mempos()
{ }

simsignal_t Mempos::mobilityStateChangedSignal = registerSignal("mobilityStateChanged");

void Mempos::initialize()
{

    getSimulation()->getSystemModule()->subscribe(mobilityStateChangedSignal, this);
}

void Mempos::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{

    if (signalID == mobilityStateChangedSignal) {
        IMobility *mobility = dynamic_cast<IMobility *>(source);
        if (mobility) {
            Coord c = mobility->getCurrentPosition();
            posit.push_back(c);

        }
    }

}

void Mempos::stampaLista(){

   for (int i = 0; i < posit.size(); ++i)
             {

           cout <<posit[i];

            }

}

std::vector<Coord> Mempos::getListaPos()const{
    return posit;
}

void Mempos::finish(){

      std::vector<Coord>positions = getListaPos();
       int r; int c;
       double matrix[positions.size()][positions.size()];



   /*   for (int r=0;r<positions.size();r++){
      for (int c=0; c<positions.size();c++){

          if(std::isnan(getAngolo(positions[r],positions[c+1]))){
                    cout<<"X"<<" ";

                } else
                { cout<<getAngolo(positions[r],positions[c+1])<<"°"<<" ";}



      if(c==positions.size()-1)cout<<"\n"<< endl;
      }
      }
      */

/*
       for (int r=0;r<positions.size();r++){
             for (int c=0; c<positions.size();c++){

                 matrix[r][c]=getAngolo(positions[r],positions[c]) ;
                 if(std::isnan(matrix[r][c])){
                                                      cout<<"X"<<"  ";
                                                  }else
                 {cout<<std::setprecision(4)<<matrix[r][c]<<"°"<<"  ";}

                 if(c==positions.size()-1)cout<<"\n"<< endl;
             }

             }
       */


       for (int f=0;f<positions.size();f++){
                    for (int g=0; g<positions.size();g++){

                        matrix[f][g]= positions[f].angle(positions[g])*(180/3.14) ;
                        if(std::isnan(matrix[f][g])){
                                                             cout<<"X"<<"  ";
                                                         }else
                        {cout<<std::setprecision(4)<<matrix[f][g]<<"°"<<"  ";}

                        if(g==positions.size()-1)cout<<"\n"<< endl;
                    }

                    }





}

double Mempos::getAngolo(Coord p1, Coord p2)const{
    double angolo;
    double cangl, intercept;
    double x1, y1, x2, y2;
    double dx, dy;

    x1=p1.x;y1=p1.y;
    x2=p2.x;y2=p2.y;
    dx=x2-x1;
    dy=y2-y1;
    cangl=dy/dx;
    angolo = atan(cangl)*(180/3.14);
    return angolo;

}




} /* namespace physicallayer */
} /* namespace inet */



