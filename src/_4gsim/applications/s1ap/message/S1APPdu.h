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

#ifndef S1APPDU_H_
#define S1APPDU_H_

#include "S1APContainer.h"
#include "S1APIe.h"
#include "PerDecoder.h"

enum PresenceValues {
	optional 	= 0,
	conditional = 1,
	mandatory	= 2
};

enum CriticalityValues {
	reject = 0,
	ignore = 1,
	notify = 2
};

class S1APProcedure : public Sequence {
private:
	static const void *itemsInfo[1];
	static bool itemsPres[1];
public:
	static const Info theInfo;
	S1APProcedure(ProtocolIeContainer *container = NULL);

	ProtocolIeContainer *getContainer() { return static_cast<ProtocolIeContainer*>(items.at(0)); }
};

enum S1APPduChoices {
	initiatingMessage = 0,
	successfulOutcome = 1,
	unsuccessfulOutcome = 2
};

class S1APPdu : public Choice {
private:
	static const void *choicesInfo[3];
public:
	static const Info theInfo;
	S1APPdu() : Choice(&theInfo) {}

};

class InitiatingMessage : public Sequence {
private:
	static const void *itemsInfo[3];
	static bool itemsPres[3];
public:
	static const Info theInfo;
	InitiatingMessage(unsigned char code, unsigned char crit, AbstractType *val);

	unsigned char getProcedureCode() { return static_cast<IntegerBase*>(items.at(0))->getValue(); }
	OpenType *getValue() { return static_cast<OpenType*>(items.at(2)); }

	void setValue(OpenType *val);
};

typedef InitiatingMessage SuccessfulOutcome;

typedef InitiatingMessage UnsuccessfulOutcome;

#endif /* S1APPDU_H_ */
