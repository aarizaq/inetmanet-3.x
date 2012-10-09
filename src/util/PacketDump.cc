//
// Copyright (C) 2005 Michael Tuexen
// Copyright (C) 2008 Irene Ruengeler
// Copyright (C) 2009 Thomas Dreibholz
// Copyright (C) 2009 Thomas Reschka
// Copyright (C) 2011 Zoltan Bojthe
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include <errno.h>

#include "PacketDump.h"

#ifdef WITH_UDP
#include "UDPPacket_m.h"
#endif

#ifdef WITH_SCTP
#include "SCTPMessage.h"
#include "SCTPAssociation.h"
#endif

#ifdef WITH_TCP_COMMON
#include "TCPSegment.h"
#endif

#ifdef WITH_IPv4
#include "ICMPMessage.h"
#include "IPv4Datagram.h"
#endif

#ifdef WITH_IPv6
#include "IPv6Datagram.h"
#endif


PacketDump::PacketDump()
{
     outp = &std::cout;
     verbose = false;
}

PacketDump::~PacketDump()
{
}

void PacketDump::sctpDump(const char *label, SCTPMessage *sctpmsg, 
	    const std::string& srcAddr, const std::string& destAddr, const char *comment)
{
   std::ostream& out = *outp;
   uint32 numberOfChunks;
   SCTPChunk* chunk;
   uint8 type;
   SCTPParameter* sctpParam;
   // seq and time (not part of the tcpdump format)
   char buf[30];
   sprintf(buf,"[%.3f%s] ", simulation.getSimTime().dbl(), label);
   out << buf;

#ifndef WITH_SCTP
    out << "[sctp] " << srcAddr  << " > " << destAddr;
#else
   // src/dest

   out << srcAddr  << "." << sctpmsg->getSrcPort()  << " > "
        << destAddr << "." << sctpmsg->getDestPort() << ": ";
    
   if (sctpmsg->hasBitError())
   {
      sctpmsg->setChecksumOk(false);
   }
   numberOfChunks = sctpmsg->getChunksArraySize();
   out << "numberOfChunks="<<numberOfChunks<<" VTag="<<sctpmsg->getTag()<<"\n";
   if (sctpmsg->hasBitError())
      out << "Packet has bit error!!\n";
   for (uint32 i=0; i<numberOfChunks; i++)
   {
      chunk = (SCTPChunk*)sctpmsg->getChunks(i);
      type   = chunk->getChunkType();
      switch (type)
      {
         case INIT:
            out << "INIT ";
            break;
         case INIT_ACK:
            out << "INIT_ACK ";
            break;
         case COOKIE_ECHO:
            out << "COOKIE_ECHO ";
            break;
         case COOKIE_ACK:
            out << "COOKIE_ACK ";
            break;
         case DATA:
            out << "DATA ";
            break;
         case SACK:
            out << "SACK ";
            break;
         case HEARTBEAT:
            out << "HEARTBEAT ";
            break;
         case HEARTBEAT_ACK:
            out << "HEARTBEAT_ACK ";
            break;
         case ABORT:
            out << "ABORT ";
            break;
         case FORWARD_TSN:
            out << "FORWARD_TSN";
            break;
         case STREAM_RESET:
            out << "STREAM_RESET";
            break;
         case ASCONF:
            out << "ASCONF";
            break;
         case ASCONF_ACK:
            out << "ASCONF_ACK";
            break;
         case AUTH:
            out << "AUTH ";
            break;
         case PKTDROP:
            out << "PACKET_DROP ";
            break;
         case SHUTDOWN:
            out << "SHUTDOWN ";
            break;
         case SHUTDOWN_ACK:
            out << "SHUTDOWN_ACK ";
            break;
         case SHUTDOWN_COMPLETE:
            out << "SHUTDOWN_COMPLETE ";
            break;
         case ERRORTYPE:
            out << "ERROR";
            break;
      }
   }

   if (verbose)
   {
      out << endl;
      for (uint32 i=0; i<numberOfChunks; i++)
      {
         chunk = (SCTPChunk*)sctpmsg->getChunks(i);
         type   = chunk->getChunkType();

         sprintf(buf,  "     %3u: ", i + 1);
         out << buf;
         switch (type)
         {
            case INIT:
            {
               SCTPInitChunk* initChunk;
               initChunk = check_and_cast<SCTPInitChunk *>(chunk);
               out << "INIT[InitiateTag=";
               out << initChunk->getInitTag();
               out << "; a_rwnd=";
               out << initChunk->getA_rwnd();
               out << "; OS=";
               out << initChunk->getNoOutStreams();
               out << "; IS=";
               out << initChunk->getNoInStreams();
               out << "; InitialTSN=";
               out << initChunk->getInitTSN();
               if (initChunk->getAddressesArraySize() > 0)
               {
                  out <<"; Addresses=";
                  for (uint32 i = 0; i < initChunk->getAddressesArraySize(); i++)
                  {
                     if (i > 0)
                        out << ",";
                     if (initChunk->getAddresses(i).isIPv6())
                        out << initChunk->getAddresses(i).str();
                     else
                        out << initChunk->getAddresses(i);
                  }
               }
               if (initChunk->getHmacTypesArraySize() != 0)
               {
                  out <<"; HMAC_Types=";
                  for (uint32 i=0; i<initChunk->getHmacTypesArraySize(); i++)
                  {
                     if (i>0)
                        out << ", ";
                     out << initChunk->getHmacTypes(i);
                  }
                  out <<"; Random:";
                  for (uint32 k=0; k<initChunk->getRandomArraySize(); k++)
                  {
                      sprintf(buf,   "%02x", initChunk->getRandom(k));
                      out << buf;
                  }
                  out <<"; Chunks:";
                  for (uint32 j=0; j<initChunk->getChunkTypesArraySize();j++)
                  {
                     if (j>0)
                        out << ", ";
                     out << SCTP::intToChunk(initChunk->getChunkTypes(j));
                  }
               }
               if (initChunk->getSepChunksArraySize() > 0)
               {
                  out <<"; Supported Extensions=";
                  for (uint32 l = 0; l < initChunk->getSepChunksArraySize(); l++)
                  {
                     if (l > 0)
                        out << ",";
                     out << SCTP::intToChunk(initChunk->getSepChunks(l));
                  }
               }

               out <<"]";
               break;
            }
            case INIT_ACK:
            {
               SCTPInitAckChunk* initackChunk;
               initackChunk = check_and_cast<SCTPInitAckChunk *>(chunk);
               out << "INIT_ACK[InitiateTag=";
               out << initackChunk->getInitTag();
               out << "; a_rwnd=";
               out << initackChunk->getA_rwnd();
               out << "; OS=";
               out << initackChunk->getNoOutStreams();
               out << "; IS=";
               out << initackChunk->getNoInStreams();
               out << "; InitialTSN=";
               out << initackChunk->getInitTSN();
               out << "; CookieLength=";
               out << initackChunk->getCookieArraySize();
               if (initackChunk->getAddressesArraySize() > 0)
               {
                  out <<"; Addresses=";
                  for (uint32 i = 0; i < initackChunk->getAddressesArraySize(); i++)
                  {
                     if (i > 0)
                        out << ",";
                     out << initackChunk->getAddresses(i);
                  }
               }
               if (initackChunk->getHmacTypesArraySize() != 0)
               {
                  out <<"; HMAC_Types=";
                  for (uint32 i=0; i<initackChunk->getHmacTypesArraySize(); i++)
                  {
                     if (i>0)
                        out << ", ";
                     out << initackChunk->getHmacTypes(i);
                  }
                  out <<"; Random:";
                  for (uint32 k=0; k<initackChunk->getRandomArraySize(); k++)
                  {
                     sprintf(buf,  "%02x", initackChunk->getRandom(k));
                     out << buf;
                     //out << initackChunk->getRandom(k);
                  }
                  out <<"; Chunks:";
                  for (uint32 j=0; j<initackChunk->getChunkTypesArraySize();j++)
                  {
                     if (j>0)
                        out << ", ";
                     out << SCTP::intToChunk(initackChunk->getChunkTypes(j));
                  }
               }
               if (initackChunk->getSepChunksArraySize() > 0)
               {
                  out <<"; Supported Extensions=";
                  for (uint32 l = 0; l < initackChunk->getSepChunksArraySize(); l++)
                  {
                     if (l > 0)
                        out << ",";
                     out << SCTP::intToChunk(initackChunk->getSepChunks(l));
                  }
               }
               out <<"]";
               break;
            }
            case COOKIE_ECHO:
               out << "COOKIE_ECHO[CookieLength=";
               out <<  chunk->getBitLength()/8 - 4;
               out <<"]";
               break;
            case COOKIE_ACK:
               out << "COOKIE_ACK ";
               break;
            case DATA:
            {
               SCTPDataChunk* dataChunk;
               dataChunk = check_and_cast<SCTPDataChunk *>(chunk);
               out << "DATA[TSN=";
               out << dataChunk->getTsn();
               out << "; SID=";
               out << dataChunk->getSid();
               out << "; SSN=";
               out << dataChunk->getSsn();
               out << "; PPID=";
               out << dataChunk->getPpid();
               out << "; PayloadLength=";
               out << dataChunk->getBitLength()/8 - 16;
               out << "; Flags=";
               if (dataChunk->getBBit())
                  out << "B";
               if (dataChunk->getEBit())
                  out << "E";
               if (dataChunk->getUBit())
                  out << "U";
               if (dataChunk->getIBit())
                  out << "I";
               out <<"]";
               break;
            }
            case SACK:
            {
               SCTPSackChunk* sackChunk;
               sackChunk = check_and_cast<SCTPSackChunk *>(chunk);
               out << "SACK[CumTSNAck=";
               out << sackChunk->getCumTsnAck();
               out << "; a_rwnd=";
               out << sackChunk->getA_rwnd();
               if (sackChunk->getGapStartArraySize() > 0)
               {
                  out <<"; Gaps=";
                  for (uint32 i = 0; i < sackChunk->getGapStartArraySize(); i++)
                  {
                     if (i > 0)
                        out << ", ";
                     out << sackChunk->getGapStart(i) << "-" << sackChunk->getGapStop(i);
                  }
               }
               if (sackChunk->getDupTsnsArraySize() > 0)
               {
                  out <<"; Dups=";
                  for (uint32 i = 0; i < sackChunk->getDupTsnsArraySize(); i++)
                  {
                     if (i > 0)
                        out << ", ";
                     out << sackChunk->getDupTsns(i);
                  }
               }
               out <<"]";
               break;
            }
            case HEARTBEAT:
               SCTPHeartbeatChunk* heartbeatChunk;
               heartbeatChunk = check_and_cast<SCTPHeartbeatChunk *>(chunk);
               out << "HEARTBEAT[InfoLength=";
               out <<  chunk->getBitLength()/8 - 4;
               out << "; time=";
               out << heartbeatChunk->getTimeField();
               out <<"]";
               break;
            case HEARTBEAT_ACK:
               out << "HEARTBEAT_ACK[InfoLength=";
               out <<  chunk->getBitLength()/8 - 4;
               out <<"]";
               break;
            case ABORT:
               SCTPAbortChunk* abortChunk;
               abortChunk = check_and_cast<SCTPAbortChunk *>(chunk);
               out << "ABORT[T-Bit=";
               out << abortChunk->getT_Bit();
               out << "]";
               break;
            case SHUTDOWN:
               SCTPShutdownChunk* shutdown;
               shutdown = check_and_cast<SCTPShutdownChunk *>(chunk);
               out << "SHUTDOWN[CumTSNAck=";
               out << shutdown->getCumTsnAck();
               out << "]";
               break;
            case SHUTDOWN_ACK:
               out << "SHUTDOWN_ACK ";
               break;
            case SHUTDOWN_COMPLETE:
               out << "SHUTDOWN_COMPLETE ";
               break;
            case ASCONF:
            {
               SCTPAsconfChunk* asconfChunk;
               asconfChunk = check_and_cast<SCTPAsconfChunk *>(chunk);
               out << "ASCONF[SerialNumber=";
               out << asconfChunk->getSerialNumber();
               out << "; Address=";
               out << asconfChunk->getAddressParam();
               if (asconfChunk->getAsconfParamsArraySize()>0)
                  out << ";";
               for (uint32 i=0; i<asconfChunk->getAsconfParamsArraySize(); i++)
               {
                  out << "\n           Parameter "<<i+1<<": ";
                  sctpParam = (SCTPParameter*)(asconfChunk->getAsconfParams(i));
                  switch (sctpParam->getParameterType())
                  {
                     case ADD_IP_ADDRESS:
                        SCTPAddIPParameter* ipParam;
                        ipParam = check_and_cast<SCTPAddIPParameter*>(sctpParam);
                        out << "ADD_IP_PARAMETER: Address=" << ipParam->getAddressParam();
                        out << "; CorrelationId=" << ipParam->getRequestCorrelationId();
                        break;
                     case DELETE_IP_ADDRESS:
                        SCTPDeleteIPParameter* delParam;
                        delParam = check_and_cast<SCTPDeleteIPParameter*>(sctpParam);
                        out << "DELETE_IP_PARAMETER: Address=" << delParam->getAddressParam();
                        out << "; CorrelationId=" << delParam->getRequestCorrelationId();
                        break;
                     case SET_PRIMARY_ADDRESS:
                        SCTPSetPrimaryIPParameter* priParam;
                        priParam = check_and_cast<SCTPSetPrimaryIPParameter*>(sctpParam);
                        out << "SET_PRIMARY_ADDRESS: Address=" << priParam->getAddressParam();
                        out << "; CorrelationId=" << priParam->getRequestCorrelationId();
                        break;
                  }
               }
               out << "]";
               break;
            }
            case ASCONF_ACK:
            {
               SCTPAsconfAckChunk* asconfAckChunk;
               asconfAckChunk = check_and_cast<SCTPAsconfAckChunk *>(chunk);
               out << "ASCONF_ACK[SerialNumber=";
               out << asconfAckChunk->getSerialNumber();
               if (asconfAckChunk->getAsconfResponseArraySize()>0)
                  out << ";";
               for (uint32 j=0; j<asconfAckChunk->getAsconfResponseArraySize(); j++)
               {
                  out << "\n           Parameter "<<j+1<<": ";
                  sctpParam = (SCTPParameter*)(asconfAckChunk->getAsconfResponse(j));
                  sctpEV3<<"ParameterType="<<sctpParam->getParameterType()<<"\n";
                  switch (sctpParam->getParameterType())
                  {
                     case SUCCESS_INDICATION:
                        SCTPSuccessIndication* success;
                        success = check_and_cast<SCTPSuccessIndication*>(sctpParam);
                        out << "SUCCESS_INDICATION: CorrelationId=" << success->getResponseCorrelationId();
                        break;
                     case ERROR_CAUSE_INDICATION:
                        SCTPErrorCauseParameter* error;
                        error = check_and_cast<SCTPErrorCauseParameter*>(sctpParam);
                        out << "ERROR_CAUSE_INDICATION: CorrelationId=" << error->getResponseCorrelationId();
                        out << "; ErrorType=" << error->getErrorCauseType();
                        break;

                  }
               }
               out << "]";
               break;
            }
            case AUTH:
            {
               SCTPAuthenticationChunk* authChunk;
               authChunk = check_and_cast<SCTPAuthenticationChunk*>(chunk);
               out << "AUTH[SharedKey=" << authChunk->getSharedKey();
               out << " HMACIdentifier=" << authChunk->getHMacIdentifier();
               out << " HMACCorrect=" << authChunk->getHMacOk();
               out << " HMAC=";
               for (uint32 i=0; i<authChunk->getHMACArraySize(); i++)
                  out << authChunk->getHMAC(i);
               out << "]";
               break;
            }
            case FORWARD_TSN:
            {
               SCTPForwardTsnChunk* fwChunk;
               fwChunk = check_and_cast<SCTPForwardTsnChunk *>(chunk);
               out << "FORWARD_TSN[NewCumTSN=";
               out << fwChunk->getNewCumTsn();
               if (fwChunk->getSidArraySize() > 0)
               {
                  out <<"; Streams=";
                  for (uint32 i = 0; i < fwChunk->getSidArraySize(); i++)
                  {
                     if (i > 0)
                        out << ", ";
                     out << fwChunk->getSid(i) << ":" << fwChunk->getSsn(i);
                  }
               }
               out << "]";
               break;
            }
            case STREAM_RESET:
            {
               SCTPStreamResetChunk* resetChunk;
               SCTPParameter* param;
               resetChunk = check_and_cast<SCTPStreamResetChunk*>(chunk);
               uint32 numberOfParameters = resetChunk->getParametersArraySize();
               uint32 parameterType;
               for (uint32 i=0; i<numberOfParameters; i++)
               {
                  param = (SCTPParameter*)resetChunk->getParameters(i);
                  parameterType   = param->getParameterType();
                  switch (parameterType)
                  {
                     case OUTGOING_RESET_REQUEST_PARAMETER:
                        out << "OUTGOING_RESET_REQUEST_PARAMETER[RequestSeqN=";
                        SCTPOutgoingSSNResetRequestParameter* outp;
                        outp = check_and_cast<SCTPOutgoingSSNResetRequestParameter*>(param);
                        out << outp->getSrReqSn();
                        out << " ResponseSeqN="<<outp->getSrResSn();
                        out << " LastAssignedTSN="<<outp->getLastTsn()<<"]";
                        break;
                     case INCOMING_RESET_REQUEST_PARAMETER:
                        out << "INCOMING_RESET_REQUEST_PARAMETER[RequestSeqN=";
                        SCTPIncomingSSNResetRequestParameter* inp;
                        inp = check_and_cast<SCTPIncomingSSNResetRequestParameter*>(param);
                        out << inp->getSrReqSn()<<"]";
                        break;
                     case SSN_TSN_RESET_REQUEST_PARAMETER:
                        out << "SSN_TSN_RESET_REQUEST_PARAMETER[RequestSeqN=";
                        SCTPSSNTSNResetRequestParameter* stp;
                        stp = check_and_cast<SCTPSSNTSNResetRequestParameter*>(param);
                        out << stp->getSrReqSn()<<"]";
                        break;
                     case STREAM_RESET_RESPONSE_PARAMETER:
                        out << "STREAM_RESET_RESPONSE_PARAMETER[ResponseSeqN=";
                        SCTPStreamResetResponseParameter* resp;
                        resp = check_and_cast<SCTPStreamResetResponseParameter*>(param);
                        out << resp->getSrResSn();
                        out << " Result="<<resp->getResult();
                        if (resp->getSendersNextTsn() != 0)
                        {
                           out << " SendersNextTsn = "<<resp->getSendersNextTsn();
                           out << " ReceiversNextTsn = "<<resp->getReceiversNextTsn();
                        }
                        out << "]";
                        break;
                  }
               }
               break;
            }
            case PKTDROP:
            {
               SCTPPacketDropChunk* packetDrop;
               packetDrop = check_and_cast<SCTPPacketDropChunk*>(chunk);
               out << "[MaxRwnd="<<packetDrop->getMaxRwnd()<<";";
               out << " CTBM="<<packetDrop->getCFlag()<<""<<packetDrop->getTFlag()<< "" <<packetDrop->getBFlag()<<""<<packetDrop->getMFlag()<<";";
               out << " queuedData="<<packetDrop->getQueuedData()<<";";
               out << " TruncLength="<<packetDrop->getTruncLength()<<"]";
               if (dynamic_cast<SCTPMessage *>(packetDrop->getEncapsulatedPacket()))
               {
                  out << "\n\nDropped Packet:\n\n";
                  SCTPMessage *encmsg=(SCTPMessage*)(packetDrop->getEncapsulatedPacket());
                  bool dir = encmsg->arrivedOn("in1");
                  sctpDump("", (SCTPMessage *)encmsg, std::string(dir?"A":"B"),std::string(dir?"B":"A"));
               }
               break;
            }
            case ERRORTYPE:
            {
               SCTPErrorChunk* errorChunk;
               errorChunk = check_and_cast<SCTPErrorChunk*>(chunk);
               uint32 numberOfParameters = errorChunk->getParametersArraySize();
               uint32 parameterType;
               for (uint32 i=0; i<numberOfParameters; i++)
               {
                  SCTPParameter* param = (SCTPParameter*)errorChunk->getParameters(i);
                  parameterType   = param->getParameterType();
                  switch (parameterType)
                  {
                     case UNSUPPORTED_HMAC:
                     {
                        out << "UNSUPPORTED_HMAC:";
                        SCTPSimpleErrorCauseParameter* err = check_and_cast<SCTPSimpleErrorCauseParameter*>(param);
                        out << err->getValue();
                        break;
                     }
                     case MISSING_NAT_ENTRY:
                     {
                        out << "Missing NAT table entry";
                        break;
                     }
                  }
               }

               break;
            }
         }
         out << endl;
      }
   }
   // comment
   if (comment)
      out << "# " << comment;
#endif
   out << endl;
}



