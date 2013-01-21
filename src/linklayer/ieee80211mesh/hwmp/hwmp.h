/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008,2009 IITP RAS
 * Copyright (c) 2011 Universidad de M�laga
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Authors: Kirill Andreev <andreev@iitp.ru>
 * Author: Alfonso Ariza <aarizaq@uma.es>
 */

#ifndef HWMP_PROTOCOL_H
#define HWMP_PROTOCOL_H

#include "hwmp-rtable.h"
#include <vector>
#include <deque>
#include <map>
#include "ManetRoutingBase.h"
#include "Ieee80211MgmtFrames_m.h"

/**
 * \ingroup dot11s
 *
 * \brief Hybrid wireless mesh protocol -- a routing protocol of IEEE 802.11s draft.
 */

class PreqTimeout : public ManetTimer
{
    public:
        MACAddress dest;
        virtual void expire();
        PreqTimeout(MACAddress);
        PreqTimeout(MACAddress, ManetRoutingBase* agent);

};

class PreqTimer : public ManetTimer
{
    public:
        virtual void expire();
        PreqTimer(ManetRoutingBase* agent) :
                ManetTimer(agent)
        {
        }
        ;
};

class ProactivePreqTimer : public ManetTimer
{
    public:
        virtual void expire();
        ProactivePreqTimer(ManetRoutingBase* agent) :
                ManetTimer(agent)
        {
        }
        ;
};

class PerrTimer : public ManetTimer
{
    public:
        virtual void expire();
        PerrTimer(ManetRoutingBase* agent) :
                ManetTimer(agent)
        {
        }
        ;

};

class GannTimer : public ManetTimer
{
    public:
        virtual void expire();
        GannTimer(ManetRoutingBase* agent) :
                ManetTimer(agent)
        {
        }
        ;

};

class HwmpProtocol : public ManetRoutingBase
{
    public:
        HwmpProtocol();
        virtual ~HwmpProtocol();
        void initialize(int);
        virtual int numInitStages() const
        {
            return 5;
        }
        virtual void handleMessage(cMessage *msg);
        // Detect a transmission fault
        virtual void processLinkBreak(const cPolymorphic *details);
        virtual void processLinkBreakManagement(const cPolymorphic *details);
        virtual void packetFailedMac(Ieee80211TwoAddressFrame *frame);
        // promiscuous frame process.
        virtual void processFullPromiscuous(const cPolymorphic *details);
        ///\brief This callback is used to obtain active neighbours on a given interface
        ///\param cb is a callback, which returns a list of addresses on given interface (uint32_t)
        ///\name Proactive PREQ mechanism:
        ///\{
        virtual void setRoot();
        virtual void UnsetRoot();
        ///\}
        ///\brief Statistics:
        virtual bool isProactive()
        {
            return false;
        }
        ;
        virtual bool supportGetRoute() {return false;}
        virtual bool isOurType(cPacket *);
        virtual bool getDestAddress(cPacket *, ManetAddress &);
        virtual uint32_t getRoute(const ManetAddress &, std::vector<ManetAddress> &);
        virtual bool getNextHop(const ManetAddress &dest, ManetAddress &add, int &iface, double &cost);
        virtual bool getNextHopProactive(const ManetAddress &dest, ManetAddress &add, int &iface, double &cost);
        virtual bool getNextHopReactive(const ManetAddress &dest, ManetAddress &add, int &iface, double &cost);
        virtual void setRefreshRoute(const ManetAddress &destination, const ManetAddress & nextHop, bool isReverse);

        virtual bool getBestGan(ManetAddress &, ManetAddress &);
    private:
        simtime_t timeLimitQueue;
        friend class PreqTimeout;
        friend class PreqTimer;
        friend class ProactivePreqTimer;
        friend class PerrTimer;
        friend class GannTimer;

        bool m_isGann;
        unsigned int gannSeqNumber;
        bool propagateProactive;

        struct GannData
        {
                unsigned int numHops;
                unsigned int seqNum;
                MACAddress gannAddr;
        };
        std::vector<GannData> ganVector;


        uint32_t dot11MeshHWMPnetDiameter;
        std::vector<PREQElem> myPendingReq;

        /**
         * \brief Structure of path error: IePerr and list of receivers:
         * interfaces and MAC address
         */

