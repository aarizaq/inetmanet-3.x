

#include "Ieee80211AgentSTAExtended.h"
#include "Ieee80211Primitives_m.h"
#include "NotifierConsts.h"
#include "InterfaceTableAccess.h"



Define_Module(Ieee80211AgentSTAExtended);

#define MK_STARTUP  1

void Ieee80211AgentSTAExtended::initialize(int stage)
{
    if (stage==0)
    {
        // read parameters
        activeScan = par("activeScan");
        probeDelay = par("probeDelay");
        minChannelTime = par("minChannelTime");
        maxChannelTime = par("maxChannelTime");
        authenticationTimeout = par("authenticationTimeout");
        associationTimeout = par("associationTimeout");
        // JcM add: agent starting time
        startingTime = par("startingTime");

        // JcM add: get the default ssid, if there is one.
        default_ssid = par("default_ssid").stringValue();

        cStringTokenizer tokenizer(par("channelsToScan"));
        const char *token;
        while ((token = tokenizer.nextToken())!=NULL)
            channelsToScan.push_back(atoi(token));

        NotificationBoard *nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_L2_BEACON_LOST);

        // JcM add: allow to disable the agent
        // startingTime > 0 to engage the agent
        if (startingTime!=0)
        {
            // JcM Fix: start up: send scan request according the starting time
            scheduleAt(simTime()+startingTime, new cMessage("startUp", MK_STARTUP));
        }
        else
        {
            EV << "Agent Disabled" << endl;
        }

    }
    else if (stage==1)
    {
        // JcM Add: Obtain our MAC Address from the InterfaceTable
        std::string ifname;
        if (this->gate("mgmtOut")->isConnected())
        {
            cModule* wlan_module = this->gate("mgmtOut")->getNextGate()->getOwnerModule();
            //TODO:Check this method considering multiples radios
            ifname = wlan_module->getFullName();
        }
        else
        {
            error("Agent is not Connected");
        }

        myEntry = NULL;
        IInterfaceTable *ift = InterfaceTableAccess().getIfExists();
        if (ift)
        {
            for (int i = 0; i < ift->getNumInterfaces(); i++)
            {
                if (ift->getInterface(i)->getName()==ifname)
                {
                    myEntry = ift->getInterface(i);
                    EV << "Interface Entry: " << myEntry->getFullName() << endl;
                }
            }
        }
        else
        {
            EV << "There is no interface Table to get the interface MAC Address" << endl;
        }
    }
}

void Ieee80211AgentSTAExtended::handleMessage(cMessage *msg)
{
    if (msg->isSelfMessage())
    {
        this->handleTimer(msg);
    }
    else
        this->handleResponse(msg);
}

void Ieee80211AgentSTAExtended::handleTimer(cMessage *msg)
{
    if (msg->getKind()==MK_STARTUP)
    {
        EV << "Starting up\n";
        sendScanRequest();
        delete msg;
    }
}

void Ieee80211AgentSTAExtended::handleResponse(cMessage *msg)
{
    cPolymorphic *ctrl = msg->removeControlInfo();
    delete msg;

    EV << "Processing confirmation from mgmt: " << ctrl->getClassName() << "\n";

    if (dynamic_cast<Ieee80211Prim_ScanConfirm *>(ctrl))
        processScanConfirm((Ieee80211Prim_ScanConfirm *)ctrl);
    else if (dynamic_cast<Ieee80211Prim_AuthenticateConfirm *>(ctrl))
        processAuthenticateConfirm((Ieee80211Prim_AuthenticateConfirm *)ctrl);
    else if (dynamic_cast<Ieee80211Prim_AssociateConfirm *>(ctrl))
        processAssociateConfirm((Ieee80211Prim_AssociateConfirm *)ctrl);
    else if (dynamic_cast<Ieee80211Prim_ReassociateConfirm *>(ctrl))
        processReassociateConfirm((Ieee80211Prim_ReassociateConfirm *)ctrl);
    else if (ctrl)
        error("handleResponse(): unrecognized control info class `%s'", ctrl->getClassName());
    else
        error("handleResponse(): control info is NULL");
    delete ctrl;
}

void Ieee80211AgentSTAExtended::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (category == NF_L2_BEACON_LOST)
    {
        if ((cPolymorphic *)myEntry != details)
            return;
        //XXX should check details if it's about this NIC
        EV << "beacon lost, starting scanning again\n";
        getParentModule()->getParentModule()->bubble("Beacon lost!");
        // TODO: track the associated BSSID in order to send the dissasociate request
        //sendDisassociateRequest();
        sendScanRequest();
    }
}

void Ieee80211AgentSTAExtended::sendRequest(Ieee80211PrimRequest *req)
{
    cMessage *msg = new cMessage(req->getClassName());
    msg->setControlInfo(req);
    send(msg, "mgmtOut");
}

void Ieee80211AgentSTAExtended::sendScanRequest()
{
    EV << "Sending ScanRequest primitive to mgmt\n";
    Ieee80211Prim_ScanRequest *req = new Ieee80211Prim_ScanRequest();
    req->setBSSType(BSSTYPE_INFRASTRUCTURE);
    req->setActiveScan(activeScan);
    req->setProbeDelay(probeDelay);
    req->setMinChannelTime(minChannelTime);
    req->setMaxChannelTime(maxChannelTime);
    req->setChannelListArraySize(channelsToScan.size());
    for (int i=0; i<(int)channelsToScan.size(); i++)
        req->setChannelList(i, channelsToScan[i]);
    //XXX BSSID, SSID are left at default ("any")

    sendRequest(req);
}

