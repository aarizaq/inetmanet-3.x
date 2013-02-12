#include "CommonPartSublayerReceiver.h"
#include "PhyControlInfo_m.h"
#include "Ieee80216MacHeader_m.h"
Define_Module(CommonPartSublayerReceiver);

CommonPartSublayerReceiver::CommonPartSublayerReceiver()
{
    last_floor = 0;             // helper for updating the display string for the uplink rate
    downlink_rate = 0;
    downlink_daten = 0;
}

CommonPartSublayerReceiver::~CommonPartSublayerReceiver()
{
	for (map<string, vector<cOutVector *> >::iterator it = map_delay_vectors.begin(); it != map_delay_vectors.end(); it++)
	{
		for (vector<cOutVector *>::iterator it2 = it->second.begin(); it2 != it->second.end(); it2++)
		{
			delete *it2;
		}
	}
}

void CommonPartSublayerReceiver::initialize()
{
    convergenceGateOut = findGate("convergenceGateOut");
    controlPlaneGateIn = findGate("controlPlaneGateIn");
    controlPlaneGateOut = findGate("controlPlaneGateOut");
    fragmentationGateIn = findGate("fragmentationGateIn");
    fragmentationGateOut = findGate("fragmentationGateOut");

//    cvec_endToEndDelay.setName("End-to-End Delay");

    cvec_endToEndDelay[pUGS].setName("UGS gEnd-to-End Delay");
    cvec_endToEndDelay[pRTPS].setName("rtPS End-to-End Delay");
    cvec_endToEndDelay[pERTPS].setName("ertPS End-to-End Delay");
    cvec_endToEndDelay[pNRTPS].setName("nrtPS End-to-End Delay");
    cvec_endToEndDelay[pBE].setName("BE End-to-End Delay");
    cvec_downlinkUGS.setName("Downlink UGS");
    // if the sending station is not yet in the list of output vectors, intialize the vectors
    // FIXME: i sollte dynamisch an die anzahl mobilstationen angepasst sein
    for (int i = 1; i <= 50; i++)
    {
        char station_name[10];
        if (i < 10)
            sprintf(station_name, "ms%d%d", 0, i);
        else
            sprintf(station_name, "ms%d", i);

        cOutVector *vecs_for_station[pBE + 1];
        for (int i = pUGS; i <= pBE; i++)
        {
            map_delay_vectors[station_name].push_back(new cOutVector());
            vecs_for_station[i] = map_delay_vectors[station_name].at(i);
        }

        char name[30];
        sprintf(name, "ugs delay for %s", station_name);
        vecs_for_station[pUGS]->setName(name);
        sprintf(name, "rtps delay for %s", station_name);
        vecs_for_station[pRTPS]->setName(name);
        sprintf(name, "ertps delay for %s", station_name);
        vecs_for_station[pERTPS]->setName(name);
        sprintf(name, "nrtps delay for %s", station_name);
        vecs_for_station[pNRTPS]->setName(name);
        sprintf(name, "be delay for %s", station_name);
        vecs_for_station[pBE]->setName(name);

//    double max_delays[pBE+1];
//    max_delays[pUGS] = max_delays[pRTPS] = max_delays[pERTPS] = max_delays[pNRTPS] = max_delays[pBE] = -1;
//
//    map_sums[genericMacFrame->name()] = max_delays;
    }
}

void CommonPartSublayerReceiver::handleMessage(cMessage *msg)
{
    EV << "(in CommonPartSublayerReceiver::handleMessage) " << msg << endl;
    if (msg->getArrivalGateId() == fragmentationGateIn)
    {                           // Nachricht vom physikal Layer empfangen
        EV << "(in CommonPartSublayerReceiver::handleMessage) sende Nachricht an handleLowerMsg-Funktion.\n";
        handleLowerMsg(msg);
    }
    else if (msg->getArrivalGateId() == controlPlaneGateIn)
    {                           // handle commands
        EV << "(in CommonPartSublayerReceiver::handleMessage) sende Nachricht an das Gate fragmentationGateOut.\n";
        send(msg, fragmentationGateOut);
    }
    else
    {
        EV << "(in CommonPartSublayerReceiver::handleMessage) nothing to do in function CommonPartSublayerReceiver::handleMessage" << endl;
    }

    updateDisplay();
}

