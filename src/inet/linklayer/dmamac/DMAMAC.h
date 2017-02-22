//**************************************************************************
// * file:        DMAMAC header file
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

#ifndef __DMAMAC_H__
#define __DMAMAC_H__

#include <omnetpp.h>

#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/linklayer/contract/IMACProtocol.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/base/MACProtocolBase.h"
#include "inet/linklayer/bmac/BMacFrame_m.h"

#include "inet/linklayer/dmamac/packets/DMAMACPkt_m.h"        // MAC Data/ACK packet
#include "inet/linklayer/dmamac/packets/AlertPkt_m.h"         // Short Alert packet
#include "inet/linklayer/dmamac/packets/DMAMACSinkPkt_m.h"    // Notification packet from sink
#include "inet/power/contract/IEnergyStorage.h"
#include <map>                  // For neighbor recognition from xmls

/* @brief XML files are used to store slot structure for easy input differentiation from execution files */

/* @brief
 * A dual mode adaptive MAC (DMAMAC) protocol for WSAN is a Medium Access Control protocol for Wireless Sensor 
 * Actuator Netorks. It is designed to cater for Process control applications that have two states of operation:
 * transient and steady. Thus, DMAMAC has two modes of MAC operation transient mode and steady mode.
 * Here we describe how it handles different slots including notification, data, acks and alerts. Alert is used
 * indicate a switch of operational modes on application requirements. DMAMAC has two versions, DMAMAC-Hybrid and
 * DMAMAC-TDMA and both are described in DMAMAC.cc. The only difference between these two versions are that
 * DMAMAC-Hybrid is a hybrid with CSMA used in alert slots and DMAMAC-TDMA is fully TDMA with TDMA used in alert slots.
 * Point to note collision in the implementation context is packet dropped during unability to read the bits (not real).
 * In the DMAMAC-Hybrid context collision can happen at alert slots, thus we specify it as collision.
 * Check "Towards a Dual-Mode Adaptive MAC Protocol (DMAMAC) for Feedback-based Networked Control Systems"
 * in Procedia Computer Science, FNC 2014 for first version information on DMAMAC.
 * For information on DMAMAC-Hybrid and DMAMAC-TDMA check the upcoming publication:
 * "Simulation-based Evaluation of DMAMAC - A Dual-Mode Adaptive MAC Protocol for Process Control", in SIMUTOOLS
 * 2015 (To be published August).
 */
namespace inet {

using namespace physicallayer;
using namespace power;

#define macpkt_ptr_t DMAMACPkt*

class INET_API DMAMAC : public MACProtocolBase, public IMACProtocol
{
    /* @brief Parts copied from LMAC definition.  */

    private:
        /* @brief Copy constructor is not allowed.*/
        DMAMAC(const DMAMAC&);

        /* @brief Assignment operator is not allowed.*/
        DMAMAC& operator=(const DMAMAC&);

    /* @brief Initializing variables. */

	/* @brief DMAMAC inherits from the BaseMacLayer thus needs to be initialized */
    public:

       bool shutDown = false;

       DMAMAC():MACProtocolBase()
        , macPktQueue()
        , slotDuration(0)
        , mySlot(0)
        , numSlots(0)
        , numSlotsTransient(0)
        , numSlotsSteady(0)
        , currentSlot()
        , transmitSlot()
        , receiveSlot()
        , bitrate(0)
        , txPower(0)
        , nextEvent()
        , parent()
    {globalLocatorTable.clear(); sinkToClientAddress.clear();  clientToSinkAddress.clear();}

    /* @brief Signal for change in Hoststate mainly used for battery depletion or death of node */
    // const static simsignal_t catHostStateSignal;

    /* @brief Module destructor DMAMAC */
    virtual ~DMAMAC();

    /* @brief MAC initialization DMAMAC and BaseMacLayer */
    virtual void initialize(int stage) override;

    /* @brief Deletes all dynamically allocated objects */
    virtual void finish() override;

    /* @brief Handles packets received from the Physical Layer */
    virtual void handleLowerPacket(cPacket* msg) override;

    /* @brief
     * Handles packets from the upper layer (Network/Application) and prepares
     * them to be sent down. 
     */
    virtual void handleUpperPacket(cPacket* msg) override;

