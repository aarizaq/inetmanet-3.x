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
#include "inet/common/INETDefs.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/linklayer/ieee80211mesh/mgmt/lwmpls_data.h"
#include "inet/linklayer/ieee80211mesh/mgmt/LWMPLSPacket_m.h"
#include "inet/networklayer/common/L3Address.h"
#include "inet/routing/extras/base/ManetRoutingBase.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211Etx.h"
#include "inet/common/WirelessNumHops.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Mac.h"
//#include "Radio.h"
#include "inet/securityModule/message/securityPkt_m.h"

namespace inet{

namespace ieee80211 {

using namespace physicallayer;

using namespace inetmanet;

/**
 * Used in 802.11 ligh wireless mpls  mode. See corresponding NED file for a detailed description.
 * This implementation ignores many details.
 *
 * @author Alfonso Ariza
 */

#define CHEAT_IEEE80211MESH

class INET_API Ieee80211Mesh : public Ieee80211MgmtBase, public cListener
{
    public:
        enum SelectionCriteria
        {
            ETX = 1 , MINQUEUE, LASTUSED,MINQUEUELASTUSED, LASTUSEDMINQUEUE
        };
    private:

        WirelessNumHops *getOtpimunRoute = nullptr;

        static simsignal_t numHopsSignal;
        static simsignal_t numFixHopsSignal;

        static const int MaxSeqNum;
        class SeqNumberData
        {
            uint64_t seqNum = 0;
            int numTimes = 0;
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
        //std::vector<Radio*> radioInterfaces;
        typedef std::deque<SeqNumberData> SeqNumberVector;
        typedef std::map<L3Address, SeqNumberVector> SeqNumberInfo;
        SeqNumberInfo seqNumberInfo;

        uint64_t numRoutingBytes = 0;
        uint64_t numDataBytes = 0;
        //
        // Multi mac interfaces
        //
        unsigned int numMac;
        SelectionCriteria selectionCriteria;
        bool inteligentBroadcastRouting;

        cMessage *WMPLSCHECKMAC = nullptr;
        cMessage *gateWayTimeOut = nullptr;

        double limitDelay = 10000;
        bool proactiveFeedback = false;
        int maxHopProactiveFeedback = -1; // Maximun number of hops for to use the proactive feedback
        int maxHopProactive = -1; // Maximun number of hops in the fix part of the network with the proactive feedback
        int maxHopReactive = -1; // Maximun number of hops by the reactive part for to use the proactive feedback

        bool floodingConfirmation = false;

        bool hasSecurity = false;

        struct ConfirmationInfo
        {
                Ieee80211MeshFrame* frame = nullptr;
                int reintent = 0;
        };
        std::vector<ConfirmationInfo> confirmationFrames;

        inetmanet::ManetRoutingBase *routingModuleProactive = nullptr;
        inetmanet::ManetRoutingBase *routingModuleReactive = nullptr;
        inetmanet::ManetRoutingBase *routingModuleHwmp = nullptr;
        Ieee80211Etx * ETXProcess = nullptr;

        IInterfaceTable *ift = nullptr;
        bool useLwmpls = false;
        int maxTTL = 32;

        LWMPLSDataStructure * mplsData = nullptr;

        double multipler_active_break = 1;
        simtime_t timer_active_refresh; // 0
        bool activeMacBreak = false;
        int macBaseGateId = -1; // id of the nicOut[0] gate  // FIXME macBaseGateId is unused, what is it?

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
        bool isGateWay = false;
        typedef std::map<uint64_t, simtime_t> AssociatedAddress;
        AssociatedAddress associatedAddress;
        struct GateWayData
        {
                MACAddress idAddress;
                MACAddress ethAddress;
                ManetRoutingBase *proactive = nullptr;
                ManetRoutingBase *reactive = nullptr;
                ManetRoutingBase *hwmp = nullptr;
                AssociatedAddress *associatedAddress = nullptr;
        };
        typedef std::map<L3Address, GateWayData> GateWayDataMap;
        GateWayDataMap *localGateWayDataMap = nullptr;
#ifdef CHEAT_IEEE80211MESH
        // cheat, we suppose that the information between gateway is interchanged with the wired
        static GateWayDataMap *gateWayDataMap;
#else
        GateWayDataMap *gateWayDataMap = nullptr;
#endif
        int gateWayIndex = -1;

