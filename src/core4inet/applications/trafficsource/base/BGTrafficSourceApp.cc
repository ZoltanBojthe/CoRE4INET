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

#include "core4inet/applications/trafficsource/base/BGTrafficSourceApp.h"

//CoRE4INET
#include "core4inet/buffer/base/BGBuffer.h"
#include "core4inet/utilities/ConfigFunctions.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/linklayer/ethernet/EtherEncap.h"

namespace CoRE4INET {

Define_Module(BGTrafficSourceApp);

simsignal_t BGTrafficSourceApp::sigSendInterval = registerSignal("intervalSignal");


BGTrafficSourceApp::BGTrafficSourceApp()
{
    this->sendInterval = 0;
    this->parametersInitialized = false;
}

void BGTrafficSourceApp::initialize()
{
    TrafficSourceAppBase::initialize();
}

void BGTrafficSourceApp::handleMessage(cMessage *msg)
{

    if (msg->isSelfMessage())
    {
        if (getEnvir()->isGUI())
        {
            getDisplayString().removeTag("i2");
        }
        sendMessage();
        scheduleAt(simTime() + this->sendInterval, msg);
        emit(sigSendInterval,this->sendInterval);
        handleParameterChange("sendInterval");
    }
    else
    {
        delete msg;
    }
}

void BGTrafficSourceApp::sendMessage()
{
    for (std::list<BGBuffer*>::const_iterator buf = bgbuffers.begin(); buf != bgbuffers.end(); ++buf)
    {
        auto *packet = new inet::Packet("Best-Effort Traffic", 7); //kind 7 = black
        auto frame = inet::makeShared<inet::EthernetMacHeader>();
        size_t payloadBytes = getPayloadBytes();

        frame->setDest(this->getDestAddress());
        //TODO set sourceAddress
        frame->setTypeOrLength(par("bgEtherType"));
        ASSERT2(inet::isEth2Header(*frame), "the etherType parameter is too small, implemented as length");

        auto payload = inet::makeShared<inet::ByteCountChunk>(inet::B(payloadBytes));
        packet->insertAtFront(payload);
        packet->insertAtFront(frame);

        packet->insertAtBack(inet::makeShared<inet::EthernetFcs>(inet::FcsMode::FCS_DECLARED_CORRECT));    //TODO get fcsMode from parameter

        //PacketProtocolTag
        packet->addTag<inet::PacketProtocolTag>()->setProtocol(&inet::Protocol::ethernetMac);

        sendDirect(packet, (*buf)->gate("in"));
    }
}

void BGTrafficSourceApp::handleParameterChange(const char* parname)
{
    TrafficSourceAppBase::handleParameterChange(parname);

    if (!parname && !parametersInitialized)
    {
        parametersInitialized = true;
    }

    if (!parname || !strcmp(parname, "sendInterval"))
    {
        this->sendInterval = parameterDoubleCheckRange(par("sendInterval"), 0, SIMTIME_MAX.dbl(), true);
    }
    if (!parname || !strcmp(parname, "destAddress"))
    {
        if (par("destAddress").stdstringValue() == "auto")
        {
            // assign automatic address
            this->destAddress = inet::MacAddress::generateAutoAddress();

            // change module parameter from "auto" to concrete address
            par("destAddress").setStringValue(this->destAddress.str());
        }
        else
        {
            this->destAddress.setAddress(par("destAddress").stringValue());
        }
    }
}

inet::MacAddress BGTrafficSourceApp::getDestAddress()
{
    if (!parametersInitialized)
    {
        handleParameterChange(nullptr);
    }
    return this->destAddress;
}

} //namespace