    /* @brief Handle self messages such as timer, to move through slots in DMAMAC  */
    virtual void handleSelfMessage(cMessage*) override;

    /* @brief
     * Handles and forwards the control messages received
     * from the physical layer.
     */
    /** @brief Handle control messages from lower layer */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long value, cObject *details) override;

    // handle shutdown and crash events.

    virtual bool handleNodeShutdown(IDoneCallback *doneCallback) override;

    virtual void handleNodeCrash() override;

    /* @brief
     * Encapsulates the packet from the upper layer and
     * creates and attaches the signal to it.
     */
    virtual MACFrameBase* encapsMsg(cPacket* Pkt);

    /* @brief
     * Called after the radio is switched to TX mode. Encapsulates
     * and sends down the packet to the physical layer.
     */
    virtual void handleRadioSwitchedToTX();

    /* @brief
     * Called when the radio is switched to RX.
     * Sets the MAC state to RX. 
     */
    virtual void handleRadioSwitchedToRX();

    virtual void initializeMACAddress();

    virtual InterfaceEntry *createInterfaceEntry() override;

    virtual cPacket *decapsMsg(MACFrameBase *macPkt);
    cObject *setUpControlInfo(cMessage *const pMsg, const MACAddress& pSrcAddr);

    virtual void discoverIfNodeIsRelay();

    virtual void initializeRandomSeq();

protected:

    int baseAddress = 0;
    /** @brief The radio. */
      IRadio *radio = nullptr;
      IRadio::TransmissionState transmissionState = IRadio::TRANSMISSION_STATE_UNDEFINED;

    /* @brief My listener to listen to the signal emitted for hostState change,
     * mainly to track battery depletion 
	 */
    cIListener *myListener = nullptr;

    /* @brief Defining at type for MAC packet buffer 
     * Mac packet queue to store incoming packets from upper layer  
	 */
    typedef std::list<DMAMACPkt*> MacPktQueue;
	
	MacPktQueue macPktQueue;
	
    /* @brief length of the queue */
    unsigned queueLength = 0;

    /* @brief Defining bool variable to indicate alertMessage to be forwarded */
    /*@{*/
    bool alertMessageFromDown = false;
    /*@}*/

    /* @brief MAC states
     *  The MAC states help to keep track which operation state MAC is in
     *  This is especially useful when radio switching takes some time.
     *  STARTUP    -- Starting phase, if setup procedure is present
     *  MAC_SLEEP      -- Node radio sleeps but mac accepts packets from the network layer
     *  WAIT_DATA  -- Node switches radio to receive mode and is waiting to accept DATA packet
     *  WAIT_ACK   -- DATA packet has been sent and then node switches radio to receive mode
     *                and is waiting to accept ACK packet
	 *  WAIT_ALERT -- Node is waiting to accept alert packets (Radio switched to RX)
	 *  WAIT_NOTIFICATION -- Node Awaiting notification packets from the sink (Radio switched to RX)
     *  SEND_DATA  -- Node has send slot and checks whether it has packets to send and then
     *                switches radio to transmission mode if required
     *  SEND_ACK   -- Node has received a DATA packet and then prepares to send ACK packet
     *  SEND_ALERT -- Node detects a state switch (steady to transient) in the process and needs to alert
     *                the sink about the detection
	 *  SEND_NOTIFICATION -- state used by sink only to send notification message (Radio switched to TX)
	 *  SCHEDULE_ALERT -- For the state between WAIT_ALERT and SEND_ALERT so we do not recieve alert packets
     *					This is an intermediate state before alert will be sent (if any).	
     */

    enum macTypes{
            HYBRID = 0,             // Has both CSMA and TDMA
            TDMA   = 1,             // TDMA only even for alert.
        };

    enum macPeriods{
            DATA,               // MAC is in the data communication period (transient or steady)
            ALERT,              // MAC is in the alert period (steady only)
        };

    enum macStates{
            STARTUP,            // Setup phase
            MAC_SLEEP,          // Sleeping
            WAIT_DATA,          // Waiting for DATA
            WAIT_ACK,           // Waiting for ACK
            WAIT_ALERT,         // Wait for Alert data (parent nodes only)
            WAIT_NOTIFICATION,  // Wait for notification message (all nodes except sink)
            SEND_DATA,          // Sending DATA
            SEND_ACK,           // Sending ACK
            SEND_ALERT,         // Sending Alert message (Any node except sink)
            SEND_NOTIFICATION,  // Send notification message (sink only)
            SCHEDULE_ALERT,     // For the state between WAIT_ALERT and SEND_ALERT so we do not recieve alert packets
    };

