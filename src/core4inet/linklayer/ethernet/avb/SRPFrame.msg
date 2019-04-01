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

cplusplus {{
//INET

#if defined(__clang__)
#  pragma clang diagnostic ignored "-Winconsistent-missing-override"
#elif defined(__GNUC__) && __GNUC__ >= 5
#  pragma GCC diagnostic ignored "-Wsuggest-override"
#endif
}}

import inet.common.packet.chunk.Chunk;
import inet.linklayer.common.MacAddress;

namespace CoRE4INET;

enum AttributeType {
    SRP_TALKER_ADVERTISE = 1;
    SRP_TALKER_FAILED = 2;
    SRP_LISTENER = 3;
};

enum AttributeSubtype {     //KLUDGE
    SRP_LISTENER_READY = 31;
    SRP_LISTENER_ASKING_FAILED = 32;
    SRP_LISTENER_READY_FAILED = 33;
}
//
// Represents a Stream Reservation Protokol Frame (SRP)
//
// @see ~EtherFrame
//
// @author Philipp Meyer
//
class SRPFrame  extends inet::FieldsChunk {
    chunkLength = inet::B(8);
    uint8_t protocolVersion = 0;
    AttributeType attributeType;
    // ID of the AVB Stream
    // 64 Bit Field (48bit MAC + 16bit UniqueID)
    uint64_t streamID;

    // for internal implementation:
    //string displayString="b=15,15,rect,black,black,5";
}

class TalkerAdvertise extends SRPFrame{
    chunkLength = inet::B(25);
    attributeType = SRP_TALKER_ADVERTISE;
    // DataFrameParameters => destination adress and vlan identifier
    // 64 Bit Field
    inet::MacAddress destination_address; //only one Talker allowed per destination_address
    uint16_t vlan_identifier;
    // TSpec => Max frame size and sending interval of an AVB stream
    // 32 Bit Field
    uint16_t MaxFrameSize;
    uint16_t MaxIntervalFrames;
    // PriorityAndRank => Data Frame Priority: 6 = SRClass A & 5 = SRClass B
    // 8 Bit Field (3 Bit Data Frame Priority | 1 Bit Rank | 4 Bit Reserved (Zero filled))
    uint8_t priorityAndRank;    
    // notused
    // 32 Bit Field
    uint32_t accumulatedLatency;
}

class TalkerFailed extends TalkerAdvertise{
    chunkLength = inet::B(34);
    attributeType = SRP_TALKER_FAILED;
    // FailureInformation
    //
    // 72 Bit Field
}

class SrpListener extends SRPFrame{
    chunkLength = inet::B(8);
    attributeType = SRP_LISTENER;
    AttributeSubtype attributeSubtype = static_cast<AttributeSubtype>(-1);  //KLUDGE field - what is the real difference between the SrpListener variants?
    uint16_t vlan_identifier;
}

class ListenerReady extends SrpListener{
    chunkLength = inet::B(8);
    attributeSubtype = SRP_LISTENER_READY;
}

class ListenerAskingFailed extends SrpListener{
    chunkLength = inet::B(17);
    attributeSubtype = SRP_LISTENER_ASKING_FAILED;

    // FailureInformation
    //
    // 72 Bit Field 
}

class ListenerReadyFailed extends ListenerReady{
    chunkLength = inet::B(17);
    attributeSubtype = SRP_LISTENER_READY_FAILED;

    // FailureInformation
    //
    // 72 Bit Field
}