void PacketDump::dump(const char *label, const char *msg)
{
    std::ostream& out = *outp;

    // seq and time (not part of the tcpdump format)
    char buf[30];

    sprintf(buf, "[%.3f%s] ", simulation.getSimTime().dbl(), label);
    out << buf << msg << endl;
}

void PacketDump::dumpPacket(bool l2r, cPacket *msg)
{
    std::ostream& out = *outp;

#ifdef WITH_IPv4
    if (dynamic_cast<IPv4Datagram *>(msg))
    {
        dumpIPv4(l2r, "", (IPv4Datagram *)msg, "");
    }
    else
#endif
#ifdef WITH_SCTP
    if (dynamic_cast<SCTPMessage *>(msg))
    {
        sctpDump("", (SCTPMessage *)msg, std::string(l2r ? "A" : "B"), std::string(l2r ? "B" : "A"));
    }
    else
#endif
#ifdef WITH_TCP_COMMON
    if (dynamic_cast<TCPSegment *>(msg))
    {
        tcpDump(l2r, "", (TCPSegment *)msg, std::string(l2r ? "A" : "B"), std::string(l2r ? "B" : "A"));
    }
    else
#endif
#ifdef WITH_IPv4
    if (dynamic_cast<ICMPMessage *>(msg))
    {
        out << "ICMPMessage " << msg->getName() << (msg->hasBitError() ? " (BitError)" : "") << endl;
    }
    else
#endif
    {
        // search for encapsulated IPv4[v6]Datagram in it
        while (msg)
        {
#ifdef WITH_IPv4
            if (dynamic_cast<IPv4Datagram *>(msg))
            {
                dumpIPv4(l2r, "", (IPv4Datagram *)msg);
                break;
            }
#endif
#ifdef WITH_IPv6
            if (dynamic_cast<IPv6Datagram *>(msg))
            {
                dumpIPv6(l2r, "", (IPv6Datagram *)msg);
                break;
            }
#endif
            out << "Packet " << msg->getClassName() << " '" << msg->getName() << "'"
                 << (msg->hasBitError() ? " (BitError)" : "")<< ": ";
            msg = msg->getEncapsulatedPacket();
        }

        if (!msg)
        {
            //We do not want this to end in an error if EtherAutoconf messages
            //are passed, so just print a warning. -WEI
            out << "CANNOT DECODE, packet doesn't contain either IPv4 or IPv6 Datagram\n";
        }
    }
}