        // Neighbor structure active if etx process is inactive
        struct Neighbor // neighbor
        {
                simtime_t lastTime;
                uint32_t cost;
                uint32_t lost;
        };

        std::map<MACAddress, Neighbor> neighborMap;

        bool useEtxProc;

        struct MyPerr
        {
                std::vector<HwmpFailedDestination> destinations;
                std::vector<MACAddress> receivers;
        };

        struct PathError
        {
                std::vector<HwmpFailedDestination> destinations; ///< destination list: Mac48Address and sequence number
                std::vector<std::pair<uint32_t, MACAddress> > receivers; ///< list of PathError receivers (in case of unicast PERR)
        };

        MyPerr m_myPerr;

        /// Packet waiting its routing information
        struct QueuedPacket
        {
                cPacket *pkt; ///< the packet
                MACAddress src; ///< src address
                MACAddress dst; ///< dst address
                uint16_t protocol; ///< protocol number
                simtime_t queueTime;
                uint32_t inInterface; ///< incoming device interface ID. (if packet has come from upper layers, this is Mesh point ID)
                QueuedPacket()
                {
                    pkt = NULL; ///< the packet
                    src = MACAddress::UNSPECIFIED_ADDRESS; ///< src address
                    dst = MACAddress::UNSPECIFIED_ADDRESS; ///< dst address
                    protocol = 0; ///< protocol number
                    inInterface = -1; ///< incoming
                }
        };

        ///\name Interaction with HWMP MAC plugin
        //\{
        void processRann(cMessage *msg)
        {
            delete msg;
            return;
        }
        void processPreq(cMessage *msg);
        void processPrep(cMessage *msg);
        void processPerr(cMessage *msg);
        void processGann(cMessage *msg);

        void processData(cMessage *msg);
        void receivePreq(Ieee80211ActionPREQFrame *preqFrame, MACAddress from, uint32_t interface, MACAddress fromMp,
                uint32_t metric);
        void receivePrep(Ieee80211ActionPREPFrame * prepFrame, MACAddress from, uint32_t interface, MACAddress fromMp,
                uint32_t metric);
        void receivePerr(std::vector<HwmpFailedDestination> destinations, MACAddress from, uint32_t interface,
                MACAddress fromMp);
        void sendPrep(MACAddress src, MACAddress targetAdd, MACAddress retransmitter, uint32_t initMetric,
                uint32_t originatorSn, uint32_t targetSn, uint32_t lifetime, uint32_t interface, uint8_t hops, bool proactive = false);

        void sendPreq(PREQElem preq, bool isProactive = false);
        void sendPreq(std::vector<PREQElem> preq, bool isProactive = false);
        void sendPreqProactive();
        void sendGann();
        void sendPerr(std::vector<HwmpFailedDestination> failedDestinations, std::vector<MACAddress> receivers);
        void requestDestination(MACAddress dst, uint32_t dst_seqno);
        void forwardPerr(std::vector<HwmpFailedDestination> failedDestinations, std::vector<MACAddress> receivers);
        void initiatePerr(std::vector<HwmpFailedDestination> failedDestinations, std::vector<MACAddress> receivers);

        int getInterfaceReceiver(MACAddress);

        Ieee80211ActionPREQFrame*
        createPReq(PREQElem preq, bool individual, MACAddress addr, bool isProactive);
        Ieee80211ActionPREQFrame*
        createPReq(std::vector<PREQElem> preq, bool individual, MACAddress addr, bool isProactive);

        /**
         * \brief forms a path error information element when list of destination fails on a given interface
         * \attention removes all entries from routing table!
         */
        HwmpProtocol::PathError makePathError(const std::vector<HwmpFailedDestination> &destinations);
        ///\brief Forwards a received path error
        void forwardPathError(PathError perr);
        ///\brief Passes a self-generated PERR to interface-plugin
        void initiatePathError(PathError perr);
        /// \return list of addresses where a PERR should be sent to
        std::vector<std::pair<uint32_t, MACAddress> > getPerrReceivers(const std::vector<HwmpFailedDestination> &failedDest);

        /// \return list of addresses where a PERR should be sent to
        std::vector<MACAddress> getPreqReceivers(uint32_t interface);
        /// \return list of addresses where a broadcast should be
        //retransmitted
        std::vector<MACAddress> getBroadcastReceivers(uint32_t interface);
        //\}

