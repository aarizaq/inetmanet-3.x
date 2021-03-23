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

#ifndef _EGOADJACENCY_H_
#define _EGOADJACENCY_H_


#include <omnetpp.h>
#include <string>
#include "inet/common/INETDefs.h"
#include "inet/routing/extras/saors/base/SAORS_BEACON_m.h"

namespace inet {

namespace inetmanet {


typedef std::pair<const uint32_t, VectorOfSAORSBeaconBlocks> bcnInfoPair;	 ///< The pair of the beacon information stored in the Ego-Adjacency map
typedef std::map<uint32_t, VectorOfSAORSBeaconBlocks> bcnInfo;				 ///< A map of VectorOfDTDYMOBeaconBlocks according to the received address
typedef std::map<uint32_t, VectorOfSAORSBeaconBlocks>::iterator bcnInfoIter; ///< A map of VectorOfDTDYMOBeaconBlocks according to the received address


/********************************************************************************************
 * class EgoAdjacency: This class represents the ego-adjacency matrix necessary for the
 *                     computation of the ego-betweenness metric.
********************************************************************************************/
class EgoAdjacency
{
  protected:
	bcnInfo pastBeacons;		///< A map of the beacons of the neighbors received in the past

  public:
	/** @brief constructor */
	EgoAdjacency();

	/** @brief destructor */
	virtual ~EgoAdjacency();

	/** @brief Return the number of hosts for which beacon information is stored */
	virtual int getEgoAdjacencySize() const { return pastBeacons.size(); };

	/** @brief returns the beacon information for a specific address if it exists */
	virtual bcnInfoPair* findBeaconInfo(uint32_t address);

	/** @brief returns the beacon information position for a specified address - start point is 0 */
	virtual int findBeaconInfoPosition(uint32_t address);

	/** @brief returns the beacon information for an entry in the specified position - start point is 0 */
	virtual bcnInfoPair* getBeaconInfo(uint position);

	/** @brief returns the IP Address for an entry in the specified position - start point is 0 */
	virtual int getBeaconEntry(uint position);

	/** @brief checks if the information of the beacon are present and if yes updates them */
	virtual void proccessBeaconInfo(uint32_t address, VectorOfSAORSBeaconBlocks info);

	/** @brief removes the information of the beacon if present */
	virtual void removeBeaconInfo(uint32_t address);

	/** @brief prints out the information stored in the Ego-Adjacency Matrix */
	virtual std::string str() const;

	/** @brief calculates the betweenness of the host, according to the Ego-Adjacency information */
	virtual double calculateBetweenness();

	/** @brief returns whether the given address is part of the node community
	 * The community is defined by the nodes in the ego adjacency matrix and those
	 * included in the beacon block*/
	bool isCommuninityMenber(uint32_t address);

	/** @brief overloading the << operator for printing the output of the class */
	friend std::ostream& operator<<(std::ostream& os, const EgoAdjacency& o);
};

} // namespace inetmanet

} // namespace inet

#endif /* EGOADJACENCY_H_ */