/**
* Funktion zum Abarbeiten von Nachrichten, die vom Control Plane Modul
* gesendet wurde.
*
****************************************************************************/
void CommonPartSublayerReceiver::handleControlPlaneMsg(cMessage *msg)
{
    if (!msg->isPacket() && msg->getKind() != 0) // Ueberpruefe ob Nachricht ein Befehl ist
    {
        EV << "(in CommonPartSublayerReceiver::handleControlPlaneMsg) Message " << msg->getName() <<
            " ist ein Befehl, also sende an die Funktion handleCommand().\n";
        handleCommand(msg);
        return;
    }
    else
    {
        EV << "(in CommonPartSublayerReceiver::handleControlPlaneMsg) nothing to do in function CommonPartSublayerReceiver::handleControlPlaneMsg" << endl;
    }
}

void CommonPartSublayerReceiver::handleCommand(cMessage *msg)
{
    EV << "(in CommonPartSublayerReceiver::handleCommand) sende Nachricht an das Gate fragmentationGateOut.\n";
    send(msg, fragmentationGateOut);
}

//
//Handelt Pakete aus dem physikal Layer ab
//Überprüft Pakettyp und CID
//
void CommonPartSublayerReceiver::handleLowerMsg(cMessage *msg)
{
    EV << "(in CommonPartSublayerReceiver::handleLowerMsg) received message from physical layer: " << msg << endl;

    Ieee80216MacHeader *macFrame = dynamic_cast<Ieee80216MacHeader *>(msg); // Empfangendes Paket ist eine IEEE802.16e Frame
    if (!macFrame)              // Wenn nicht Fehlermeldung ausgeben
    {
#ifdef FRAMETYPESTOP
        error("message from physical layer is not a IEEE 802.16e MAC frame", msg->getClassName(),
              msg->getName(), msg->getByteLength());
#endif
        EV << "message from physical layer (%s)%s is not a subclass of Ieee80216MacHeader " <<
            msg->getClassName() << " " << msg->getName() << endl;
        delete msg;
        return;

    }

    EV << "CID: " << macFrame->getCID() << endl;

    switch (macFrame->getCID())
    {
    case 0xFFFF:               //Broadcast Mac Frame Tabele 345 im IEEE802.16e Standard
        send(macFrame, controlPlaneGateOut);
        //handleMacFrameType(MacFrame);
        break;

    case 0x0000:               // Wird während des Ranging Process verwendet Tabele 345 im Standard IEEE802.16e
        send(macFrame, controlPlaneGateOut);

        //handleMacFrameType(macFrame);
        break;

    default:
        EV << "Ueberpruefe die PDU:" << macFrame->getName() << " ob ihre CID: " <<
            macFrame->getCID() << " in der MAP Conection enthalten ist." << endl;
        //Ueberprueft ob CID in der Connection Map enthalten ist
        if (CIDInConnectionMap(macFrame->getCID()))
        {
            EV << "CID: " << macFrame->getCID() << " ist in der MAP Conection enthalten!" << endl;
            handleMacFrameType(macFrame);
        }
        else
        {
            EV << "CID: " << macFrame->getCID() << " ist in der MAP Conection NICHT enthalten!" << endl;
            delete macFrame;
        }
    }
}

