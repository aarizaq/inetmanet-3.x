+----------------------+
|       README         |
+----------------------+

== Introduction ==

For the needs of research, multiple routing schemes might be required to be
examined. To simplify this whole process, a common base system was introduced,
the Socially-Aware Opportunistic Routing System (SAORS). It provides a
simple platform, based on the OMNET++ INETMANET framework, for developing and
simulating opportunistic routing protocols. SAORS is based on the simple idea
that ad-hoc and opportunistic networking can be combined to provide a reliable
solution against disruptions. Additionally, the fact that opportunistic routing 
protocols are able to operate using the same or similar data structures and 
functions as the ad-hoc protocols, greatly simplifies their implementation 
complexity.

Up to now, OMNET++ and the INET and INETMANET frameworks do not include
an opportunistic routing protocol implementation. A careful examination 
reveals that there are three very distinctive stages in the decision procedure 
of every opportunistic routing protocol. SAORS takes advantage of this 
observation and incorporates it in the system architecture. It is based on the 
DYMO ad-hoc routing protocol. SAORS provides the necessary functions for the 
development of opportunistic schemes such as access to the node's routing table, 
a database for storing the Delay-Tolerant messages, as well as the ad-hoc 
functionality accomplished by DYMO. It is using by default a beacon mechanism 
to perform the social network discovery. Beacons contain the ID of the 
transmitting node and its most important social information (as defined by the 
routing protocol).


== Installation ==

Before trying to install SAORS, you first have to have OMNET++ simulation 
environment and INETMANET-2.0 framework already up and running in your system. 
For more information on that you can check:

for OMNET++			---	http://www.omnetpp.org/documentation
for INETMANET-2.0	---	https://github.com/aarizaq/inetmanet-2.0

After you have completed all the above steps, you can use download the
compressed SAORS. In order to import the simulator into OMNET++  you just have
to follow a simple procedure. First you go to File->Import  and choose
General->Archive File. After that you just press Next and  that it. You have
made it! The only thing left it to build the download  project. To do this, you
need to go to Run->RunConfiguration... and  create a new configuration by double
clicking on the OMNET++ Simulation  element on the left side list. There is no
simulations folder, so you will have to create your a new network to test a
SAORS module. Feel free to play around!

One last thing is the relationship between the SAORS Framework and the
DYMO implementation in DYMOFAU of INETMANET-2.0. The directory location is 
this:

inetmanet/src/networklayer/manetrouting/dymo_fau

There you neet to change in file DYMO.h lines:

	private:
		  friend class DYMO_RoutingTable;

into:

	protected:
		  friend class DYMO_RoutingTable;

This way you allow inheritance of the DYMO class structure at all extention
classes, such as SAORSBase.


== Architecture ==

SAORS takes into account the independent stages of the opportunistic routing
procedure. Therefore, it allows the development of a new opportunistic protocol
only by adding the missing pieces in the code of the base system. There define
the behaviour of the scheme in the following procedure:

1) How to detect neighbouring nodes (mainly using beacons)
2) How to compute the routing metrics necessary
3) How to choose the best carrier according to the computed metrics

There can be filled following the hooks of the SAORSBase class. These are the
following functions:

sendBeacon()
handleBeacon(SAORS_BEACON* my_beacon)
findEncounterProb(const SAORSBase_RoutingEntry* routeToNode)
compareEncounterProb(const SAORSBase_RoutingEntry* dtEntry, const SAORS_RREP* rrep)
sendEncounterProb(SAORS_RREQ* rreq, const SAORSBase_RoutingEntry* dtEntry)

The above functions determine the behaviour of the routing algorithm when it
send or receives beacons, so what information to put in and  how to extract it.
The "findEncounterProb" determines the metric that the node will include in its
RREQ packets and RREP, to be used  for the opportunistic operation, if the
destination is not found.  The "sendEncounterProb" determines whether the node
will reply in an in an incoming RREQ as an intermediate node, if it does not 
have a contemporary contact with the destination. Finally, the
"compareEncounterProb" compares the RREQ requested contact probability with the
one of the received RREP and determines if the nodes will accept the RREP
transmitting node as an intermediate router. The final three functions might
seem similar but were left like that for implementation scalability. 

SAORS deals with the Delay-Routing table, which is different that the one used
by DYMO and stores the received opportunistic messages in its database. So, each
scheme can use the specified functions to determine the choices of the nodes
only, rather than the whole  functionality of the opportunistic protocol. 
Several solutions have already been implemented, but as they as currently under
development, should be used with extra care! There are:

1) PROPHET, included in a DT-DYMO implementation.
2) Epidemic routing.
3) A Random choice opportunistic protocol.
4) A SimBetTS-like routing protocol.
5) A multi-phase scheme called SAMPhO.

Any extra developments and help are always welcome. For more information,
please contact me.  


== Notes ==

Tested only for omnetpp-4.6 with inetmanet-2.2.

For any comments email me at nvasta@essex.ac.uk

Nikolaos Vastardis

