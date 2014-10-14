//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef _INET_ANYMACXMACLAYER_H_
#define _INET_ANYMACXMACLAYER_H_

#include <string>
#include <sstream>
#include <vector>
#include <list>

#include "inet/physicallayer/contract/IRadio.h"
#include "inet/linklayer/contract/IMACProtocol.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/linklayer/common/MACProtocolBase.h"

#include "inet/linklayer/xmac/XMacPkt_m.h"

namespace inet {
using namespace physicallayer;


/**
 * @brief Implementation of Any MAC / X-MAC
 *
 *
 *
 */
class INET_API AnyMacXmacLayer : public MACProtocolBase, public IMACProtocol
{
  private:
	/** @brief Copy constructor is not allowed.
	 */
	AnyMacXmacLayer(const AnyMacXmacLayer&);
	/** @brief Assignment operator is not allowed.
	 */
	AnyMacXmacLayer& operator=(const AnyMacXmacLayer&);

	std::set<MACAddress> pendingPreambles;
	std::set<MACAddress>::iterator pendingPreambleIterator;
	unsigned int preambleCont;

  public:
	AnyMacXmacLayer()
		: MACProtocolBase()
        , preambleCont(0)
		, macQueue()
		, nbTxDataPackets(0), nbTxPreambles(0), nbRxDataPackets(0), nbRxPreambles(0)
		, nbMissedAcks(0), nbRecvdAcks(0), nbDroppedDataPackets(0), nbTxAcks(0)
		, macState(INIT)
		, resend_data(NULL), ack_timeout(NULL), start_xmac(NULL), wakeup(NULL)
		, send_ack(NULL), cca_timeout(NULL), ack_tx_over(NULL), send_preamble(NULL), stop_preambles(NULL)
		, data_tx_over(NULL), data_timeout(NULL)
		, send_ack2(NULL), ack2_tx_over(NULL), cca2_timeout(NULL)
		, lastDataPktSrcAddr()
		, lastDataPktDestAddr()
		, txAttempts(0)
		, nicId(-1)
		, queueLength(0)
		, animation(false)
		, slotDuration(0), bitrate(0), headerLength(0), checkInterval(0), txPower(0)
		, useMacAcks(0)
		, maxTxAttempts(0)
		, stats(false)

	{}
	virtual ~AnyMacXmacLayer();

    /** @brief Initialization of the module and some variables*/
    virtual int numInitStages() const {return NUM_INIT_STAGES;}
    virtual void initialize(int);

    /** @brief Delete all dynamically allocated objects of the module*/
    virtual void finish();

    /** @brief Handle messages from lower layer */
    virtual void handleLowerPacket(cPacket*);

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(cPacket*);

    /** @brief Handle self messages such as timers */
    virtual void handleSelfMessage(cMessage*);

  protected:

    bool xmac;

    int ADDR_SIZE; // bits used by address fild

    typedef std::list<XMacPkt*> MacQueue;

    /** @brief The MAC address of the interface. */
    MACAddress address;

    /** @brief A queue to store packets from upper layer in case another
	packet is still waiting for transmission.*/
    MacQueue macQueue;

    /** @brief The radio. */
    IRadio *radio;
    IRadio::TransmissionState transmissionState;


    /** @name Different tracked statistics.*/
	/*@{*/
	long nbTxDataPackets;
	long nbTxPreambles;
	long nbRxDataPackets;
	long nbRxPreambles;
	long nbMissedAcks;
	long nbRecvdAcks;
	long nbDroppedDataPackets;
	long nbTxAcks;
	/*@}*/

	/** @brief MAC states
	*
	*  The MAC states help to keep track what the MAC is actually
	*  trying to do.
	*  INIT -- node has just started and its status is unclear
	*  SLEEP -- node sleeps, but accepts packets from the network layer
	*  CCA -- Clear Channel Assessment - MAC checks
	*         whether medium is busy
	*  SEND_PREAMBLE -- node sends preambles to wake up all nodes
	*  WAIT_DATA -- node has received at least one preamble from another node
	*  				and wiats for the actual data packet
	*  SEND_DATA -- node has sent enough preambles and sends the actual data
	*  				packet
	*  WAIT_TX_DATA_OVER -- node waits until the data packet sending is ready
	*  WAIT_ACK -- node has sent the data packet and waits for ack from the
	*  			   receiving node
	*  SEND_ACK -- node send an ACK back to the sender
	*  WAIT_ACK_TX -- node waits until the transmission of the ack packet is
	*  				  over
	*/
	enum States {
		INIT,	//0
		SLEEP,	//1
		CCA,	//2
		SEND_PREAMBLE, 	//3
		WAIT_DATA,		//4
		SEND_DATA,		//5
		WAIT_TX_DATA_OVER,	//6
		WAIT_ACK,		//7
		SEND_ACK,		//8
		WAIT_ACK_TX,		//9
		SEND_ACK2,		//10
		WAIT_ACK2_TX,		//11
		SEND_PREAMBLE_TX_END, //12
		WAIT_ACK2, //13
		SEND_DATA2, //14
		CCA2, //15
		CCA_LOST	//16
	  };
	/** @brief The current state of the protocol */
	States macState;
	States prevMacState;

