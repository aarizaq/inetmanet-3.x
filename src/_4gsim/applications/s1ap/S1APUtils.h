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

#ifndef S1APUTILS_H_
#define S1APUTILS_H_

#include <vector>
#include <omnetpp.h>

#define MMECODE_CODED_SIZE	2
#define GROUPID_CODED_SIZE	4

/*
 * Utility class for storing information about SupportedTa's. Each item will
 * be read, during the initialization phase of the module, from the configuration
 * file and will be stored in a vector in the module class. This information
 * is used mainly during the establishment of S1AP connections.
 */
class SupportedTaItem {
public:
	char *tac;
	std::vector<char*> bplmns;
	SupportedTaItem();
	SupportedTaItem(const SupportedTaItem& other) { operator=(other);}
	virtual ~SupportedTaItem();

	SupportedTaItem& operator=(const SupportedTaItem& other);
};

/*
 * Utility class for storing information about ServedGummei's. Each item will
 * be read, during the initialization phase of the module, from the configuration
 * file and will be stored in a vector in the module class. This information
 * is used mainly during the establishment of S1AP connections.
 */
class ServedGummeiItem {
public:
	std::vector<char*> servPlmns;
	std::vector<char*> servGrIds;
	std::vector<char*> servMmecs;
	ServedGummeiItem() {}
	ServedGummeiItem(const ServedGummeiItem& other) { operator=(other);}
	virtual ~ServedGummeiItem();

	ServedGummeiItem& operator=(const ServedGummeiItem& other);
};

class S1APUtils {
public:
	S1APUtils();
	virtual ~S1APUtils();

};

#endif /* S1APUTILS_H_ */
