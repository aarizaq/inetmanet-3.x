#ifndef Scheduling_H
#define Scheduling_H

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>

//#include <IPDatagram_m.h>

#include "Ieee80216ManagementMessages_m.h"
#include "Ieee80216Primitives_m.h"
#include "global_enums.h"

#include <list>
using namespace std;

/**
 * Module for WiMAX scheduling
 */
class CommonPartSublayerScheduling : public cSimpleModule
{
  private:
    cMessage *per_second_timer;

    int commonPartGateOut, upperLayerGateIn;

    string scheduler;

    // Queues for the different QoS-Classes 
    list<Ieee80216GenericMacHeader> queue_ugs, queue_rtps, queue_ertps, queue_nrtps, queue_be;
    list<Ieee80216GenericMacHeader> queue_fifo;

    double weight_ugs_par, weight_rtps_par, weight_ertps_par, weight_nrtps_par, weight_be_par;

    int max_data_to_send;

    cMessage *scheduleTimer;
    double scheduleInterval;

    int granted_upload_size;

    // bits per scheduler run and QoS class
    int sums[5], sums_perSecond[5];
    int sum_fifo;               //, sum_ugs, sum_rtps, sum_ertps, sum_nrtps, sum_be;

    // output vectors for scheduled traffic
    cOutVector cvec_fifo, cvec_ugs, cvec_rtps, cvec_ertps, cvec_nrtps, cvec_sum_be;
    cOutVector cvec_fifo_perSecond, cvec_ugs_perSecond, cvec_rtps_perSecond, cvec_ertps_perSecond,
        cvec_nrtps_perSecond, cvec_sum_be_perSecond;

    bool equal_weights_for_wrr;

    /**
     * This field keeps track of the connected stations of a BS.
     * When a new station connects, the traffic from the generators is increased
     * and packets are sent to each connected station.
     *
     * Note: This becomes unneccessary as soon as IP-routing is implemented, because
     *           then MSs can communicate with each other and no traffic simulation in
     *           the BS is needed anymore.
     */
//    int connectedStations;

    void doFIFOQueuing(int max_burst_size);
    void doWeightedRoundRobin(int max_burst_size, int *scheduled_packets_size, bool equal_weights);
    void doAbsolutePriorityQueuing(int max_burst_size, int *scheduled_packets_size);

    void updateDisplay();

  public:

    CommonPartSublayerScheduling();
    ~CommonPartSublayerScheduling();

//    void setNumberOfConnectedStations( int stations );
    void sendPacketsDown(int burst_capacity);

  protected:
    void initialize();
    void finish();

    void handleMessage(cMessage *msg);
    void handleUpperMessage(cMessage *msg);
    void handleSelfMessage(cMessage *msg);

    void handleScheduleTimer(cMessage *msg);

    void sendDown(Ieee80216GenericMacHeader *msg);

//    void recordDatarates();
};

#endif
