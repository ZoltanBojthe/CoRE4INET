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

#include "core4inet/buffer/base/Buffer.h"

//CORE4INET
#include "core4inet/base/CoRE4INET_Defs.h"
#include "core4inet/utilities/ConfigFunctions.h"

#include "inet/common/ModuleAccess.h"
#include "inet/linklayer/ethernet/EtherEncap.h"
#include "inet/linklayer/ethernet/Ethernet.h"

//std
#include <algorithm>

using namespace CoRE4INET;

simsignal_t Buffer::txPkSignal = registerSignal("txPk");
simsignal_t Buffer::rxPkSignal = registerSignal("rxPk");

Buffer::Buffer()
{
    this->maxMessageSize = 0;
    this->enabled = false;
    this->parametersInitialized = false;
}

Buffer::~Buffer()
{
    destinationGates.clear();
}

int Buffer::numInitStages() const
{
    return 1;
}

void Buffer::initialize(int stage)
{
    if (stage == 0)
    {
        EV_DETAIL << "Initialize Buffer" << endl;

        if (getEnvir()->isGUI())
        {
            //Update displaystring
            getDisplayString().setTagArg("i", 0, "buffer/empty");
            if (!isEnabled())
            {
                getDisplayString().setTagArg("i2", 0, "status/stop");
            }
        }
    }
}

void Buffer::recordPacketSent(inet::Packet *frame)
{
    emit(txPkSignal, frame);
}

void Buffer::recordPacketReceived(inet::Packet *frame)
{
    emit(rxPkSignal, frame);
}

inet::Packet* Buffer::getFrame()
{
    if (par("enabled").boolValue())
    {
        return dequeue();
    }
    else
    {
        return nullptr;
    }
}

void Buffer::putFrame(inet::Packet* frame)
{
    enqueue(frame);
}

void Buffer::handleMessage(cMessage *msg)
{
    if (msg && msg->arrivedOn("in"))
    {
        if (inet::Packet *frame = dynamic_cast<inet::Packet *>(msg))
        {
            recordPacketReceived(frame);

            if (frame->getDataLength() < inet::MIN_ETHERNET_FRAME_BYTES)
            {
                auto oldFcs = frame->removeAtBack<inet::EthernetFcs>(inet::B(4));
                inet::EtherEncap::addPaddingAndFcs(frame, oldFcs->getFcsMode(), inet::MIN_ETHERNET_FRAME_BYTES);
            }
            if (static_cast<size_t>(frame->getByteLength()) <= maxMessageSize)
            {
                putFrame(frame);
            }
            else
            {
                EV_ERROR << "Buffer received message with larger size than maxMessageSize" << endl;
            }
        }
        else
        {
            throw cRuntimeError("received Message on gate in that is no Packet");
        }
    }
}

bool Buffer::isEnabled()
{
    if (!parametersInitialized)
    {
        handleParameterChange(nullptr);
    }
    return this->enabled;
}

void Buffer::handleParameterChange(const char* parname)
{
    if (!parname && !parametersInitialized)
    {
        parametersInitialized = true;
    }

    if (!parname || !strcmp(parname, "maxMessageSize"))
    {
        this->maxMessageSize = parameterULongCheckRange(par("maxMessageSize"),
                inet::MIN_ETHERNET_FRAME_BYTES.get(), MAX_ETHER_8021Q_FRAME_BYTES.get());
    }
    if (!parname || !strcmp(parname, "destination_gates"))
    {
        destinationGates.clear();
        std::vector<cGate*> gates = parameterToGateList(par("destination_gates"), DELIMITERS);
        for (std::vector<cGate*>::const_iterator gate_it = gates.begin(); gate_it != gates.end(); ++gate_it)
        {
            if (inet::findContainingNode((*gate_it)->getOwnerModule()) != inet::findContainingNode(this))
            {
                throw cRuntimeError(
                        "Configuration problem of parameter destination_gates in module %s: Gate: %s is not in node %s! Maybe a copy-paste problem?",
                        this->getFullName(), (*gate_it)->getFullName(), inet::findContainingNode(this)->getFullName());
            }
            std::list<cGate*>::iterator findIter = std::find(destinationGates.begin(), destinationGates.end(),
                    *gate_it);
            if (findIter != destinationGates.end())
            {
                throw cRuntimeError(
                        "Configuration problem of parameter destination_gates in module %s: Gate: %s is multiple times in list! Maybe a copy-paste problem?",
                        this->getFullName(), (*gate_it)->getFullName());
            }
            destinationGates.push_back(*gate_it);
        }
    }
    if (!parname || !strcmp(parname, "enabled"))
    {
        this->enabled = par("enabled").boolValue();
    }
}

#if defined(__clang__)
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wsuggest-attribute=noreturn"
#endif

void Buffer::enqueue(__attribute__((unused)) inet::Packet *newFrame)
{
    throw cRuntimeError("Buffer::enqueue not implemented");
}

inet::Packet * Buffer::dequeue()
{
    throw cRuntimeError("Buffer::dequeue not implemented");
}

#if defined(__clang__)
#elif defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

long Buffer::getRequiredBandwidth()
{
    return -1;
}

size_t Buffer::getMaxMessageSize()
{
    return this->maxMessageSize;
}