void PacketDump::udpDump(bool l2r, const char *label, UDPPacket* udppkt,
        const std::string& srcAddr, const std::string& destAddr, const char *comment)
{
    std::ostream& out = *outp;

    char buf[30];
    sprintf(buf, "[%.3f%s] ", simulation.getSimTime().dbl(), label);
    out << buf;

#ifndef WITH_UDP
    if (l2r)
        out << "[UDP] "<< srcAddr << " > " << destAddr << ": ";
    else
        out << "[UDP] "<< destAddr << " < " << srcAddr << ": ";
#else
    // seq and time (not part of the tcpdump format)
    // src/dest
    if (l2r)
    {
        out << srcAddr << "." << udppkt->getSourcePort() << " > ";
        out << destAddr << "." << udppkt->getDestinationPort() << ": ";
    }
    else
    {
        out << destAddr << "." << udppkt->getDestinationPort() << " < ";
        out << srcAddr << "." << udppkt->getSourcePort() << ": ";
    }

    //out << endl;
    out << "UDP: Payload length=" << udppkt->getByteLength() - 8 << endl;

#ifdef WITH_SCTP
    if (udppkt->getSourcePort() == 9899 || udppkt->getDestinationPort() == 9899)
    {
        if (dynamic_cast<SCTPMessage *>(udppkt->getEncapsulatedPacket()))
            sctpDump("", (SCTPMessage *)(udppkt->getEncapsulatedPacket()),
                    std::string(l2r?"A":"B"), std::string(l2r?"B" : "A"));
    }
#endif
#endif

    // comment
    if (comment)
        out << "# " << comment;

    out << endl;
}

