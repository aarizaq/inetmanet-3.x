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
#include "inet/physicallayer/antenna/PhasedArray.h"
#ifndef MEMPOS_H_
#define MEMPOS_H_

namespace inet {
namespace physicallayer {

class INET_API Mempos : public cSimpleModule, protected cListener {

public:
    Mempos();

    static simsignal_t mobilityStateChangedSignal;
    std::vector<Coord> posit;
    virtual void initialize() override;
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;
    virtual void stampaLista();
    virtual void finish() override;
    virtual std::vector<Coord> getListaPos()const;
    virtual double getAngolo(Coord p1, Coord p2)const;
};

} /* namespace physicallayer */
} /* namespace inet */

#endif /* MEMPOS_H_ */