    /*
     * The DMAMAC protocol is dual mode protocol with different operational
     * modes for different process states (steady and transient)
     */
    enum macMode {
            STEADY,
            TRANSIENT,
    };

    /* @brief Transmission modes  !new */
    enum slotTypes {
            SEND = 100,              // Not used yet used nodeId for send and receive
            RECEIVE = 101,           // Not used yet
            SLEEP = 102,
            BROADCAST_RECEIVE = 103, // For sink to transmit and all others receive.
            NOTIFICATION = 104,      // Notification slots send by sink
            ALERT_SINK = 105,        // Sink only receives and has this number for alert receiving.
            ALERT_LEVEL1 = 106,      // Alert for the level adjacent to sink
            ALERT_LEVEL2 = 107,      // Alert for the next level 
            ALERT_LEAF = 108,		 // Alert for the leaf nodes
    };


    /* @brief Self Message types, used as timer messages corresponding to MAC states except for timeouts and ACK_RECEIVED  */
    enum selfMessagetypes {
            DMAMAC_STARTUP,
            DMAMAC_SLEEP,
            DMAMAC_WAIT_DATA,
            DMAMAC_WAIT_ACK,
            DMAMAC_WAIT_ALERT,          // Only intermediate (parent) nodes wait for alert messages 
            DMAMAC_WAIT_NOTIFICATION,   // Nodes wait for notificaition message from sink
            DMAMAC_SEND_DATA,
            DMAMAC_SEND_ACK,
            DMAMAC_SCHEDULE_ALERT,      // Intermediate state to schedule an alert with a random delay. 
            DMAMAC_SEND_ALERT,          // Any node can send an alert message if they dont have an alert message they continue radio sleep 
            DMAMAC_SEND_NOTIFICATION,   // Sink sends notification message
            DMAMAC_ACK_RECEIVED,
            DMAMAC_ACK_TIMEOUT,			// Maximum time allocated to wait for ACK.
            DMAMAC_DATA_TIMEOUT,        // Not staying in RX for the entire slot if no data transmission is detected.
            DMAMAC_ALERT_TIMEOUT,       // For alert timeout in DMAMAC-TMDA for Hybrid not time-outs.
            DMAMAC_HOPPING_TIMEOUT,
    };


    /* @brief Packet types  */
    enum packetTypes {
            DMAMAC_DATA,            // marking data packet
            DMAMAC_ACTUATOR_DATA,   // marking actuator data
            DMAMAC_ACK,             // marking ACK packet
            DMAMAC_ALERT,           // marking Alert packet
            DMAMAC_NOTIFICATION,    // marking Notification packet from sink
    };

    /* @brief Mac mode (Steady,transient) and Mac states defined above */
    /*@{*/
    macStates currentMacState = STARTUP;
    macMode previousMacMode;
    macMode currentMacMode;
    macTypes macType;				// Specification of either DMAMAC-TDMA or DMAMAC-Hybrid
    macPeriods macPeriod;			// Period of superframe, DATA, ALERT and SLEEP
    bool changeMacMode;				// Indicates change of MAC mode initiated or not
    bool sendAlertMessage;          // To set sending of AlertMessage after delay 
    bool checkForSuperframeChange;  // For checking when alert packets collide and result in no superframe change
	
    struct nodeStatus {
        macStates currentMacState;
        macMode previousMacMode;
        macMode currentMacMode;
        int currentSlot;
    };
    static std::map<uint64_t,nodeStatus> slotMap; // debug


    /* @brief To declare if the node is an actuator, inputtaken from ini file */
    bool isActuator;

    /* @brief To decide if the node should wait for alert in its alert slot or not */
    bool hasSensorChild;

    /* @brief To set when received packet is destined for child node */
    bool forChildNode;
    /*@}*/


