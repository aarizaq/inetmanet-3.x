#include "CommonPartSublayerScheduling.h"
#include "IPv4Datagram.h"

Define_Module(CommonPartSublayerScheduling);

CommonPartSublayerScheduling::CommonPartSublayerScheduling()
{
}

CommonPartSublayerScheduling::~CommonPartSublayerScheduling()
{
}

void CommonPartSublayerScheduling::initialize()
{
    commonPartGateOut = findGate("commonPartGateOut");
    upperLayerGateIn = findGate("upperLayerGateIn");
    // par stehen immer in der .ini
    scheduler = par("scheduler").stringValue();

    weight_ugs_par = par("weight_ugs");
    weight_rtps_par = par("weight_rtps");
    weight_ertps_par = par("weight_ertps");
    weight_nrtps_par = par("weight_nrtps");
    weight_be_par = par("weight_be");

    equal_weights_for_wrr = par("equal_weights_for_wrr").boolValue();

    max_data_to_send = 128;

    cvec_fifo.setName("Scheduled FIFO Traffic"); // Output-Vektoren zum Aufzeichnen der TG-Daten
    cvec_ugs.setName("Scheduled UGS Traffic");
    cvec_rtps.setName("Scheduled rtPS Traffic");
    cvec_ertps.setName("Scheduled ertPS Traffic");
    cvec_nrtps.setName("Scheduled nrtPS Traffic");
    cvec_sum_be.setName("Scheduled BE Traffic");

// cvec_fifo_perSecond.setName("Scheduled FIFO Traffic per Second");
// cvec_ugs_perSecond.setName("Scheduled UGS Traffic per Second");
// cvec_rtps_perSecond.setName("Scheduled rtPS Traffic per Second");
// cvec_ertps_perSecond.setName("Scheduled ertPS Traffic per Second");
// cvec_nrtps_perSecond.setName("Scheduled nrtPS Traffic per Second");
// cvec_sum_be_perSecond.setName("Scheduled BE Traffic per Second");

// per_second_timer = new cMessage("per_second_timer");
// scheduleAt(simTime(), per_second_timer);

    updateDisplay();            // GUI wird aktualisiert: Füllstand der Queues, Formatierung...
}

void CommonPartSublayerScheduling::finish()
{
// char sched[];
// strcpy( sched, scheduler.c_str() );
// recordScalar("Packet Scheduler", sched);

    recordScalar("WRR equal weights", equal_weights_for_wrr);
    recordScalar("WRR weight UGS", weight_ugs_par);
    recordScalar("WRR weight RTPS", weight_rtps_par);
    recordScalar("WRR weight ERTPS", weight_ertps_par);
    recordScalar("WRR weight NRTPS", weight_nrtps_par);
    recordScalar("WRR weight BE", weight_be_par); //schreiben auf HDD
}                               //ggf. können die records gelöscht werden....werden nicht weiter verwendet.

void CommonPartSublayerScheduling::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
//  if ( msg == per_second_timer ) {
//   recordDatarates();
//   scheduleAt(simTime()+1, per_second_timer);
//  }
    }
    // message arrives from traffic classification
    else if (msg->getArrivalGateId() == upperLayerGateIn)
    {

        Ieee80216GenericMacHeader* upper_msg = check_and_cast<Ieee80216GenericMacHeader*>(msg);

        // attach the timestamp for delay measurements in receiver module
        cMsgPar *mac_entry_time = new cMsgPar();
        mac_entry_time->setName("mac_entry_time"); //for identifying parameter at receiver
        mac_entry_time->setDoubleValue(simTime().dbl());
        upper_msg->addPar(mac_entry_time);
        IPv4Datagram* upper_ipd = check_and_cast<IPv4Datagram*>(upper_msg->getEncapsulatedPacket());
        Ieee80216TGControlInformation* ipd_control =
            check_and_cast<Ieee80216TGControlInformation*>(upper_ipd->getControlInfo());

        if (scheduler == "FIFO")
        {
            if (queue_fifo.size() < 3000)
                queue_fifo.push_back(*upper_msg);
        }
        else if (scheduler == "WRR" || scheduler == "APQ")
        {
            switch (ipd_control->getTraffic_type())
            {
            case UGS:
                if (queue_ugs.size() < 500) // Queue  Size hardcoded here. Maybe .ini is a better place for this
                    queue_ugs.push_back(*upper_msg);
                break;

            case RTPS:
                if (queue_rtps.size() < 500)
                    queue_rtps.push_back(*upper_msg);
                break;

            case ERTPS:
                if (queue_ertps.size() < 500)
                    queue_ertps.push_back(*upper_msg);
                break;

            case NRTPS:
                if (queue_nrtps.size() < 500)
                    queue_nrtps.push_back(*upper_msg);
                break;

            case BE:
                if (queue_be.size() < 500)
                    queue_be.push_back(*upper_msg);
                break;
            }
        }
        else
            delete upper_msg;

        delete msg;
    }

    updateDisplay();
}

