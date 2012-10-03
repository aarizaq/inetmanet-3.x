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

#ifndef GTPUSER_H_
#define GTPUSER_H_

#include "GTP.h"

class GTPUser : public GTP {
protected:

public:
	GTPUser();
	virtual ~GTPUser();

	virtual void initialize(int stage);

	/* notification board */
	virtual void receiveChangeNotification(int category, const cPolymorphic *details);
};

#endif /* GTPUSER_H_ */