    /* @brief All variables involing slots. And also
     * Slot indicator arrays, one main array, two specific arrays for states steady and transient */
    /*@{*/
    simtime_t slotDuration;
    simtime_t controlDuration;         // Unused currently, used for calculating time required for control operations
    simtime_t maxRadioSwitchDelay;     // Radio switching delay between radio states (Not to interfere in between)

    int mySlot;                     // The slot in which the node can transmit
    int maxNumSlots;                // Maximum number of slots (specifically in steady superframe
    int numSlots;                   // Number of slots in the current superframe
    int numSlotsTransient;          // Number of slots in the transient superframe
    int numSlotsSteady;             // Number of slots in the steady superframe
    int currentSlot;
    typedef std::vector<int> Slots;
    Slots transmitSlot;			// 150 normal slots with configuration 1x, 2x, 3x and 4x we have upto 600 slots
    Slots receiveSlot;
    Slots transmitSlotSteady;
    Slots receiveSlotSteady;
    Slots transmitSlotTransient;
    Slots receiveSlotTransient;
    int randomNumber;               // To generate random number
    int maxNodes;                   // To know how much nodes in the network required for initializing slots from XML
    int maxChildren;
    /*@}*/

    // variables used to frequent hopping
    simtime_t timeRef; // reference time synchronized with the sink
    bool isSincronized = false; // If the node is synchronized
    bool frequentHopping = false;
    int initialSeed;

    cMessage *hoppingTimer = nullptr;

    /* @brief Self messages for TDMA slot timer callbacks */
    /*@{*/
    cMessage* startup;
    cMessage* setSleep;
    cMessage* waitData;
    cMessage* waitAck;
    cMessage* waitAlert;
    cMessage* waitNotification;
    cMessage* sendData;
    cMessage* sendAck;
    cMessage* scheduleAlert;      
    cMessage* sendAlert;
    cMessage* sendNotification;
    cMessage* ackReceived;
    cMessage* ackTimeout;
    cMessage* dataTimeout;      
    cMessage* alertTimeout;     
    /*@}*/

    /* @name Help variables for the acknowledgment process. From #BMAC */
    /*@{*/
    MACAddress lastDataPktSrcAddr;
    MACAddress lastDataPktDestAddr;
    MACAddress destAddr;
    MACAddress myMacAddr;
    int txAttempts;
    /*@}*/

    /* @brief Other important variables */
    /*@{*/
    double bitrate;
    /** @brief Length of the header*/
    int headerLength;

    // DroppedPacket droppedPacket;
    double txPower;             //Transmission power of the node
    simtime_t nextEvent;           // Time to next event to be used in event scheduler calls
    double ackTimeoutValue;     // Time to wait for ACK
    double dataTimeoutValue;    // Time to wait for data before going to sleep !aaks Nov 27
    double alertTimeoutValue;         // Time to wait for alert in the TDMA setting.
    double nextSlot;            // Calculating time remaining to next slot
    int alertDelayMax;          // Input to be taken #aaks 29 Apr
    /*@}*/

    /* @brief Statistics part and bool to set tracking on or off from config file */
    /*@{*/
    bool stats;
    long nbTxData;
    long nbTxActuatorData;
    long nbTxDataFailures;      // No lossy links currently so not useful as such 
    long nbTxAcks;
    long nbTxAlert;
    long nbTxSlots;
    long nbTxNotifications;
    long nbRxData;
    long nbRxActuatorData;
    long nbRxAcks;
    long nbRxAlert;
    long nbRxNotifications;
    //long nbMissedAcks;        // No lossy links
    long nbSleepSlots;
    long nbCollisions;
    long nbDroppedDataPackets;  // No dropping scenario designed yet.
    long nbTransient;
    long nbSteady;
    long nbSteadyToTransient;
    long nbTransientToSteady;
    long nbFailedSwitch;        // Counts switches failed due to collision (or dropping of packets based on error)
    long nbMidSwitch;           // Counts switches that happen between steady state
    long nbSkippedAlert;        // Counts alert skipped because of carrier sense
    long nbForwardedAlert;
    long nbDiscardedAlerts;
    long nbTimeouts;            // number of timeouts recorded
    long nbAlertRxSlots;
    /*@}*/

