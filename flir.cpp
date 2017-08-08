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

  for(i=0;i<num_bytes;i++){
    this->AddByte(*in_buf++);
  }

  this->myCrc.CcittGeneric(in_buf,num_bytes,0);
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

