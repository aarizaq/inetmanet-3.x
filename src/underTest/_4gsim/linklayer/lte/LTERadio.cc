//
// Copyright (C) 2012 Calin Cerchez
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "LTERadio.h"
#include "PhyControlInfo_m.h"
#include "InterfaceTableAccess.h"
#include "IPv4InterfaceData.h"
Define_Module(LTERadio);

LTERadio::LTERadio() : rs(this->getId()) {
    radioModel = NULL;
    receptionModel = NULL;
}

LTERadio::~LTERadio() {
    delete radioModel;
    delete receptionModel;
}

void LTERadio::initialize(int stage) {

    ChannelAccess::initialize(stage);

    EV << "Initializing LTERadio, stage=" << stage << endl;

    if (stage == 0) {
    	rs.setChannelNumber((int)par("channelNumber"));
    	rs.setState(RadioState::IDLE);

        std::string rModel = par("radioModel").stdstringValue();
        if (rModel=="")
            rModel = "LTERadioModel";

        radioModel = (IRadioModel *) createOne(rModel.c_str());
        radioModel->initializeFrom(this);

        obstacles = ObstacleControlAccess().getIfExists();
        if (obstacles) EV << "Found ObstacleControl" << endl;

        // this is the parameter of the channel controller (global)
        std::string propModel = getChannelControlPar("propagationModel").stdstringValue();
        if (propModel == "")
            propModel = "FreeSpaceModel";

        receptionModel = (IReceptionModel *) createOne(propModel.c_str());
        receptionModel->initializeFrom(this);

    } else if (stage == 2)
        cc->setRadioChannel(myRadioRef, rs.getChannelNumber());

    else  if(stage == 4)
	{
    	if(rs.getChannelNumber()!=10)
    	{
    	    ift = InterfaceTableAccess().get();
    	    InterfaceEntry *entry = new InterfaceEntry(this);
    	    IPv4InterfaceData *ipv4data = new IPv4InterfaceData();
    	    entry->setIPv4Data(ipv4data);
    	    entry->setName("UERadioInterface");
    	    entry->setMACAddress(MACAddress::generateAutoAddress());
    	    ift->addInterface(entry);
    	}
	}


}

void LTERadio::handleMessage(cMessage *msg) {

    	if(msg->arrivedOn("radioIn"))
    	{
    		// must be an AirFrame
        AirFrame *airframe = (AirFrame*)msg;
        handleLowerMsg(airframe);

    	}
    	else
    	{
    	        AirFrame *airframe = encapsulatePacket(PK(msg));
    	        handleUpperMsg(airframe);
    	}

}

AirFrame *LTERadio::encapsulatePacket(cPacket *frame) {
   PhyControlInfo *ctrl = dynamic_cast<PhyControlInfo *>(frame->removeControlInfo());

    // Note: we don't set length() of the AirFrame, because duration will be used everywhere instead
    AirFrame *airframe = new AirFrame();
    airframe->setName(frame->getName());
//    airframe->setPSend(transmitterPower);
    if (ctrl == NULL) {
    	airframe->setChannelNumber(rs.getChannelNumber());
    } else {
    	airframe->setChannelNumber(ctrl->getChannelNumber());
    	delete ctrl;
    }
    airframe->encapsulate(frame);
//    airframe->setBitrate(ctrl ? ctrl->getBitrate() : rs.getBitrate());
//    airframe->setDuration(radioModel->calculateDuration(airframe));
//    airframe->setSenderPos(getMyPosition());


//    EV << "Frame (" << frame->getClassName() << ")" << frame->getName()
//       << " will be transmitted at " << (airframe->getBitrate()/1e6) << "Mbps\n";
    return airframe;
}

void LTERadio::handleUpperMsg(AirFrame* airframe) {
    // change radio status
//    EV << "sending, changing RadioState to TRANSMIT\n";
//    rs.setState(RadioState::TRANSMIT);
if(airframe->arrivedOn("netIn"))
	airframe->setKind(user);
else
	airframe->setKind(control);
    sendDown(airframe);
}

void LTERadio::handleLowerMsg(AirFrame *airframe) {
	EV << "receiving frame " << airframe->getName() << endl;
	EV << "reception of frame over, preparing to send packet to upper layer\n";
	sendUp(airframe);
}

void LTERadio::sendUp(AirFrame *airframe) {
    cPacket *frame = airframe->decapsulate();
    PhyControlInfo *ctrl = new PhyControlInfo();
    ctrl->setChannelNumber(airframe->getChannelNumber());
    frame->setControlInfo(ctrl);

    EV << "sending up frame " << frame->getName() << endl;
    if(airframe->getKind()==control)
    	send(frame, gate("uppergateOut"));
    else
    	send(frame,gate("netOut"));
    delete airframe;
}

void LTERadio::sendDown(AirFrame *airframe) {
	sendToChannel(airframe);
}
