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

#include "core4inet/linklayer/ethernet/AS6802/CTFrame.h"

namespace CoRE4INET {

const char *ctFrameDisplayString="b=15,15,rect,black,kind,5";
const char *ttFrameDisplayString="b=15,15,rect,black,red,5";
const char *rcFrameDisplayString="b=15,15,rect,black,orange,5";

unsigned short getCtID(const inet::EthernetMacHeader& hdr)
{
    inet::MacAddress mac = hdr.getDest();
    uint16_t CtID = 0;
    CtID = static_cast<uint16_t>(mac.getAddressByte(4) << 8);
    CtID = static_cast<uint16_t>(CtID | static_cast<uint16_t>(mac.getAddressByte(5)));
    return CtID;
}

void setCtID(inet::EthernetMacHeader& hdr, uint16_t ctID)
{
    inet::MacAddress mac = hdr.getDest();
    mac.setAddressByte(4, static_cast<unsigned char>(ctID >> 8));
    mac.setAddressByte(5, static_cast<unsigned char>(ctID));
    hdr.setDest(mac);
}

unsigned int getCtMarker(const inet::EthernetMacHeader& hdr)
{
    inet::MacAddress mac = hdr.getDest();
    uint32_t CtMarker = 0;
    CtMarker = static_cast<uint32_t>(mac.getAddressByte(0) << 24);
    CtMarker = CtMarker | static_cast<uint32_t>(mac.getAddressByte(1) << 16);
    CtMarker = CtMarker | static_cast<uint32_t>(mac.getAddressByte(2) << 8);
    CtMarker = CtMarker | static_cast<uint32_t>(mac.getAddressByte(3));
    return CtMarker;
}

void setCtMarker(inet::EthernetMacHeader& hdr, uint32_t ctMarker)
{
    inet::MacAddress mac = hdr.getDest();
    mac.setAddressByte(0, static_cast<unsigned char>(ctMarker >> 24));
    mac.setAddressByte(1, static_cast<unsigned char>(ctMarker >> 16));
    mac.setAddressByte(2, static_cast<unsigned char>(ctMarker >> 8));
    mac.setAddressByte(3, static_cast<unsigned char>(ctMarker));
    hdr.setDest(mac);
}

}
