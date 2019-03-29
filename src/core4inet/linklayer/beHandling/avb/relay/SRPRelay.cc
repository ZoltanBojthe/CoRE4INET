//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#include "core4inet/linklayer/beHandling/avb/relay/SRPRelay.h"

//CoRE4INET
#include "core4inet/base/avb/AVBDefs.h"

//INET
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"

//Auto-generated Messages
#include "core4inet/linklayer/contract/ExtendedIeee802Ctrl_m.h"


namespace CoRE4INET {

Define_Module(SRPRelay);

void SRPRelay::initialize(int stage)
{
    inet::Ieee8021dRelay::initialize(stage);
}

/*
void SRPRelay::handleUpperPacket(inet::Packet *packet)
{
    if (packet->arrivedOn("srpIn"))
    {
        SRPFrame * srpFrame = check_and_cast<SRPFrame*>(msg);
        dispatchSRP(srpFrame);

    }
}
*/

void SRPRelay::handleLowerPacket(inet::Packet *packet)
{
    const auto& frame = packet->peekAtFront<inet::EthernetMacHeader>();
    if ((frame->getDest() == SRP_ADDRESS || frame->getDest() == bridgeAddress))
    {
        // copied from Ieee8021dRelay::handleLowerPacket()
        numReceivedNetworkFrames++;
        EV_INFO << "Received " << packet << " from network." << endl;
        delete packet->removeTagIfPresent<inet::DispatchProtocolReq>();
        // end of copy

        EV_DETAIL << "Deliver SRPFrame to the SRP module" << endl;
        deliverSRP(packet); // deliver to the SRP module
    }
    else
    {
        inet::Ieee8021dRelay::handleLowerPacket(packet);
    }
}

bool SRPRelay::isUpperMessage(cMessage *message)
{
    return
            message->arrivedOn("srpIn");
}

void SRPRelay::dispatchSRP(inet::Packet *packet) // (SRPFrame * srp)
{
    const auto& frame = packet->peekAtFront<inet::EthernetMacHeader>();

    int outInterfaceId = -1;
    if (auto ir = packet->findTag<inet::InterfaceReq>())
        outInterfaceId = ir->getInterfaceId();

    int arrivalInterfaceId = -1;
    if (auto ii = packet->findTag<inet::InterfaceInd>())
        arrivalInterfaceId = ii->getInterfaceId();

    //TODO frame->setSrc(bridgeAddress);

    if (outInterfaceId < 0)
    {
        //Broadcast
        broadcast(packet, arrivalInterfaceId);
    }
    else
    {
        inet::InterfaceEntry *ie = ifTable->getInterfaceById(outInterfaceId);
        dispatch(packet, ie);
    }
}

void SRPRelay::deliverSRP(inet::Packet *packet)
{

    EV_INFO << "Sending SRP frame " << packet << " to the SRP module" << endl;
    send(packet, "srpOut");
}

}
