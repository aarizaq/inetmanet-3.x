@author 	: A. Ajith Kumar S.
@homepage 	: http://home.hib.no/ansatte/aaks/
@date 		: April 2015
@copyright  : (c) A. Ajith Kumar S. 				
Part of the protocol are taken from LMAC protocol in MiXiM examples.
@License
======================================================================
This file is part of DMAMAC (DMAMAC Protocol Implementation).

DMAMAC is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

DMAMAC is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with DMAMAC.  If not, see <http://www.gnu.org/licenses/>.
======================================================================
@description: Configuration files 
======================================================================
Used: Common files of both folder 
Config.xml : Contains the required analogue model/pathloss description
NeighborData : Description of routing connections in tree topology.
Nic802154_TI_CC2420_Decider : Obtained from MiXiM-OMNeT (Decider for CC2420)

steadySuperFrameAxTdma.xml (A :2,3,4) : slot schedules for steady superframe
			as 2 times, 3 times and 4 times of transient superframe length
transientSuperframe.xml : slot schedules for transient superframae

Files in TDMA folder cater to TDMA schedule for alert
and Hybrid gives CSMA schedule for alert.

PositionData.ini contains the backup for the planned node positions for
25 nodes configuration
======================================================================