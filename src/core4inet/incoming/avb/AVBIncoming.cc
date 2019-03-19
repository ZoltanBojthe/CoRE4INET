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

#include "core4inet/incoming/avb/AVBIncoming.h"

//Std
#include <string>
using namespace std;

//CoRE4INET auto-generated
#include "core4inet/linklayer/ethernet/avb/AVBFrame_m.h"

//INET
#include "inet/common/ModuleAccess.h"

namespace CoRE4INET {

Define_Module(AVBIncoming);

AVBIncoming::AVBIncoming()
{
    this->srptable = nullptr;
}

void AVBIncoming::initialize()
{
    this->srptable = inet::getModuleFromPar<SRPTable>(par("srpTable"), this, true);
    if(!srptable){
        throw cRuntimeError("Cannot find module srpTable in node. srpTable is required");
    }
}

void AVBIncoming::handleMessage(cMessage* msg)
{
    ASSERT(msg && msg->arrivedOn("in"));
    {
        auto inPacket = check_and_cast<inet::Packet*>(msg);
        const auto& ethHeader = inPacket->popAtFront<inet::EthernetMacHeader>();
        const auto& avbHeader = inPacket->popAtFront<AVBFrame>();

        auto ctag = ethHeader->getCTag();
        if (ctag == nullptr)
            throw cRuntimeError("Missing ethernet CTAG.");

        {
            std::list<cModule*> listeners = srptable->getListenersForTalkerAddress(ethHeader->getDest(),
                    ctag->getVid());
            SR_CLASS srClass = srptable->getSrClassForTalkerAddress(ethHeader->getDest(), ctag->getVid());
            if (listeners.empty())
            {
                emit(droppedSignal, inPacket);
            }
            else
            {
                for (std::list<cModule*>::const_iterator listener = listeners.begin(); listener != listeners.end();
                        ++listener)
                {
                    if (strcmp((*listener)->getName(), "phy") == 0)
                    {
                        string outputStr;
                        if (srClass == SR_CLASS::A)
                            outputStr = "AVBAout";
                        else if (srClass == SR_CLASS::B)
                            outputStr = "AVBBout";
                        sendDelayed(inPacket->dup(), getHardwareDelay(),
                                gate(outputStr.c_str(), (*listener)->getIndex()));
                        emit(rxPkSignal, inPacket);
                    }
                    else
                    {
                        if ((*listener)->hasGate("AVBin"))
                        {
                            sendDirect(inPacket->dup(), (*listener)->gate("AVBin"));
                            emit(rxPkSignal, inPacket);
                        }
                    }
                }
            }
            delete msg;
        }
    }
}

} /* namespace CoRE4INET */

