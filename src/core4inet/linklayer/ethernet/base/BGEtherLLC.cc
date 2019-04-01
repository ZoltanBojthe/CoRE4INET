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

#include "core4inet/linklayer/ethernet/base/BGEtherLLC.h"

namespace CoRE4INET {

Define_Module(BGEtherLLC);

void BGEtherLLC::handleMessageWhenUp(cMessage *msg)
{
    if (msg->arrivedOn("bgIn")) {
        // from upper layer
        EV_INFO << "Received " << msg << " from BG." << endl;
        processPacketFromHigherLayer(check_and_cast<inet::Packet *>(msg));
    }
    else
        Ieee8022Llc::handleMessageWhenUp(msg);
}

void BGEtherLLC::processPacketFromMac(inet::Packet *packet)
{
    send(packet->dup(), gate("bgOut"));
    Ieee8022Llc::processPacketFromMac(packet);
}

}