//
//Ueberprueft ob MAC Paket eine Bandbreitenanvorderung oder ein Generic Mac paket ist
//Wenn es ein Generic Mac Paket ist, ob Daten oder Managementnachricht
//
void CommonPartSublayerReceiver::handleMacFrameType(Ieee80216MacHeader *macFrame)
{
    EV << "(in CommonPartSublayerReceiver::handleMacFrameType) " << endl;
    EV << "Ueberpruefe ob die PDU " << macFrame->getName() << " ein Generic oder Bandwidth Header besitzt.\n";
    if (macFrame->getHT() == 0) // Mac Frame ist ein Generic Mac Frame
    {
        Ieee80216GenericMacHeader *genericMacFrame = dynamic_cast<Ieee80216GenericMacHeader *>(macFrame);
        if (!genericMacFrame)   // Wenn nicht Fehlermeldung ausgeben
            error("Message is not a IEEE 802.16e Generic MAC frame", macFrame->getClassName(),
                  macFrame->getName(), macFrame->getByteLength());
        //send(macFrame,controlPlaneGateOut);

        if (isManagementCID(macFrame->getCID()))
        {
            send(macFrame, controlPlaneGateOut);
        }
        else
        {
            // this is the sink for data packets
            EV << "Name der PDU: " << genericMacFrame->getName() << "\n";
            EV << "Subheader : " << genericMacFrame->getTYPE().Subheader << "\n";
            EV << "Daten PDU erhalten\n";

            downlink_rate += genericMacFrame->getByteLength();

            /**
             * extract the delay from the time of scheduling until arrival in the receiver
             */
            double max_delays[pBE + 1];
            max_delays[pUGS] = max_delays[pRTPS] = max_delays[pERTPS] = max_delays[pNRTPS] =
                max_delays[pBE] = -1;

            Ieee80216PackingSubHeader *psh =
                dynamic_cast<Ieee80216PackingSubHeader *>(genericMacFrame->decapsulate());
            if (psh)
            {

                int incl_datagrams = psh->getIncluded_datagramsArraySize();
                for (int i = 0; i < incl_datagrams; i++)
                {
                    Ieee80216GenericMacHeader *enc_gmh =
                        check_and_cast<Ieee80216GenericMacHeader *>(&psh->getIncluded_datagrams(i));

                    int index = enc_gmh->findPar("priority");
                    int prio = enc_gmh->par(index).doubleValue();

                    index = enc_gmh->findPar("mac_entry_time");
                    double mac_entry_time = enc_gmh->par(index);

                    if (simTime() - mac_entry_time > max_delays[prio])
                        max_delays[prio] = simTime().dbl() - mac_entry_time;
                }
                for (int i = pUGS; i <= pBE; i++)
                {
                    if (max_delays[i] != -1)
                    {
                        cvec_endToEndDelay[i].record(max_delays[i]);
                        //map_delay_vectors[genericMacFrame->name()].at(i)->record( max_delays[i] ); //Auskommentiert von RS erzeugt Fehler
                    }

                    if (i == pUGS)
                    {
                        downlink_daten += genericMacFrame->getByteLength() - 48; //Die gesamte Nutzdatenmenge aufzeichnen
                        cvec_downlinkUGS.record(downlink_daten);
                    }
                }
            }
            delete psh;
            // end of extraction

            delete genericMacFrame;
        }
    }

    else if (macFrame->getHT() == 1) // Mac Frame ist ein Bandwidth Request Frame
    {
        Ieee80216BandwidthMacHeader *bandwidthMacFrame =
            dynamic_cast<Ieee80216BandwidthMacHeader *>(macFrame);
        if (!bandwidthMacFrame) // Wenn nicht Fehlermeldung ausgeben
        {
            error("Message is not a IEEE 802.16e Bandwidth MAC frame", macFrame->getClassName(),
                  macFrame->getName(), macFrame->getByteLength());
        }
        else
        {
            EV << "Ist ein Bandwidth Request! \n";
            //error("Bandwidth Request ACHTUNG muss noch bearbeitet werden!",macFrame->className(), macFrame->name(), macFrame->byteLength());

            send(bandwidthMacFrame, controlPlaneGateOut);
        }
    }

    else
    {
        error("Mac Frame ist weder Generic oder Bandwidth Frame", macFrame->getClassName(),
              macFrame->getName(), macFrame->getByteLength());
    }
}

