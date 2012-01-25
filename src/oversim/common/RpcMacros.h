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
 * This file declares some macros for RPC implementation.
 *
 * @file RpcMacros.h
 * @author Sebastian Mies
 */
#ifndef __RPC_MACROS_H
#define __RPC_MACROS_H

/**
 * Marks the beginning of a Remote-Procedure-Call Switch block.
 * RPC_CALL, RPC_ON_CALL, RPC_ON_RESPONSE are allowed inside
 * this block.
 */
#define RPC_SWITCH_START( message ) \
    bool rpcHandled = false;\
    do { \
        BaseRpcMessage* ___msg = dynamic_cast<BaseRpcMessage*>(message);

/**
 * Marks the end of a Remote-Procedure-Call Switch block.
 */
#define RPC_SWITCH_END() \
    } while (false);

#define IF_RPC_HANDLED \
    if (rpcHandled)

#define RPC_HANDLED \
    rpcHandled

/**
 * Declares a RPC method delegation.
 *
 * @param name The message Name of the RPC
 * @param method The method to call
 */
#define RPC_DELEGATE( name, method ) \
    name##Call* _##name##Call = dynamic_cast<name##Call*>(___msg); \
    if (_##name##Call != NULL) { rpcHandled = true; method(_##name##Call); \
     break; }

/**
 * Declares an if-statement for a specific call
 *
 * @name The message name of the RPC
 */
#define RPC_ON_CALL( name ) \
    name##Call* _##name##Call = dynamic_cast<name##Call*>(___msg); \
    if (_##name##Call != NULL && !rpcHandled)

/**
 * Declares an if-statement for a specific response
 *
 * @name The message name of the RPC
 */
#define RPC_ON_RESPONSE( name ) \
    name##Response* _##name##Response = dynamic_cast<name##Response*>(___msg); \
    if (_##name##Response != NULL && !rpcHandled)


#endif

