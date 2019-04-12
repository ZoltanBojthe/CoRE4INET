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

#ifndef CORE4INET_CT_FRAME_H_
#define CORE4INET_CT_FRAME_H_

//Auto-generated Messages
#include "inet/linklayer/ethernet/EtherFrame_m.h"

namespace CoRE4INET {

enum {
    CTFrameEtherType = 0x891d;
};
/**
 * @brief Base class for the CTFrame message
 *
 * Basically implements the abstract CTID and CTMarker parameters
 * for the GUI that are extracted from the destination MAC
 *
 * @ingroup AS6802
 *
 * @author Till Steinbach
 */
    /**
     * @brief Implements abstract CtID getter.
     *
     * @return critical traffic id from destination mac
     */
    uint16_t getCtID(const inet::EthernetMacHeader& hdr);

    /**
     * @brief Implements abstract CtID setter.
     *
     * @param ctID critical traffic id that should be set in destination mac
     */
    void setCtID(inet::EthernetMacHeader& hdr, uint16_t ctID);

    /**
     * @brief Implements abstract CtMarker getter.
     *
     * @return critical traffic marker from destination mac
     */
    uint32_t getCtMarker(const inet::EthernetMacHeader& hdr);

    /**
     * @brief Implements abstract CtMarker setter.
     *
     * @param ctMarker critical traffic marker that should be set in destination mac
     */
    void setCtMarker(inet::EthernetMacHeader& hdr, uint32_t ctMarker);

    const char *ctFrameDisplayString="b=15,15,rect,black,kind,5";
    const char *ttFrameDisplayString="b=15,15,rect,black,red,5";
}

#endif /* __CORE4INET_CT_FRAME_CC_ */
