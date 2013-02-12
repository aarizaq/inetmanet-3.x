//
// Autor Roland Siedlaczek
//
// Das "CommonPartSublayerDown" Module bearbeitet Pakete  "ControlPlane**" <-> "CommonPartSublayerDown"
// die von Höheren Schichten kommen und Pakete die vom       |
// "ControlPlane**" Module Kommen.
// Managementnachrichten die von Höheren Schichten gesendet
// wurden, werden an die "ControlPlane**" weitergeleitet.
//

#ifndef IEEE_80216_MAC_H
#define IEEE_80216_MAC_H

#define FSM_DEBUG

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include <INETDefs.h>
#include <NotifierConsts.h>

#include "MACAddress.h"
#include "IPassiveQueue.h"

#include "Ieee80216MacHeader_m.h"
#include "NotificationBoard.h"
#include "RadioState.h"         // Beinhaltet die für die Statusabfrage des Radio Modules nötigen Variablen
#include "PhyControlInfo_m.h"   // Beinhaltet die Kommandobefehle für die Steuerung des RadioModuls
#include "Ieee80216Primitives_m.h"
#include "FSMA.h"
#include "global_enums.h"

#include "ControlPlaneBase.h"
#include "ControlPlaneMobilestation.h"
#include "ControlPlaneBasestation.h"

/**
 * IEEEE802.16e Common Part Sublayer
 *
 */
class CommonPartSublayerTransceiver : public cSimpleModule, public INotifiable
{
    typedef std::list<Ieee80216MacHeader *> Ieee80216MacHeaderFrameList; //Warteschlange fr MAC-Pakete

  protected:
    /** brief gate id*/
    //{
    int controlPlaneGateIn;
    int controlPlaneGateOut;
    int schedulingGateIn;
    int fragmentationGateOut;

    int sfIn, sfOut;

    //@}

    /** @brief Speichert Zeiger auf das Notification Board Module*/
    NotificationBoard *nb;

  protected:
    /** @name Timer messages */

    cMessage *nextControlQueueElement;
    cMessage *nextDataQueueElement;

    /** Radio state change self message. Wenn sich der Satus des Radio aendert wird diese Nachricht an das Modul gesendet */
    cMessage *mediumStateChange; //Self-Message bei Änderungen des Radio State

    cMessage *endTimeout;

    /**
     * Wenn die Radio Kontrol Nachricht nicht an das Radio Modul senden kann, weil es im Transmit/Receive Modus ist
     * wird die nachricht hier zwischengespeichert und erst wenn das Module bereit ist weitergeleitet.
     */
    cMessage *pendingRadioConfigMsg;

    cMessage *stopTransmision;

    /**
    * @name Ieee80216 state variables
    * Zustandsinformation und Variablen der State Machine
    **/

/*
    enum State {
        START,
        BROADCASTSEND,
        DATASEND,
    };
*/
    enum State {
        START,
        CONTROLSEND,
        CONTROLQUEUE
    };

    cFSM fsmb;

    RadioState::State radioState; // Kopie des Zustands (state) des Physical Radio (IDLE,SLEEP,TRANSMIT,RECEIVE)
    RadioState *newRadioState;

  protected:
    /** Passive queue module to request messages from */
    IPassiveQueue *queueModule;

    /** @brief Nachrichten zwischenspeichern und
    @brief spaeter an das Radio Module weiter senden */
    Ieee80216MacHeaderFrameList requestMsgQueue;
    Ieee80216MacHeaderFrameList controlMsgQueue;
    Ieee80216MacHeaderFrameList basicCIDQueue;
    Ieee80216MacHeaderFrameList primaryCIDQueue;
    Ieee80216MacHeaderFrameList dataMsgQueue;

    bool transmitControlPDU;
    bool transmitDataPDU;

    cOutVector cvec_module_sending;

  private:
    int frame_CID;
    double bitrate;
    bool allowedToSend;
    int radioId;
    ControlPlaneBase *controlplane;

    queue_id queue_to_pop;

    int uplink_rate, cur_floor, last_floor;
    int missed_cqe_counter;

    cMessage *per_second_timer;
    int sums_perSecond[5];
    int sums_Data[5];
    cOutVector cvec_ugs_perSecond, cvec_rtps_perSecond, cvec_ertps_perSecond, cvec_nrtps_perSecond,
        cvec_be_perSecond;
    cOutVector cvec_throughput_uplink;
    cOutVector cvec_sum_ugs, cvec_sum_rtps, cvec_sum_ertps, cvec_sum_nrtps, cvec_sum_be;
  public:
    CommonPartSublayerTransceiver();
    virtual ~CommonPartSublayerTransceiver();

  protected:
    /**
    * @brief Hauptfunktionen
    ***************************************************************************************/

    void initialize();

    /**
    * @brief Wird immer dann aufgerufen wenn eine Nachricht das Modul erreicht
    */
    void handleMessage(cMessage *msg);
    /**
    * @brief Wird von handleMessage aufgerufen wenn die Nachricht von der oberen Ebene kommt
    */
    void handleUpperSublayerMsg(cMessage *msg);
    /**
    * @brief Wird von handleMessage aufgerufen wenn die Nachricht von dem ControlPlane Module kommt
    */
    void handleControlPlaneMsg(cMessage *msg);

    void handleControlRequest(cMessage *msg);

    /**
    * @brief Wird von ControlPlaneMsg aufgerufen wenn die Nachricht von der ControlPlane eine Radio Cofig Message ist
    */
    void handleCommand(cMessage *msg);
    /**
    * @brief Bearbeitet alle Nachrichten und �derungen die mit der State Maschine in Verbindung stehen
    */
    void handleWithFSM(cMessage *msg);
    /**
    * @brief Wird vom NotificationBoard aufgerufen, wann immer eine �derung an einem im NED angeschlossenen Module detektiert wird
    */
    virtual void receiveChangeNotification(int category, const cPolymorphic *details);
    /**
    * @brief Wird aufgerufen wenn die Nachricht an das Radio Modul gesendet werden soll
    */
    void sendDown(cMessage *msg);

  private:
    /**
    * @brief Hilfsfunktionen
    ***************************************************************************************/

    /**
    * @brief Funktionen fr die Bearbeitung der Queue
    */
    Ieee80216MacHeader* currentManagementMsgQueue();
    void popManagementMsgQueue();

    /**
    * @brief Funktionen um die Art der Nachricht zu bestimmen
    */
    bool isUpperSublayerMsg(cMessage *msg);
    bool isControlPlaneMsg(cMessage *msg);
    bool isNextControlQueueElement(cMessage *msg);

    bool isMediumStateChange(cMessage *msg);
    bool isTimeout(cMessage *msg);
    bool isStopTransmision(cMessage *msg);

    //(mk)
    bool isBroadcastSendRequest(cMessage *msg);

    /**
    * @brief Funktionen zur Abfrage des Radio Status
    */
    bool isMediumFree();        // Ist der Radio Satus = IDLE und damit bereit zu Senden
    bool isMediumTransmit();    // Ist der Radio Satus = Transmit und damit belegt
    bool isMagMsg(cMessage *msg);

    void sendDownPendingRadioConfigMsg();

    void setAllowedToSend(bool send);
    bool getAllowedToSend();
    bool isAllowedToSend();

    Ieee80216GenericMacHeader* packedDataForCID(int CID, int bits_for_burst);
    bool managementQueuesEmpty(Ieee80216Prim_sendControlRequest *sc_req);

    void recordDatarates();

    void updateDisplay();

    void recordDataoutput();
};

#endif
