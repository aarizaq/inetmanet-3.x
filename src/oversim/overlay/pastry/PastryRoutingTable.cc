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
 * @file PastryRoutingTable.cc
 * @author Felix Palmen
 */

#include "PastryRoutingTable.h"

Define_Module(PastryRoutingTable);

uint32_t PastryRoutingTable::digitAt(uint32_t n,
                                     const OverlayKey& key) const
{
    return key.getBitRange(OverlayKey::getLength() - ++n * bitsPerDigit, bitsPerDigit);
}

void PastryRoutingTable::earlyInit(void)
{
    WATCH_VECTOR(rows);
}

void PastryRoutingTable::initializeTable(uint32_t bitsPerDigit,
                                         double repairTimeout,
                                         const NodeHandle& owner)
{
    this->owner = owner;
    this->repairTimeout = repairTimeout;
    this->bitsPerDigit = bitsPerDigit;
    nodesPerRow = 1 << bitsPerDigit; // 2 ^ bitsPerDigit

    // forget old routing table contents in case of restart:
    if (!rows.empty()) rows.clear();

    // clear pending repair requests:
    if (!awaitingRepair.empty()) awaitingRepair.clear();

    // Create first row in table:
    addRow();
}

const PastryExtendedNode& PastryRoutingTable::nodeAt(uint32_t row, uint32_t col) const
{
    if (rows.size() <= row) return unspecNode();
    if (col >= nodesPerRow) return unspecNode();

    return *((rows.begin()+row)->begin()+col);
}

const NodeHandle& PastryRoutingTable::lookupNextHop(const OverlayKey& destination)
{
    if (destination == owner.getKey()) opp_error("trying to lookup own key!");

    uint32_t shl = owner.getKey().sharedPrefixLength(destination) / bitsPerDigit;
    uint32_t digit = digitAt(shl, destination);

    if (shl >= rows.size()) {
        EV << "Pastry: Unable to find next hop for " << destination
        << ", row is empty." << endl;
        return NodeHandle::UNSPECIFIED_NODE;
    }

    const PastryExtendedNode& next = nodeAt(shl, digit);

    if (next.node.isUnspecified()) {
        EV << "Pastry: Unable to find next hop for " << destination <<
        ", routing table entry is empty." << endl;
    }
    return next.node;
}

const NodeHandle& PastryRoutingTable::findCloserNode(const OverlayKey& destination,
                                                     bool optimize)
{
    if (destination == owner.getKey())
        opp_error("trying to find closer node to own key!");

    const PastryExtendedNode* entry;

    if (optimize) {
        // pointer to later return value, initialize to unspecified, so
        // the specialCloserCondition() check will be done against our own
        // node as long as no node closer to the destination than our own was
        // found.
        const NodeHandle* ret = &NodeHandle::UNSPECIFIED_NODE;

        // a numerically closer node can only be found in the row containing
        // nodes with the same prefix length and in the row above.
        int shl = owner.getKey().sharedPrefixLength(destination) / bitsPerDigit;
        int digit = digitAt(shl, destination);
        int x = digitAt(shl, owner.getKey()); // position index of own node

        // first try the row with same prefix length:
        int n = nodesPerRow;
        int a = digit - 1; // position index of search to the left
        int b = digit + 1; // position index of search to the right

        while ((a >= 0) || (b < n)) {
            // no need to continue search in one direction when own entry is
            // reached:
            if (a == x) a = -1;
            if (b == x) b = n;

            if (a >= 0) {
                entry = &(nodeAt(shl, a));
                if ((!entry->node.isUnspecified()) &&
                        specialCloserCondition(entry->node, destination, *ret))
                    ret = &(entry->node);
                a--;
            }
            if (b < n) {
                entry = &(nodeAt(shl, b));
                if ((!entry->node.isUnspecified()) &&
                        specialCloserCondition(entry->node, destination, *ret))
                    ret = &(entry->node);
                b++;
            }
        }

        // it this was not the first row, two more nodes to check:
        if (shl != 0) {
            // go up one row:
            x = digitAt(--shl, owner.getKey());

            if (destination < owner.getKey()) {
                entry = &(nodeAt(shl, digit - 1));
                if ((!entry->node.isUnspecified()) &&
                        specialCloserCondition(entry->node, destination, *ret))
                    ret = &(entry->node);
            } else {
                entry = &(nodeAt(shl, digit + 1));
                if ((!entry->node.isUnspecified()) &&
                        specialCloserCondition(entry->node, destination, *ret))
                    ret = &(entry->node);
            }
        }

        return *ret; // still unspecified if no closer node was found
    } else {
        // no optimization, return the first closer node found
        for (uint32_t y = 0; y < rows.size(); ++y) {
            for (uint32_t x = 0; x < nodesPerRow; ++x) {
                entry = &(nodeAt(y, x));
                if ((!entry->node.isUnspecified()) &&
                        specialCloserCondition(entry->node, destination))
                    return entry->node;
            }
        }

        return NodeHandle::UNSPECIFIED_NODE;
    }
}

