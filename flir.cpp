#include "flir.h"

Flir::Flir(){
  this->cmdlen = 0;
}

void Flir::MakeCMD( byte cmd, byte *in_buf, size_t num_bytes){
  uint8_t i;

  this->cmdlen = 0;

  this->AddByte(CMDSTART);
  this->AddByte(0);
  this->AddByte(0);
  this->AddByte(cmd);
  this->AddByte(0);
  this->AddByte(num_bytes);

  this->myCrc.CcittGeneric(this->buf,this->cmdlen,0);
  this->AddByte((byte) (myCrc.crc >> 8));
  this->AddByte((byte) myCrc.crc);

  this->myCrc.CcittGeneric(in_buf,num_bytes,0);
  for(i=0;i<num_bytes;i++){
    this->AddByte(*in_buf++);
  }

  this->AddByte((byte) (myCrc.crc >> 8));
  this->AddByte((byte) myCrc.crc);
}

void Flir::LoadBUF( byte *in_buf, size_t num_bytes){
  uint8_t i;
  this->cmdlen = 0;

  for(i=0;i<num_bytes;i++){
    this->AddByte(*in_buf++);
  }
}

void Flir::AddByte(byte b){
  this->buf[this->cmdlen++] = b;
}

FLIR_ERR_TypeDef Flir::TestBUF(){

  if (this->cmdlen  = 0)    return FLIR_ERR_NOANS;
  if (this->buf[0] != 0x6E) return FLIR_ERR_NOSTART;
  if (this->cmdlen  < 6)    return FLIR_ERR_TOSHORT;
       
  this->myCrc.CcittGeneric(this->buf,6,0);
  if (this->myCrc.crc != (uint16_t) *(this->buf+6) * 256 + *(this->buf+7)) return FLIR_ERR_BADCMDCRC;

  if (this->cmdlen  < 8 + this->buf[5] )  return FLIR_ERR_TOSHORT; 
   
  this->myCrc.CcittGeneric(this->buf+8,this->buf[5],0);  
  if (this->myCrc.crc != (uint16_t) *(this->buf+this->cmdlen-2) * 256 + *(this->buf+this->cmdlen-1)) return FLIR_ERR_BADDATACRC;
  
  return FLIR_ERR_NOERR;
}

