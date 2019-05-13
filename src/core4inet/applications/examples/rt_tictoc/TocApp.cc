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

#include "core4inet/applications/examples/rt_tictoc/TocApp.h"

//CoRE4INET
#include "core4inet/incoming/base/Incoming.h"
//Autp-generated Messages
#include "core4inet/applications/examples/rt_tictoc/TicToc_m.h"
#include "core4inet/linklayer/ethernet/AS6802/CTFrame.h"

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/EtherEncap.h"

namespace CoRE4INET {

Define_Module(TocApp);

void TocApp::initialize()
{
    ApplicationBase::initialize();
}

void TocApp::handleMessage(cMessage *msg)
{
    ApplicationBase::handleMessage(msg);

    if (msg->arrivedOn("TTin"))
    {
        if (msg->isPacket())   //KLUDGE: if TTFrame
        {
            auto inpacket = check_and_cast<inet::Packet*>(msg);
            auto ttframe = inpacket->popAtFront<inet::EthernetMacHeader>();
            //TODO check ttframe typeorlength field
            auto tic = inpacket->peekAtFront<Tic>();
            bubble(tic->getRequest());
            par("counter").setIntValue(static_cast<long>(tic->getCount()));

            auto packet = new inet::Packet("Toc");
            packet->setTimestamp();

            auto toc = inet::makeShared<Toc>();
            toc->setRoundtrip_start(tic->getRoundtrip_start());
            toc->setCount(tic->getCount() + 1);

            auto frame = inet::makeShared<inet::EthernetMacHeader>();
            frame->setTypeOrLength(CTFrameEtherType);   //RCFrame
            setCtID(*frame, par("ct_id"));
            packet->insertAtFront(toc);
            packet->insertAtFront(frame);

            packet->insertAtBack(inet::makeShared<inet::EthernetFcs>(inet::FcsMode::FCS_DECLARED_CORRECT));    //TODO get fcsMode from parameter

            //PacketProtocolTag
            packet->addTag<inet::PacketProtocolTag>()->setProtocol(&inet::Protocol::ethernetMac);

            delete msg;

            EV_DETAIL << "Answering Tic Message with Toc Message\n";
            std::list<CTBuffer*> buffer = ctbuffers[getCtID(*frame)];
            for (std::list<CTBuffer*>::const_iterator buf = buffer.begin(); buf != buffer.end(); ++buf)
            {
                Incoming* in = dynamic_cast<Incoming *>((*buf)->gate("in")->getPathStartGate()->getOwner());
                sendDirect(packet->dup(), in->gate("in"));
            }
            delete packet;
        }
        else{
            delete msg;
        }
    }
    else
    {
        delete msg;
    }
}

}