void PastryRoutingTable::findCloserNodes(const OverlayKey& destination,
                                         NodeVector* nodes)
{
    //TODO
    const PastryExtendedNode* entry;

    for (uint32_t y = 0; y < rows.size(); ++y) {
        for (uint32_t x = 0; x < nodesPerRow; ++x) {
            entry = &(nodeAt(y, x));
            if (!entry->node.isUnspecified()
            /* && specialCloserCondition(entry->node, destination)*/)
                nodes->add(entry->node);
        }
    }
}

void PastryRoutingTable::dumpToStateMessage(PastryStateMessage* msg) const
{
    uint32_t i = 0;
    uint32_t size = 0;
    std::vector<PRTRow>::const_iterator itRows;
    PRTRow::const_iterator itCols;

    msg->setRoutingTableArraySize(rows.size() * nodesPerRow);

    for (itRows = rows.begin(); itRows != rows.end(); itRows++) {
        for (itCols = itRows->begin(); itCols != itRows->end(); itCols++) {
            if (!itCols->node.isUnspecified()) {
                ++size;
                msg->setRoutingTable(i++, itCols->node);
            }
        }
    }
    msg->setRoutingTableArraySize(size);

}

void PastryRoutingTable::dumpRowToMessage(PastryRoutingRowMessage* msg,
                                          int row) const
{
    uint32_t i = 0;
    uint32_t size = 0;
    std::vector<PRTRow>::const_iterator itRows;
    PRTRow::const_iterator itCols;

    msg->setRoutingTableArraySize(nodesPerRow);
    if (row == -1) {
        itRows = rows.end() - 1;
    } else if (row > (int)rows.size()) {
        EV << "asked for nonexistent row";
        // TODO: verify this - added by ib
        msg->setRoutingTableArraySize(0);
        return;
    } else {
        itRows = rows.begin() + row - 1;
    }
    for (itCols = itRows->begin(); itCols != itRows->end(); itCols++) {
        if (!itCols->node.isUnspecified()) {
            ++size;
            msg->setRoutingTable(i++, itCols->node);
        }
    }
    msg->setRoutingTableArraySize(size);
}

//TODO ugly duplication of code
void PastryRoutingTable::dumpRowToMessage(PastryStateMessage* msg,
                                          int row) const
{
    uint32_t i = 0;
    uint32_t size = 0;
    std::vector<PRTRow>::const_iterator itRows;
    PRTRow::const_iterator itCols;

    msg->setRoutingTableArraySize(nodesPerRow);
    if ((row == -1) || (row > (int)rows.size())) {
        itRows = rows.end() - 1;
    } else if (row > (int)rows.size()) {
        EV << "asked for nonexistent row";
        // TODO: verify this - added by ib
        msg->setRoutingTableArraySize(0);
        return;
    } else {
        itRows = rows.begin() + row - 1;
    }
    for (itCols = itRows->begin(); itCols != itRows->end(); itCols++) {
        if (!itCols->node.isUnspecified()) {
            ++size;
            msg->setRoutingTable(i++, itCols->node);
        }
    }
    // TODO: verify this - added by ib
    msg->setRoutingTableArraySize(size);
}

int PastryRoutingTable::getLastRow()
{
    return rows.size();
}

