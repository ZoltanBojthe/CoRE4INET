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

#include "core4inet/applications/trafficsource/AS6802/CTTrafficSourceAppBase.h"

//CoRE4INET
#include "core4inet/base/AS6802/AS6802Defs.h"
#include "core4inet/linklayer/ethernet/AS6802/CTFrame.h"
#include "core4inet/buffer/AS6802/TTBuffer.h"
#include "core4inet/buffer/AS6802/RCBuffer.h"
#include "core4inet/incoming/base/Incoming.h"
#include "core4inet/utilities/ConfigFunctions.h"
//Auto-generated Messages
#include "core4inet/linklayer/ethernet/AS6802/TTFrame_m.h"
#include "core4inet/linklayer/ethernet/AS6802/RCFrame_m.h"

#include "inet/common/ProtocolTag_m.h"
#include "inet/common/packet/Packet.h"
#include "inet/common/packet/chunk/ByteCountChunk.h"
#include "inet/linklayer/ethernet/EtherEncap.h"

namespace CoRE4INET {


CTTrafficSourceAppBase::CTTrafficSourceAppBase()
{
    this->ct_id = -1;
}

CTTrafficSourceAppBase::~CTTrafficSourceAppBase()
{

}

void CTTrafficSourceAppBase::initialize()
{
    TrafficSourceAppBase::initialize();
    CTApplicationBase::initialize();
}

void CTTrafficSourceAppBase::sendMessage()
{

    std::list<CTBuffer*> buffer;
    if (this->ct_id >= 0)
    {
        buffer = ctbuffers[static_cast<uint16_t>(this->ct_id)];
    }
    else
    {
        for (std::unordered_map<uint16_t, std::list<CTBuffer*> >::iterator bufmap = ctbuffers.begin();
                bufmap != ctbuffers.end(); ++bufmap)
        {
            buffer.merge(bufmap->second);
        }
    }
    if (buffer.empty())
    {
        EV_ERROR << "No buffer with such CT " << endl;
        if (getEnvir()->isGUI())
        {
            bubble("No buffer with such CT");
            getDisplayString().setTagArg("i2", 0, "status/excl3");
            getDisplayString().setTagArg("tt", 0, "WARNING: No buffer with such CT");
            getParentModule()->getDisplayString().setTagArg("i2", 0, "status/excl3");
            getParentModule()->getDisplayString().setTagArg("tt", 0, "No buffer with such CT");
        }
    }
    else
    {
        for (std::list<CTBuffer*>::const_iterator buf = buffer.begin(); buf != buffer.end(); ++buf)
        {
            inet::Ptr<CTFrame> frame;
            if (dynamic_cast<TTBuffer*>(*buf))
            {
                frame = inet::makeShared<TTFrame>();
            }
            else if (dynamic_cast<RCBuffer*>(*buf))
            {
                frame = inet::makeShared<RCFrame>();
            }
            else
            {
                continue;
            }

            if (this->ct_id > 0)
            {
                frame->setCtID(static_cast<uint16_t>(this->ct_id));
            }

            auto packet = new inet::Packet((*buf)->getName());
            packet->setTimestamp();
            auto payload = inet::makeShared<inet::ByteCountChunk>(inet::B(getPayloadBytes()));
            packet->insertAtFront(payload);
            packet->insertAtFront(frame);

            //Padding
            inet::EtherEncap::addPaddingAndFcs(packet, inet::FcsMode::FCS_DECLARED_CORRECT);    //TODO get crcMode from parameter

            //PacketProtocolTag
            packet->addTag<inet::PacketProtocolTag>()->setProtocol(&inet::Protocol::ethernetMac);

            if ((*buf)->gate("in")->getPathStartGate())
            {
                Incoming* in = dynamic_cast<Incoming *>((*buf)->gate("in")->getPathStartGate()->getOwner());
                if (in)
                {
                    sendDirect(packet, in->gate("in"));
                }
                else
                {
                    throw cRuntimeError(
                            "You can only connect an Incoming module to a Buffer (Buffer:%s, Attached Module:%s)",
                            (*buf)->getFullPath().c_str(),
                            (*buf)->gate("in")->getPathStartGate()->getOwner()->getFullPath().c_str());
                }
            }
            else //It is ok to directly send a frame to a buffer if no incoming is attached!
            {
                sendDirect(packet, (*buf)->gate("in"));
            }
        }
    }

}

void CTTrafficSourceAppBase::handleParameterChange(const char* parname)
{
    TrafficSourceAppBase::handleParameterChange(parname);
    CTApplicationBase::handleParameterChange(parname);
    if (!parname || !strcmp(parname, "ct_id"))
    {
        this->ct_id = static_cast<int>(parameterLongCheckRange(par("ct_id"), -1, MAX_CT_ID));
    }
}

} //namespace
