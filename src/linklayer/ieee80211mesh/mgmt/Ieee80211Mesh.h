//
// Copyright (C) 2008 Alfonso Ariza
// Copyright (C) 2009 Alfonso Ariza
// Copyright (C) 2009 Alfonso Ariza
// Copyright (C) 2010 Alfonso Ariza
// Copyright (C) 2011 Alfonso Ariza
// Copyright (C) 2012 Alfonso Ariza
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

#ifndef IEEE80211_MESH_ADHOC_H
#define IEEE80211_MESH_ADHOC_H
#include <deque>
#include "INETDefs.h"
#include "Ieee80211MgmtBase.h"
#include "NotificationBoard.h"
#include "IInterfaceTable.h"
#include "lwmpls_data.h"
#include "LWMPLSPacket_m.h"
#include "ManetAddress.h"
#include "ManetRoutingBase.h"
#include "Ieee80211Etx.h"
#include "WirelessNumHops.h"
#include "Ieee80211Mac.h"
#include "Radio.h"

/**
 * Used in 802.11 ligh wireless mpls  mode. See corresponding NED file for a detailed description.
 * This implementation ignores many details.
 *
 * @author Alfonso Ariza
 */

#define CHEAT_IEEE80211MESH

class INET_API Ieee80211Mesh : public Ieee80211MgmtBase
{
    public:
        enum SelectionCriteria
        {
            ETX = 1 , MINQUEUE, LASTUSED,MINQUEUELASTUSED, LASTUSEDMINQUEUE
        };
    private:

        WirelessNumHops *getOtpimunRoute;

        static simsignal_t numHopsSignal;
        static simsignal_t numFixHopsSignal;

        static const int MaxSeqNum;
        class SeqNumberData
        {
            uint64_t seqNum;
            int numTimes;
            public:
                SeqNumberData() {seqNum = 0; numTimes = 0;}
                SeqNumberData(uint64_t s, int t ) {seqNum = s; numTimes = t;}
                uint64_t getSeqNum() const {return seqNum;}
                int getNumTimes() const {return numTimes;}
                void setSeqNum(uint64_t s) {seqNum = s;}
                void  setNumTimes(int t) {numTimes = t;}
                inline bool operator<(const SeqNumberData& b) const
                {
                    return seqNum < b.seqNum;
                }

                inline bool operator>(const SeqNumberData& b) const
                {
                    return seqNum > b.seqNum;
                }
                inline bool operator==(const SeqNumberData& b) const
                {
                    return seqNum == b.seqNum;
                }
        };

        typedef std::map<MACAddress, simtime_t> LastTimeReception;
        std::vector<LastTimeReception> timeReceptionInterface;

        std::vector<Ieee80211Mac*> macInterfaces;
        std::vector<Radio*> radioInterfaces;
        typedef std::deque<SeqNumberData> SeqNumberVector;
        typedef std::map<ManetAddress, SeqNumberVector> SeqNumberInfo;
        SeqNumberInfo seqNumberInfo;

        uint64_t numRoutingBytes;
        //
        // Multi mac interfaces
        //
        unsigned int numMac;
        SelectionCriteria selectionCriteria;
        bool inteligentBroadcastRouting;

        cMessage *WMPLSCHECKMAC;
        cMessage *gateWayTimeOut;

        double limitDelay;
        NotificationBoard *nb;
        bool proactiveFeedback;
        int maxHopProactiveFeedback; // Maximun number of hops for to use the proactive feedback
        int maxHopProactive; // Maximun number of hops in the fix part of the network with the proactive feedback
        int maxHopReactive; // Maximun number of hops by the reactive part for to use the proactive feedback

        bool floodingConfirmation;

        struct ConfirmationInfo
        {
                Ieee80211MeshFrame* frame;
                int reintent;
        };
        std::vector<ConfirmationInfo> confirmationFrames;

        ManetRoutingBase *routingModuleProactive;
        ManetRoutingBase *routingModuleReactive;
        ManetRoutingBase *routingModuleHwmp;
        Ieee80211Etx * ETXProcess;

        IInterfaceTable *ift;
        bool useLwmpls;
        int maxTTL;

        LWMPLSDataStructure * mplsData;

        double multipler_active_break;
        simtime_t timer_active_refresh;
        bool activeMacBreak;
        int macBaseGateId; // id of the nicOut[0] gate  // FIXME macBaseGateId is unused, what is it?

        // start routing proccess
        virtual void startReactive();
        virtual void startProactive();
        virtual void startHwmp();
        virtual void startEtx();
        virtual void startGateWay();

// LWMPLS methods
        cPacket * decapsulateMpls(LWMPLSPacket *frame);
        Ieee80211DataFrame *encapsulate(cPacket *msg, MACAddress dest);
        virtual void mplsSendAck(int label);
        virtual void mplsCreateNewPath(int label, LWMPLSPacket *mpls_pk_ptr, MACAddress sta_addr);
        virtual void mplsBreakPath(int label, LWMPLSPacket *mpls_pk_ptr, MACAddress sta_addr);
        virtual void mplsNotFoundPath(int label, LWMPLSPacket *mpls_pk_ptr, MACAddress sta_addr);
        virtual void mplsForwardData(int label, LWMPLSPacket *mpls_pk_ptr, MACAddress sta_addr,
                LWmpls_Forwarding_Structure *forwarding_data);
        virtual void mplsBasicSend(LWMPLSPacket *mpls_pk_ptr, MACAddress sta_addr);
        virtual void mplsAckPath(int label, LWMPLSPacket *mpls_pk_ptr, MACAddress sta_addr);
        virtual void mplsDataProcess(LWMPLSPacket * mpls_pk_ptr, MACAddress sta_addr);
        virtual void mplsBreakMacLink(MACAddress mac_id);
        void mplsCheckRouteTime();
        virtual void mplsInitializeCheckMac();
        virtual void mplsPurge(LWmpls_Forwarding_Structure *forwarding_ptr, bool purge_break);
        virtual bool mplsIsBroadcastProcessed(const MACAddress &, const uint32 &);
        virtual bool mplsForwardBroadcast(const MACAddress &);
        virtual bool forwardMessage(Ieee80211DataFrame *);
        virtual bool macLabelBasedSend(Ieee80211DataFrame *);
        virtual void actualizeReactive(cPacket *pkt, bool out);
        virtual bool isSendToGateway(Ieee80211DataOrMgmtFrame *frame);
        virtual int getBestInterface(Ieee80211DataOrMgmtFrame *frame);