const TransportAddress& PastryRoutingTable::getRandomNode(int row)
{
    std::vector<PRTRow>::const_iterator itRows;
    PRTRow::const_iterator itCols;
    uint32_t rnd;

    itRows = rows.begin() + row;
    if (itRows >= rows.end()) {
        EV << "[PastryRoutingTable::getRandomNode()]\n"
           << "    tried to get random Node from nonexistent row"
           << endl;
    }
    rnd = intuniform(0, nodesPerRow - 1, 0);
    itCols = itRows->begin() + rnd;
    while (itCols != itRows->end()) {
        if (!itCols->node.isUnspecified()) return itCols->node;
        else itCols++;
    }
    itCols = itRows->begin() + rnd;
    while (itCols >= itRows->begin()) {
        if (!itCols->node.isUnspecified()) return itCols->node;
        else itCols--;
    }
    return TransportAddress::UNSPECIFIED_NODE;
}

bool PastryRoutingTable::mergeNode(const NodeHandle& node, simtime_t prox)
{
    if (node.getKey() == owner.getKey())
        opp_error("trying to merge node with same key!");

    uint32_t shl;
    uint32_t digit;
    PRTRow::iterator position;

    shl = owner.getKey().sharedPrefixLength(node.getKey()) / bitsPerDigit;
    digit = digitAt(shl, node.getKey());

    while (rows.size() <= shl) addRow();
    position = (rows.begin() + shl)->begin() + digit;
    if (position->node.isUnspecified() || (prox < position->rtt)) {
        EV << "[PastryRoutingTable::mergeNode()]\n"
           << "    Node " << owner.getKey()
           << endl;
        EV << "        placing node " << node.getKey() << "in row "
           << shl << ", col" << digit << endl;
        if (! position->node.isUnspecified()) {
            EV << "        (replaced because of better proximity: "
            << prox << " < " << position->rtt << ")" << endl;
        }
        position->node = node;
        position->rtt = prox;
        return true;
    }
    return false;
}

bool PastryRoutingTable::initStateFromHandleVector(const std::vector<PastryStateMsgHandle>& handles)
{
    std::vector<PastryStateMsgHandle>::const_iterator it;
    int hopCheck = 0;

    for (it = handles.begin(); it != handles.end(); ++it) {
        if (it->msg->getJoinHopCount() != ++hopCheck) return false;
        mergeState(it->msg, it->prox);
    }
    return true;
}

void PastryRoutingTable::dumpToVector(std::vector<TransportAddress>& affected)
const
{
    std::vector<PRTRow>::const_iterator itRows;
    PRTRow::const_iterator itCols;

    for (itRows = rows.begin(); itRows != rows.end(); itRows++)
        for (itCols = itRows->begin(); itCols != itRows->end(); itCols++)
            if (!itCols->node.isUnspecified())
                affected.push_back(itCols->node);
}

void PastryRoutingTable::addRow(void)
{
    PRTRow row(nodesPerRow, unspecNode());

    // place own node at correct position:
    (row.begin() + digitAt(rows.size(), owner.getKey()))->node = owner;
    rows.push_back(row);
}

std::ostream& operator<<(std::ostream& os, const PRTRow& row)
{
    os << "Pastry IRoutingTable row {" << endl;
    for (PRTRow::const_iterator i = row.begin(); i != row.end(); i++) {
        os << "        " << i->node << " ; Ping: ";
        if (i->rtt != SimTime::getMaxTime())
            os << i->rtt << endl;
        else os << "<unknown>" << endl;
    }
    os << "    }";
    return os;
}

const TransportAddress& PastryRoutingTable::failedNode(const TransportAddress& failed)
{
    std::vector<PRTRow>::iterator itRows;
    PRTRow::iterator itCols;
    PRTTrackRepair tmpTrack;

    bool found = false;

    // find node in table:
    for (itRows = rows.begin(); itRows != rows.end(); itRows++) {
        for (itCols = itRows->begin(); itCols != itRows->end(); itCols++) {
            if ((! itCols->node.isUnspecified()) &&
                    (itCols->node.getIp() == failed.getIp())) {
                itCols->node = NodeHandle::UNSPECIFIED_NODE;
                itCols->rtt = PASTRY_PROX_UNDEF;
                found = true;
                break;
            }
        }
        if (found) break;
    }

    // not found, nothing to do:
    if (!found) return TransportAddress::UNSPECIFIED_NODE;

    // else fill temporary record:
    tmpTrack.failedRow = itRows - rows.begin();
    tmpTrack.failedCol = itCols - itRows->begin();
    tmpTrack.node = TransportAddress::UNSPECIFIED_NODE;
    findNextNodeToAsk(tmpTrack);
    tmpTrack.timestamp = simTime();

    if (tmpTrack.node.isUnspecified())
        return TransportAddress::UNSPECIFIED_NODE;
    awaitingRepair.push_back(tmpTrack);
    return awaitingRepair.back().node;
}

