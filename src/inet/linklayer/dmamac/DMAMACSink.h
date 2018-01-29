//**************************************************************************
// * file:        DMAMACSink header file
// *
// * author:      A. Ajith Kumar S.
// * copyright:   (c) A. Ajith Kumar S. 
// * homepage:    www.hib.no/ansatte/aaks
// * email:       aji3003 @ gmail.com
// **************************************************************************
// * part of:     A dual mode adaptive MAC (DMAMAC) protocol for WSAN.
// * Refined on:  25-Apr-2015
// **************************************************************************
// *This file is part of DMAMAC (DMAMAC Protocol Implementation on MiXiM-OMNeT).
// *
// *DMAMAC is free software: you can redistribute it and/or modify
// *it under the terms of the GNU General Public License as published by
// *the Free Software Foundation, either version 3 of the License, or
// *(at your option) any later version.
// *
// *DMAMAC is distributed in the hope that it will be useful,
// *but WITHOUT ANY WARRANTY; without even the implied warranty of
// *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// *GNU General Public License for more details.
// *
// *You should have received a copy of the GNU General Public License
// *along with DMAMAC.  If not, see <http://www.gnu.org/licenses/>./
// **************************************************************************

#ifndef __DMAMACSINK_H__
#define __DMAMACSINK_H__

#include "inet/linklayer/dmamac/DMAMAC.h"
#include <deque>

/* @brief XML files are used to store slot structure for easy input differentiation 
 * from execution files */

/*
 * @description DMAMACSink is bit different from host nodes, does not need to have an application layer. 
 * The packets are created directly at the MAC for each actuator.
 */
namespace inet {

using namespace physicallayer;
using namespace power;
  
class INET_API DMAMACSink : public DMAMAC
{
    /* @brief Partly used from LMAC definition.  */

    private:
        /* @brief Copy constructor is not allowed.*/
        DMAMACSink(const DMAMACSink&);

        /* @brief Assignment operator is not allowed.*/
        DMAMACSink& operator=(const DMAMACSink&);

    /* @brief Initializing variables. */

    public:DMAMACSink():DMAMAC()
    {}

    ~DMAMACSink() {}

    /* @brief Module destructor DMAMAC Sink version*/
    //~DMAMACSink();

    /* @brief MAC layer initialization for DMAMAC and BaseMacLayer definition  */
    void initialize(int stage) override;

    /* @brief Delete all dynamically allocated objects (if any) of the module DMAMAC */
    //void finish();

    /* @brief Handles packets received from the Physical Layer.*/
    virtual void handleLowerPacket(cPacket* msg) override;

    /* @brief Handle self messages such as timers used, to move through slots in DMAMAC  */
    virtual void handleSelfMessage(cMessage*) override;

    /* @brief Handles the control messages received from the physical layer. */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long value, cObject *details) override;

    /* @brief
     * Called after the radio switched to TX mode. Encapsulates
     * and sends down the packet to the physical layer.
     * LMAC does this in handleSelfMessage
     */
    virtual void handleRadioSwitchedToTX() override;

    /* @brief
     * Called when the radio is switched to RX.
     * Sets the MAC state to RX.
     * LMAC does this in handleSelfMessage
     */
    virtual void handleRadioSwitchedToRX() override;

    /* Defining the static slot schedule */
    //void slotInitialize(); Will be inherited from DMAMAC.cc

protected:


    bool changeMacModeInNextSuperFrame = false;

	/* @brief Stores addresses for actuator nodes. */
    std::vector<int64_t> actuatorNodes;

    /* @brief Finds the distant next slot after the current slot (say after going to sleep) #GinMAC */
    virtual void findDistantNextSlot() override;

    /* @brief Finds the immediate next slot after the current slot #GinMAC  */
    virtual void findImmediateNextSlot(int currentSlotLocal,simtime_t nextSlot) override;

    /* @brief Initialization part for sink only */
    void sinkInitialize();
};

}

#endif
