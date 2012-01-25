//
// Copyright (C) 2009 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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
 * @file CryptoModule.h
 * @author Ingmar Baumgart
 */


#ifndef __CRYPTOMODULE_H_
#define __CRYPTOMODULE_H_

#include <omnetpp.h>
#include <cnetcommbuffer.h>

class GlobalStatistics;
class BaseOverlay;

/**
 * The CryptoModule contains several method needed for message authentication.
 *
 * The CryptoModule contains several method needed for message authentication.
 *
 * @author Ingmar Baumgart
 */
class CryptoModule : public cSimpleModule
{

public:
    CryptoModule();
    virtual ~CryptoModule();

    /**
     * Signs an RPC message.
     *
     * This method signs the given BaseRpcMessage msg with the node
     * private key.
     *
     * @param msg the message to sign
     */
    virtual void signMessage(BaseRpcMessage *msg);

    /**
     * Verifies the signature of an RPC message.
     *
     * This method verifies the signature of the BaseRpcMessage msg
     * and returns true, if the signature is valid.
     *
     * @param msg the message to verify
     * @return true, if the message contains a valid signature
     */
    virtual bool verifyMessage(BaseRpcMessage *msg);

protected:
    // see omnetpp.h
    virtual void initialize();

    // see omnetpp.h
    virtual void handleMessage(cMessage *msg);

    // see omnetpp.h
    virtual void finish();

private:
    // references to global modules
    GlobalStatistics* globalStatistics;  /**< pointer to GlobalStatistics module in this node */
    BaseOverlay* overlay; /**< pointer to the overlay module in this node */

    cNetCommBuffer commBuffer; /**< the buffer used to serialize messages */

    int numSign; /**< message signature counter for statistics */

};

#endif
