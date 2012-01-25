//
// Copyright (C) 2010 Karlsruhe Institute of Technology (KIT),
//                    Institute of Telematics
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
 * @file PeerStorage.h
 * @author Ingmar Baumgart
 */

#ifndef __PEERSTORAGE_H__
#define __PEERSTORAGE_H__

#include <vector>
#include <list>

#include <oversim_mapset.h>
#include <IPvXAddress.h>
#include <TransportAddress.h>
#include <HashFunc.h>
#include <PeerInfo.h>

/**
 * BootstrapEntry consists of
 * TransportAddress and PeerInfo
 * and is used (together with
 * IPvXAddress) as an entry in the peerSet
 */
struct BootstrapEntry
{
    TransportAddress* node;
    PeerInfo* info;
    uint32_t peerVectorIndex;
    friend std::ostream& operator<<(std::ostream& Stream, const BootstrapEntry entry);
};

typedef UNORDERED_MAP<IPvXAddress, BootstrapEntry> PeerHashMap;


/**
 *
 * @author IngmarBaumgart
 */
class PeerStorage
{
public:
    ~PeerStorage();
    size_t size();
    const PeerHashMap::iterator find(const IPvXAddress& ip);
    const PeerHashMap::iterator begin();
    const PeerHashMap::iterator end();
    std::pair<const PeerHashMap::iterator, bool> insert(const std::pair<IPvXAddress, BootstrapEntry>& element);
    void erase(const PeerHashMap::iterator it);
    const PeerHashMap::iterator getRandomNode(int32_t nodeType,
                                              bool bootstrappedNeeded,
                                              bool inoffensiveNeeded);
    void setMalicious(const PeerHashMap::iterator it, bool malicious);
    void setBootstrapped(const PeerHashMap::iterator it, bool bootstrapped);
    const PeerHashMap& getPeerHashMap() { return peerHashMap; };

private:
    void insertMapIteratorIntoVector(PeerHashMap::iterator it);
    void removeMapIteratorFromVector(PeerHashMap::iterator it);
    inline size_t offsetSize();
    inline uint8_t calcOffset(bool bootstrapped, bool malicious);

    PeerHashMap peerHashMap;
    std::vector<std::vector<PeerHashMap::iterator> > peerVector;
    std::vector<std::vector<uint32_t> >freeVector;
};

#endif