        ///////////////////////
        // gateWay methods
        ///////////////////////
        void publishGateWayIdentity();
        void processControlPacket(LWMPLSControl *);
        virtual GateWayDataMap * getGateWayDataMap()
        {
            if (isGateWay)
                return gateWayDataMap;
            return nullptr;
        }

        virtual GateWayDataMap * getLocalGateWayDataMap()
        {
            if (isGateWay)
                return localGateWayDataMap;
            return nullptr;
        }


        virtual bool selectGateWay(const L3Address &, MACAddress &);

        virtual void discoverGateWay();

        bool hasLocator = false;
        bool isMultiMac = false;
        bool hasRelayUnit = false;
    protected:
        virtual void initializeBase(int stage);

    public:
        Ieee80211Mesh();
        virtual ~Ieee80211Mesh();
        bool getCostNode(const MACAddress &, unsigned int &);
    protected:
        // methos for efficient distribution of packets
        bool setSeqNum(const L3Address &addr, const uint64_t &sqnum, const int &numTimes);
        int findSeqNum(const L3Address &addr, const uint64_t &sqnum);
        int getNumVisit(const std::vector<L3Address> &path);
        int getNumVisit(const L3Address &addr, const std::vector<L3Address> &path);
        bool getNextInPath(const L3Address &addr, const std::vector<L3Address> &path, std::vector<L3Address> &next);
        bool getNextInPath(const std::vector<L3Address> &path, std::vector<L3Address> &next);
        void processDistributionPacket(Ieee80211MeshFrame *frame);

    protected:
        virtual void finish() override;
        virtual int numInitStages() const override
        {
            return NUM_INIT_STAGES;
        }
        virtual void initialize(int) override;

        virtual void handleMessage(cMessage*) override;

        /** Implements abstract to use ETX packets */
        virtual void handleEtxMessage(cPacket*);

        /** Implements abstract to use routing protocols in the mac layer */
        virtual void handleRoutingMessage(cPacket*);

        /** Implements abstract to use inter gateway communication */
        virtual void handleWateGayDataReceive(cPacket *);

        /** Implements the redirection of a data packets from a gateway to other */
        virtual void handleReroutingGateway(Ieee80211DataFrame *);

        /** Implements abstract Ieee80211MgmtBase method */
        virtual void handleTimer(cMessage *msg) override;

        /** Implements abstract Ieee80211MgmtBase method */
        virtual void handleUpperMessage(cPacket *msg) override;

        /** Implements abstract Ieee80211MgmtBase method -- throws an error (no commands supported) */
        virtual void handleCommand(int msgkind, cObject *ctrl) override;

        /** Utility function for handleUpperMessage() */
        virtual Ieee80211DataFrame *encapsulate(cPacket *msg);

        /** Called by the NotificationBoard whenever a change occurs we're interested in */
        virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

        /** @name Processing of different frame types */
        //@{
        virtual void handleDataFrame(Ieee80211DataFrame *frame) override;
        virtual void handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame) override;
        virtual void handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame) override;
        virtual void handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame) override;
        virtual void handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame) override;
        virtual void handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame) override;
        virtual void handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame) override;
        virtual void handleDisassociationFrame(Ieee80211DisassociationFrame *frame) override;
        virtual void handleBeaconFrame(Ieee80211BeaconFrame *frame) override;
        virtual void handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame) override;
        virtual void handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame) override;
        virtual void handleCCMPFrame(CCMPFrame *frame);
        //@}
        /** Redefined from Ieee80211MgmtBase: send message to MAC */
        virtual void sendOut(cMessage *msg);
        /** Redefined from Ieee80211MgmtBase Utility method: sends the packet to the upper layer */
        //virtual void sendUp(cMessage *msg);
        virtual cPacket * decapsulate(Ieee80211DataFrame *frame);
        virtual void sendFrameDown(cPacket *frame);
        virtual void sendDownMulti(cPacket *frame, int i)
        {
            ASSERT(isOperational);
            send(frame, "macOutMulti",i);
        }

        virtual bool isAddressForUs(const MACAddress &add);
};

}

}

#endif

