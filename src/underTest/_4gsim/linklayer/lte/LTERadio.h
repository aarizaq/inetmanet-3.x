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

#ifndef LTERADIO_H_
#define LTERADIO_H_

#include "ChannelAccess.h"
#include "AirFrame_m.h"
#include "IRadioModel.h"
#include "IReceptionModel.h"
#include "IInterfaceTable.h"
#include "RadioState.h"
#include "ObstacleControl.h"
enum kind{
	control = 0,
	user = 1
};

class LTERadio : public ChannelAccess {
public:
	LTERadio();
	virtual ~LTERadio();
protected:
	RadioState rs;

    IInterfaceTable *ift;

    ObstacleControl* obstacles;
    IRadioModel *radioModel;
    IReceptionModel *receptionModel;


	virtual IReceptionModel *createReceptionModel() {return (IReceptionModel *)createOne("PathLossReceptionModel");}
    virtual IRadioModel *createRadioModel() {return (IRadioModel *)createOne("LTERadioModel");}
	virtual int numInitStages() const {
		return 5;
	}
    virtual void initialize(int stage);
    virtual void handleMessage(cMessage *msg);

    virtual void handleUpperMsg(AirFrame*);
    virtual void handleLowerMsg(AirFrame *airframe);

    virtual AirFrame *encapsulatePacket(cPacket *frame);

    virtual void sendDown(AirFrame *airframe);
    virtual void sendUp(AirFrame *airframe);
};

#endif /* LTERADIO_H_ */
