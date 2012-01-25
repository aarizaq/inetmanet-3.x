//
// Copyright (C) 2008 Institut fuer Telematik, Universitaet Karlsruhe (TH)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
     
package inet.oversim.common;

//
// The BootstrapList module maintains a list of bootstrap node
// candidates received from various sources (GlobalNodeList for
// simulations and Zeroconf for SingleHostUnderlay). This list is also
// used to detect overlay partitions and triggers the merging process.
//
// @author Bin Zheng, Ingmar Baumgart
// 
simple BootstrapList extends BaseApp
{
    parameters:
        @class(BootstrapList);
        @display("i=block/table");        
        bool mergeOverlayPartitions;  // if true, detect and merge overlay partitions
        bool maintainList; //maintain a list of bootstrap candidates and check them periodically
}
