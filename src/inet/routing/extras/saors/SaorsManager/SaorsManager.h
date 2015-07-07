/*
 *  Copyright (C) 2012 Nikolaos Vastardis
 *  Copyright (C) 2012 University of Essex
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __SAORS_MANAGER_H__
#define __SAORS_MANAGER_H__


#include "inet/common/INETDefs.h"
// ICMP type 2, code 4: fragmentation needed, but don't-fragment bit set

namespace inet {

namespace inetmanet {


/*****************************************************************************************
 * class SaorsManager: Socially-Aware Opportunistic Routing System (SAORS) Manager Class
 *****************************************************************************************/
class SaorsManager : public cSimpleModule
{
  private:
    enum RouteTypeProtocol {
        DTDYMO,
        SAMPhO,
        EPDYMO,
        RDYMO,
        SIMBETTS
    };                                      ///< For now, only two routing protocols are incorporated (DT-DYMO, SD-DYMO)
    cModule *routingModule;                 ///< The inner routing module of the manager
    RouteTypeProtocol routing_protocol;     ///< Describes the type of the DT Routing Protocol used
    bool dynamicLoad;                       ///< Indicates whether the inner routing module will be created dynamically on load

  protected:
    // config
    bool DTActive;                          ///< Indicates whether the SAORS Manager is active or not
    const char *routingProtocol;            ///< The name of the routing protocol

  public:
    /** @brief class constructor */
    SaorsManager() {dynamicLoad=false;};

  protected:
    /** @brief return the number of stages in the initialization */
    int numInitStages() const  {return 5;}

    /** @brief initialization function */
    void initialize(int stage);

    /** @brief handles received messages - forwards them to the routing module */
    virtual void handleMessage(cMessage *msg);

    /** @brief finish function */
    virtual void finish() {};
};

} // namespace inetmanet

} // namespace inet

//class SaorsManagerStatic : public SaorsManager {};
#endif /* __SAORS_MANAGER_H__ */