	/** @brief Types of messages (self messages and packets) the node can
	 * process **/
	enum TYPES {
		// packet types
		XMAC_PREAMBLE = 191,
		XMAC_DATA, // 192
		XMAC_ACK, // 193
		// self message types
		XMAC_RESEND_DATA, // 194
		XMAC_ACK_TIMEOUT, // 195
		XMAC_START_XMAC, // 196
		XMAC_WAKE_UP, // 197
		XMAC_SEND_ACK, // 198
		XMAC_CCA_TIMEOUT, // 199
		XMAC_ACK_TX_OVER, // 200
		XMAC_SEND_PREAMBLE, // 201
		XMAC_STOP_PREAMBLES, // 202
		XMAC_DATA_TX_OVER, // 203
		XMAC_DATA_TIMEOUT, // 204
		XMAC_ACK2, // 205
		XMAC_SEND_ACK2, // 206
		XMAC_ACK2_TX_OVER, // 207
		XMAC_PREAMBLE_TX_OVER, // 208
		XMAC_CCA2_TIMEOUT // 209
	};

	// messages used in the FSM
	cMessage *resend_data;
	cMessage *ack_timeout;
	cMessage *start_xmac;
	cMessage *wakeup;
	cMessage *send_ack;
	cMessage *cca_timeout;
	cMessage *ack_tx_over;
	cMessage *send_preamble;
	cMessage *stop_preambles;
	cMessage *data_tx_over;
	cMessage *preamble_tx_over;
	cMessage *data_timeout;
	cMessage *send_ack2;
	cMessage *ack2_tx_over;
	cMessage *cca2_timeout;

	/** @name Help variables for the acknowledgment process. */
	/*@{*/
	MACAddress lastDataPktSrcAddr;
	MACAddress lastDataPktDestAddr;
	MACAddress destAddressNext;
	MACAddress destDestAddrAck2;
	int              txAttempts;
	/*@}*/
	bool   useAck2; // if the preamble has destination address can send ACK2
	bool   PreambleAddress; // Include the destination address in the preamble


	/** @brief publish dropped packets nic wide */
	int nicId;

	/** @brief The maximum length of the queue */
	double queueLength;
	/** @brief Animate (colorize) the nodes.
	 *
	 * The color of the node reflects its basic status (not the exact state!)
	 * BLACK - node is sleeping
	 * GREEN - node is receiving
	 * YELLOW - node is sending
	 */
	bool animation;
	/** @brief The duration of the slot in secs. */
	double slotDuration;
	/** @brief The bitrate of transmission */
	double bitrate;
	/** @brief The length of the MAC header */
	double headerLength;
	/** @brief The duration of CCA */
	double checkInterval;
	/** @brief Transmission power of the node */
	double txPower;
	/** @brief Use MAC level acks or not */
	bool useMacAcks;
	/** @brief Maximum transmission attempts per data packet, when ACKs are
	 * used */
	int maxTxAttempts;
	/** @brief Gather stats at the end of the simulation */
	bool stats;

	/** @brief Possible colors of the node for animation */
	enum XMAC_COLORS {
		GREEN = 1,
		BLUE = 2,
		RED = 3,
		BLACK = 4,
		YELLOW = 5
	};

    /** @brief Generate new interface address*/
    virtual void initializeMACAddress();
    virtual InterfaceEntry *createInterfaceEntry();
    virtual void handleCommand(cMessage *msg) {}

	/** @brief Internal function to change the color of the node */
	void changeDisplayColor(XMAC_COLORS color);

	/** @brief Internal function to send the first packet in the queue */
	void sendDataPacket(const MACAddress & addr);
	void sendDataPacket();

	/** @brief Internal function to send an ACK */
	void sendMacAck();

	/** @brief Internal function to send an ACK */
	void sendMacAck2(const MACAddress & addr);

	/** @brief Internal function to send one preamble */
	void sendPreamble(MACAddress dest = MACAddress::BROADCAST_ADDRESS);
	void sendPreamble(const std::vector<MACAddress> &addr);

	/** @brief Internal function to attach a signal to the packet */
	void attachSignal(XMacPkt *macPkt);

	/** @brief Internal function to add a new packet from upper to the queue */
	bool addToQueue(cMessage * msg);

	virtual void receiveSignal(cComponent *source, simsignal_t signalID, long value);

    virtual XMacPkt *  encapsMsg(cPacket *);
    virtual cPacket* decapsMsg(XMacPkt * );
    virtual cObject* setUpControlInfo(cMessage * const pMsg, const MACAddress&);
};

} // namespace inet

#endif /* BMACLAYER_H_ */
