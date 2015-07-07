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

#ifndef SAORSBASE_BEACONBLOCK_H_
#define SAORSBASE_BEACONBLOCK_H_

#include <stdexcept>
#include <math.h>
#include <iostream>
#include "inet/networklayer/contract/ipv4/IPv4Address.h"


const double init_value = 0.2;
namespace inet {

class IARP;

namespace inetmanet {

/********************************************************************************************
 * class SAORSBase_BeaconBlock: Defines the fields and functions of the entries included in
 *                              the beacon messages, such as setter and getter functions.
 ********************************************************************************************/
class SAORSBase_BeaconBlock
{
	protected:
		uint32_t address;	///< The IP address of a node that can be reached via the DT-DYMO router adding this information.
		uint8_t prefix; 	///< The Node.Address is a network address with a particular prefix length.
		uint32_t beacons;	///< The number of beacons collected from that destination.
		uint8_t prob;		///< The probability to encounter the specified node.
		uint8_t similarity; ///< The similarity to the specified node.

	public:
		SAORSBase_BeaconBlock(uint32_t Address=0, uint8_t Prefix=0, double Prob=0, int Beacons=0, double Similarity=0)
		{
			setAddress(Address);
			setPrefix(Prefix);
			setBeacons(Beacons);
			setProbability(Prob);
			setSimilarity(Similarity);
		}

		SAORSBase_BeaconBlock(const SAORSBase_BeaconBlock& other)
		{
			operator=(other);
		}

		SAORSBase_BeaconBlock& operator=(const SAORSBase_BeaconBlock& other)
		{
			if (this==&other) return *this;
			this->address = other.address;
			this->prefix = other.prefix;
			this->beacons = other.beacons;
			this->prob = other.prob;
			return *this;
		}

		IPv4Address getAddress() const
		{
			return IPv4Address(address);
		}

		void setAddress(uint32_t address)
		{
			this->address = address;
		}

		uint8_t getPrefix() const
		{
			return prefix;
		}

		void setPrefix(uint8_t prefix)
		{
			this->prefix = prefix;
		}

		uint32_t getBeacons() const
		{
			return beacons;
		}

		void setBeacons(uint32_t Beacons)
		{
			this->beacons = Beacons;
		}

		double getProbability() const
		{
			return ((prob-init_value)/253);
		}

		void setProbability(double input) {
			if(input<0 || input>1) throw std::runtime_error("Given probability if not between [0,1]!!!");
				prob = static_cast<uint8_t>((input * 253) + init_value);
		}

		double getSimilarity()
        {
            return (double)(similarity/253);
        }

		void setSimilarity(double input) {
            if(input<0 || input>1) throw std::runtime_error("Given probability if not between [0,1]!!!" );
                similarity = static_cast<uint8_t>((input * 253) + init_value);
        }

		/** @brief overloading the << operator for printing the output of the class */
		friend std::ostream& operator<<(std::ostream& os, const SAORSBase_BeaconBlock& o) {
			os << o.address;
			return os;
		};

};

} // namespace inetmanet

} // namespace inet

#endif /* SAORSBASE_BEACONBLOCK_H_ */

