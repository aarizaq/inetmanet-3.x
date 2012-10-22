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

#ifndef GTPMESSAGE_H_
#define GTPMESSAGE_H_

#include "GTPMessage_m.h"
#include "INETDefs.h"

/*
 * Class for GTP message. This class inherits the message base class from .msg file
 * and adds the vector with the GTP information elements.
 */
class GTPMessage : public GTPMessage_Base {
private:
	typedef std::vector<GTPInfoElemPtr> GTPInfoElems;
	GTPInfoElems ies;
public:
	GTPMessage(const char *name=NULL, int kind=0) : GTPMessage_Base(name,kind) {}
	GTPMessage(const GTPMessage& other) : GTPMessage_Base(other.getName()) {operator=(other);}
	virtual ~GTPMessage();

	GTPMessage& operator=(const GTPMessage& other);
	virtual GTPMessage *dup() const {return new GTPMessage(*this);}

    /*
     * Methods overridden but not used. You should use instead pushIe.
     */
    virtual void setIesArraySize(unsigned int size);
    virtual void setIes(unsigned int k, const GTPInfoElemPtr& ies_var);

    /*
     * Getter methods.
     */
    virtual unsigned int getIesArraySize() const;
    virtual GTPInfoElemPtr& getIes(unsigned int k);

    /*
     * Method for pushing GTP information element at the end of the vector.
     */
	void pushIe(GTPInfoElemPtr ie) { ies.push_back(ie); }

    /*
     * Method for printing the message contents. Currently it prints info
     * only for the header.
     */
	void print();

    /*
     * Method for finding a GTP IE with a given GTP IE type and instance. It returns
     * NULL, if the GTP IE is not found.
     */
	GTPInfoElemPtr findIe(unsigned char type, unsigned char instance);

    /*
     * Method for finding a range of GTP IEs with a given GTP IE type and instance.
     */
	std::vector<GTPInfoElemPtr> findIes(unsigned char type, unsigned char instance);
};

#endif /* GTPMESSAGE_H_ */
