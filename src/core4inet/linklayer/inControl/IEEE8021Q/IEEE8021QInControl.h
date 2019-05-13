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

#ifndef CORE4INET_IEEE8021QINCONTROLINCONTROL_H_
#define CORE4INET_IEEE8021QINCONTROLINCONTROL_H_

#include <algorithm>    // std::sort

//CoRE4INET
#include "core4inet/utilities/ConfigFunctions.h"

//INET Auto-generated Messages
#include "inet/linklayer/ethernet/EtherFrame_m.h"

namespace CoRE4INET {

/**
 * @brief Represents the part of a port that receives messages (RX)
 *
 * @author Till Steinbach
 */
template<class IC>
class IEEE8021QInControl : public IC
{
    public:
        /**
         * @brief Constructor
         */
        IEEE8021QInControl();
    protected:
        /**
         * @brief Forwards frames to the appropriate incoming modules
         *
         * Critical traffic arriving on in-gate is forwarded to the incoming modules
         * or dropped if there is no module configured. Best-effort frames are
         * forwarded through the out-gate. The function timestamps messages using the
         * received and received_total parameters.
         *
         * @param msg incoming message
         */
        virtual void handleMessage(cMessage *msg) override;

        /**
         * @brief Indicates a parameter has changed.
         *
         * @param parname Name of the changed parameter or nullptr if multiple parameter changed.
         */
        virtual void handleParameterChange(const char* parname) override;

    private:
        /**
         * @brief Untagged VLAN.
         * Untagged incoming frames get tagged with this VLAN. Outgoing frames with this VLAN get untagged.
         */
        uint16_t untaggedVID;
        /**
         * @brief Tagged VLANs.
         * Only outgoing frames with one of the VLANs in this list are transmitted. Default is "0" to allow for untagged frames
         */
        std::vector<int> taggedVIDs;
};

template<class IC>
IEEE8021QInControl<IC>::IEEE8021QInControl()
{
    untaggedVID = 0;
}

template<class IC>
void IEEE8021QInControl<IC>::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("in"))
    {
        auto packet = check_and_cast<inet::Packet*>(msg);
        packet->trimFront();
        auto frame = packet->removeAtFront<inet::EthernetMacHeader>();
        if (auto frameQtag = frame->getCTagForUpdate())
        {
            //VLAN tag if requested
            if (this->untaggedVID && frameQtag->getVid() == 0)
            {
                frameQtag->setVid(this->untaggedVID);
            }
            //VLAN check if port is tagged with VLAN
            bool found = false;
            for (std::vector<int>::iterator vid = this->taggedVIDs.begin(); vid != this->taggedVIDs.end(); ++vid)
            {
                //Shortcut due to sorted vector
                if ((*vid) > frameQtag->getVid())
                {
                    break;
                }
                if ((*vid) == frameQtag->getVid())
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                EV_WARN << "Received IEEE 802.1Q frame with VID " << frameQtag->getVid()
                        << " but port was not tagged with that VID. Frame was dropped";
                delete packet;
                return;
            }
        }
        packet->insertAtFront(frame);
        //FIXME update FCS, too

        this->recordPacketReceived(packet);

        if (IC::isPromiscuous() || frame->getDest().isMulticast())
        {
            cSimpleModule::send(msg, "out");
        }
        else
        {
            inet::MacAddress address;
//            address.setAddress(packet->getArrivalGate()->getPathStartGate()->getOwnerModule()->par("address"));     //FIXME this is a KLUDGE of CoRE4INET developers. Why did need it?
            address = check_and_cast<inet::InterfaceEntry*>(packet->getArrivalGate()->getPathStartGate()->getOwnerModule()->getParentModule())->getMacAddress();     //FIXME this is a KLUDGE of CoRE4INET developers. Why did need it?
            if (frame->getDest().equals(address))
            {
                cSimpleModule::send(msg, "out");
            }
            else
            {
                IC::handleMessage(msg);
            }
        }
    }
    else
    {
        IC::handleMessage(msg);
    }
}

template<class IC>
void IEEE8021QInControl<IC>::handleParameterChange(const char* parname)
{
    IC::handleParameterChange(parname);

    if (!parname || !strcmp(parname, "untaggedVID"))
    {
        this->untaggedVID = static_cast<uint16_t>(parameterULongCheckRange(cComponent::par("untaggedVID"), 0,
                MAX_VLAN_NUMBER));
    }
    if (!parname || !strcmp(parname, "taggedVIDs"))
    {
        taggedVIDs = cStringTokenizer(cComponent::par("taggedVIDs"), ",").asIntVector();
        std::sort(taggedVIDs.begin(), taggedVIDs.end());
    }
}

}

#endif
