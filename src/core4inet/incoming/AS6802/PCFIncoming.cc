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

#include "core4inet/incoming/AS6802/PCFIncoming.h"

//CoRE4INET
#include "core4inet/utilities/ConfigFunctions.h"

namespace CoRE4INET {

Define_Module(PCFIncoming);

PCFIncoming::PCFIncoming()
{
}

void PCFIncoming::initialize()
{
}

void PCFIncoming::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("in"))
    {
        auto packet = check_and_cast<inet::Packet*>(msg);
        recordPacketReceived(packet);
        auto ethHeader = packet->peekAtFront<inet::EthernetMacHeader>();
        if (ethHeader->getTypeOrLength() != 0x891d) {
            EV_ERROR << "FRAME DROPPED, wrong type:" << static_cast<int>(ethHeader->getTypeOrLength()) << " should be 0x891d" << endl;
            emit(droppedSignal, packet);
            delete packet;
            return;
        }

        auto frame = packet->peekAt<PCFrame>(ethHeader->getChunkLength());
        if (frame->getType() != static_cast<uint8_t>(pcfType))
        {
            EV_ERROR << "FRAME DROPPED, wrong type:" << static_cast<int>(frame->getType()) << " should be " << pcfType << endl;
            emit(droppedSignal, packet);
            delete packet;
            return;
        }
        sendDelayed(packet, getHardwareDelay(), "out");
    }
}

void PCFIncoming::handleParameterChange(const char* parname)
{
    CTIncoming::handleParameterChange(parname);

    if (!parname || !strcmp(parname, "pcfType"))
    {
        pcfType = static_cast<PCFType>(parameterULongCheckRange(par("pcfType"), 1, 3));
    }
}

}
