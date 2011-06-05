#
# OMNeT++/OMNEST Makefile for libinet
#
# This file was generated with the command:
#  opp_makemake -f --deep --make-so -o inet -O out -Xapplications/voiptool -Xtransport/tcp_lwip -Xtransport/tcp_nsc -DWITH_RSTP -DWITH_TCP_COMMON -DWITH_TCP_INET -DWITH_IPv4 -DWITH_IPv6 -DWITH_xMIPv6 -DWITH_UDP -DWITH_RTP -DWITH_SCTP -DWITH_ETHERNET -DWITH_PPP -DWITH_EXT_IF -DWITH_MPLS -DWITH_OSPFv2 -DWITH_BGPv4 -DWITH_MANET
#

# Name of target to be created (-o option)
TARGET = libinet$(SHARED_LIB_SUFFIX)

# C++ include paths (with -I)
INCLUDE_PATH = \
    -I. \
    -Iapplications \
    -Iapplications/ethernet \
    -Iapplications/generic \
    -Iapplications/pingapp \
    -Iapplications/rtpapp \
    -Iapplications/sctpapp \
    -Iapplications/tcpapp \
    -Iapplications/traci \
    -Iapplications/trafgen \
    -Iapplications/udpapp \
    -Ibase \
    -Ibattery \
    -Ibattery/models \
    -Ilinklayer \
    -Ilinklayer/channels \
    -Ilinklayer/contract \
    -Ilinklayer/ethernet \
    -Ilinklayer/ethernet/switch \
    -Ilinklayer/ext \
    -Ilinklayer/ieee80211 \
    -Ilinklayer/ieee80211/mac \
    -Ilinklayer/ieee80211/mgmt \
    -Ilinklayer/ieee80211/radio \
    -Ilinklayer/ieee80211/radio/errormodel \
    -Ilinklayer/ieee80211mesh \
    -Ilinklayer/ieee80211mesh/hwmp \
    -Ilinklayer/ieee80211mesh/mgmt \
    -Ilinklayer/mf80211 \
    -Ilinklayer/mf80211/core \
    -Ilinklayer/mf80211/macLayer \
    -Ilinklayer/mf80211/phyLayer \
    -Ilinklayer/mf80211/phyLayer/decider \
    -Ilinklayer/mf80211/phyLayer/snrEval \
    -Ilinklayer/ppp \
    -Ilinklayer/radio \
    -Ilinklayer/radio/propagation \
    -Ilinklayer/rstp \
    -Imobility \
    -Imobility/models \
    -Imobility/traci \
    -Inetworklayer \
    -Inetworklayer/arp \
    -Inetworklayer/autorouting \
    -Inetworklayer/autorouting/ipv4 \
    -Inetworklayer/autorouting/ipv6 \
    -Inetworklayer/bgpv4 \
    -Inetworklayer/bgpv4/BGPMessage \
    -Inetworklayer/common \
    -Inetworklayer/contract \
    -Inetworklayer/extras \
    -Inetworklayer/icmpv6 \
    -Inetworklayer/ipv4 \
    -Inetworklayer/ipv4/contract \
    -Inetworklayer/ipv6 \
    -Inetworklayer/ipv6/contract \
    -Inetworklayer/ipv6tunneling \
    -Inetworklayer/ldp \
    -Inetworklayer/manetrouting \
    -Inetworklayer/manetrouting/aodv \
    -Inetworklayer/manetrouting/aodv/aodv-uu \
    -Inetworklayer/manetrouting/base \
    -Inetworklayer/manetrouting/batman \
    -Inetworklayer/manetrouting/dsdv \
    -Inetworklayer/manetrouting/dsr \
    -Inetworklayer/manetrouting/dsr/dsr-uu \
    -Inetworklayer/manetrouting/dymo \
    -Inetworklayer/manetrouting/dymo/dymoum \
    -Inetworklayer/manetrouting/dymo_fau \
    -Inetworklayer/manetrouting/olsr \
    -Inetworklayer/mpls \
    -Inetworklayer/ospfv2 \
    -Inetworklayer/ospfv2/interface \
    -Inetworklayer/ospfv2/messagehandler \
    -Inetworklayer/ospfv2/neighbor \
    -Inetworklayer/ospfv2/router \
    -Inetworklayer/queue \
    -Inetworklayer/rsvp_te \
    -Inetworklayer/ted \
    -Inetworklayer/xmipv6 \
    -Inodes \
    -Inodes/bgp \
    -Inodes/ethernet \
    -Inodes/inet \
    -Inodes/ipv6 \
    -Inodes/mf80211 \
    -Inodes/mpls \
    -Inodes/ospfv2 \
    -Inodes/wireless \
    -Inodes/xmipv6 \
    -Itransport \
    -Itransport/rtp \
    -Itransport/rtp/profiles \
    -Itransport/rtp/profiles/avprofile \
    -Itransport/sctp \
    -Itransport/sctp/contract \
    -Itransport/tcp \
    -Itransport/tcp/flavours \
    -Itransport/tcp/queues \
    -Itransport/tcp_common \
    -Itransport/tcp_common/contract \
    -Itransport/udp \
    -Itransport/udp/contract \
    -IunderTest \
    -IunderTest/applications \
    -IunderTest/applications/Ieee802154TestApp \
    -IunderTest/applications/TrafficGenerator_Erlangen \
    -IunderTest/applications/httpApp \
    -IunderTest/linklayer \
    -IunderTest/linklayer/Ieee80216 \
    -IunderTest/linklayer/Ieee80216/ControlPlane \
    -IunderTest/linklayer/Ieee80216/MACSublayer \
    -IunderTest/linklayer/Ieee80216/MACSublayer/QoS \
    -IunderTest/linklayer/Ieee80216/MACSublayer/Receiver \
    -IunderTest/linklayer/Ieee80216/MACSublayer/Transceiver \
    -IunderTest/linklayer/Ieee80216/Mac \
    -IunderTest/linklayer/Ieee80216/phyLayer \
    -IunderTest/linklayer/Ieee80216/phyLayer/decider \
    -IunderTest/linklayer/Ieee80216/phyLayer/snrEval \
    -IunderTest/linklayer/ieee80211 \
    -IunderTest/linklayer/ieee80211/mgmt \
    -IunderTest/linklayer/radio \
    -IunderTest/nodes \
    -IunderTest/nodes/WiMAX \
    -IunderTest/nodes/wireless \
    -Iutil \
    -Iutil/headerserializers \
    -Iutil/headerserializers/headers \
    -Iutil/headerserializers/ipv4 \
    -Iutil/headerserializers/ipv4/headers \
    -Iutil/headerserializers/sctp \
    -Iutil/headerserializers/sctp/headers \
    -Iutil/headerserializers/tcp \
    -Iutil/headerserializers/tcp/headers \
    -Iutil/headerserializers/udp \
    -Iutil/headerserializers/udp/headers \
    -Iworld \
    -Iworld/annotations \
    -Iworld/obstacles \
    -Iworld/powercontrol \
    -Iworld/radio \
    -Iworld/scenario \
    -Iworld/traci \
    -Iwpan \
    -Iwpan/Isabel \
    -Iwpan/Isabel/Analysis \
    -Iwpan/Isabel/Base \
    -Iwpan/applications \
    -Iwpan/applications/Ieee802154TestApp \
    -Iwpan/linklayer \
    -Iwpan/linklayer/bmac \
    -Iwpan/linklayer/ieee802154 \
    -Iwpan/linklayer/ieee802154/contract \
    -Iwpan/linklayer/ieee802154/mac \
    -Iwpan/linklayer/ieee802154/phyLayer \
    -Iwpan/networklayer \
    -Iwpan/networklayer/Ieee802154StarRouting \
    -Iwpan/networklayer/contract \
    -Iwpan/nodes

# Additional object and library files to link with
EXTRA_OBJS =

# Additional libraries (-L, -l options)
LIBS =

# Output directory
PROJECT_OUTPUT_DIR = ../out
PROJECTRELATIVE_PATH = src
O = $(PROJECT_OUTPUT_DIR)/$(CONFIGNAME)/$(PROJECTRELATIVE_PATH)

# Object files for local .cc and .msg files
OBJS = \
    $O/applications/ethernet/EtherAppCli.o \
    $O/applications/ethernet/EtherAppSrv.o \
    $O/applications/generic/IPvXTrafGen.o \
    $O/applications/pingapp/PingApp.o \
    $O/applications/rtpapp/RTPApplication.o \
    $O/applications/sctpapp/SCTPClient.o \
    $O/applications/sctpapp/SCTPPeer.o \
    $O/applications/sctpapp/SCTPServer.o \
    $O/applications/tcpapp/TCPBasicClientApp.o \
    $O/applications/tcpapp/TCPEchoApp.o \
    $O/applications/tcpapp/TCPGenericCliAppBase.o \
    $O/applications/tcpapp/TCPGenericSrvApp.o \
    $O/applications/tcpapp/TCPGenericSrvThread.o \
    $O/applications/tcpapp/TCPSessionApp.o \
    $O/applications/tcpapp/TCPSinkApp.o \
    $O/applications/tcpapp/TCPSrvHostApp.o \
    $O/applications/tcpapp/TelnetApp.o \
    $O/applications/traci/Test_TraCI.o \
    $O/applications/trafgen/TrafGen.o \
    $O/applications/trafgen/TrafGenPar.o \
    $O/applications/udpapp/UDPAppBase.o \
    $O/applications/udpapp/UDPBasicApp.o \
    $O/applications/udpapp/UDPBasicBurst.o \
    $O/applications/udpapp/UDPEchoApp.o \
    $O/applications/udpapp/UDPSink.o \
    $O/applications/udpapp/UDPVideoStreamCli.o \
    $O/applications/udpapp/UDPVideoStreamSvr.o \
    $O/base/AbstractQueue.o \
    $O/base/BasicModule.o \
    $O/base/Blackboard.o \
    $O/base/ByteArray.o \
    $O/base/ByteArrayBuffer.o \
    $O/base/ByteArrayMessage.o \
    $O/base/Join.o \
    $O/base/MMapBoard.o \
    $O/base/ModuleAccess.o \
    $O/base/NedFunctions.o \
    $O/base/NotificationBoard.o \
    $O/base/NotifierConsts.o \
    $O/base/PassiveQueueBase.o \
    $O/base/ProtocolMap.o \
    $O/base/QueueBase.o \
    $O/base/QueueWithQoS.o \
    $O/base/ReassemblyBuffer.o \
    $O/base/Sink.o \
    $O/base/UnimplementedModule.o \
    $O/battery/models/BasicBattery.o \
    $O/battery/models/InetSimpleBattery.o \
    $O/linklayer/channels/LinkFailureManager.o \
    $O/linklayer/channels/MyThroughputMeteringChannel.o \
    $O/linklayer/channels/ProgramedFailureChannel.o \
    $O/linklayer/contract/MACAddress.o \
    $O/linklayer/ethernet/EtherBus.o \
    $O/linklayer/ethernet/EtherEncap.o \
    $O/linklayer/ethernet/EtherHub.o \
    $O/linklayer/ethernet/EtherLLC.o \
    $O/linklayer/ethernet/EtherMAC.o \
    $O/linklayer/ethernet/EtherMACBase.o \
    $O/linklayer/ethernet/EtherMACFullDuplex.o \
    $O/linklayer/ethernet/switch/MACRelayUnitBase.o \
    $O/linklayer/ethernet/switch/MACRelayUnitNP.o \
    $O/linklayer/ethernet/switch/MACRelayUnitPP.o \
    $O/linklayer/ext/cSocketRTScheduler.o \
    $O/linklayer/ext/ExtInterface.o \
    $O/linklayer/ieee80211/mac/Ieee80211Mac.o \
    $O/linklayer/ieee80211/mac/Ieee80211NewMac.o \
    $O/linklayer/ieee80211/mac/WifiMode.o \
    $O/linklayer/ieee80211/mac/WirelessMacBase.o \
    $O/linklayer/ieee80211/mgmt/Ieee80211AgentSTA.o \
    $O/linklayer/ieee80211/mgmt/Ieee80211eClassifier.o \
    $O/linklayer/ieee80211/mgmt/Ieee80211Etx.o \
    $O/linklayer/ieee80211/mgmt/Ieee80211MgmtAdhoc.o \
    $O/linklayer/ieee80211/mgmt/Ieee80211MgmtAdhocWithEtx.o \
    $O/linklayer/ieee80211/mgmt/Ieee80211MgmtAP.o \
    $O/linklayer/ieee80211/mgmt/Ieee80211MgmtAPBase.o \
    $O/linklayer/ieee80211/mgmt/Ieee80211MgmtAPSimplified.o \
    $O/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.o \
    $O/linklayer/ieee80211/mgmt/Ieee80211MgmtSTA.o \
    $O/linklayer/ieee80211/mgmt/Ieee80211MgmtSTASimplified.o \
    $O/linklayer/ieee80211/radio/BerParseFile.o \
    $O/linklayer/ieee80211/radio/Ieee80211NewRadioModel.o \
    $O/linklayer/ieee80211/radio/Ieee80211RadioModel.o \
    $O/linklayer/ieee80211/radio/errormodel/dsss-error-rate-model.o \
    $O/linklayer/ieee80211/radio/errormodel/yans-error-rate-model.o \
    $O/linklayer/ieee80211mesh/hwmp/hwmp-rtable.o \
    $O/linklayer/ieee80211mesh/hwmp/hwmp.o \
    $O/linklayer/ieee80211mesh/mgmt/Ieee80211Mesh.o \
    $O/linklayer/ieee80211mesh/mgmt/Ieee80211MeshLwmpls.o \
    $O/linklayer/ieee80211mesh/mgmt/lwmpls_data.o \
    $O/linklayer/mf80211/core/BasicDecider.o \
    $O/linklayer/mf80211/core/BasicSnrEval.o \
    $O/linklayer/mf80211/macLayer/CSMAMacLayer.o \
    $O/linklayer/mf80211/macLayer/Mac80211.o \
    $O/linklayer/mf80211/macLayer/MacBase.o \
    $O/linklayer/mf80211/phyLayer/decider/Decider80211.o \
    $O/linklayer/mf80211/phyLayer/decider/ErrAndCollDecider.o \
    $O/linklayer/mf80211/phyLayer/decider/SnrDecider.o \
    $O/linklayer/mf80211/phyLayer/snrEval/GilbertElliotSnr.o \
    $O/linklayer/mf80211/phyLayer/snrEval/SnrEval.o \
    $O/linklayer/mf80211/phyLayer/snrEval/SnrEval80211.o \
    $O/linklayer/ppp/PPP.o \
    $O/linklayer/radio/GenericRadioModel.o \
    $O/linklayer/radio/Modulation.o \
    $O/linklayer/radio/Radio.o \
    $O/linklayer/radio/propagation/FreeSpaceModel.o \
    $O/linklayer/radio/propagation/LogNormalShadowingModel.o \
    $O/linklayer/radio/propagation/NakagamiModel.o \
    $O/linklayer/radio/propagation/RayleighModel.o \
    $O/linklayer/radio/propagation/RiceModel.o \
    $O/linklayer/radio/propagation/TwoRayGroundModel.o \
    $O/linklayer/rstp/MACRelayUnitSTPNP.o \
    $O/linklayer/rstp/STPDefinitions.o \
    $O/mobility/models/ANSimMobility.o \
    $O/mobility/models/BasicMobility.o \
    $O/mobility/models/BonnMotionFileCache.o \
    $O/mobility/models/BonnMotionMobility.o \
    $O/mobility/models/ChiangMobility.o \
    $O/mobility/models/CircleMobility.o \
    $O/mobility/models/ConstSpeedMobility.o \
    $O/mobility/models/GaussMarkovMobility.o \
    $O/mobility/models/LinearMobility.o \
    $O/mobility/models/LineSegmentsMobilityBase.o \
    $O/mobility/models/MassMobility.o \
    $O/mobility/models/Ns2MotionMobility.o \
    $O/mobility/models/NullMobility.o \
    $O/mobility/models/RandomWPMobility.o \
    $O/mobility/models/RectangleMobility.o \
    $O/mobility/models/StaticGridMobility.o \
    $O/mobility/models/TurtleMobility.o \
    $O/mobility/traci/TraCIMobility.o \
    $O/networklayer/arp/ARP.o \
    $O/networklayer/autorouting/ipv4/FlatNetworkConfigurator.o \
    $O/networklayer/autorouting/ipv4/HostAutoConfigurator2.o \
    $O/networklayer/autorouting/ipv4/NetworkConfigurator.o \
    $O/networklayer/autorouting/ipv6/FlatNetworkConfigurator6.o \
    $O/networklayer/bgpv4/BGPFSM.o \
    $O/networklayer/bgpv4/BGPRouting.o \
    $O/networklayer/bgpv4/BGPSession.o \
    $O/networklayer/bgpv4/BGPMessage/BGPUpdate.o \
    $O/networklayer/common/InterfaceEntry.o \
    $O/networklayer/common/InterfaceTable.o \
    $O/networklayer/contract/IPv4Address.o \
    $O/networklayer/contract/IPv6Address.o \
    $O/networklayer/contract/IPvXAddress.o \
    $O/networklayer/contract/IPvXAddressResolver.o \
    $O/networklayer/extras/Dummy.o \
    $O/networklayer/extras/FailureManager.o \
    $O/networklayer/icmpv6/ICMPv6.o \
    $O/networklayer/icmpv6/IPv6NeighbourCache.o \
    $O/networklayer/icmpv6/IPv6NeighbourDiscovery.o \
    $O/networklayer/ipv4/ErrorHandling.o \
    $O/networklayer/ipv4/ICMP.o \
    $O/networklayer/ipv4/IPv4.o \
    $O/networklayer/ipv4/IPv4FragBuf.o \
    $O/networklayer/ipv4/IPv4InterfaceData.o \
    $O/networklayer/ipv4/IPv4Route.o \
    $O/networklayer/ipv4/IPv4RouteRule.o \
    $O/networklayer/ipv4/NetworkInfo.o \
    $O/networklayer/ipv4/RoutingTable.o \
    $O/networklayer/ipv4/RoutingTableParser.o \
    $O/networklayer/ipv4/contract/IPv4ControlInfo.o \
    $O/networklayer/ipv6/IPv6.o \
    $O/networklayer/ipv6/IPv6Datagram.o \
    $O/networklayer/ipv6/IPv6ErrorHandling.o \
    $O/networklayer/ipv6/IPv6ExtensionHeaders.o \
    $O/networklayer/ipv6/IPv6FragBuf.o \
    $O/networklayer/ipv6/IPv6InterfaceData.o \
    $O/networklayer/ipv6/RoutingTable6.o \
    $O/networklayer/ipv6/contract/IPv6ControlInfo.o \
    $O/networklayer/ipv6tunneling/IPv6Tunneling.o \
    $O/networklayer/ldp/LDP.o \
    $O/networklayer/manetrouting/aodv/aodv_msg_struct.o \
    $O/networklayer/manetrouting/aodv/aodv_uu_omnet.o \
    $O/networklayer/manetrouting/aodv/packet_queue_omnet.o \
    $O/networklayer/manetrouting/aodv/aodv-uu/aodv_hello.o \
    $O/networklayer/manetrouting/aodv/aodv-uu/aodv_neighbor.o \
    $O/networklayer/manetrouting/aodv/aodv-uu/aodv_rerr.o \
    $O/networklayer/manetrouting/aodv/aodv-uu/aodv_rrep.o \
    $O/networklayer/manetrouting/aodv/aodv-uu/aodv_rreq.o \
    $O/networklayer/manetrouting/aodv/aodv-uu/aodv_socket.o \
    $O/networklayer/manetrouting/aodv/aodv-uu/aodv_timeout.o \
    $O/networklayer/manetrouting/aodv/aodv-uu/debug_aodv.o \
    $O/networklayer/manetrouting/aodv/aodv-uu/list.o \
    $O/networklayer/manetrouting/aodv/aodv-uu/locality.o \
    $O/networklayer/manetrouting/aodv/aodv-uu/routing_table.o \
    $O/networklayer/manetrouting/aodv/aodv-uu/seek_list.o \
    $O/networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.o \
    $O/networklayer/manetrouting/base/ManetRoutingBase.o \
    $O/networklayer/manetrouting/batman/BatmanData.o \
    $O/networklayer/manetrouting/batman/batmanHna.o \
    $O/networklayer/manetrouting/batman/BatmanMain.o \
    $O/networklayer/manetrouting/dsdv/DSDV_2.o \
    $O/networklayer/manetrouting/dsr/dsr-pkt_omnet.o \
    $O/networklayer/manetrouting/dsr/dsr-uu-omnetpp.o \
    $O/networklayer/manetrouting/dsr/dsr-uu/dsr-ack.o \
    $O/networklayer/manetrouting/dsr/dsr-uu/dsr-io.o \
    $O/networklayer/manetrouting/dsr/dsr-uu/dsr-opt.o \
    $O/networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.o \
    $O/networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.o \
    $O/networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.o \
    $O/networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.o \
    $O/networklayer/manetrouting/dsr/dsr-uu/dsr-srt.o \
    $O/networklayer/manetrouting/dsr/dsr-uu/link-cache.o \
    $O/networklayer/manetrouting/dsr/dsr-uu/maint-buf.o \
    $O/networklayer/manetrouting/dsr/dsr-uu/neigh.o \
    $O/networklayer/manetrouting/dsr/dsr-uu/path-cache.o \
    $O/networklayer/manetrouting/dsr/dsr-uu/send-buf.o \
    $O/networklayer/manetrouting/dymo/dymo_msg_struct.o \
    $O/networklayer/manetrouting/dymo/dymo_packet_queue_omnet.o \
    $O/networklayer/manetrouting/dymo/dymo_um_omnet.o \
    $O/networklayer/manetrouting/dymo/dymoum/blacklist.o \
    $O/networklayer/manetrouting/dymo/dymoum/debug_dymo.o \
    $O/networklayer/manetrouting/dymo/dymoum/dymo_generic.o \
    $O/networklayer/manetrouting/dymo/dymoum/dymo_hello.o \
    $O/networklayer/manetrouting/dymo/dymoum/dymo_nb.o \
    $O/networklayer/manetrouting/dymo/dymoum/dymo_re.o \
    $O/networklayer/manetrouting/dymo/dymoum/dymo_rerr.o \
    $O/networklayer/manetrouting/dymo/dymoum/dymo_socket.o \
    $O/networklayer/manetrouting/dymo/dymoum/dymo_timeout.o \
    $O/networklayer/manetrouting/dymo/dymoum/dymo_uerr.o \
    $O/networklayer/manetrouting/dymo/dymoum/icmp_socket.o \
    $O/networklayer/manetrouting/dymo/dymoum/pending_rreq.o \
    $O/networklayer/manetrouting/dymo/dymoum/rtable.o \
    $O/networklayer/manetrouting/dymo/dymoum/timer_queue.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO_DataQueue.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO_OutstandingRREQList.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO_RoutingEntry.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO_RoutingTable.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO_Timer.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO_TokenBucket.o \
    $O/networklayer/manetrouting/dymo_fau/InspectorHelpers.o \
    $O/networklayer/manetrouting/olsr/OLSR.o \
    $O/networklayer/manetrouting/olsr/OLSR_ETX.o \
    $O/networklayer/manetrouting/olsr/OLSR_ETX_dijkstra.o \
    $O/networklayer/manetrouting/olsr/OLSR_ETX_state.o \
    $O/networklayer/manetrouting/olsr/OLSR_rtable.o \
    $O/networklayer/manetrouting/olsr/OLSR_state.o \
    $O/networklayer/mpls/LIBTable.o \
    $O/networklayer/mpls/MPLS.o \
    $O/networklayer/mpls/MPLSPacket.o \
    $O/networklayer/ospfv2/OSPFRouting.o \
    $O/networklayer/ospfv2/interface/OSPFInterface.o \
    $O/networklayer/ospfv2/interface/OSPFInterfaceState.o \
    $O/networklayer/ospfv2/interface/OSPFInterfaceStateBackup.o \
    $O/networklayer/ospfv2/interface/OSPFInterfaceStateDesignatedRouter.o \
    $O/networklayer/ospfv2/interface/OSPFInterfaceStateDown.o \
    $O/networklayer/ospfv2/interface/OSPFInterfaceStateLoopback.o \
    $O/networklayer/ospfv2/interface/OSPFInterfaceStateNotDesignatedRouter.o \
    $O/networklayer/ospfv2/interface/OSPFInterfaceStatePointToPoint.o \
    $O/networklayer/ospfv2/interface/OSPFInterfaceStateWaiting.o \
    $O/networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.o \
    $O/networklayer/ospfv2/messagehandler/HelloHandler.o \
    $O/networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.o \
    $O/networklayer/ospfv2/messagehandler/LinkStateRequestHandler.o \
    $O/networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.o \
    $O/networklayer/ospfv2/messagehandler/MessageHandler.o \
    $O/networklayer/ospfv2/neighbor/OSPFNeighbor.o \
    $O/networklayer/ospfv2/neighbor/OSPFNeighborState.o \
    $O/networklayer/ospfv2/neighbor/OSPFNeighborStateAttempt.o \
    $O/networklayer/ospfv2/neighbor/OSPFNeighborStateDown.o \
    $O/networklayer/ospfv2/neighbor/OSPFNeighborStateExchange.o \
    $O/networklayer/ospfv2/neighbor/OSPFNeighborStateExchangeStart.o \
    $O/networklayer/ospfv2/neighbor/OSPFNeighborStateFull.o \
    $O/networklayer/ospfv2/neighbor/OSPFNeighborStateInit.o \
    $O/networklayer/ospfv2/neighbor/OSPFNeighborStateLoading.o \
    $O/networklayer/ospfv2/neighbor/OSPFNeighborStateTwoWay.o \
    $O/networklayer/ospfv2/router/ASExternalLSA.o \
    $O/networklayer/ospfv2/router/NetworkLSA.o \
    $O/networklayer/ospfv2/router/OSPFArea.o \
    $O/networklayer/ospfv2/router/OSPFRouter.o \
    $O/networklayer/ospfv2/router/RouterLSA.o \
    $O/networklayer/ospfv2/router/SummaryLSA.o \
    $O/networklayer/queue/BasicDSCPClassifier.o \
    $O/networklayer/queue/DropTailQoSQueue.o \
    $O/networklayer/queue/DropTailQueue.o \
    $O/networklayer/queue/REDQueue.o \
    $O/networklayer/rsvp_te/RSVP.o \
    $O/networklayer/rsvp_te/SimpleClassifier.o \
    $O/networklayer/rsvp_te/Utils.o \
    $O/networklayer/ted/LinkStateRouting.o \
    $O/networklayer/ted/TED.o \
    $O/networklayer/xmipv6/BindingCache.o \
    $O/networklayer/xmipv6/BindingUpdateList.o \
    $O/networklayer/xmipv6/MobilityHeader.o \
    $O/networklayer/xmipv6/xMIPv6.o \
    $O/transport/rtp/reports.o \
    $O/transport/rtp/RTCP.o \
    $O/transport/rtp/RTCPPacket.o \
    $O/transport/rtp/RTP.o \
    $O/transport/rtp/RTPInnerPacket.o \
    $O/transport/rtp/RTPPacket.o \
    $O/transport/rtp/RTPParticipantInfo.o \
    $O/transport/rtp/RTPPayloadReceiver.o \
    $O/transport/rtp/RTPPayloadSender.o \
    $O/transport/rtp/RTPProfile.o \
    $O/transport/rtp/RTPReceiverInfo.o \
    $O/transport/rtp/RTPSenderInfo.o \
    $O/transport/rtp/sdes.o \
    $O/transport/rtp/profiles/avprofile/RTPAVProfile.o \
    $O/transport/rtp/profiles/avprofile/RTPAVProfilePayload32Receiver.o \
    $O/transport/rtp/profiles/avprofile/RTPAVProfilePayload32Sender.o \
    $O/transport/sctp/SCTP.o \
    $O/transport/sctp/SCTPAlg.o \
    $O/transport/sctp/SCTPAssociationBase.o \
    $O/transport/sctp/SCTPAssociationEventProc.o \
    $O/transport/sctp/SCTPAssociationRcvMessage.o \
    $O/transport/sctp/SCTPAssociationSendAll.o \
    $O/transport/sctp/SCTPAssociationUtil.o \
    $O/transport/sctp/SCTPCCFunctions.o \
    $O/transport/sctp/SCTPMessage.o \
    $O/transport/sctp/SCTPQueue.o \
    $O/transport/sctp/SCTPReceiveStream.o \
    $O/transport/sctp/SCTPSendStream.o \
    $O/transport/sctp/SCTPSSFunctions.o \
    $O/transport/sctp/contract/SCTPSocket.o \
    $O/transport/tcp/TCP.o \
    $O/transport/tcp/TCPConnectionBase.o \
    $O/transport/tcp/TCPConnectionEventProc.o \
    $O/transport/tcp/TCPConnectionRcvSegment.o \
    $O/transport/tcp/TCPConnectionSackUtil.o \
    $O/transport/tcp/TCPConnectionUtil.o \
    $O/transport/tcp/TCPSACKRexmitQueue.o \
    $O/transport/tcp/flavours/DumbTCP.o \
    $O/transport/tcp/flavours/TCPBaseAlg.o \
    $O/transport/tcp/flavours/TCPNewReno.o \
    $O/transport/tcp/flavours/TCPNoCongestionControl.o \
    $O/transport/tcp/flavours/TCPReno.o \
    $O/transport/tcp/flavours/TCPTahoe.o \
    $O/transport/tcp/flavours/TCPTahoeRenoFamily.o \
    $O/transport/tcp/queues/TCPDataStreamRcvQueue.o \
    $O/transport/tcp/queues/TCPDataStreamSendQueue.o \
    $O/transport/tcp/queues/TCPMsgBasedRcvQueue.o \
    $O/transport/tcp/queues/TCPMsgBasedSendQueue.o \
    $O/transport/tcp/queues/TCPVirtualDataRcvQueue.o \
    $O/transport/tcp/queues/TCPVirtualDataSendQueue.o \
    $O/transport/tcp_common/TCPSegment.o \
    $O/transport/tcp_common/TCPSpoof.o \
    $O/transport/tcp_common/contract/TCPSocket.o \
    $O/transport/tcp_common/contract/TCPSocketMap.o \
    $O/transport/udp/UDP.o \
    $O/transport/udp/contract/UDPSocket.o \
    $O/underTest/applications/TrafficGenerator_Erlangen/WiMAXQoSTrafficGenerator.o \
    $O/underTest/applications/httpApp/httptBrowser.o \
    $O/underTest/applications/httpApp/httptBrowserBase.o \
    $O/underTest/applications/httpApp/httptBrowserDirect.o \
    $O/underTest/applications/httpApp/httptController.o \
    $O/underTest/applications/httpApp/httptNodeBase.o \
    $O/underTest/applications/httpApp/httptRandom.o \
    $O/underTest/applications/httpApp/httptServer.o \
    $O/underTest/applications/httpApp/httptServerBase.o \
    $O/underTest/applications/httpApp/httptServerDirect.o \
    $O/underTest/applications/httpApp/httptServerDirectEvilA.o \
    $O/underTest/applications/httpApp/httptServerDirectEvilB.o \
    $O/underTest/applications/httpApp/httptServerEvilA.o \
    $O/underTest/applications/httpApp/httptServerEvilB.o \
    $O/underTest/applications/httpApp/httptUtils.o \
    $O/underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneAP.o \
    $O/underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBase.o \
    $O/underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBaseAP.o \
    $O/underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBaseMS.o \
    $O/underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBasestation.o \
    $O/underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneMobilestation.o \
    $O/underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneMS.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerAuthorizationModule.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerControlModule.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerFragmentation.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerQoSAP.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerScheduling.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows_BS.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows_MS.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/QoS/ConvergenceSublayerControlModule.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/QoS/ConvergenceSublayerHeaderCompression.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/QoS/ConvergenceSublayerTrafficClassification.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/Receiver/CommonPartSublayerReceiver.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/Receiver/ConvergenceSublayerReceiver.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/Receiver/SecuritySublayerReceiver.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/Transceiver/CommonPartSublayerTransceiver.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/Transceiver/ConvergenceSublayerTransceiver.o \
    $O/underTest/linklayer/Ieee80216/MACSublayer/Transceiver/SecuritySublayerTransceiver.o \
    $O/underTest/linklayer/Ieee80216/Mac/CommonPartSublayerDown.o \
    $O/underTest/linklayer/Ieee80216/Mac/CommonPartSublayerUp.o \
    $O/underTest/linklayer/Ieee80216/Mac/ConvergenceSublayerDown.o \
    $O/underTest/linklayer/Ieee80216/Mac/ConvergenceSublayerUp.o \
    $O/underTest/linklayer/Ieee80216/Mac/SecuritySublayerDown.o \
    $O/underTest/linklayer/Ieee80216/Mac/SecuritySublayerUp.o \
    $O/underTest/linklayer/Ieee80216/phyLayer/decider/Decider80216.o \
    $O/underTest/linklayer/ieee80211/mgmt/Ieee80211AgentSTAExtended.o \
    $O/underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtAPExtended.o \
    $O/underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtBaseExtended.o \
    $O/underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtSTAExtended.o \
    $O/underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtWDSExtended.o \
    $O/underTest/linklayer/radio/Ieee80216Radio.o \
    $O/underTest/linklayer/radio/Ieee80216RadioModel.o \
    $O/underTest/linklayer/radio/WiMAXPathLossReceptionModel.o \
    $O/util/common.o \
    $O/util/DropsGenerator.o \
    $O/util/DuplicatesGenerator.o \
    $O/util/int128.o \
    $O/util/Macho.o \
    $O/util/MessageChecker.o \
    $O/util/NetAnimTrace.o \
    $O/util/opp_utils.o \
    $O/util/PacketDump.o \
    $O/util/PcapDump.o \
    $O/util/PcapRecorder.o \
    $O/util/TCPDump.o \
    $O/util/ThruputMeter.o \
    $O/util/ThruputMeteringChannel.o \
    $O/util/uint128.o \
    $O/util/XMLUtils.o \
    $O/util/headerserializers/TCPIPchecksum.o \
    $O/util/headerserializers/ipv4/ICMPSerializer.o \
    $O/util/headerserializers/ipv4/IPv4Serializer.o \
    $O/util/headerserializers/sctp/SCTPSerializer.o \
    $O/util/headerserializers/tcp/TCPSerializer.o \
    $O/util/headerserializers/udp/UDPSerializer.o \
    $O/world/annotations/AnnotationDummy.o \
    $O/world/annotations/AnnotationManager.o \
    $O/world/obstacles/Obstacle.o \
    $O/world/obstacles/ObstacleControl.o \
    $O/world/powercontrol/PowerControlManager.o \
    $O/world/radio/ChannelAccess.o \
    $O/world/radio/ChannelControl.o \
    $O/world/scenario/ScenarioManager.o \
    $O/world/traci/TraCIScenarioManager.o \
    $O/world/traci/TraCIScenarioManagerLaunchd.o \
    $O/wpan/Isabel/Analysis/AnalysisEnergy.o \
    $O/wpan/Isabel/Base/Display.o \
    $O/wpan/applications/Ieee802154TestApp/Ieee802154TestApp.o \
    $O/wpan/linklayer/bmac/BMacLayer.o \
    $O/wpan/linklayer/ieee802154/mac/csma802154.o \
    $O/wpan/linklayer/ieee802154/mac/Ieee802154Link.o \
    $O/wpan/linklayer/ieee802154/mac/Ieee802154Mac.o \
    $O/wpan/linklayer/ieee802154/phyLayer/Ieee802154Phy.o \
    $O/wpan/linklayer/ieee802154/phyLayer/Ieee802154RadioModel.o \
    $O/wpan/networklayer/Ieee802154StarRouting/Ieee802154StarRouting.o \
    $O/applications/ethernet/EtherApp_m.o \
    $O/applications/pingapp/PingPayload_m.o \
    $O/applications/tcpapp/GenericAppMsg_m.o \
    $O/applications/udpapp/UDPEchoAppMsg_m.o \
    $O/base/ByteArray_m.o \
    $O/base/ByteArrayMessage_m.o \
    $O/linklayer/channels/LinkFailureMessage_m.o \
    $O/linklayer/contract/Ieee802Ctrl_m.o \
    $O/linklayer/contract/PhyControlInfo_m.o \
    $O/linklayer/ethernet/EtherFrame_m.o \
    $O/linklayer/ext/ExtFrame_m.o \
    $O/linklayer/ieee80211/mac/Ieee80211Frame_m.o \
    $O/linklayer/ieee80211/mgmt/ETXPacket_m.o \
    $O/linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.o \
    $O/linklayer/ieee80211/mgmt/Ieee80211Primitives_m.o \
    $O/linklayer/ieee80211mesh/mgmt/LWMPLSPacket_m.o \
    $O/linklayer/mf80211/core/SnrControlInfo_m.o \
    $O/linklayer/mf80211/core/TransmComplete_m.o \
    $O/linklayer/mf80211/macLayer/Mac80211Pkt_m.o \
    $O/linklayer/mf80211/macLayer/MacPkt_m.o \
    $O/linklayer/ppp/PPPFrame_m.o \
    $O/linklayer/radio/AirFrame_m.o \
    $O/linklayer/radio/Radio80211aControlInfo_m.o \
    $O/linklayer/rstp/bpdu_m.o \
    $O/linklayer/rstp/stpTimers_m.o \
    $O/networklayer/arp/ARPPacket_m.o \
    $O/networklayer/bgpv4/BGPMessage/BGPASPathSegment_m.o \
    $O/networklayer/bgpv4/BGPMessage/BGPHeader_m.o \
    $O/networklayer/bgpv4/BGPMessage/BGPKeepAlive_m.o \
    $O/networklayer/bgpv4/BGPMessage/BGPOpen_m.o \
    $O/networklayer/bgpv4/BGPMessage/BGPUpdate_m.o \
    $O/networklayer/bgpv4/BGPMessage/BGPUpdatePathAttributes_m.o \
    $O/networklayer/contract/IPProtocolId_m.o \
    $O/networklayer/icmpv6/ICMPv6Message_m.o \
    $O/networklayer/icmpv6/IPv6NDMessage_m.o \
    $O/networklayer/ipv4/ICMPMessage_m.o \
    $O/networklayer/ipv4/IPv4Datagram_m.o \
    $O/networklayer/ipv4/contract/IPv4ControlInfo_m.o \
    $O/networklayer/ipv6/IPv6Datagram_m.o \
    $O/networklayer/ipv6/IPv6ExtensionHeaders_m.o \
    $O/networklayer/ipv6/contract/IPv6ControlInfo_m.o \
    $O/networklayer/ldp/LDPPacket_m.o \
    $O/networklayer/manetrouting/base/ControlInfoBreakLink_m.o \
    $O/networklayer/manetrouting/base/ControlManetRouting_m.o \
    $O/networklayer/manetrouting/base/MeshControlInfo_m.o \
    $O/networklayer/manetrouting/batman/BatmanMsg_m.o \
    $O/networklayer/manetrouting/dsdv/DSDVhello_m.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO_Packet_m.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage_m.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO_RERR_m.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO_RM_m.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO_RREP_m.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO_RREQ_m.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO_Timeout_m.o \
    $O/networklayer/manetrouting/dymo_fau/DYMO_UERR_m.o \
    $O/networklayer/manetrouting/olsr/OLSRpkt_m.o \
    $O/networklayer/ospfv2/OSPFPacket_m.o \
    $O/networklayer/ospfv2/OSPFTimer_m.o \
    $O/networklayer/rsvp_te/IntServ_m.o \
    $O/networklayer/rsvp_te/RSVPHello_m.o \
    $O/networklayer/rsvp_te/RSVPPacket_m.o \
    $O/networklayer/rsvp_te/RSVPPathMsg_m.o \
    $O/networklayer/rsvp_te/RSVPResvMsg_m.o \
    $O/networklayer/rsvp_te/SignallingMsg_m.o \
    $O/networklayer/ted/LinkStatePacket_m.o \
    $O/networklayer/ted/TED_m.o \
    $O/networklayer/xmipv6/MobilityHeader_m.o \
    $O/transport/rtp/reports_m.o \
    $O/transport/rtp/RTCPPacket1_m.o \
    $O/transport/rtp/RTCPPacket2_m.o \
    $O/transport/rtp/RTCPPacket3_m.o \
    $O/transport/rtp/RTPInnerPacket_m.o \
    $O/transport/rtp/RTPInterfacePacket_m.o \
    $O/transport/rtp/RTPPacket_m.o \
    $O/transport/rtp/RTPParticipantInfo_m.o \
    $O/transport/rtp/RTPSenderControlMessage_m.o \
    $O/transport/rtp/RTPSenderStatusMessage_m.o \
    $O/transport/rtp/profiles/avprofile/RTPMpegPacket_m.o \
    $O/transport/sctp/SCTPMessage_m.o \
    $O/transport/sctp/contract/SCTPCommand_m.o \
    $O/transport/tcp_common/TCPSegment_m.o \
    $O/transport/tcp_common/contract/TCPCommand_m.o \
    $O/transport/udp/UDPPacket_m.o \
    $O/transport/udp/contract/UDPControlInfo_m.o \
    $O/underTest/applications/httpApp/httptEventMessages_m.o \
    $O/underTest/applications/httpApp/httptMessages_m.o \
    $O/underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.o \
    $O/underTest/linklayer/Ieee80216/ControlPlane/Ieee80216Primitives_m.o \
    $O/underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.o \
    $O/underTest/linklayer/Ieee80216/Mac/WiMAXRadioControl_m.o \
    $O/world/powercontrol/PowerControlMessages_m.o \
    $O/wpan/applications/Ieee802154TestApp/Ieee802154AppPkt_m.o \
    $O/wpan/linklayer/bmac/bmacpkt_m.o \
    $O/wpan/linklayer/ieee802154/contract/Ieee802154MacPhyPrimitives_m.o \
    $O/wpan/linklayer/ieee802154/mac/Ieee802154Frame_m.o \
    $O/wpan/networklayer/contract/Ieee802154NetworkCtrlInfo_m.o

