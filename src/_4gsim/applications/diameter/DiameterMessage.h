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

#ifndef DIAMETERMESSAGE_H_
#define DIAMETERMESSAGE_H_

#include "INETDefs.h"
#include "DiameterMessage_m.h"

/*
 * Class for Diameter message. This class inherits the message base class from
 * .msg file and adds the vector with the AVPs.
 */
class DiameterMessage : public DiameterMessage_Base {
protected:
	typedef std::vector<AVPPtr> AVPVector;
	AVPVector avps;
public:
	DiameterMessage(const char *name=NULL, int kind=0) : DiameterMessage_Base(name,kind) {}
	DiameterMessage(const DiameterMessage& other) : DiameterMessage_Base(other.getName()) {operator=(other);}
	virtual ~DiameterMessage();

	DiameterMessage& operator=(const DiameterMessage& other);
	virtual DiameterMessage *dup() const {return new DiameterMessage(*this);}

	/*
	 * Methods overridden but not used. You should use instead pushAvp or insertAvp.
	 */
    virtual void setAvpsArraySize(unsigned int size);
    virtual void setAvps(unsigned int k, const AVPPtr& avps_var);

    /*
     * Getter methods.
     */
    virtual unsigned int getAvpsArraySize() const;
    virtual AVPPtr& getAvps(unsigned int k);

    /*
     * Method for pushing AVP at the end of the vector.
     */
	void pushAvp(AVPPtr avp) { avps.push_back(avp); }

	/*
	 * Method for inserting AVP at a given position in the vector.
	 */
	void insertAvp(unsigned pos, AVPPtr avp);

	/*
	 * Method for printing the message contents. Currently it prints info
	 * only for the header.
	 */
	void print();

	/*
	 * Method for finding a AVP with a given AVP code within the vector. It returns
	 * NULL, if the AVP is not found.
	 */
	AVPPtr findAvp(unsigned avpCode);
};

#endif /* DIAMETERMESSAGE_H_ */
