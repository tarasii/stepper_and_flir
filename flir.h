#ifndef Flir_h
#define Flir_h

#include "Arduino.h"
#include "crc16.h"

#define BUFLEN 20
#define CMDSTART 0x6E

class Flir
{
  public:
    Flir();

    void MakeCMD( uint8_t cmd, byte *in_buf, size_t num_bytes);
    void LoadBUF( byte *in_buf, size_t num_bytes);
    byte buf[BUFLEN];
    size_t cmdlen;

  private:
    void AddByte(byte c);

    CRC16 myCrc;

};

#endif
