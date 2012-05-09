//******************************
//LQI measurement not implemented
//

#ifndef IEEE_802154_PHY_H
#define IEEE_802154_PHY_H

#include "Radio.h"
#include "RadioState.h"
#include "Ieee802154Const.h"
#include "Ieee802154Def.h"
#include "Ieee802154Enum.h"
#include "Ieee802154MacPhyPrimitives_m.h"
#include "AirFrame_m.h"
#include "IRadioModel.h"
#include "IReceptionModel.h"
#include "FWMath.h"
#include "NotificationBoard.h"
#include "NotifierConsts.h"
#include "ObstacleControl.h"


class INET_API Ieee802154Phy : public ChannelAccess, public IPowerControl
{
    public:
        static uint16_t aMaxPHYPacketSize; //max PSDU size (in bytes) the PHY shall be able to receive
        static uint16_t aTurnaroundTime; //Rx-to-Tx or Tx-to-Rx max turnaround time (in symbol period)
        static uint16_t aMaxBeaconOverhead; //max # of octets added by the MAC sublayer to the payload of its beacon frame
        static uint16_t aMaxBeaconPayloadLength;
        static uint16_t aMaxMACFrameSize;
        static uint16_t aMaxFrameOverhead;

    public:
        Ieee802154Phy();
        virtual ~Ieee802154Phy();

    protected:
        virtual void initialize(int);
        virtual int numInitStages() const { return 3; }
        virtual void finish();

    // message handle functions
        virtual void handleMessage(cMessage*);
        virtual void handleUpperMsg(AirFrame*);
        virtual void handleSelfMsg(cMessage*);
        virtual void handlePrimitive(int, cMessage*);
        virtual void handleLowerMsgStart(AirFrame*);
        virtual void handleLowerMsgEnd(AirFrame*);
        virtual AirFrame* encapsulatePacket(cMessage*);
        virtual void bufferMsg(AirFrame*);
        virtual AirFrame* unbufferMsg(cMessage*);
        virtual void sendUp(cMessage*);
        virtual void sendDown(AirFrame*);

        virtual IReceptionModel *createReceptionModel() { return (IReceptionModel *) createOne(par("attenuationModel").stringValue()); }
        virtual IRadioModel *createRadioModel() { return (IRadioModel *) createOne(par("radioModel").stringValue()); }

        // primitives processing functions
        virtual void PD_DATA_confirm(PHYenum status);
        virtual void PD_DATA_confirm(PHYenum status, short);
        virtual void PLME_CCA_confirm(PHYenum status);
        virtual void PLME_ED_confirm(PHYenum status, uint16_t energyLevel);
        virtual void handle_PLME_SET_TRX_STATE_request(PHYenum setState);
        virtual void PLME_SET_TRX_STATE_confirm(PHYenum status);
        virtual void handle_PLME_SET_request(Ieee802154MacPhyPrimitives *primitive);
        virtual void PLME_SET_confirm(PHYenum status, PHYPIBenum attribute);
        virtual void PLME_bitRate(double bitRate);

        virtual void setRadioState(RadioState::State newState);
        virtual int getChannelNumber() const { return rs.getChannelNumber(); }
        virtual void addNewSnr();
        virtual void changeChannel(int newChannel);
        virtual bool channelSupported(int channel);
        virtual uint16_t calculateEnergyLevel();
        virtual double getRate(char dataOrSymbol);
        virtual bool processAirFrame(AirFrame*);


        virtual void disconnectTransceiver() {transceiverConnect = false;}
        virtual void connectTransceiver() {transceiverConnect = true;}
        virtual void disconnectReceiver();
        virtual void connectReceiver();


  protected:
        bool transceiverConnect;
        bool receiverConnect;
        bool m_debug; // debug switch
        IRadioModel* radioModel;
        IReceptionModel* receptionModel;
        ObstacleControl* obstacles;

        int upperLayerOut;
        int upperLayerIn;

        double transmitterPower; // in mW
        double noiseLevel;
        double carrierFrequency;
        double sensitivity; // in mW
        double thermalNoise;

        struct SnrStruct
        {
                AirFrame* ptr; ///< pointer to the message this information belongs to
                double rcvdPower; ///< received power of the message
                SnrList sList; ///< stores SNR over time
        };
        SnrStruct snrInfo; // stores the snrList and the the recvdPower for the
        // message currently being received, together with a pointer to the message.
        struct Compare {
            bool operator() (AirFrame* const &lhs, AirFrame* const &rhs) const {
                ASSERT(lhs && rhs);
                return lhs->getId() < rhs->getId();
            }
        };
        typedef std::map<AirFrame*, double, Compare> RecvBuff;
        RecvBuff recvBuff; // A buffer to store a pointer to a message and the related receive power.

        AirFrame* txPktCopy; // duplicated outgoing pkt, accessing encapsulated msg only when transmitter is forcibly turned off
        // set a error flag into the encapsulated msg to inform all nodes rxing this pkt that the transmition is terminated and the pkt is corrupted
        // use the new feature "Reference counting" of encapsulated messages
        //AirFrame *rxPkt;
        double rxPower[27]; // accumulated received power in each channel, for ED measurement purpose
        double rxPeakPower; // peak power in current channle, for ED measurement purpose
        int numCurrRx;

        RadioState rs; // four states model: idle, rxing, txing, sleep
        PHYenum phyRadioState; // three states model, according to spec: RX_ON, TX_ON, TRX_OFF
        PHYenum newState;
        PHYenum newState_turnaround;
        bool isCCAStartIdle; // indicating wheter channel is idle at the starting of CCA

        // timer
        cMessage* CCA_timer; // timer for CCA, delay 8 symbols
        cMessage* ED_timer; // timer for ED measurement
        cMessage* TRX_timer; // timer for Tx2Rx turnaround
        cMessage* TxOver_timer; // timer for tx over
        virtual void registerBattery();
        virtual void updateDisplayString();
        virtual double calcDistFreeSpace();
        bool drawCoverage;
        cMessage *updateString;
        simtime_t updateStringInterval;

        // statistics:
        unsigned long numReceivedCorrectly;
        unsigned long numGivenUp;
        double lossRate;
        static simsignal_t bitrateSignal;
        static simsignal_t radioStateSignal; //enum
        static simsignal_t channelNumberSignal;
        static simsignal_t lossRateSignal;

};

#endif