/**
 * Public method to be called from the control plane module.
 * It needs to be called just before the DL-MAP is built (basestation)
 * and when the UL-MAP arrives (mobilestation).
 *
 * Packets are scheduled according to the specified scheduling method until
 * the burst limit is reached or the queues run empty.
 *
 * @par burst_capacity
 *      The maximum size in bits for the next downlink- /uplink-burst
 */
void CommonPartSublayerScheduling::sendPacketsDown(int burst_capacity)
{
    Enter_Method("sendPacketsDown()");

    int pending_packets =
        queue_ugs.size() + queue_rtps.size() + queue_be.size() + queue_nrtps.size() +
        queue_ertps.size();
    int scheduled_packets_size = 0;

    //sum_fifo = 0;
    sums[pUGS] = sums[pRTPS] = sums[pERTPS] = sums[pNRTPS] = sums[pBE] = 0;

    if (scheduler == "FIFO")
    {
        doFIFOQueuing(burst_capacity);
    }
    else if (scheduler == "WRR")
    {
        while (scheduled_packets_size<burst_capacity && pending_packets>0)
        {
            EV << "scheduledPacketSize: " << scheduled_packets_size << "  burstCapacity: " <<
                burst_capacity << "\n";
            doWeightedRoundRobin(burst_capacity, &scheduled_packets_size, false);

            pending_packets =
                queue_ugs.size() + queue_rtps.size() + queue_be.size() + queue_nrtps.size() +
                queue_ertps.size();
        }
    }
    else if (scheduler == "APQ")
    {
        EV << "scheduledPacketSize: " << scheduled_packets_size << "  burstCapacity: " <<
            burst_capacity << "\n";
        doAbsolutePriorityQueuing(burst_capacity, &scheduled_packets_size);
    }

    //cvec_fifo.record( sum_fifo );

    cvec_ugs.record(sums[pUGS]);
    cvec_rtps.record(sums[pRTPS]);
    cvec_ertps.record(sums[pERTPS]);
    cvec_nrtps.record(sums[pNRTPS]);
    cvec_sum_be.record(sums[pBE]);

    updateDisplay();
}

