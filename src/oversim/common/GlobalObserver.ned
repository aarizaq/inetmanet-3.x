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
// global module that contains the globalNodeList and the globalStatistics module
//
module GlobalObserver
{
    parameters:
        int numGlobalFunctions;
        @display("i=block/control");

    submodules:
        globalNodeList: GlobalNodeList {
            parameters:
                @display("p=60,60;i=block/control");
        }
        globalParameters: GlobalParameters {
            parameters:
                @display("p=180,180;i=block/control");
        }
        globalTraceManager: GlobalTraceManager {
            parameters:
                @display("p=60,180;i=block/control");
        }
        globalFunctions[numGlobalFunctions]: GlobalFunctions {
            parameters:
                @display("p=60,300,column;i=block/control");
        }
        globalStatistics: GlobalStatistics {
            parameters:
                @display("p=180,60;i=block/control");
        }
}

module GlobalFunctions
{
    parameters:
        string functionType; // type of globalFunctions
    submodules:
        function: <functionType> like IGlobalFunctions {
            parameters:
                @display("i=block/control");
        }
}
