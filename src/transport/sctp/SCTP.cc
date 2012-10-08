//
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009-2012 Thomas Dreibholz
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


#include "SCTP.h"
#include "SCTPAssociation.h"
#include "SCTPCommand_m.h"
#include "IPv4ControlInfo.h"
#include "IPv6ControlInfo.h"
#include "IPv4Datagram.h"

#include "UDPControlInfo_m.h"
#include "UDPSocket.h"


Define_Module(SCTP);


bool  SCTP::testing;
bool  SCTP::logverbose;
bool  SCTP::checkQueues;
int32 SCTP::nextAssocId = 0;


void SCTP::printInfoAssocMap()
{
   SCTPAssociation* assoc;
   SockPair         key;
   sctpEV3 << "Number of Assocs: " << sizeAssocMap << endl;
   if (sizeAssocMap > 0) {
      for (SctpAssocMap::iterator i = sctpAssocMap.begin(); i!=sctpAssocMap.end(); ++i) {
         assoc = i->second;
         key   = i->first;
         sctpEV3 << "assocId: " << assoc->assocId << "  assoc: " << assoc
                 << " src: "    << IPvXAddress(key.localAddr)
                 << " dst: "    << IPvXAddress(key.remoteAddr)
                 << " lPort: "  << key.localPort
                 << " rPort: "  << key.remotePort << endl;
      }
      sctpEV3 << endl;
   }
}

void SCTP::printVTagMap()
{
   int32 assocId;
   VTagPair   key;
   sctpEV3 << "Number of Assocs: "<< sctpVTagMap.size() << endl;
   if (sctpVTagMap.size()>0)
   {
      for (SctpVTagMap::iterator i = sctpVTagMap.begin(); i!=sctpVTagMap.end(); ++i)
      {
         assocId = i->first;
         key = i->second;
         sctpEV3 << "assocId: "    << assocId << " peerVTag: " << key.peerVTag
                 << " localVTag: " << key.localVTag
                 << " localPort: " << key.localPort
                 << " rPort: "     << key.remotePort << endl;
      }
      sctpEV3 << endl;
   }
}

void SCTP::bindPortForUDP()
{
    EV << "Binding to UDP port " << SCTP_OVER_UDP_UDPPORT << endl;

    udpSocket.setOutputGate(gate("to_ip"));
    udpSocket.bind(SCTP_OVER_UDP_UDPPORT);
}

void SCTP::initialize()
{
   nextEphemeralPort = (uint16)(intrand(10000) + 30000);
   cModule* netw     = simulation.getSystemModule();

   checkQueues = par("checkQueues");
   testing = netw->hasPar("testing") && netw->par("testing").boolValue();
   if(testing) {
      checkQueues = true;   // Always check queues in testing mode.
   }
   if (netw->hasPar("testTimeout")) {
      testTimeout = (simtime_t)netw->par("testTimeout");
   }
   this->auth    = (bool)par("auth");
   this->pktdrop = (bool)par("packetDrop");
   this->sackNow = (bool)par("sackNow");
   numPktDropReports  = 0;
   numPacketsReceived = 0;
   numPacketsDropped  = 0;
   sizeAssocMap       = 0;
   if ((bool)par("udpEncapsEnabled")) {
      bindPortForUDP();
   }
#ifdef HAVE_GETTIMEOFDAY
   StartupTime = getMicroTime();
#endif
}

SCTP::~SCTP()
{
   if (!(sctpAppAssocMap.empty())) {
      sctpAppAssocMap.clear();
   }
   if (!(assocStatMap.empty())) {
      assocStatMap.clear();
   }
   if (!(sctpVTagMap.empty())) {
      sctpVTagMap.clear();
   }
}