# Message files
MSGFILES = \
    applications/ethernet/EtherApp.msg \
    applications/pingapp/PingPayload.msg \
    applications/tcpapp/GenericAppMsg.msg \
    applications/udpapp/UDPEchoAppMsg.msg \
    base/ByteArray.msg \
    base/ByteArrayMessage.msg \
    linklayer/channels/LinkFailureMessage.msg \
    linklayer/contract/Ieee802Ctrl.msg \
    linklayer/contract/PhyControlInfo.msg \
    linklayer/ethernet/EtherFrame.msg \
    linklayer/ext/ExtFrame.msg \
    linklayer/ieee80211/mac/Ieee80211Frame.msg \
    linklayer/ieee80211/mgmt/ETXPacket.msg \
    linklayer/ieee80211/mgmt/Ieee80211MgmtFrames.msg \
    linklayer/ieee80211/mgmt/Ieee80211Primitives.msg \
    linklayer/ieee80211mesh/mgmt/LWMPLSPacket.msg \
    linklayer/mf80211/core/SnrControlInfo.msg \
    linklayer/mf80211/core/TransmComplete.msg \
    linklayer/mf80211/macLayer/Mac80211Pkt.msg \
    linklayer/mf80211/macLayer/MacPkt.msg \
    linklayer/ppp/PPPFrame.msg \
    linklayer/radio/AirFrame.msg \
    linklayer/radio/Radio80211aControlInfo.msg \
    linklayer/rstp/bpdu.msg \
    linklayer/rstp/stpTimers.msg \
    networklayer/arp/ARPPacket.msg \
    networklayer/bgpv4/BGPMessage/BGPASPathSegment.msg \
    networklayer/bgpv4/BGPMessage/BGPHeader.msg \
    networklayer/bgpv4/BGPMessage/BGPKeepAlive.msg \
    networklayer/bgpv4/BGPMessage/BGPOpen.msg \
    networklayer/bgpv4/BGPMessage/BGPUpdate.msg \
    networklayer/bgpv4/BGPMessage/BGPUpdatePathAttributes.msg \
    networklayer/contract/IPProtocolId.msg \
    networklayer/icmpv6/ICMPv6Message.msg \
    networklayer/icmpv6/IPv6NDMessage.msg \
    networklayer/ipv4/ICMPMessage.msg \
    networklayer/ipv4/IPv4Datagram.msg \
    networklayer/ipv4/contract/IPv4ControlInfo.msg \
    networklayer/ipv6/IPv6Datagram.msg \
    networklayer/ipv6/IPv6ExtensionHeaders.msg \
    networklayer/ipv6/contract/IPv6ControlInfo.msg \
    networklayer/ldp/LDPPacket.msg \
    networklayer/manetrouting/base/ControlInfoBreakLink.msg \
    networklayer/manetrouting/base/ControlManetRouting.msg \
    networklayer/manetrouting/base/MeshControlInfo.msg \
    networklayer/manetrouting/batman/BatmanMsg.msg \
    networklayer/manetrouting/dsdv/DSDVhello.msg \
    networklayer/manetrouting/dymo_fau/DYMO_Packet.msg \
    networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage.msg \
    networklayer/manetrouting/dymo_fau/DYMO_RERR.msg \
    networklayer/manetrouting/dymo_fau/DYMO_RM.msg \
    networklayer/manetrouting/dymo_fau/DYMO_RREP.msg \
    networklayer/manetrouting/dymo_fau/DYMO_RREQ.msg \
    networklayer/manetrouting/dymo_fau/DYMO_Timeout.msg \
    networklayer/manetrouting/dymo_fau/DYMO_UERR.msg \
    networklayer/manetrouting/olsr/OLSRpkt.msg \
    networklayer/ospfv2/OSPFPacket.msg \
    networklayer/ospfv2/OSPFTimer.msg \
    networklayer/rsvp_te/IntServ.msg \
    networklayer/rsvp_te/RSVPHello.msg \
    networklayer/rsvp_te/RSVPPacket.msg \
    networklayer/rsvp_te/RSVPPathMsg.msg \
    networklayer/rsvp_te/RSVPResvMsg.msg \
    networklayer/rsvp_te/SignallingMsg.msg \
    networklayer/ted/LinkStatePacket.msg \
    networklayer/ted/TED.msg \
    networklayer/xmipv6/MobilityHeader.msg \
    transport/rtp/reports.msg \
    transport/rtp/RTCPPacket1.msg \
    transport/rtp/RTCPPacket2.msg \
    transport/rtp/RTCPPacket3.msg \
    transport/rtp/RTPInnerPacket.msg \
    transport/rtp/RTPInterfacePacket.msg \
    transport/rtp/RTPPacket.msg \
    transport/rtp/RTPParticipantInfo.msg \
    transport/rtp/RTPSenderControlMessage.msg \
    transport/rtp/RTPSenderStatusMessage.msg \
    transport/rtp/profiles/avprofile/RTPMpegPacket.msg \
    transport/sctp/SCTPMessage.msg \
    transport/sctp/contract/SCTPCommand.msg \
    transport/tcp_common/TCPSegment.msg \
    transport/tcp_common/contract/TCPCommand.msg \
    transport/udp/UDPPacket.msg \
    transport/udp/contract/UDPControlInfo.msg \
    underTest/applications/httpApp/httptEventMessages.msg \
    underTest/applications/httpApp/httptMessages.msg \
    underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages.msg \
    underTest/linklayer/Ieee80216/ControlPlane/Ieee80216Primitives.msg \
    underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader.msg \
    underTest/linklayer/Ieee80216/Mac/WiMAXRadioControl.msg \
    world/powercontrol/PowerControlMessages.msg \
    wpan/applications/Ieee802154TestApp/Ieee802154AppPkt.msg \
    wpan/linklayer/bmac/bmacpkt.msg \
    wpan/linklayer/ieee802154/contract/Ieee802154MacPhyPrimitives.msg \
    wpan/linklayer/ieee802154/mac/Ieee802154Frame.msg \
    wpan/networklayer/contract/Ieee802154NetworkCtrlInfo.msg

#------------------------------------------------------------------------------

# Pull in OMNeT++ configuration (Makefile.inc or configuser.vc)

ifneq ("$(OMNETPP_CONFIGFILE)","")
CONFIGFILE = $(OMNETPP_CONFIGFILE)
else
ifneq ("$(OMNETPP_ROOT)","")
CONFIGFILE = $(OMNETPP_ROOT)/Makefile.inc
else
CONFIGFILE = $(shell opp_configfilepath)
endif
endif

ifeq ("$(wildcard $(CONFIGFILE))","")
$(error Config file '$(CONFIGFILE)' does not exist -- add the OMNeT++ bin directory to the path so that opp_configfilepath can be found, or set the OMNETPP_CONFIGFILE variable to point to Makefile.inc)
endif

include $(CONFIGFILE)

# Simulation kernel and user interface libraries
OMNETPP_LIB_SUBDIR = $(OMNETPP_LIB_DIR)/$(TOOLCHAIN_NAME)
OMNETPP_LIBS = -L"$(OMNETPP_LIB_SUBDIR)" -L"$(OMNETPP_LIB_DIR)" -loppenvir$D $(KERNEL_LIBS) $(SYS_LIBS)

COPTS = $(CFLAGS) \
    -DWITH_RSTP \
    -DWITH_TCP_COMMON \
    -DWITH_TCP_INET \
    -DWITH_IPv4 \
    -DWITH_IPv6 \
    -DWITH_xMIPv6 \
    -DWITH_UDP \
    -DWITH_RTP \
    -DWITH_SCTP \
    -DWITH_ETHERNET \
    -DWITH_PPP \
    -DWITH_EXT_IF \
    -DWITH_MPLS \
    -DWITH_OSPFv2 \
    -DWITH_BGPv4 \
    -DWITH_MANET $(INCLUDE_PATH) -I$(OMNETPP_INCL_DIR)
MSGCOPTS = $(INCLUDE_PATH)

#------------------------------------------------------------------------------
# User-supplied makefile fragment(s)
# >>>
# inserted from file 'makefrag':
# if you want to use external interfaces with inet, you should install
# winpcap (on windows) or libpcap-dev (on linux) and re-run ./configure in
# the omnetpp root directory and comment out the following line:
HAVE_PCAP=no

ifeq ($(HAVE_PCAP),yes)
  # link with PCAP libs too
  LIBS += $(PCAP_LIBS)
else
  # remove the HAVE_PCAP define if we do not need PCAP
  CFLAGS := $(filter-out -DHAVE_PCAP,$(CFLAGS))
endif

# TCP implementaion using the Network Simulation Cradle
NSC_VERSION= $(shell ls -d ../3rdparty/nsc* 2>/dev/null | sed 's/^.*-//')

ifneq ($(NSC_VERSION),)
  CFLAGS += -I../3rdparty/nsc-$(NSC_VERSION)/sim
  LIBS += -Wl,-rpath,`abspath ../3rdparty/nsc-$(NSC_VERSION)`
endif

# <<<
#------------------------------------------------------------------------------

# Main target
all: $(TARGET)

$(TARGET) : $O/$(TARGET)
	$(LN) $O/$(TARGET) .

$O/$(TARGET): $(OBJS)  $(wildcard $(EXTRA_OBJS)) Makefile
	@$(MKPATH) $O
	echo >.tmp$$$$ $(OBJS) $(EXTRA_OBJS) && $(AR) .tmplib$$$$ @.tmp$$$$ && $(SHLIB_LD) -o $O/$(TARGET) $(WHOLE_ARCHIVE_ON) .tmplib$$$$ $(LIBS) $(WHOLE_ARCHIVE_OFF) $(OMNETPP_LIBS) $(LDFLAGS) && rm .tmp$$$$ && rm .tmplib$$$$
	$(SHLIB_POSTPROCESS) $O/$(TARGET)

.PHONY:

.SUFFIXES: .cc

$O/%.o: %.cc
	@$(MKPATH) $(dir $@)
	$(CXX) -c $(COPTS) -o $@ $<

%_m.cc %_m.h: %.msg
	$(MSGC) -s _m.cc $(MSGCOPTS) $?

msgheaders: $(MSGFILES:.msg=_m.h)

