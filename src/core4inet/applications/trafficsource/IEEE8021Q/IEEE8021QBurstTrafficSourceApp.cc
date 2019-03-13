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

#include "core4inet/applications/trafficsource/IEEE8021Q/IEEE8021QBurstTrafficSourceApp.h"

//CoRE4INET
#include "core4inet/base/CoRE4INET_Defs.h"
#include "core4inet/buffer/base/BGBuffer.h"
#include "core4inet/utilities/ConfigFunctions.h"
//Auto-generated messages
#include "core4inet/linklayer/ethernet/base/EtherFrameWithQTag_m.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/linklayer/ethernet/EtherEncap.h"

namespace CoRE4INET {

Define_Module(IEEE8021QBurstTrafficSourceApp);

IEEE8021QBurstTrafficSourceApp::IEEE8021QBurstTrafficSourceApp()
{
    this->priority = 0;
    this->vid = 0;
}

void IEEE8021QBurstTrafficSourceApp::sendMessage()
{

    for (std::list<BGBuffer*>::const_iterator buf = bgbuffers.begin(); buf != bgbuffers.end(); ++buf)
    {
        unsigned long size_left = this->getBurstSize();
        while (size_left > 0)
        {
            auto *packet = new inet::Packet("IEEE 802.1Q Burst Traffic");     //TODO set blue color
            auto frame = inet::makeShared<inet::EthernetMacHeader>();

            frame->setDest(this->getDestAddress());
            //TODO set sourceAddress
            //TODO set etherType
            auto qtag = new inet::Ieee8021qHeader();
            qtag->setVid(this->vid);
            qtag->setPcp(priority);
            qtag->setDe(false);
            frame->setCTag(qtag);

            packet->setSchedulingPriority(static_cast<short>(SCHEDULING_PRIORITY_OFFSET_8021Q - priority));     //TODO KLUDGE???

            size_t payloadBytes = getPayloadBytes();
            inet::B curPayload;
            if (size_left >= payloadBytes)
            {
                curPayload = inet::B(payloadBytes);
                size_left -= payloadBytes;
            }
            else
            {
                curPayload = inet::B(size_left);
                size_left = 0;
            }
            auto payload = inet::makeShared<inet::ByteCountChunk>(curPayload);
            packet->insertAtFront(payload);
            packet->insertAtFront(frame);

            //Padding
            inet::EtherEncap::addPaddingAndFcs(packet, inet::FcsMode::FCS_DECLARED_CORRECT);    //TODO get crcMode from parameter

            //PacketProtocolTag
            packet->addTag<inet::PacketProtocolTag>()->setProtocol(&inet::Protocol::ethernetMac);

            sendDirect(packet, (*buf)->gate("in"));
        }
    }
}

void IEEE8021QBurstTrafficSourceApp::handleParameterChange(const char* parname)
{
    BGBurstTrafficSourceApp::handleParameterChange(parname);

    if (!parname || !strcmp(parname, "priority"))
    {
        this->priority = static_cast<uint8_t>(parameterLongCheckRange(par("priority"), 0, 7));
    }
    if (!parname || !strcmp(parname, "vid"))
    {
        this->vid = static_cast<uint16_t>(parameterLongCheckRange(par("vid"), 0, MAX_VLAN_ID));
    }
}

} /* namespace CoRE4INET */
