//==========================================================================
//  INDEX.H - part of
//                          OverSim
//            A Flexible Overlay Simulation Framework
//
//  Defines modules for documentation generation. Nothing useful for the
//  C++ compiler.
//
//==========================================================================

/**
 * @mainpage OverSim API Reference
 *
 * Pick one of the entries in the following categories, or choose from the
 * links at the top of this page:<br>
 *
 * <b>Underlay</b> - underlying network modelling classes<br>
 *   - Underlay configurator base class (UnderlayConfigurator)<br>
 *     - for SimpleUnderlay (SimpleUnderlayConfigurator)<br>
 *     - for InetUnderlay (InetUnderlayConfigurator)<br>
 *     - for ReaSEUnderlay (ReaSEUnderlayConfigurator)<br>
 *     - for SingleHostUnderlay (SingleHostUnderlayConfigurator)<br>
 *
 * <b>Overlay</b> - overlay classes and message types<br>
 * - Base class for overlay modules on Tier 0 / KBR (BaseOverlay / BaseRpc)<br>
 *   - structured overlay oversim::Chord
 *   - structured overlay oversim::Koorde
 *   - structured overlay Broose
 *   - structured overlay Pastry
 *   - structured overlay Kademlia
 *   - structured overlay Bamboo
 *   - unstructured overlay Gia
 *   - application layer multicast oversim::Nice
 *   - gaming overlay PubSubMMOG
 *   - gaming overlay Vast
 *   - gaming overlay NTree
 *   - gaming overlay Quon
 * - Base class for lookups in the overlay (IterativeLookup)<br>
 *
 * <b>Application</b> - application classes and message types<br>
 * - Base class for Applications on Tier 1 (BaseApp / BaseRpc)<br>
 *   - Application for testing Key-based routing (KBRTestApp)<br>
 *   - Application for testing Gia searching (GIASearchApp)<br>
 *   - A distributed hash table for data storage (DHT)<br>
 *   - A real world test application (RealWorldTestApp)<br>
 *   - A simple MMOG client (SimpleGameClient)
 * - Base class for Applications on Tier 2 (BaseApp)<br>
 *   - Application for testing a DHT (DHTTestApp)<br>
 *   - A distributed name service for P2PSIP (P2pns)<br>
 * - Base class for Applications on Tier 3 (BaseApp)<br>
 *   - XML-RPC interface to connect external application to OverSim (XmlRpcInterface)<br>
 *
 * <b>Churn generators</b> - different churn generation models<br>
 *   - LifetimeChurn, ParetoChurn, RandomChurn, NoChurn<br>
 *
 * <b>Utility classes</b> - data structures and helper classes<br>
 *   - OverlayKey, TransportAddress, NodeHandle, BinaryValue<br>
 *
 * If you want to use, copy, change or distribute OverSim read the @ref GPL
 */

/**
 * @page GPL
 *
 * @verbinclude License
 */
