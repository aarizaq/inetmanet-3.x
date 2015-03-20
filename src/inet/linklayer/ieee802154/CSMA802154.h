/* -*- mode:c++ -*- ********************************************************
 * file:        CSMA.h
 *
 * author:     Jerome Rousselot, Marcel Steine, Amre El-Hoiydi,
 *				Marc Loebbers, Yosia Hadisusanto
 *
 * copyright:	(C) 2007-2009 CSEM SA
 *              (C) 2009 T.U. Eindhoven
 *				(C) 2004 Telecommunication Networks Group (TKN) at
 *              Technische Universitaet Berlin, Germany.
 *
 *              This program is free software; you can redistribute it
 *              and/or modify it under the terms of the GNU General Public
 *              License as published by the Free Software Foundation; either
 *              version 2 of the License, or (at your option) any later
 *              version.
 *              For further information see file COPYING
 *              in the top level directory
 *
 * Funding: This work was partially financed by the European Commission under the
 * Framework 6 IST Project "Wirelessly Accessible Sensor Populations"
 * (WASP) under contract IST-034963.
 ***************************************************************************
 * part of:    Modifications to the MF-2 framework by CSEM
 **************************************************************************/

#ifndef __INET_CSMA802154_H
#define __INET_CSMA802154_H

#include "inet/linklayer/csma/CSMA.h"
#include "inet/physicallayer/ieee802154/bitlevel/Ieee802154UWBIRTransmitter.h"

namespace inet {

using namespace physicallayer;

/**
 * @brief Generic CSMA Mac-Layer.
 *
 * Supports constant, linear and exponential backoffs as well as
 * MAC ACKs.
 *
 * @author Jerome Rousselot, Amre El-Hoiydi, Marc Loebbers, Yosia Hadisusanto, Andreas Koepke
 * @author Karl Wessel (port for MiXiM)
 *
 * \image html csmaFSM.png "CSMA Mac-Layer - finite state machine"
 */

class INET_API CSMA802154 : public CSMA
{
  public:
        CSMA802154() : CSMA()
        {}

    /** @brief Initialization of the module and some variables*/
    virtual void initialize(int);

  protected:
    Ieee802154UWBIRTransmitter *transmitter;
    /** @brief Generate new interface address*/
    virtual void initializeMACAddress();
    virtual InterfaceEntry *createInterfaceEntry();

  private:
    /** @brief Copy constructor is not allowed.
     */
    CSMA802154(const CSMA&);
    /** @brief Assignment operator is not allowed.
     */
    CSMA802154& operator=(const CSMA802154&);
};

} // namespace inet

#endif // ifndef __INET_CSMA_H

