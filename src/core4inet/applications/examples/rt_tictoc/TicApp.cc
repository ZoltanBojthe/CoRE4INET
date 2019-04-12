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

#include "core4inet/applications/examples/rt_tictoc/TicApp.h"

//CoRE4INET
#include "core4inet/incoming/base/Incoming.h"
//Auto-generated Messages
#include "core4inet/applications/examples/rt_tictoc/TicToc_m.h"
#include "core4inet/linklayer/ethernet/AS6802/CTFrame.h"

#include "inet/common/packet/Packet.h"
#include "inet/linklayer/ethernet/EtherEncap.h"

namespace CoRE4INET {

Define_Module(TicApp);

simsignal_t TicApp::rxPkSignal = registerSignal("rxPk");
simsignal_t TicApp::roundtripSignal = registerSignal("roundtrip");

void TicApp::initialize()
{
    ApplicationBase::initialize();
    Scheduled::initialize();

    SchedulerActionTimeEvent *event = new SchedulerActionTimeEvent("API Scheduler Task Event", ACTION_TIME_EVENT);
    event->setAction_time(par("action_time"));
    event->setDestinationGate(gate("schedulerIn"));
    getPeriod()->registerEvent(event);
}

void TicApp::handleMessage(cMessage *msg)
{
    ApplicationBase::handleMessage(msg);

    if (msg->arrivedOn("schedulerIn"))
    {
        auto *packet = new inet::Packet("Tic");
        packet->setTimestamp();
        auto tic = inet::makeShared<Tic>();
        tic->setRoundtrip_start(simTime());
        tic->setCount(par("counter"));
        auto frame = inet::makeShared<inet::EthernetMacHeader>();
        frame->setTypeOrLength(CTFrameEtherType);
        setCtID(*frame, par("ct_id"));
        packet->insertAtFront(tic);
        packet->insertAtFront(frame);

        //Padding
        inet::EtherEncap::addPaddingAndFcs(packet, inet::FcsMode::FCS_DECLARED_CORRECT);    //TODO get crcMode from parameter

        //PacketProtocolTag
        packet->addTag<inet::PacketProtocolTag>()->setProtocol(&inet::Protocol::ethernetMac);

        EV_DETAIL << "Sending Tic Message\n";
        std::list<CTBuffer*> buffer = ctbuffers[getCtID(*frame)];
        for (std::list<CTBuffer*>::const_iterator buf = buffer.begin(); buf != buffer.end(); ++buf)
        {
            Incoming* in = dynamic_cast<Incoming *>((*buf)->gate("in")->getPathStartGate()->getOwner());
            sendDirect(packet->dup(), in->gate("in"));
        }
        delete packet;

        if(SchedulerActionTimeEvent *event = dynamic_cast<SchedulerActionTimeEvent *>(msg))
        {
            event->setNext_cycle(true);
            getPeriod()->registerEvent(event);
        }
        else
        {
            throw cRuntimeError("Received wrong Message on schedulerIn");
        }
    }
    else if (msg->arrivedOn("RCin"))
    {
        auto packet = check_and_cast<inet::Packet*>(msg);
        emit(rxPkSignal, packet);
        auto rcframe = packet->popAtFront<RCFrame>();
        auto toc = packet->popAtFront<Toc>();
        bubble(toc->getResponse());
        par("counter").setIntValue(static_cast<long>(toc->getCount()));
        emit(roundtripSignal, toc->getRoundtrip_start() - simTime());
        delete msg;
    }
    else
    {
        delete msg;
    }
}

void TicApp::handleParameterChange(const char* parname){
    CTApplicationBase::handleParameterChange(parname);
    Scheduled::handleParameterChange(parname);
}

}
