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

#ifndef BGETHERENCAP_H_
#define BGETHERENCAP_H_

//INET
#include "inet/linklayer/ethernet/EtherEncap.h"

namespace CoRE4INET {

/**
 * @brief This module forwards frames to the bg buffers
 *
 * @sa EtherEncap
 *
 * @author Kai-Uwe von Deylen
 */
class BGEtherEncap : public virtual inet::EtherEncap
{
  protected:
    virtual void handleMessageWhenUp(cMessage *msg) override;
    virtual void processPacketFromMac(inet::Packet *packet) override;
}; // class BGEtherEncap


} /* namespace CoRE4INET */

#endif /* BGETHERENCAP_H_ */

