/*
 * Copyright (C) 2003 Andras Varga; CTIE, Monash University, Australia
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include "EtherEncapMesh.h"
#include "EtherFrame_m.h"
#include "Ieee802Ctrl_m.h"
#include "IInterfaceTable.h"
#include "InterfaceTableAccess.h"
#include "EtherMAC.h"
#include "Ieee80211MgmtFrames_m.h"


Define_Module(EtherEncapMesh);

void EtherEncapMesh::initialize()
{
    EtherEncap::initialize();
    totalFromWifi =totalToWifi=0;
    WATCH(totalFromWifi);
}

void EtherEncapMesh::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("lowerLayerIn"))
    {
        processFrameFromMAC(check_and_cast<EtherFrame *>(msg));
    }
    else if (msg->arrivedOn("wifiMeshIn"))
    {
        processFrameFromWifiMesh(check_and_cast<Ieee80211Frame *>(msg));
    }
    else
    {
        // from higher layer
        switch(msg->getKind())
        {
            case IEEE802CTRL_DATA:
            case 0: // default message kind (0) is also accepted
              processPacketFromHigherLayer(PK(msg));
              break;

            case IEEE802CTRL_SENDPAUSE:
              // higher layer want MAC to send PAUSE frame
              handleSendPause(msg);
              break;

            default:
              error("received message `%s' with unknown message kind %d", msg->getName(), msg->getKind());
        }
    }

    if (ev.isGUI())
        updateDisplayString();
}

void EtherEncapMesh::updateDisplayString()
{
    char buf[80];
    sprintf(buf, "passed up: %ld\nsent: %ld \ntotal rec: %ld\ntotal from wifi: %ld", totalFromMAC, totalFromHigherLayer,totalFromMAC+totalToWifi,totalFromWifi);
    getDisplayString().setTagArg("t",0,buf);
}

void EtherEncapMesh::processFrameFromMAC(EtherFrame *frame)
{
    // decapsulate and attach control info
    cPacket *higherlayermsg = frame->decapsulate();
    Ieee80211MeshFrame *frameMesh=dynamic_cast<Ieee80211MeshFrame *>(higherlayermsg);
    if (frameMesh)
    {
       if (frameMesh->getRealLength()>0) // fragmented frame
       {
           // Fragment, is the last?
           EV << "reassemble wifi frame over ethernet " << endl;
           if (frameMesh->getIsFragment())
           {
               delete higherlayermsg;
               delete frame;
               return;
           }
       }
    }

    Ieee80211ActionHWMPFrame *frameAction = dynamic_cast<Ieee80211ActionHWMPFrame *>(higherlayermsg);
    if (frameAction)
    {
       if (frameAction->getRealLength()>0) // fragmented frame
       {
           // Fragment, is the last?
           EV << "reassemble wifi frame over ethernet " << endl;
           if (frameAction->getIsFragment())
           {
               delete higherlayermsg;
               delete frame;
               return;
           }
       }
    }
    // add Ieee802Ctrl to packet
    Ieee802Ctrl *etherctrl = new Ieee802Ctrl();
    etherctrl->setSrc(frame->getSrc());
    etherctrl->setDest(frame->getDest());

    //Stephan Janosch patch
    EthernetIIFrame * e2frame = dynamic_cast<EthernetIIFrame *>(frame);
    if (e2frame)
    	etherctrl->setEtherType(e2frame->getEtherType());
    // end Stephan Janosch pathc

    higherlayermsg->setControlInfo(etherctrl);

    // check if the frame is wifi
    if (frameMesh)
    {
       // check if is a fragment frame
       if (frameMesh->getRealLength()>0)
       {
    	   // set to real Length
       	   EV << "reassemble wifi frame over ethernet : last" << endl;
           higherlayermsg->setByteLength(frameMesh->getRealLength());
       }
       totalToWifi++;
       send(higherlayermsg,"wifiMeshOut");
    }
    else if (frameAction)
    {
       // check if is a fragment frame
       if (frameAction->getRealLength()>0)
       {
           // set to real Length
           EV << "reassemble wifi frame over ethernet : last" << endl;
           higherlayermsg->setByteLength(frameAction->getRealLength());
       }
       totalToWifi++;
       send(higherlayermsg,"wifiMeshOut");
    }
    else
    {
       EV << "Decapsulating frame `" << frame->getName() <<"', passing up contained "
    		   "packet `" << higherlayermsg->getName() << "' to higher layer\n";
       totalFromMAC++;
       // pass up to higher layers.
       send(higherlayermsg, "upperLayerOut");
    }
    delete frame;
}



void EtherEncapMesh::processFrameFromWifiMesh(Ieee80211Frame *msg)
{
// TODO: fragment packets before send to ethernet
	totalFromWifi++;
    if (msg->getByteLength() > MAX_ETHERNET_DATA_BYTES)
    {
        if (dynamic_cast<Ieee80211MeshFrame *>(msg))
        {
     // check if fragment
           EV << "Fragment wifi frame over ethernet" << endl;
           Ieee80211MeshFrame *frameMesh=dynamic_cast<Ieee80211MeshFrame *>(msg);
           frameMesh->setRealLength(msg->getByteLength());
           int numFrames = ceil((double)msg->getByteLength()/(double)MAX_ETHERNET_DATA_BYTES);
           uint64_t remain = msg->getByteLength();
           Ieee802Ctrl *etherctrl = check_and_cast<Ieee802Ctrl*>(msg->removeControlInfo());
           for (int i = 0; i < numFrames - 1; i++)
           {
               Ieee80211MeshFrame *frameAux = frameMesh->dup();
               frameAux->setByteLength(MAX_ETHERNET_DATA_BYTES);
               frameAux->setIsFragment(true);
               remain -= MAX_ETHERNET_DATA_BYTES;
               EtherFrame *frame = NULL;
               if (useSNAP)
               {
                   EtherFrameWithSNAP *snapFrame = new EtherFrameWithSNAP(msg->getName());
                   snapFrame->setSrc(etherctrl->getSrc()); // if blank, will be filled in by MAC
                   snapFrame->setDest(etherctrl->getDest());
                   snapFrame->setOrgCode(0);
                   snapFrame->setLocalcode(etherctrl->getEtherType());
                   snapFrame->setByteLength(ETHER_MAC_FRAME_BYTES + ETHER_LLC_HEADER_LENGTH + ETHER_SNAP_HEADER_LENGTH);
                   frame = snapFrame;
               }
               else
               {
                   EthernetIIFrame *eth2Frame = new EthernetIIFrame(msg->getName());
                   eth2Frame->setSrc(etherctrl->getSrc()); // if blank, will be filled in by MAC
                   eth2Frame->setDest(etherctrl->getDest());
                   eth2Frame->setEtherType(etherctrl->getEtherType());
                   eth2Frame->setByteLength(ETHER_MAC_FRAME_BYTES);
                   frame = eth2Frame;
               }
               frame->encapsulate(frameAux);
               if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
                    frame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
               send(frame, "lowerLayerOut");
           }



           if (remain>0 && remain<=MAX_ETHERNET_DATA_BYTES)
           {
               EtherFrame *frame = NULL;
               if (useSNAP)
               {
                   EtherFrameWithSNAP *snapFrame = new EtherFrameWithSNAP(msg->getName());
                   snapFrame->setSrc(etherctrl->getSrc()); // if blank, will be filled in by MAC
                   snapFrame->setDest(etherctrl->getDest());
                   snapFrame->setOrgCode(0);
                   snapFrame->setLocalcode(etherctrl->getEtherType());
                   snapFrame->setByteLength(ETHER_MAC_FRAME_BYTES + ETHER_LLC_HEADER_LENGTH + ETHER_SNAP_HEADER_LENGTH);
                   frame = snapFrame;
               }
               else
               {
                   EthernetIIFrame *eth2Frame = new EthernetIIFrame(msg->getName());
                   eth2Frame->setSrc(etherctrl->getSrc()); // if blank, will be filled in by MAC
                   eth2Frame->setDest(etherctrl->getDest());
                   eth2Frame->setEtherType(etherctrl->getEtherType());
                   eth2Frame->setByteLength(ETHER_MAC_FRAME_BYTES);
                   frame = eth2Frame;
               }
               msg->setByteLength(remain);
               frame->encapsulate(msg);
               if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
                   frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);  // "padding"
               send(frame, "lowerLayerOut");
               delete etherctrl;
               return;
           }
           else
               error("""Error in fragmentation");
        }
        else if (dynamic_cast<Ieee80211ActionHWMPFrame *>(msg) )
        {
     // check if fragment
           EV << "Fragment wifi frame over ethernet" << endl;
           Ieee80211ActionHWMPFrame *frameMesh=dynamic_cast<Ieee80211ActionHWMPFrame *>(msg);
           frameMesh->setRealLength(msg->getByteLength());
           int numFrames = ceil((double)msg->getByteLength()/(double)MAX_ETHERNET_DATA_BYTES);
           uint64_t remain = msg->getByteLength();
           Ieee802Ctrl *etherctrl = check_and_cast<Ieee802Ctrl*>(msg->removeControlInfo());
           for (int i = 0; i < numFrames - 1; i++)
           {
               Ieee80211ActionHWMPFrame *frameAux = frameMesh->dup();
               frameAux->setByteLength(MAX_ETHERNET_DATA_BYTES);
               frameAux->setIsFragment(true);
               remain -= MAX_ETHERNET_DATA_BYTES;
               EtherFrame *frame = NULL;
               if (useSNAP)
               {
                   EtherFrameWithSNAP *snapFrame = new EtherFrameWithSNAP(msg->getName());
                   snapFrame->setSrc(etherctrl->getSrc()); // if blank, will be filled in by MAC
                   snapFrame->setDest(etherctrl->getDest());
                   snapFrame->setOrgCode(0);
                   snapFrame->setLocalcode(etherctrl->getEtherType());
                   snapFrame->setByteLength(ETHER_MAC_FRAME_BYTES + ETHER_LLC_HEADER_LENGTH + ETHER_SNAP_HEADER_LENGTH);
                   frame = snapFrame;
               }
               else
               {
                   EthernetIIFrame *eth2Frame = new EthernetIIFrame(msg->getName());
                   eth2Frame->setSrc(etherctrl->getSrc()); // if blank, will be filled in by MAC
                   eth2Frame->setDest(etherctrl->getDest());
                   eth2Frame->setEtherType(etherctrl->getEtherType());
                   eth2Frame->setByteLength(ETHER_MAC_FRAME_BYTES);
                   frame = eth2Frame;
               }
               frame->encapsulate(frameAux);
               if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
                    frame->setByteLength(MIN_ETHERNET_FRAME_BYTES); // "padding"
               send(frame, "lowerLayerOut");
           }



           if (remain>0 && remain<=MAX_ETHERNET_DATA_BYTES)
           {
               EtherFrame *frame = NULL;
               if (useSNAP)
               {
                   EtherFrameWithSNAP *snapFrame = new EtherFrameWithSNAP(msg->getName());
                   snapFrame->setSrc(etherctrl->getSrc()); // if blank, will be filled in by MAC
                   snapFrame->setDest(etherctrl->getDest());
                   snapFrame->setOrgCode(0);
                   snapFrame->setLocalcode(etherctrl->getEtherType());
                   snapFrame->setByteLength(ETHER_MAC_FRAME_BYTES + ETHER_LLC_HEADER_LENGTH + ETHER_SNAP_HEADER_LENGTH);
                   frame = snapFrame;
               }
               else
               {
                   EthernetIIFrame *eth2Frame = new EthernetIIFrame(msg->getName());
                   eth2Frame->setSrc(etherctrl->getSrc()); // if blank, will be filled in by MAC
                   eth2Frame->setDest(etherctrl->getDest());
                   eth2Frame->setEtherType(etherctrl->getEtherType());
                   eth2Frame->setByteLength(ETHER_MAC_FRAME_BYTES);
                   frame = eth2Frame;
               }
               msg->setByteLength(remain);
               frame->encapsulate(msg);
               if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
                   frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);  // "padding"
               send(frame, "lowerLayerOut");
               delete etherctrl;
               return;
           }
           else
               error("""Error in fragmentation");
        }
        else
            error("packet from higher layer (%d bytes) exceeds maximum Ethernet payload length (%d)", (int)msg->getByteLength(), MAX_ETHERNET_DATA_BYTES);
    }

    // Creates MAC header information and encapsulates received higher layer data
    // with this information and transmits resultant frame to lower layer

    // create Ethernet frame, fill it in from Ieee802Ctrl and encapsulate msg in it
    EV << "Encapsulating higher layer packet `" << msg->getName() <<"' for MAC\n";

    Ieee802Ctrl *etherctrl = check_and_cast<Ieee802Ctrl*>(msg->removeControlInfo());
    EthernetIIFrame *frame = new EthernetIIFrame(msg->getName());

    frame->setSrc(etherctrl->getSrc());  // if blank, will be filled in by MAC
    frame->setDest(etherctrl->getDest());
    frame->setEtherType(etherctrl->getEtherType());
    frame->setByteLength(ETHER_MAC_FRAME_BYTES);
    delete etherctrl;

    frame->encapsulate(msg);
    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);  // "padding"

    send(frame, "lowerLayerOut");
}



