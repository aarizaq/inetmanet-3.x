//
// Copyright (C) 2009 Juan-Carlos Maureira
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//


#include "Ieee80211MgmtWDSExtended.h"
#include "Ieee802Ctrl_m.h"
#include "EtherFrame_m.h"
#include "NotifierConsts.h"
#include "RadioState.h"

#include "IPv4.h"

Define_Module(Ieee80211MgmtWDSExtended);

static std::ostream& operator<< (std::ostream& os, const Ieee80211MgmtWDSExtended::WDSClientInfo& sta)
{
    os << "state:" << sta.status;
    return os;
}

void Ieee80211MgmtWDSExtended::initialize(int stage)
{
    Ieee80211MgmtBaseExtended::initialize(stage);

    if (stage==0)
    {
        // read params and init vars
        ssid = par("ssid").stringValue();

        channelNumber = -1;  // value will arrive from physical layer in receiveChangeNotification()

        WATCH(ssid);
        WATCH(channelNumber);

        WATCH_MAP(wdsList);

        // subscribe for notifications
        NotificationBoard *nb = NotificationBoardAccess().get();
        nb->subscribe(this, NF_RADIO_CHANNEL_CHANGED);

    }
    if (stage==1)
    {
        // parse the WDS Client parameter

        cStringTokenizer wdsTokens(par("WDSClients"),",");
        const char* token = NULL;
        while ((token = wdsTokens.nextToken())!=NULL)
        {
            EV << "WDS Client: " << token;
            // chekc if the wds client is a MAC module path
            cModule* wds_client = simulation.getModuleByPath(token);
            MACAddress wds_client_mac;
            if (wds_client!=NULL)
            {
                // the module exists. let's see if we can get an MAC Address from the module.
                cModule* wds_client_mac_module = wds_client->getSubmodule("mac");

                EV << wds_client_mac_module  << endl;

                if (wds_client_mac_module != NULL)
                {
                    if (wds_client_mac_module->hasPar("address"))
                    {
                        wds_client_mac = MACAddress(wds_client_mac_module->par("address").stringValue());
                        EV << " MAC Address: " << wds_client_mac << endl;
                    }
                    else
                    {
                        error("WDSClient module does not provides a MAC address ");
                    }
                }
                else
                {
                    error("WDSClient module is not an wlan interface (does not contain a MAC module");
                }
            }
            else
            {
                // module does not exists. let's try if the token is a MAC Address
                wds_client_mac = MACAddress(token);
                EV << " MAC Address: " << wds_client_mac << endl;

            }
            WDSClientInfo* wds_client_info = &(this->wdsList[wds_client_mac]);
            wds_client_info->address = wds_client_mac;
            // TODO: Authenticate the WDS client
            // by now, we assume the client is already connected
            wds_client_info->status = CONNECTED;
        }
    }
}

void Ieee80211MgmtWDSExtended::handleTimer(cMessage *msg)
{
    EV << "Timer Arrived" << msg << endl;
}

cPacket* Ieee80211MgmtWDSExtended::decapsulate(Ieee80211DataFrame *frame)
{
    if (!frame->getToDS() && !frame->getFromDS())
    {
        // frame is flagged as a 4 address frame.
        EV << "Incoming frame is a 4 address frame. decapsulating it" << endl;

        cPacket *payload = frame->decapsulate();

        Ieee802Ctrl *ctrl = new Ieee802Ctrl();
        ctrl->setSrc(frame->getAddress4());
        ctrl->setDest(frame->getAddress3());

        payload->setControlInfo(ctrl);

        delete frame;
        return payload;
    }
    else
    {
        EV << "Incoming frame is not a 4 address frame. ignoring it" << endl;
        // frame is not a 4 address frame. so. ignore it
        delete(frame);
    }
    return NULL;
}

Ieee80211DataFrame* Ieee80211MgmtWDSExtended::encapsulate(cPacket* msg)
{
    Ieee80211DataFrame *frame = new Ieee80211DataFrame(msg->getName());

    // frame is a 4 address data frame, so ToDS and FromDS are clear
    frame->setToDS(false);
    frame->setFromDS(false);

    // receiver is set in the deliver method

    // destination address is in address3
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    if (ctrl!=NULL)
    {
        frame->setAddress3(ctrl->getDest());
        // source address is in address4
        frame->setAddress4(ctrl->getSrc());
        delete ctrl;

        EV << "Encapsulating frame " <<  msg->getName() << " src:" << frame->getAddress4() << " dst:" << frame->getAddress3() << endl;

        frame->encapsulate(msg);
        return frame;
    }
    return NULL;
}

// Incoming message must come always decapsulated already.
// for relays units, use a EtherEncap module
void Ieee80211MgmtWDSExtended::handleUpperMessage(cPacket *msg)
{

    EV << "Handling upper message from Network Layer" << endl;

    // encapsulate the msg in a 4 address frame
    Ieee80211DataFrame *frame = this->encapsulate(msg);
    if (frame!=NULL)
    {
        // send the msg to each WDS client
        for (WDSClientList::iterator it = this->wdsList.begin(); it!=this->wdsList.end(); it++)
        {
            WDSClientInfo* wds_client_info = &(it->second);
            // if the wds client is connected, forward the frame
            if (wds_client_info->status == CONNECTED)
            {
                frame->setReceiverAddress(wds_client_info->address);
                sendOrEnqueue(frame->dup());
            }
        }
        // dispose the frame.
        delete(frame);
    }
    else
    {
        opp_error("Incoming packet has no Control Info data (src and dst)");
    }
}

void Ieee80211MgmtWDSExtended::receiveChangeNotification(int category, const cPolymorphic *details)
{
    Enter_Method_Silent();
    printNotificationBanner(category, details);

    if (category == NF_RADIO_CHANNEL_CHANGED)
    {
        RadioState* rs = check_and_cast<RadioState *>(details);

        cModule* radio = simulation.getModule(rs->getRadioId());
        if (radio!=NULL)
        {
            if (radio->getParentModule() == this->getParentModule())
            {
                // if the radio that generate the notification is contained in the same module of this mgmt, update the channel
                this->channelNumber = rs->getChannelNumber();
                EV << "updating channel number to " << channelNumber << endl;
            }
        }
    }
}

Ieee80211MgmtWDSExtended::WDSClientInfo *Ieee80211MgmtWDSExtended::lookupSenderWDSClient(MACAddress mac)
{
    WDSClientList::iterator it = wdsList.find(mac);
    return it==wdsList.end() ? NULL : &(it->second);
}

void Ieee80211MgmtWDSExtended::handleDataFrame(Ieee80211DataFrame *frame)
{
    cPacket* payload = this->decapsulate(frame);
    if (payload!=NULL)
    {
        send(payload,"upperLayerOut");
    }
}

