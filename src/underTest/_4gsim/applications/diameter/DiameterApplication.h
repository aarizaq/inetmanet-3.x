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

#ifndef DIAMETERAPPLICATION_H_
#define DIAMETERAPPLICATION_H_

#define TGPP	10415
#define S6a		16777251

/*
 * Class for holding info about Diameter application. At the moment it only holds
 * application id and vendor id for S6a application. Maybe it should be moved in
 * peer class...
 */
class DiameterApplication {
public:
	unsigned applId;
	unsigned vendorId;
public:
	DiameterApplication(unsigned applId, unsigned vendorId);
	virtual ~DiameterApplication();

};

#endif /* DIAMETERAPPLICATION_H_ */
