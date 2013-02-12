#include "ConvergenceSublayerControlModule.h"
#include "IPv4.h"
#include "IPv4ControlInfo.h"

Define_Module(ConvergenceSublayerControlModule);

ConvergenceSublayerControlModule::ConvergenceSublayerControlModule()
{
}

ConvergenceSublayerControlModule::~ConvergenceSublayerControlModule()
{
}

void ConvergenceSublayerControlModule::initialize()
{
// find the existing gates for better performance
    outerIn = findGate("outerIn");
    outerOut = findGate("outerOut");
    tcIn = findGate("tcIn");
    tcOut = findGate("tcOut");
    compIn = findGate("compIn");
    compOut = findGate("compOut");

    //timerEvent = new cMessage("selfTimer");
    //scheduleAt(0.005, timerEvent);
}

void ConvergenceSublayerControlModule::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {                           //msg comes from outside (the packet generator).
        handleSelfMessage(msg);
    }
    else if (msg->getArrivalGateId() == outerIn)
    {
        handleUpperLayerMessage(check_and_cast<cPacket *>(msg));
    }
    //If the message comes back from classification, fragment/pack it
    else if (msg->getArrivalGateId() == tcIn)
    {

        if (dynamic_cast<Ieee80216_DSA_REQ *>(msg))
        {
            send(msg, outerOut);
        }

        //send( msg, compOut);
    }
}

//msg comes from outside (the packet generator).
//It has to be classified, so it's forwarded to the TrafficClassification module
void ConvergenceSublayerControlModule::handleUpperLayerMessage(cPacket *msg)
{
    EV << "Message arrived in CS: " << msg << "\n";

    if (msg->getByteLength() != 0)
    {
        EV << "MESSAGE LENGTH: " << msg->getByteLength() << "\n";

        // vorrübergehend: handle incoming packet as payload and
        // encapsulate into IPDatagram for classification
        IPv4Datagram *ipd = new IPv4Datagram();
        ipd->encapsulate(msg);

        send(ipd, tcOut);

        if (dynamic_cast<Ieee80216_DSA_REQ *>(msg))
        {
            //forward to CPL
            //send( msg, )
        }

        // incoming ip-packets
        if (dynamic_cast<IPv4ControlInfo *>(msg->getControlInfo()))
        {
        	IPv4ControlInfo *ip_ctrlinfo = check_and_cast<IPv4ControlInfo *>(msg->getControlInfo());

            if (ip_ctrlinfo != NULL)
            {
                EV << "IPControlInfo arrived. Length: " << ip_ctrlinfo << "\n";
                send(msg, tcOut);
            }
            else
            {
                EV << "Non-IP-Packet arrived\n";
            }
        }
    }

    //send( msg, "tcOut" );
}

void ConvergenceSublayerControlModule::handleSelfMessage(cMessage *msg)
{
    //dummy-content until MobileIP is integrated
    IPv4Datagram *ipd = new IPv4Datagram();
    ipd->setDestAddress(IPv4Address("123.123.123.123"));
    ipd->setHeaderLength(IP_HEADER_BYTES);

    EV << "Created new IPDatagram - sending it to TrafficClassification\n";

    send(ipd, tcOut);

//  IInterfaceTable *itable = InterfaceTableAccess().get();
//  EV << "\n\n\nInterface: " << itable->getInterface(0)->name() <<"\n\n\n";
    //------------------------------------------
}
