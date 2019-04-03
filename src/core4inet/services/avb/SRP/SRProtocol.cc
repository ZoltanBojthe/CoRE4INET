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

#include "core4inet/services/avb/SRP/SRProtocol.h"

//Std
#include <algorithm>

//CoRE4INET
#include "core4inet/base/avb/AVBDefs.h"
#include "core4inet/linklayer/contract/ExtendedIeee802Ctrl_m.h"
#include "core4inet/base/NotifierConsts.h"

//INET
#include "inet/common/ModuleAccess.h"
#include "inet/common/Protocol.h"
#include "inet/common/ProtocolTag_m.h"
#include "inet/linklayer/common/InterfaceTag_m.h"
#include "inet/linklayer/common/MacAddressTag_m.h"

//Auto-generated Messages
#include "core4inet/linklayer/ethernet/avb/SRPFrame_m.h"

namespace CoRE4INET {

Define_Module(SRProtocol);

SRProtocol::SRProtocol(){
    this->srpTable = nullptr;
}

void SRProtocol::initialize()
{
    srpTable = inet::getModuleFromPar<SRPTable>(par("srpTable"), this, true);
    if (!srpTable)
    {
        throw cRuntimeError("srpTable module required for stream reservation");
    }
    srpTable->subscribe(NF_AVB_TALKER_REGISTERED, this);
    srpTable->subscribe(NF_AVB_LISTENER_REGISTERED, this);
    srpTable->subscribe(NF_AVB_LISTENER_UPDATED, this);

    ifTable = inet::getModuleFromPar<inet::IInterfaceTable>(par("interfaceTableModule"), this);
}

void SRProtocol::processTalkerAdvertise(inet::Packet *packet)
{
    const auto& talkerAdvertise = packet->peekAtFront<TalkerAdvertise>();
    int arrivalInterfaceId = packet->getTag<inet::InterfaceInd>()->getInterfaceId();
    inet::InterfaceEntry *nicModule = CHK(ifTable->getInterfaceById(arrivalInterfaceId));

    SR_CLASS srClass;
    if (talkerAdvertise->getPriorityAndRank() == PRIOANDRANK_SRCLASSA)
    {
        srClass = SR_CLASS::A;
    }
    else if (talkerAdvertise->getPriorityAndRank() == PRIOANDRANK_SRCLASSB)
    {
        srClass = SR_CLASS::B;
    }
    else
    {
        srClass = SR_CLASS::A;
    }
    srpTable->updateTalkerWithStreamId(talkerAdvertise->getStreamID(), nicModule,
            talkerAdvertise->getDestination_address(), srClass, talkerAdvertise->getMaxFrameSize(),
            talkerAdvertise->getMaxIntervalFrames(), talkerAdvertise->getVlan_identifier());
}

void SRProtocol::processListenerReady(inet::Packet *packet)
{
    const auto& listenerReady = packet->peekAtFront<ListenerReady>();
    int arrivalInterfaceId = packet->getTag<inet::InterfaceInd>()->getInterfaceId();
    inet::InterfaceEntry *nicModule = CHK(ifTable->getInterfaceById(arrivalInterfaceId));

    bool update = false;
    std::list<cModule*> listeners = srpTable->getListenersForStreamId(listenerReady->getStreamID(),
            listenerReady->getVlan_identifier());
    //Is this a new or updated stream
    if (std::find(listeners.begin(), listeners.end(), nicModule) != listeners.end()) {
        update = true;
    }

    unsigned long utilizedBandwidth = srpTable->getBandwidthForModule(nicModule);
    //Add Higher Priority Bandwidth
    utilizedBandwidth += static_cast<unsigned long>(nicModule->getSubmodule("shaper")->par("AVBHigherPriorityBandwidth"));
    unsigned long requiredBandwidth = srpTable->getBandwidthForStream(listenerReady->getStreamID(),
            listenerReady->getVlan_identifier());

    cGate *physOutGate = nicModule->getSubmodule("mac")->gate("phys$o");
    cChannel *outChannel = physOutGate->findTransmissionChannel();

    unsigned long totalBandwidth = static_cast<unsigned long>(outChannel->getNominalDatarate());

    double reservableBandwidth = par("reservableBandwidth").doubleValue();

    if (update
            || ((utilizedBandwidth + requiredBandwidth)
                    <= (static_cast<double>(totalBandwidth) * reservableBandwidth)))
    {
        srpTable->updateListenerWithStreamId(listenerReady->getStreamID(), nicModule,
                listenerReady->getVlan_identifier());
        if (!update)
        {
            EV_DETAIL << "Listener for stream " << listenerReady->getStreamID() << " registered on port "
                    << nicModule->getFullName() << ". Utilized Bandwidth: "
                    << static_cast<double>(utilizedBandwidth + requiredBandwidth)
                            / static_cast<double>(1000000) << " of "
                    << (static_cast<double>(totalBandwidth) * reservableBandwidth)
                            / static_cast<double>(1000000) << " reservable Bandwidth of "
                    << static_cast<double>(totalBandwidth) / static_cast<double>(1000000) << " MBit/s.";
        }
    }
    else
    {
        EV_DETAIL << "Listener for stream " << listenerReady->getStreamID()
                << " could not be registered on port " << nicModule->getFullName() << ". Required bandwidth: "
                << static_cast<double>(requiredBandwidth) / static_cast<double>(1000000)
                << "MBit/s, remaining bandwidth "
                << ((static_cast<double>(totalBandwidth) * reservableBandwidth)
                        - static_cast<double>(utilizedBandwidth)) / static_cast<double>(1000000)
                << "MBit/s.";
        auto outPacket = new inet::Packet();
        inet::Ptr<SRPFrame> srp;
        if (srpTable->getListenersForStreamId(listenerReady->getStreamID(),
                listenerReady->getVlan_identifier()).size() > 0)
        {
            bubble("Listener Ready Failed!");
            auto lrf = inet::makeShared<ListenerReadyFailed>();
            packet->setName("Listener Ready Failed");
            packet->setKind(inet::IEEE802CTRL_DATA);
            lrf->setVlan_identifier(listenerReady->getVlan_identifier());
            srp = lrf;
        }
        else
        {
            bubble("Listener Asking Failed!");
            auto laf = inet::makeShared<ListenerAskingFailed>();
            packet->setName("Listener Asking Failed");
            packet->setKind(inet::IEEE802CTRL_DATA);
            laf->setVlan_identifier(listenerReady->getVlan_identifier());
            srp = laf;
        }
        srp->setStreamID(listenerReady->getStreamID());
        outPacket->insertAtFront(srp);

        outPacket->addTag<inet::PacketProtocolTag>()->setProtocol(&inet::Protocol::srp);
        auto macAddressReq = outPacket->addTag<inet::MacAddressReq>();
        // macAddressReq->setSrcAddress());
        macAddressReq->setDestAddress(SRP_ADDRESS);
        cModule* talker = srpTable->getTalkerForStreamId(listenerReady->getStreamID(),
                listenerReady->getVlan_identifier());
        if (talker && talker->isName("phy"))
        {
            auto ie = check_and_cast<inet::InterfaceEntry*>(talker);
            outPacket->addTag<inet::InterfaceReq>()->setInterfaceId(ie->getInterfaceId());    // new_etherctrl->setSwitchPort(talker->getIndex());
            send(outPacket, gate("out"));
        }
        else
            delete outPacket;
    }
}

void SRProtocol::processListenerAskingFailed(inet::Packet *packet)
{
    const auto& listenerFailed = packet->peekAtFront<ListenerAskingFailed>();

    bubble("Listener Asking Failed!");
    auto outPacket = new inet::Packet();
    inet::Ptr<SRPFrame> srp;
    outPacket->addTag<inet::PacketProtocolTag>()->setProtocol(&inet::Protocol::srp);
    auto macAddressReq = outPacket->addTag<inet::MacAddressReq>();
    // macAddressReq->setSrcAddress());
    macAddressReq->setDestAddress(SRP_ADDRESS);
    outPacket->insertAtFront(srp);
    cModule* talker = srpTable->getTalkerForStreamId(listenerFailed->getStreamID(),
            listenerFailed->getVlan_identifier());
    if (talker && talker->isName("phy"))
    {
        //Necessary because controlInfo is not duplicated
        auto ie = check_and_cast<inet::InterfaceEntry*>(talker);
        outPacket->addTag<inet::InterfaceReq>()->setInterfaceId(ie->getInterfaceId());    // new_etherctrl->setSwitchPort(talker->getIndex());
        send(outPacket, gate("out"));
    }
    else
        delete outPacket;
}

void SRProtocol::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("in"))
    {
        auto *packet = check_and_cast<inet::Packet*>(msg);
        const auto& srpFrame = packet->peekAtFront<SRPFrame>();

        switch (srpFrame->getAttributeType()) {
            case SRP_TALKER_ADVERTISE:
                processTalkerAdvertise(packet);
                break;
            case SRP_LISTENER: {
                auto listenerBase = CHK(inet::dynamicPtrCast<const SrpListener>(srpFrame));
                switch(listenerBase->getAttributeSubtype()) {
                    case SRP_LISTENER_READY:
                        processListenerReady(packet);
                        break;
                    case SRP_LISTENER_ASKING_FAILED:
                        processListenerAskingFailed(packet);
                        break;
                    default:
                        throw cRuntimeError("unaccepted SRP LISTENER frame: %s, subtype=%d", packet->getName(), (int)listenerBase->getAttributeSubtype());
                }
                break;
            }
            default:
                throw cRuntimeError("unaccepted SRP frame: %s, type=%d", packet->getName(), (int)srpFrame->getAttributeType());
        }
    }
    delete msg;
}