void CommonPartSublayerScheduling::doFIFOQueuing(int max_burst_size)
{
    int scheduled_packets_size = 0;

    while (scheduled_packets_size < max_burst_size && queue_fifo.size() > 0)
    {
        // get pointer to first element of the queue
        Ieee80216GenericMacHeader *tmp_msg = &queue_fifo.front();

        if (tmp_msg->getByteLength() + scheduled_packets_size <= max_burst_size)
        {
            scheduled_packets_size += tmp_msg->getByteLength();
            //sum_fifo += tmp_msg->length();
            IPv4Datagram* upper_ipd = check_and_cast<IPv4Datagram*>(tmp_msg->getEncapsulatedPacket());
            Ieee80216TGControlInformation* ipd_control =
                check_and_cast<Ieee80216TGControlInformation*>(upper_ipd->getControlInfo());

            priorities priority;

            // record burst-sizes for datarate calculations
            switch (ipd_control->getTraffic_type())
            {
            case UGS:
                sums[pUGS] += tmp_msg->getByteLength();
                priority = pUGS;
//              sums_perSecond[pUGS] += tmp_msg->length();
                break;
            case RTPS:
                sums[pRTPS] += tmp_msg->getByteLength();
                priority = pRTPS;
//              sums_perSecond[pRTPS] += tmp_msg->length();
                break;
            case ERTPS:
                sums[pERTPS] += tmp_msg->getByteLength();
                priority = pERTPS;
//              sums_perSecond[pERTPS] += tmp_msg->length();
                break;
            case NRTPS:
                sums[pNRTPS] += tmp_msg->getByteLength();
                priority = pNRTPS;
//              sums_perSecond[pNRTPS] += tmp_msg->length();
                break;
            case BE:
                sums[pBE] += tmp_msg->getByteLength();
                priority = pBE;
//              sums_perSecond[pBE] += tmp_msg->length();
                break;
            }

            Ieee80216GenericMacHeader* msg = (Ieee80216GenericMacHeader*) queue_fifo.front().dup();

            cMsgPar* prio = new cMsgPar();
            prio->setName("priority");
            prio->setDoubleValue(priority);
            msg->addPar(prio);

            sendDown(msg);
            queue_fifo.pop_front();
        }
        else
        {
            scheduled_packets_size = max_burst_size;
        }

        delete tmp_msg;
    }
}

void CommonPartSublayerScheduling::doWeightedRoundRobin(int max_burst_size,
                                                        int *scheduled_packets_size,
                                                        bool equal_weights)
{

    // preparations for recording purposes
    int packet_size_too_large_counter = 0;
    int weights[5];

    list<Ieee80216GenericMacHeader>*queue_ptr[5];
    queue_ptr[pUGS] = &queue_ugs;
    queue_ptr[pRTPS] = &queue_rtps;
    queue_ptr[pERTPS] = &queue_ertps;
    queue_ptr[pNRTPS] = &queue_nrtps;
    queue_ptr[pBE] = &queue_be;

    if (equal_weights)
    {
        weights[pUGS] = weights[pRTPS] = weights[pERTPS] = weights[pNRTPS] = weights[pBE] = 1;
    }
    else
    {
        // order represents descending priority
        weights[pUGS] = (int) weight_ugs_par;
        weights[pRTPS] = (int) weight_rtps_par;
        weights[pERTPS] = (int) weight_ertps_par;
        weights[pNRTPS] = (int) weight_nrtps_par;
        weights[pBE] = (int) weight_be_par;
    }

    // (W)RR Algorithm
    for (int priority = pUGS; priority <= pBE; priority++)
    {
        bool cancel_priority = false;

        for (int number_of_packets = 0; number_of_packets < weights[priority]; number_of_packets++)
        {
            if (queue_ptr[priority]->size() > 0 && !cancel_priority)
            {
                Ieee80216GenericMacHeader* msg =
                    (Ieee80216GenericMacHeader *)queue_ptr[priority]->front().dup();

                if (*scheduled_packets_size + msg->getByteLength() <= max_burst_size)
                {
                    *scheduled_packets_size += msg->getByteLength();
                    EV << "  packetSize: " << msg->getByteLength() << "\n";

                    sums[priority] += msg->getByteLength();
//                  sums_perSecond[priority] += msg->length();

                    cMsgPar* prio = new cMsgPar();
                    prio->setName("priority");
                    prio->setDoubleValue(priority);
                    msg->addPar(prio);

                    sendDown(msg);
                    queue_ptr[priority]->pop_front();
                }
                else
                {
                    // EXAMPLE: in case that UGS doesn`t fit into max. burst_size per grant. Then try, if next lower priority is still fitting, e.g. rtps packets => fits, because packet size is much more smaller
                    // NOTE: Packet Size is variable. So not only the weighting factor is important for this.
                    // TODO: FRAGMENTATION has got impact on this?!!!?!?
                    cancel_priority = true;
                    delete msg;
                }
            }
        }

        if (cancel_priority)
            packet_size_too_large_counter++; //If all packets to large. If even BE doesn't fit, then break. Otherwise endless loop.
        //EV << "packet size error counter: " << packet_size_too_large_counter << "\n";
    }

    if (packet_size_too_large_counter > 0)
        *scheduled_packets_size = max_burst_size; // Für Abbr. Bdg. in While-Schleife
}