void PacketDump::dumpIPv4(bool l2r, const char *label, IPv4Datagram *dgram, const char *comment)
{
     std::ostream& out = *outp;
     char buf[30];

#ifdef WITH_IPv4
    cPacket *encapmsg = dgram->getEncapsulatedPacket();

#ifdef WITH_TCP_COMMON
    if (dynamic_cast<TCPSegment *>(encapmsg))
    {
         // if TCP, dump as TCP
         tcpDump(l2r, label, (TCPSegment *)encapmsg, dgram->getSrcAddress().str(),
                 dgram->getDestAddress().str(), comment);
    }
    else
#endif
#ifdef WITH_UDP
    if (dynamic_cast<UDPPacket *>(encapmsg))
    {
        udpDump(l2r, label, (UDPPacket *)encapmsg, dgram->getSrcAddress().str(),
                dgram->getDestAddress().str(), comment);
    }
    else
#endif
#ifdef WITH_SCTP
    if (dynamic_cast<SCTPMessage *>(dgram->getEncapsulatedPacket()))
    {
        SCTPMessage *sctpmsg = check_and_cast<SCTPMessage *>(dgram->getEncapsulatedPacket());
        if (dgram->hasBitError())
            sctpmsg->setBitError(true);
        sctpDump(label, sctpmsg, dgram->getSrcAddress().str(), dgram->getDestAddress().str(), comment);
    }
    else
#endif
    {
         // some other packet, dump what we can
         // seq and time (not part of the tcpdump format)
         sprintf(buf, "[%.3f%s] ", SIMTIME_DBL(simTime()), label);
         out << buf;

         // packet class and name
         out << "? " << encapmsg->getClassName() << " \"" << encapmsg->getName() << "\"";

         // comment
         if (comment)
             out << " # " << comment;

         out << endl;
    }
#else
    sprintf(buf, "[%.3f%s] ", SIMTIME_DBL(simTime()), label);
    out << buf << "[IPv4]";

    // comment
    if (comment)
        out << " # " << comment;

    out << endl;
#endif
}