void Ieee80211AgentSTAExtended::sendAuthenticateRequest(const MACAddress& address)
{
    EV << "Sending AuthenticateRequest primitive to mgmt\n";
    Ieee80211Prim_AuthenticateRequest *req = new Ieee80211Prim_AuthenticateRequest();
    req->setAddress(address);
    req->setTimeout(authenticationTimeout);
    sendRequest(req);
}

void Ieee80211AgentSTAExtended::sendDeauthenticateRequest(const MACAddress& address, int reasonCode)
{
    EV << "Sending DeauthenticateRequest primitive to mgmt\n";
    Ieee80211Prim_DeauthenticateRequest *req = new Ieee80211Prim_DeauthenticateRequest();
    req->setAddress(address);
    req->setReasonCode(reasonCode);
    sendRequest(req);
}

void Ieee80211AgentSTAExtended::sendAssociateRequest(const MACAddress& address)
{
    EV << "Sending AssociateRequest primitive to mgmt\n";
    Ieee80211Prim_AssociateRequest *req = new Ieee80211Prim_AssociateRequest();
    req->setAddress(address);
    req->setTimeout(associationTimeout);
    sendRequest(req);
}

void Ieee80211AgentSTAExtended::sendReassociateRequest(const MACAddress& address)
{
    EV << "Sending ReassociateRequest primitive to mgmt\n";
    Ieee80211Prim_ReassociateRequest *req = new Ieee80211Prim_ReassociateRequest();
    req->setAddress(address);
    req->setTimeout(associationTimeout);
    sendRequest(req);
}

void Ieee80211AgentSTAExtended::sendDisassociateRequest(const MACAddress& address, int reasonCode)
{
    EV << "Sending DisassociateRequest primitive to mgmt\n";
    Ieee80211Prim_DisassociateRequest *req = new Ieee80211Prim_DisassociateRequest();
    req->setAddress(address);
    req->setReasonCode(reasonCode);
    sendRequest(req);
}

void Ieee80211AgentSTAExtended::processScanConfirm(Ieee80211Prim_ScanConfirm *resp)
{
    dumpAPList(resp);

    int bssIndex = -1;
    // choose best AP
    if (this->default_ssid=="")
    {
        // no default ssid, so pick the best one
        bssIndex = chooseBSS(resp);
    }
    else
    {
        // search if the default_ssid is in the list, otherwise
        // keep searching.

        for (int i=0; i<(int)resp->getBssListArraySize(); i++)
        {
            std::string resp_ssid = resp->getBssList(i).getSSID();
            if (resp_ssid == this->default_ssid)
            {
                EV << "found default SSID " << resp_ssid << endl;
                bssIndex = i;
                break;
            }
        }
    }

    if (bssIndex==-1)
    {
        EV << "No (suitable) AP found, continue scanning\n";
        sendScanRequest();
        return;
    }

    Ieee80211Prim_BSSDescription& bssDesc = resp->getBssList(bssIndex);
    EV << "Chosen AP address=" << bssDesc.getBSSID() << " from list, starting authentication\n";
    sendAuthenticateRequest(bssDesc.getBSSID());
}

void Ieee80211AgentSTAExtended::dumpAPList(Ieee80211Prim_ScanConfirm *resp)
{
    EV << "Received AP list:\n";
    for (int i=0; i<(int)resp->getBssListArraySize(); i++)
    {
        Ieee80211Prim_BSSDescription& bssDesc = resp->getBssList(i);
        EV << "    " << i << ". "
        << " address=" << bssDesc.getBSSID()
        << " channel=" << bssDesc.getChannelNumber()
        << " SSID=" << bssDesc.getSSID()
        << " beaconIntvl=" << bssDesc.getBeaconInterval()
        << " rxPower=" << bssDesc.getRxPower()
        << endl;
        // later: supportedRates
    }
}

int Ieee80211AgentSTAExtended::chooseBSS(Ieee80211Prim_ScanConfirm *resp)
{
    if (resp->getBssListArraySize()==0)
        return -1;

    // here, just choose the one with the greatest receive power
    // TODO and which supports a good data rate we support
    int bestIndex = 0;
    for (int i=0; i<(int)resp->getBssListArraySize(); i++)
        if (resp->getBssList(i).getRxPower() > resp->getBssList(bestIndex).getRxPower())
            bestIndex = i;
    return bestIndex;
}

void Ieee80211AgentSTAExtended::processAuthenticateConfirm(Ieee80211Prim_AuthenticateConfirm *resp)
{
    if (resp->getResultCode()!=PRC_SUCCESS)
    {
        EV << "Authentication error\n";

        // try scanning again, maybe we'll have better luck next time, possibly with a different AP
        EV << "Going back to scanning\n";
        sendScanRequest();
    }
    else
    {
        EV << "Authentication successful, let's try to associate\n";
        sendAssociateRequest(resp->getAddress());
    }
}

void Ieee80211AgentSTAExtended::processAssociateConfirm(Ieee80211Prim_AssociateConfirm *resp)
{
    if (resp->getResultCode()!=PRC_SUCCESS)
    {
        EV << "Association error\n";

        // try scanning again, maybe we'll have better luck next time, possibly with a different AP
        EV << "Going back to scanning\n";
        sendScanRequest();
    }
    else
    {
        EV << "Association successful\n";
        // we are happy!
        getParentModule()->getParentModule()->bubble("Associated with AP");

        // notify the association
    }
}

void Ieee80211AgentSTAExtended::processReassociateConfirm(Ieee80211Prim_ReassociateConfirm *resp)
{
    // treat the same way as AssociateConfirm
    if (resp->getResultCode()!=PRC_SUCCESS)
    {
        EV << "Reassociation error\n";
        EV << "Going back to scanning\n";
        sendScanRequest();
    }
    else
    {
        EV << "Reassociation successful\n";
        // we are happy!
    }
}

