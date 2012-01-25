//
// Copyright (C) 2006 Institut fuer Telematik, Universitaet Karlsruhe (TH)
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

cplusplus {{
#include <CommonMessages_m.h>

static const int GIAAPPCOMMAND_L = 8;
#define GIASEARCHAPP_L(msg) (GIAAPPCOMMAND_L)
#define GIAPUT_L(msg) (GIASEARCHAPP_L(msg) + msg->getKeysArraySize() * KEY_L)
#define GIAGET_L(msg) (GIASEARCHAPP_L(msg) + KEY_L + MAXRESPONSES_L)
#define GIAGETRSP_L(msg) (GIASEARCHAPP_L(msg) + KEY_L + NODEHANDLE_L)
}}



class noncobject NodeHandle;

class noncobject OverlayKey;

enum GIAAppCommand
{

    GIA_PUT = 0;
    GIA_SEARCH = 1;
    GIA_ANSWER = 2;
}

//
// @author Bernhard Heep
//
packet GIASearchAppMessage
{
	int command enum (GIAAppCommand);
}

//
// @author Robert Palmer
//
packet GIAput extends GIASearchAppMessage
{
    	OverlayKey keys[];
}

//
// @author Bernhard Heep
//
packet GIAremove extends GIASearchAppMessage
{
}

//
// @author Robert Palmer
//
packet GIAsearch extends GIASearchAppMessage
{
   	OverlayKey searchKey;
	int maxResponses;
}

//
// @author Robert Palmer
//
packet GIAanswer extends GIASearchAppMessage
{
   	OverlayKey searchKey;
	NodeHandle node;
}
