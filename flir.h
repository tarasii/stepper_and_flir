#ifndef Flir_h
#define Flir_h

#include "Arduino.h"
#include "crc16.h"

#define BUFLEN 20
#define CMDSTART 0x6E

typedef enum
{
  FLIR_ERR_NOERR = 0,
  FLIR_ERR_NOANS = 1,
  FLIR_ERR_NOSTART = 2,
  FLIR_ERR_TOSHORT = 3,
  FLIR_ERR_BADCMDCRC = 4,
  FLIR_ERR_BADDATACRC = 5,
} FLIR_ERR_TypeDef;
  

class Flir
{
  public:
    Flir();

    void MakeCMD( uint8_t cmd, byte *in_buf, size_t num_bytes);
    void LoadBUF( byte *in_buf, size_t num_bytes);
    byte buf[BUFLEN];
    size_t cmdlen;
    FLIR_ERR_TypeDef TestBUF();

  private:
    void AddByte(byte c);

    CRC16 myCrc;

};

#endif
