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

//==============================================================================

//CoRE4INET
#include "core4inet/base/CoRE4INET_Defs.h"
#include "core4inet/linklayer/ethernet/base/BGEtherEncap.h"

//==============================================================================

namespace CoRE4INET {

//==============================================================================

Define_Module(BGEtherEncap);

void BGEtherEncap::handleMessageWhenUp(cMessage *msg)
{
    if (msg->arrivedOn("bgIn")) {
        // from upper layer
        EV_INFO << "Received " << msg << " from BG." << endl;
        send(msg, gate("lowerLayerOut"));
    }
    else
        EtherEncap::handleMessageWhenUp(msg);
}

void BGEtherEncap::processPacketFromMac(inet::Packet *packet)
{
    send(packet->dup(), gate("bgOut"));
    EtherEncap::processPacketFromMac(packet);
}

} /* namespace CoRE4INET */
