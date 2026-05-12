
#include "Packet.hpp"
#include <span>
#include <tuple>
#include <algorithm>
#include <ranges>
#include <cstring>
#include <cstdio>
#include "crc.h"
#include "stm32wlxx_hal_crc.h"
#include "stm32wlxx_hal_crc_ex.h"

Packet::Packet() = default;
Packet::~Packet() = default;

// Helper that returns a byte view over a POD value.
static auto GetAsByteArray(auto const& data) { return std::span{reinterpret_cast<const uint8_t*>(&data), sizeof(data)}; }

uint16_t calculate_crc(std::span<uint8_t> data) {
    if (data.empty()) {
        return 0xFFFFu;
    }

    extern CRC_HandleTypeDef hcrc;
    CRC_HandleTypeDef modbus_crc = hcrc;

    modbus_crc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_DISABLE;
    modbus_crc.Init.GeneratingPolynomial = 0x8005U;            // CRC-16/MODBUS polynomial
    modbus_crc.Init.CRCLength = CRC_POLYLENGTH_16B;
    modbus_crc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_DISABLE;
    modbus_crc.Init.InitValue = 0xFFFFU;                      // MODBUS initial value
    modbus_crc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_BYTE;
    modbus_crc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_ENABLE;
    modbus_crc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;

    if (HAL_CRC_Init(&modbus_crc) != HAL_OK) {
        return 0xFFFFu;
    }

    uint16_t crc = HAL_CRC_Calculate(&modbus_crc,
                                     reinterpret_cast<uint32_t*>(data.data()),
                                     static_cast<uint32_t>(data.size()));
    return crc & 0xFFFFu;
}


auto Packet::SegmentPacketToSpans(std::array<uint8_t,Packet::max_packet_size>& buffer){
    std::span Packet_ID = std::span{buffer}.first(Packet::packet_id_size); // Packet ID is 2 bytes
    std::span data_length = std::span{buffer}.subspan(Packet::packet_id_size, Packet::data_length_size); // Data Length is 2 bytes
    std::span CRC_data = std::span{buffer}.subspan(Packet::packet_id_size+Packet::data_length_size, Packet::crc_size); // CRC is 2 bytes (CRC16)
    std::span Payload = std::span{buffer}.subspan(Packet::header_size); // The rest is payload
    return std::make_tuple(Packet_ID, data_length, CRC_data, Payload);
}


 bool Packet::CreatePacket(PacketType Type, std::vector<uint8_t> &Payload_input){
    if(Payload_input.size() > Packet::max_payload_size || Payload_input.empty())  {
        printf("Payload size error!\n");     
        return false; // Payload too large to fit in packet
    }

   auto [Packet_ID, data_lenght,CRC_data,Payload] = Packet::SegmentPacketToSpans(Packet_out);
   std::ranges::copy(GetAsByteArray(ID), Packet_ID.begin()); // Copy Packet ID to packet 
   std::ranges::copy(Payload_input, Payload.begin()); // Copy Payload to packet  
   uint16_t payload_size = static_cast<uint16_t>(Payload_input.size());
   std::ranges::copy(GetAsByteArray(payload_size), data_lenght.begin()); // Copy Data Length to packet
   
   uint16_t crc = calculate_crc(Payload);
   printf("Calculated CRC: 0x%04X\n", crc); // Print calculated CRC for debugging
   std::ranges::copy(GetAsByteArray(crc), CRC_data.begin());
   return true; // Packet created successfully
 }

 bool Packet::ParsePacket(std::array<uint8_t, Packet::max_packet_size> const& Packet_inp, PacketType &Type_out, std::array<uint8_t, Packet::max_packet_size> &Payload_output ){ 

 // ZISKANI SPANU PRO KAZDOU CAST PACKETU
 auto [Packet_ID, data_lenght,CRC_data,Payload] = Packet::SegmentPacketToSpans(const_cast<std::array<uint8_t, Packet::max_packet_size>&>(Packet_inp));
 
 // KONVERZE SPANU NA PRIMITIVNI TYPY
 auto m_paket_ID =    *reinterpret_cast<const uint16_t*>(Packet_ID.data()); // Convert data length span to uint16_t
 auto m_data_length = *reinterpret_cast<const uint16_t*>(data_lenght.data()); // Convert data length span to uint16_t
 auto m_crc_data =    *reinterpret_cast<const uint16_t*>(CRC_data.data()); // Convert   CRC span to uint16_t    
 std::array<uint8_t, Packet::max_packet_size> m_payload{};
 std::ranges::copy(Payload, m_payload.data());
 

 printf("Packet ID: %u\n", m_paket_ID);
 printf("Data Length: %u\n", m_data_length);       
 printf("Payload: ");
 for( auto byte : m_payload) {
        printf("%02X ", static_cast<int>(byte));
     }
     printf("\n");

 // verify CRC
 uint32_t calculated_crc = calculate_crc(Payload); // Calculate CRC over header and payload`
    if (calculated_crc != m_crc_data) {
            printf("CRC check failed! Expected: 0x%04X, Calculated: 0x%04X\n", m_crc_data, calculated_crc);
            return false; // CRC check failed
        }
        printf("CRC check passed!\n");
        std::ranges::copy(Payload.first(m_data_length), Payload_output.begin()); // Copy payload to output vector
        Type_out = static_cast<PacketType>(Payload[0]); // Assuming the first byte of the payload indicates the packet type 
return true; // Packet parsed successfully
}