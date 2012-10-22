#include "CommonPartSublayerUp.h"
#include "PhyControlInfo_m.h"

Define_Module(CommonPartSublayerUp);

CommonPartSublayerUp::CommonPartSublayerUp()
{
    endTransmissionEvent = NULL;
}

CommonPartSublayerUp::~CommonPartSublayerUp()
{
    cancelAndDelete(endTransmissionEvent);
}

void CommonPartSublayerUp::initialize()
{
    eigene_CID = 0x0000;

    queue.setName("queue");
    endTransmissionEvent = new cMessage("endTxEvent");
}

void CommonPartSublayerUp::handleMessage(cMessage *msg)
{
    // handle commands
/*    if (msg->arrivedOn("controlIn") && msg->getByteLength() == 0)
    {
    }
*/
    if (msg->arrivedOn("controlIn"))
    {
        //send(msg,"lowerLayerOut");
    }
    else if (msg->arrivedOn("lowerLayerIn")) // Nachricht vom physikal Layer empfangen
    {
        handleLowerMsg(check_and_cast<cPacket *>(msg));
    }
    else
    {
        ev << "nothing" << endl;
    }
}

void CommonPartSublayerUp::handleCommand(int msgkind, cPolymorphic *ctrl)
{
}

//
//Handelt Pakete aus dem physikal Layer ab
//Überprüft Pakettyp und CID
//
void CommonPartSublayerUp::handleLowerMsg(cPacket *msg)
{
    ev << "received message from physical layer: " << msg << endl;

    Ieee80216MacHeader* MacFrame = dynamic_cast<Ieee80216MacHeader *>(msg); // Empfangendes Paket ist eine IEEE802.16e Frame

    if (!MacFrame)              // Wenn nicht Fehlermeldung ausgeben
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

    ev << "CID: " << MacFrame->getCID() << endl;

    switch (MacFrame->getCID())
    {
    case 0xFFFF:               //Broadcast Mac Frame Tabele 345 im IEEE802.16e Standard
        handleMacFrameType(MacFrame);
        break;

    case 0x0000:               // Wird während des Ranging Process verwendet Tabele 345 im Standard IEEE802.16e
        handleMacFrameType(MacFrame);
        break;

    default:
        if (MacFrame->getCID() == eigene_CID)
        {
            handleMacFrameType(MacFrame);
        }
        else
        {
            ev << "Die CID: " << MacFrame->getCID() << " ist nicht vergeben" << endl;
        }
        break;
    }
}

//
//Überprüft ob MAC Paket eine Bandbreitenanvorderung oder ein Generic Mac paket ist
//Wenn es ein Generic Mac Paket ist, ob Daten oder Managementnachricht
//
void CommonPartSublayerUp::handleMacFrameType(Ieee80216MacHeader *MacFrame)
{
    if (MacFrame && MacFrame->getHT() == 0) // Mac Frame ist ein Generic Mac Frame
    {
        Ieee80216GenericMacHeader* GenericMacFrame =
            dynamic_cast<Ieee80216GenericMacHeader *>(MacFrame);
        if (!MacFrame)          // Wenn nicht Fehlermeldung ausgeben
            error("Message is not a IEEE 802.16e Generic MAC frame", MacFrame->getClassName(),
                  MacFrame->getName(), MacFrame->getByteLength());

        switch (GenericMacFrame->getTYPE().Subheader) //Enthält Generic Mac Paket Daten oder Kontrolnachrichten
        {
        case 0:
            break;

        case 1:                // Enthält Kontrolnachrichten
            send(GenericMacFrame, "controlplaneOut");
            break;
        }
    }
    else                        // Mac Frame ist ein Bandwidth Request Frame
    {
        Ieee80216BandwidthMacHeader* BandwidthMacFrame =
            dynamic_cast<Ieee80216BandwidthMacHeader *>(MacFrame);
        if (!BandwidthMacFrame)          // Wenn nicht Fehlermeldung ausgeben
            error("Message is not a IEEE 802.16e Bandwidth MAC frame", MacFrame->getClassName(),
                  MacFrame->getName(), MacFrame->getByteLength());
    }
}
