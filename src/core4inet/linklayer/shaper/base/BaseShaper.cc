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
#include "core4inet/linklayer/shaper/base/BaseShaper.h"

#include "inet/common/ModuleAccess.h"

//INET
#include "inet/linklayer/ethernet/EtherMacFullDuplex.h"

//Std
#include <algorithm>

using namespace CoRE4INET;

void BaseShaper::initialize(int stage)
{
    if (stage == 0)
    {
        inet::EtherMacFullDuplex* mac =
                dynamic_cast<inet::EtherMacFullDuplex*>(gate("out")->getPathEndGate()->getOwner());
        if (mac->gate("phys$o"))
        {
            cGate *physOutGate = mac->gate("phys$o");
            outChannel = physOutGate->findTransmissionChannel();
        }
        else
        {
            throw cRuntimeError("A shaper can only be used attached to an EtherMacFullDuplex");
        }
        WATCH(framesRequested);
    }
}

int BaseShaper::numInitStages() const
{
    return 1;
}

void BaseShaper::handleMessage(cMessage *msg)
{
    EV_WARN << "Shaper has no method to handle frametype. Dropping frame" << endl;
    delete msg;
}

void BaseShaper::enqueueMessage(cMessage *msg)
{
    EV_WARN << "Shaper has no method to enqueue frametype. Dropping frame" << endl;
    delete msg;
}
