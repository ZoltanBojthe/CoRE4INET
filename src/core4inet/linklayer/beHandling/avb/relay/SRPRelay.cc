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
        delete packet->removeTagIfPresent<DispatchProtocolReq>();
        // end of copy

        EV_DETAIL << "Deliver SRPFrame to the SRP module" << endl;
        deliverSRP(frame); // deliver to the SRP module
    }
    else
    {
        inet::Ieee8021dRelay::handleLowerPacket(msg);
    }
}

bool SRPRelay::isUpperMessage(cMessage *message)
{
    return
            message->arrivedOn("srpIn");
}

void SRPRelay::dispatchSRP(inet::Packet *packet) // (SRPFrame * srp)
{
    int outInterfaceId = packet->getTag<InterfaceReq>()->getInterfaceId();
    InterfaceEntry *outInterface = ifTable->getInterfaceById(outInterfaceId);

    int portNum = controlInfo->getSwitchPort();
    int notPortNum = controlInfo->getNotSwitchPort();
    inet::MacAddress address = controlInfo->getDest();

    if (portNum >= static_cast<int>(portCount))
        throw cRuntimeError("Output port %d doesn't exist!", portNum);

    const auto& frame = packet->peekAtFront<inet::EthernetMac>();
    frame->setSrc(bridgeAddress);
    frame->setDest(address);
    frame->setByteLength(ETHER_MAC_FRAME_BYTES);
    frame->setEtherType(MSRP_ETHERTYPE);
    frame->encapsulate(srp);

    delete controlInfo;

    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);

    //Broadcast
    if (portNum < 0)
    {
        for (size_t i = 0; i < portCount; ++i)
        {
            if (static_cast<int>(i) != notPortNum)
            {
                send(frame->dup(), "ifOut", static_cast<int>(i));
                EV_INFO << "Sending SRP frame " << frame << " with destination = " << frame->getDest() << ", port = "
                        << i << endl;
            }
        }
        delete frame;
    }
    else
    {
        send(frame, "ifOut", portNum);
        EV_INFO << "Sending SRP frame " << frame << " with destination = " << frame->getDest() << ", port = " << portNum
                << endl;
    }
}

void SRPRelay::deliverSRP(inet::Packet *packet)
{
    SRPFrame * srp = check_and_cast<SRPFrame *>(frame->decapsulate());

    inet::Ieee802Ctrl * controlInfo = new inet::Ieee802Ctrl();
    controlInfo->setSrc(frame->getSrc());
    controlInfo->setSwitchPort(frame->getArrivalGate()->getIndex());
    controlInfo->setDest(frame->getDest());

    srp->setControlInfo(controlInfo);

    delete frame; // we have the SRP packet, so delete the frame

    EV_INFO << "Sending SRP frame " << srp << " to the SRP module" << endl;
    send(srp, "srpOut");
}

}