void CommonPartSublayerScheduling::doAbsolutePriorityQueuing(int max_burst_size,
                                                             int *scheduled_packets_size)
{
    list<Ieee80216GenericMacHeader>* queue_ptr[5];
    queue_ptr[pUGS] = &queue_ugs;
    queue_ptr[pRTPS] = &queue_rtps;
    queue_ptr[pERTPS] = &queue_ertps;
    queue_ptr[pNRTPS] = &queue_nrtps;
    queue_ptr[pBE] = &queue_be;

    for (int priority = pUGS; priority <= pBE; priority++)
    {
        bool cancel_priority = false;

        while (queue_ptr[priority]->size() > 0 && *scheduled_packets_size < max_burst_size
               && !cancel_priority)
        {

            Ieee80216GenericMacHeader *msg =
                (Ieee80216GenericMacHeader *) queue_ptr[priority]->front().dup();

            if (*scheduled_packets_size + msg->getByteLength() <= max_burst_size)
            {
                *scheduled_packets_size += msg->getByteLength();
                EV << "  packetSize: " << msg->getByteLength() << "\n";

                sums[priority] += msg->getByteLength();
//              sums_perSecond[priority] += msg->length();

                cMsgPar* prio = new cMsgPar();
                prio->setName("priority");
                prio->setDoubleValue(priority);
                msg->addPar(prio);

                sendDown(msg);
                queue_ptr[priority]->pop_front();
            }
            else
            {
                cancel_priority = true;
                delete msg;
            }
        }
    }

    /*
    for ( unsigned int i=0; i<queue_ugs.size(); i++ )
    {
        cMessage *msg = &(queue_ugs.front());
        data_sent += msg->length();

        if ( data_sent <= max_data_to_send )
        {
            send( msg, commonPartGateOut );
            queue_ugs.pop_front();
        }
        else return;
    }

    for ( unsigned int i=0; i<queue_rtps.size(); i++ )
    {
        cMessage *msg = &(queue_rtps.front());
        data_sent += msg->length();

        if ( data_sent <= max_data_to_send )
        {
            send( msg, commonPartGateOut );
            queue_rtps.pop_front();
        }
        else
            return;
    }

    for ( unsigned int i=0; i<queue_ertps.size(); i++)
    {
        cMessage *msg = &(queue_ertps.front());
        data_sent += msg->length();

        if ( data_sent <= max_data_to_send )
        {
            send( msg, commonPartGateOut );
            queue_ertps.pop_front();
        }
        else
            return;
    }

    for ( unsigned int i=0; i<queue_nrtps.size(); i++)
    {
        cMessage *msg = &(queue_nrtps.front());
        data_sent += msg->length();

        if ( data_sent <= max_data_to_send )
        {
            send( msg, commonPartGateOut );
            queue_nrtps.pop_front();
        }
        else
            return;
    }

    for ( unsigned int i=0; i<queue_be.size(); i++)
    {
        cMessage *msg = &(queue_be.front());
        data_sent += msg->length();

        if ( data_sent <= max_data_to_send )
        {
            send( msg, commonPartGateOut );
            queue_be.pop_front();
        }
        else
            return;
    }
    */
}

void CommonPartSublayerScheduling::sendDown(Ieee80216GenericMacHeader* msg)
{
    send(msg, commonPartGateOut);
}

void CommonPartSublayerScheduling::updateDisplay()
{
    char buf[90];

    if (scheduler == "FIFO")
    {
        sprintf(buf, "FIFO Queue: %d", (int) (queue_fifo.size()));
    }
    else if (scheduler == "WRR" || scheduler == "APQ")
    {
        sprintf(buf,
                "UGS Queue: %d\nrtPS Queue: %d\nertPS Queue: %d\nnrtPS Queue: %d\nBE Queue: %d",
                (int) (queue_ugs.size()), (int) (queue_rtps.size()), (int) (queue_ertps.size()),
                (int) (queue_nrtps.size()), (int) (queue_be.size()));
    }

    getDisplayString().setTagArg("t", 0, buf);
    getDisplayString().setTagArg("t", 1, "r");
}
