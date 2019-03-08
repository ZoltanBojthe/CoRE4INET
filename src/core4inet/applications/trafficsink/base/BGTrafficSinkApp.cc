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

#include "core4inet/applications/trafficsink/base/BGTrafficSinkApp.h"

//CoRE4INET
#include "core4inet/buffer/base/BGBuffer.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"

namespace CoRE4INET {

Define_Module(BGTrafficSinkApp);

BGTrafficSinkApp::BGTrafficSinkApp(){
    received = 0;
}

void BGTrafficSinkApp::initialize()
{
    TrafficSinkApp::initialize();

    if (par("srcAddress").stdstringValue() == "auto")
    {
        // change module parameter from "auto" to concrete address
        par("srcAddress").setStringValue(inet::MacAddress::UNSPECIFIED_ADDRESS.str());
    }
    address = inet::MacAddress(par("srcAddress").stringValue());
}

void BGTrafficSinkApp::handleMessage(cMessage *msg)
{
    auto packet = check_and_cast<inet::Packet*>(msg);
    auto protocol = packet->getTag<inet::PacketProtocolTag>()->getProtocol();
    if (protocol == &inet::Protocol::ethernetMac)
    {
        auto frame = packet->popAtFront<inet::EthernetMacHeader>();
        if (address.isUnspecified() || frame->getSrc() == address)
        {
            if((!received && par("replyFirst").boolValue()) || par("reply").boolValue()){
                auto replyPacket = new inet::Packet("Reply");
                auto replyHdr = inet::makeShared<inet::EthernetMacHeader>();
                replyHdr->setDest(frame->getSrc());

                //Payloadsize:
                int payloadSize = frame->getTypeOrLength();
                if (!inet::isIeee8023Length(payloadSize))
                    payloadSize = packet->getByteLength();
                replyPacket->insertAtFront(packet->peekDataAt(inet::b(0), inet::B(payloadSize)));
                replyPacket->insertAtFront(replyHdr);
                for (std::list<BGBuffer*>::const_iterator buf = bgbuffers.begin();
                            buf != bgbuffers.end(); ++buf) {
                    sendDirect(replyPacket->dup(), (*buf)->gate("in"));
                }
                delete replyPacket;
            }
            received++;
            TrafficSinkApp::handleMessage(msg);
        }
        else
        {
            delete msg;
        }
    }
    else
    {
        delete msg;
    }
}

} //namespace