void SCTP::handleMessage(cMessage *msg)
{
   IPvXAddress      destAddr;
   IPvXAddress      srcAddr;
   IPv4ControlInfo*   controlInfo   = NULL;
   IPv6ControlInfo* controlInfoV6 = NULL;
   bool findListen = false;
   bool bitError   = false;

   sctpEV3 << "SCTP::handleMessage at " << getFullPath() << endl;

   if (msg->isSelfMessage()) {
      SCTPAssociation* assoc = (SCTPAssociation*)msg->getContextPointer();
      const bool ret = assoc->processTimer(msg);
      if (!ret) {
         removeAssociation(assoc);
      }
   }
   else if (msg->arrivedOn("from_ip") || msg->arrivedOn("from_ipv6")) {
      sctpEV3 << "Message from IP" << endl;
      printInfoAssocMap();
      if (!dynamic_cast<SCTPMessage *>(msg)) {
         sctpEV3 << "no sctp message, delete it" << endl;
         delete msg;
         return;
      }
      SCTPMessage* sctpmsg = check_and_cast<SCTPMessage*>(msg);

      numPacketsReceived++;

      if (!pktdrop && (sctpmsg->hasBitError() || !(sctpmsg->getChecksumOk()))) {
         sctpEV3 << "Packet has bit-error. delete it" << endl;
         bitError = true;
         numPacketsDropped++;
         delete msg;
         return;
      }
      if (msg->arrivedOn("from_ip")) {
         if (par("udpEncapsEnabled")) {
             std::cout<<"Size of SCTPMSG="<<sctpmsg->getByteLength()<<"\n";
             UDPDataIndication *ctrl = check_and_cast<UDPDataIndication *>(msg->removeControlInfo());
             srcAddr = ctrl->getSrcAddr();
             destAddr = ctrl->getDestAddr();
             std::cout<<"controlInfo srcAddr="<<srcAddr<<" destAddr="<<destAddr<<"\n";
             std::cout<<"VTag="<<sctpmsg->getTag()<<"\n";
         }
         else {
            controlInfo          = check_and_cast<IPv4ControlInfo *>(msg->removeControlInfo());
            IPv4Datagram* datagram = controlInfo->removeOrigDatagram();
            delete datagram;
            srcAddr  = controlInfo->getSrcAddr();
            destAddr = controlInfo->getDestAddr();
         }
      }
      else
      {
         controlInfoV6 = check_and_cast<IPv6ControlInfo *>(msg->removeControlInfo());
         srcAddr  = controlInfoV6->getSrcAddr();
         destAddr = controlInfoV6->getDestAddr();
      }

      if (sctpmsg->getBitLength()>(SCTP_COMMON_HEADER*8)) {
         if (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()==INIT ||
             ((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()==INIT_ACK ||
             ((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()==PKTDROP) {
            findListen = true;
         }

         SCTPAssociation* assoc = findAssocForMessage(srcAddr, destAddr, sctpmsg->getSrcPort(),sctpmsg->getDestPort(), findListen);
         if (!assoc && sctpAssocMap.size()>0)
            assoc = findAssocWithVTag(sctpmsg->getTag(),sctpmsg->getSrcPort(), sctpmsg->getDestPort());
         if (!assoc) {
            if (bitError) {
               delete sctpmsg;
               return;
            }
            if (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()==SHUTDOWN_ACK) {
               sendShutdownCompleteFromMain(sctpmsg, destAddr, srcAddr);
            }
            else if (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()!=ABORT  &&
                     ((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()!=SHUTDOWN_COMPLETE) {
               sendAbortFromMain(sctpmsg, destAddr, srcAddr);
            }
            delete sctpmsg;
         }
         else
         {
            bool ret = assoc->processSCTPMessage(sctpmsg, srcAddr, destAddr);
            if (!ret) {
               removeAssociation(assoc);
               delete sctpmsg;
            }
            else {
               delete sctpmsg;
            }
         }
      }
      else {
         delete sctpmsg;
      }
   }
   else // must be from app
   {
      SCTPCommand* controlInfo = check_and_cast<SCTPCommand *>(msg->getControlInfo());

      int32 appGateIndex;
      if (controlInfo->getGate() != -1) {
         appGateIndex = controlInfo->getGate();
      }
      else {
         appGateIndex = msg->getArrivalGate()->getIndex();
      }
      int32 assocId = controlInfo->getAssocId();
      sctpEV3 << "msg arrived from app for assoc " << assocId << endl;

      SCTPAssociation* assoc = findAssocForApp(appGateIndex, assocId);
      if (!assoc) {
         sctpEV3 << "no assoc found. msg=" << msg->getName()
                 << " number of assocs = " << assocList.size() << endl;

         if ((strcmp(msg->getName(),"PassiveOPEN")==0) ||
             (strcmp(msg->getName(),"Associate")==0)) {
            if (assocList.size() > 0) {
               assoc = NULL;
               SCTPOpenCommand* open = check_and_cast<SCTPOpenCommand*>(controlInfo);
               sctpEV3 << "Looking for assoc with remoteAddr="<<open->getRemoteAddr()<<", remotePort="<<open->getRemotePort()<<", localPort="<<open->getLocalPort()<< endl;
               for (std::list<SCTPAssociation*>::iterator iter=assocList.begin(); iter!=assocList.end(); iter++)
               {
                  sctpEV3 << "remoteAddr="<<(*iter)->remoteAddr<<", remotePort="<<(*iter)->remotePort<<", localPort="<<(*iter)->localPort<< endl;
                  if ((*iter)->remoteAddr == open->getRemoteAddr() && (*iter)->localPort==open->getLocalPort() && (*iter)->remotePort==open->getRemotePort())
                  {
                     assoc = (*iter);
                     break;
                  }
               }
            }
            if (assoc==NULL) {
               assoc = new SCTPAssociation(this,appGateIndex,assocId);

               AppAssocKey key;
               key.appGateIndex    = appGateIndex;
               key.assocId         = assocId;
               sctpAppAssocMap[key] = assoc;
               sctpEV3 << "SCTP association created for appGateIndex " << appGateIndex
                       << " and assoc "<<assocId << endl;

               const bool ret = assoc->processAppCommand(PK(msg));
               if (!ret) {
                  removeAssociation(assoc);
               }
            }
         }
      }
      else
      {
         sctpEV3 << "assoc found" << endl;
         const bool ret = assoc->processAppCommand(PK(msg));
         if (!ret) {
            removeAssociation(assoc);
         }
      }
      delete msg;
   }
   if (ev.isGUI()) {
      updateDisplayString();
   }
}

void SCTP::sendAbortFromMain(SCTPMessage*       sctpmsg,
                             const IPvXAddress& srcAddr,
                             const IPvXAddress& destAddr)
{
   SCTPMessage* msg = new SCTPMessage();

   sctpEV3 << "SCTP::sendAbortFromMain" << endl;

   msg->setSrcPort(sctpmsg->getDestPort());
   msg->setDestPort(sctpmsg->getSrcPort());
   msg->setBitLength(SCTP_COMMON_HEADER*8);
   msg->setChecksumOk(true);

   SCTPAbortChunk* abortChunk = new SCTPAbortChunk("ABORT");
   abortChunk->setChunkType(ABORT);
   if ((sctpmsg->getChunksArraySize()>0) &&
       (((SCTPChunk*)(sctpmsg->getChunks(0)))->getChunkType()==INIT)) {

      SCTPInitChunk* initChunk = check_and_cast<SCTPInitChunk *>(sctpmsg->getChunks(0));
      abortChunk->setT_Bit(0);
      msg->setTag(initChunk->getInitTag());
   }
   else {
      abortChunk->setT_Bit(1);
      msg->setTag(sctpmsg->getTag());
   }
   abortChunk->setBitLength(SCTP_ABORT_CHUNK_LENGTH*8);
   msg->addChunk(abortChunk);
   if ((bool)par("udpEncapsEnabled")) {
      std::cout<<"VTag="<<msg->getTag()<<"\n";
      udpSocket.sendTo(msg, destAddr, SCTP_OVER_UDP_UDPPORT);
   }
   else {
      IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
      controlInfo->setProtocol(IP_PROT_SCTP);
      controlInfo->setSrcAddr(srcAddr.get4());
      controlInfo->setDestAddr(destAddr.get4());
      msg->setControlInfo(controlInfo);
   }
   send(msg,"to_ip");
}

void SCTP::sendShutdownCompleteFromMain(SCTPMessage*       sctpmsg,
                                        const IPvXAddress& srcAddr,
                                        const IPvXAddress& destAddr)
{
   SCTPMessage* msg = new SCTPMessage();

   sctpEV3 << "SCTP::sendShutdownCompleteFromMain" << endl;

   msg->setSrcPort(sctpmsg->getDestPort());
   msg->setDestPort(sctpmsg->getSrcPort());
   msg->setBitLength(SCTP_COMMON_HEADER*8);
   msg->setChecksumOk(true);

   SCTPShutdownCompleteChunk* scChunk = new SCTPShutdownCompleteChunk("SHUTDOWN_COMPLETE");
   scChunk->setChunkType(SHUTDOWN_COMPLETE);
   scChunk->setTBit(1);
   msg->setTag(sctpmsg->getTag());

   scChunk->setBitLength(SCTP_SHUTDOWN_ACK_LENGTH*8);
   msg->addChunk(scChunk);
   IPv4ControlInfo *controlInfo = new IPv4ControlInfo();
   controlInfo->setProtocol(IP_PROT_SCTP);
   controlInfo->setSrcAddr(srcAddr.get4());
   controlInfo->setDestAddr(destAddr.get4());
   msg->setControlInfo(controlInfo);

   send(msg,"to_ip");
}


void SCTP::updateDisplayString()
{
   if (ev.disable_tracing)
   {
      // in express mode, we don't bother to update the display
      // (std::map's iteration is not very fast if map is large)
      getDisplayString().setTagArg("t",0,"");
      return;
   }

}

SCTPAssociation* SCTP::findAssocWithVTag(const uint32 peerVTag,
                                         const uint32 remotePort,
                                         const uint32 localPort)
{
   printVTagMap();
   sctpEV3 << "findAssocWithVTag: peerVTag=" << peerVTag
           << " srcPort="  << remotePort
           << " destPort=" << localPort << endl;
   printInfoAssocMap();

   // try with fully qualified SockPair
   for (SctpVTagMap::iterator i=sctpVTagMap.begin(); i!=sctpVTagMap.end();i++) {
      if ((i->second.peerVTag==peerVTag && i->second.localPort==localPort
         && i->second.remotePort==remotePort)
         || (i->second.localVTag==peerVTag && i->second.localPort==localPort
         && i->second.remotePort==remotePort))
         return getAssoc(i->first);
   }
   return NULL;
}

SCTPAssociation* SCTP::findAssocForMessage(const IPvXAddress& srcAddr,
                                           const IPvXAddress& destAddr,
                                           const uint32       srcPort,
                                           const uint32       destPort,
                                           const bool         findListen)
{
   SockPair key;

   key.localAddr  = destAddr;
   key.remoteAddr = srcAddr;
   key.localPort  = destPort;
   key.remotePort = srcPort;
   SockPair save = key;
   sctpEV3 << "findAssocForMessage: localAddr=" << destAddr << " remoteAddr=" << srcAddr
           << " localPort=" << destPort << " remotePort=" << srcPort << endl;
   printInfoAssocMap();

   // try with fully qualified SockPair
   SctpAssocMap::iterator i;
   i = sctpAssocMap.find(key);
   if (i!=sctpAssocMap.end())
      return i->second;


   // try with localAddr missing (only localPort specified in passive/active open)
   key.localAddr.set("0.0.0.0");

   i = sctpAssocMap.find(key);
   if (i!=sctpAssocMap.end()) {
      return i->second;
   }

   if (findListen == true) {
      // try fully qualified local socket + blank remote socket (for incoming SYN)
      key = save;
      key.remoteAddr.set("0.0.0.0");
      key.remotePort = 0;
      i = sctpAssocMap.find(key);
      if (i!=sctpAssocMap.end()) {
         return i->second;
      }

      // try with blank remote socket, and localAddr missing (for incoming SYN)
      key.localAddr.set("0.0.0.0");
      i = sctpAssocMap.find(key);
      if (i != sctpAssocMap.end()) {
         return i->second;
      }
   }

   // given up
   sctpEV3 << "giving up on trying to find assoc for localAddr=" << destAddr << " remoteAddr=" << srcAddr
           << " localPort=" << destPort << " remotePort=" << srcPort << endl;
   return NULL;
}

SCTPAssociation* SCTP::findAssocForApp(const int32 appGateIndex,
                                       const int32 assocId)
{
   AppAssocKey key;
   key.appGateIndex = appGateIndex;
   key.assocId      = assocId;
   sctpEV3 << "findAssoc for appGateIndex "<<appGateIndex<<" and assoc "<<assocId<< endl;
   SctpAppAssocMap::iterator i = sctpAppAssocMap.find(key);
   return((i == sctpAppAssocMap.end()) ? NULL : i->second);
}

int16 SCTP::getEphemeralPort()
{
   if (nextEphemeralPort == 5000) {
      error("Ephemeral port range 1024..4999 exhausted (email SCTP model "
            "author that he should implement reuse of ephemeral ports!!!)");
   }
   return nextEphemeralPort++;
}

void SCTP::updateSockPair(SCTPAssociation*   assoc,
                          const IPvXAddress& localAddr,
                          const IPvXAddress& remoteAddr,
                          const int32        localPort,
                          const int32        remotePort)
{
   sctpEV3 << "updateSockPair: localAddr=" << localAddr <<" remoteAddr=" << remoteAddr
           <<" localPort=" << localPort << " remotePort=" << remotePort << endl;

   SockPair key;
   key.localAddr  = (assoc->localAddr = localAddr);
   key.remoteAddr = (assoc->remoteAddr = remoteAddr);
   key.localPort  = assoc->localPort = localPort;
   key.remotePort = assoc->remotePort = remotePort;

   for (SctpAssocMap::iterator i=sctpAssocMap.begin(); i!=sctpAssocMap.end(); i++) {
      if (i->second == assoc) {
         sctpAssocMap.erase(i);
         break;
      }
   }

   sctpAssocMap[key] = assoc;
   sizeAssocMap = sctpAssocMap.size();

   printInfoAssocMap();
}

void SCTP::addLocalAddress(SCTPAssociation*   assoc,
                           const IPvXAddress& address)
{
   SockPair key;
   key.localAddr  = assoc->localAddr;
   key.remoteAddr = assoc->remoteAddr;
   key.localPort  = assoc->localPort;
   key.remotePort = assoc->remotePort;

   SctpAssocMap::iterator i = sctpAssocMap.find(key);
   if (i!=sctpAssocMap.end()) {
      ASSERT(i->second == assoc);
      if (key.localAddr.isUnspecified()) {
         sctpAssocMap.erase(i);
         sizeAssocMap--;
      }
   }

   key.localAddr = address;
   sctpAssocMap[key] = assoc;
   sizeAssocMap = sctpAssocMap.size();
   sctpEV3 << "number of assocections="<<sizeAssocMap<< endl;

   printInfoAssocMap();
}

void SCTP::addLocalAddressToAllRemoteAddresses(SCTPAssociation*                assoc,
                                               const IPvXAddress&              address,
                                               const std::vector<IPvXAddress>& remAddresses)
{
   SockPair key;
   for (AddressVector::const_iterator i=remAddresses.begin(); i!=remAddresses.end(); ++i) {
      key.localAddr = assoc->localAddr;
      key.remoteAddr = (*i);
      key.localPort = assoc->localPort;
      key.remotePort = assoc->remotePort;

      SctpAssocMap::iterator j = sctpAssocMap.find(key);
      if (j!=sctpAssocMap.end()) {
         ASSERT(j->second==assoc);
         if (key.localAddr.isUnspecified()) {
            sctpAssocMap.erase(j);
            sizeAssocMap--;
         }
      }

      key.localAddr = address;
      sctpAssocMap[key] = assoc;
      sizeAssocMap++;
      sctpEV3 << "number of assocections=" << sctpAssocMap.size() << endl;

      printInfoAssocMap();
   }
}

void SCTP::removeLocalAddressFromAllRemoteAddresses(SCTPAssociation*                assoc,
                                                    const IPvXAddress&              address,
                                                    const std::vector<IPvXAddress>& remAddresses)
{
   SockPair key;
   for (AddressVector::const_iterator i=remAddresses.begin(); i!=remAddresses.end(); ++i) {
      key.localAddr  = address;
      key.remoteAddr = (*i);
      key.localPort  = assoc->localPort;
      key.remotePort = assoc->remotePort;

      SctpAssocMap::iterator j = sctpAssocMap.find(key);
      if (j!=sctpAssocMap.end()) {
         ASSERT(j->second == assoc);
         sctpAssocMap.erase(j);
         sizeAssocMap--;
      }

      printInfoAssocMap();
   }
}

void SCTP::removeRemoteAddressFromAllAssociations(SCTPAssociation*                assoc,
                                                  const IPvXAddress&              address,
                                                  const std::vector<IPvXAddress>& locAddresses)
{
   SockPair key;
   for (AddressVector::const_iterator i=locAddresses.begin(); i!=locAddresses.end(); i++) {
      key.localAddr  = (*i);
      key.remoteAddr = address;
      key.localPort  = assoc->localPort;
      key.remotePort = assoc->remotePort;

      SctpAssocMap::iterator j = sctpAssocMap.find(key);
      if (j!=sctpAssocMap.end()) {
         ASSERT(j->second == assoc);
         sctpAssocMap.erase(j);
         sizeAssocMap--;
      }

      printInfoAssocMap();
   }
}

void SCTP::addRemoteAddress(SCTPAssociation*   assoc,
                            const IPvXAddress& localAddress,
                            const IPvXAddress& remoteAddress)
{
   sctpEV3 << "Add remote address " << remoteAddress
           << " to local address " << localAddress << endl;

   SockPair key;
   key.localAddr  = localAddress;
   key.remoteAddr = remoteAddress;
   key.localPort  = assoc->localPort;
   key.remotePort = assoc->remotePort;

   SctpAssocMap::iterator i = sctpAssocMap.find(key);
   if (i!=sctpAssocMap.end()) {
      ASSERT(i->second == assoc);
   }
   else {
      sctpAssocMap[key] = assoc;
      sizeAssocMap++;
   }

   printInfoAssocMap();
}

void SCTP::addForkedAssociation(SCTPAssociation*   assoc,
                                SCTPAssociation*   newAssoc,
                                const IPvXAddress& localAddr,
                                const IPvXAddress& remoteAddr,
                                const int32        localPort,
                                const int32        remotePort)
{
   sctpEV3 << "addForkedAssociation assocId= "<< assoc->assocId
           << " newId=" << newAssoc->assocId << endl;

   SockPair keyAssoc;
   for (SctpAssocMap::iterator j=sctpAssocMap.begin(); j!=sctpAssocMap.end(); ++j) {
      if (assoc->assocId==j->second->assocId) {
         keyAssoc = j->first;
      }
   }

   // update assoc's socket pair, and register newAssoc (which'll keep LISTENing)
   updateSockPair(assoc, localAddr, remoteAddr, localPort, remotePort);
   updateSockPair(newAssoc, keyAssoc.localAddr, keyAssoc.remoteAddr, keyAssoc.localPort, keyAssoc.remotePort);

   // assoc will get a new assocId...
   AppAssocKey key;
   key.appGateIndex = assoc->appGateIndex;
   key.assocId      = assoc->assocId;
   sctpAppAssocMap.erase(key);
   key.assocId = assoc->assocId = getNewAssocId();
   sctpAppAssocMap[key] = assoc;

   // ...and newAssoc will live on with the old assocId
   key.appGateIndex = newAssoc->appGateIndex;
   key.assocId      = newAssoc->assocId;
   sctpAppAssocMap[key] = newAssoc;

   printInfoAssocMap();
}

void SCTP::removeAssociation(SCTPAssociation* assoc)
{
   bool        ok   = false;
   bool        find = false;
   const int32 id   = assoc->assocId;

   sctpEV3 << "removeAssociation assocId= "<< id << endl;

   printInfoAssocMap();
   if (sizeAssocMap > 0) {
      AssocStatMap::iterator assocStatMapIterator = assocStatMap.find(assoc->assocId);
      if (assocStatMapIterator != assocStatMap.end()) {
         assocStatMapIterator->second.stop       = simulation.getSimTime();
         assocStatMapIterator->second.lifeTime   = assocStatMapIterator->second.stop - assocStatMapIterator->second.start;
         assocStatMapIterator->second.throughput = assocStatMapIterator->second.ackedBytes*8 / assocStatMapIterator->second.lifeTime.dbl();
      }
      while (!ok) {
         if (sizeAssocMap == 0) {
            ok = true;
         }
         else {
            for (SctpAssocMap::iterator sctpAssocMapIterator = sctpAssocMap.begin();
                 sctpAssocMapIterator != sctpAssocMap.end(); sctpAssocMapIterator++) {
               if (sctpAssocMapIterator->second != NULL) {
                  SCTPAssociation* myAssoc = sctpAssocMapIterator->second;
                  if (myAssoc->assocId == assoc->assocId) {
                     if (myAssoc->T1_InitTimer) {
                        myAssoc->stopTimer(myAssoc->T1_InitTimer);
                     }
                     if (myAssoc->T2_ShutdownTimer) {
                        myAssoc->stopTimer(myAssoc->T2_ShutdownTimer);
                     }
                     if (myAssoc->T5_ShutdownGuardTimer) {
                        myAssoc->stopTimer(myAssoc->T5_ShutdownGuardTimer);
                     }
                     if (myAssoc->SackTimer) {
                        myAssoc->stopTimer(myAssoc->SackTimer);
                     }
                     if (myAssoc->StartAddIP) {
                        myAssoc->stopTimer(myAssoc->StartAddIP);
                     }
                     sctpAssocMap.erase(sctpAssocMapIterator);
                     sizeAssocMap--;
                     find = true;
                     break;
                  }
               }
            }
         }

         if (!find) {
            ok = true;
         }
         else {
            find = false;
         }
      }
   }

   // T.D. 26.11.09: Write statistics
   char str[128];
   for (SCTPAssociation::SCTPPathMap::iterator pathMapIterator = assoc->sctpPathMap.begin();
        pathMapIterator != assoc->sctpPathMap.end(); pathMapIterator++) {
      const SCTPPathVariables* path = pathMapIterator->second;
      snprintf((char*)&str, sizeof(str), "Number of Fast Retransmissions %d:%s",
               assoc->assocId, path->remoteAddress.str().c_str());
      recordScalar(str, path->numberOfFastRetransmissions);
      snprintf((char*)&str, sizeof(str), "Number of Timer-Based Retransmissions %d:%s",
               assoc->assocId, path->remoteAddress.str().c_str());
      recordScalar(str, path->numberOfTimerBasedRetransmissions);
      snprintf((char*)&str, sizeof(str), "Number of Duplicates %d:%s",
               assoc->assocId, path->remoteAddress.str().c_str());
      recordScalar(str, path->numberOfDuplicates);
      path->statisticsPathOutstandingBytes->setUtilizationMaximum(assoc->state->sendQueueLimit);
      path->statisticsPathQueuedSentBytes->setUtilizationMaximum(assoc->state->sendQueueLimit);
   }
   assoc->statisticsQueuedSentBytes->setUtilizationMaximum(assoc->state->sendQueueLimit);
   assoc->statisticsQueuedReceivedBytes->setUtilizationMaximum((double)par("arwnd"));

   assoc->removePath();
   assoc->deleteStreams();

   // TD 20.11.09: Chunks may be in the transmission and retransmission queues simultaneously.
   //              Remove entry from transmission queue if it is already in the retransmission queue.
   for (SCTPQueue::PayloadQueue::iterator i = assoc->getRetransmissionQueue()->payloadQueue.begin();
        i != assoc->getRetransmissionQueue()->payloadQueue.end(); i++) {
      SCTPQueue::PayloadQueue::iterator j = assoc->getTransmissionQueue()->payloadQueue.find(i->second->tsn);
      if(j != assoc->getTransmissionQueue()->payloadQueue.end()) {
         assoc->getTransmissionQueue()->payloadQueue.erase(j);
      }
   }
    // TD 20.11.09: Now, both queues can be safely deleted.
   delete assoc->getRetransmissionQueue();
   delete assoc->getTransmissionQueue();

   AppAssocKey key;
   key.appGateIndex = assoc->appGateIndex;
   key.assocId      = assoc->assocId;
   sctpAppAssocMap.erase(key);
   assocList.remove(assoc);
   delete assoc;
}

SCTPAssociation* SCTP::getAssoc(const int32 assocId)
{
   for (SctpAppAssocMap::iterator i = sctpAppAssocMap.begin(); i!=sctpAppAssocMap.end(); i++) {
      if (i->first.assocId == assocId) {
         return i->second;
      }
   }
   return NULL;
}

void SCTP::finish()
{
   SctpAssocMap::iterator assocMapIterator = sctpAssocMap.begin();
   while (assocMapIterator != sctpAssocMap.end()) {
      removeAssociation(assocMapIterator->second);
      assocMapIterator = sctpAssocMap.begin();
   }
   ev << getFullPath() << ": finishing SCTP with "
      << sctpAssocMap.size() << " connections open." << endl;

   for (AssocStatMap::const_iterator iterator = assocStatMap.begin();
        iterator != assocStatMap.end(); iterator++) {
      const SCTP::AssocStat& assoc = iterator->second;

      ev << "Association " << assoc.assocId << ": started at " << assoc.start
         << " and finished at " << assoc.stop << " --> lifetime: " << assoc.lifeTime << endl;
      ev << "Association " << assoc.assocId << ": sent bytes=" << assoc.sentBytes
         << ", acked bytes=" << assoc.ackedBytes<< ", throughput=" << assoc.throughput<< " bit/s" << endl;
      ev << "Association " << assoc.assocId << ": transmitted Bytes="
         << assoc.transmittedBytes<< ", retransmitted Bytes=" << assoc.transmittedBytes-assoc.ackedBytes<< endl;
      ev << "Association " << assoc.assocId << ": number of Fast RTX="
         << assoc.numFastRtx << ", number of Timer-Based RTX=" << assoc.numT3Rtx
         << ", path failures=" << assoc.numPathFailures<< ", ForwardTsns=" << assoc.numForwardTsn<< endl;
      ev << "AllMessages=" <<numPacketsReceived<< " BadMessages=" <<numPacketsDropped<< endl;

      recordScalar("Association Lifetime",  assoc.lifeTime);
      recordScalar("Acked Bytes",           assoc.ackedBytes);
      recordScalar("Throughput [bit/s]",    assoc.throughput);
      recordScalar("Transmitted Bytes",     assoc.transmittedBytes);
      recordScalar("Packets Received",      numPacketsReceived);
      recordScalar("Packets Dropped",       numPacketsDropped);
      recordScalar("Fast RTX",              assoc.numFastRtx);
      recordScalar("Timer-Based RTX",       assoc.numT3Rtx);
      recordScalar("Sum of R Gap Ranges",   assoc.sumRGapRanges);
      recordScalar("Sum of NR Gap Ranges",  assoc.sumNRGapRanges);
      recordScalar("Duplicate Acks",        assoc.numDups);
      recordScalar("Overfull SACKs",        assoc.numOverfullSACKs);
      recordScalar("Drops Because New TSN Greater Than Highest TSN", assoc.numDropsBecauseNewTSNGreaterThanHighestTSN);
      recordScalar("Drops Because No Room In Buffer",                assoc.numDropsBecauseNoRoomInBuffer);
      recordScalar("Chunks Reneged",                                 assoc.numChunksReneged);
      recordScalar("sackPeriod", (simtime_t)par("sackPeriod"));
      if ((double)par("fairStart") > 0) {
         recordScalar("fair acked bytes",assoc.fairAckedBytes);
         recordScalar("fair start time",assoc.fairStart);
         recordScalar("fair stop time",assoc.fairStop);
         recordScalar("fair lifetime", assoc.fairLifeTime);
         recordScalar("fair throughput", assoc.fairThroughput);
      }
      recordScalar("Number of PacketDrop Reports", numPktDropReports);

      if ((assoc.cumEndToEndDelay / assoc.numEndToEndMessages) > 0) {
         uint32 msgnum = assoc.numEndToEndMessages - assoc.startEndToEndDelay;
         if (assoc.stopEndToEndDelay > 0)
             msgnum -= (assoc.numEndToEndMessages - assoc.stopEndToEndDelay);
         recordScalar("Average End to End Delay", assoc.cumEndToEndDelay / msgnum);
      }

      recordScalar("RTXMethod", (double)par("RTXMethod"));
#ifdef HAVE_GETTIMEOFDAY
      const unsigned long long shutdownTime = getMicroTime();
      recordScalar("Real Time Consumed", (double)(shutdownTime - StartupTime) / 1000000.0);
#endif
   }
}


const char* SCTP::intToChunk(const uint32 type)
{
   switch (type)
   {
      case   0: return "DATA";
      case   1: return "INIT";
      case   2: return "INIT_ACK";
      case   3: return "SACK";
      case   4: return "HEARTBEAT";
      case   5: return "HEARTBEAT_ACK";
      case   6: return "ABORT";
      case   7: return "SHUTDOWN";
      case   8: return "SHUTDOWN_ACK";
      case   9: return "ERRORTYPE";
      case  10: return "COOKIE_ECHO";
      case  11: return "COOKIE_ACK";
      case  14: return "SHUTDOWN_COMPLETE";
      case  15: return "AUTH";
      case  16: return "NR-SACK";
      case 128: return "ASCONF_ACK";
      case 129: return "PKTDROP";
      case 130: return "STREAM_RESET";
      case 192: return "FORWARD_TSN";
      case 193: return "ASCONF";
   }
   return("???");
}

uint32 SCTP::chunkToInt(const char* type)
{
   if (strcmp(type, "DATA")==0)              return 0;
   if (strcmp(type, "INIT")==0)              return 1;
   if (strcmp(type, "INIT_ACK")==0)          return 2;
   if (strcmp(type, "SACK")==0)              return 3;
   if (strcmp(type, "HEARTBEAT")==0)         return 4;
   if (strcmp(type, "HEARTBEAT_ACK")==0)     return 5;
   if (strcmp(type, "ABORT")==0)             return 6;
   if (strcmp(type, "SHUTDOWN")==0)          return 7;
   if (strcmp(type, "SHUTDOWN_ACK")==0)      return 8;
   if (strcmp(type, "ERRORTYPE")==0)         return 9;
   if (strcmp(type, "COOKIE_ECHO")==0)       return 10;
   if (strcmp(type, "COOKIE_ACK")==0)        return 11;
   if (strcmp(type, "SHUTDOWN_COMPLETE")==0) return 14;
   if (strcmp(type, "AUTH")==0)              return 15;
   if (strcmp(type, "NR-SACK")==0)           return 16;
   if (strcmp(type, "ASCONF_ACK")==0)        return 128;
   if (strcmp(type, "PKTDROP")==0)           return 129;
   if (strcmp(type, "STREAM_RESET")==0)      return 130;
   if (strcmp(type, "FORWARD_TSN")==0)       return 192;
   if (strcmp(type, "ASCONF")==0)            return 193;
   sctpEV3 << "ChunkConversion not successful" << endl;
   return(0xffffffff);
}

#ifdef HAVE_GETTIMEOFDAY
unsigned long long SCTP::getMicroTime()
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return(((unsigned long long)tv.tv_sec * (unsigned long long)1000000) +
         (unsigned long long)tv.tv_usec);
}
#endif