bool CommonPartSublayerReceiver::CIDInConnectionMap(int mac_pdu_CID)
{
    EV << "(in CommonPartSublayerReceiver::CIDInConnectionMap) " << endl;

    if (map_connections)
    {
        map<int, int>::iterator p_cid = map_connections->find(mac_pdu_CID);
        if (p_cid == map_connections->end())
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        EV << "MAP-Connection nicht vorhanden!\n";
        return false;
    }
}

bool CommonPartSublayerReceiver::isManagementCID(int mac_pdu_CID)
{
    MobileSubscriberStationList::iterator mslist_it;
    EV << "(in CommonPartSublayerReceiver::isManagementCID) " << endl;
    // if the list is still empty, every message has to be forwarded to the ControlPlane
    // because re
    if (msslist)
    {
        for (mslist_it = msslist->begin(); mslist_it != msslist->end(); mslist_it++)
        {
            if (mac_pdu_CID == mslist_it->Basic_CID ||
                mac_pdu_CID == mslist_it->Primary_Management_CID ||
                mac_pdu_CID == mslist_it->Secondary_Management_CID)
            {
                return true;
            }
        }
    }
    return false;
}

/**
 * Helper methods.
 * Only called during initialization.
 */

void CommonPartSublayerReceiver::setConnectionMap(ConnectionMap *pointer_ConnectionsMap)
{
    EV << "(in CommonPartSublayerReceiver::setConnectionMap) " << endl;
    Enter_Method("setConnectionMap()");
    EV << "Pointer " << pointer_ConnectionsMap << "zur MAP-Connection wurde gesetzt!\n";
    map_connections = pointer_ConnectionsMap; //Kopiert Zeiger der ConnectionMap
}

void CommonPartSublayerReceiver::setSubscriberList(MobileSubscriberStationList *pointer_msslist)
{
    EV << "(in CommonPartSublayerReceiver::setSubscriberList(MobileSubscriberStationList)) " << endl;
    Enter_Method("setSubscriberList( MobileSubscriberStationList )");
    msslist = pointer_msslist;
}

void CommonPartSublayerReceiver::setSubscriberList(structMobilestationInfo *mssinfo)
{
    EV << "(in CommonPartSublayerReceiver::setSubscriberList(structMobilestationInfo)) " << endl;
    Enter_Method("setSubscriberList()");

    MobileSubscriberStationList *pointer_msslist = new MobileSubscriberStationList();

    // helper object...
    structMobilestationInfo *mssinfo_local = new structMobilestationInfo();
    mssinfo_local->Basic_CID = mssinfo->Basic_CID;
    mssinfo_local->Primary_Management_CID = mssinfo->Primary_Management_CID;
    mssinfo_local->Secondary_Management_CID = mssinfo->Secondary_Management_CID;

    pointer_msslist->push_back(*mssinfo_local);

    msslist = pointer_msslist;
}

void CommonPartSublayerReceiver::updateDisplay()
{
    /**
     * The transmitted datarate is displayed beside WiMAX-Module.
     * Update the displayed string every second -> datarate per second
     */

    EV << "\n\ndownlink: " << "downlink_rate: " << downlink_rate << ", cur_floor:  " << cur_floor <<
        ", last_floor: " << last_floor << "\n\n";
    cur_floor = floor(simTime().dbl());
    if ((cur_floor - last_floor) == 1)
    {
        char rates_buf[50];
        sprintf(rates_buf, "\n\nRx: %d bit/s", downlink_rate);
        getParentModule()->getParentModule()->getDisplayString().setTagArg("t", 0, rates_buf);
        getParentModule()->getParentModule()->getDisplayString().setTagArg("t", 1, "l");

        downlink_rate = 0;
        last_floor = cur_floor;
    }
}