        //////////////////////////////////////////
        // Gateway structures
        /////////////////////////////////////////////////
        bool isGateWay;
        typedef std::map<uint64_t, simtime_t> AssociatedAddress;
        AssociatedAddress associatedAddress;
        struct GateWayData
        {
                MACAddress idAddress;
                MACAddress ethAddress;
                ManetRoutingBase *proactive;
                ManetRoutingBase *reactive;
                AssociatedAddress *associatedAddress;
        };
        typedef std::map<ManetAddress, GateWayData> GateWayDataMap;
#ifdef CHEAT_IEEE80211MESH
        // cheat, we suppose that the information between gateway is interchanged with the wired
        static GateWayDataMap *gateWayDataMap;
#else
        GateWayDataMap *gateWayDataMap;
#endif
        int gateWayIndex;

        ///////////////////////
        // gateWay methods
        ///////////////////////
        void publishGateWayIdentity();
        void processControlPacket(LWMPLSControl *);
        virtual GateWayDataMap * getGateWayDataMap()
        {
            if (isGateWay)
                return gateWayDataMap;
            return NULL;
        }
        virtual bool selectGateWay(const ManetAddress &, MACAddress &);

        bool hasLocator;
        bool hasRelayUnit;
    protected:
        virtual void initializeBase(int stage);

    public:
        Ieee80211Mesh();
        virtual ~Ieee80211Mesh();
        bool getCostNode(const MACAddress &, unsigned int &);
    protected:
        // methos for efficient distribution of packets
        bool setSeqNum(const ManetAddress &addr, const uint64_t &sqnum, const int &numTimes);
        int findSeqNum(const ManetAddress &addr, const uint64_t &sqnum);
        int getNumVisit(const std::vector<ManetAddress> &path);
        int getNumVisit(const ManetAddress &addr, const std::vector<ManetAddress> &path);
        bool getNextInPath(const ManetAddress &addr, const std::vector<ManetAddress> &path, std::vector<ManetAddress> &next);
        bool getNextInPath(const std::vector<ManetAddress> &path, std::vector<ManetAddress> &next);
        void processDistributionPacket(Ieee80211MeshFrame *frame);

    protected:
        virtual void finish();
        virtual int numInitStages() const
        {
            return 6;
        }
        virtual void initialize(int);

        virtual void handleMessage(cMessage*);

        /** Implements abstract to use ETX packets */
        virtual void handleEtxMessage(cPacket*);

        /** Implements abstract to use routing protocols in the mac layer */
        virtual void handleRoutingMessage(cPacket*);

        /** Implements abstract to use inter gateway communication */
        virtual void handleWateGayDataReceive(cPacket *);

        /** Implements the redirection of a data packets from a gateway to other */
        virtual void handleReroutingGateway(Ieee80211DataFrame *);

        /** Implements abstract Ieee80211MgmtBase method */
        virtual void handleTimer(cMessage *msg);

        /** Implements abstract Ieee80211MgmtBase method */
        virtual void handleUpperMessage(cPacket *msg);

        /** Implements abstract Ieee80211MgmtBase method -- throws an error (no commands supported) */
        virtual void handleCommand(int msgkind, cPolymorphic *ctrl);

        /** Utility function for handleUpperMessage() */
        virtual Ieee80211DataFrame *encapsulate(cPacket *msg);

        /** Called by the NotificationBoard whenever a change occurs we're interested in */
        virtual void receiveChangeNotification(int category, const cPolymorphic *details);

        /** @name Processing of different frame types */
        //@{
        virtual void handleDataFrame(Ieee80211DataFrame *frame);
        virtual void handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame);
        virtual void handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame);
        virtual void handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame);
        virtual void handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame);
        virtual void handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame);
        virtual void handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame);
        virtual void handleDisassociationFrame(Ieee80211DisassociationFrame *frame);
        virtual void handleBeaconFrame(Ieee80211BeaconFrame *frame);
        virtual void handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame);
        virtual void handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame);
        //@}
        /** Redefined from Ieee80211MgmtBase: send message to MAC */
        virtual void sendOut(cMessage *msg);
        /** Redefined from Ieee80211MgmtBase Utility method: sends the packet to the upper layer */
        //virtual void sendUp(cMessage *msg);
        virtual cPacket * decapsulate(Ieee80211DataFrame *frame);
        virtual void sendOrEnqueue(cPacket *frame);

        virtual bool isAddressForUs(const MACAddress &add);
};

#endif

