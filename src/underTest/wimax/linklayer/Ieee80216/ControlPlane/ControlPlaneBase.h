//
// Diese Datei ist teil einer WiMAX mobile Simulation
// unter OmNet++ und INET
// Copyright (C) 2007 Roland Siedlaczek
//
#ifndef Control_Plane_Base_H
#define Control_Plane_Base_H

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "PhyControlInfo_m.h"
#include "RadioState.h"
#include <cmodule.h>
#include "SnrList.h"

#include "Ieee80216ManagementMessages_m.h"
#include "Ieee80216Primitives_m.h"
#include "global_enums.h"
#include "Ieee80216Radio.h"
#include "SnrList.h"

/**
* Die ControlPlaneBase Klasse enthält die Grundfunktionen
* die fr den Umgang mit Nachrichten an den Gates.
* Die Klasse besitzt drei Eingänge:     cpsDownIn
*                                       cpsUpIn
*                                       radioDownIn
* Die Klasse besitzt drei Ausgaenge     cpsDownOut
*                                       cpsUpOut
*                                       radioUpOut
*/

//class NotificationBoard;

class ControlPlaneBase : public cSimpleModule
{
  public:

    /**
    * @name Burst Profile List
    * Die Liste enthaelt Burst Parameter aller Verbindungen
    */
    //typedef std::list<RadioBurstStruct> RadioBurstList;
    //RadioBurstList burstList;

  protected:
   /**
   *  @brief gate id
   *******************/
    int receiverCommonGateIn;
    int receiverCommonGateOut;
    int transceiverCommonGateIn;
    int transceiverCommonGateOut;

    int trafficClassificationGateIn;
    int serviceFlowsGateIn, serviceFlowsGateOut;

    /** Physical radio (medium) state copied from physical layer */
    RadioState::State radioState;

    cModule *nicMS;
    cModule *snrRadio;

    //cOutVector recSnrVec;
    //cOutVector recDistVec;

    /**
     * These maps hold the CID/SFID (map_connections)
     * and SFID/ServiceFlow (map_serviceFlows) associations for each station.
     * Management connections do not have a ServiceFlow,
     * so their SFID is set to -1.
     */
    ConnectionMap *map_connections;
    ServiceFlowMap *map_serviceFlows;

    /**
    *  Informationselemente der DL- und UL-MAp, enhalten Informationen ueber die Burst-Pofile
    *******************/

    typedef std::list<Ieee80216DL_MAP_IE> DL_MAP_InformationselementList;
    DL_MAP_InformationselementList dl_map_ie_List;

    typedef std::list<Ieee80216UL_MAP_IE> UL_MAP_InformationselementList;
    UL_MAP_InformationselementList ul_map_ie_List;

  private:

  public:
    Ieee80216Radio *receiverRadio;
    Ieee80216Radio *transceiverRadio;

    int bitrate;
    bool stationHasUplinkGrants;

  protected:
  /**
  * @brief Hauptfunktionen
  ***************************************************************************************/
    virtual void initialize(int);

    void handleMessage(cMessage *msg);

    virtual void handleClassificationCommand(Ieee80216ClassificationCommand *msg) = 0;
    virtual void handleServiceFlowMessage(cMessage *msg) = 0;
    virtual void handleHigherLayerMsg(cMessage *msg) = 0;
    virtual void handleLowerLayerMsg(cMessage *msg) = 0;
    virtual void handleSelfMsg(cMessage *msg) = 0;

  /**
  * @brief Hilfsfunktionen
  ***************************************************************************************/
    bool isHigherLayerMsg(cMessage *msg);
    bool isLowerLayerMsg(cMessage *msg);

    void sendtoLowerLayer(cMessage *msg);
    void sendtoHigherLayer(cMessage *msg);

    void sendRequest(Ieee80216PrimRequest *req);

    void listActiveServiceFlows();
    ServiceFlow *getServiceFlowForCID(int cid);
    int getSFIDForCID(int cid);
  /**
   * QoS specific functions
   */

  public:
    typedef std::list<Ieee80216MacHeader *> Ieee80216MacHeaderFrameList; //Warteschlange fuer MAC-Pakete

    void storeBSInfo(double rcvdPower);
    void setSNR(double rcvdPower);
    void setTime(double rcvdPower);
    void setDistance(double rcvdPower);
    void setBSID(double rcvdPower);

    double searchMinSnr();

    double pduDuration(Ieee80216MacHeader *msg);
    double pduDuration(int bits, double bitrate);
    std::list<int> findMatchingServiceFlow(ip_traffic_types traffic_type, link_direction link_type);

    virtual station_type getStationType() = 0;
    virtual management_type getManagementType(int CID) = 0;
    virtual void setDataMsgQueue(Ieee80216MacHeaderFrameList *data_queue) = 0;

    virtual int getBasicCID() = 0;
    virtual int getPrimaryCID() = 0;
};
#endif