const TransportAddress& PastryRoutingTable::repair(const PastryStateMessage* msg,
                                                   const PastryStateMsgProximity* prox)
{
    std::vector<PRTTrackRepair>::iterator it;
    simtime_t now = simTime();

    // first eliminate outdated entries in awaitingRepair:
    for (it = awaitingRepair.begin(); it != awaitingRepair.end();) {
        if (it->timestamp < (now - repairTimeout))
            it = awaitingRepair.erase(it);
        else it++;
    }

    // don't expect any more repair messages:
    if (awaitingRepair.empty()) return TransportAddress::UNSPECIFIED_NODE;

    // look for source node in our list:
    for (it = awaitingRepair.begin(); it != awaitingRepair.end(); it++)
        if (it->node == msg->getSender()) break;

    // if not found, return from function:
    if (it == awaitingRepair.end()) return TransportAddress::UNSPECIFIED_NODE;

    // merge state:
    mergeState(msg, prox);

    // repair not yet done?
    if (nodeAt(it->failedRow, it->failedCol).node.isUnspecified()) {
        // ask next node
        findNextNodeToAsk(*it);
        if (it->node.isUnspecified()) {
            // no more nodes to ask, give up:
            EV << "[PastryRoutingTable::repair()]\n"
               << "    RoutingTable giving up repair attempt."
               << endl;
            awaitingRepair.erase(it);
            return TransportAddress::UNSPECIFIED_NODE;
        }
        else return it->node;
    }

    // repair done: clean up
    EV << "[PastryRoutingTable::repair()]\n"
       << "    RoutingTable repair was successful."
       << endl;
    return TransportAddress::UNSPECIFIED_NODE;
}

void PastryRoutingTable::findNextNodeToAsk(PRTTrackRepair& track) const
{
    const TransportAddress* ask;

    if (track.node.isUnspecified()) {
        track.askedRow = track.failedRow;
        if (track.failedCol == 0)
            track.askedCol = 1;
        else
            track.askedCol = 0;
        ask = static_cast<const TransportAddress*>(
                &(nodeAt(track.askedRow, track.askedCol).node));
        track.node = *ask;
        if ( (! track.node.isUnspecified()) &&
                (track.node != owner) )
            return;
    }

    do {
        // point to next possible position in routing table:
        track.askedCol++;
        if ((track.askedRow == track.failedRow) &&
                (track.askedCol == track.failedCol)) track.askedCol++;
        if (track.askedCol == nodesPerRow) {
            if ((track.askedRow > track.askedCol) ||
                    (track.askedRow == (rows.size() - 1))) {
                // no more nodes that could be asked, give up:
                track.node = TransportAddress::UNSPECIFIED_NODE;
                return;
            }
            track.askedRow++;
            track.askedCol = 0;
        }

        ask = static_cast<const TransportAddress*>(
                &(nodeAt(track.askedRow, track.askedCol).node));

//        if (!ask->isUnspecified() && !track.node.isUnspecified() && track.node == *ask)
//            std::cout << "burp! " << owner.getKey() << " " << (static_cast<const NodeHandle*>(ask))->key << "\n("
//            << track.failedRow << ", " << track.failedCol << ") -> ("
//            << track.askedRow << ", " << track.askedCol << ")"
//            << std::endl;

        if (track.node.isUnspecified() ||
            (!ask->isUnspecified() && track.node != *ask))
            track.node = *ask; //only happens if track.node == owner
        else track.node = TransportAddress::UNSPECIFIED_NODE;
    }
    while (track.node.isUnspecified() || (track.node == owner) );
}