void PacketDump::dumpIPv6(bool l2r, const char *label, IPv6Datagram *dgram, const char *comment)
{
     std::ostream& out = *outp;
     char buf[30];

#ifdef WITH_IPv6
    cPacket *encapmsg = dgram->getEncapsulatedPacket();

#ifdef WITH_TCP_COMMON
    if (dynamic_cast<TCPSegment *>(encapmsg))
    {
         // if TCP, dump as TCP
         tcpDump(l2r, label, (TCPSegment *)encapmsg, dgram->getSrcAddress().str(),
                 dgram->getDestAddress().str(), comment);
    }
    else
#endif
    {
         // some other packet, dump what we can
         // seq and time (not part of the tcpdump format)
         sprintf(buf, "[%.3f%s] ", SIMTIME_DBL(simTime()), label);
         out << buf;

         // packet class and name
         out << "? " << encapmsg->getClassName() << " \"" << encapmsg->getName() << "\"\n";

         // comment
         if (comment)
             out << "# " << comment;

         out << endl;
    }
#else
    sprintf(buf, "[%.3f%s] ", SIMTIME_DBL(simTime()), label);
    out << buf << "[IPv4]";

    // comment
    if (comment)
        out << " # " << comment;

    out << endl;
#endif
}

void PacketDump::tcpDump(bool l2r, const char *label, TCPSegment *tcpseg,
        const std::string& srcAddr, const std::string& destAddr, const char *comment)
{
     std::ostream& out = *outp;

    // seq and time (not part of the tcpdump format)
    char buf[30];
    sprintf(buf, "[%.3f%s] ", SIMTIME_DBL(simTime()), label);
    out << buf;

#ifndef WITH_TCP_COMMON
    if (l2r)
        out << srcAddr << " > " << destAddr << ": ";
    else
        out << destAddr << " < " << srcAddr << ": ";
#else
    // src/dest ports
    if (l2r)
    {
        out << srcAddr << "." << tcpseg->getSrcPort() << " > ";
        out << destAddr << "." << tcpseg->getDestPort() << ": ";
    }
    else
    {
        out << destAddr << "." << tcpseg->getDestPort() << " < ";
        out << srcAddr << "." << tcpseg->getSrcPort() << ": ";
    }

    // flags
    bool flags = false;
    if (tcpseg->getUrgBit()) {flags = true; out << "U ";}
    if (tcpseg->getAckBit()) {flags = true; out << "A ";}
    if (tcpseg->getPshBit()) {flags = true; out << "P ";}
    if (tcpseg->getRstBit()) {flags = true; out << "R ";}
    if (tcpseg->getSynBit()) {flags = true; out << "S ";}
    if (tcpseg->getFinBit()) {flags = true; out << "F ";}
    if (!flags) {out << ". ";}

    // data-seqno
    if (tcpseg->getPayloadLength()>0 || tcpseg->getSynBit())
    {
        out << tcpseg->getSequenceNo() << ":" << tcpseg->getSequenceNo()+tcpseg->getPayloadLength();
        out << "(" << tcpseg->getPayloadLength() << ") ";
    }

    // ack
    if (tcpseg->getAckBit())
        out << "ack " << tcpseg->getAckNo() << " ";

    // window
    out << "win " << tcpseg->getWindow() << " ";

    // urgent
    if (tcpseg->getUrgBit())
        out << "urg " << tcpseg->getUrgentPointer() << " ";

    // options present?
    if (tcpseg->getHeaderLength() > 20)
    {
        std::string direction = "sent";

        if (l2r) // change direction
            {direction = "received";}

        unsigned short numOptions = tcpseg->getOptionsArraySize();
        out << "\nTCP Header Option(s) " << direction << ":\n";

        for (int i=0; i<numOptions; i++)
        {
            TCPOption option = tcpseg->getOptions(i);
            unsigned short kind = option.getKind();
            unsigned short length = option.getLength();
            out << (i+1) << ". option kind=" << kind << " length=" << length << "\n";
        }
    }
#endif

    // comment
    if (comment)
        out << "# " << comment;

    out << endl;
}