    /* @brief Parameters and lists to read the input xml file */
    /*@{*/
    cXMLElement *xmlFileSteady;   					// Contains slot details for steady superframe 
    cXMLElement *xmlFileTransient;					// Contains slot details for transient superframe 
    cXMLElement *xmlBuffer;	
    cXMLElement *xmlNeighborData;					// Contains routing information
    cXMLElementList nListBuffer;					// Extra buffers and variables reauired
    cXMLElementList::iterator xmlListIterator;	
    /*@}*/

    /* @brief Probability of transient frame appearing next, taken from configuration ini file */
    int stateProbability;
    /* @brief Probability of nodes to generate alert*/
    int alertProbability;

    /* @brief Tree topology routing stuff */
    /*@{*/
    long parent;                    // For upstream data   (sensorData, Alert)
    MACAddress sinkAddressGlobal;
    MACAddress sinkAddress;
    static bool twoLevels;
    bool isSink = false;
    bool isRelayNode = false;
    int reserveChannel = -1;
    int alertLevel;

    typedef std::map<MACAddress,MACAddress> LocatorTable;
    LocatorTable locatorTable;
    // to simplify the code will use global variables

    static LocatorTable globalLocatorTable; // this table allow to know what node is connected with which sink.
    // relay node functions, used to know the two address of the relay nodes, the client-sink and sink-client correspondence.
    static LocatorTable sinkToClientAddress; //
    static LocatorTable clientToSinkAddress;

    typedef struct routeTable{
        int nextHop;
        int reachableAddress[10];   // Maximum nodes reachable equal to number of nodes in the network
    }routeTable;

    routeTable downStream[3];       // For upstream data we always send it to the parent, 3 is maxChildren possible for all nodes
    /*@}*/

    /* Protected methods */

    /* @brief Finds the distant next slot after the current slot (say after going to sleep) */
    virtual void findDistantNextSlot();

    /* @brief Finds the immediate next slot after the current slot */
    virtual void findImmediateNextSlot(int currentSlotLocal,simtime_t nextSlot);

    /* @brief Internal function to attach a signal to the packet (essential function) */
    virtual void attachSignal(MACFrameBase* macPkt);

    /* @brief To change main superframe to the new mac mode superframe */
    virtual void changeSuperFrame(macMode mode);

    /* @brief Defining the static slot schedule from xml files which will be inherited by DMAMACSink without changes*/
    void slotInitialize();
    // channels
protected:

    //
    bool sendUppperLayer = false; // if false the module deletes the packet, other case, it sends the packet to the upper layer.

    int actualChannel = -1;

    struct channelsInfo {
        double bandwith;
        double mean;
    };
    std::vector<channelsInfo> channels = {{2e6,2.405e9},{2e6,2.410e9},{2e6,2.415e9},{2e6,2.420e9},{2e6,2.425e9},
                                          {2e6,2.430e9},{2e6,2.435e9},{2e6,2.440e9},{2e6,2.445e9},{2e6,2.450e9},
                                          {2e6,2.455e9},{2e6,2.460e9},{2e6,2.465e9},{2e6,2.470e9},{2e6,2.475e9},{2e6,2.480e9}};

    class CRandomMother {                  // Encapsulate random number generator
    public:
       void randomInit(int seed);          // Initialization
       virtual int iRandom(int min, int max);      // Get integer random number in desired interval
       double random();                    // Get floating point random number
       uint32_t bRandom();                 // Output random bits
       uint32_t bRandom(const uint32_t *v);                 // Output random bits
       CRandomMother(int seed) {           // Constructor
          randomInit(seed);}
       void getRandSeq(uint32_t *v);
       void setRandSeq(const uint32_t *v);
       void setRandSeq(const uint64_t &v);
       void getPrevSeq(uint32_t *v);
       virtual int iRandom(int min, int max, const uint32_t *v);      // Get integer random number in desired interval
    protected:
       uint32_t x[5];                      // History buffer
       uint32_t Prevx[5];                      // History buffer
    };

    CRandomMother *randomGenerator = nullptr;
public:
    virtual void refreshDisplay();
    virtual void setChannel(const int &channel);
    virtual int getChannel(){return (actualChannel);}
    virtual double getCarrierChannel(const int &channel);
    virtual double getBandwithChannel(const int &channel);
    virtual void setNextSequenceChannel();
    virtual void setChannelWithSeq(const uint32_t *v);
    virtual void setRandSeq(const uint64_t &v);
};

}
#endif
