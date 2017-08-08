#include "crc16.h"

CRC16::CRC16(){

  this->tabccitt_init = false;
  this->InitTab();
  this->crc = 0;

}

void CRC16::InitTab( void ){
  uint16_t i;
  uint16_t j;
  uint16_t crc;
  uint16_t c;

  for (i=0; i<256; i++) {

    crc = 0;
    c   = i << 8;

    for (j=0; j<8; j++) {

      if ( (crc ^ c) & 0x8000 ) crc = ( crc << 1 ) ^ CRC_POLY_CCITT;
      else                      crc =   crc << 1;

      c = c << 1;
    }

    this->tabccitt[i] = crc;
  }

  this->tabccitt_init = true;

}

void CRC16::CcittGeneric( const unsigned char *input_str, size_t num_bytes, uint16_t start_value ){
  const unsigned char *ptr;
  size_t a;

  this->crc = start_value;
  ptr = input_str;

  if ( ptr != NULL ) for (a=0; a<num_bytes; a++) {

    this->UpdateCrc(*ptr++);

  }

}

void CRC16::UpdateCrc(unsigned char c){

  this->crc = (this->crc << 8) ^ this->tabccitt[ ((this->crc >> 8) ^ (uint16_t) c) & 0x00FF ];

}



