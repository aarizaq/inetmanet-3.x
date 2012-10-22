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

#ifndef GTPPATHTABLE_H_
#define GTPPATHTABLE_H_

#include "GTPPath.h"

/*
 * Class for GTP path table. This table will hold all the path owned by the GTP model
 * implementation for both version, GTP control and GTP user. The paths for GTP
 * control and GTP user are stored in a single path table.
 */
class GTPPathTable : public cSimpleModule {
private:
	typedef std::vector<GTPPath*> GTPPaths;
	GTPPaths paths;
public:
	GTPPathTable();
	virtual ~GTPPathTable();

	/*
	 * Method for initializing a GTP path table.
	 */
    void initialize(int stage);

    /*
     * Method for finding a GTP path for a given IP address and path type. The method
     * returns the path, if it is found, or NULL otherwise.
     */
    GTPPath *findPath(IPvXAddress addr, unsigned char type);

    /*
     * Method for finding a GTP path for a given ctrl IP address. The method returns
     * the path, if it is found, or NULL otherwise.
     */
    GTPPath *findPath(IPvXAddress ctrlAddr);

    /*
     * Method for finding a GTP path for a given path type. The method returns
     * the path, if it is found, or NULL otherwise.
     */
    GTPPath *findPath(char type);

    /*
     * Method for finding a GTP path for a given message. The method returns
     * the path, if it is found, or NULL otherwise.
     */
    GTPPath *findPath(cMessage *msg);

    /*
     * Method for deleting a GTP path. The method calls first the destructor
     * for each path between start and end position and removes them afterwards.
     */
    void erase(unsigned start, unsigned end);

    /*
     * Wrapper methods.
     */
    void push_back(GTPPath *path) { paths.push_back(path); }
    unsigned size() { return paths.size(); }
    GTPPath *at(unsigned i) { return paths[i]; }

};

#endif /* GTPPATHTABLE_H_ */
