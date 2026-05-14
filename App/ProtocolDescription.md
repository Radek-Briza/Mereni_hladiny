
# packet header
Bajty
0-1   = identifikace  protokolu 0x21 , 0x12
1-2   = identifikace  typu paketu :
            type_undefined = 0x00
            Level_request = 0x01
            Level_response = 0x02
            Battery_request = 0x03
            Battery_response = 0x04
3-4  = pocet bajtu payload , L bajt  prvni
5-6  = CRC16 hlavicky + payload , L bajt  prvni  