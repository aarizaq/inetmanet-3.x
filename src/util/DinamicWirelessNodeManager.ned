//
// Copyright (C) 2013 Alfonso Ariza. Universidad de Malaga
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
package inet.util;

simple DinamicWirelessNodeManager
{
    int NumNodes = default(0);
    volatile double startLife @unit(s);
    volatile double endLife @unit(s);
    string nodeType;
    string nodeName;
}
