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

#include "S1APPdu.h"
#include "PerEncoder.h"

const void *S1APProcedure::itemsInfo[1] = {
	&ProtocolIeContainer::theInfo,
};

bool S1APProcedure::itemsPres[1] = {
	1
};

const S1APProcedure::Info S1APProcedure::theInfo = {
	S1APProcedure::create,
	SEQUENCE,
	0,
	true,
	itemsInfo,
	itemsPres,
	1, 0, 0
};

S1APProcedure::S1APProcedure(ProtocolIeContainer *container) : Sequence(&theInfo) {
	if (container != NULL)
		items.at(0) = container;
}

const void *S1APPdu::choicesInfo[3] = {
	&InitiatingMessage::theInfo,
	&SuccessfulOutcome::theInfo,
	&UnsuccessfulOutcome::theInfo
};

const S1APPdu::Info S1APPdu::theInfo = {
	Choice::create,
	CHOICE,
	0,
	true,
	choicesInfo,
	2
};

const void *InitiatingMessage::itemsInfo[3] = {
	&Integer<CONSTRAINED, 0, 255>::theInfo,
	&Enumerated<false, 2>::theInfo,
	&OpenType::theInfo
};

bool InitiatingMessage::itemsPres[3] = {
	1,
	1,
	1
};

const InitiatingMessage::Info InitiatingMessage::theInfo = {
	InitiatingMessage::create,
	SEQUENCE,
	0,
	false,
	itemsInfo,
	itemsPres,
	3, 0, 0
};

InitiatingMessage::InitiatingMessage(unsigned char code, unsigned char crit, AbstractType *val) : Sequence(&theInfo) {
	static_cast<IntegerBase*>(items.at(0))->setValue(code);
	static_cast<EnumeratedBase*>(items.at(1))->setValue(crit);
	setValue(new OpenType(val));
}

void InitiatingMessage::setValue(OpenType *val)  {
	static_cast<OpenType*>(items.at(2))->setLength(val->getLength());
	static_cast<OpenType*>(items.at(2))->setValue(val->getValue());
}
