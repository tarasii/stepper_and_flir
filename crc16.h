#ifndef CRC16_h
#define CRC16_h

#include "Arduino.h"

#define    CRC_POLY_CCITT    0x1021

class CRC16
{
  public:
    CRC16();
    uint16_t crc;
    void CcittGeneric( const unsigned char *input_str, size_t num_bytes, uint16_t start_value );
  private:
    bool tabccitt_init;
    uint16_t tabccitt[256];

    void UpdateCrc( unsigned char c );
    void InitTab( void );
};

#endif