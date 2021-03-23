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

#include "EgoAdjacency.h"

namespace inet {

namespace inetmanet {

/*****************************************************************************************
 * The Constructor of the class.
 *****************************************************************************************/
EgoAdjacency::EgoAdjacency() {
	pastBeacons.clear();

}


/*****************************************************************************************
 * The Destructor of the class.
 *****************************************************************************************/
EgoAdjacency::~EgoAdjacency() {
	pastBeacons.clear();
}


/*****************************************************************************************
 * Returns the beacon information for a specific address if it exists.
 *****************************************************************************************/
bcnInfoPair* EgoAdjacency::findBeaconInfo(uint32_t address) {
	//If the beacon information exists, return true
	bcnInfoIter iter=pastBeacons.find(address);

	if(iter!=pastBeacons.end()) {
		bcnInfoPair *info=&*iter;
		return(info);
	}

	//Else insert return false
	return(NULL);
}


/*****************************************************************************************
 * Returns the beacon information position for a specified address
 * - start point is 0 and return 0 if not found.
 *****************************************************************************************/
int EgoAdjacency::findBeaconInfoPosition(uint32_t address) {

	bcnInfoIter iter=pastBeacons.begin();
	int position=0;
	//Start from the beginning until address is found

	//Look at all the positions
	while(iter!=pastBeacons.end()) {
		//Count only the positions with existing beacon information
		if(iter->second.size()>0) {
			//If address is found return the position
			if (iter->first==address)
				return position+1;
			position++;
		}
		iter++;
	}

	return -1;
}


/*****************************************************************************************
 * Returns the beacon information for a entry in the specified position - start point is 0.
 *****************************************************************************************/
bcnInfoPair* EgoAdjacency::getBeaconInfo(uint position) {
	bcnInfoIter iter=pastBeacons.begin();

	//If the position provided is valid
	if(position<pastBeacons.size()) {

		//Move the iterator
		for(uint i=0;i<position;i++) {
			iter++;
		}

		bcnInfoPair *info=&*iter;
		//Return the Beacon Information
		return(info);
	}

	//Else return a null pointer
	return NULL;
}


/*****************************************************************************************
 * Returns the beacon information for a entry in the specified position - start point is 0.
 *****************************************************************************************/
int EgoAdjacency::getBeaconEntry(uint position) {
	bcnInfoIter iter=pastBeacons.begin();

	//If the position provided is valid
	if(position<pastBeacons.size()) {

		//Move the iterator
		for(uint i=0;i<position;i++) {
			iter++;
		}

		int entry=iter->first;
		//Return the Beacon Information
		return(entry);
	}

	//Else return a null pointer
	return(-1);
}


/*****************************************************************************************
 * Returns the IP Address for an entry in the specified position - start point is 0.
 *****************************************************************************************/
void EgoAdjacency::proccessBeaconInfo(uint32_t address, VectorOfSAORSBeaconBlocks info) {
	//If the beacon information already exists, update it
	bcnInfoIter iter=pastBeacons.find(address);
	if(iter!=pastBeacons.end()) {
		//If the beacon is not empty
		if(info.size()>0)
			iter->second=info;
	}
	//Else insert it
	else {
		pastBeacons.insert(std::pair<uint32_t,VectorOfSAORSBeaconBlocks>(address,info));
	}
}


/*****************************************************************************************
 * Removes the information of the beacon if present.
 *****************************************************************************************/
void EgoAdjacency::removeBeaconInfo(uint32_t address) {
	//If the beacon information exists, remove it
	bcnInfoIter iter=pastBeacons.find(address);
	if(iter!=pastBeacons.end()) {
		pastBeacons.erase(iter);
	}
	//Else do nothing
	else {
		EV << "Tried to remove non existing beacon information from the Ego-Adjacency matrix!" << endl;
	}
}


/*****************************************************************************************
 * Prints out the information stored in the Ego-Adjacency Matrix.
 *****************************************************************************************/
std::string EgoAdjacency::str() const {
	std::ostringstream ss;

	for(std::map<uint32_t, VectorOfSAORSBeaconBlocks>::const_iterator iter=pastBeacons.begin();iter!=pastBeacons.end();iter++) {
		ss << "Address: "<< iter->first << " - ";
		for(int i=0;i<iter->second.size();i++) {
			ss << iter->second.at(i) << "->";
		}
		ss << std::endl;
	}

	return(ss.str());
}


/*****************************************************************************************
 * Calculates the betweenness of the host, according to the Ego-Adjacency information
 * Betweenness, if the ego adjacency matrix is represented by A can be calculated as
 * follows:
 *
 * - A^2_i,j contains the number of walks of length 2 connecting i and j
 *
 * - (A^2)[1-A]_i,j gives the number of geodesic paths of length 2 joining i to j
 *
 * Therefore, summing up the reciprocal entries of (A^2)[1-A], considering only the zero
 * entries above the leading diagonal, gives us the ego betweenness.
 *****************************************************************************************/
double EgoAdjacency::calculateBetweenness() {
	int beaconCounter=1;
	double betweenness=-1;

	//Find out how many beacon information exist in the Ego Adjacency Matrix
	for(bcnInfoIter iter=pastBeacons.begin();iter!=pastBeacons.end();iter++) {
		//If the beacon information exists for this host
		if(iter->second.size()>0) {
			//!!!Be careful cause the beaconCounter is +1 bigger than the size of the beacon table!!!
			beaconCounter++;
		}
	}

	//If there is sufficient information to calculate the betweenness value
	if(beaconCounter>4) {
		//Set up matrix of zeros
		int A[beaconCounter][beaconCounter];
		memset(&A, 0, sizeof(A));

		// --- Create A matrix ---
		int neighbor=1;
		for(bcnInfoIter iter=pastBeacons.begin();iter!=pastBeacons.end();iter++) {
			//If the beacon information exists for this host
			if(iter->second.size()>0) {
				//Find the position of each address in the stored beacon info
				for(uint i=0;i<iter->second.size();i++) {
					//Get the position of the address stores in the beacon info
					int position=findBeaconInfoPosition(iter->second.at(i).getAddress().getInt());

					//if the position is found the place a 1 in the matrix
					if(position!=-1) {
						//The matrix should be symmetric
						A[neighbor][position]=1;
						A[position][neighbor]=1;
					}
				}

				//Move down the matrix
				neighbor++;
			}
		}

		//The first line and column belong to the current host so should have all ones...
		for(int i=0;i<beaconCounter;i++) {
			A[0][i]=1;
			A[i][0]=1;
		}
		A[0][0]=0;

		// --- Create the A^2 matrix ---
		int A2[beaconCounter][beaconCounter];
		memset(&A2, 0, sizeof(A2));

		for(int i=0;i<beaconCounter;i++) {
			for(int j=0;j<beaconCounter;j++) {
				for(int p=0;p<beaconCounter;p++) {
					A2[i][j]+=A[i][p]*A[p][j];
				}
				//std::cout << A[i][j] << " ";
			}
			//std::cout << endl;
		}
		//std::cout << endl;

		//Calculate Betweenness
		betweenness=0;
		for(int i=0;i<beaconCounter;i++) {
			for(int j=(i+1);j<beaconCounter;j++) {
				if(1-A[i][j]==1 && A2[i][j]!=0) {
					betweenness+=(double)((double)1/(double)A2[i][j]);
				}
			}
		}

	}

	return(betweenness);
}

/*****************************************************************************************
 * Returns whether the given address is part of the community. Communities are defined
 * by the nodes in the ego adjacency matrix and those included in the beacon block.
*****************************************************************************************/
bool EgoAdjacency::isCommuninityMenber(uint32_t address) {
    //Iterators
    bcnInfoIter iter=pastBeacons.begin();
    VectorOfSAORSBeaconBlocks::iterator biIter;

    //First check if the address is included in the beacon sending nodes
    if(pastBeacons.find(address)!=pastBeacons.end())
        return true;

    //If not, check all all the beacon information of every entry
    while(iter!=pastBeacons.end()) {
        //Check the beacon information
        //From the first beacon info entry
        biIter=iter->second.begin();

        //Until the end look for the specified address
        while(biIter!=iter->second.end()) {
            //If you found it return true
            if(biIter->getAddress().getInt()==address)
                return true;

            biIter++;
        }

        //Check the next entry
        iter++;
    }

    //Else if not found return false
    return false;
}


/*****************************************************************************************
 * Overloading the << operator for printing the output of the class.
 *****************************************************************************************/
std::ostream& operator<<(std::ostream& os, const EgoAdjacency& o)
{
	os << o.str();
	return os;
}

} // namespace inetmanet

} // namespace inet

