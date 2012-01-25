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

package inet.oversim.underlay.inetunderlay;

//
// Access net connects participating hosts over an access router with the backbone
//
simple AccessNet
{
    parameters:
        @display("i=block/cogwheel");
        string channelTypes;   // list of (tx) channel types (from common/channels.ned)
        string channelTypesRx; // list of (rx) channel types (from common/channels.ned)
        bool useIPv6Addresses = default(false);
}
