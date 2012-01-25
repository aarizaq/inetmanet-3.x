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


//
// @titlepage
//
// <center><h1>The OverSim Overlay Framwork</h1></center>
//
// <nohtml>OverSim is a flexible overlay network simulation framework
// based on OMNeT++. The framework includes several
// structured and unstructured peer-to-peer protocols like
// Chord, Pastry, Bamboo, Koorde, Broose, Gia, Kademlia, Pastry and Vast.
// These protocol implementations can be used for both simulation as well
// as real world networks. To facilitate the implementation of
// additional protocols and to make them more comparable
// OverSim provides several common functions like a generic
// lookup mechanism for structured peer-to-peer networks and an
// RPC interface. Several exchangeable underlay network models
// allow to simulate complex heterogeneous underlay networks as
// well as simplified networks for large-scale simulations. With
// OverSim simulations of overlay networks with up to 100,000
// nodes are feasible.</nohtml>
//
// <a href="history.html">History and contributors</a>.
//
// For up-to-date info, visit the current
// <a href="http://www.oversim.org/" target="_top">OverSim home page</a>.
//
// A few useful starting points for browsing the documentation:
//  - the <a href="full-ned-usage-diagram.html">Usage Diagram</a>
//    includes all modules and networks
//  - base classes: BaseOverlay, BaseApp, BaseRpc
//  - overlay protocols: Chord, Gia, Broose, Koorde, Pastry, Kademlia, Bamboo,<br>
//    Nice, Vast, NTree, Quon and PubSubMMOG
//  - tier1 applications: KBRTestApp, DHT, GIASearchApp, RealWorldTestApp<br>
//    and SimpleGameClient
//  - tier2 applications: DHTTestApp, P2pns
//  - tier3 applications: XmlRpcInterface
//  - underlay models: SimpleUnderlayNetwork, InetUnderlayNetwork, InetUnderlayNetwork6, ReaSEUnderlayNetwork<br>
//    and SingleHostUnderlayNetwork
//  - hosts: SimpleOverlayHost, InetOverlayHost, AccessRouter, OverlayAccessRouter, OverlayRouter etc.
//  - network channels: simple_dsl, simple_ethernetline, simple_dsl_lossy, simple_ethernetline_lossy,<br>
//    inet_dsl, inet_ethernetline, inet_dsl_lossy, inet_ethernetline_lossy, inet_fiberline
//  - global modules: GlobalObserver, GlobalNodeList
//  - message classes: BaseOverlayMessage, CommonAPIMessage, ExtAPIMessages,<br>
//    KBRTestMessage, ChordMessage, GiaMessage, PastryMessage, SimpleUDPPacket, ...
//  - control info classes (used in communication between protocol layers):<br>
//    OverlayCtrlInfo
//  - churn generators: LifetimeChurn, ParetoChurn, RandomChurn, NoChurn 
//
// Related documentation:
//   - <a href="../doxy/main.html" target="_top">C++ source documentation (Doxygen)</a>
//
// @page history.html, History
//
// The OverSim framework was written by Ingmar Baumgart, Bernhard Heep,
// Stephan Krause and others at the Institut of Telematics,
// Karlsruhe Institute of Technology (KIT) in 2006-2010.
//
// For changes please read the <a href="../ChangeLog">ChangeLog</a>
//

@license(GPL);
package inet.oversim;

