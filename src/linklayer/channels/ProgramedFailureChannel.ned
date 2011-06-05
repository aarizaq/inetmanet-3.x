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

package inet.linklayer.channels;

channel ProgramedFailureDataRateChannel extends ned.DatarateChannel
{
    @class(ProgramedFailureDataRateChannel);
    string failureAt = default("");  // comma separated list of simulation times to make the link fails
    string recoveryAt = default("");  // comma separated list of simulation times to make the link recovers
}

channel ProgramedFailureChannel extends ned.DelayChannel
{
    @class(ProgramedFailureChannel);
    string failureAt = default("");  // comma separated list of simulation times to make the link fails
    string recoveryAt = default("");  // comma separated list of simulation times to make the link recovers
}