        ///\name Methods related to Queue/Dequeue procedures
        ///\{
        bool QueuePacket(QueuedPacket packet);
        QueuedPacket dequeueFirstPacketByDst(MACAddress dst);
        QueuedPacket dequeueFirstPacket();
        void reactivePathResolved(MACAddress dst);
        void proactivePathResolved();
        ///\}
        ///\name Methods responsible for path discovery retry procedure:
        ///\{
        /**
         * \brief checks when the last path discovery procedure was started for a given destination.
         *
         * If the retry counter has not achieved the maximum level - preq should not be sent
         */
        bool shouldSendPreq(MACAddress dst);

        /**
         * \brief Generates PREQ retry when retry timeout has expired and route is still unresolved.
         *
         * When PREQ retry has achieved the maximum level - retry mechanism should be canceled
         */
        void retryPathDiscovery(MACAddress dst);
        void sendMyPreq();
        void sendMyPerr();

        void discoverRouteToGan(const MACAddress &add);
        ///\}
        ///\return address of MeshPointDevice
        MACAddress GetAddress();
        ///\name Methods needed by HwmpMacLugin to access protocol parameters:
        ///\{
        bool GetToFlag();
        bool GetRfFlag();
        double GetPreqMinInterval();
        double GetPerrMinInterval();
        uint8_t GetMaxTtl();
        uint32_t GetNextPreqId();
        uint32_t GetNextHwmpSeqno();
        uint32_t GetNextGannSeqno();
        uint32_t GetActivePathLifetime();
        uint32_t GetRootPathLifetime();
        uint8_t GetUnicastPerrThreshold();
        bool isRoot()
        {
            return m_isRoot;
        }
        bool isGann()
        {
            return m_isGann;
        }
        uint32_t GetLinkMetric(const MACAddress &peerAddress);
        ///\}
    private:
        ///\name Statistics:
        ///\{
        struct Statistics
        {
                uint16_t txUnicast;
                uint16_t txBroadcast;
                uint32_t txBytes;
                uint16_t droppedTtl;
                uint16_t totalQueued;
                uint16_t totalDropped;
                uint16_t initiatedPreq;
                uint16_t initiatedPrep;
                uint16_t initiatedPerr;
                uint16_t txPrep;
                uint16_t txPerr;
                uint16_t txMgt;
                Statistics()
                {
                    memset(this, 0, sizeof(Statistics));
                }
        };
        Statistics m_stats;
        ///\}
        uint32_t m_dataSeqno;
        uint32_t m_hwmpSeqno;
        uint32_t m_preqId;
        ///\name Sequence number filters
        ///\{
        /// keeps HWMP seqno (first in pair) and HWMP metric (second in pair) for each address
        std::map<MACAddress, std::pair<uint32_t, uint32_t> > m_hwmpSeqnoMetricDatabase;
        ///\}

        /// Routing table
        HwmpRtable* m_rtable;

        ///\name Timers:
        //\{

        struct PreqEvent
        {
                PreqTimeout *preqTimeout;
                simtime_t whenScheduled;
                int numOfRetry;
        };
        std::map<MACAddress, PreqEvent> m_preqTimeouts;
        ProactivePreqTimer * m_proactivePreqTimer;
        PreqTimer * m_preqTimer;
        PerrTimer * m_perrTimer;
        simtime_t nextProactive;

        GannTimer *m_gannTimer;
        ///\}
        /// Packet Queue
        std::deque<QueuedPacket> m_rqueue;
        ///\name HWMP-protocol parameters (attributes of GetTypeId)
        ///\{
        uint16_t m_maxQueueSize;
        uint8_t m_dot11MeshHWMPmaxPREQretries;
        double m_dot11MeshHWMPnetDiameterTraversalTime;
        double m_dot11MeshHWMPpreqMinInterval;
        double m_dot11MeshHWMPperrMinInterval;
        double m_dot11MeshHWMPactiveRootTimeout;
        double m_dot11MeshHWMPactivePathTimeout;
        double m_dot11MeshHWMPpathToRootInterval;
        double m_dot11MeshHWMPrannInterval;
        double m_dot11MeshHWMPGannInterval;
        bool m_isRoot;
        uint8_t m_maxTtl;
        uint8_t m_unicastPerrThreshold;
        uint8_t m_unicastPreqThreshold;
        uint8_t m_unicastDataThreshold;
        bool m_ToFlag;
        bool m_concurrentReactive;
        ///\}
};
#endif

