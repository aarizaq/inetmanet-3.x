/*
 * Configuration.h
 *
 *  Created on: 18.03.2011
 *      Author: sbeiti
 */
//add by eugen
#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

//#define RPS
#ifdef RPS // connect omnet to RPS
#include "RPSSocket.h"
#endif
#endif /* CONFIGURATION_H_ */

//#define OPENSSL_IS_LINKED //is only defined if the openssl library is installed and linked; necessary to evaluate PASER.
//end add
