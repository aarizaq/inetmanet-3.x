//**************************************************************************
// * file:        DMAMAC MAC descriptor file
// *
// * author:      A. Ajith Kumar S.
// * copyright:   (c) A. Ajith Kumar S. 
// * homepage:    www.hib.no/ansatte/aaks
// * email:       aji3003 @ gmail.com
// **************************************************************************
// * part of:     A dual mode adaptive MAC (DMAMAC) protocol for WSAN.
// * Refined on:  25-Apr-2015
// **************************************************************************
// *This file is part of DMAMAC (DMAMAC Protocol Implementation on MiXiM-OMNeT).
// *
// *DMAMAC is free software: you can redistribute it and/or modify
// *it under the terms of the GNU General Public License as published by
// *the Free Software Foundation, either version 3 of the License, or
// *(at your option) any later version.
// *
// *DMAMAC is distributed in the hope that it will be useful,
// *but WITHOUT ANY WARRANTY; without even the implied warranty of
// *MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// *GNU General Public License for more details.
// *
// *You should have received a copy of the GNU General Public License
// *along with DMAMAC.  If not, see <http://www.gnu.org/licenses/>./
// **************************************************************************

package inet.linklayer.dmamac;


import inet.linklayer.base.MACProtocolBase;
import inet.linklayer.contract.IMACProtocol;

simple DMAMAC extends MACProtocolBase like IMACProtocol
{
    parameters:
        string address = default("auto"); // MAC address as hex string (12 hex digits), or
                                          // "auto". "auto" values will be replaced by
                                          // a generated MAC address in init stage 0.
                                          
        int subnetworkId = default(0);
        int addressMaskSize = default(-1); 

        int baseAddress = default(-1);
        int mtu @unit("B") = default(300B);
        
    	 // Enable per module debug
        bool debug = default(false);
        // How long is a slot?
        double slotDuration @unit(s) = default(10ms);
        // Length of MAC header
        int headerLength @unit(bit) = default(80b);
        int queueLength = default(10);
        // Default channel 
        double defaultChannel = default(0);
        // Bit rate 250 kbps (CC2420 value)
        double bitrate @unit(bps) = default(250000bps);
	
		// Ack Timeout
		double ackTimeout @unit(s) = default(0.002s); 
		// Data Timeout 
		double dataTimeout @unit(s) = default(0.002s);
		// Alert Timeout 
		double alertTimeout @unit(s) = default(0.002s);  
		// Stats collection
		bool stats = default(true);
		
        int numSlotsTransient = default(10);
        int numSlotsSteady = default(30);
        double txPower = default(50);
        
        
        // Getting slots
        xml xmlFileSteadyN;
        xml xmlFileTransientN;
        
        // Getting neighbor data
        xml neighborData;
		
		// State change probability (transient to steady);
        double stateProbability = default(5);
        // Alert probability
        double alertProbability = default(1);
        
        // Using new alert message
        bool newAlertMessage = default(false);
        
        // Input to be taken
        int alertDelayMax = default(90);    
        
        // Sink address 
        int sinkAddress=default(0);
                // Sink address 
        int sinkAddressGlobal = default(sinkAddress);
               
        // Radio switch delay from RX->SLEEP or TX->SLEEP, default value is of CC2420
        double maxRadioSwitchDelay @unit(s) = default(0.001792s);
        
        // Is the node an actuator or not.
        bool isActuator = default(false);
        
        // Has Sensor child or not.
        bool hasSensorChild = default(true);
        
        bool NoRetransmission = default(false);
        
        // Initialization parameters
        int maxNodes;
        int maxChildren;
        
        // Mac type Hybrid (TDMA-CSMA as 0) or only TDMA as 1
        int macTypeInput @enum(HYBRID,TDMA) = default(0); 
        
        // New parameters
        string radioModule = default("^.radio");   // The path to the Radio module  //FIXME remove default value
        
        string StartAddressRange = default("0"); // range of acepted address in this node, if 0 accept all
        string EndAddressRange = default("0");
        
        int initialSeed = default(-1);
        bool sendUppperLayer = default(false); // if false the module deletes the packet, other case, it sends the packet to the upper layer.
        bool procUppperLayer = default(true); // if false the module deletes the packet received from upper layers, 
        int initialChannel @enum(11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26) = default(11); // if the random seed is different than -1 it uses this value, in other case it uses the random sequence.
        int reserveChannel = default(-1); // used only if two layer exist. 
        double initTime @unit(s) = default(1s); // this value is used to start the random hop frequency sequency.
        bool frequentHopping = default(false);
        
        
        bool sendDisorganized = default(false);
        bool localActuators = default(true);
        bool disableChecks = default(false);
        
        bool checkDup = default(false); // add 1 byte at the size
        bool useSignalsToChangeChannel = default(false);
        
        @signal[DmaMacChangeChannel];
        @signal[dmamacSendPkt];
        @signal[rcvdPkDma](type=cPacket);        
        @statistic[rcvdPkLifetimeDma](title="received DMA packet lifetime"; source="messageAge(rcvdPkDma)"; unit=s; record=stats,vector; interpolationmode=none);
        
    	@class("DMAMAC");
}