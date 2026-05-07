#include "../Core/Inc/main.h" 

#include <array>
#include <cstdint>
#include <span>
#include <cstring>
#include <vector>


class Packet
{   
public:
    Packet();
    ~Packet();

    
    public:
    enum PacketType
        {
            Data_level = 0x01,
            Data_battery = 0x02,
            Ack = 0x03,
            Nack = 0x04,
        };

        void CreatePacket(PacketType Type, std::vector<uint8_t> &Payload_input);
        
        
};  