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

#include "core4inet/linklayer/ethernet/avb/SRPEtherEncap.h"

#include "core4inet/base/avb/AVBDefs.h"
#include "core4inet/linklayer/contract/ExtendedIeee802Ctrl_m.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

//==============================================================================

namespace CoRE4INET {

//==============================================================================

Define_Module(SRPEtherEncap);

//==============================================================================

SRPEtherEncap::SRPEtherEncap()
{

}

//==============================================================================

SRPEtherEncap::~SRPEtherEncap()
{
}

//==============================================================================

void SRPEtherEncap::handleMessage(cMessage *msg)
{
    if (msg && msg->arrivedOn("srpIn"))
    {
        auto * srpFrame = check_and_cast<inet::Packet*>(msg);
        dispatchSRP(srpFrame);
    }
    else if (msg && msg->arrivedOn("lowerLayerIn"))
    {
        inet::Packet * packet = check_and_cast<inet::Packet*>(msg);
        const auto& frame = packet->peekAtFront<inet::EthernetMacHeader>();
        if (frame->getDest() == SRP_ADDRESS)
        {
            EV_DETAIL << "Deliver SRPFrame to the SRP module" << endl;
            deliverSRP(packet); // deliver to the SRP module
        }
        else
        {
            BGEtherEncap::handleMessage(msg);
        }
    }
    else
    {
        BGEtherEncap::handleMessage(msg);
    }
}

//==============================================================================

void SRPEtherEncap::dispatchSRP(inet::Packet *packet)
{
    int outInterfaceId = -1;
    if (auto ir = packet->findTag<inet::InterfaceReq>())
        outInterfaceId = ir->getInterfaceId();

    if (portNum >= 1)
        throw cRuntimeError("Output port %d doesn't exist!", portNum);

    int arrivalInterfaceId = -1;
    if (auto ii = packet->findTag<inet::InterfaceInd>())
        arrivalInterfaceId = ii->getInterfaceId();

    auto macAddressReq = packet->getTag<inet::MacAddressReq>();
    const auto& frame = inet::makeShared<inet::EthernetMacHeader>();
    frame->setDest(macAddressReq->getDestAddress());
    frame->setSrc(macAddressReq->getSrcAddress());
    frame->setTypeOrLength(MSRP_ETHERTYPE);
    packet->insertAtFront(frame);
    addPaddingAndFcs(packet, fcsMode);
    packet->addTagIfAbsent<inet::PacketProtocolTag>()->setProtocol(&inet::Protocol::ethernetMac);

    if (notPortNum != 0)
    {
        send(packet, "lowerLayerOut");
        EV_INFO << "Sending SRP frame " << packet << " with destination = " << frame->getDest() << ", port = " << portNum
                << endl;
    }
    else
    {
        delete packet;
    }
}

//==============================================================================

void SRPEtherEncap::deliverSRP(inet::Packet * packet)
{
    const auto& frame = packet->popAtFront<inet::EthernetMacHeader>();
//    SRPFrame * srp = check_and_cast<SRPFrame *>(frame->decapsulate());

    auto addrTag = packet->addTag<inet::MacAddressInd>();
    addrTag->setSrcAddress(frame->getSrc());
    addrTag->setDestAddress(frame->getDest());

    controlInfo->setSwitchPort(0);  //TODO

    EV_INFO << "Sending SRP frame " << packet << " to the SRP module" << endl;
    send(packet, "srpOut");
}

//==============================================================================

} /* namespace CoRE4INET */
