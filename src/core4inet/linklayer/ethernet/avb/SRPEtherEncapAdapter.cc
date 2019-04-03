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

#include "SRPEtherEncapAdapter.h"

#include "inet/common/packet/Packet.h"

namespace CoRE4INET {

Define_Module(SRPEtherEncapAdapter);


void SRPEtherEncapAdapter::initialize()
{
    portCount = static_cast<size_t>(gate("encapOut", 0)->size());
    WATCH(portCount);
}

void SRPEtherEncapAdapter::handleMessage(cMessage *msg)
{
    if (msg == nullptr)
        return;     //FIXME why needed it?

    auto packet = check_and_cast<inet::Packet*>(msg);
    auto controlInfo = msg->getControlInfo();
    if (msg->arrivedOn("srpIn")) {
        const auto& srpFrame = packet->peekAtFront<SRPFrame>();

        if (srpFrame->getAttributeType() ==  SRP_LISTENER && CHK(inet::dynamicPtrCast<const SrpListener>(srpFrame))->getAttributeSubtype() == SRP_LISTENER_READY) {
            send(msg, "encapOut", controlInfo->getSwitchPort());
        }
        else if (srpFrame->getAttributeType() ==  SRP_TALKER_ADVERTISE) {
            if(controlInfo->getNotSwitchPort() == -1) {
                for (size_t i = 0; i < portCount; i++) {
                    auto newMsg = msg->dup();
                    newMsg->setControlInfo(controlInfo->dup());
                    send(newMsg, "encapOut", static_cast<int>(i));
                }
            }
            delete msg;
        } else {
            for (size_t i = 0; i < portCount; ++i) {
                auto newMsg = msg->dup();
                newMsg->setControlInfo(controlInfo->dup());
                send(newMsg, "encapOut", static_cast<int>(i));
            }
            delete msg;
        }
    } else if (msg->arrivedOn("encapIn")) {
        send(msg, "srpOut");
    } else {
        delete msg;
    }
}

} //namespace