void SRProtocol::receiveSignal(cComponent *src, simsignal_t id, cObject *obj, __attribute__((unused)) cObject *details)
{
    Enter_Method_Silent();

    if (id == NF_AVB_TALKER_REGISTERED)
    {
        if (SRPTable::TalkerEntry *tentry = dynamic_cast<SRPTable::TalkerEntry*>(obj))
        {
            auto *outPacket = new inet::Packet("Talker Advertise", inet::IEEE802CTRL_DATA);
            const auto& talkerAdvertise = inet::makeShared<TalkerAdvertise>();
            //talkerAdvertise->setStreamDA(tentry->address);
            talkerAdvertise->setStreamID(tentry->streamId);
            talkerAdvertise->setMaxFrameSize(static_cast<uint16_t>(tentry->framesize));
            talkerAdvertise->setMaxIntervalFrames(tentry->intervalFrames);
            talkerAdvertise->setDestination_address(tentry->address);
            talkerAdvertise->setVlan_identifier(tentry->vlan_id);
            if (tentry->srClass == SR_CLASS::A)
                talkerAdvertise->setPriorityAndRank(PRIOANDRANK_SRCLASSA);
            if (tentry->srClass == SR_CLASS::B)
                talkerAdvertise->setPriorityAndRank(PRIOANDRANK_SRCLASSB);

            outPacket->insertAtFront(talkerAdvertise);

            outPacket->addTag<inet::PacketProtocolTag>()->setProtocol(&inet::Protocol::srp);
            auto macAddressReq = outPacket->addTag<inet::MacAddressReq>();
            // macAddressReq->setSrcAddress());
            macAddressReq->setDestAddress(SRP_ADDRESS);

            //If talker was received from phy we have to exclude the incoming port
            if (strcmp(tentry->module->getName(), "phy") == 0) {
                auto ie = check_and_cast<inet::InterfaceEntry*>(tentry->module);
                outPacket->addTag<inet::InterfaceInd>()->setInterfaceId(ie->getInterfaceId());    // etherctrl->setNotSwitchPort(tentry->module->getIndex());
            }

            send(outPacket, gate("out"));
        }
        else
        {
            throw cRuntimeError("Received signal with wrong object type");
        }
    }
    else if (id == NF_AVB_LISTENER_REGISTERED || id == NF_AVB_LISTENER_UPDATED)
    {
        if (SRPTable::ListenerEntry *lentry = dynamic_cast<SRPTable::ListenerEntry*>(obj))
        {

            //Get Talker Port
            SRPTable *signal_srpTable = dynamic_cast<SRPTable *>(src);
            if (!signal_srpTable)
            {
                throw cRuntimeError(
                        "listenerRegistered or listenerUpdated signal received, from module that is not a SRPTable");
            }
            cModule* talker = signal_srpTable->getTalkerForStreamId(lentry->streamId, lentry->vlan_id);
            //Send listener ready only when talker is not a local application
            if (talker && talker->isName("phy"))
            {
                auto *outPacket = new inet::Packet("Listener Ready", inet::IEEE802CTRL_DATA);
                const auto& listenerReady = inet::makeShared<ListenerReady>();
                listenerReady->setStreamID(lentry->streamId);
                listenerReady->setVlan_identifier(lentry->vlan_id);

                outPacket->insertAtFront(listenerReady);

                outPacket->addTag<inet::PacketProtocolTag>()->setProtocol(&inet::Protocol::srp);
                auto macAddressReq = outPacket->addTag<inet::MacAddressReq>();
                // macAddressReq->setSrcAddress());
                macAddressReq->setDestAddress(SRP_ADDRESS);

                auto ie = check_and_cast<inet::InterfaceEntry*>(talker);
                outPacket->addTag<inet::InterfaceReq>()->setInterfaceId(ie->getInterfaceId());    // etherctrl->setSwitchPort(talker->getIndex());

                send(outPacket, gate("out"));
            }
        }
        else
        {
            throw cRuntimeError("Received signal with wrong object type");
        }

    }
}

} /* namespace CoRE4INET */