clean:
	-rm -rf $O
	-rm -f inet inet.exe libinet.so libinet.a libinet.dll libinet.dylib
	-rm -f ./*_m.cc ./*_m.h
	-rm -f applications/*_m.cc applications/*_m.h
	-rm -f applications/ethernet/*_m.cc applications/ethernet/*_m.h
	-rm -f applications/generic/*_m.cc applications/generic/*_m.h
	-rm -f applications/pingapp/*_m.cc applications/pingapp/*_m.h
	-rm -f applications/rtpapp/*_m.cc applications/rtpapp/*_m.h
	-rm -f applications/sctpapp/*_m.cc applications/sctpapp/*_m.h
	-rm -f applications/tcpapp/*_m.cc applications/tcpapp/*_m.h
	-rm -f applications/traci/*_m.cc applications/traci/*_m.h
	-rm -f applications/trafgen/*_m.cc applications/trafgen/*_m.h
	-rm -f applications/udpapp/*_m.cc applications/udpapp/*_m.h
	-rm -f base/*_m.cc base/*_m.h
	-rm -f battery/*_m.cc battery/*_m.h
	-rm -f battery/models/*_m.cc battery/models/*_m.h
	-rm -f linklayer/*_m.cc linklayer/*_m.h
	-rm -f linklayer/channels/*_m.cc linklayer/channels/*_m.h
	-rm -f linklayer/contract/*_m.cc linklayer/contract/*_m.h
	-rm -f linklayer/ethernet/*_m.cc linklayer/ethernet/*_m.h
	-rm -f linklayer/ethernet/switch/*_m.cc linklayer/ethernet/switch/*_m.h
	-rm -f linklayer/ext/*_m.cc linklayer/ext/*_m.h
	-rm -f linklayer/ieee80211/*_m.cc linklayer/ieee80211/*_m.h
	-rm -f linklayer/ieee80211/mac/*_m.cc linklayer/ieee80211/mac/*_m.h
	-rm -f linklayer/ieee80211/mgmt/*_m.cc linklayer/ieee80211/mgmt/*_m.h
	-rm -f linklayer/ieee80211/radio/*_m.cc linklayer/ieee80211/radio/*_m.h
	-rm -f linklayer/ieee80211/radio/errormodel/*_m.cc linklayer/ieee80211/radio/errormodel/*_m.h
	-rm -f linklayer/ieee80211mesh/*_m.cc linklayer/ieee80211mesh/*_m.h
	-rm -f linklayer/ieee80211mesh/hwmp/*_m.cc linklayer/ieee80211mesh/hwmp/*_m.h
	-rm -f linklayer/ieee80211mesh/mgmt/*_m.cc linklayer/ieee80211mesh/mgmt/*_m.h
	-rm -f linklayer/mf80211/*_m.cc linklayer/mf80211/*_m.h
	-rm -f linklayer/mf80211/core/*_m.cc linklayer/mf80211/core/*_m.h
	-rm -f linklayer/mf80211/macLayer/*_m.cc linklayer/mf80211/macLayer/*_m.h
	-rm -f linklayer/mf80211/phyLayer/*_m.cc linklayer/mf80211/phyLayer/*_m.h
	-rm -f linklayer/mf80211/phyLayer/decider/*_m.cc linklayer/mf80211/phyLayer/decider/*_m.h
	-rm -f linklayer/mf80211/phyLayer/snrEval/*_m.cc linklayer/mf80211/phyLayer/snrEval/*_m.h
	-rm -f linklayer/ppp/*_m.cc linklayer/ppp/*_m.h
	-rm -f linklayer/radio/*_m.cc linklayer/radio/*_m.h
	-rm -f linklayer/radio/propagation/*_m.cc linklayer/radio/propagation/*_m.h
	-rm -f linklayer/rstp/*_m.cc linklayer/rstp/*_m.h
	-rm -f mobility/*_m.cc mobility/*_m.h
	-rm -f mobility/models/*_m.cc mobility/models/*_m.h
	-rm -f mobility/traci/*_m.cc mobility/traci/*_m.h
	-rm -f networklayer/*_m.cc networklayer/*_m.h
	-rm -f networklayer/arp/*_m.cc networklayer/arp/*_m.h
	-rm -f networklayer/autorouting/*_m.cc networklayer/autorouting/*_m.h
	-rm -f networklayer/autorouting/ipv4/*_m.cc networklayer/autorouting/ipv4/*_m.h
	-rm -f networklayer/autorouting/ipv6/*_m.cc networklayer/autorouting/ipv6/*_m.h
	-rm -f networklayer/bgpv4/*_m.cc networklayer/bgpv4/*_m.h
	-rm -f networklayer/bgpv4/BGPMessage/*_m.cc networklayer/bgpv4/BGPMessage/*_m.h
	-rm -f networklayer/common/*_m.cc networklayer/common/*_m.h
	-rm -f networklayer/contract/*_m.cc networklayer/contract/*_m.h
	-rm -f networklayer/extras/*_m.cc networklayer/extras/*_m.h
	-rm -f networklayer/icmpv6/*_m.cc networklayer/icmpv6/*_m.h
	-rm -f networklayer/ipv4/*_m.cc networklayer/ipv4/*_m.h
	-rm -f networklayer/ipv4/contract/*_m.cc networklayer/ipv4/contract/*_m.h
	-rm -f networklayer/ipv6/*_m.cc networklayer/ipv6/*_m.h
	-rm -f networklayer/ipv6/contract/*_m.cc networklayer/ipv6/contract/*_m.h
	-rm -f networklayer/ipv6tunneling/*_m.cc networklayer/ipv6tunneling/*_m.h
	-rm -f networklayer/ldp/*_m.cc networklayer/ldp/*_m.h
	-rm -f networklayer/manetrouting/*_m.cc networklayer/manetrouting/*_m.h
	-rm -f networklayer/manetrouting/aodv/*_m.cc networklayer/manetrouting/aodv/*_m.h
	-rm -f networklayer/manetrouting/aodv/aodv-uu/*_m.cc networklayer/manetrouting/aodv/aodv-uu/*_m.h
	-rm -f networklayer/manetrouting/base/*_m.cc networklayer/manetrouting/base/*_m.h
	-rm -f networklayer/manetrouting/batman/*_m.cc networklayer/manetrouting/batman/*_m.h
	-rm -f networklayer/manetrouting/dsdv/*_m.cc networklayer/manetrouting/dsdv/*_m.h
	-rm -f networklayer/manetrouting/dsr/*_m.cc networklayer/manetrouting/dsr/*_m.h
	-rm -f networklayer/manetrouting/dsr/dsr-uu/*_m.cc networklayer/manetrouting/dsr/dsr-uu/*_m.h
	-rm -f networklayer/manetrouting/dymo/*_m.cc networklayer/manetrouting/dymo/*_m.h
	-rm -f networklayer/manetrouting/dymo/dymoum/*_m.cc networklayer/manetrouting/dymo/dymoum/*_m.h
	-rm -f networklayer/manetrouting/dymo_fau/*_m.cc networklayer/manetrouting/dymo_fau/*_m.h
	-rm -f networklayer/manetrouting/olsr/*_m.cc networklayer/manetrouting/olsr/*_m.h
	-rm -f networklayer/mpls/*_m.cc networklayer/mpls/*_m.h
	-rm -f networklayer/ospfv2/*_m.cc networklayer/ospfv2/*_m.h
	-rm -f networklayer/ospfv2/interface/*_m.cc networklayer/ospfv2/interface/*_m.h
	-rm -f networklayer/ospfv2/messagehandler/*_m.cc networklayer/ospfv2/messagehandler/*_m.h
	-rm -f networklayer/ospfv2/neighbor/*_m.cc networklayer/ospfv2/neighbor/*_m.h
	-rm -f networklayer/ospfv2/router/*_m.cc networklayer/ospfv2/router/*_m.h
	-rm -f networklayer/queue/*_m.cc networklayer/queue/*_m.h
	-rm -f networklayer/rsvp_te/*_m.cc networklayer/rsvp_te/*_m.h
	-rm -f networklayer/ted/*_m.cc networklayer/ted/*_m.h
	-rm -f networklayer/xmipv6/*_m.cc networklayer/xmipv6/*_m.h
	-rm -f nodes/*_m.cc nodes/*_m.h
	-rm -f nodes/bgp/*_m.cc nodes/bgp/*_m.h
	-rm -f nodes/ethernet/*_m.cc nodes/ethernet/*_m.h
	-rm -f nodes/inet/*_m.cc nodes/inet/*_m.h
	-rm -f nodes/ipv6/*_m.cc nodes/ipv6/*_m.h
	-rm -f nodes/mf80211/*_m.cc nodes/mf80211/*_m.h
	-rm -f nodes/mpls/*_m.cc nodes/mpls/*_m.h
	-rm -f nodes/ospfv2/*_m.cc nodes/ospfv2/*_m.h
	-rm -f nodes/wireless/*_m.cc nodes/wireless/*_m.h
	-rm -f nodes/xmipv6/*_m.cc nodes/xmipv6/*_m.h
	-rm -f transport/*_m.cc transport/*_m.h
	-rm -f transport/rtp/*_m.cc transport/rtp/*_m.h
	-rm -f transport/rtp/profiles/*_m.cc transport/rtp/profiles/*_m.h
	-rm -f transport/rtp/profiles/avprofile/*_m.cc transport/rtp/profiles/avprofile/*_m.h
	-rm -f transport/sctp/*_m.cc transport/sctp/*_m.h
	-rm -f transport/sctp/contract/*_m.cc transport/sctp/contract/*_m.h
	-rm -f transport/tcp/*_m.cc transport/tcp/*_m.h
	-rm -f transport/tcp/flavours/*_m.cc transport/tcp/flavours/*_m.h
	-rm -f transport/tcp/queues/*_m.cc transport/tcp/queues/*_m.h
	-rm -f transport/tcp_common/*_m.cc transport/tcp_common/*_m.h
	-rm -f transport/tcp_common/contract/*_m.cc transport/tcp_common/contract/*_m.h
	-rm -f transport/udp/*_m.cc transport/udp/*_m.h
	-rm -f transport/udp/contract/*_m.cc transport/udp/contract/*_m.h
	-rm -f underTest/*_m.cc underTest/*_m.h
	-rm -f underTest/applications/*_m.cc underTest/applications/*_m.h
	-rm -f underTest/applications/Ieee802154TestApp/*_m.cc underTest/applications/Ieee802154TestApp/*_m.h
	-rm -f underTest/applications/TrafficGenerator_Erlangen/*_m.cc underTest/applications/TrafficGenerator_Erlangen/*_m.h
	-rm -f underTest/applications/httpApp/*_m.cc underTest/applications/httpApp/*_m.h
	-rm -f underTest/linklayer/*_m.cc underTest/linklayer/*_m.h
	-rm -f underTest/linklayer/Ieee80216/*_m.cc underTest/linklayer/Ieee80216/*_m.h
	-rm -f underTest/linklayer/Ieee80216/ControlPlane/*_m.cc underTest/linklayer/Ieee80216/ControlPlane/*_m.h
	-rm -f underTest/linklayer/Ieee80216/MACSublayer/*_m.cc underTest/linklayer/Ieee80216/MACSublayer/*_m.h
	-rm -f underTest/linklayer/Ieee80216/MACSublayer/QoS/*_m.cc underTest/linklayer/Ieee80216/MACSublayer/QoS/*_m.h
	-rm -f underTest/linklayer/Ieee80216/MACSublayer/Receiver/*_m.cc underTest/linklayer/Ieee80216/MACSublayer/Receiver/*_m.h
	-rm -f underTest/linklayer/Ieee80216/MACSublayer/Transceiver/*_m.cc underTest/linklayer/Ieee80216/MACSublayer/Transceiver/*_m.h
	-rm -f underTest/linklayer/Ieee80216/Mac/*_m.cc underTest/linklayer/Ieee80216/Mac/*_m.h
	-rm -f underTest/linklayer/Ieee80216/phyLayer/*_m.cc underTest/linklayer/Ieee80216/phyLayer/*_m.h
	-rm -f underTest/linklayer/Ieee80216/phyLayer/decider/*_m.cc underTest/linklayer/Ieee80216/phyLayer/decider/*_m.h
	-rm -f underTest/linklayer/Ieee80216/phyLayer/snrEval/*_m.cc underTest/linklayer/Ieee80216/phyLayer/snrEval/*_m.h
	-rm -f underTest/linklayer/ieee80211/*_m.cc underTest/linklayer/ieee80211/*_m.h
	-rm -f underTest/linklayer/ieee80211/mgmt/*_m.cc underTest/linklayer/ieee80211/mgmt/*_m.h
	-rm -f underTest/linklayer/radio/*_m.cc underTest/linklayer/radio/*_m.h
	-rm -f underTest/nodes/*_m.cc underTest/nodes/*_m.h
	-rm -f underTest/nodes/WiMAX/*_m.cc underTest/nodes/WiMAX/*_m.h
	-rm -f underTest/nodes/wireless/*_m.cc underTest/nodes/wireless/*_m.h
	-rm -f util/*_m.cc util/*_m.h
	-rm -f util/headerserializers/*_m.cc util/headerserializers/*_m.h
	-rm -f util/headerserializers/headers/*_m.cc util/headerserializers/headers/*_m.h
	-rm -f util/headerserializers/ipv4/*_m.cc util/headerserializers/ipv4/*_m.h
	-rm -f util/headerserializers/ipv4/headers/*_m.cc util/headerserializers/ipv4/headers/*_m.h
	-rm -f util/headerserializers/sctp/*_m.cc util/headerserializers/sctp/*_m.h
	-rm -f util/headerserializers/sctp/headers/*_m.cc util/headerserializers/sctp/headers/*_m.h
	-rm -f util/headerserializers/tcp/*_m.cc util/headerserializers/tcp/*_m.h
	-rm -f util/headerserializers/tcp/headers/*_m.cc util/headerserializers/tcp/headers/*_m.h
	-rm -f util/headerserializers/udp/*_m.cc util/headerserializers/udp/*_m.h
	-rm -f util/headerserializers/udp/headers/*_m.cc util/headerserializers/udp/headers/*_m.h
	-rm -f world/*_m.cc world/*_m.h
	-rm -f world/annotations/*_m.cc world/annotations/*_m.h
	-rm -f world/obstacles/*_m.cc world/obstacles/*_m.h
	-rm -f world/powercontrol/*_m.cc world/powercontrol/*_m.h
	-rm -f world/radio/*_m.cc world/radio/*_m.h
	-rm -f world/scenario/*_m.cc world/scenario/*_m.h
	-rm -f world/traci/*_m.cc world/traci/*_m.h
	-rm -f wpan/*_m.cc wpan/*_m.h
	-rm -f wpan/Isabel/*_m.cc wpan/Isabel/*_m.h
	-rm -f wpan/Isabel/Analysis/*_m.cc wpan/Isabel/Analysis/*_m.h
	-rm -f wpan/Isabel/Base/*_m.cc wpan/Isabel/Base/*_m.h
	-rm -f wpan/applications/*_m.cc wpan/applications/*_m.h
	-rm -f wpan/applications/Ieee802154TestApp/*_m.cc wpan/applications/Ieee802154TestApp/*_m.h
	-rm -f wpan/linklayer/*_m.cc wpan/linklayer/*_m.h
	-rm -f wpan/linklayer/bmac/*_m.cc wpan/linklayer/bmac/*_m.h
	-rm -f wpan/linklayer/ieee802154/*_m.cc wpan/linklayer/ieee802154/*_m.h
	-rm -f wpan/linklayer/ieee802154/contract/*_m.cc wpan/linklayer/ieee802154/contract/*_m.h
	-rm -f wpan/linklayer/ieee802154/mac/*_m.cc wpan/linklayer/ieee802154/mac/*_m.h
	-rm -f wpan/linklayer/ieee802154/phyLayer/*_m.cc wpan/linklayer/ieee802154/phyLayer/*_m.h
	-rm -f wpan/networklayer/*_m.cc wpan/networklayer/*_m.h
	-rm -f wpan/networklayer/Ieee802154StarRouting/*_m.cc wpan/networklayer/Ieee802154StarRouting/*_m.h
	-rm -f wpan/networklayer/contract/*_m.cc wpan/networklayer/contract/*_m.h
	-rm -f wpan/nodes/*_m.cc wpan/nodes/*_m.h

cleanall: clean
	-rm -rf $(PROJECT_OUTPUT_DIR)

depend:
	$(MAKEDEPEND) $(INCLUDE_PATH) -f Makefile -P\$$O/ -- $(MSG_CC_FILES)  ./*.cc applications/*.cc applications/ethernet/*.cc applications/generic/*.cc applications/pingapp/*.cc applications/rtpapp/*.cc applications/sctpapp/*.cc applications/tcpapp/*.cc applications/traci/*.cc applications/trafgen/*.cc applications/udpapp/*.cc base/*.cc battery/*.cc battery/models/*.cc linklayer/*.cc linklayer/channels/*.cc linklayer/contract/*.cc linklayer/ethernet/*.cc linklayer/ethernet/switch/*.cc linklayer/ext/*.cc linklayer/ieee80211/*.cc linklayer/ieee80211/mac/*.cc linklayer/ieee80211/mgmt/*.cc linklayer/ieee80211/radio/*.cc linklayer/ieee80211/radio/errormodel/*.cc linklayer/ieee80211mesh/*.cc linklayer/ieee80211mesh/hwmp/*.cc linklayer/ieee80211mesh/mgmt/*.cc linklayer/mf80211/*.cc linklayer/mf80211/core/*.cc linklayer/mf80211/macLayer/*.cc linklayer/mf80211/phyLayer/*.cc linklayer/mf80211/phyLayer/decider/*.cc linklayer/mf80211/phyLayer/snrEval/*.cc linklayer/ppp/*.cc linklayer/radio/*.cc linklayer/radio/propagation/*.cc linklayer/rstp/*.cc mobility/*.cc mobility/models/*.cc mobility/traci/*.cc networklayer/*.cc networklayer/arp/*.cc networklayer/autorouting/*.cc networklayer/autorouting/ipv4/*.cc networklayer/autorouting/ipv6/*.cc networklayer/bgpv4/*.cc networklayer/bgpv4/BGPMessage/*.cc networklayer/common/*.cc networklayer/contract/*.cc networklayer/extras/*.cc networklayer/icmpv6/*.cc networklayer/ipv4/*.cc networklayer/ipv4/contract/*.cc networklayer/ipv6/*.cc networklayer/ipv6/contract/*.cc networklayer/ipv6tunneling/*.cc networklayer/ldp/*.cc networklayer/manetrouting/*.cc networklayer/manetrouting/aodv/*.cc networklayer/manetrouting/aodv/aodv-uu/*.cc networklayer/manetrouting/base/*.cc networklayer/manetrouting/batman/*.cc networklayer/manetrouting/dsdv/*.cc networklayer/manetrouting/dsr/*.cc networklayer/manetrouting/dsr/dsr-uu/*.cc networklayer/manetrouting/dymo/*.cc networklayer/manetrouting/dymo/dymoum/*.cc networklayer/manetrouting/dymo_fau/*.cc networklayer/manetrouting/olsr/*.cc networklayer/mpls/*.cc networklayer/ospfv2/*.cc networklayer/ospfv2/interface/*.cc networklayer/ospfv2/messagehandler/*.cc networklayer/ospfv2/neighbor/*.cc networklayer/ospfv2/router/*.cc networklayer/queue/*.cc networklayer/rsvp_te/*.cc networklayer/ted/*.cc networklayer/xmipv6/*.cc nodes/*.cc nodes/bgp/*.cc nodes/ethernet/*.cc nodes/inet/*.cc nodes/ipv6/*.cc nodes/mf80211/*.cc nodes/mpls/*.cc nodes/ospfv2/*.cc nodes/wireless/*.cc nodes/xmipv6/*.cc transport/*.cc transport/rtp/*.cc transport/rtp/profiles/*.cc transport/rtp/profiles/avprofile/*.cc transport/sctp/*.cc transport/sctp/contract/*.cc transport/tcp/*.cc transport/tcp/flavours/*.cc transport/tcp/queues/*.cc transport/tcp_common/*.cc transport/tcp_common/contract/*.cc transport/udp/*.cc transport/udp/contract/*.cc underTest/*.cc underTest/applications/*.cc underTest/applications/Ieee802154TestApp/*.cc underTest/applications/TrafficGenerator_Erlangen/*.cc underTest/applications/httpApp/*.cc underTest/linklayer/*.cc underTest/linklayer/Ieee80216/*.cc underTest/linklayer/Ieee80216/ControlPlane/*.cc underTest/linklayer/Ieee80216/MACSublayer/*.cc underTest/linklayer/Ieee80216/MACSublayer/QoS/*.cc underTest/linklayer/Ieee80216/MACSublayer/Receiver/*.cc underTest/linklayer/Ieee80216/MACSublayer/Transceiver/*.cc underTest/linklayer/Ieee80216/Mac/*.cc underTest/linklayer/Ieee80216/phyLayer/*.cc underTest/linklayer/Ieee80216/phyLayer/decider/*.cc underTest/linklayer/Ieee80216/phyLayer/snrEval/*.cc underTest/linklayer/ieee80211/*.cc underTest/linklayer/ieee80211/mgmt/*.cc underTest/linklayer/radio/*.cc underTest/nodes/*.cc underTest/nodes/WiMAX/*.cc underTest/nodes/wireless/*.cc util/*.cc util/headerserializers/*.cc util/headerserializers/headers/*.cc util/headerserializers/ipv4/*.cc util/headerserializers/ipv4/headers/*.cc util/headerserializers/sctp/*.cc util/headerserializers/sctp/headers/*.cc util/headerserializers/tcp/*.cc util/headerserializers/tcp/headers/*.cc util/headerserializers/udp/*.cc util/headerserializers/udp/headers/*.cc world/*.cc world/annotations/*.cc world/obstacles/*.cc world/powercontrol/*.cc world/radio/*.cc world/scenario/*.cc world/traci/*.cc wpan/*.cc wpan/Isabel/*.cc wpan/Isabel/Analysis/*.cc wpan/Isabel/Base/*.cc wpan/applications/*.cc wpan/applications/Ieee802154TestApp/*.cc wpan/linklayer/*.cc wpan/linklayer/bmac/*.cc wpan/linklayer/ieee802154/*.cc wpan/linklayer/ieee802154/contract/*.cc wpan/linklayer/ieee802154/mac/*.cc wpan/linklayer/ieee802154/phyLayer/*.cc wpan/networklayer/*.cc wpan/networklayer/Ieee802154StarRouting/*.cc wpan/networklayer/contract/*.cc wpan/nodes/*.cc

# DO NOT DELETE THIS LINE -- make depend depends on it.
$O/applications/ethernet/EtherAppCli.o: applications/ethernet/EtherAppCli.cc \
	base/INETDefs.h \
	applications/ethernet/EtherApp_m.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	applications/ethernet/EtherAppCli.h
$O/applications/ethernet/EtherApp_m.o: applications/ethernet/EtherApp_m.cc \
	base/INETDefs.h \
	applications/ethernet/EtherApp_m.h
$O/applications/ethernet/EtherAppSrv.o: applications/ethernet/EtherAppSrv.cc \
	applications/ethernet/EtherAppSrv.h \
	base/INETDefs.h \
	applications/ethernet/EtherApp_m.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h
$O/applications/generic/IPvXTrafGen.o: applications/generic/IPvXTrafGen.cc \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPvXAddressResolver.h \
	applications/generic/IPvXTrafGen.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	linklayer/contract/MACAddress.h \
	networklayer/contract/IPProtocolId_m.h
$O/applications/pingapp/PingApp.o: applications/pingapp/PingApp.cc \
	base/INETDefs.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	networklayer/contract/IPv4Address.h \
	applications/pingapp/PingApp.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	applications/pingapp/PingPayload_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPProtocolId_m.h
$O/applications/pingapp/PingPayload_m.o: applications/pingapp/PingPayload_m.cc \
	applications/pingapp/PingPayload_m.h \
	base/INETDefs.h
$O/applications/rtpapp/RTPApplication.o: applications/rtpapp/RTPApplication.cc \
	networklayer/contract/IPvXAddress.h \
	transport/rtp/RTPSenderStatusMessage_m.h \
	base/INETDefs.h \
	applications/rtpapp/RTPApplication.h \
	transport/rtp/RTPInterfacePacket_m.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h \
	transport/rtp/RTPSenderControlMessage_m.h
$O/applications/sctpapp/SCTPClient.o: applications/sctpapp/SCTPClient.cc \
	transport/sctp/SCTPMessage.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/contract/SCTPCommand.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPQueue.h \
	transport/sctp/contract/SCTPSocket.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h \
	transport/sctp/SCTPReceiveStream.h \
	networklayer/common/InterfaceTable.h \
	applications/sctpapp/SCTPClient.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	transport/sctp/contract/SCTPCommand_m.h \
	transport/sctp/SCTPSendStream.h \
	networklayer/contract/IPv6Address.h
$O/applications/sctpapp/SCTPServer.o: applications/sctpapp/SCTPServer.cc \
	transport/sctp/SCTPMessage.h \
	applications/sctpapp/SCTPServer.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/contract/SCTPCommand.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPQueue.h \
	transport/sctp/contract/SCTPSocket.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h \
	transport/sctp/SCTPReceiveStream.h \
	networklayer/common/InterfaceTable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	transport/sctp/contract/SCTPCommand_m.h \
	transport/sctp/SCTPSendStream.h \
	networklayer/contract/IPv6Address.h
$O/applications/sctpapp/SCTPPeer.o: applications/sctpapp/SCTPPeer.cc \
	transport/sctp/SCTPMessage.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/contract/SCTPCommand.h \
	applications/sctpapp/SCTPPeer.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPQueue.h \
	transport/sctp/contract/SCTPSocket.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/common/InterfaceEntry.h \
	transport/sctp/SCTPReceiveStream.h \
	networklayer/common/InterfaceTable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	transport/sctp/contract/SCTPCommand_m.h \
	transport/sctp/SCTPSendStream.h \
	networklayer/contract/IPv6Address.h
$O/applications/tcpapp/TCPGenericSrvThread.o: applications/tcpapp/TCPGenericSrvThread.cc \
	transport/tcp_common/contract/TCPSocket.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	applications/tcpapp/GenericAppMsg_m.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	applications/tcpapp/TCPGenericSrvThread.h \
	transport/tcp_common/contract/TCPSocketMap.h \
	applications/tcpapp/TCPSrvHostApp.h
$O/applications/tcpapp/TCPSessionApp.o: applications/tcpapp/TCPSessionApp.cc \
	base/ByteArray_m.h \
	base/ByteArray.h \
	applications/tcpapp/TCPSessionApp.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	base/ByteArrayMessage_m.h \
	transport/tcp_common/contract/TCPSocket.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	applications/tcpapp/GenericAppMsg_m.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/contract/IPvXAddressResolver.h \
	base/ByteArrayMessage.h \
	networklayer/contract/IPv6Address.h
$O/applications/tcpapp/TCPEchoApp.o: applications/tcpapp/TCPEchoApp.cc \
	base/ByteArrayMessage_m.h \
	transport/tcp_common/contract/TCPSocket.h \
	applications/tcpapp/TCPEchoApp.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	base/ByteArrayMessage.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h
$O/applications/tcpapp/GenericAppMsg_m.o: applications/tcpapp/GenericAppMsg_m.cc \
	applications/tcpapp/GenericAppMsg_m.h \
	base/INETDefs.h
$O/applications/tcpapp/TCPGenericSrvApp.o: applications/tcpapp/TCPGenericSrvApp.cc \
	transport/tcp_common/contract/TCPSocket.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	applications/tcpapp/GenericAppMsg_m.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	applications/tcpapp/TCPGenericSrvApp.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h
$O/applications/tcpapp/TelnetApp.o: applications/tcpapp/TelnetApp.cc \
	transport/tcp_common/contract/TCPSocket.h \
	applications/tcpapp/TCPGenericCliAppBase.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	applications/tcpapp/TelnetApp.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h
$O/applications/tcpapp/TCPGenericCliAppBase.o: applications/tcpapp/TCPGenericCliAppBase.cc \
	transport/tcp_common/contract/TCPSocket.h \
	applications/tcpapp/TCPGenericCliAppBase.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	applications/tcpapp/GenericAppMsg_m.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h
$O/applications/tcpapp/TCPSinkApp.o: applications/tcpapp/TCPSinkApp.cc \
	applications/tcpapp/TCPSinkApp.h \
	transport/tcp_common/contract/TCPSocket.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h
$O/applications/tcpapp/TCPBasicClientApp.o: applications/tcpapp/TCPBasicClientApp.cc \
	transport/tcp_common/contract/TCPSocket.h \
	applications/tcpapp/TCPGenericCliAppBase.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	applications/tcpapp/TCPBasicClientApp.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h
$O/applications/tcpapp/TCPSrvHostApp.o: applications/tcpapp/TCPSrvHostApp.cc \
	transport/tcp_common/contract/TCPSocket.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	transport/tcp_common/contract/TCPSocketMap.h \
	applications/tcpapp/TCPSrvHostApp.h
$O/applications/traci/Test_TraCI.o: applications/traci/Test_TraCI.cc \
	util/FWMath.h \
	applications/traci/Test_TraCI.h \
	base/INETDefs.h \
	base/INotifiable.h \
	mobility/traci/TraCIMobility.h \
	base/Coord.h \
	base/NotifierConsts.h \
	world/traci/TraCIScenarioManager.h \
	world/radio/IChannelControl.h \
	mobility/models/BasicMobility.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/applications/trafgen/TrafGenPar.o: applications/trafgen/TrafGenPar.cc \
	applications/trafgen/TrafGenPar.h
$O/applications/trafgen/TrafGen.o: applications/trafgen/TrafGen.cc \
	applications/trafgen/TrafGen.h
$O/applications/udpapp/UDPVideoStreamCli.o: applications/udpapp/UDPVideoStreamCli.cc \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPvXAddressResolver.h \
	applications/udpapp/UDPVideoStreamCli.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h \
	applications/udpapp/UDPAppBase.h
$O/applications/udpapp/UDPEchoApp.o: applications/udpapp/UDPEchoApp.cc \
	networklayer/contract/IPvXAddress.h \
	applications/udpapp/UDPBasicApp.h \
	base/INETDefs.h \
	applications/udpapp/UDPEchoAppMsg_m.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h \
	transport/udp/contract/UDPControlInfo_m.h \
	applications/udpapp/UDPAppBase.h \
	applications/udpapp/UDPEchoApp.h
$O/applications/udpapp/UDPSink.o: applications/udpapp/UDPSink.cc \
	base/INETDefs.h \
	applications/udpapp/UDPSink.h \
	applications/udpapp/UDPAppBase.h
$O/applications/udpapp/UDPEchoAppMsg_m.o: applications/udpapp/UDPEchoAppMsg_m.cc \
	base/INETDefs.h \
	applications/udpapp/UDPEchoAppMsg_m.h
$O/applications/udpapp/UDPBasicApp.o: applications/udpapp/UDPBasicApp.cc \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	applications/udpapp/UDPBasicApp.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	applications/udpapp/UDPAppBase.h
$O/applications/udpapp/UDPVideoStreamSvr.o: applications/udpapp/UDPVideoStreamSvr.cc \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	applications/udpapp/UDPVideoStreamSvr.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	transport/udp/contract/UDPControlInfo_m.h \
	applications/udpapp/UDPAppBase.h
$O/applications/udpapp/UDPAppBase.o: applications/udpapp/UDPAppBase.cc \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	transport/udp/contract/UDPSocket.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	transport/udp/contract/UDPControlInfo_m.h \
	applications/udpapp/UDPAppBase.h
$O/applications/udpapp/UDPBasicBurst.o: applications/udpapp/UDPBasicBurst.cc \
	base/MMapBoard.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h \
	transport/udp/contract/UDPControlInfo_m.h \
	applications/udpapp/UDPBasicBurst.h \
	base/ModuleAccess.h \
	applications/udpapp/UDPAppBase.h
$O/base/UnimplementedModule.o: base/UnimplementedModule.cc \
	base/UnimplementedModule.h
$O/base/ByteArrayMessage.o: base/ByteArrayMessage.cc \
	base/ByteArrayMessage_m.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	base/INETDefs.h \
	base/ByteArrayMessage.h
$O/base/ReassemblyBuffer.o: base/ReassemblyBuffer.cc \
	base/INETDefs.h \
	base/ReassemblyBuffer.h
$O/base/ByteArray_m.o: base/ByteArray_m.cc \
	base/ByteArray_m.h \
	base/INETDefs.h
$O/base/Join.o: base/Join.cc \
	base/INETDefs.h
$O/base/Sink.o: base/Sink.cc \
	base/INETDefs.h \
	base/Sink.h
$O/base/AbstractQueue.o: base/AbstractQueue.cc \
	base/AbstractQueue.h \
	base/INETDefs.h
$O/base/QueueWithQoS.o: base/QueueWithQoS.cc \
	base/AbstractQueue.h \
	base/INETDefs.h \
	base/QueueWithQoS.h \
	base/EnqueueHook.h
$O/base/PassiveQueueBase.o: base/PassiveQueueBase.cc \
	base/IPassiveQueue.h \
	base/INETDefs.h \
	base/PassiveQueueBase.h
$O/base/NotificationBoard.o: base/NotificationBoard.cc \
	base/INotifiable.h \
	base/INETDefs.h \
	base/ModuleAccess.h \
	base/NotifierConsts.h \
	base/NotificationBoard.h
$O/base/ProtocolMap.o: base/ProtocolMap.cc \
	base/ProtocolMap.h \
	base/INETDefs.h
$O/base/MMapBoard.o: base/MMapBoard.cc \
	base/MMapBoard.h \
	base/INETDefs.h \
	base/ModuleAccess.h
$O/base/ModuleAccess.o: base/ModuleAccess.cc \
	base/INETDefs.h \
	base/ModuleAccess.h
$O/base/ByteArray.o: base/ByteArray.cc \
	base/ByteArray_m.h \
	base/ByteArray.h \
	base/INETDefs.h
$O/base/NedFunctions.o: base/NedFunctions.cc \
	base/INETDefs.h
$O/base/ByteArrayBuffer.o: base/ByteArrayBuffer.cc \
	base/ByteArray_m.h \
	base/ByteArray.h \
	base/ByteArrayBuffer.h \
	base/INETDefs.h
$O/base/ByteArrayMessage_m.o: base/ByteArrayMessage_m.cc \
	base/ByteArrayMessage_m.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	base/INETDefs.h
$O/base/QueueBase.o: base/QueueBase.cc \
	base/AbstractQueue.h \
	base/INETDefs.h \
	base/QueueBase.h
$O/base/BasicModule.o: base/BasicModule.cc \
	base/INotifiable.h \
	base/INETDefs.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	base/NotifierConsts.h \
	base/BasicModule.h
$O/base/Blackboard.o: base/Blackboard.cc \
	base/INETDefs.h \
	base/Blackboard.h \
	base/ModuleAccess.h
$O/base/NotifierConsts.o: base/NotifierConsts.cc \
	base/INETDefs.h \
	base/NotifierConsts.h
$O/battery/models/InetSimpleBattery.o: battery/models/InetSimpleBattery.cc \
	battery/models/BasicBattery.h \
	linklayer/contract/RadioState.h \
	base/INotifiable.h \
	base/INETDefs.h \
	base/NotifierConsts.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	battery/models/Energy.h \
	battery/models/InetSimpleBattery.h
$O/battery/models/BasicBattery.o: battery/models/BasicBattery.cc \
	battery/models/BasicBattery.h \
	base/INotifiable.h \
	base/INETDefs.h \
	base/NotifierConsts.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h
$O/linklayer/channels/LinkFailureMessage_m.o: linklayer/channels/LinkFailureMessage_m.cc \
	linklayer/channels/LinkFailureManager.h \
	linklayer/channels/LinkFailureMessage_m.h
$O/linklayer/channels/LinkFailureManager.o: linklayer/channels/LinkFailureManager.cc \
	linklayer/channels/LinkFailureManager.h \
	linklayer/channels/ProgramedFailureChannel.h \
	linklayer/channels/LinkFailureMessage_m.h
$O/linklayer/channels/ProgramedFailureChannel.o: linklayer/channels/ProgramedFailureChannel.cc \
	linklayer/channels/LinkFailureManager.h \
	linklayer/channels/ProgramedFailureChannel.h
$O/linklayer/channels/MyThroughputMeteringChannel.o: linklayer/channels/MyThroughputMeteringChannel.cc \
	linklayer/channels/MyThroughputMeteringChannel.h
$O/linklayer/contract/MACAddress.o: linklayer/contract/MACAddress.cc \
	networklayer/common/InterfaceToken.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h
$O/linklayer/contract/Ieee802Ctrl_m.o: linklayer/contract/Ieee802Ctrl_m.cc \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h
$O/linklayer/contract/PhyControlInfo_m.o: linklayer/contract/PhyControlInfo_m.cc \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h
$O/linklayer/ethernet/EtherFrame_m.o: linklayer/ethernet/EtherFrame_m.cc \
	base/INETDefs.h \
	linklayer/ethernet/Ethernet.h \
	linklayer/contract/MACAddress.h \
	linklayer/ethernet/EtherFrame_m.h \
	linklayer/contract/Ieee802Ctrl_m.h
$O/linklayer/ethernet/EtherMACFullDuplex.o: linklayer/ethernet/EtherMACFullDuplex.cc \
	base/IPassiveQueue.h \
	linklayer/contract/TxNotifDetails.h \
	linklayer/ethernet/EtherMACFullDuplex.h \
	base/INETDefs.h \
	base/INotifiable.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ethernet/Ethernet.h \
	base/NotificationBoard.h \
	linklayer/ethernet/EtherMACBase.h \
	base/ModuleAccess.h \
	linklayer/ethernet/EtherFrame_m.h
$O/linklayer/ethernet/EtherMAC.o: linklayer/ethernet/EtherMAC.cc \
	base/IPassiveQueue.h \
	linklayer/contract/TxNotifDetails.h \
	linklayer/ethernet/EtherMAC.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/ethernet/Ethernet.h \
	linklayer/ethernet/EtherMACBase.h \
	linklayer/contract/MACAddress.h \
	linklayer/ethernet/EtherFrame_m.h \
	linklayer/contract/Ieee802Ctrl_m.h
$O/linklayer/ethernet/EtherHub.o: linklayer/ethernet/EtherHub.cc \
	base/INETDefs.h \
	linklayer/ethernet/EtherHub.h
$O/linklayer/ethernet/EtherLLC.o: linklayer/ethernet/EtherLLC.cc \
	linklayer/ethernet/EtherLLC.h \
	base/INETDefs.h \
	linklayer/ethernet/Ethernet.h \
	linklayer/contract/MACAddress.h \
	linklayer/ethernet/EtherFrame_m.h \
	linklayer/contract/Ieee802Ctrl_m.h
$O/linklayer/ethernet/EtherEncap.o: linklayer/ethernet/EtherEncap.cc \
	linklayer/ethernet/EtherEncap.h \
	networklayer/common/InterfaceToken.h \
	base/INETDefs.h \
	networklayer/common/IInterfaceTable.h \
	linklayer/ethernet/Ethernet.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/ethernet/EtherFrame_m.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/common/InterfaceEntry.h
$O/linklayer/ethernet/EtherMACBase.o: linklayer/ethernet/EtherMACBase.cc \
	base/IPassiveQueue.h \
	linklayer/contract/TxNotifDetails.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/common/InterfaceTableAccess.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/common/InterfaceToken.h \
	util/opp_utils.h \
	networklayer/common/IInterfaceTable.h \
	linklayer/ethernet/Ethernet.h \
	linklayer/ethernet/EtherMACBase.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	linklayer/ethernet/EtherFrame_m.h \
	networklayer/common/InterfaceEntry.h
$O/linklayer/ethernet/EtherBus.o: linklayer/ethernet/EtherBus.cc \
	linklayer/ethernet/EtherBus.h \
	base/INETDefs.h
$O/linklayer/ethernet/switch/MACRelayUnitNP.o: linklayer/ethernet/switch/MACRelayUnitNP.cc \
	linklayer/ethernet/switch/MACRelayUnitNP.h \
	linklayer/ethernet/switch/MACRelayUnitBase.h \
	base/INETDefs.h \
	linklayer/ethernet/Ethernet.h \
	linklayer/contract/MACAddress.h \
	linklayer/ethernet/EtherFrame_m.h \
	linklayer/contract/Ieee802Ctrl_m.h
$O/linklayer/ethernet/switch/MACRelayUnitBase.o: linklayer/ethernet/switch/MACRelayUnitBase.cc \
	linklayer/ethernet/switch/MACRelayUnitBase.h \
	base/INETDefs.h \
	linklayer/ethernet/Ethernet.h \
	linklayer/ethernet/EtherFrame_m.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h
$O/linklayer/ethernet/switch/MACRelayUnitPP.o: linklayer/ethernet/switch/MACRelayUnitPP.cc \
	linklayer/ethernet/switch/MACRelayUnitPP.h \
	linklayer/ethernet/switch/MACRelayUnitBase.h \
	base/INETDefs.h \
	linklayer/ethernet/Ethernet.h \
	linklayer/contract/MACAddress.h \
	linklayer/ethernet/EtherFrame_m.h \
	linklayer/contract/Ieee802Ctrl_m.h
$O/linklayer/ext/ExtInterface.o: linklayer/ext/ExtInterface.cc \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/common/InterfaceTable.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/ext/ExtFrame_m.h \
	linklayer/ext/cSocketRTScheduler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	util/headerserializers/headers/bsdint.h \
	networklayer/ipv4/IPv4Datagram.h \
	util/headerserializers/ipv4/IPv4Serializer.h \
	networklayer/common/InterfaceToken.h \
	util/opp_utils.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	linklayer/ext/ExtInterface.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h
$O/linklayer/ext/ExtFrame_m.o: linklayer/ext/ExtFrame_m.cc \
	linklayer/ext/ExtFrame_m.h
$O/linklayer/ext/cSocketRTScheduler.o: linklayer/ext/cSocketRTScheduler.cc \
	base/INETDefs.h \
	linklayer/ext/ExtFrame_m.h \
	linklayer/ext/cSocketRTScheduler.h \
	util/headerserializers/headers/ethernet.h \
	util/headerserializers/headers/bsdint.h
$O/linklayer/ieee80211/mac/Ieee80211Frame_m.o: linklayer/ieee80211/mac/Ieee80211Frame_m.cc \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	base/INETDefs.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h
$O/linklayer/ieee80211/mac/Ieee80211Mac.o: linklayer/ieee80211/mac/Ieee80211Mac.cc \
	base/IPassiveQueue.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	linklayer/contract/PhyControlInfo_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/common/InterfaceTableAccess.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/contract/RadioState.h \
	networklayer/common/InterfaceToken.h \
	linklayer/ieee80211/mac/WirelessMacBase.h \
	util/opp_utils.h \
	networklayer/common/IInterfaceTable.h \
	base/FSMA.h \
	linklayer/ieee80211/mac/Ieee80211Mac.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h
$O/linklayer/ieee80211/mac/WirelessMacBase.o: linklayer/ieee80211/mac/WirelessMacBase.cc \
	linklayer/ieee80211/mac/WirelessMacBase.h \
	base/INotifiable.h \
	base/INETDefs.h \
	base/ModuleAccess.h \
	base/NotifierConsts.h \
	base/NotificationBoard.h
$O/linklayer/ieee80211/mac/Ieee80211NewMac.o: linklayer/ieee80211/mac/Ieee80211NewMac.cc \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/ieee80211/mac/Ieee80211DataRate.h \
	linklayer/radio/AirFrame_m.h \
	linklayer/contract/MACAddress.h \
	linklayer/ieee80211/mgmt/Ieee80211eClassifier.h \
	linklayer/contract/RadioState.h \
	networklayer/common/InterfaceToken.h \
	linklayer/ieee80211/mac/WirelessMacBase.h \
	networklayer/queue/IQoSClassifier.h \
	networklayer/common/IInterfaceTable.h \
	base/FSMA.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h \
	util/FWMath.h \
	base/IPassiveQueue.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	linklayer/contract/PhyControlInfo_m.h \
	linklayer/ieee80211/radio/WifiPreambleType.h \
	networklayer/common/InterfaceTableAccess.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	base/Coord.h \
	base/NotifierConsts.h \
	linklayer/ieee80211/mac/WifiMode.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/radio/Radio80211aControlInfo_m.h \
	linklayer/ieee80211/mac/Ieee80211NewMac.h \
	linklayer/radio/ModulationType.h
$O/linklayer/ieee80211/mac/WifiMode.o: linklayer/ieee80211/mac/WifiMode.cc \
	base/INETDefs.h \
	linklayer/ieee80211/radio/WifiPreambleType.h \
	linklayer/radio/ModulationType.h \
	linklayer/ieee80211/mac/Ieee80211DataRate.h \
	linklayer/ieee80211/mac/WifiMode.h
$O/linklayer/ieee80211/mgmt/Ieee80211MgmtAdhoc.o: linklayer/ieee80211/mgmt/Ieee80211MgmtAdhoc.cc \
	base/IPassiveQueue.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtAdhoc.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	base/PassiveQueueBase.h
$O/linklayer/ieee80211/mgmt/Ieee80211MgmtSTASimplified.o: linklayer/ieee80211/mgmt/Ieee80211MgmtSTASimplified.cc \
	base/IPassiveQueue.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtSTASimplified.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	base/PassiveQueueBase.h
$O/linklayer/ieee80211/mgmt/Ieee80211Etx.o: linklayer/ieee80211/mgmt/Ieee80211Etx.cc \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	linklayer/ieee80211/mgmt/Ieee80211Etx.h \
	util/uint128.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/radio/Radio80211aControlInfo_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	linklayer/radio/ModulationType.h \
	networklayer/contract/IPv6Address.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	linklayer/ieee80211/mgmt/ETXPacket_m.h \
	networklayer/common/InterfaceEntry.h
$O/linklayer/ieee80211/mgmt/Ieee80211MgmtAPSimplified.o: linklayer/ieee80211/mgmt/Ieee80211MgmtAPSimplified.cc \
	base/IPassiveQueue.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtAPBase.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ethernet/Ethernet.h \
	base/ModuleAccess.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtAPSimplified.h \
	base/NotificationBoard.h \
	linklayer/ethernet/EtherFrame_m.h \
	base/PassiveQueueBase.h
$O/linklayer/ieee80211/mgmt/ETXPacket_m.o: linklayer/ieee80211/mgmt/ETXPacket_m.cc \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	linklayer/ieee80211/mgmt/ETXPacket_m.h
$O/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.o: linklayer/ieee80211/mgmt/Ieee80211MgmtBase.cc \
	base/IPassiveQueue.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	base/PassiveQueueBase.h
$O/linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.o: linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.cc \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	base/INETDefs.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h
$O/linklayer/ieee80211/mgmt/Ieee80211AgentSTA.o: linklayer/ieee80211/mgmt/Ieee80211AgentSTA.cc \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/ieee80211/mgmt/Ieee80211Primitives_m.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	linklayer/ieee80211/mgmt/Ieee80211AgentSTA.h
$O/linklayer/ieee80211/mgmt/Ieee80211MgmtAPBase.o: linklayer/ieee80211/mgmt/Ieee80211MgmtAPBase.cc \
	base/IPassiveQueue.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtAPBase.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ethernet/Ethernet.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	linklayer/ethernet/EtherFrame_m.h \
	base/PassiveQueueBase.h
$O/linklayer/ieee80211/mgmt/Ieee80211MgmtSTA.o: linklayer/ieee80211/mgmt/Ieee80211MgmtSTA.cc \
	base/IPassiveQueue.h \
	util/FWMath.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	linklayer/contract/PhyControlInfo_m.h \
	world/radio/ChannelAccess.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/ieee80211/mgmt/Ieee80211Primitives_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h \
	base/Coord.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	world/radio/IChannelControl.h \
	linklayer/contract/RadioState.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtSTA.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	base/BasicModule.h \
	base/PassiveQueueBase.h
$O/linklayer/ieee80211/mgmt/Ieee80211MgmtAP.o: linklayer/ieee80211/mgmt/Ieee80211MgmtAP.cc \
	base/IPassiveQueue.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtAPBase.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/contract/RadioState.h \
	linklayer/ethernet/Ethernet.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtAP.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	linklayer/ethernet/EtherFrame_m.h \
	base/PassiveQueueBase.h
$O/linklayer/ieee80211/mgmt/Ieee80211MgmtAdhocWithEtx.o: linklayer/ieee80211/mgmt/Ieee80211MgmtAdhocWithEtx.cc \
	base/IPassiveQueue.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	linklayer/ieee80211/mgmt/Ieee80211Etx.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtAdhocWithEtx.h \
	util/uint128.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/common/InterfaceToken.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtAdhoc.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/contract/IPv6Address.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	linklayer/ieee80211/mgmt/ETXPacket_m.h \
	base/PassiveQueueBase.h \
	networklayer/common/InterfaceEntry.h
$O/linklayer/ieee80211/mgmt/Ieee80211eClassifier.o: linklayer/ieee80211/mgmt/Ieee80211eClassifier.cc \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/ByteArray_m.h \
	base/ByteArray.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	linklayer/ieee80211/mgmt/Ieee80211eClassifier.h \
	transport/udp/UDPPacket_m.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv6/IPv6Datagram.h \
	networklayer/icmpv6/ICMPv6Message_m.h \
	networklayer/queue/IQoSClassifier.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/contract/IPProtocolId_m.h
$O/linklayer/ieee80211/mgmt/Ieee80211Primitives_m.o: linklayer/ieee80211/mgmt/Ieee80211Primitives_m.cc \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	base/INETDefs.h \
	linklayer/ieee80211/mgmt/Ieee80211Primitives_m.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h
$O/linklayer/ieee80211/radio/Ieee80211NewRadioModel.o: linklayer/ieee80211/radio/Ieee80211NewRadioModel.cc \
	util/FWMath.h \
	linklayer/radio/SnrList.h \
	base/INETDefs.h \
	linklayer/ieee80211/radio/WifiPreambleType.h \
	linklayer/ieee80211/radio/BerParseFile.h \
	base/Coord.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	linklayer/radio/AirFrame_m.h \
	linklayer/ieee80211/mac/WifiMode.h \
	linklayer/ieee80211/radio/errormodel/yans-error-rate-model.h \
	linklayer/ieee80211/radio/errormodel/dsss-error-rate-model.h \
	linklayer/radio/IRadioModel.h \
	linklayer/ieee80211/radio/Ieee80211NewRadioModel.h \
	linklayer/radio/ModulationType.h
$O/linklayer/ieee80211/radio/BerParseFile.o: linklayer/ieee80211/radio/BerParseFile.cc \
	linklayer/ieee80211/radio/BerParseFile.h
$O/linklayer/ieee80211/radio/Ieee80211RadioModel.o: linklayer/ieee80211/radio/Ieee80211RadioModel.cc \
	util/FWMath.h \
	linklayer/radio/IRadioModel.h \
	linklayer/radio/SnrList.h \
	linklayer/ieee80211/radio/Ieee80211RadioModel.h \
	base/INETDefs.h \
	linklayer/radio/ModulationType.h \
	base/Coord.h \
	linklayer/radio/AirFrame_m.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h
$O/linklayer/ieee80211/radio/errormodel/yans-error-rate-model.o: linklayer/ieee80211/radio/errormodel/yans-error-rate-model.cc \
	linklayer/ieee80211/radio/errormodel/dsss-error-rate-model.h \
	base/INETDefs.h \
	linklayer/ieee80211/radio/WifiPreambleType.h \
	linklayer/radio/ModulationType.h \
	linklayer/ieee80211/mac/WifiMode.h \
	linklayer/ieee80211/radio/errormodel/yans-error-rate-model.h
$O/linklayer/ieee80211/radio/errormodel/dsss-error-rate-model.o: linklayer/ieee80211/radio/errormodel/dsss-error-rate-model.cc \
	linklayer/ieee80211/radio/errormodel/dsss-error-rate-model.h
$O/linklayer/ieee80211mesh/hwmp/hwmp.o: linklayer/ieee80211mesh/hwmp/hwmp.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/common/InterfaceToken.h \
	linklayer/ieee80211mesh/hwmp/hwmp.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/contract/IPProtocolId_m.h \
	linklayer/ieee80211/mgmt/ETXPacket_m.h \
	networklayer/common/InterfaceEntry.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	linklayer/ieee80211/mgmt/Ieee80211Etx.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	linklayer/ieee80211mesh/hwmp/hwmp-rtable.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	linklayer/radio/Radio80211aControlInfo_m.h \
	networklayer/ipv4/ICMP.h \
	linklayer/radio/ModulationType.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/IRoutingTable.h
$O/linklayer/ieee80211mesh/hwmp/hwmp-rtable.o: linklayer/ieee80211mesh/hwmp/hwmp-rtable.cc \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	linklayer/ieee80211mesh/hwmp/hwmp-rtable.h
$O/linklayer/ieee80211mesh/mgmt/LWMPLSPacket_m.o: linklayer/ieee80211mesh/mgmt/LWMPLSPacket_m.cc \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	linklayer/ieee80211mesh/mgmt/LWMPLSPacket_m.h
$O/linklayer/ieee80211mesh/mgmt/lwmpls_data.o: linklayer/ieee80211mesh/mgmt/lwmpls_data.cc \
	linklayer/ieee80211mesh/mgmt/lwmpls_data.h
$O/linklayer/ieee80211mesh/mgmt/Ieee80211Mesh.o: linklayer/ieee80211mesh/mgmt/Ieee80211Mesh.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/olsr/OLSRpkt_m.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	linklayer/ieee80211mesh/mgmt/lwmpls_data.h \
	networklayer/contract/IPProtocolId_m.h \
	linklayer/ieee80211/mgmt/ETXPacket_m.h \
	networklayer/common/InterfaceEntry.h \
	base/IPassiveQueue.h \
	networklayer/manetrouting/dymo/dymo_msg_struct.h \
	networklayer/manetrouting/aodv/aodv_msg_struct.h \
	networklayer/ted/LinkStatePacket_m.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv6/IPv6Datagram.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/ipv4/ICMPMessage_m.h \
	base/PassiveQueueBase.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/NotificationBoard.h \
	linklayer/ieee80211mesh/mgmt/Ieee80211Mesh.h \
	linklayer/ieee80211mesh/mgmt/LWMPLSPacket_m.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	linklayer/ieee80211/mgmt/Ieee80211Etx.h \
	util/uint128.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	networklayer/mpls/MPLSPacket.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	networklayer/manetrouting/olsr/OLSR_ETX_parameter.h \
	networklayer/arp/ARPPacket_m.h \
	networklayer/ipv4/ICMP.h \
	networklayer/ted/TED_m.h \
	networklayer/manetrouting/base/MeshControlInfo_m.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/base/ControlInfoBreakLink_m.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/IRoutingTable.h
$O/linklayer/ieee80211mesh/mgmt/Ieee80211MeshLwmpls.o: linklayer/ieee80211mesh/mgmt/Ieee80211MeshLwmpls.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/olsr/OLSRpkt_m.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	linklayer/ieee80211mesh/mgmt/lwmpls_data.h \
	networklayer/contract/IPProtocolId_m.h \
	linklayer/ieee80211/mgmt/ETXPacket_m.h \
	networklayer/common/InterfaceEntry.h \
	base/IPassiveQueue.h \
	networklayer/manetrouting/dymo/dymo_msg_struct.h \
	networklayer/manetrouting/aodv/aodv_msg_struct.h \
	networklayer/ted/LinkStatePacket_m.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv6/IPv6Datagram.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/ipv4/ICMPMessage_m.h \
	base/PassiveQueueBase.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/NotificationBoard.h \
	linklayer/ieee80211mesh/mgmt/Ieee80211Mesh.h \
	linklayer/ieee80211mesh/mgmt/LWMPLSPacket_m.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	linklayer/ieee80211/mgmt/Ieee80211Etx.h \
	util/uint128.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	networklayer/mpls/MPLSPacket.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	networklayer/manetrouting/olsr/OLSR_ETX_parameter.h \
	networklayer/arp/ARPPacket_m.h \
	networklayer/ipv4/ICMP.h \
	networklayer/ted/TED_m.h \
	networklayer/manetrouting/base/MeshControlInfo_m.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/base/ControlInfoBreakLink_m.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/IRoutingTable.h
$O/linklayer/mf80211/core/BasicDecider.o: linklayer/mf80211/core/BasicDecider.cc \
	util/FWMath.h \
	linklayer/radio/SnrList.h \
	base/INETDefs.h \
	base/INotifiable.h \
	base/Coord.h \
	linklayer/radio/AirFrame_m.h \
	base/NotifierConsts.h \
	linklayer/mf80211/core/SnrControlInfo_m.h \
	linklayer/mf80211/core/BasicDecider.h \
	linklayer/radio/ModulationType.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/linklayer/mf80211/core/TransmComplete_m.o: linklayer/mf80211/core/TransmComplete_m.cc \
	linklayer/mf80211/core/TransmComplete_m.h \
	base/INETDefs.h
$O/linklayer/mf80211/core/SnrControlInfo_m.o: linklayer/mf80211/core/SnrControlInfo_m.cc \
	linklayer/radio/SnrList.h \
	base/INETDefs.h \
	linklayer/mf80211/core/SnrControlInfo_m.h
$O/linklayer/mf80211/core/BasicSnrEval.o: linklayer/mf80211/core/BasicSnrEval.cc \
	linklayer/mf80211/core/TransmComplete_m.h \
	util/FWMath.h \
	linklayer/radio/SnrList.h \
	world/radio/ChannelAccess.h \
	base/INETDefs.h \
	base/INotifiable.h \
	base/Coord.h \
	linklayer/radio/AirFrame_m.h \
	base/NotifierConsts.h \
	linklayer/mf80211/core/SnrControlInfo_m.h \
	world/radio/IChannelControl.h \
	linklayer/radio/ModulationType.h \
	linklayer/mf80211/core/BasicSnrEval.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	base/BasicModule.h
$O/linklayer/mf80211/macLayer/MacBase.o: linklayer/mf80211/macLayer/MacBase.cc \
	linklayer/mf80211/macLayer/MacBase.h \
	base/INotifiable.h \
	base/INETDefs.h \
	base/ModuleAccess.h \
	base/NotifierConsts.h \
	base/NotificationBoard.h
$O/linklayer/mf80211/macLayer/Mac80211.o: linklayer/mf80211/macLayer/Mac80211.cc \
	linklayer/mf80211/macLayer/MacBase.h \
	linklayer/mf80211/macLayer/MacPkt_m.h \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/mf80211/macLayer/Mac80211Pkt_m.h \
	networklayer/common/InterfaceTableAccess.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/common/InterfaceToken.h \
	linklayer/contract/RadioState.h \
	util/opp_utils.h \
	linklayer/mf80211/macLayer/Mac80211.h \
	linklayer/mf80211/macLayer/Consts80211.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h
$O/linklayer/mf80211/macLayer/Mac80211Pkt_m.o: linklayer/mf80211/macLayer/Mac80211Pkt_m.cc \
	linklayer/mf80211/macLayer/MacPkt_m.h \
	base/INETDefs.h \
	linklayer/mf80211/macLayer/Mac80211Pkt_m.h \
	linklayer/contract/MACAddress.h
$O/linklayer/mf80211/macLayer/CSMAMacLayer.o: linklayer/mf80211/macLayer/CSMAMacLayer.cc \
	linklayer/mf80211/macLayer/MacBase.h \
	linklayer/mf80211/macLayer/MacPkt_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/common/InterfaceTableAccess.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/common/InterfaceToken.h \
	linklayer/contract/RadioState.h \
	util/opp_utils.h \
	linklayer/mf80211/macLayer/CSMAMacLayer.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/common/InterfaceEntry.h
$O/linklayer/mf80211/macLayer/MacPkt_m.o: linklayer/mf80211/macLayer/MacPkt_m.cc \
	linklayer/mf80211/macLayer/MacPkt_m.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h
$O/linklayer/mf80211/phyLayer/decider/Decider80211.o: linklayer/mf80211/phyLayer/decider/Decider80211.cc \
	util/FWMath.h \
	linklayer/mf80211/phyLayer/decider/Decider80211.h \
	linklayer/radio/SnrList.h \
	linklayer/contract/PhyControlInfo_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/radio/AirFrame_m.h \
	base/Coord.h \
	base/NotifierConsts.h \
	linklayer/mf80211/core/SnrControlInfo_m.h \
	linklayer/mf80211/macLayer/Consts80211.h \
	linklayer/mf80211/core/BasicDecider.h \
	linklayer/radio/ModulationType.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	base/BasicModule.h
$O/linklayer/mf80211/phyLayer/decider/ErrAndCollDecider.o: linklayer/mf80211/phyLayer/decider/ErrAndCollDecider.cc \
	util/FWMath.h \
	linklayer/mf80211/phyLayer/decider/SnrDecider.h \
	linklayer/radio/SnrList.h \
	linklayer/mf80211/phyLayer/decider/ErrAndCollDecider.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/radio/AirFrame_m.h \
	base/Coord.h \
	base/NotifierConsts.h \
	linklayer/mf80211/core/SnrControlInfo_m.h \
	linklayer/mf80211/core/BasicDecider.h \
	linklayer/radio/ModulationType.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	base/BasicModule.h
$O/linklayer/mf80211/phyLayer/decider/SnrDecider.o: linklayer/mf80211/phyLayer/decider/SnrDecider.cc \
	util/FWMath.h \
	linklayer/mf80211/phyLayer/decider/SnrDecider.h \
	linklayer/radio/SnrList.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/radio/AirFrame_m.h \
	base/Coord.h \
	base/NotifierConsts.h \
	linklayer/mf80211/core/SnrControlInfo_m.h \
	linklayer/mf80211/core/BasicDecider.h \
	linklayer/radio/ModulationType.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	base/BasicModule.h
$O/linklayer/mf80211/phyLayer/snrEval/GilbertElliotSnr.o: linklayer/mf80211/phyLayer/snrEval/GilbertElliotSnr.cc \
	util/FWMath.h \
	linklayer/radio/SnrList.h \
	linklayer/contract/PhyControlInfo_m.h \
	world/radio/ChannelAccess.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/mf80211/phyLayer/snrEval/GilbertElliotSnr.h \
	linklayer/mf80211/phyLayer/snrEval/SnrEval.h \
	linklayer/radio/AirFrame_m.h \
	base/Coord.h \
	base/NotifierConsts.h \
	linklayer/mf80211/core/SnrControlInfo_m.h \
	world/radio/IChannelControl.h \
	linklayer/contract/RadioState.h \
	linklayer/radio/ModulationType.h \
	linklayer/mf80211/core/BasicSnrEval.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/linklayer/mf80211/phyLayer/snrEval/SnrEval.o: linklayer/mf80211/phyLayer/snrEval/SnrEval.cc \
	util/FWMath.h \
	linklayer/radio/SnrList.h \
	world/radio/ChannelAccess.h \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/mf80211/phyLayer/snrEval/SnrEval.h \
	linklayer/radio/AirFrame_m.h \
	base/Coord.h \
	base/NotifierConsts.h \
	linklayer/mf80211/core/SnrControlInfo_m.h \
	world/radio/IChannelControl.h \
	linklayer/contract/RadioState.h \
	linklayer/radio/ModulationType.h \
	linklayer/mf80211/core/BasicSnrEval.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	base/BasicModule.h
$O/linklayer/mf80211/phyLayer/snrEval/SnrEval80211.o: linklayer/mf80211/phyLayer/snrEval/SnrEval80211.cc \
	util/FWMath.h \
	linklayer/radio/SnrList.h \
	linklayer/contract/PhyControlInfo_m.h \
	world/radio/ChannelAccess.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/mf80211/phyLayer/snrEval/SnrEval.h \
	linklayer/radio/AirFrame_m.h \
	base/Coord.h \
	base/NotifierConsts.h \
	linklayer/mf80211/core/SnrControlInfo_m.h \
	world/radio/IChannelControl.h \
	linklayer/contract/RadioState.h \
	linklayer/mf80211/macLayer/Consts80211.h \
	linklayer/radio/ModulationType.h \
	linklayer/mf80211/core/BasicSnrEval.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	base/BasicModule.h \
	linklayer/mf80211/phyLayer/snrEval/SnrEval80211.h
$O/linklayer/ppp/PPP.o: linklayer/ppp/PPP.cc \
	base/IPassiveQueue.h \
	linklayer/contract/TxNotifDetails.h \
	linklayer/ppp/PPPFrame_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/ppp/PPP.h \
	networklayer/common/InterfaceTableAccess.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceToken.h \
	util/opp_utils.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h
$O/linklayer/ppp/PPPFrame_m.o: linklayer/ppp/PPPFrame_m.cc \
	linklayer/ppp/PPPFrame_m.h \
	base/INETDefs.h
$O/linklayer/radio/GenericRadioModel.o: linklayer/radio/GenericRadioModel.cc \
	linklayer/radio/IModulation.h \
	util/FWMath.h \
	linklayer/radio/IRadioModel.h \
	linklayer/radio/SnrList.h \
	base/INETDefs.h \
	linklayer/radio/ModulationType.h \
	base/Coord.h \
	linklayer/radio/AirFrame_m.h \
	linklayer/radio/Modulation.h \
	linklayer/radio/GenericRadioModel.h
$O/linklayer/radio/Radio.o: linklayer/radio/Radio.cc \
	util/FWMath.h \
	world/powercontrol/IPowerControl.h \
	linklayer/radio/propagation/IReceptionModel.h \
	world/obstacles/Obstacle.h \
	linklayer/radio/SnrList.h \
	world/radio/ChannelAccess.h \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/radio/AirFrame_m.h \
	base/Coord.h \
	base/NotifierConsts.h \
	battery/models/BasicBattery.h \
	world/radio/IChannelControl.h \
	linklayer/radio/IRadioModel.h \
	linklayer/radio/Radio80211aControlInfo_m.h \
	world/annotations/AnnotationManager.h \
	linklayer/contract/RadioState.h \
	linklayer/radio/ModulationType.h \
	world/obstacles/ObstacleControl.h \
	linklayer/radio/Radio.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	base/BasicModule.h
$O/linklayer/radio/Modulation.o: linklayer/radio/Modulation.cc \
	linklayer/radio/IModulation.h \
	base/INETDefs.h \
	linklayer/radio/Modulation.h
$O/linklayer/radio/Radio80211aControlInfo_m.o: linklayer/radio/Radio80211aControlInfo_m.cc \
	linklayer/radio/Radio80211aControlInfo_m.h \
	linklayer/radio/ModulationType.h
$O/linklayer/radio/AirFrame_m.o: linklayer/radio/AirFrame_m.cc \
	util/FWMath.h \
	base/INETDefs.h \
	linklayer/radio/ModulationType.h \
	base/Coord.h \
	linklayer/radio/AirFrame_m.h
$O/linklayer/radio/propagation/FreeSpaceModel.o: linklayer/radio/propagation/FreeSpaceModel.cc \
	util/FWMath.h \
	linklayer/radio/propagation/IReceptionModel.h \
	world/radio/IChannelControl.h \
	world/radio/ChannelAccess.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/radio/propagation/FreeSpaceModel.h \
	base/Coord.h \
	base/ModuleAccess.h \
	base/NotifierConsts.h \
	base/NotificationBoard.h \
	base/BasicModule.h
$O/linklayer/radio/propagation/RayleighModel.o: linklayer/radio/propagation/RayleighModel.cc \
	util/FWMath.h \
	linklayer/radio/propagation/IReceptionModel.h \
	linklayer/radio/propagation/RayleighModel.h \
	base/INETDefs.h \
	linklayer/radio/propagation/FreeSpaceModel.h
$O/linklayer/radio/propagation/LogNormalShadowingModel.o: linklayer/radio/propagation/LogNormalShadowingModel.cc \
	util/FWMath.h \
	linklayer/radio/propagation/LogNormalShadowingModel.h \
	linklayer/radio/propagation/IReceptionModel.h \
	base/INETDefs.h \
	linklayer/radio/propagation/FreeSpaceModel.h
$O/linklayer/radio/propagation/TwoRayGroundModel.o: linklayer/radio/propagation/TwoRayGroundModel.cc \
	util/FWMath.h \
	linklayer/radio/propagation/IReceptionModel.h \
	base/INETDefs.h \
	linklayer/radio/propagation/FreeSpaceModel.h \
	linklayer/radio/propagation/TwoRayGroundModel.h
$O/linklayer/radio/propagation/RiceModel.o: linklayer/radio/propagation/RiceModel.cc \
	util/FWMath.h \
	linklayer/radio/propagation/RiceModel.h \
	linklayer/radio/propagation/IReceptionModel.h \
	base/INETDefs.h \
	linklayer/radio/propagation/FreeSpaceModel.h
$O/linklayer/radio/propagation/NakagamiModel.o: linklayer/radio/propagation/NakagamiModel.cc \
	util/FWMath.h \
	linklayer/radio/propagation/IReceptionModel.h \
	linklayer/radio/propagation/NakagamiModel.h \
	base/INETDefs.h \
	linklayer/radio/propagation/FreeSpaceModel.h
$O/linklayer/rstp/stpTimers_m.o: linklayer/rstp/stpTimers_m.cc \
	linklayer/rstp/stpTimers_m.h
$O/linklayer/rstp/STPDefinitions.o: linklayer/rstp/STPDefinitions.cc \
	base/INETDefs.h \
	linklayer/rstp/STPDefinitions.h \
	linklayer/contract/MACAddress.h
$O/linklayer/rstp/MACRelayUnitSTPNP.o: linklayer/rstp/MACRelayUnitSTPNP.cc \
	linklayer/ethernet/switch/MACRelayUnitNP.h \
	linklayer/rstp/bpdu_m.h \
	linklayer/rstp/MACRelayUnitSTPNP.h \
	linklayer/ethernet/switch/MACRelayUnitBase.h \
	base/INETDefs.h \
	linklayer/ethernet/Ethernet.h \
	linklayer/rstp/STPDefinitions.h \
	linklayer/contract/MACAddress.h \
	linklayer/ethernet/EtherFrame_m.h \
	linklayer/rstp/stpTimers_m.h \
	linklayer/contract/Ieee802Ctrl_m.h
$O/linklayer/rstp/bpdu_m.o: linklayer/rstp/bpdu_m.cc \
	linklayer/rstp/bpdu_m.h \
	base/INETDefs.h \
	linklayer/rstp/STPDefinitions.h \
	linklayer/contract/MACAddress.h
$O/mobility/models/RectangleMobility.o: mobility/models/RectangleMobility.cc \
	util/FWMath.h \
	mobility/models/RectangleMobility.h \
	base/INotifiable.h \
	base/INETDefs.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	base/NotificationBoard.h \
	base/NotifierConsts.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/mobility/models/GaussMarkovMobility.o: mobility/models/GaussMarkovMobility.cc \
	util/FWMath.h \
	base/INotifiable.h \
	base/INETDefs.h \
	mobility/models/GaussMarkovMobility.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	base/NotificationBoard.h \
	base/NotifierConsts.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/mobility/models/BonnMotionFileCache.o: mobility/models/BonnMotionFileCache.cc \
	util/FWMath.h \
	mobility/models/BonnMotionFileCache.h \
	base/INotifiable.h \
	base/INETDefs.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	base/NotificationBoard.h \
	base/NotifierConsts.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/mobility/models/BasicMobility.o: mobility/models/BasicMobility.cc \
	util/FWMath.h \
	base/INETDefs.h \
	base/INotifiable.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	base/ModuleAccess.h \
	base/NotifierConsts.h \
	base/NotificationBoard.h \
	base/BasicModule.h
$O/mobility/models/MassMobility.o: mobility/models/MassMobility.cc \
	util/FWMath.h \
	base/INETDefs.h \
	base/INotifiable.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	base/ModuleAccess.h \
	base/NotifierConsts.h \
	base/NotificationBoard.h \
	mobility/models/MassMobility.h \
	base/BasicModule.h
$O/mobility/models/RandomWPMobility.o: mobility/models/RandomWPMobility.cc \
	util/FWMath.h \
	mobility/models/LineSegmentsMobilityBase.h \
	base/INotifiable.h \
	base/INETDefs.h \
	mobility/models/RandomWPMobility.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	base/NotificationBoard.h \
	base/NotifierConsts.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/mobility/models/ChiangMobility.o: mobility/models/ChiangMobility.cc \
	util/FWMath.h \
	base/INETDefs.h \
	base/INotifiable.h \
	mobility/models/BasicMobility.h \
	mobility/models/ChiangMobility.h \
	base/Coord.h \
	base/ModuleAccess.h \
	base/NotifierConsts.h \
	base/NotificationBoard.h \
	base/BasicModule.h
$O/mobility/models/TurtleMobility.o: mobility/models/TurtleMobility.cc \
	util/FWMath.h \
	mobility/models/LineSegmentsMobilityBase.h \
	base/INotifiable.h \
	base/INETDefs.h \
	mobility/models/BasicMobility.h \
	mobility/models/TurtleMobility.h \
	base/Coord.h \
	base/NotificationBoard.h \
	base/NotifierConsts.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/mobility/models/ANSimMobility.o: mobility/models/ANSimMobility.cc \
	util/FWMath.h \
	mobility/models/LineSegmentsMobilityBase.h \
	base/INotifiable.h \
	base/INETDefs.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	base/NotificationBoard.h \
	base/NotifierConsts.h \
	base/ModuleAccess.h \
	base/BasicModule.h \
	mobility/models/ANSimMobility.h
$O/mobility/models/CircleMobility.o: mobility/models/CircleMobility.cc \
	util/FWMath.h \
	base/INETDefs.h \
	base/INotifiable.h \
	mobility/models/CircleMobility.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	base/ModuleAccess.h \
	base/NotifierConsts.h \
	base/NotificationBoard.h \
	base/BasicModule.h
$O/mobility/models/NullMobility.o: mobility/models/NullMobility.cc \
	util/FWMath.h \
	mobility/models/NullMobility.h \
	base/INotifiable.h \
	base/INETDefs.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	base/NotificationBoard.h \
	base/NotifierConsts.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/mobility/models/StaticGridMobility.o: mobility/models/StaticGridMobility.cc \
	util/FWMath.h \
	base/INETDefs.h \
	base/INotifiable.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	mobility/models/StaticGridMobility.h \
	base/ModuleAccess.h \
	base/NotifierConsts.h \
	base/NotificationBoard.h \
	base/BasicModule.h
$O/mobility/models/LineSegmentsMobilityBase.o: mobility/models/LineSegmentsMobilityBase.cc \
	util/FWMath.h \
	mobility/models/LineSegmentsMobilityBase.h \
	base/INotifiable.h \
	base/INETDefs.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	base/NotificationBoard.h \
	base/NotifierConsts.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/mobility/models/ConstSpeedMobility.o: mobility/models/ConstSpeedMobility.cc \
	util/FWMath.h \
	mobility/models/ConstSpeedMobility.h \
	base/INotifiable.h \
	base/INETDefs.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	base/NotificationBoard.h \
	base/NotifierConsts.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/mobility/models/LinearMobility.o: mobility/models/LinearMobility.cc \
	util/FWMath.h \
	base/INotifiable.h \
	base/INETDefs.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	base/ModuleAccess.h \
	base/NotifierConsts.h \
	base/NotificationBoard.h \
	base/BasicModule.h \
	mobility/models/LinearMobility.h
$O/mobility/models/BonnMotionMobility.o: mobility/models/BonnMotionMobility.cc \
	util/FWMath.h \
	mobility/models/LineSegmentsMobilityBase.h \
	mobility/models/BonnMotionFileCache.h \
	base/INotifiable.h \
	base/INETDefs.h \
	mobility/models/BasicMobility.h \
	mobility/models/BonnMotionMobility.h \
	base/Coord.h \
	base/NotificationBoard.h \
	base/NotifierConsts.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/mobility/models/Ns2MotionMobility.o: mobility/models/Ns2MotionMobility.cc \
	util/FWMath.h \
	mobility/models/LineSegmentsMobilityBase.h \
	base/INotifiable.h \
	base/INETDefs.h \
	mobility/models/Ns2MotionMobility.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	base/NotificationBoard.h \
	base/NotifierConsts.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/mobility/traci/TraCIMobility.o: mobility/traci/TraCIMobility.cc \
	util/FWMath.h \
	world/traci/TraCIScenarioManager.h \
	world/radio/IChannelControl.h \
	base/INotifiable.h \
	base/INETDefs.h \
	mobility/models/BasicMobility.h \
	base/Coord.h \
	mobility/traci/TraCIMobility.h \
	base/NotifierConsts.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/networklayer/arp/ARPPacket_m.o: networklayer/arp/ARPPacket_m.cc \
	networklayer/arp/ARPPacket_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h
$O/networklayer/arp/ARP.o: networklayer/arp/ARP.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/arp/ARP.h \
	networklayer/ipv4/RoutingTableAccess.h \
	base/INETDefs.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/arp/ARPPacket_m.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/autorouting/ipv4/HostAutoConfigurator2.o: networklayer/autorouting/ipv4/HostAutoConfigurator2.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/autorouting/ipv4/HostAutoConfigurator2.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv4/IRoutingTable.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/autorouting/ipv4/NetworkConfigurator.o: networklayer/autorouting/ipv4/NetworkConfigurator.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	linklayer/contract/MACAddress.h \
	networklayer/autorouting/ipv4/NetworkConfigurator.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv4/IRoutingTable.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/autorouting/ipv4/FlatNetworkConfigurator.o: networklayer/autorouting/ipv4/FlatNetworkConfigurator.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/autorouting/ipv4/FlatNetworkConfigurator.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv4/IRoutingTable.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/autorouting/ipv6/FlatNetworkConfigurator6.o: networklayer/autorouting/ipv6/FlatNetworkConfigurator6.cc \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/autorouting/ipv6/FlatNetworkConfigurator6.h \
	networklayer/ipv6/IPv6InterfaceData.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv6/RoutingTable6.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/contract/IPv6Address.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/bgpv4/BGPRouting.o: networklayer/bgpv4/BGPRouting.cc \
	networklayer/bgpv4/BGPFSM.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	networklayer/bgpv4/BGPMessage/BGPOpen.h \
	networklayer/bgpv4/BGPMessage/BGPUpdate_m.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/bgpv4/BGPCommon.h \
	networklayer/ospfv2/OSPFRoutingAccess.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/bgpv4/BGPMessage/BGPKeepAlive.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/bgpv4/BGPMessage/BGPOpen_m.h \
	base/NotifierConsts.h \
	networklayer/ospfv2/OSPFRouting.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/bgpv4/BGPRoutingTableEntry.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	util/Macho.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/bgpv4/BGPSession.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/bgpv4/BGPRouting.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	transport/tcp_common/contract/TCPSocket.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	transport/tcp_common/contract/TCPSocketMap.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/bgpv4/BGPMessage/BGPKeepAlive_m.h \
	networklayer/bgpv4/BGPMessage/BGPASPathSegment_m.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/ipv4/RoutingTable.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/bgpv4/BGPMessage/BGPHeader_m.h \
	networklayer/bgpv4/BGPMessage/BGPUpdate.h \
	networklayer/bgpv4/BGPMessage/BGPUpdatePathAttributes_m.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/bgpv4/BGPFSM.o: networklayer/bgpv4/BGPFSM.cc \
	networklayer/bgpv4/BGPFSM.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	networklayer/bgpv4/BGPMessage/BGPUpdate_m.h \
	networklayer/bgpv4/BGPMessage/BGPOpen.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/bgpv4/BGPCommon.h \
	networklayer/ospfv2/OSPFRoutingAccess.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/bgpv4/BGPMessage/BGPKeepAlive.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/bgpv4/BGPMessage/BGPOpen_m.h \
	networklayer/ospfv2/OSPFRouting.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/bgpv4/BGPRoutingTableEntry.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	util/Macho.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/bgpv4/BGPSession.h \
	networklayer/bgpv4/BGPRouting.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	transport/tcp_common/contract/TCPSocket.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	transport/tcp_common/contract/TCPSocketMap.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/bgpv4/BGPMessage/BGPKeepAlive_m.h \
	networklayer/bgpv4/BGPMessage/BGPASPathSegment_m.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/ipv4/RoutingTable.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/bgpv4/BGPMessage/BGPHeader_m.h \
	networklayer/bgpv4/BGPMessage/BGPUpdate.h \
	networklayer/bgpv4/BGPMessage/BGPUpdatePathAttributes_m.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/bgpv4/BGPSession.o: networklayer/bgpv4/BGPSession.cc \
	networklayer/bgpv4/BGPFSM.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	networklayer/bgpv4/BGPMessage/BGPUpdate_m.h \
	networklayer/bgpv4/BGPMessage/BGPOpen.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/bgpv4/BGPCommon.h \
	networklayer/ospfv2/OSPFRoutingAccess.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/bgpv4/BGPMessage/BGPKeepAlive.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/bgpv4/BGPMessage/BGPOpen_m.h \
	networklayer/ospfv2/OSPFRouting.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/bgpv4/BGPRoutingTableEntry.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	util/Macho.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/bgpv4/BGPSession.h \
	networklayer/bgpv4/BGPRouting.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	transport/tcp_common/contract/TCPSocket.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	transport/tcp_common/contract/TCPSocketMap.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/bgpv4/BGPMessage/BGPKeepAlive_m.h \
	networklayer/bgpv4/BGPMessage/BGPASPathSegment_m.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/ipv4/RoutingTable.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/bgpv4/BGPMessage/BGPHeader_m.h \
	networklayer/bgpv4/BGPMessage/BGPUpdate.h \
	networklayer/bgpv4/BGPMessage/BGPUpdatePathAttributes_m.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/bgpv4/BGPMessage/BGPHeader_m.o: networklayer/bgpv4/BGPMessage/BGPHeader_m.cc \
	networklayer/bgpv4/BGPMessage/BGPHeader_m.h
$O/networklayer/bgpv4/BGPMessage/BGPUpdate_m.o: networklayer/bgpv4/BGPMessage/BGPUpdate_m.cc \
	networklayer/bgpv4/BGPMessage/BGPUpdatePathAttributes_m.h \
	networklayer/bgpv4/BGPMessage/BGPASPathSegment_m.h \
	base/INETDefs.h \
	networklayer/bgpv4/BGPMessage/BGPUpdate_m.h \
	networklayer/contract/IPv4Address.h \
	networklayer/bgpv4/BGPMessage/BGPHeader_m.h
$O/networklayer/bgpv4/BGPMessage/BGPOpen_m.o: networklayer/bgpv4/BGPMessage/BGPOpen_m.cc \
	networklayer/bgpv4/BGPMessage/BGPOpen_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/bgpv4/BGPMessage/BGPHeader_m.h
$O/networklayer/bgpv4/BGPMessage/BGPUpdate.o: networklayer/bgpv4/BGPMessage/BGPUpdate.cc \
	networklayer/bgpv4/BGPMessage/BGPUpdatePathAttributes_m.h \
	networklayer/bgpv4/BGPMessage/BGPASPathSegment_m.h \
	base/INETDefs.h \
	networklayer/bgpv4/BGPMessage/BGPUpdate_m.h \
	networklayer/contract/IPv4Address.h \
	networklayer/bgpv4/BGPMessage/BGPHeader_m.h \
	networklayer/bgpv4/BGPMessage/BGPUpdate.h
$O/networklayer/bgpv4/BGPMessage/BGPUpdatePathAttributes_m.o: networklayer/bgpv4/BGPMessage/BGPUpdatePathAttributes_m.cc \
	networklayer/bgpv4/BGPMessage/BGPUpdatePathAttributes_m.h \
	networklayer/bgpv4/BGPMessage/BGPASPathSegment_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h
$O/networklayer/bgpv4/BGPMessage/BGPKeepAlive_m.o: networklayer/bgpv4/BGPMessage/BGPKeepAlive_m.cc \
	networklayer/bgpv4/BGPMessage/BGPKeepAlive_m.h \
	networklayer/bgpv4/BGPMessage/BGPHeader_m.h
$O/networklayer/bgpv4/BGPMessage/BGPASPathSegment_m.o: networklayer/bgpv4/BGPMessage/BGPASPathSegment_m.cc \
	networklayer/bgpv4/BGPMessage/BGPASPathSegment_m.h
$O/networklayer/common/InterfaceTable.o: networklayer/common/InterfaceTable.cc \
	networklayer/common/InterfaceTable.h \
	networklayer/common/InterfaceToken.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/common/InterfaceEntry.o: networklayer/common/InterfaceEntry.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/common/InterfaceToken.h \
	base/INETDefs.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ipv6/IPv6InterfaceData.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/contract/IPv6Address.o: networklayer/contract/IPv6Address.cc \
	networklayer/common/InterfaceToken.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h
$O/networklayer/contract/IPvXAddressResolver.o: networklayer/contract/IPvXAddressResolver.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/ipv6/IPv6InterfaceData.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv6/RoutingTable6.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/contract/IPv6Address.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv4/IRoutingTable.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/contract/IPProtocolId_m.o: networklayer/contract/IPProtocolId_m.cc \
	base/INETDefs.h \
	networklayer/contract/IPProtocolId_m.h
$O/networklayer/contract/IPvXAddress.o: networklayer/contract/IPvXAddress.cc \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h
$O/networklayer/contract/IPv4Address.o: networklayer/contract/IPv4Address.cc \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h
$O/networklayer/extras/Dummy.o: networklayer/extras/Dummy.cc
$O/networklayer/extras/FailureManager.o: networklayer/extras/FailureManager.cc \
	base/INETDefs.h \
	networklayer/extras/FailureManager.h \
	base/IScriptable.h
$O/networklayer/icmpv6/IPv6NeighbourCache.o: networklayer/icmpv6/IPv6NeighbourCache.cc \
	networklayer/icmpv6/IPv6NeighbourCache.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h \
	linklayer/contract/MACAddress.h
$O/networklayer/icmpv6/IPv6NeighbourDiscovery.o: networklayer/icmpv6/IPv6NeighbourDiscovery.cc \
	networklayer/icmpv6/ICMPv6Access.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/ipv6/IPv6InterfaceData.h \
	linklayer/contract/MACAddress.h \
	networklayer/icmpv6/IPv6NeighbourCache.h \
	networklayer/common/InterfaceToken.h \
	networklayer/xmipv6/xMIPv6Access.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/icmpv6/ICMPv6.h \
	networklayer/ipv6/IPv6ExtensionHeaders.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/xmipv6/MobilityHeader_m.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv6tunneling/IPv6Tunneling.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/xmipv6/BindingUpdateList.h \
	networklayer/xmipv6/xMIPv6.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/icmpv6/IPv6NeighbourDiscovery.h \
	networklayer/icmpv6/IPv6NDMessage_m.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	base/NotifierConsts.h \
	networklayer/ipv6/RoutingTable6.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/ipv6/IPv6Datagram.h \
	networklayer/icmpv6/ICMPv6Message_m.h \
	networklayer/xmipv6/MobilityHeader.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/RoutingTable6Access.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/ipv6/IPv6ExtensionHeaders_m.h
$O/networklayer/icmpv6/ICMPv6Message_m.o: networklayer/icmpv6/ICMPv6Message_m.cc \
	base/INETDefs.h \
	networklayer/icmpv6/ICMPv6Message_m.h
$O/networklayer/icmpv6/ICMPv6.o: networklayer/icmpv6/ICMPv6.cc \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/ipv6/IPv6Datagram.h \
	base/INETDefs.h \
	networklayer/icmpv6/ICMPv6Message_m.h \
	networklayer/icmpv6/ICMPv6.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/contract/IPProtocolId_m.h
$O/networklayer/icmpv6/IPv6NDMessage_m.o: networklayer/icmpv6/IPv6NDMessage_m.cc \
	networklayer/icmpv6/ICMPv6Message_m.h \
	base/INETDefs.h \
	networklayer/icmpv6/IPv6NDMessage_m.h \
	networklayer/contract/IPv6Address.h \
	linklayer/contract/MACAddress.h
$O/networklayer/ipv4/IPv4.o: networklayer/ipv4/IPv4.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	base/QueueBase.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	transport/tcp_common/TCPSegment.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ipv4/IPv4.h \
	base/ProtocolMap.h \
	base/AbstractQueue.h \
	base/ByteArray_m.h \
	base/ByteArray.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	transport/udp/UDPPacket_m.h \
	networklayer/ipv4/IPv4FragBuf.h \
	networklayer/arp/ARPPacket_m.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/ipv4/ICMP.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	base/ReassemblyBuffer.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ipv4/RoutingTableParser.o: networklayer/ipv4/RoutingTableParser.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/RoutingTableParser.h \
	networklayer/ipv4/RoutingTable.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv4/IRoutingTable.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/ipv4/IPv4FragBuf.o: networklayer/ipv4/IPv4FragBuf.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/ipv4/RoutingTableAccess.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ipv4/IPv4FragBuf.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/ICMP.h \
	networklayer/ipv4/IPv4Datagram.h \
	base/ModuleAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/contract/IPProtocolId_m.h \
	base/ReassemblyBuffer.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ipv4/IPv4Datagram_m.o: networklayer/ipv4/IPv4Datagram_m.cc \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPProtocolId_m.h
$O/networklayer/ipv4/ICMP.o: networklayer/ipv4/ICMP.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/ipv4/RoutingTableAccess.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/ipv4/ICMP.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPMessage_m.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ipv4/RoutingTable.o: networklayer/ipv4/RoutingTable.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/RoutingTableParser.h \
	networklayer/ipv4/RoutingTable.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv4/IRoutingTable.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/ipv4/IPv4Route.o: networklayer/ipv4/IPv4Route.cc \
	networklayer/common/InterfaceToken.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/ipv4/ICMPMessage_m.o: networklayer/ipv4/ICMPMessage_m.cc \
	base/INETDefs.h \
	networklayer/ipv4/ICMPMessage_m.h
$O/networklayer/ipv4/IPv4InterfaceData.o: networklayer/ipv4/IPv4InterfaceData.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/common/InterfaceToken.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/ipv4/NetworkInfo.o: networklayer/ipv4/NetworkInfo.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/NetworkInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/contract/IPv6Address.h \
	base/IScriptable.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ipv4/ErrorHandling.o: networklayer/ipv4/ErrorHandling.cc \
	networklayer/ipv4/ErrorHandling.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/ipv4/IPv4Datagram.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/contract/IPProtocolId_m.h
$O/networklayer/ipv4/IPv4RouteRule.o: networklayer/ipv4/IPv4RouteRule.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/common/InterfaceToken.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/ipv4/contract/IPv4ControlInfo_m.o: networklayer/ipv4/contract/IPv4ControlInfo_m.cc \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h \
	networklayer/contract/IPProtocolId_m.h
$O/networklayer/ipv4/contract/IPv4ControlInfo.o: networklayer/ipv4/contract/IPv4ControlInfo.cc \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/ipv4/IPv4Datagram.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h \
	networklayer/contract/IPProtocolId_m.h
$O/networklayer/ipv6/IPv6ExtensionHeaders.o: networklayer/ipv6/IPv6ExtensionHeaders.cc \
	networklayer/ipv6/IPv6Datagram.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/ipv6/IPv6ExtensionHeaders.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv6/IPv6ExtensionHeaders_m.h
$O/networklayer/ipv6/IPv6Datagram.o: networklayer/ipv6/IPv6Datagram.cc \
	networklayer/ipv6/IPv6Datagram.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/ipv6/IPv6ExtensionHeaders.h \
	networklayer/ipv6/IPv6ExtensionHeaders_m.h \
	networklayer/contract/IPProtocolId_m.h
$O/networklayer/ipv6/IPv6FragBuf.o: networklayer/ipv6/IPv6FragBuf.cc \
	base/INETDefs.h \
	networklayer/ipv6/IPv6FragBuf.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/ipv6/IPv6Datagram.h \
	networklayer/icmpv6/ICMPv6Message_m.h \
	networklayer/icmpv6/ICMPv6.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/IPv6ExtensionHeaders.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv6/IPv6ExtensionHeaders_m.h \
	base/ReassemblyBuffer.h
$O/networklayer/ipv6/IPv6Datagram_m.o: networklayer/ipv6/IPv6Datagram_m.cc \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/contract/IPProtocolId_m.h
$O/networklayer/ipv6/IPv6.o: networklayer/ipv6/IPv6.cc \
	networklayer/icmpv6/ICMPv6Access.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/ipv6/IPv6InterfaceData.h \
	networklayer/ipv6/IPv6FragBuf.h \
	linklayer/contract/MACAddress.h \
	base/QueueBase.h \
	networklayer/icmpv6/IPv6NeighbourCache.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/icmpv6/ICMPv6.h \
	base/NotificationBoard.h \
	networklayer/ipv6/IPv6ExtensionHeaders.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/xmipv6/MobilityHeader_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ipv6tunneling/IPv6Tunneling.h \
	base/ProtocolMap.h \
	base/AbstractQueue.h \
	networklayer/ipv6tunneling/IPv6TunnelingAccess.h \
	networklayer/icmpv6/IPv6NeighbourDiscoveryAccess.h \
	networklayer/ipv6/IPv6.h \
	networklayer/icmpv6/IPv6NDMessage_m.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/icmpv6/IPv6NeighbourDiscovery.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	base/NotifierConsts.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/ipv6/RoutingTable6.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/ipv6/IPv6Datagram.h \
	networklayer/icmpv6/ICMPv6Message_m.h \
	networklayer/xmipv6/MobilityHeader.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/ipv6/RoutingTable6Access.h \
	networklayer/ipv6/IPv6ExtensionHeaders_m.h \
	base/ReassemblyBuffer.h
$O/networklayer/ipv6/IPv6ErrorHandling.o: networklayer/ipv6/IPv6ErrorHandling.cc \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/ipv6/IPv6Datagram.h \
	networklayer/icmpv6/ICMPv6Message_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv6/IPv6ErrorHandling.h
$O/networklayer/ipv6/RoutingTable6.o: networklayer/ipv6/RoutingTable6.cc \
	networklayer/ipv6tunneling/IPv6TunnelingAccess.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/ipv6/IPv6InterfaceData.h \
	networklayer/common/InterfaceTableAccess.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv6/RoutingTable6.h \
	networklayer/common/InterfaceToken.h \
	util/opp_utils.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/contract/IPv6Address.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/ipv6tunneling/IPv6Tunneling.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/ipv6/IPv6InterfaceData.o: networklayer/ipv6/IPv6InterfaceData.cc \
	networklayer/ipv6/RoutingTable6.h \
	networklayer/common/InterfaceToken.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/ipv6/IPv6InterfaceData.h \
	networklayer/contract/IPv6Address.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	base/NotifierConsts.h \
	networklayer/ipv6/RoutingTable6Access.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/ipv6/IPv6ExtensionHeaders_m.o: networklayer/ipv6/IPv6ExtensionHeaders_m.cc \
	networklayer/ipv6/IPv6Datagram.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv6/IPv6ExtensionHeaders_m.h
$O/networklayer/ipv6/contract/IPv6ControlInfo.o: networklayer/ipv6/contract/IPv6ControlInfo.cc \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/ipv6/IPv6Datagram.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/contract/IPProtocolId_m.h
$O/networklayer/ipv6/contract/IPv6ControlInfo_m.o: networklayer/ipv6/contract/IPv6ControlInfo_m.cc \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPProtocolId_m.h
$O/networklayer/ipv6tunneling/IPv6Tunneling.o: networklayer/ipv6tunneling/IPv6Tunneling.cc \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/ipv6/IPv6InterfaceData.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceToken.h \
	networklayer/xmipv6/xMIPv6Access.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/ipv6/IPv6ExtensionHeaders.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/xmipv6/MobilityHeader_m.h \
	networklayer/ipv6tunneling/IPv6Tunneling.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/xmipv6/BindingUpdateList.h \
	networklayer/xmipv6/xMIPv6.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	base/NotifierConsts.h \
	networklayer/ipv6/RoutingTable6.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/ipv6/IPv6Datagram.h \
	networklayer/xmipv6/MobilityHeader.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/ipv6/RoutingTable6Access.h \
	networklayer/ipv6/IPv6ExtensionHeaders_m.h
$O/networklayer/ldp/LDPPacket_m.o: networklayer/ldp/LDPPacket_m.cc \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ldp/LDPPacket_m.h
$O/networklayer/ldp/LDP.o: networklayer/ldp/LDP.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/rsvp_te/IntServ.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/mpls/LIBTable.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/mpls/IClassifier.h \
	networklayer/ted/TEDAccess.h \
	linklayer/contract/MACAddress.h \
	networklayer/ted/TED.h \
	transport/tcp_common/contract/TCPSocket.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/rsvp_te/IntServ_m.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	transport/tcp_common/contract/TCPSocketMap.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ldp/LDP.h \
	base/ByteArray_m.h \
	base/ByteArray.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	transport/udp/UDPPacket_m.h \
	networklayer/mpls/ConstType.h \
	networklayer/mpls/LIBTableAccess.h \
	networklayer/ted/TED_m.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/ipv4/IPv4Datagram.h \
	transport/udp/UDPPacket.h \
	transport/udp/contract/UDPSocket.h \
	transport/udp/contract/UDPControlInfo_m.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ldp/LDPPacket_m.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/aodv/aodv_msg_struct.o: networklayer/manetrouting/aodv/aodv_msg_struct.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	base/ProtocolMap.h \
	networklayer/common/InterfaceTable.h \
	networklayer/manetrouting/aodv/aodv_msg_struct.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/ipv4/RoutingTable.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	transport/udp/UDPPacket_m.h \
	networklayer/ipv4/ICMP.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/ipv4/IPv4Datagram.h \
	transport/udp/UDPPacket.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/aodv/packet_queue_omnet.o: networklayer/manetrouting/aodv/packet_queue_omnet.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/aodv/aodv-uu/seek_list.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/common/InterfaceToken.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rrep.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/manetrouting/aodv/packet_queue_omnet.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/aodv/aodv_uu_omnet.h \
	networklayer/manetrouting/aodv/aodv-uu/params.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_hello.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/aodv/aodv-uu/debug_aodv.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.h \
	networklayer/manetrouting/aodv/aodv_msg_struct.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/aodv/aodv-uu/routing_table.h \
	networklayer/contract/IPv4Address.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/aodv/aodv-uu/list.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_neighbor.h \
	networklayer/manetrouting/aodv/aodv-uu/locality.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rerr.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_socket.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_timeout.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rreq.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/manetrouting/aodv/aodv-uu/defs_aodv.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/aodv/aodv_uu_omnet.o: networklayer/manetrouting/aodv/aodv_uu_omnet.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/aodv/aodv-uu/seek_list.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/manetrouting/aodv/packet_queue_omnet.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_hello.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/aodv/aodv-uu/debug_aodv.h \
	networklayer/manetrouting/aodv/aodv_msg_struct.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/aodv/aodv-uu/locality.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_socket.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rreq.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_timeout.h \
	networklayer/manetrouting/aodv/aodv-uu/defs_aodv.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rrep.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/manetrouting/aodv/aodv_uu_omnet.h \
	networklayer/manetrouting/aodv/aodv-uu/params.h \
	base/NotificationBoard.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	base/ProtocolMap.h \
	networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/aodv/aodv-uu/routing_table.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	networklayer/contract/IPv4Address.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	networklayer/manetrouting/aodv/aodv-uu/list.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rerr.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_neighbor.h \
	transport/udp/UDPPacket_m.h \
	networklayer/ipv4/ICMP.h \
	transport/udp/UDPPacket.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/aodv/aodv-uu/aodv_rerr.o: networklayer/manetrouting/aodv/aodv-uu/aodv_rerr.cc \
	networklayer/manetrouting/aodv/aodv-uu/debug_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.h \
	util/uint128.h \
	networklayer/manetrouting/aodv/aodv-uu/routing_table.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/aodv/aodv-uu/list.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rerr.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_socket.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_timeout.h \
	networklayer/manetrouting/aodv/aodv-uu/defs_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/params.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h
$O/networklayer/manetrouting/aodv/aodv-uu/aodv_rrep.o: networklayer/manetrouting/aodv/aodv-uu/aodv_rrep.cc \
	networklayer/manetrouting/aodv/aodv-uu/debug_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/aodv/aodv-uu/routing_table.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/aodv/aodv-uu/list.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_neighbor.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rerr.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_socket.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rrep.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_timeout.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/manetrouting/aodv/aodv-uu/defs_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/params.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_hello.h
$O/networklayer/manetrouting/aodv/aodv-uu/aodv_rreq.o: networklayer/manetrouting/aodv/aodv-uu/aodv_rreq.cc \
	networklayer/manetrouting/aodv/aodv-uu/debug_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/seek_list.h \
	networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/aodv/aodv-uu/routing_table.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/aodv/aodv-uu/list.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rerr.h \
	networklayer/manetrouting/aodv/aodv-uu/locality.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_socket.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rrep.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rreq.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_timeout.h \
	networklayer/manetrouting/aodv/aodv-uu/params.h \
	networklayer/manetrouting/aodv/aodv-uu/defs_aodv.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h
$O/networklayer/manetrouting/aodv/aodv-uu/aodv_neighbor.o: networklayer/manetrouting/aodv/aodv-uu/aodv_neighbor.cc \
	networklayer/manetrouting/aodv/aodv-uu/debug_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/aodv/aodv-uu/routing_table.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/aodv/aodv-uu/list.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_neighbor.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rerr.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_socket.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rrep.h \
	networklayer/manetrouting/aodv/aodv-uu/params.h \
	networklayer/manetrouting/aodv/aodv-uu/defs_aodv.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_hello.h
$O/networklayer/manetrouting/aodv/aodv-uu/routing_table.o: networklayer/manetrouting/aodv/aodv-uu/routing_table.cc \
	networklayer/manetrouting/aodv/aodv-uu/debug_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/seek_list.h \
	networklayer/manetrouting/aodv/aodv-uu/nl.h \
	networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/aodv/aodv-uu/routing_table.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/aodv/aodv-uu/list.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rerr.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_neighbor.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_socket.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rrep.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_timeout.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/manetrouting/aodv/aodv-uu/defs_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/params.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_hello.h
$O/networklayer/manetrouting/aodv/aodv-uu/locality.o: networklayer/manetrouting/aodv/aodv-uu/locality.cc \
	networklayer/manetrouting/aodv/aodv-uu/debug_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/manetrouting/aodv/aodv-uu/defs_aodv.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/aodv/aodv-uu/list.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/aodv/aodv-uu/locality.h
$O/networklayer/manetrouting/aodv/aodv-uu/aodv_socket.o: networklayer/manetrouting/aodv/aodv-uu/aodv_socket.cc \
	networklayer/manetrouting/aodv/aodv-uu/debug_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/seek_list.h \
	networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/aodv/aodv-uu/routing_table.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/aodv/aodv-uu/list.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rerr.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_neighbor.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_socket.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rrep.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rreq.h \
	networklayer/manetrouting/aodv/aodv-uu/params.h \
	networklayer/manetrouting/aodv/aodv-uu/defs_aodv.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_hello.h
$O/networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.o: networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.cc \
	networklayer/manetrouting/aodv/aodv-uu/debug_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/manetrouting/aodv/aodv-uu/defs_aodv.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/aodv/aodv-uu/list.h \
	linklayer/contract/MACAddress.h
$O/networklayer/manetrouting/aodv/aodv-uu/aodv_timeout.o: networklayer/manetrouting/aodv/aodv-uu/aodv_timeout.cc \
	networklayer/manetrouting/aodv/aodv-uu/debug_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/seek_list.h \
	networklayer/manetrouting/aodv/aodv-uu/nl.h \
	networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/aodv/aodv-uu/routing_table.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/aodv/aodv-uu/list.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_neighbor.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rerr.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_socket.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rrep.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_timeout.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rreq.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/manetrouting/aodv/aodv-uu/defs_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/params.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_hello.h
$O/networklayer/manetrouting/aodv/aodv-uu/debug_aodv.o: networklayer/manetrouting/aodv/aodv-uu/debug_aodv.cc \
	networklayer/manetrouting/aodv/aodv-uu/debug_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/seek_list.h \
	networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.h \
	util/uint128.h \
	networklayer/manetrouting/aodv/aodv-uu/routing_table.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/aodv/aodv-uu/list.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rerr.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rrep.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rreq.h \
	networklayer/manetrouting/aodv/aodv-uu/defs_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/params.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h
$O/networklayer/manetrouting/aodv/aodv-uu/seek_list.o: networklayer/manetrouting/aodv/aodv-uu/seek_list.cc \
	networklayer/manetrouting/aodv/aodv-uu/debug_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/seek_list.h \
	networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/aodv/aodv-uu/list.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_timeout.h \
	networklayer/manetrouting/aodv/aodv-uu/defs_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/params.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h
$O/networklayer/manetrouting/aodv/aodv-uu/aodv_hello.o: networklayer/manetrouting/aodv/aodv-uu/aodv_hello.cc \
	networklayer/manetrouting/aodv/aodv-uu/debug_aodv.h \
	networklayer/manetrouting/aodv/aodv-uu/seek_list.h \
	networklayer/manetrouting/aodv/aodv-uu/timer_queue_aodv.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/aodv/aodv-uu/routing_table.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/aodv/aodv-uu/list.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rerr.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_socket.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rrep.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_timeout.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_rreq.h \
	networklayer/manetrouting/aodv/aodv-uu/params.h \
	networklayer/manetrouting/aodv/aodv-uu/defs_aodv.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/aodv/aodv-uu/aodv_hello.h
$O/networklayer/manetrouting/aodv/aodv-uu/list.o: networklayer/manetrouting/aodv/aodv-uu/list.cc \
	networklayer/manetrouting/aodv/aodv-uu/list.h
$O/networklayer/manetrouting/base/ManetRoutingBase.o: networklayer/manetrouting/base/ManetRoutingBase.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	util/FWMath.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	util/uint128.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/common/InterfaceTableAccess.h \
	base/Coord.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	transport/udp/UDPPacket_m.h \
	networklayer/ipv4/ICMP.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	transport/udp/UDPPacket.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/base/ControlInfoBreakLink_m.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/base/ControlManetRouting_m.o: networklayer/manetrouting/base/ControlManetRouting_m.cc \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	linklayer/contract/MACAddress.h
$O/networklayer/manetrouting/base/MeshControlInfo_m.o: networklayer/manetrouting/base/MeshControlInfo_m.cc \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/base/MeshControlInfo_m.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h
$O/networklayer/manetrouting/base/ControlInfoBreakLink_m.o: networklayer/manetrouting/base/ControlInfoBreakLink_m.cc \
	base/INETDefs.h \
	networklayer/manetrouting/base/ControlInfoBreakLink_m.h \
	linklayer/contract/MACAddress.h
$O/networklayer/manetrouting/batman/BatmanMain.o: networklayer/manetrouting/batman/BatmanMain.cc \
	networklayer/manetrouting/batman/BatmanData.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/manetrouting/batman/BatmanMsg_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	transport/udp/UDPPacket_m.h \
	networklayer/manetrouting/batman/batman.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/batman/batmanHna.o: networklayer/manetrouting/batman/batmanHna.cc \
	networklayer/manetrouting/batman/BatmanData.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/manetrouting/batman/BatmanMsg_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	util/uint128.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/batman/batman.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/batman/BatmanData.o: networklayer/manetrouting/batman/BatmanData.cc \
	networklayer/manetrouting/batman/BatmanData.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/manetrouting/batman/BatmanMsg_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	util/uint128.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/batman/batman.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/batman/BatmanMsg_m.o: networklayer/manetrouting/batman/BatmanMsg_m.cc \
	networklayer/manetrouting/batman/BatmanMsg_m.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	linklayer/contract/MACAddress.h
$O/networklayer/manetrouting/dsdv/DSDV_2.o: networklayer/manetrouting/dsdv/DSDV_2.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/ipv4/RoutingTable.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsdv/DSDVhello_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsdv/DSDV_2.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dsdv/DSDVhello_m.o: networklayer/manetrouting/dsdv/DSDVhello_m.cc \
	networklayer/manetrouting/dsdv/DSDVhello_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h
$O/networklayer/manetrouting/dsr/dsr-pkt_omnet.o: networklayer/manetrouting/dsr/dsr-pkt_omnet.cc \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h
$O/networklayer/manetrouting/dsr/dsr-uu-omnetpp.o: networklayer/manetrouting/dsr/dsr-uu-omnetpp.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/dsr/dsr-uu/maint-buf.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu-omnetpp.h \
	networklayer/common/InterfaceToken.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-ack.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/tbl.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-io.h \
	networklayer/manetrouting/dsr/dsr-uu/debug_dsr.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	networklayer/manetrouting/dsr/dsr-uu/ns-agent.h \
	base/Blackboard.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INETDefs.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-srt.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	base/NotificationBoard.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	base/ProtocolMap.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr_list.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/manetrouting/dsr/dsr-uu/neigh.h \
	networklayer/manetrouting/dsr/dsr-uu/link-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/path-cache.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dsr/dsr-uu/send-buf.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.o: networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/dsr/dsr-uu/maint-buf.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu-omnetpp.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-ack.h \
	networklayer/manetrouting/dsr/dsr-uu/tbl.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-io.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/debug_dsr.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	networklayer/manetrouting/dsr/dsr-uu/ns-agent.h \
	base/Blackboard.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-srt.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-dev.h \
	base/NotificationBoard.h \
	base/ProtocolMap.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr_list.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dsr/dsr-uu/link-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/neigh.h \
	networklayer/manetrouting/dsr/dsr-uu/path-cache.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dsr/dsr-uu/send-buf.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.o: networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/dsr/dsr-uu/maint-buf.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu-omnetpp.h \
	networklayer/common/InterfaceToken.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-ack.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/tbl.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-io.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dsr/dsr-uu/debug_dsr.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	networklayer/manetrouting/dsr/dsr-uu/ns-agent.h \
	base/Blackboard.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-srt.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-dev.h \
	base/NotificationBoard.h \
	base/ProtocolMap.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr_list.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dsr/dsr-uu/link-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/neigh.h \
	networklayer/manetrouting/dsr/dsr-uu/path-cache.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dsr/dsr-uu/send-buf.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.o: networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/dsr/dsr-uu/maint-buf.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu-omnetpp.h \
	networklayer/common/InterfaceToken.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-ack.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.h \
	networklayer/manetrouting/dsr/dsr-uu/tbl.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-io.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/debug_dsr.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	networklayer/manetrouting/dsr/dsr-uu/ns-agent.h \
	base/Blackboard.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-srt.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-dev.h \
	base/NotificationBoard.h \
	base/ProtocolMap.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr_list.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dsr/dsr-uu/neigh.h \
	networklayer/manetrouting/dsr/dsr-uu/link-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/path-cache.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dsr/dsr-uu/send-buf.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dsr/dsr-uu/dsr-opt.o: networklayer/manetrouting/dsr/dsr-uu/dsr-opt.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/dsr/dsr-uu/maint-buf.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu-omnetpp.h \
	networklayer/common/InterfaceToken.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-ack.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/tbl.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-io.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dsr/dsr-uu/debug_dsr.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	networklayer/manetrouting/dsr/dsr-uu/ns-agent.h \
	base/Blackboard.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-srt.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	base/NotificationBoard.h \
	base/ProtocolMap.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr_list.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dsr/dsr-uu/neigh.h \
	networklayer/manetrouting/dsr/dsr-uu/link-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/path-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/ipv4/ICMP.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dsr/dsr-uu/send-buf.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dsr/dsr-uu/link-cache.o: networklayer/manetrouting/dsr/dsr-uu/link-cache.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/dsr/dsr-uu/maint-buf.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu-omnetpp.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-ack.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	networklayer/manetrouting/dsr/dsr-uu/tbl.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-io.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/debug_dsr.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	networklayer/manetrouting/dsr/dsr-uu/ns-agent.h \
	base/Blackboard.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rtc.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-srt.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	base/NotificationBoard.h \
	base/ProtocolMap.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr_list.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dsr/dsr-uu/neigh.h \
	networklayer/manetrouting/dsr/dsr-uu/link-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/path-cache.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dsr/dsr-uu/send-buf.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.o: networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/dsr/dsr-uu/maint-buf.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu-omnetpp.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-ack.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	networklayer/manetrouting/dsr/dsr-uu/tbl.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-io.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/debug_dsr.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	networklayer/manetrouting/dsr/dsr-uu/ns-agent.h \
	base/Blackboard.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-srt.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	base/NotificationBoard.h \
	base/ProtocolMap.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr_list.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/manetrouting/dsr/dsr-uu/link-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/neigh.h \
	networklayer/manetrouting/dsr/dsr-uu/path-cache.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dsr/dsr-uu/send-buf.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dsr/dsr-uu/send-buf.o: networklayer/manetrouting/dsr/dsr-uu/send-buf.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/dsr/dsr-uu/maint-buf.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu-omnetpp.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-ack.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	networklayer/manetrouting/dsr/dsr-uu/tbl.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-io.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/debug_dsr.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	networklayer/manetrouting/dsr/dsr-uu/ns-agent.h \
	base/Blackboard.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-srt.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	base/NotificationBoard.h \
	base/ProtocolMap.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr_list.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dsr/dsr-uu/link-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/neigh.h \
	networklayer/manetrouting/dsr/dsr-uu/path-cache.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dsr/dsr-uu/send-buf.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dsr/dsr-uu/dsr-srt.o: networklayer/manetrouting/dsr/dsr-uu/dsr-srt.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/dsr/dsr-uu/maint-buf.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu-omnetpp.h \
	networklayer/common/InterfaceToken.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-ack.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/tbl.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-io.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dsr/dsr-uu/debug_dsr.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	networklayer/manetrouting/dsr/dsr-uu/ns-agent.h \
	base/Blackboard.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-srt.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	base/NotificationBoard.h \
	base/ProtocolMap.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr_list.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dsr/dsr-uu/link-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/neigh.h \
	networklayer/manetrouting/dsr/dsr-uu/path-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/ipv4/ICMP.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dsr/dsr-uu/send-buf.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dsr/dsr-uu/dsr-ack.o: networklayer/manetrouting/dsr/dsr-uu/dsr-ack.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/dsr/dsr-uu/maint-buf.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu-omnetpp.h \
	networklayer/common/InterfaceToken.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-ack.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/tbl.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-io.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dsr/dsr-uu/debug_dsr.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	networklayer/manetrouting/dsr/dsr-uu/ns-agent.h \
	base/Blackboard.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-srt.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-dev.h \
	base/NotificationBoard.h \
	base/ProtocolMap.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr_list.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dsr/dsr-uu/link-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/neigh.h \
	networklayer/manetrouting/dsr/dsr-uu/path-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/ipv4/ICMP.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dsr/dsr-uu/send-buf.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dsr/dsr-uu/maint-buf.o: networklayer/manetrouting/dsr/dsr-uu/maint-buf.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/dsr/dsr-uu/maint-buf.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu-omnetpp.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-ack.h \
	networklayer/manetrouting/dsr/dsr-uu/tbl.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-io.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/debug_dsr.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	networklayer/manetrouting/dsr/dsr-uu/ns-agent.h \
	base/Blackboard.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-srt.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-dev.h \
	base/NotificationBoard.h \
	base/ProtocolMap.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr_list.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dsr/dsr-uu/neigh.h \
	networklayer/manetrouting/dsr/dsr-uu/link-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/path-cache.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dsr/dsr-uu/send-buf.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dsr/dsr-uu/path-cache.o: networklayer/manetrouting/dsr/dsr-uu/path-cache.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/dsr/dsr-uu/maint-buf.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu-omnetpp.h \
	networklayer/common/InterfaceToken.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-ack.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/tbl.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-io.h \
	networklayer/manetrouting/dsr/dsr-uu/debug_dsr.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	networklayer/manetrouting/dsr/dsr-uu/ns-agent.h \
	base/Blackboard.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rtc.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-srt.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	base/NotificationBoard.h \
	base/ProtocolMap.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr_list.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dsr/dsr-uu/neigh.h \
	networklayer/manetrouting/dsr/dsr-uu/link-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/path-cache.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dsr/dsr-uu/send-buf.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dsr/dsr-uu/dsr-io.o: networklayer/manetrouting/dsr/dsr-uu/dsr-io.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/dsr/dsr-uu/maint-buf.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu-omnetpp.h \
	networklayer/common/InterfaceToken.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-ack.h \
	networklayer/manetrouting/dsr/dsr-uu/tbl.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-io.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/debug_dsr.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	networklayer/manetrouting/dsr/dsr-uu/ns-agent.h \
	base/Blackboard.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rtc.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-srt.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-dev.h \
	base/NotificationBoard.h \
	base/ProtocolMap.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr_list.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dsr/dsr-uu/link-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/neigh.h \
	networklayer/manetrouting/dsr/dsr-uu/path-cache.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dsr/dsr-uu/send-buf.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dsr/dsr-uu/neigh.o: networklayer/manetrouting/dsr/dsr-uu/neigh.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/dsr/dsr-uu/maint-buf.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rerr.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dsr/dsr-uu-omnetpp.h \
	networklayer/manetrouting/base/compatibility_dsr.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-ack.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rrep.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr.h \
	networklayer/manetrouting/dsr/dsr-uu/tbl.h \
	base/ModuleAccess.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-io.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dsr/dsr-uu/debug_dsr.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dsr/dsr-pkt_omnet.h \
	networklayer/manetrouting/dsr/dsr-uu/ns-agent.h \
	base/Blackboard.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/manetrouting/dsr/dsr-uu/timer.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-srt.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	base/NotificationBoard.h \
	base/ProtocolMap.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-rreq.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr_list.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dsr/dsr-uu/neigh.h \
	networklayer/manetrouting/dsr/dsr-uu/link-cache.h \
	networklayer/manetrouting/dsr/dsr-uu/path-cache.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-opt.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dsr/dsr-uu/send-buf.h \
	networklayer/manetrouting/dsr/dsr-uu/dsr-pkt.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dymo/dymo_packet_queue_omnet.o: networklayer/manetrouting/dymo/dymo_packet_queue_omnet.cc \
	networklayer/manetrouting/dymo/dymoum/icmp_socket.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/dymo/dymo_packet_queue_omnet.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dymo/dymoum/dymo_generic.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dymo/dymo_msg_struct.h \
	networklayer/manetrouting/dymo/dymoum/dymo_uerr.h \
	networklayer/manetrouting/dymo/dymoum/blacklist.h \
	networklayer/manetrouting/dymo/dymoum/dymo_nb.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/manetrouting/dymo/dymoum/dymo_rerr.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/manetrouting/dymo/dymoum/dymo_socket.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/manetrouting/dymo/dymoum/dymo_re.h \
	networklayer/manetrouting/dymo/dymoum/dymo_hello.h \
	base/NotificationBoard.h \
	networklayer/manetrouting/dymo/dymoum/pending_rreq.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	util/uint128.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/contract/IPv4Address.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/manetrouting/dymo/dymoum/rtable.h \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/dymo/dymo_um_omnet.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/manetrouting/dymo/dymoum/dymo_timeout.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dymo/dymo_um_omnet.o: networklayer/manetrouting/dymo/dymo_um_omnet.cc \
	networklayer/manetrouting/dymo/dymoum/icmp_socket.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/manetrouting/dymo/dymo_packet_queue_omnet.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dymo/dymoum/dymo_generic.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dymo/dymo_msg_struct.h \
	networklayer/manetrouting/dymo/dymoum/dymo_uerr.h \
	networklayer/manetrouting/dymo/dymoum/blacklist.h \
	networklayer/manetrouting/dymo/dymoum/dymo_nb.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/manetrouting/dymo/dymoum/dymo_rerr.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo/dymoum/dymo_socket.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/manetrouting/dymo/dymoum/dymo_re.h \
	networklayer/manetrouting/dymo/dymoum/dymo_hello.h \
	base/NotificationBoard.h \
	networklayer/manetrouting/dymo/dymoum/pending_rreq.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	base/ProtocolMap.h \
	util/uint128.h \
	networklayer/ipv4/RoutingTableAccess.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/manetrouting/dymo/dymoum/rtable.h \
	transport/udp/UDPPacket_m.h \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/ipv4/ICMP.h \
	transport/udp/UDPPacket.h \
	networklayer/manetrouting/dymo/dymo_um_omnet.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/manetrouting/dymo/dymoum/dymo_timeout.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dymo/dymo_msg_struct.o: networklayer/manetrouting/dymo/dymo_msg_struct.cc \
	networklayer/manetrouting/dymo/dymo_msg_struct.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h
$O/networklayer/manetrouting/dymo/dymoum/debug_dymo.o: networklayer/manetrouting/dymo/dymoum/debug_dymo.cc \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h \
	linklayer/contract/MACAddress.h
$O/networklayer/manetrouting/dymo/dymoum/dymo_re.o: networklayer/manetrouting/dymo/dymoum/dymo_re.cc \
	networklayer/manetrouting/dymo/dymoum/icmp_socket.h \
	util/uint128.h \
	networklayer/manetrouting/dymo/dymoum/blacklist.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo/dymoum/dymo_socket.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/manetrouting/dymo/dymoum/rtable.h \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	networklayer/manetrouting/base/MeshControlInfo_m.h \
	networklayer/manetrouting/dymo/dymoum/dymo_re.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/manetrouting/dymo/dymoum/dymo_generic.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h \
	networklayer/manetrouting/dymo/dymoum/dymo_timeout.h \
	networklayer/manetrouting/dymo/dymoum/pending_rreq.h
$O/networklayer/manetrouting/dymo/dymoum/dymo_socket.o: networklayer/manetrouting/dymo/dymoum/dymo_socket.cc \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo/dymoum/dymo_socket.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/dymo/dymoum/rtable.h \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	networklayer/manetrouting/dymo/dymoum/dymo_re.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/manetrouting/dymo/dymoum/dymo_generic.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h \
	networklayer/contract/IPv6Address.h
$O/networklayer/manetrouting/dymo/dymoum/dymo_nb.o: networklayer/manetrouting/dymo/dymoum/dymo_nb.cc \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo/dymoum/dymo_nb.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/dymo/dymoum/dymo_timeout.h
$O/networklayer/manetrouting/dymo/dymoum/pending_rreq.o: networklayer/manetrouting/dymo/dymoum/pending_rreq.cc \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/dymo/dymoum/pending_rreq.h
$O/networklayer/manetrouting/dymo/dymoum/dymo_timeout.o: networklayer/manetrouting/dymo/dymoum/dymo_timeout.cc \
	networklayer/manetrouting/dymo/dymoum/blacklist.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo/dymoum/dymo_nb.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/dymo/dymoum/rtable.h \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	networklayer/manetrouting/dymo/dymoum/dymo_re.h \
	networklayer/manetrouting/dymo/dymoum/dymo_netlink.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h \
	networklayer/manetrouting/dymo/dymoum/dymo_timeout.h \
	networklayer/manetrouting/dymo/dymoum/pending_rreq.h
$O/networklayer/manetrouting/dymo/dymoum/dymo_uerr.o: networklayer/manetrouting/dymo/dymoum/dymo_uerr.cc \
	networklayer/manetrouting/dymo/dymoum/dymo_uerr.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo/dymoum/dymo_socket.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/dymo/dymoum/rtable.h \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	networklayer/manetrouting/dymo/dymoum/dymo_re.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/manetrouting/dymo/dymoum/dymo_generic.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h
$O/networklayer/manetrouting/dymo/dymoum/blacklist.o: networklayer/manetrouting/dymo/dymoum/blacklist.cc \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	util/uint128.h \
	networklayer/manetrouting/dymo/dymoum/blacklist.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h \
	linklayer/contract/MACAddress.h
$O/networklayer/manetrouting/dymo/dymoum/icmp_socket.o: networklayer/manetrouting/dymo/dymoum/icmp_socket.cc \
	networklayer/manetrouting/dymo/dymoum/icmp_socket.h \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	util/uint128.h \
	networklayer/manetrouting/dymo/dymoum/blacklist.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h \
	linklayer/contract/MACAddress.h
$O/networklayer/manetrouting/dymo/dymoum/rtable.o: networklayer/manetrouting/dymo/dymoum/rtable.cc \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	networklayer/manetrouting/dymo/dymoum/k_route.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/dymo/dymoum/rtable.h \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	networklayer/manetrouting/dymo/dymoum/dymo_netlink.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h \
	networklayer/manetrouting/dymo/dymoum/pending_rreq.h \
	networklayer/manetrouting/dymo/dymoum/dymo_timeout.h
$O/networklayer/manetrouting/dymo/dymoum/dymo_generic.o: networklayer/manetrouting/dymo/dymoum/dymo_generic.cc \
	networklayer/manetrouting/dymo/dymoum/dymo_uerr.h \
	networklayer/manetrouting/dymo/dymoum/dymo_rerr.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/dymo/dymoum/rtable.h \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	networklayer/manetrouting/dymo/dymoum/dymo_re.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/manetrouting/dymo/dymoum/dymo_generic.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dymo/dymoum/dymo_hello.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h
$O/networklayer/manetrouting/dymo/dymoum/dymo_hello.o: networklayer/manetrouting/dymo/dymoum/dymo_hello.cc \
	util/uint128.h \
	networklayer/manetrouting/dymo/dymoum/dymo_nb.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	networklayer/manetrouting/dymo/dymoum/dymo_socket.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/dymo/dymoum/rtable.h \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	networklayer/manetrouting/dymo/dymoum/dymo_re.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/manetrouting/dymo/dymoum/dymo_generic.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dymo/dymoum/dymo_hello.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h
$O/networklayer/manetrouting/dymo/dymoum/dymo_rerr.o: networklayer/manetrouting/dymo/dymoum/dymo_rerr.cc \
	networklayer/manetrouting/dymo/dymoum/dymo_rerr.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo/dymoum/dymo_socket.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/dymo/dymoum/rtable.h \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	networklayer/manetrouting/dymo/dymoum/dymo_re.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/manetrouting/dymo/dymoum/dymo_generic.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dymo/dymoum/debug_dymo.h
$O/networklayer/manetrouting/dymo/dymoum/timer_queue.o: networklayer/manetrouting/dymo/dymoum/timer_queue.cc \
	networklayer/manetrouting/dymo/dymoum/dlist.h \
	networklayer/manetrouting/dymo/dymoum/defs_dymo.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo/dymoum/timer_queue.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	linklayer/contract/MACAddress.h
$O/networklayer/manetrouting/dymo_fau/DYMO_RoutingTable.o: networklayer/manetrouting/dymo_fau/DYMO_RoutingTable.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_Packet_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_UERR_m.h \
	base/QueueBase.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/manetrouting/dymo_fau/DYMO_RERR_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_DataQueue.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dymo_fau/DYMO_RoutingTable.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dymo_fau/InspectorHelpers.h \
	base/AbstractQueue.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage_m.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_RoutingEntry.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/manetrouting/dymo_fau/DYMO_OutstandingRREQList.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/dymo_fau/DYMO.h \
	networklayer/manetrouting/dymo_fau/DYMO_RM_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_Timer.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/manetrouting/dymo_fau/DYMO_AddressBlock.h \
	base/NotificationBoard.h \
	networklayer/ipv4/IPv4.h \
	networklayer/manetrouting/dymo_fau/DYMO_TokenBucket.h \
	base/ProtocolMap.h \
	networklayer/common/InterfaceTable.h \
	util/uint128.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/ipv4/RoutingTable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dymo_fau/DYMO_RREQ_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_Timeout_m.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/ipv4/IPv4FragBuf.h \
	networklayer/ipv4/ICMP.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/manetrouting/dymo_fau/DYMO_RREP_m.h \
	base/ReassemblyBuffer.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dymo_fau/DYMO_Packet_m.o: networklayer/manetrouting/dymo_fau/DYMO_Packet_m.cc \
	networklayer/manetrouting/dymo_fau/DYMO_Packet_m.h
$O/networklayer/manetrouting/dymo_fau/DYMO_RREP_m.o: networklayer/manetrouting/dymo_fau/DYMO_RREP_m.cc \
	networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_RERR_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_AddressBlock.h \
	networklayer/manetrouting/dymo_fau/DYMO_RREP_m.h \
	networklayer/manetrouting/dymo_fau/InspectorHelpers.h \
	networklayer/manetrouting/dymo_fau/DYMO_RM_m.h
$O/networklayer/manetrouting/dymo_fau/DYMO_RREQ_m.o: networklayer/manetrouting/dymo_fau/DYMO_RREQ_m.cc \
	networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_RERR_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_AddressBlock.h \
	networklayer/manetrouting/dymo_fau/DYMO_RREQ_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_RM_m.h \
	networklayer/manetrouting/dymo_fau/InspectorHelpers.h
$O/networklayer/manetrouting/dymo_fau/DYMO_Timeout_m.o: networklayer/manetrouting/dymo_fau/DYMO_Timeout_m.cc \
	networklayer/manetrouting/dymo_fau/DYMO_Timeout_m.h
$O/networklayer/manetrouting/dymo_fau/DYMO_UERR_m.o: networklayer/manetrouting/dymo_fau/DYMO_UERR_m.cc \
	networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_UERR_m.h
$O/networklayer/manetrouting/dymo_fau/DYMO_TokenBucket.o: networklayer/manetrouting/dymo_fau/DYMO_TokenBucket.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_UERR_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_Packet_m.h \
	base/QueueBase.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/manetrouting/dymo_fau/DYMO_RERR_m.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dymo_fau/DYMO_DataQueue.h \
	networklayer/manetrouting/dymo_fau/DYMO_RoutingTable.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dymo_fau/InspectorHelpers.h \
	networklayer/common/InterfaceEntry.h \
	base/AbstractQueue.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage_m.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_RoutingEntry.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/manetrouting/dymo_fau/DYMO_OutstandingRREQList.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/dymo_fau/DYMO_RM_m.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/manetrouting/dymo_fau/DYMO_Timer.h \
	networklayer/manetrouting/dymo_fau/DYMO_AddressBlock.h \
	base/NotificationBoard.h \
	networklayer/ipv4/IPv4.h \
	networklayer/manetrouting/dymo_fau/DYMO_TokenBucket.h \
	base/ProtocolMap.h \
	networklayer/common/InterfaceTable.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/ipv4/RoutingTable.h \
	util/uint128.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dymo_fau/DYMO_Timeout_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_RREQ_m.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/ipv4/IPv4FragBuf.h \
	networklayer/ipv4/ICMP.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/dymo_fau/DYMO_RREP_m.h \
	base/ReassemblyBuffer.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dymo_fau/DYMO_RoutingEntry.o: networklayer/manetrouting/dymo_fau/DYMO_RoutingEntry.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_Packet_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_UERR_m.h \
	base/QueueBase.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/manetrouting/dymo_fau/DYMO_RERR_m.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dymo_fau/DYMO_DataQueue.h \
	networklayer/manetrouting/dymo_fau/DYMO_RoutingTable.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/manetrouting/dymo_fau/InspectorHelpers.h \
	base/AbstractQueue.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage_m.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_RoutingEntry.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo_fau/DYMO_OutstandingRREQList.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/dymo_fau/DYMO.h \
	networklayer/manetrouting/dymo_fau/DYMO_RM_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_Timer.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/manetrouting/dymo_fau/DYMO_AddressBlock.h \
	base/NotificationBoard.h \
	networklayer/ipv4/IPv4.h \
	networklayer/manetrouting/dymo_fau/DYMO_TokenBucket.h \
	base/ProtocolMap.h \
	networklayer/common/InterfaceTable.h \
	util/uint128.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/ipv4/RoutingTable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dymo_fau/DYMO_RREQ_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_Timeout_m.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/ipv4/IPv4FragBuf.h \
	networklayer/ipv4/ICMP.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/manetrouting/dymo_fau/DYMO_RREP_m.h \
	base/ReassemblyBuffer.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dymo_fau/DYMO.o: networklayer/manetrouting/dymo_fau/DYMO.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_Packet_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_UERR_m.h \
	base/QueueBase.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/manetrouting/dymo_fau/DYMO_RERR_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_DataQueue.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dymo_fau/DYMO_RoutingTable.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/manetrouting/dymo_fau/InspectorHelpers.h \
	networklayer/common/InterfaceEntry.h \
	base/AbstractQueue.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage_m.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/manetrouting/base/compatibility.h \
	transport/udp/contract/UDPSocket.h \
	transport/udp/contract/UDPControlInfo_m.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_RoutingEntry.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/manetrouting/dymo_fau/DYMO_OutstandingRREQList.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/dymo_fau/DYMO.h \
	networklayer/manetrouting/dymo_fau/DYMO_RM_m.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/manetrouting/dymo_fau/DYMO_Timer.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/manetrouting/dymo_fau/DYMO_AddressBlock.h \
	base/NotificationBoard.h \
	networklayer/ipv4/IPv4.h \
	networklayer/manetrouting/dymo_fau/DYMO_TokenBucket.h \
	base/ProtocolMap.h \
	networklayer/common/InterfaceTable.h \
	networklayer/ipv4/RoutingTable.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/manetrouting/dymo_fau/DYMO_Timeout_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_RREQ_m.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	transport/udp/UDPPacket_m.h \
	networklayer/ipv4/IPv4FragBuf.h \
	networklayer/ipv4/ICMP.h \
	transport/udp/UDPPacket.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/manetrouting/dymo_fau/DYMO_RREP_m.h \
	base/ReassemblyBuffer.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dymo_fau/DYMO_RM_m.o: networklayer/manetrouting/dymo_fau/DYMO_RM_m.cc \
	networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_RERR_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_AddressBlock.h \
	networklayer/manetrouting/dymo_fau/InspectorHelpers.h \
	networklayer/manetrouting/dymo_fau/DYMO_RM_m.h
$O/networklayer/manetrouting/dymo_fau/DYMO_RERR_m.o: networklayer/manetrouting/dymo_fau/DYMO_RERR_m.cc \
	networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_RERR_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_AddressBlock.h \
	networklayer/manetrouting/dymo_fau/DYMO_RM_m.h \
	networklayer/manetrouting/dymo_fau/InspectorHelpers.h
$O/networklayer/manetrouting/dymo_fau/DYMO_OutstandingRREQList.o: networklayer/manetrouting/dymo_fau/DYMO_OutstandingRREQList.cc \
	networklayer/manetrouting/dymo_fau/DYMO_Timer.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/manetrouting/dymo_fau/DYMO_OutstandingRREQList.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/manetrouting/dymo_fau/DYMO_Timeout_m.h
$O/networklayer/manetrouting/dymo_fau/InspectorHelpers.o: networklayer/manetrouting/dymo_fau/InspectorHelpers.cc \
	networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_RERR_m.h \
	networklayer/manetrouting/dymo_fau/DYMO_AddressBlock.h \
	networklayer/manetrouting/dymo_fau/DYMO_RM_m.h \
	networklayer/manetrouting/dymo_fau/InspectorHelpers.h
$O/networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage_m.o: networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage_m.cc \
	networklayer/manetrouting/dymo_fau/DYMO_PacketBBMessage_m.h
$O/networklayer/manetrouting/dymo_fau/DYMO_DataQueue.o: networklayer/manetrouting/dymo_fau/DYMO_DataQueue.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	base/QueueBase.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/manetrouting/dymo_fau/DYMO_DataQueue.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ipv4/IPv4.h \
	base/ProtocolMap.h \
	networklayer/common/InterfaceTable.h \
	base/AbstractQueue.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/ipv4/RoutingTable.h \
	util/uint128.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/ipv4/IPv4FragBuf.h \
	networklayer/ipv4/ICMP.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/ICMPAccess.h \
	base/ReassemblyBuffer.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/dymo_fau/DYMO_Timer.o: networklayer/manetrouting/dymo_fau/DYMO_Timer.cc \
	networklayer/manetrouting/dymo_fau/DYMO_Timer.h \
	networklayer/manetrouting/dymo_fau/DYMO_Timeout_m.h
$O/networklayer/manetrouting/olsr/OLSR_state.o: networklayer/manetrouting/olsr/OLSR_state.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/olsr/OLSRpkt_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/olsr/OLSR_state.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/manetrouting/olsr/OLSR_rtable.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	util/uint128.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/olsr/OLSR.h \
	networklayer/manetrouting/olsr/OLSR_ETX_parameter.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/olsr/OLSR_repositories.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/olsr/OLSR_rtable.o: networklayer/manetrouting/olsr/OLSR_rtable.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/olsr/OLSRpkt_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/olsr/OLSR_state.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/manetrouting/olsr/OLSR_rtable.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	util/uint128.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/manetrouting/olsr/OLSR.h \
	networklayer/manetrouting/olsr/OLSR_ETX_parameter.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/olsr/OLSR_repositories.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/olsr/OLSR_ETX_dijkstra.o: networklayer/manetrouting/olsr/OLSR_ETX_dijkstra.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/olsr/OLSRpkt_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/olsr/OLSR_ETX_repositories.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/olsr/OLSR_state.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/manetrouting/olsr/OLSR_rtable.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/olsr/OLSR_ETX.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/manetrouting/olsr/OLSR.h \
	networklayer/manetrouting/olsr/OLSR_ETX_parameter.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/olsr/OLSR_ETX_state.h \
	networklayer/manetrouting/olsr/OLSR_ETX_dijkstra.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/manetrouting/olsr/OLSR_repositories.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/olsr/OLSR.o: networklayer/manetrouting/olsr/OLSR.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/olsr/OLSRpkt_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/olsr/OLSR_state.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/manetrouting/olsr/OLSR_rtable.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/manetrouting/olsr/OLSR.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/manetrouting/olsr/OLSR_ETX_parameter.h \
	transport/udp/UDPPacket_m.h \
	networklayer/ipv4/ICMP.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	transport/udp/UDPPacket.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/manetrouting/olsr/OLSR_repositories.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/olsr/OLSRpkt_m.o: networklayer/manetrouting/olsr/OLSRpkt_m.cc \
	networklayer/manetrouting/olsr/OLSR_ETX_parameter.h \
	networklayer/manetrouting/olsr/OLSRpkt_m.h \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	linklayer/contract/MACAddress.h
$O/networklayer/manetrouting/olsr/OLSR_ETX.o: networklayer/manetrouting/olsr/OLSR_ETX.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/olsr/OLSRpkt_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/olsr/OLSR_ETX_repositories.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/olsr/OLSR_state.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/manetrouting/olsr/OLSR_rtable.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/olsr/OLSR_ETX.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/manetrouting/olsr/OLSR.h \
	networklayer/manetrouting/olsr/OLSR_ETX_parameter.h \
	transport/udp/UDPPacket_m.h \
	networklayer/ipv4/ICMP.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/manetrouting/olsr/OLSR_ETX_state.h \
	networklayer/manetrouting/olsr/OLSR_ETX_dijkstra.h \
	transport/udp/UDPPacket.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/olsr/OLSR_repositories.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/manetrouting/olsr/OLSR_ETX_state.o: networklayer/manetrouting/olsr/OLSR_ETX_state.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/manetrouting/olsr/OLSRpkt_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/manetrouting/olsr/OLSR_ETX_repositories.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/manetrouting/olsr/OLSR_state.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/manetrouting/olsr/OLSR_rtable.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/manetrouting/olsr/OLSR_ETX.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/manetrouting/olsr/OLSR.h \
	networklayer/manetrouting/olsr/OLSR_ETX_parameter.h \
	networklayer/ipv4/ICMP.h \
	networklayer/manetrouting/olsr/OLSR_ETX_state.h \
	networklayer/manetrouting/base/compatibility.h \
	networklayer/contract/IPv6Address.h \
	networklayer/manetrouting/olsr/OLSR_repositories.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/manetrouting/base/ManetRoutingBase.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/mpls/MPLSPacket.o: networklayer/mpls/MPLSPacket.cc \
	base/INETDefs.h \
	networklayer/mpls/MPLSPacket.h
$O/networklayer/mpls/LIBTable.o: networklayer/mpls/LIBTable.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/mpls/LIBTable.h \
	networklayer/ipv4/RoutingTableAccess.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	util/XMLUtils.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/mpls/ConstType.h \
	networklayer/ipv4/IPv4Datagram.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/mpls/MPLS.o: networklayer/mpls/MPLS.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/rsvp_te/IntServ.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/mpls/LIBTable.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/mpls/IClassifier.h \
	networklayer/rsvp_te/Utils.h \
	linklayer/contract/MACAddress.h \
	transport/tcp_common/contract/TCPSocket.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/rsvp_te/IntServ_m.h \
	transport/tcp_common/contract/TCPSocketMap.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ldp/LDP.h \
	base/ByteArray_m.h \
	networklayer/ipv4/RoutingTableAccess.h \
	base/ByteArray.h \
	networklayer/mpls/MPLS.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/mpls/MPLSPacket.h \
	networklayer/mpls/ConstType.h \
	networklayer/mpls/LIBTableAccess.h \
	networklayer/ipv4/IPv4Datagram.h \
	transport/tcp_common/TCPSegment_m.h \
	transport/udp/contract/UDPSocket.h \
	transport/udp/contract/UDPControlInfo_m.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ldp/LDPPacket_m.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/OSPFTimer_m.o: networklayer/ospfv2/OSPFTimer_m.cc \
	base/INETDefs.h \
	networklayer/ospfv2/OSPFTimer_m.h
$O/networklayer/ospfv2/OSPFRouting.o: networklayer/ospfv2/OSPFRouting.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/OSPFRouting.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/OSPFPacket_m.o: networklayer/ospfv2/OSPFPacket_m.cc \
	base/INETDefs.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	networklayer/contract/IPv4Address.h
$O/networklayer/ospfv2/interface/OSPFInterfaceStateDesignatedRouter.o: networklayer/ospfv2/interface/OSPFInterfaceStateDesignatedRouter.cc \
	networklayer/ospfv2/interface/OSPFInterfaceStateDesignatedRouter.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	networklayer/ospfv2/interface/OSPFInterfaceState.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateLoopback.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateDown.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/interface/OSPFInterfaceStatePointToPoint.o: networklayer/ospfv2/interface/OSPFInterfaceStatePointToPoint.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	networklayer/ospfv2/interface/OSPFInterfaceState.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateLoopback.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateDown.h \
	networklayer/ospfv2/interface/OSPFInterfaceStatePointToPoint.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/interface/OSPFInterface.o: networklayer/ospfv2/interface/OSPFInterface.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	networklayer/ospfv2/interface/OSPFInterfaceState.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateDown.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/interface/OSPFInterfaceStateLoopback.o: networklayer/ospfv2/interface/OSPFInterfaceStateLoopback.cc \
	networklayer/ospfv2/interface/OSPFInterfaceStateLoopback.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	base/INETDefs.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/interface/OSPFInterfaceState.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateDown.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/interface/OSPFInterface.h
$O/networklayer/ospfv2/interface/OSPFInterfaceStateWaiting.o: networklayer/ospfv2/interface/OSPFInterfaceStateWaiting.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	networklayer/ospfv2/interface/OSPFInterfaceState.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateLoopback.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateWaiting.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateDown.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/interface/OSPFInterfaceStateBackup.o: networklayer/ospfv2/interface/OSPFInterfaceStateBackup.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateBackup.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	networklayer/ospfv2/interface/OSPFInterfaceState.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateLoopback.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateDown.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/interface/OSPFInterfaceState.o: networklayer/ospfv2/interface/OSPFInterfaceState.cc \
	networklayer/ospfv2/interface/OSPFInterfaceStateDesignatedRouter.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateBackup.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	networklayer/ospfv2/interface/OSPFInterfaceState.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateNotDesignatedRouter.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/interface/OSPFInterfaceStateNotDesignatedRouter.o: networklayer/ospfv2/interface/OSPFInterfaceStateNotDesignatedRouter.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	networklayer/ospfv2/interface/OSPFInterfaceState.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateLoopback.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateDown.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateNotDesignatedRouter.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/interface/OSPFInterfaceStateDown.o: networklayer/ospfv2/interface/OSPFInterfaceStateDown.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	networklayer/ospfv2/interface/OSPFInterfaceState.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateLoopback.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateWaiting.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/ospfv2/interface/OSPFInterfaceStatePointToPoint.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateDown.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/interface/OSPFInterfaceStateNotDesignatedRouter.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.o: networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.o: networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.o: networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.cc \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/messagehandler/HelloHandler.o: networklayer/ospfv2/messagehandler/HelloHandler.cc \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/messagehandler/LinkStateRequestHandler.o: networklayer/ospfv2/messagehandler/LinkStateRequestHandler.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/messagehandler/MessageHandler.o: networklayer/ospfv2/messagehandler/MessageHandler.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/neighbor/OSPFNeighborStateInit.o: networklayer/ospfv2/neighbor/OSPFNeighborStateInit.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateExchangeStart.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateInit.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateTwoWay.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/neighbor/OSPFNeighborState.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateDown.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/neighbor/OSPFNeighborStateLoading.o: networklayer/ospfv2/neighbor/OSPFNeighborStateLoading.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateExchangeStart.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateInit.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateTwoWay.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateLoading.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/neighbor/OSPFNeighborState.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateDown.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateFull.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/neighbor/OSPFNeighborStateExchange.o: networklayer/ospfv2/neighbor/OSPFNeighborStateExchange.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateExchangeStart.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateInit.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateLoading.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateTwoWay.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/neighbor/OSPFNeighborState.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateDown.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateExchange.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateFull.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/neighbor/OSPFNeighborStateExchangeStart.o: networklayer/ospfv2/neighbor/OSPFNeighborStateExchangeStart.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateExchangeStart.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateInit.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateTwoWay.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/neighbor/OSPFNeighborState.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateDown.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateExchange.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/neighbor/OSPFNeighborStateFull.o: networklayer/ospfv2/neighbor/OSPFNeighborStateFull.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateExchangeStart.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateInit.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateTwoWay.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/neighbor/OSPFNeighborState.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateDown.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateFull.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/neighbor/OSPFNeighborState.o: networklayer/ospfv2/neighbor/OSPFNeighborState.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/neighbor/OSPFNeighborState.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/neighbor/OSPFNeighbor.o: networklayer/ospfv2/neighbor/OSPFNeighbor.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/neighbor/OSPFNeighborState.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateDown.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/neighbor/OSPFNeighborStateAttempt.o: networklayer/ospfv2/neighbor/OSPFNeighborStateAttempt.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateAttempt.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateInit.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/neighbor/OSPFNeighborState.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateDown.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/neighbor/OSPFNeighborStateTwoWay.o: networklayer/ospfv2/neighbor/OSPFNeighborStateTwoWay.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateExchangeStart.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateInit.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateTwoWay.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/neighbor/OSPFNeighborState.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateDown.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/neighbor/OSPFNeighborStateDown.o: networklayer/ospfv2/neighbor/OSPFNeighborStateDown.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateAttempt.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateInit.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/neighbor/OSPFNeighborState.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/neighbor/OSPFNeighborStateDown.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/router/OSPFRouter.o: networklayer/ospfv2/router/OSPFRouter.cc \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/ospfv2/router/RouterLSA.o: networklayer/ospfv2/router/RouterLSA.cc \
	base/INETDefs.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ospfv2/router/OSPFcommon.h
$O/networklayer/ospfv2/router/SummaryLSA.o: networklayer/ospfv2/router/SummaryLSA.cc \
	base/INETDefs.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ospfv2/router/OSPFcommon.h
$O/networklayer/ospfv2/router/NetworkLSA.o: networklayer/ospfv2/router/NetworkLSA.cc \
	base/INETDefs.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ospfv2/router/OSPFcommon.h
$O/networklayer/ospfv2/router/ASExternalLSA.o: networklayer/ospfv2/router/ASExternalLSA.cc \
	base/INETDefs.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ospfv2/router/OSPFcommon.h
$O/networklayer/ospfv2/router/OSPFArea.o: networklayer/ospfv2/router/OSPFArea.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ospfv2/messagehandler/MessageHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateRequestHandler.h \
	base/INETDefs.h \
	networklayer/ospfv2/messagehandler/DatabaseDescriptionHandler.h \
	networklayer/ospfv2/messagehandler/IMessageHandler.h \
	linklayer/contract/MACAddress.h \
	networklayer/ospfv2/router/OSPFcommon.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ospfv2/OSPFPacket_m.h \
	base/ModuleAccess.h \
	networklayer/ospfv2/router/OSPFRouter.h \
	networklayer/ospfv2/messagehandler/LinkStateUpdateHandler.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ospfv2/interface/OSPFInterface.h \
	networklayer/ospfv2/OSPFTimer_m.h \
	networklayer/ospfv2/messagehandler/HelloHandler.h \
	networklayer/ospfv2/messagehandler/LinkStateAcknowledgementHandler.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	networklayer/ospfv2/neighbor/OSPFNeighbor.h \
	networklayer/ospfv2/router/OSPFArea.h \
	networklayer/ospfv2/router/LSA.h \
	networklayer/ospfv2/router/OSPFRoutingTableEntry.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/queue/REDQueue.o: networklayer/queue/REDQueue.cc \
	base/IPassiveQueue.h \
	base/INETDefs.h \
	networklayer/queue/REDQueue.h \
	base/PassiveQueueBase.h
$O/networklayer/queue/BasicDSCPClassifier.o: networklayer/queue/BasicDSCPClassifier.cc \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv6/IPv6Datagram.h \
	networklayer/queue/IQoSClassifier.h \
	base/INETDefs.h \
	networklayer/queue/BasicDSCPClassifier.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/contract/IPProtocolId_m.h
$O/networklayer/queue/DropTailQoSQueue.o: networklayer/queue/DropTailQoSQueue.cc \
	base/IPassiveQueue.h \
	networklayer/queue/DropTailQoSQueue.h \
	networklayer/queue/IQoSClassifier.h \
	base/INETDefs.h \
	base/PassiveQueueBase.h
$O/networklayer/queue/DropTailQueue.o: networklayer/queue/DropTailQueue.cc \
	base/IPassiveQueue.h \
	base/INETDefs.h \
	networklayer/queue/DropTailQueue.h \
	base/PassiveQueueBase.h
$O/networklayer/rsvp_te/SimpleClassifier.o: networklayer/rsvp_te/SimpleClassifier.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/rsvp_te/RSVPHelloMsg.h \
	networklayer/rsvp_te/IntServ.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/rsvp_te/RSVPPacket_m.h \
	networklayer/mpls/LIBTable.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/mpls/IClassifier.h \
	networklayer/rsvp_te/IRSVPClassifier.h \
	networklayer/rsvp_te/SimpleClassifier.h \
	networklayer/rsvp_te/RSVPPacket.h \
	networklayer/rsvp_te/RSVPResvMsg.h \
	networklayer/rsvp_te/RSVPPathMsg_m.h \
	base/IScriptable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/rsvp_te/IntServ_m.h \
	networklayer/rsvp_te/SignallingMsg_m.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/rsvp_te/RSVPPathMsg.h \
	networklayer/rsvp_te/RSVPAccess.h \
	networklayer/rsvp_te/RSVP.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	util/XMLUtils.h \
	base/NotifierConsts.h \
	networklayer/mpls/ConstType.h \
	networklayer/mpls/LIBTableAccess.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/rsvp_te/RSVPHello_m.h \
	networklayer/rsvp_te/RSVPResvMsg_m.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/rsvp_te/SignallingMsg_m.o: networklayer/rsvp_te/SignallingMsg_m.cc \
	networklayer/rsvp_te/IntServ.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/rsvp_te/IntServ_m.h \
	networklayer/rsvp_te/SignallingMsg_m.h
$O/networklayer/rsvp_te/RSVPPacket_m.o: networklayer/rsvp_te/RSVPPacket_m.cc \
	networklayer/rsvp_te/RSVPPacket_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/rsvp_te/IntServ_m.h
$O/networklayer/rsvp_te/Utils.o: networklayer/rsvp_te/Utils.cc \
	networklayer/rsvp_te/IntServ.h \
	base/INETDefs.h \
	networklayer/rsvp_te/Utils.h \
	networklayer/contract/IPv4Address.h \
	networklayer/rsvp_te/IntServ_m.h
$O/networklayer/rsvp_te/RSVPResvMsg_m.o: networklayer/rsvp_te/RSVPResvMsg_m.cc \
	networklayer/rsvp_te/RSVPPacket.h \
	networklayer/rsvp_te/IntServ.h \
	networklayer/rsvp_te/RSVPPacket_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/rsvp_te/RSVPResvMsg_m.h \
	networklayer/rsvp_te/IntServ_m.h
$O/networklayer/rsvp_te/RSVP.o: networklayer/rsvp_te/RSVP.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/rsvp_te/RSVPHelloMsg.h \
	networklayer/rsvp_te/IntServ.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/rsvp_te/RSVPPacket_m.h \
	networklayer/mpls/LIBTable.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/mpls/IClassifier.h \
	networklayer/rsvp_te/Utils.h \
	networklayer/rsvp_te/IRSVPClassifier.h \
	networklayer/ted/TEDAccess.h \
	linklayer/contract/MACAddress.h \
	networklayer/rsvp_te/RSVPPacket.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ted/TED.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/rsvp_te/RSVPResvMsg.h \
	networklayer/rsvp_te/RSVPPathMsg_m.h \
	base/IScriptable.h \
	networklayer/rsvp_te/IntServ_m.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/rsvp_te/SignallingMsg_m.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/rsvp_te/RSVPPathMsg.h \
	networklayer/common/InterfaceEntry.h \
	util/common.h \
	networklayer/rsvp_te/RSVP.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	util/XMLUtils.h \
	networklayer/mpls/ConstType.h \
	networklayer/mpls/LIBTableAccess.h \
	networklayer/ted/TED_m.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/rsvp_te/RSVPHello_m.h \
	networklayer/contract/IPv6Address.h \
	networklayer/rsvp_te/RSVPResvMsg_m.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/rsvp_te/IntServ_m.o: networklayer/rsvp_te/IntServ_m.cc \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/rsvp_te/IntServ_m.h
$O/networklayer/rsvp_te/RSVPPathMsg_m.o: networklayer/rsvp_te/RSVPPathMsg_m.cc \
	networklayer/rsvp_te/RSVPPacket.h \
	networklayer/rsvp_te/IntServ.h \
	networklayer/rsvp_te/RSVPPacket_m.h \
	base/INETDefs.h \
	networklayer/rsvp_te/RSVPPathMsg_m.h \
	networklayer/contract/IPv4Address.h \
	networklayer/rsvp_te/IntServ_m.h
$O/networklayer/rsvp_te/RSVPHello_m.o: networklayer/rsvp_te/RSVPHello_m.cc \
	networklayer/rsvp_te/RSVPPacket.h \
	networklayer/rsvp_te/RSVPPacket_m.h \
	base/INETDefs.h \
	networklayer/rsvp_te/RSVPHello_m.h \
	networklayer/contract/IPv4Address.h \
	networklayer/rsvp_te/IntServ_m.h
$O/networklayer/ted/TED_m.o: networklayer/ted/TED_m.cc \
	networklayer/ted/TED_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h
$O/networklayer/ted/TED.o: networklayer/ted/TED.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/rsvp_te/IntServ.h \
	networklayer/ipv4/RoutingTableAccess.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ted/TED.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/ted/TED_m.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/rsvp_te/IntServ_m.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv4/IRoutingTable.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/ted/LinkStatePacket_m.o: networklayer/ted/LinkStatePacket_m.cc \
	networklayer/ted/TED_m.h \
	base/INETDefs.h \
	networklayer/ted/LinkStatePacket_m.h \
	networklayer/contract/IPv4Address.h
$O/networklayer/ted/LinkStateRouting.o: networklayer/ted/LinkStateRouting.cc \
	networklayer/ipv4/IPv4InterfaceData.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/rsvp_te/IntServ.h \
	base/INETDefs.h \
	networklayer/ted/LinkStateRouting.h \
	base/INotifiable.h \
	networklayer/ted/TEDAccess.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ted/TED.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/rsvp_te/IntServ_m.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/ted/LinkStatePacket_m.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	networklayer/ted/TED_m.h \
	networklayer/ipv4/IRoutingTable.h
$O/networklayer/xmipv6/MobilityHeader_m.o: networklayer/xmipv6/MobilityHeader_m.cc \
	networklayer/ipv6/IPv6Datagram.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/IPv6ExtensionHeaders.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv6/IPv6ExtensionHeaders_m.h \
	networklayer/xmipv6/MobilityHeader_m.h
$O/networklayer/xmipv6/BindingUpdateList.o: networklayer/xmipv6/BindingUpdateList.cc \
	networklayer/xmipv6/BindingUpdateList.h \
	networklayer/common/InterfaceToken.h \
	base/INETDefs.h \
	networklayer/ipv6/IPv6InterfaceData.h \
	networklayer/contract/IPv6Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceEntry.h
$O/networklayer/xmipv6/BindingCache.o: networklayer/xmipv6/BindingCache.cc \
	networklayer/xmipv6/BindingCache.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h
$O/networklayer/xmipv6/xMIPv6.o: networklayer/xmipv6/xMIPv6.cc \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/ipv6/IPv6InterfaceData.h \
	linklayer/contract/MACAddress.h \
	networklayer/xmipv6/BindingCacheAccess.h \
	networklayer/icmpv6/IPv6NeighbourCache.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/ipv6/IPv6ExtensionHeaders.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/xmipv6/MobilityHeader_m.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv6tunneling/IPv6Tunneling.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/xmipv6/BindingUpdateList.h \
	networklayer/ipv6tunneling/IPv6TunnelingAccess.h \
	networklayer/icmpv6/IPv6NeighbourDiscoveryAccess.h \
	networklayer/xmipv6/xMIPv6.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/icmpv6/IPv6NDMessage_m.h \
	networklayer/icmpv6/IPv6NeighbourDiscovery.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	base/NotifierConsts.h \
	networklayer/xmipv6/BindingCache.h \
	networklayer/ipv6/RoutingTable6.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/ipv6/IPv6Datagram.h \
	networklayer/xmipv6/BindingUpdateListAccess.h \
	networklayer/icmpv6/ICMPv6Message_m.h \
	networklayer/xmipv6/MobilityHeader.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/ipv6/RoutingTable6Access.h \
	networklayer/ipv6/IPv6ExtensionHeaders_m.h
$O/networklayer/xmipv6/MobilityHeader.o: networklayer/xmipv6/MobilityHeader.cc \
	networklayer/ipv6/IPv6Datagram.h \
	base/INETDefs.h \
	networklayer/xmipv6/MobilityHeader.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/ipv6/IPv6ExtensionHeaders.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/xmipv6/MobilityHeader_m.h \
	networklayer/ipv6/IPv6ExtensionHeaders_m.h
$O/transport/rtp/RTPInnerPacket.o: transport/rtp/RTPInnerPacket.cc \
	transport/rtp/RTPPacket_m.h \
	transport/rtp/RTPSenderStatusMessage_m.h \
	transport/rtp/RTPPacket.h \
	transport/rtp/RTPInnerPacket_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	transport/rtp/RTPInnerPacket.h \
	transport/rtp/RTPSenderControlMessage_m.h
$O/transport/rtp/RTPPayloadReceiver.o: transport/rtp/RTPPayloadReceiver.cc \
	transport/rtp/RTPPacket_m.h \
	transport/rtp/RTPSenderStatusMessage_m.h \
	transport/rtp/RTPPacket.h \
	transport/rtp/RTPInnerPacket_m.h \
	base/INETDefs.h \
	transport/rtp/RTPPayloadReceiver.h \
	networklayer/contract/IPv4Address.h \
	transport/rtp/RTPInnerPacket.h \
	transport/rtp/RTPSenderControlMessage_m.h
$O/transport/rtp/RTPParticipantInfo_m.o: transport/rtp/RTPParticipantInfo_m.cc \
	base/INETDefs.h \
	transport/rtp/RTPParticipantInfo_m.h \
	networklayer/contract/IPv4Address.h
$O/transport/rtp/RTPPacket_m.o: transport/rtp/RTPPacket_m.cc \
	transport/rtp/RTPPacket_m.h \
	base/INETDefs.h
$O/transport/rtp/RTPParticipantInfo.o: transport/rtp/RTPParticipantInfo.cc \
	transport/rtp/reports_m.h \
	transport/rtp/RTPPacket_m.h \
	transport/rtp/RTPPacket.h \
	base/INETDefs.h \
	transport/rtp/reports.h \
	transport/rtp/RTPParticipantInfo_m.h \
	networklayer/contract/IPv4Address.h \
	transport/rtp/sdes.h \
	transport/rtp/RTPParticipantInfo.h
$O/transport/rtp/RTCP.o: transport/rtp/RTCP.cc \
	transport/rtp/RTPSenderInfo.h \
	base/INETDefs.h \
	transport/rtp/RTPParticipantInfo_m.h \
	transport/rtp/RTCPPacket1_m.h \
	transport/rtp/RTCPPacket3.h \
	transport/rtp/RTCPPacket2.h \
	networklayer/contract/IPvXAddress.h \
	transport/rtp/RTPInnerPacket_m.h \
	transport/rtp/reports.h \
	transport/rtp/RTCP.h \
	transport/rtp/RTCPPacket1.h \
	transport/rtp/RTPReceiverInfo.h \
	networklayer/contract/IPv4Address.h \
	transport/rtp/RTCPPacket2_m.h \
	transport/rtp/RTPInnerPacket.h \
	transport/rtp/RTPSenderControlMessage_m.h \
	transport/rtp/reports_m.h \
	transport/rtp/RTCPPacket3_m.h \
	transport/rtp/RTPPacket_m.h \
	transport/rtp/RTPSenderStatusMessage_m.h \
	transport/rtp/RTPPacket.h \
	transport/udp/contract/UDPSocket.h \
	networklayer/contract/IPv6Address.h \
	transport/udp/contract/UDPControlInfo_m.h \
	transport/rtp/RTCPPacket.h \
	transport/rtp/sdes.h \
	transport/rtp/RTPParticipantInfo.h
$O/transport/rtp/RTCPPacket1_m.o: transport/rtp/RTCPPacket1_m.cc \
	transport/rtp/RTCPPacket1_m.h
$O/transport/rtp/RTPInterfacePacket_m.o: transport/rtp/RTPInterfacePacket_m.cc \
	transport/rtp/RTPSenderStatusMessage_m.h \
	base/INETDefs.h \
	transport/rtp/RTPInterfacePacket_m.h \
	networklayer/contract/IPv4Address.h \
	transport/rtp/RTPSenderControlMessage_m.h
$O/transport/rtp/RTPSenderStatusMessage_m.o: transport/rtp/RTPSenderStatusMessage_m.cc \
	transport/rtp/RTPSenderStatusMessage_m.h \
	base/INETDefs.h
$O/transport/rtp/RTPPayloadSender.o: transport/rtp/RTPPayloadSender.cc \
	transport/rtp/RTPPacket_m.h \
	transport/rtp/RTPPayloadSender.h \
	transport/rtp/RTPPacket.h \
	transport/rtp/RTPSenderStatusMessage_m.h \
	transport/rtp/RTPInnerPacket_m.h \
	base/INETDefs.h \
	transport/rtp/RTPInterfacePacket_m.h \
	networklayer/contract/IPv4Address.h \
	transport/rtp/RTPInnerPacket.h \
	transport/rtp/RTPSenderControlMessage_m.h
$O/transport/rtp/RTCPPacket2_m.o: transport/rtp/RTCPPacket2_m.cc \
	transport/rtp/reports_m.h \
	transport/rtp/RTCPPacket1.h \
	transport/rtp/reports.h \
	base/INETDefs.h \
	transport/rtp/RTCPPacket1_m.h \
	transport/rtp/RTCPPacket2_m.h \
	transport/rtp/sdes.h
$O/transport/rtp/RTPSenderControlMessage_m.o: transport/rtp/RTPSenderControlMessage_m.cc \
	base/INETDefs.h \
	transport/rtp/RTPSenderControlMessage_m.h
$O/transport/rtp/RTPPacket.o: transport/rtp/RTPPacket.cc \
	transport/rtp/RTPPacket_m.h \
	transport/rtp/RTPPacket.h \
	base/INETDefs.h
$O/transport/rtp/RTPInnerPacket_m.o: transport/rtp/RTPInnerPacket_m.cc \
	transport/rtp/RTPPacket_m.h \
	transport/rtp/RTPPacket.h \
	transport/rtp/RTPSenderStatusMessage_m.h \
	base/INETDefs.h \
	transport/rtp/RTPInnerPacket_m.h \
	networklayer/contract/IPv4Address.h \
	transport/rtp/RTPSenderControlMessage_m.h
$O/transport/rtp/RTCPPacket.o: transport/rtp/RTCPPacket.cc \
	transport/rtp/RTCPPacket3_m.h \
	transport/rtp/reports_m.h \
	transport/rtp/RTCPPacket1.h \
	base/INETDefs.h \
	transport/rtp/reports.h \
	transport/rtp/RTCPPacket1_m.h \
	transport/rtp/RTCPPacket3.h \
	transport/rtp/RTCPPacket2_m.h \
	transport/rtp/RTCPPacket.h \
	transport/rtp/sdes.h \
	transport/rtp/RTCPPacket2.h
$O/transport/rtp/reports.o: transport/rtp/reports.cc \
	transport/rtp/reports_m.h \
	transport/rtp/reports.h
$O/transport/rtp/RTPReceiverInfo.o: transport/rtp/RTPReceiverInfo.cc \
	transport/rtp/reports_m.h \
	transport/rtp/RTPPacket_m.h \
	transport/rtp/RTPReceiverInfo.h \
	transport/rtp/RTPPacket.h \
	base/INETDefs.h \
	transport/rtp/reports.h \
	transport/rtp/RTPParticipantInfo_m.h \
	networklayer/contract/IPv4Address.h \
	transport/rtp/sdes.h \
	transport/rtp/RTPParticipantInfo.h
$O/transport/rtp/RTPProfile.o: transport/rtp/RTPProfile.cc \
	transport/rtp/RTPProfile.h \
	transport/rtp/RTPPacket_m.h \
	transport/rtp/RTPPayloadSender.h \
	transport/rtp/RTPPacket.h \
	transport/rtp/RTPSenderStatusMessage_m.h \
	transport/rtp/RTPInnerPacket_m.h \
	base/INETDefs.h \
	transport/rtp/RTPPayloadReceiver.h \
	networklayer/contract/IPv4Address.h \
	transport/rtp/RTPInnerPacket.h \
	transport/rtp/RTPSenderControlMessage_m.h
$O/transport/rtp/reports_m.o: transport/rtp/reports_m.cc \
	transport/rtp/reports_m.h
$O/transport/rtp/RTP.o: transport/rtp/RTP.cc \
	transport/rtp/RTPProfile.h \
	networklayer/ipv4/IPv4RouteRule.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	transport/rtp/RTPInnerPacket_m.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ipv4/RoutingTableAccess.h \
	transport/rtp/RTPInterfacePacket_m.h \
	networklayer/contract/IPv4Address.h \
	transport/rtp/RTP.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	transport/rtp/RTPInnerPacket.h \
	transport/rtp/RTPSenderControlMessage_m.h \
	transport/rtp/RTPPacket_m.h \
	transport/rtp/RTPSenderStatusMessage_m.h \
	transport/rtp/RTPPacket.h \
	transport/udp/contract/UDPSocket.h \
	transport/udp/contract/UDPControlInfo_m.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/IRoutingTable.h
$O/transport/rtp/RTCPPacket3_m.o: transport/rtp/RTCPPacket3_m.cc \
	transport/rtp/reports_m.h \
	transport/rtp/RTCPPacket3_m.h \
	transport/rtp/RTCPPacket1.h \
	transport/rtp/reports.h \
	base/INETDefs.h \
	transport/rtp/RTCPPacket1_m.h \
	transport/rtp/RTCPPacket2_m.h \
	transport/rtp/sdes.h \
	transport/rtp/RTCPPacket2.h
$O/transport/rtp/RTPSenderInfo.o: transport/rtp/RTPSenderInfo.cc \
	transport/rtp/reports_m.h \
	transport/rtp/RTPPacket_m.h \
	transport/rtp/RTPSenderInfo.h \
	transport/rtp/RTPPacket.h \
	base/INETDefs.h \
	transport/rtp/reports.h \
	transport/rtp/RTPParticipantInfo_m.h \
	networklayer/contract/IPv4Address.h \
	transport/rtp/sdes.h \
	transport/rtp/RTPParticipantInfo.h
$O/transport/rtp/sdes.o: transport/rtp/sdes.cc \
	base/INETDefs.h \
	transport/rtp/sdes.h
$O/transport/rtp/profiles/avprofile/RTPAVProfilePayload32Sender.o: transport/rtp/profiles/avprofile/RTPAVProfilePayload32Sender.cc \
	transport/rtp/profiles/avprofile/RTPAVProfilePayload32Sender.h \
	transport/rtp/RTPPacket_m.h \
	transport/rtp/RTPPayloadSender.h \
	transport/rtp/RTPSenderStatusMessage_m.h \
	transport/rtp/RTPPacket.h \
	transport/rtp/RTPInnerPacket_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	transport/rtp/profiles/avprofile/RTPMpegPacket_m.h \
	transport/rtp/RTPInnerPacket.h \
	transport/rtp/RTPSenderControlMessage_m.h
$O/transport/rtp/profiles/avprofile/RTPMpegPacket_m.o: transport/rtp/profiles/avprofile/RTPMpegPacket_m.cc \
	transport/rtp/profiles/avprofile/RTPMpegPacket_m.h
$O/transport/rtp/profiles/avprofile/RTPAVProfilePayload32Receiver.o: transport/rtp/profiles/avprofile/RTPAVProfilePayload32Receiver.cc \
	transport/rtp/RTPPacket_m.h \
	transport/rtp/RTPPacket.h \
	base/INETDefs.h \
	transport/rtp/RTPPayloadReceiver.h \
	transport/rtp/profiles/avprofile/RTPMpegPacket_m.h \
	transport/rtp/profiles/avprofile/RTPAVProfilePayload32Receiver.h
$O/transport/rtp/profiles/avprofile/RTPAVProfile.o: transport/rtp/profiles/avprofile/RTPAVProfile.cc \
	transport/rtp/RTPProfile.h \
	base/INETDefs.h \
	transport/rtp/profiles/avprofile/RTPAVProfile.h
$O/transport/sctp/SCTPSendStream.o: transport/sctp/SCTPSendStream.cc \
	transport/sctp/SCTPReceiveStream.h \
	transport/sctp/SCTPMessage.h \
	networklayer/common/InterfaceTable.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPSendStream.h \
	transport/sctp/SCTPQueue.h \
	networklayer/contract/IPv6Address.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h
$O/transport/sctp/SCTP.o: transport/sctp/SCTP.cc \
	transport/sctp/SCTPMessage.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	transport/sctp/SCTP.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/contract/SCTPCommand.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPQueue.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	transport/sctp/SCTPReceiveStream.h \
	networklayer/common/InterfaceTable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	transport/sctp/contract/SCTPCommand_m.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	transport/udp/contract/UDPSocket.h \
	transport/sctp/SCTPSendStream.h \
	transport/udp/contract/UDPControlInfo_m.h \
	networklayer/contract/IPv6Address.h
$O/transport/sctp/SCTPSSFunctions.o: transport/sctp/SCTPSSFunctions.cc \
	transport/sctp/SCTPReceiveStream.h \
	transport/sctp/SCTPMessage.h \
	networklayer/common/InterfaceTable.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPSendStream.h \
	transport/sctp/SCTPQueue.h \
	networklayer/contract/IPv6Address.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/common/InterfaceEntry.h
$O/transport/sctp/SCTPMessage.o: transport/sctp/SCTPMessage.cc \
	transport/sctp/SCTPMessage.h \
	transport/sctp/SCTPReceiveStream.h \
	networklayer/common/InterfaceTable.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPSendStream.h \
	transport/sctp/SCTPQueue.h \
	networklayer/contract/IPv6Address.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/common/InterfaceEntry.h
$O/transport/sctp/SCTPAssociationEventProc.o: transport/sctp/SCTPAssociationEventProc.cc \
	transport/sctp/SCTPMessage.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	transport/sctp/contract/SCTPCommand.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPQueue.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	transport/sctp/SCTPReceiveStream.h \
	networklayer/common/InterfaceTable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	transport/sctp/contract/SCTPCommand_m.h \
	transport/sctp/SCTPAlgorithm.h \
	transport/sctp/SCTPSendStream.h \
	networklayer/contract/IPv6Address.h
$O/transport/sctp/SCTPAlg.o: transport/sctp/SCTPAlg.cc \
	transport/sctp/SCTPReceiveStream.h \
	transport/sctp/SCTPMessage.h \
	networklayer/common/InterfaceTable.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/common/InterfaceTableAccess.h \
	transport/sctp/SCTPAlg.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	transport/sctp/SCTPAlgorithm.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPSendStream.h \
	transport/sctp/SCTPQueue.h \
	networklayer/contract/IPv6Address.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h
$O/transport/sctp/SCTPQueue.o: transport/sctp/SCTPQueue.cc \
	transport/sctp/SCTPReceiveStream.h \
	transport/sctp/SCTPMessage.h \
	networklayer/common/InterfaceTable.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPSendStream.h \
	transport/sctp/SCTPQueue.h \
	networklayer/contract/IPv6Address.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/common/InterfaceEntry.h
$O/transport/sctp/SCTPCCFunctions.o: transport/sctp/SCTPCCFunctions.cc \
	transport/sctp/SCTPReceiveStream.h \
	transport/sctp/SCTPMessage.h \
	networklayer/common/InterfaceTable.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPSendStream.h \
	transport/sctp/SCTPQueue.h \
	networklayer/contract/IPv6Address.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/common/InterfaceEntry.h
$O/transport/sctp/SCTPAssociationBase.o: transport/sctp/SCTPAssociationBase.cc \
	transport/sctp/SCTPMessage.h \
	networklayer/ipv4/IPv4RouteRule.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/contract/SCTPCommand.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPQueue.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	transport/sctp/SCTPReceiveStream.h \
	networklayer/common/InterfaceTable.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	transport/sctp/contract/SCTPCommand_m.h \
	transport/sctp/SCTPAlgorithm.h \
	transport/sctp/SCTPSendStream.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/IRoutingTable.h
$O/transport/sctp/SCTPMessage_m.o: transport/sctp/SCTPMessage_m.cc \
	transport/sctp/SCTPMessage_m.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h
$O/transport/sctp/SCTPAssociationRcvMessage.o: transport/sctp/SCTPAssociationRcvMessage.cc \
	transport/sctp/SCTPMessage.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/ipv6/IPv6InterfaceData.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	transport/sctp/contract/SCTPCommand.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPQueue.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	util/common.h \
	transport/sctp/SCTPReceiveStream.h \
	networklayer/common/InterfaceTable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	transport/sctp/contract/SCTPCommand_m.h \
	transport/sctp/SCTPAlgorithm.h \
	transport/sctp/SCTPSendStream.h \
	networklayer/contract/IPv6Address.h
$O/transport/sctp/SCTPReceiveStream.o: transport/sctp/SCTPReceiveStream.cc \
	transport/sctp/SCTPReceiveStream.h \
	transport/sctp/SCTPMessage.h \
	networklayer/common/InterfaceTable.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPSendStream.h \
	transport/sctp/SCTPQueue.h \
	networklayer/contract/IPv6Address.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/common/InterfaceEntry.h
$O/transport/sctp/SCTPAssociationSendAll.o: transport/sctp/SCTPAssociationSendAll.cc \
	transport/sctp/SCTPMessage.h \
	transport/sctp/SCTPReceiveStream.h \
	networklayer/common/InterfaceTable.h \
	transport/sctp/SCTP.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	transport/sctp/contract/SCTPCommand.h \
	transport/sctp/contract/SCTPCommand_m.h \
	networklayer/common/InterfaceToken.h \
	transport/sctp/SCTPAlgorithm.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPQueue.h \
	transport/sctp/SCTPSendStream.h \
	networklayer/contract/IPv6Address.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h
$O/transport/sctp/SCTPAssociationUtil.o: transport/sctp/SCTPAssociationUtil.cc \
	transport/sctp/SCTPMessage.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/ipv4/IPv4InterfaceData.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/ipv6/IPv6InterfaceData.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	transport/sctp/contract/SCTPCommand.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPQueue.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	transport/sctp/SCTPReceiveStream.h \
	util/common.h \
	networklayer/common/InterfaceTable.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/ipv4/RoutingTable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	transport/sctp/contract/SCTPCommand_m.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	transport/sctp/SCTPAlgorithm.h \
	transport/sctp/SCTPSendStream.h \
	networklayer/contract/IPv6Address.h \
	transport/udp/contract/UDPControlInfo_m.h \
	networklayer/ipv4/IRoutingTable.h
$O/transport/sctp/contract/SCTPCommand_m.o: transport/sctp/contract/SCTPCommand_m.cc \
	transport/sctp/SCTPMessage.h \
	transport/sctp/SCTPReceiveStream.h \
	networklayer/common/InterfaceTable.h \
	transport/sctp/SCTP.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	transport/sctp/contract/SCTPCommand.h \
	transport/sctp/contract/SCTPCommand_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPSendStream.h \
	transport/sctp/SCTPQueue.h \
	networklayer/contract/IPv6Address.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h
$O/transport/sctp/contract/SCTPSocket.o: transport/sctp/contract/SCTPSocket.cc \
	transport/sctp/SCTPMessage.h \
	transport/sctp/SCTPReceiveStream.h \
	networklayer/common/InterfaceTable.h \
	transport/sctp/SCTP.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	transport/sctp/contract/SCTPCommand.h \
	transport/sctp/contract/SCTPCommand_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPQueue.h \
	transport/sctp/SCTPSendStream.h \
	networklayer/contract/IPv6Address.h \
	transport/sctp/contract/SCTPSocket.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h
$O/transport/tcp/TCPConnectionBase.o: transport/tcp/TCPConnectionBase.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	transport/tcp/TCPSendQueue.h \
	transport/tcp/TCPSACKRexmitQueue.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	transport/tcp/TCPAlgorithm.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h \
	transport/tcp/TCPReceiveQueue.h
$O/transport/tcp/TCPConnectionSackUtil.o: transport/tcp/TCPConnectionSackUtil.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	transport/tcp/TCPSendQueue.h \
	transport/tcp/TCPSACKRexmitQueue.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	transport/tcp/TCPAlgorithm.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h \
	transport/tcp/TCPReceiveQueue.h
$O/transport/tcp/TCP.o: transport/tcp/TCP.cc \
	transport/tcp/TCPConnection.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	transport/tcp/queues/TCPVirtualDataRcvQueue.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	transport/tcp/TCP.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/contract/IPvXAddress.h \
	base/ByteArrayBuffer.h \
	transport/tcp_common/TCPSegment.h \
	transport/tcp/TCPReceiveQueue.h \
	networklayer/contract/IPProtocolId_m.h \
	base/ByteArray_m.h \
	base/ByteArray.h \
	transport/tcp/TCPSendQueue.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/queues/TCPMsgBasedRcvQueue.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/icmpv6/ICMPv6Message_m.h \
	transport/tcp/queues/TCPVirtualDataSendQueue.h \
	transport/tcp/queues/TCPDataStreamSendQueue.h \
	networklayer/contract/IPv6Address.h \
	transport/tcp/queues/TCPDataStreamRcvQueue.h \
	networklayer/ipv4/ICMPMessage_m.h \
	transport/tcp/queues/TCPMsgBasedSendQueue.h
$O/transport/tcp/TCPSACKRexmitQueue.o: transport/tcp/TCPSACKRexmitQueue.cc \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp/TCPConnection.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/contract/IPvXAddress.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	base/INETDefs.h \
	transport/tcp/TCPSACKRexmitQueue.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h
$O/transport/tcp/TCPConnectionRcvSegment.o: transport/tcp/TCPConnectionRcvSegment.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray_m.h \
	base/ByteArray.h \
	transport/tcp/TCPSendQueue.h \
	transport/tcp/TCPSACKRexmitQueue.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	transport/tcp/TCPAlgorithm.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h \
	transport/tcp/TCPReceiveQueue.h
$O/transport/tcp/TCPConnectionUtil.o: transport/tcp/TCPConnectionUtil.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	base/INETDefs.h \
	transport/tcp/TCPSendQueue.h \
	transport/tcp/TCPSACKRexmitQueue.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	transport/tcp_common/TCPSegment_m.h \
	transport/tcp/TCPAlgorithm.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPProtocolId_m.h \
	transport/tcp/TCPReceiveQueue.h
$O/transport/tcp/TCPConnectionEventProc.o: transport/tcp/TCPConnectionEventProc.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	transport/tcp/TCPSendQueue.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	transport/tcp/TCPAlgorithm.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h \
	transport/tcp/TCPReceiveQueue.h
$O/transport/tcp/flavours/TCPNoCongestionControl.o: transport/tcp/flavours/TCPNoCongestionControl.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	base/INETDefs.h \
	transport/tcp/flavours/TCPBaseAlg.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/TCP.h \
	transport/tcp/flavours/TCPNoCongestionControl.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp/TCPAlgorithm.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h
$O/transport/tcp/flavours/TCPTahoeRenoFamily.o: transport/tcp/flavours/TCPTahoeRenoFamily.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	base/INETDefs.h \
	transport/tcp/flavours/TCPBaseAlg.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/flavours/TCPTahoeRenoFamily.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	transport/tcp/TCPAlgorithm.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h
$O/transport/tcp/flavours/TCPBaseAlg.o: transport/tcp/flavours/TCPBaseAlg.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray_m.h \
	base/ByteArray.h \
	transport/tcp/TCPSACKRexmitQueue.h \
	base/INETDefs.h \
	transport/tcp/flavours/TCPBaseAlg.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	transport/tcp/TCPAlgorithm.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h
$O/transport/tcp/flavours/TCPNewReno.o: transport/tcp/flavours/TCPNewReno.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	base/INETDefs.h \
	transport/tcp/flavours/TCPBaseAlg.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/flavours/TCPNewReno.h \
	transport/tcp/flavours/TCPTahoeRenoFamily.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	transport/tcp/TCPAlgorithm.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h
$O/transport/tcp/flavours/DumbTCP.o: transport/tcp/flavours/DumbTCP.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray_m.h \
	base/ByteArray.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/flavours/DumbTCP.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp/TCPAlgorithm.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h
$O/transport/tcp/flavours/TCPReno.o: transport/tcp/flavours/TCPReno.cc \
	transport/tcp/flavours/TCPReno.h \
	transport/tcp/TCPConnection.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	base/INETDefs.h \
	transport/tcp/flavours/TCPBaseAlg.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/flavours/TCPTahoeRenoFamily.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp/TCPAlgorithm.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h
$O/transport/tcp/flavours/TCPTahoe.o: transport/tcp/flavours/TCPTahoe.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	base/INETDefs.h \
	transport/tcp/flavours/TCPBaseAlg.h \
	transport/tcp/flavours/TCPTahoe.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/flavours/TCPTahoeRenoFamily.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	transport/tcp/TCPAlgorithm.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h
$O/transport/tcp/queues/TCPMsgBasedRcvQueue.o: transport/tcp/queues/TCPMsgBasedRcvQueue.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray_m.h \
	base/ByteArray.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/queues/TCPVirtualDataRcvQueue.h \
	transport/tcp/queues/TCPMsgBasedRcvQueue.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h \
	transport/tcp/TCPReceiveQueue.h
$O/transport/tcp/queues/TCPDataStreamRcvQueue.o: transport/tcp/queues/TCPDataStreamRcvQueue.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/queues/TCPVirtualDataRcvQueue.h \
	transport/tcp/TCP.h \
	base/ByteArrayMessage_m.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/contract/IPvXAddress.h \
	base/ByteArrayMessage.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h \
	transport/tcp/queues/TCPDataStreamRcvQueue.h \
	transport/tcp/TCPReceiveQueue.h
$O/transport/tcp/queues/TCPDataStreamSendQueue.o: transport/tcp/queues/TCPDataStreamSendQueue.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	base/INETDefs.h \
	transport/tcp/TCPSendQueue.h \
	networklayer/contract/IPv4Address.h \
	base/ByteArrayMessage_m.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/contract/IPvXAddress.h \
	base/ByteArrayBuffer.h \
	transport/tcp/queues/TCPDataStreamSendQueue.h \
	base/ByteArrayMessage.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h
$O/transport/tcp/queues/TCPVirtualDataRcvQueue.o: transport/tcp/queues/TCPVirtualDataRcvQueue.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/queues/TCPVirtualDataRcvQueue.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h \
	transport/tcp/TCPReceiveQueue.h
$O/transport/tcp/queues/TCPMsgBasedSendQueue.o: transport/tcp/queues/TCPMsgBasedSendQueue.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	base/INETDefs.h \
	transport/tcp/TCPSendQueue.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h \
	transport/tcp/queues/TCPMsgBasedSendQueue.h
$O/transport/tcp/queues/TCPVirtualDataSendQueue.o: transport/tcp/queues/TCPVirtualDataSendQueue.cc \
	transport/tcp/TCPConnection.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	transport/tcp/TCPSendQueue.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	transport/tcp/TCP.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp/queues/TCPVirtualDataSendQueue.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h
$O/transport/tcp_common/TCPSegment.o: transport/tcp_common/TCPSegment.cc \
	transport/tcp_common/TCPSegment_m.h \
	base/ByteArray_m.h \
	base/ByteArray.h \
	base/INETDefs.h \
	transport/tcp_common/TCPSegment.h
$O/transport/tcp_common/TCPSegment_m.o: transport/tcp_common/TCPSegment_m.cc \
	transport/tcp_common/TCPSegment_m.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	base/INETDefs.h
$O/transport/tcp_common/TCPSpoof.o: transport/tcp_common/TCPSpoof.cc \
	base/ByteArray.h \
	base/ByteArray_m.h \
	transport/tcp_common/TCPSpoof.h \
	base/INETDefs.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/contract/IPvXAddressResolver.h \
	transport/tcp_common/TCPSegment.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPProtocolId_m.h
$O/transport/tcp_common/contract/TCPSocketMap.o: transport/tcp_common/contract/TCPSocketMap.cc \
	transport/tcp_common/contract/TCPSocket.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	transport/tcp_common/contract/TCPSocketMap.h
$O/transport/tcp_common/contract/TCPCommand_m.o: transport/tcp_common/contract/TCPCommand_m.cc \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h
$O/transport/tcp_common/contract/TCPSocket.o: transport/tcp_common/contract/TCPSocket.cc \
	transport/tcp_common/contract/TCPSocket.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h
$O/transport/udp/UDPPacket_m.o: transport/udp/UDPPacket_m.cc \
	transport/udp/UDPPacket_m.h \
	base/INETDefs.h
$O/transport/udp/UDP.o: transport/udp/UDP.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/icmpv6/ICMPv6Access.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/icmpv6/ICMPv6.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv6/contract/IPv6ControlInfo.h \
	networklayer/ipv4/IPv4Route.h \
	transport/udp/UDPPacket_m.h \
	networklayer/ipv6/contract/IPv6ControlInfo_m.h \
	networklayer/ipv4/ICMP.h \
	networklayer/ipv6/IPv6Datagram.h \
	networklayer/icmpv6/ICMPv6Message_m.h \
	transport/udp/UDPPacket.h \
	transport/udp/UDP.h \
	transport/udp/contract/UDPControlInfo_m.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv6/IPv6Datagram_m.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/IRoutingTable.h
$O/transport/udp/contract/UDPSocket.o: transport/udp/contract/UDPSocket.cc \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	transport/udp/contract/UDPSocket.h \
	networklayer/contract/IPv4Address.h \
	networklayer/contract/IPv6Address.h \
	transport/udp/contract/UDPControlInfo_m.h
$O/transport/udp/contract/UDPControlInfo_m.o: transport/udp/contract/UDPControlInfo_m.cc \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h \
	transport/udp/contract/UDPControlInfo_m.h
$O/underTest/applications/TrafficGenerator_Erlangen/WiMAXQoSTrafficGenerator.o: underTest/applications/TrafficGenerator_Erlangen/WiMAXQoSTrafficGenerator.cc \
	linklayer/radio/SnrList.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	applications/trafgen/TrafGen.h \
	base/INETDefs.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	networklayer/common/InterfaceToken.h \
	networklayer/ipv4/IPv4Datagram.h \
	networklayer/common/IInterfaceTable.h \
	underTest/applications/TrafficGenerator_Erlangen/WiMAXQoSTrafficGenerator.h \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216Primitives_m.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h
$O/underTest/applications/httpApp/httptBrowserBase.o: underTest/applications/httpApp/httptBrowserBase.cc \
	underTest/applications/httpApp/httptUtils.h \
	underTest/applications/httpApp/httptEventMessages_m.h \
	underTest/applications/httpApp/httptRandom.h \
	base/INETDefs.h \
	underTest/applications/httpApp/httptMessages_m.h \
	underTest/applications/httpApp/httptBrowserBase.h \
	underTest/applications/httpApp/httptLogdefs.h \
	underTest/applications/httpApp/httptNodeBase.h \
	underTest/applications/httpApp/httptController.h
$O/underTest/applications/httpApp/httptServerEvilA.o: underTest/applications/httpApp/httptServerEvilA.cc \
	underTest/applications/httpApp/httptEventMessages_m.h \
	underTest/applications/httpApp/httptServer.h \
	underTest/applications/httpApp/httptRandom.h \
	base/INETDefs.h \
	underTest/applications/httpApp/httptMessages_m.h \
	networklayer/contract/IPv4Address.h \
	underTest/applications/httpApp/httptLogdefs.h \
	underTest/applications/httpApp/httptServerBase.h \
	underTest/applications/httpApp/httptUtils.h \
	transport/tcp_common/contract/TCPSocket.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/contract/IPvXAddress.h \
	underTest/applications/httpApp/httptNodeBase.h \
	networklayer/contract/IPv6Address.h \
	underTest/applications/httpApp/httptController.h \
	transport/tcp_common/contract/TCPSocketMap.h \
	underTest/applications/httpApp/httptServerEvilA.h
$O/underTest/applications/httpApp/httptServerDirect.o: underTest/applications/httpApp/httptServerDirect.cc \
	underTest/applications/httpApp/httptUtils.h \
	underTest/applications/httpApp/httptEventMessages_m.h \
	underTest/applications/httpApp/httptRandom.h \
	base/INETDefs.h \
	underTest/applications/httpApp/httptMessages_m.h \
	underTest/applications/httpApp/httptLogdefs.h \
	underTest/applications/httpApp/httptNodeBase.h \
	underTest/applications/httpApp/httptController.h \
	underTest/applications/httpApp/httptServerDirect.h \
	underTest/applications/httpApp/httptServerBase.h
$O/underTest/applications/httpApp/httptMessages_m.o: underTest/applications/httpApp/httptMessages_m.cc \
	underTest/applications/httpApp/httptMessages_m.h
$O/underTest/applications/httpApp/httptRandom.o: underTest/applications/httpApp/httptRandom.cc \
	underTest/applications/httpApp/httptUtils.h \
	underTest/applications/httpApp/httptRandom.h
$O/underTest/applications/httpApp/httptNodeBase.o: underTest/applications/httpApp/httptNodeBase.cc \
	underTest/applications/httpApp/httptUtils.h \
	underTest/applications/httpApp/httptEventMessages_m.h \
	underTest/applications/httpApp/httptRandom.h \
	base/INETDefs.h \
	underTest/applications/httpApp/httptMessages_m.h \
	underTest/applications/httpApp/httptLogdefs.h \
	underTest/applications/httpApp/httptNodeBase.h \
	underTest/applications/httpApp/httptController.h
$O/underTest/applications/httpApp/httptBrowserDirect.o: underTest/applications/httpApp/httptBrowserDirect.cc \
	underTest/applications/httpApp/httptUtils.h \
	underTest/applications/httpApp/httptRandom.h \
	underTest/applications/httpApp/httptEventMessages_m.h \
	base/INETDefs.h \
	underTest/applications/httpApp/httptMessages_m.h \
	underTest/applications/httpApp/httptBrowserBase.h \
	underTest/applications/httpApp/httptBrowserDirect.h \
	underTest/applications/httpApp/httptLogdefs.h \
	underTest/applications/httpApp/httptNodeBase.h \
	underTest/applications/httpApp/httptController.h
$O/underTest/applications/httpApp/httptServerDirectEvilA.o: underTest/applications/httpApp/httptServerDirectEvilA.cc \
	underTest/applications/httpApp/httptUtils.h \
	underTest/applications/httpApp/httptServerDirectEvilA.h \
	underTest/applications/httpApp/httptEventMessages_m.h \
	underTest/applications/httpApp/httptRandom.h \
	base/INETDefs.h \
	underTest/applications/httpApp/httptMessages_m.h \
	underTest/applications/httpApp/httptNodeBase.h \
	underTest/applications/httpApp/httptLogdefs.h \
	underTest/applications/httpApp/httptController.h \
	underTest/applications/httpApp/httptServerDirect.h \
	underTest/applications/httpApp/httptServerBase.h
$O/underTest/applications/httpApp/httptUtils.o: underTest/applications/httpApp/httptUtils.cc \
	underTest/applications/httpApp/httptUtils.h
$O/underTest/applications/httpApp/httptServer.o: underTest/applications/httpApp/httptServer.cc \
	underTest/applications/httpApp/httptEventMessages_m.h \
	underTest/applications/httpApp/httptServer.h \
	underTest/applications/httpApp/httptRandom.h \
	base/INETDefs.h \
	underTest/applications/httpApp/httptMessages_m.h \
	networklayer/contract/IPv4Address.h \
	underTest/applications/httpApp/httptLogdefs.h \
	underTest/applications/httpApp/httptServerBase.h \
	underTest/applications/httpApp/httptUtils.h \
	transport/tcp_common/contract/TCPSocket.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/contract/IPvXAddress.h \
	underTest/applications/httpApp/httptNodeBase.h \
	networklayer/contract/IPv6Address.h \
	underTest/applications/httpApp/httptController.h \
	transport/tcp_common/contract/TCPSocketMap.h
$O/underTest/applications/httpApp/httptBrowser.o: underTest/applications/httpApp/httptBrowser.cc \
	underTest/applications/httpApp/httptEventMessages_m.h \
	underTest/applications/httpApp/httptRandom.h \
	underTest/applications/httpApp/httptBrowser.h \
	base/INETDefs.h \
	underTest/applications/httpApp/httptMessages_m.h \
	networklayer/contract/IPv4Address.h \
	underTest/applications/httpApp/httptLogdefs.h \
	underTest/applications/httpApp/httptUtils.h \
	transport/tcp_common/contract/TCPSocket.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/contract/IPvXAddressResolver.h \
	underTest/applications/httpApp/httptBrowserBase.h \
	underTest/applications/httpApp/httptNodeBase.h \
	networklayer/contract/IPv6Address.h \
	underTest/applications/httpApp/httptController.h \
	transport/tcp_common/contract/TCPSocketMap.h
$O/underTest/applications/httpApp/httptServerBase.o: underTest/applications/httpApp/httptServerBase.cc \
	underTest/applications/httpApp/httptUtils.h \
	underTest/applications/httpApp/httptEventMessages_m.h \
	underTest/applications/httpApp/httptRandom.h \
	base/INETDefs.h \
	underTest/applications/httpApp/httptMessages_m.h \
	underTest/applications/httpApp/httptNodeBase.h \
	underTest/applications/httpApp/httptLogdefs.h \
	underTest/applications/httpApp/httptController.h \
	underTest/applications/httpApp/httptServerBase.h
$O/underTest/applications/httpApp/httptEventMessages_m.o: underTest/applications/httpApp/httptEventMessages_m.cc \
	underTest/applications/httpApp/httptEventMessages_m.h
$O/underTest/applications/httpApp/httptController.o: underTest/applications/httpApp/httptController.cc \
	underTest/applications/httpApp/httptUtils.h \
	underTest/applications/httpApp/httptRandom.h \
	underTest/applications/httpApp/httptEventMessages_m.h \
	underTest/applications/httpApp/httptLogdefs.h \
	underTest/applications/httpApp/httptController.h
$O/underTest/applications/httpApp/httptServerDirectEvilB.o: underTest/applications/httpApp/httptServerDirectEvilB.cc \
	underTest/applications/httpApp/httptUtils.h \
	underTest/applications/httpApp/httptEventMessages_m.h \
	underTest/applications/httpApp/httptRandom.h \
	base/INETDefs.h \
	underTest/applications/httpApp/httptServerDirectEvilB.h \
	underTest/applications/httpApp/httptMessages_m.h \
	underTest/applications/httpApp/httptNodeBase.h \
	underTest/applications/httpApp/httptLogdefs.h \
	underTest/applications/httpApp/httptController.h \
	underTest/applications/httpApp/httptServerDirect.h \
	underTest/applications/httpApp/httptServerBase.h
$O/underTest/applications/httpApp/httptServerEvilB.o: underTest/applications/httpApp/httptServerEvilB.cc \
	underTest/applications/httpApp/httptEventMessages_m.h \
	underTest/applications/httpApp/httptServer.h \
	underTest/applications/httpApp/httptRandom.h \
	base/INETDefs.h \
	underTest/applications/httpApp/httptMessages_m.h \
	networklayer/contract/IPv4Address.h \
	underTest/applications/httpApp/httptLogdefs.h \
	underTest/applications/httpApp/httptServerEvilB.h \
	underTest/applications/httpApp/httptServerBase.h \
	underTest/applications/httpApp/httptUtils.h \
	transport/tcp_common/contract/TCPSocket.h \
	transport/tcp_common/contract/TCPCommand_m.h \
	networklayer/contract/IPvXAddress.h \
	underTest/applications/httpApp/httptNodeBase.h \
	networklayer/contract/IPv6Address.h \
	underTest/applications/httpApp/httptController.h \
	transport/tcp_common/contract/TCPSocketMap.h
$O/underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneMobilestation.o: underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneMobilestation.cc \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.h \
	underTest/linklayer/Ieee80216/ControlPlane/ModulationsConsts.h \
	linklayer/radio/SnrList.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	underTest/linklayer/Ieee80216/ControlPlane/Struct.h
$O/underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBaseAP.o: underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBaseAP.cc \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.h \
	underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBaseAP.h \
	underTest/linklayer/Ieee80216/Mac/WiMAXRadioControl_m.h \
	linklayer/radio/SnrList.h \
	linklayer/contract/PhyControlInfo_m.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	base/INETDefs.h \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216Primitives_m.h \
	underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneAP.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	underTest/linklayer/Ieee80216/ControlPlane/Struct.h
$O/underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBaseMS.o: underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBaseMS.cc
$O/underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneAP.o: underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneAP.cc \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.h \
	underTest/linklayer/Ieee80216/Mac/WiMAXRadioControl_m.h \
	linklayer/radio/SnrList.h \
	linklayer/contract/PhyControlInfo_m.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	base/INETDefs.h \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216Primitives_m.h \
	underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneAP.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	underTest/linklayer/Ieee80216/ControlPlane/Struct.h
$O/underTest/linklayer/Ieee80216/ControlPlane/Ieee80216Primitives_m.o: underTest/linklayer/Ieee80216/ControlPlane/Ieee80216Primitives_m.cc \
	linklayer/radio/SnrList.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	base/INETDefs.h \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216Primitives_m.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h
$O/underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.o: underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.cc \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.h \
	linklayer/radio/SnrList.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	underTest/linklayer/Ieee80216/ControlPlane/Struct.h
$O/underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBase.o: underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBase.cc \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.h \
	world/powercontrol/IPowerControl.h \
	linklayer/radio/propagation/IReceptionModel.h \
	world/obstacles/Obstacle.h \
	world/radio/ChannelAccess.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/radio/AirFrame_m.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/radio/Ieee80216Radio.h \
	underTest/linklayer/radio/WiMAXPathLossReceptionModel.h \
	linklayer/contract/RadioState.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	util/FWMath.h \
	linklayer/radio/SnrList.h \
	underTest/linklayer/radio/Ieee80216Consts.h \
	linklayer/contract/PhyControlInfo_m.h \
	underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBase.h \
	base/Coord.h \
	base/NotifierConsts.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	underTest/linklayer/Ieee80216/ControlPlane/Struct.h \
	linklayer/radio/IRadioModel.h \
	world/radio/IChannelControl.h \
	world/annotations/AnnotationManager.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	linklayer/radio/ModulationType.h \
	world/obstacles/ObstacleControl.h \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216Primitives_m.h \
	linklayer/radio/Radio.h \
	base/BasicModule.h
$O/underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneMS.o: underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneMS.cc \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.h \
	underTest/linklayer/Ieee80216/Mac/WiMAXRadioControl_m.h \
	linklayer/radio/SnrList.h \
	linklayer/contract/PhyControlInfo_m.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	base/INETDefs.h \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216Primitives_m.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneMS.h \
	underTest/linklayer/Ieee80216/ControlPlane/Struct.h
$O/underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBasestation.o: underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBasestation.cc \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.h \
	world/powercontrol/IPowerControl.h \
	underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBasestation.h \
	linklayer/radio/propagation/IReceptionModel.h \
	world/obstacles/Obstacle.h \
	world/radio/ChannelAccess.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/radio/AirFrame_m.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows_BS.h \
	underTest/linklayer/radio/Ieee80216Radio.h \
	underTest/linklayer/radio/WiMAXPathLossReceptionModel.h \
	networklayer/common/InterfaceToken.h \
	linklayer/contract/RadioState.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerScheduling.h \
	networklayer/common/IInterfaceTable.h \
	base/FSMA.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h \
	util/FWMath.h \
	underTest/linklayer/Ieee80216/MACSublayer/Receiver/CommonPartSublayerReceiver.h \
	linklayer/radio/SnrList.h \
	underTest/linklayer/radio/Ieee80216Consts.h \
	linklayer/contract/PhyControlInfo_m.h \
	networklayer/common/InterfaceTableAccess.h \
	underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBase.h \
	base/Coord.h \
	base/NotifierConsts.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows.h \
	underTest/linklayer/Ieee80216/ControlPlane/Struct.h \
	world/radio/IChannelControl.h \
	linklayer/radio/IRadioModel.h \
	underTest/linklayer/Ieee80216/ControlPlane/ModulationsConsts.h \
	world/annotations/AnnotationManager.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	linklayer/radio/ModulationType.h \
	world/obstacles/ObstacleControl.h \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216Primitives_m.h \
	linklayer/radio/Radio.h \
	base/BasicModule.h
$O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerScheduling.o: underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerScheduling.cc \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	linklayer/radio/SnrList.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/ControlPlane/Struct.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerScheduling.h \
	networklayer/ipv4/IPv4Datagram.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216Primitives_m.h \
	networklayer/contract/IPProtocolId_m.h
$O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerFragmentation.o: underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerFragmentation.cc \
	linklayer/contract/PhyControlInfo_m.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerFragmentation.h \
	base/INETDefs.h
$O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows_MS.o: underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows_MS.cc \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.h \
	linklayer/radio/SnrList.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	base/INETDefs.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows_MS.h \
	underTest/linklayer/Ieee80216/ControlPlane/Struct.h
$O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows_BS.o: underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows_BS.cc \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.h \
	linklayer/radio/SnrList.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows.h \
	underTest/linklayer/Ieee80216/ControlPlane/Struct.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows_BS.h
$O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerControlModule.o: underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerControlModule.cc \
	linklayer/contract/PhyControlInfo_m.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerControlModule.h \
	base/INETDefs.h
$O/underTest/linklayer/Ieee80216/MACSublayer/QoS/ConvergenceSublayerTrafficClassification.o: underTest/linklayer/Ieee80216/MACSublayer/QoS/ConvergenceSublayerTrafficClassification.cc \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	world/radio/ChannelAccess.h \
	linklayer/radio/AirFrame_m.h \
	base/QueueBase.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows_BS.h \
	underTest/linklayer/radio/WiMAXPathLossReceptionModel.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerScheduling.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	base/FSMA.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	util/FWMath.h \
	underTest/linklayer/Ieee80216/MACSublayer/Receiver/CommonPartSublayerReceiver.h \
	base/AbstractQueue.h \
	linklayer/radio/SnrList.h \
	underTest/linklayer/radio/Ieee80216Consts.h \
	underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneMobilestation.h \
	underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBase.h \
	base/NotifierConsts.h \
	networklayer/ipv4/IPv4Route.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows_MS.h \
	world/annotations/AnnotationManager.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	networklayer/ipv4/IPv4Datagram.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/ConvergenceSublayerTrafficClassification.h \
	linklayer/radio/ModulationType.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/ICMPAccess.h \
	base/BasicModule.h \
	underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBasestation.h \
	world/powercontrol/IPowerControl.h \
	linklayer/radio/propagation/IReceptionModel.h \
	world/radio/ChannelControl.h \
	world/obstacles/Obstacle.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/radio/Ieee80216Radio.h \
	networklayer/ipv4/ICMPMessage.h \
	linklayer/contract/RadioState.h \
	base/NotificationBoard.h \
	networklayer/ipv4/IPv4.h \
	base/ProtocolMap.h \
	linklayer/contract/PhyControlInfo_m.h \
	util/uint128.h \
	networklayer/ipv4/RoutingTableAccess.h \
	networklayer/common/InterfaceTableAccess.h \
	base/Coord.h \
	networklayer/contract/IPv4Address.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	underTest/linklayer/Ieee80216/ControlPlane/Struct.h \
	world/radio/IChannelControl.h \
	linklayer/radio/IRadioModel.h \
	networklayer/ipv4/IPv4FragBuf.h \
	networklayer/ipv4/ICMP.h \
	world/obstacles/ObstacleControl.h \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216Primitives_m.h \
	networklayer/contract/IPv6Address.h \
	linklayer/radio/Radio.h \
	networklayer/ipv4/IRoutingTable.h \
	base/ReassemblyBuffer.h
$O/underTest/linklayer/Ieee80216/MACSublayer/QoS/ConvergenceSublayerControlModule.o: underTest/linklayer/Ieee80216/MACSublayer/QoS/ConvergenceSublayerControlModule.cc \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.h \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	base/QueueBase.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/ConvergenceSublayerControlModule.h \
	networklayer/ipv4/contract/IPv4ControlInfo_m.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/ipv4/contract/IPv4ControlInfo.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	networklayer/ipv4/IPv4.h \
	base/ProtocolMap.h \
	linklayer/radio/SnrList.h \
	base/AbstractQueue.h \
	linklayer/contract/PhyControlInfo_m.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	underTest/linklayer/Ieee80216/ControlPlane/Struct.h \
	networklayer/ipv4/IPv4FragBuf.h \
	networklayer/ipv4/ICMP.h \
	networklayer/ipv4/IPv4Datagram.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/ICMPAccess.h \
	base/ReassemblyBuffer.h \
	networklayer/ipv4/IRoutingTable.h
$O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows.o: underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows.cc \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.h \
	linklayer/radio/SnrList.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows.h \
	underTest/linklayer/Ieee80216/ControlPlane/Struct.h
$O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerAuthorizationModule.o: underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerAuthorizationModule.cc \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerAuthorizationModule.h \
	linklayer/radio/SnrList.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h
$O/underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerQoSAP.o: underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerQoSAP.cc \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerQoSAP.h \
	networklayer/common/InterfaceToken.h \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h \
	networklayer/common/IInterfaceTable.h \
	networklayer/common/InterfaceTableAccess.h \
	base/NotifierConsts.h \
	base/ModuleAccess.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceEntry.h
$O/underTest/linklayer/Ieee80216/MACSublayer/QoS/ConvergenceSublayerHeaderCompression.o: underTest/linklayer/Ieee80216/MACSublayer/QoS/ConvergenceSublayerHeaderCompression.cc \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/ConvergenceSublayerHeaderCompression.h
$O/underTest/linklayer/Ieee80216/MACSublayer/Receiver/CommonPartSublayerReceiver.o: underTest/linklayer/Ieee80216/MACSublayer/Receiver/CommonPartSublayerReceiver.cc \
	underTest/linklayer/Ieee80216/MACSublayer/Receiver/CommonPartSublayerReceiver.h \
	linklayer/radio/SnrList.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h
$O/underTest/linklayer/Ieee80216/MACSublayer/Receiver/ConvergenceSublayerReceiver.o: underTest/linklayer/Ieee80216/MACSublayer/Receiver/ConvergenceSublayerReceiver.cc \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h \
	underTest/linklayer/Ieee80216/MACSublayer/Receiver/ConvergenceSublayerReceiver.h
$O/underTest/linklayer/Ieee80216/MACSublayer/Receiver/SecuritySublayerReceiver.o: underTest/linklayer/Ieee80216/MACSublayer/Receiver/SecuritySublayerReceiver.cc \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h \
	underTest/linklayer/Ieee80216/MACSublayer/Receiver/SecuritySublayerReceiver.h
$O/underTest/linklayer/Ieee80216/MACSublayer/Transceiver/ConvergenceSublayerTransceiver.o: underTest/linklayer/Ieee80216/MACSublayer/Transceiver/ConvergenceSublayerTransceiver.cc \
	underTest/linklayer/Ieee80216/MACSublayer/Transceiver/ConvergenceSublayerTransceiver.h \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h
$O/underTest/linklayer/Ieee80216/MACSublayer/Transceiver/SecuritySublayerTransceiver.o: underTest/linklayer/Ieee80216/MACSublayer/Transceiver/SecuritySublayerTransceiver.cc \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h \
	underTest/linklayer/Ieee80216/MACSublayer/Transceiver/SecuritySublayerTransceiver.h
$O/underTest/linklayer/Ieee80216/MACSublayer/Transceiver/CommonPartSublayerTransceiver.o: underTest/linklayer/Ieee80216/MACSublayer/Transceiver/CommonPartSublayerTransceiver.cc \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216ManagementMessages_m.h \
	world/powercontrol/IPowerControl.h \
	underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBasestation.h \
	linklayer/radio/propagation/IReceptionModel.h \
	world/obstacles/Obstacle.h \
	world/radio/ChannelControl.h \
	world/radio/ChannelAccess.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/radio/AirFrame_m.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows_BS.h \
	underTest/linklayer/radio/Ieee80216Radio.h \
	underTest/linklayer/radio/WiMAXPathLossReceptionModel.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerScheduling.h \
	networklayer/common/InterfaceToken.h \
	linklayer/contract/RadioState.h \
	networklayer/common/IInterfaceTable.h \
	base/FSMA.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h \
	util/FWMath.h \
	base/IPassiveQueue.h \
	underTest/linklayer/Ieee80216/MACSublayer/Receiver/CommonPartSublayerReceiver.h \
	linklayer/radio/SnrList.h \
	underTest/linklayer/radio/Ieee80216Consts.h \
	underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneMobilestation.h \
	linklayer/contract/PhyControlInfo_m.h \
	networklayer/common/InterfaceTableAccess.h \
	underTest/linklayer/Ieee80216/ControlPlane/ControlPlaneBase.h \
	base/Coord.h \
	base/NotifierConsts.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/CommonPartSublayerServiceFlows_MS.h \
	underTest/linklayer/Ieee80216/ControlPlane/Struct.h \
	world/radio/IChannelControl.h \
	linklayer/radio/IRadioModel.h \
	world/annotations/AnnotationManager.h \
	underTest/linklayer/Ieee80216/MACSublayer/QoS/global_enums.h \
	linklayer/radio/ModulationType.h \
	world/obstacles/ObstacleControl.h \
	underTest/linklayer/Ieee80216/ControlPlane/Ieee80216Primitives_m.h \
	underTest/linklayer/Ieee80216/MACSublayer/Transceiver/CommonPartSublayerTransceiver.h \
	linklayer/radio/Radio.h \
	base/BasicModule.h
$O/underTest/linklayer/Ieee80216/Mac/ConvergenceSublayerUp.o: underTest/linklayer/Ieee80216/Mac/ConvergenceSublayerUp.cc \
	underTest/linklayer/Ieee80216/Mac/ConvergenceSublayerUp.h \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h
$O/underTest/linklayer/Ieee80216/Mac/CommonPartSublayerDown.o: underTest/linklayer/Ieee80216/Mac/CommonPartSublayerDown.cc \
	underTest/linklayer/Ieee80216/Mac/CommonPartSublayerDown.h \
	underTest/linklayer/Ieee80216/Mac/WiMAXRadioControl_m.h \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h
$O/underTest/linklayer/Ieee80216/Mac/CommonPartSublayerUp.o: underTest/linklayer/Ieee80216/Mac/CommonPartSublayerUp.cc \
	underTest/linklayer/Ieee80216/Mac/WiMAXRadioControl_m.h \
	linklayer/contract/PhyControlInfo_m.h \
	underTest/linklayer/Ieee80216/Mac/CommonPartSublayerUp.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h
$O/underTest/linklayer/Ieee80216/Mac/WiMAXRadioControl_m.o: underTest/linklayer/Ieee80216/Mac/WiMAXRadioControl_m.cc \
	underTest/linklayer/Ieee80216/Mac/WiMAXRadioControl_m.h
$O/underTest/linklayer/Ieee80216/Mac/SecuritySublayerUp.o: underTest/linklayer/Ieee80216/Mac/SecuritySublayerUp.cc \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h \
	underTest/linklayer/Ieee80216/Mac/SecuritySublayerUp.h
$O/underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.o: underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.cc \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h
$O/underTest/linklayer/Ieee80216/Mac/ConvergenceSublayerDown.o: underTest/linklayer/Ieee80216/Mac/ConvergenceSublayerDown.cc \
	linklayer/contract/PhyControlInfo_m.h \
	underTest/linklayer/Ieee80216/Mac/ConvergenceSublayerDown.h \
	base/INETDefs.h
$O/underTest/linklayer/Ieee80216/Mac/SecuritySublayerDown.o: underTest/linklayer/Ieee80216/Mac/SecuritySublayerDown.cc \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h \
	underTest/linklayer/Ieee80216/Mac/SecuritySublayerDown.h
$O/underTest/linklayer/Ieee80216/phyLayer/decider/Decider80216.o: underTest/linklayer/Ieee80216/phyLayer/decider/Decider80216.cc \
	util/FWMath.h \
	linklayer/radio/SnrList.h \
	world/radio/ChannelAccess.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/radio/AirFrame_m.h \
	base/Coord.h \
	base/NotifierConsts.h \
	linklayer/mf80211/core/SnrControlInfo_m.h \
	world/radio/IChannelControl.h \
	underTest/linklayer/Ieee80216/phyLayer/decider/Decider80216.h \
	linklayer/mf80211/core/BasicDecider.h \
	linklayer/radio/ModulationType.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	base/BasicModule.h
$O/underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtSTAExtended.o: underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtSTAExtended.cc \
	world/radio/ChannelControl.h \
	world/radio/ChannelAccess.h \
	linklayer/ieee80211/mgmt/Ieee80211Primitives_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtSTAExtended.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h \
	linklayer/contract/MACAddress.h \
	networklayer/common/InterfaceToken.h \
	linklayer/contract/RadioState.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h \
	base/IPassiveQueue.h \
	util/FWMath.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	linklayer/contract/PhyControlInfo_m.h \
	networklayer/common/InterfaceTableAccess.h \
	base/Coord.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	base/NotifierConsts.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	world/radio/IChannelControl.h \
	linklayer/radio/Radio80211aControlInfo_m.h \
	linklayer/radio/ModulationType.h \
	base/BasicModule.h \
	base/PassiveQueueBase.h
$O/underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtAPExtended.o: underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtAPExtended.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtAPBase.h \
	linklayer/contract/MACAddress.h \
	base/QueueBase.h \
	networklayer/ipv4/ICMPMessage.h \
	linklayer/contract/RadioState.h \
	linklayer/ethernet/Ethernet.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	base/IPassiveQueue.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	networklayer/ipv4/IPv4.h \
	base/ProtocolMap.h \
	base/AbstractQueue.h \
	util/uint128.h \
	networklayer/ipv4/RoutingTableAccess.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	networklayer/contract/IPv4Address.h \
	underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtAPExtended.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/ipv4/IPv4FragBuf.h \
	networklayer/ipv4/ICMP.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv4/ICMPAccess.h \
	linklayer/ethernet/EtherFrame_m.h \
	base/PassiveQueueBase.h \
	networklayer/ipv4/IRoutingTable.h \
	base/ReassemblyBuffer.h
$O/underTest/linklayer/ieee80211/mgmt/Ieee80211AgentSTAExtended.o: underTest/linklayer/ieee80211/mgmt/Ieee80211AgentSTAExtended.cc \
	underTest/linklayer/ieee80211/mgmt/Ieee80211AgentSTAExtended.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	linklayer/ieee80211/mgmt/Ieee80211Primitives_m.h \
	base/INETDefs.h \
	base/INotifiable.h \
	networklayer/common/InterfaceTableAccess.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h
$O/underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtBaseExtended.o: underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtBaseExtended.cc \
	base/IPassiveQueue.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	networklayer/common/InterfaceTableAccess.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtBaseExtended.h \
	networklayer/common/InterfaceToken.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h \
	base/PassiveQueueBase.h
$O/underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtWDSExtended.o: underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtWDSExtended.cc \
	networklayer/ipv4/IPv4RouteRule.h \
	networklayer/manetrouting/base/ControlManetRouting_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	linklayer/contract/MACAddress.h \
	base/QueueBase.h \
	underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtBaseExtended.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/common/InterfaceToken.h \
	linklayer/contract/RadioState.h \
	networklayer/common/IInterfaceTable.h \
	linklayer/ethernet/Ethernet.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	base/IPassiveQueue.h \
	linklayer/ieee80211/mac/Ieee80211Frame_m.h \
	networklayer/ipv4/IPv4.h \
	base/ProtocolMap.h \
	base/AbstractQueue.h \
	networklayer/ipv4/RoutingTableAccess.h \
	util/uint128.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	networklayer/contract/IPv4Address.h \
	networklayer/ipv4/IPv4Route.h \
	base/NotifierConsts.h \
	linklayer/ieee80211/mgmt/Ieee80211MgmtFrames_m.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/ipv4/IPv4FragBuf.h \
	underTest/linklayer/ieee80211/mgmt/Ieee80211MgmtWDSExtended.h \
	networklayer/ipv4/ICMP.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPAccess.h \
	networklayer/ipv4/ICMPMessage_m.h \
	linklayer/ethernet/EtherFrame_m.h \
	base/PassiveQueueBase.h \
	base/ReassemblyBuffer.h \
	networklayer/ipv4/IRoutingTable.h
$O/underTest/linklayer/radio/Ieee80216RadioModel.o: underTest/linklayer/radio/Ieee80216RadioModel.cc \
	util/FWMath.h \
	linklayer/radio/IRadioModel.h \
	linklayer/radio/SnrList.h \
	base/INETDefs.h \
	linklayer/radio/ModulationType.h \
	linklayer/radio/AirFrame_m.h \
	base/Coord.h \
	linklayer/ieee80211/mac/Ieee80211Consts.h \
	underTest/linklayer/radio/Ieee80216RadioModel.h
$O/underTest/linklayer/radio/WiMAXPathLossReceptionModel.o: underTest/linklayer/radio/WiMAXPathLossReceptionModel.cc \
	util/FWMath.h \
	linklayer/radio/propagation/IReceptionModel.h \
	underTest/linklayer/radio/WiMAXPathLossReceptionModel.h \
	base/INETDefs.h
$O/underTest/linklayer/radio/Ieee80216Radio.o: underTest/linklayer/radio/Ieee80216Radio.cc \
	world/powercontrol/IPowerControl.h \
	linklayer/radio/propagation/IReceptionModel.h \
	world/obstacles/Obstacle.h \
	world/radio/ChannelAccess.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/radio/AirFrame_m.h \
	linklayer/contract/MACAddress.h \
	underTest/linklayer/radio/Ieee80216Radio.h \
	underTest/linklayer/radio/WiMAXPathLossReceptionModel.h \
	linklayer/contract/RadioState.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	util/FWMath.h \
	linklayer/radio/SnrList.h \
	underTest/linklayer/radio/Ieee80216Consts.h \
	base/Coord.h \
	base/NotifierConsts.h \
	underTest/linklayer/Ieee80216/Mac/Ieee80216MacHeader_m.h \
	world/radio/IChannelControl.h \
	linklayer/radio/IRadioModel.h \
	world/annotations/AnnotationManager.h \
	linklayer/radio/ModulationType.h \
	world/obstacles/ObstacleControl.h \
	linklayer/radio/Radio.h \
	base/BasicModule.h
$O/util/MessageChecker.o: util/MessageChecker.cc \
	base/INETDefs.h \
	util/MessageChecker.h
$O/util/DuplicatesGenerator.o: util/DuplicatesGenerator.cc \
	base/INETDefs.h \
	util/DuplicatesGenerator.h
$O/util/int128.o: util/int128.cc \
	base/INETDefs.h \
	util/int128.h \
	networklayer/contract/IPv4Address.h
$O/util/ThruputMeteringChannel.o: util/ThruputMeteringChannel.cc \
	util/ThruputMeteringChannel.h \
	base/INETDefs.h
$O/util/TCPDump.o: util/TCPDump.cc \
	util/TCPDump.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	util/PacketDump.h \
	networklayer/ipv4/IPv4Datagram.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	util/PcapDump.h \
	networklayer/contract/IPProtocolId_m.h
$O/util/XMLUtils.o: util/XMLUtils.cc \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPvXAddressResolver.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h \
	util/XMLUtils.h
$O/util/ThruputMeter.o: util/ThruputMeter.cc \
	util/ThruputMeter.h \
	base/INETDefs.h
$O/util/PcapRecorder.o: util/PcapRecorder.cc \
	util/PacketDump.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	networklayer/ipv4/IPv4Datagram.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	util/PcapDump.h \
	util/PcapRecorder.h \
	networklayer/contract/IPProtocolId_m.h
$O/util/opp_utils.o: util/opp_utils.cc \
	util/opp_utils.h \
	base/INETDefs.h
$O/util/common.o: util/common.cc \
	util/common.h \
	networklayer/contract/IPvXAddress.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h
$O/util/PcapDump.o: util/PcapDump.cc \
	transport/udp/UDPPacket_m.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	util/headerserializers/ipv4/IPv4Serializer.h \
	networklayer/ipv4/IPv4Datagram.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	util/PcapDump.h \
	networklayer/contract/IPProtocolId_m.h
$O/util/NetAnimTrace.o: util/NetAnimTrace.cc \
	util/NetAnimTrace.h \
	base/INETDefs.h
$O/util/PacketDump.o: util/PacketDump.cc \
	transport/sctp/SCTPMessage.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	networklayer/ipv4/ICMPMessage.h \
	networklayer/common/InterfaceToken.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	transport/sctp/SCTPQueue.h \
	transport/tcp_common/TCPSegment.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	networklayer/contract/IPProtocolId_m.h \
	networklayer/common/InterfaceEntry.h \
	transport/sctp/SCTPReceiveStream.h \
	networklayer/common/InterfaceTable.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	transport/udp/UDPPacket_m.h \
	util/PacketDump.h \
	networklayer/ipv4/IPv4Datagram.h \
	transport/tcp_common/TCPSegment_m.h \
	networklayer/ipv6/IPv6Datagram.h \
	transport/sctp/SCTPSendStream.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/ipv6/IPv6Datagram_m.h
$O/util/uint128.o: util/uint128.cc \
	util/uint128.h \
	base/INETDefs.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPv4Address.h \
	linklayer/contract/MACAddress.h
$O/util/Macho.o: util/Macho.cc \
	util/Macho.h
$O/util/DropsGenerator.o: util/DropsGenerator.cc \
	base/INETDefs.h \
	util/DropsGenerator.h
$O/util/headerserializers/TCPIPchecksum.o: util/headerserializers/TCPIPchecksum.cc \
	util/headerserializers/TCPIPchecksum.h \
	base/INETDefs.h
$O/util/headerserializers/ipv4/ICMPSerializer.o: util/headerserializers/ipv4/ICMPSerializer.cc \
	util/headerserializers/ipv4/ICMPSerializer.h \
	util/headerserializers/TCPIPchecksum.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	util/headerserializers/headers/bsdint.h \
	util/headerserializers/ipv4/headers/ip_icmp.h \
	util/headerserializers/ipv4/headers/ip.h \
	applications/pingapp/PingPayload_m.h \
	util/headerserializers/headers/defs.h \
	util/headerserializers/ipv4/IPv4Serializer.h \
	networklayer/ipv4/IPv4Datagram.h \
	util/headerserializers/headers/in_systm.h \
	util/headerserializers/headers/in.h \
	networklayer/ipv4/ICMPMessage_m.h \
	networklayer/contract/IPProtocolId_m.h
$O/util/headerserializers/ipv4/IPv4Serializer.o: util/headerserializers/ipv4/IPv4Serializer.cc \
	transport/sctp/SCTPMessage.h \
	util/headerserializers/tcp/headers/tcp.h \
	util/headerserializers/sctp/SCTPSerializer.h \
	util/headerserializers/ipv4/ICMPSerializer.h \
	networklayer/ipv4/IPv4Datagram_m.h \
	base/INETDefs.h \
	util/headerserializers/headers/bsdint.h \
	util/headerserializers/tcp/TCPSerializer.h \
	util/headerserializers/ipv4/IPv4Serializer.h \
	util/headerserializers/headers/defs.h \
	util/headerserializers/headers/in_systm.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp_common/TCPSegment.h \
	util/headerserializers/headers/in.h \
	networklayer/contract/IPProtocolId_m.h \
	util/headerserializers/TCPIPchecksum.h \
	base/ByteArray.h \
	base/ByteArray_m.h \
	networklayer/contract/IPv4Address.h \
	util/headerserializers/udp/UDPSerializer.h \
	transport/sctp/SCTPMessage_m.h \
	transport/udp/UDPPacket_m.h \
	util/headerserializers/ipv4/headers/ip.h \
	networklayer/ipv4/IPv4Datagram.h \
	transport/tcp_common/TCPSegment_m.h \
	transport/udp/UDPPacket.h \
	networklayer/contract/IPv6Address.h \
	networklayer/ipv4/ICMPMessage_m.h
$O/util/headerserializers/sctp/SCTPSerializer.o: util/headerserializers/sctp/SCTPSerializer.cc \
	transport/sctp/SCTPMessage.h \
	util/headerserializers/sctp/SCTPSerializer.h \
	transport/sctp/SCTP.h \
	base/INETDefs.h \
	base/INotifiable.h \
	linklayer/contract/MACAddress.h \
	util/headerserializers/headers/bsdint.h \
	util/headerserializers/sctp/headers/sctp.h \
	util/headerserializers/headers/defs.h \
	networklayer/common/InterfaceToken.h \
	util/headerserializers/headers/in_systm.h \
	networklayer/contract/IPvXAddress.h \
	networklayer/common/IInterfaceTable.h \
	util/headerserializers/headers/in.h \
	transport/sctp/SCTPQueue.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	networklayer/common/InterfaceEntry.h \
	transport/sctp/SCTPReceiveStream.h \
	networklayer/common/InterfaceTable.h \
	networklayer/common/InterfaceTableAccess.h \
	networklayer/contract/IPv4Address.h \
	base/NotifierConsts.h \
	transport/sctp/SCTPAssociation.h \
	transport/sctp/SCTPMessage_m.h \
	util/headerserializers/ipv4/headers/ip.h \
	transport/sctp/SCTPSendStream.h \
	networklayer/contract/IPv6Address.h
$O/util/headerserializers/tcp/TCPSerializer.o: util/headerserializers/tcp/TCPSerializer.cc \
	util/headerserializers/tcp/headers/tcp.h \
	util/headerserializers/TCPIPchecksum.h \
	base/ByteArray_m.h \
	base/ByteArray.h \
	base/INETDefs.h \
	networklayer/contract/IPv4Address.h \
	util/headerserializers/headers/bsdint.h \
	util/headerserializers/tcp/TCPSerializer.h \
	transport/tcp_common/TCPSegment_m.h \
	util/headerserializers/headers/defs.h \
	util/headerserializers/headers/in_systm.h \
	networklayer/contract/IPvXAddress.h \
	transport/tcp_common/TCPSegment.h \
	util/headerserializers/headers/in.h \
	networklayer/contract/IPv6Address.h \
	networklayer/contract/IPProtocolId_m.h
$O/util/headerserializers/udp/UDPSerializer.o: util/headerserializers/udp/UDPSerializer.cc \
	util/headerserializers/udp/UDPSerializer.h \
	util/headerserializers/TCPIPchecksum.h \
	transport/udp/UDPPacket_m.h \
	util/headerserializers/headers/defs.h \
	util/headerserializers/headers/in_systm.h \
	base/INETDefs.h \
	transport/udp/UDPPacket.h \
	util/headerserializers/headers/in.h \
	util/headerserializers/udp/headers/udp.h \
	util/headerserializers/headers/bsdint.h
$O/world/annotations/AnnotationManager.o: world/annotations/AnnotationManager.cc \
	util/FWMath.h \
	world/annotations/AnnotationManager.h \
	base/INETDefs.h \
	base/Coord.h \
	base/ModuleAccess.h
$O/world/annotations/AnnotationDummy.o: world/annotations/AnnotationDummy.cc \
	base/INETDefs.h \
	world/annotations/AnnotationDummy.h
$O/world/obstacles/ObstacleControl.o: world/obstacles/ObstacleControl.cc \
	util/FWMath.h \
	world/obstacles/Obstacle.h \
	world/annotations/AnnotationManager.h \
	base/INETDefs.h \
	base/Coord.h \
	world/obstacles/ObstacleControl.h \
	base/ModuleAccess.h
$O/world/obstacles/Obstacle.o: world/obstacles/Obstacle.cc \
	util/FWMath.h \
	world/obstacles/Obstacle.h \
	world/annotations/AnnotationManager.h \
	base/INETDefs.h \
	base/Coord.h \
	base/ModuleAccess.h
$O/world/powercontrol/PowerControlMessages_m.o: world/powercontrol/PowerControlMessages_m.cc \
	world/powercontrol/PowerControlMessages_m.h
$O/world/powercontrol/PowerControlManager.o: world/powercontrol/PowerControlManager.cc \
	world/powercontrol/IPowerControl.h \
	world/powercontrol/PowerControlMessages_m.h \
	world/powercontrol/PowerControlManager.h \
	base/INETDefs.h
$O/world/radio/ChannelAccess.o: world/radio/ChannelAccess.cc \
	util/FWMath.h \
	world/radio/IChannelControl.h \
	world/radio/ChannelAccess.h \
	base/INotifiable.h \
	base/INETDefs.h \
	base/Coord.h \
	base/ModuleAccess.h \
	base/NotifierConsts.h \
	base/NotificationBoard.h \
	base/BasicModule.h
$O/world/radio/ChannelControl.o: world/radio/ChannelControl.cc \
	util/FWMath.h \
	world/radio/IChannelControl.h \
	world/radio/ChannelControl.h \
	base/INETDefs.h \
	linklayer/radio/ModulationType.h \
	base/Coord.h \
	linklayer/radio/AirFrame_m.h
$O/world/scenario/ScenarioManager.o: world/scenario/ScenarioManager.cc \
	world/scenario/ScenarioManager.h \
	base/INETDefs.h \
	base/IScriptable.h
$O/world/traci/TraCIScenarioManager.o: world/traci/TraCIScenarioManager.cc \
	util/FWMath.h \
	world/traci/TraCIConstants.h \
	base/INETDefs.h \
	base/INotifiable.h \
	mobility/traci/TraCIMobility.h \
	base/Coord.h \
	base/NotifierConsts.h \
	world/traci/TraCIScenarioManager.h \
	world/radio/IChannelControl.h \
	mobility/models/BasicMobility.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	base/BasicModule.h
$O/world/traci/TraCIScenarioManagerLaunchd.o: world/traci/TraCIScenarioManagerLaunchd.cc \
	util/FWMath.h \
	world/traci/TraCIScenarioManager.h \
	world/radio/IChannelControl.h \
	world/traci/TraCIConstants.h \
	base/INETDefs.h \
	base/Coord.h \
	base/ModuleAccess.h \
	world/traci/TraCIScenarioManagerLaunchd.h
$O/wpan/Isabel/Analysis/AnalysisEnergy.o: wpan/Isabel/Analysis/AnalysisEnergy.cc \
	util/FWMath.h \
	battery/models/BasicBattery.h \
	world/radio/IChannelControl.h \
	world/radio/ChannelControl.h \
	base/INETDefs.h \
	base/INotifiable.h \
	base/Coord.h \
	base/NotifierConsts.h \
	battery/models/Energy.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	wpan/Isabel/Analysis/AnalysisEnergy.h
$O/wpan/Isabel/Base/Display.o: wpan/Isabel/Base/Display.cc \
	wpan/Isabel/Base/Display.h
$O/wpan/applications/Ieee802154TestApp/Ieee802154AppPkt_m.o: wpan/applications/Ieee802154TestApp/Ieee802154AppPkt_m.cc \
	wpan/applications/Ieee802154TestApp/Ieee802154AppPkt_m.h
$O/wpan/applications/Ieee802154TestApp/Ieee802154TestApp.o: wpan/applications/Ieee802154TestApp/Ieee802154TestApp.cc \
	applications/trafgen/TrafGenPar.h \
	wpan/applications/Ieee802154TestApp/Ieee802154AppPkt_m.h \
	wpan/applications/Ieee802154TestApp/Ieee802154TestApp.h
$O/wpan/linklayer/bmac/bmacpkt_m.o: wpan/linklayer/bmac/bmacpkt_m.cc \
	wpan/linklayer/bmac/bmacpkt_m.h
$O/wpan/linklayer/bmac/BMacLayer.o: wpan/linklayer/bmac/BMacLayer.cc \
	util/FWMath.h \
	base/IPassiveQueue.h \
	wpan/networklayer/contract/Ieee802154NetworkCtrlInfo_m.h \
	wpan/linklayer/ieee802154/contract/Ieee802154MacPhyPrimitives_m.h \
	linklayer/contract/PhyControlInfo_m.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Def.h \
	base/INETDefs.h \
	base/INotifiable.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Enum.h \
	wpan/linklayer/bmac/BMacLayer.h \
	networklayer/common/InterfaceTableAccess.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	networklayer/common/InterfaceToken.h \
	linklayer/contract/RadioState.h \
	wpan/linklayer/bmac/bmacpkt_m.h \
	linklayer/ieee80211/mac/WirelessMacBase.h \
	networklayer/common/IInterfaceTable.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Const.h \
	networklayer/common/InterfaceEntry.h
$O/wpan/linklayer/ieee802154/contract/Ieee802154MacPhyPrimitives_m.o: wpan/linklayer/ieee802154/contract/Ieee802154MacPhyPrimitives_m.cc \
	wpan/linklayer/ieee802154/contract/Ieee802154MacPhyPrimitives_m.h \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Def.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Enum.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Const.h
$O/wpan/linklayer/ieee802154/mac/Ieee802154Frame_m.o: wpan/linklayer/ieee802154/mac/Ieee802154Frame_m.cc \
	linklayer/contract/PhyControlInfo_m.h \
	base/INETDefs.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Def.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Enum.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Field.h \
	wpan/linklayer/ieee802154/mac/Ieee802154Frame_m.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Const.h
$O/wpan/linklayer/ieee802154/mac/Ieee802154Mac.o: wpan/linklayer/ieee802154/mac/Ieee802154Mac.cc \
	base/IPassiveQueue.h \
	wpan/networklayer/contract/Ieee802154NetworkCtrlInfo_m.h \
	wpan/linklayer/ieee802154/contract/Ieee802154MacPhyPrimitives_m.h \
	linklayer/contract/PhyControlInfo_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Def.h \
	wpan/linklayer/ieee802154/mac/Ieee802154Mac.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Enum.h \
	networklayer/common/InterfaceTableAccess.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	wpan/linklayer/ieee802154/mac/Ieee802154Link.h \
	networklayer/common/InterfaceToken.h \
	linklayer/contract/RadioState.h \
	networklayer/common/IInterfaceTable.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Field.h \
	wpan/linklayer/ieee802154/mac/Ieee802154Frame_m.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Const.h \
	networklayer/common/InterfaceEntry.h
$O/wpan/linklayer/ieee802154/mac/Ieee802154Link.o: wpan/linklayer/ieee802154/mac/Ieee802154Link.cc \
	wpan/linklayer/ieee802154/mac/Ieee802154Link.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Def.h
$O/wpan/linklayer/ieee802154/mac/csma802154.o: wpan/linklayer/ieee802154/mac/csma802154.cc \
	base/IPassiveQueue.h \
	wpan/networklayer/contract/Ieee802154NetworkCtrlInfo_m.h \
	wpan/linklayer/ieee802154/contract/Ieee802154MacPhyPrimitives_m.h \
	linklayer/contract/PhyControlInfo_m.h \
	base/INotifiable.h \
	base/INETDefs.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Def.h \
	wpan/linklayer/ieee802154/mac/Ieee802154Mac.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Enum.h \
	networklayer/common/InterfaceTableAccess.h \
	base/NotifierConsts.h \
	linklayer/contract/MACAddress.h \
	linklayer/contract/Ieee802Ctrl_m.h \
	wpan/linklayer/ieee802154/mac/Ieee802154Link.h \
	networklayer/common/InterfaceToken.h \
	linklayer/contract/RadioState.h \
	wpan/linklayer/ieee802154/mac/csma802154.h \
	networklayer/common/IInterfaceTable.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Field.h \
	wpan/linklayer/ieee802154/mac/Ieee802154Frame_m.h \
	base/ModuleAccess.h \
	base/NotificationBoard.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Const.h \
	networklayer/common/InterfaceEntry.h
$O/wpan/linklayer/ieee802154/phyLayer/Ieee802154RadioModel.o: wpan/linklayer/ieee802154/phyLayer/Ieee802154RadioModel.cc \
	util/FWMath.h \
	linklayer/radio/IRadioModel.h \
	linklayer/radio/SnrList.h \
	base/INETDefs.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Def.h \
	linklayer/radio/ModulationType.h \
	base/Coord.h \
	linklayer/radio/AirFrame_m.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Const.h \
	wpan/linklayer/ieee802154/phyLayer/Ieee802154RadioModel.h
$O/wpan/linklayer/ieee802154/phyLayer/Ieee802154Phy.o: wpan/linklayer/ieee802154/phyLayer/Ieee802154Phy.cc \
	world/powercontrol/IPowerControl.h \
	linklayer/radio/propagation/IReceptionModel.h \
	world/obstacles/Obstacle.h \
	world/radio/ChannelAccess.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Def.h \
	base/INETDefs.h \
	base/INotifiable.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Enum.h \
	linklayer/radio/AirFrame_m.h \
	linklayer/contract/RadioState.h \
	base/NotificationBoard.h \
	base/ModuleAccess.h \
	wpan/linklayer/ieee802154/contract/Ieee802154Const.h \
	util/FWMath.h \
	wpan/linklayer/ieee802154/contract/Ieee802154MacPhyPrimitives_m.h \
	linklayer/radio/SnrList.h \
	linklayer/contract/PhyControlInfo_m.h \
	base/Coord.h \
	base/NotifierConsts.h \
	battery/models/BasicBattery.h \
	world/radio/IChannelControl.h \
	linklayer/radio/IRadioModel.h \
	linklayer/radio/Radio80211aControlInfo_m.h \
	wpan/linklayer/ieee802154/phyLayer/Ieee802154Phy.h \
	world/annotations/AnnotationManager.h \
	linklayer/radio/ModulationType.h \
	world/obstacles/ObstacleControl.h \
	linklayer/radio/Radio.h \
	base/BasicModule.h
$O/wpan/networklayer/Ieee802154StarRouting/Ieee802154StarRouting.o: wpan/networklayer/Ieee802154StarRouting/Ieee802154StarRouting.cc \
	wpan/networklayer/contract/Ieee802154NetworkCtrlInfo_m.h \
	wpan/networklayer/Ieee802154StarRouting/Ieee802154StarRouting.h \
	wpan/applications/Ieee802154TestApp/Ieee802154AppPkt_m.h
$O/wpan/networklayer/contract/Ieee802154NetworkCtrlInfo_m.o: wpan/networklayer/contract/Ieee802154NetworkCtrlInfo_m.cc \
	wpan/networklayer/contract/Ieee802154NetworkCtrlInfo_m.h

