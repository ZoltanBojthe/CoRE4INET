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

#ifndef CORE4INET_BGETHERLLC_H
#define CORE4INET_BGETHERLLC_H

//CoRE4INET
#include "core4inet/base/CoRE4INET_Defs.h"

//INET
#include "inet/linklayer/ieee8022/Ieee8022Llc.h"

namespace CoRE4INET {

/**
 * @brief This module forwards frames to the bg buffers
 *
 * @sa EtherLlc
 *
 * @author Till Steinbach
 */
class BGEtherLLC : public virtual inet::Ieee8022Llc
{
  protected:
    /**
     * @brief Handles incoming messages
     *
     * @param msg cMessage pointer
     */
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void processPacketFromMac(inet::Packet *packet) override;
};

}
#endif
