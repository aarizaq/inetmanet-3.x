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

#ifndef S1APCONTAINER_H_
#define S1APCONTAINER_H_

#include "ASNTypes.h"
#include "S1APConstant.h"
#include "PerDecoder.h"

class ProtocolIeField : public Sequence {
private:
	static const void *itemsInfo[3];
	static bool itemsPres[3];
public:
	static const Info theInfo;
	ProtocolIeField(unsigned short id, unsigned char crit, AbstractType *val);

	unsigned short getProtocolId() const { return static_cast<IntegerBase*>(items.at(0))->getValue(); }
	OpenType *getValue() const { return static_cast<OpenType*>(items.at(2)); }

	void setValue(OpenType *val);
};

typedef ProtocolIeField ProtocolIeSingleContainer;

typedef SequenceOf<ProtocolIeField, CONSTRAINED, 0, maxProtocolIEs> ProtocolIeContainer;
OpenType *findValue(ProtocolIeContainer *container, unsigned short id);

typedef ProtocolIeField ProtocolExtField;

typedef SequenceOf<ProtocolExtField, CONSTRAINED, 0, maxProtocolIEs> ProtocolExtContainer;

#endif /* S1APCONTAINER_H_ */

