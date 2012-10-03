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

#include "S1APContainer.h"
#include "PerEncoder.h"

const void *ProtocolIeField::itemsInfo[3] = {
	&Integer<CONSTRAINED, 0, 65535>::theInfo,
	&Enumerated<false, 2>::theInfo,
	&OpenType::theInfo
};

bool ProtocolIeField::itemsPres[3] = {
	1,
	1,
	1
};

const ProtocolIeField::Info ProtocolIeField::theInfo = {
	ProtocolIeField::create,
	SEQUENCE,
	0,
	false,
	itemsInfo,
	itemsPres,
	3, 0, 0
};

ProtocolIeField::ProtocolIeField(unsigned short id, unsigned char crit, AbstractType *val) : Sequence(&theInfo) {
	static_cast<IntegerBase*>(items.at(0))->setValue(id);
	static_cast<EnumeratedBase*>(items.at(1))->setValue(crit);
	setValue(new OpenType(val));
}

OpenType *findValue(ProtocolIeContainer *container, unsigned short id) {
	for (int64_t i = 0; i < container->size(); i++) {
		unsigned short protId = static_cast<const ProtocolIeField>(container->at(i)).getProtocolId();
		if (protId == id)
			return static_cast<const ProtocolIeField>(container->at(i)).getValue();
	}
	return NULL;
}

void ProtocolIeField::setValue(OpenType *val)  {
	static_cast<OpenType*>(items.at(2))->setLength(val->getLength());
	static_cast<OpenType*>(items.at(2))->setValue(val->getValue());
}
