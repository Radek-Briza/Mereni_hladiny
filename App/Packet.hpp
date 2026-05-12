

#include <array>
#include <cstdint>
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

        static const int max_packet_size = 50; // Example size for header metadata
        static const int packet_id_size = 2; // Example size for Packet ID
        static const int data_length_size = 2; // Example size for Data Length
        static const int crc_size = 2; // CRC16 size for MODBUS
        static const int header_size = packet_id_size+data_length_size+crc_size ; // Header metadata size   
        static const int  max_payload_size = max_packet_size - header_size; // Remaining 

    bool CreatePacket(PacketType Type, std::vector<uint8_t> &Payload_input);
    bool ParsePacket(std::array<uint8_t, Packet::max_packet_size> const& Packet_inp, PacketType &Type_out, std::array<uint8_t, Packet::max_packet_size> &Payload_output );
        
    private:
    auto SegmentPacketToSpans(std::array<uint8_t,Packet::max_packet_size>& buffer) ;    
    std::array<uint8_t, max_packet_size> Packet_out; // Output packet buffer
    uint16_t ID; // Packet ID
        
};  