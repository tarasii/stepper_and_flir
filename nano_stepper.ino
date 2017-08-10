
#include <SPI.h>
#include <RF24.h>
#include <SoftwareSerial.h>
//#include "crc16.h"
#include "flir.h"

#define  ENABLE   8
#define  RST      7
#define  SELECT   6
#define  STEP_X   15
#define  DIR_X    4
#define  STEP_Y   3
#define  DIR_Y    2

#define  MS1      16
#define  MS2      17
#define  MS3      18

#define  IRQ      14

#define  CENT_X     512
#define  CENT_Y     500

#define  NOISE       4

#define  CMDLEN    10
#define  ANSLEN    20

#define  SERIAL_IDLE 0
#define  SERIAL_COMMAND_SEND 1
#define  SERIAL_ANSWER_WAITING 2
#define  SERIAL_ANSWER_READY 3

#define  BTN_ORIENT 64
#define  BTN_CNTR_P 32
#define  BTN_CNTR_M 16
#define  BTN_BRI_P 8
#define  BTN_BRI_M 4
#define  BTN_INFO 2
#define  BTN_RST 1

SoftwareSerial mySerial(5, 16);

RF24 radio(9,10);

byte addresses[][6] = {"1Node","2Node"};

//byte cmd[CMDLEN] = {0x6E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
//byte cmd[CMDLEN] = {0x6E, 0x00, 0x00, 0x0B, 0x00, 0x00, 0x2F, 0x4A, 0x00, 0x00};
byte cmd[CMDLEN] = {0x6E, 0x00, 0x00, 0x05, 0x00, 0x00, 0x34, 0x4b, 0x00, 0x00};
byte ans[ANSLEN];

int potValX,potValY;           // potentiometers analog readings
byte curbtn = 0, bufbtn = 0;

uint8_t serial_loop_cnt = 0;
uint8_t serial_bytes_receved;
uint8_t serial_state = 0;

uint8_t cur_fn_cmd = 0;
uint8_t val_ori = 0;

uint16_t btn_loop_cnt = 0;

//CRC16 myCrc;
Flir myFlir;


void setup() {
  // put your setup code here, to run once:
  pinMode(ENABLE, OUTPUT);
  digitalWrite(ENABLE, HIGH);

  pinMode(RST, OUTPUT);
  digitalWrite(RST, HIGH);

  pinMode(SELECT, OUTPUT);
  digitalWrite(SELECT, HIGH);

  pinMode(DIR_Y, OUTPUT);
  digitalWrite(DIR_Y, LOW);
  pinMode(STEP_Y, OUTPUT);
  digitalWrite(STEP_Y, LOW);
    
  pinMode(DIR_X, OUTPUT);
  digitalWrite(DIR_X, LOW);
  pinMode(STEP_X, OUTPUT);
  digitalWrite(STEP_X, LOW);
  
  pinMode(MS1, OUTPUT);
  digitalWrite(MS1, LOW);

  pinMode(MS2, OUTPUT);
  digitalWrite(MS2, HIGH);
  
  pinMode(MS3, OUTPUT);
  digitalWrite(MS3, LOW);


        
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);

  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1,addresses[1]);

  radio.startListening();
  
  Serial.begin(9600);
  Serial.println("Start"); 
  
  mySerial.begin(19200);
  //mySerial.println("Hello, world?");

}

void sendcmd(byte *buf, size_t len){
  int i;

  if (serial_state != SERIAL_IDLE) return;

  for (i=0;i<len;i++){
    mySerial.write(*buf++);
  }
  serial_state = SERIAL_COMMAND_SEND;

}


void printbuf(byte *buf, size_t len){
  uint8_t i;
  
  Serial.print(len);
  Serial.print(":");
  for (i=0;i<len;i++){
    Serial.print(*buf++,HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void printans(){
  uint8_t i, l;

  if (serial_bytes_receved <ANSLEN)
    l = serial_bytes_receved;
  else
    l = ANSLEN;

  printbuf(ans, l);

}



void loop() {
  // put your main code here, to run repeatedly:

  if (radio.available()){

    unsigned long state;
    radio.read( &state, sizeof(unsigned long) );
    
//    Serial.println(state);

    potValY = (state & 0x3FF) - CENT_Y;
    potValX = ((state >> 10) & 0x3FF) - CENT_X;
    curbtn = (state >> 24) & 0xFF;

    Serial.print(state);  
    Serial.print(" ");
    Serial.print(curbtn);  
    Serial.print(" ");
    Serial.print(potValX);
    Serial.print(" ");
    Serial.print(potValY);
    Serial.print(" ");
    Serial.println(btn_loop_cnt);

    btn_loop_cnt = 0;
    
    if (potValY>0)  digitalWrite(DIR_Y, HIGH);
    else   digitalWrite(DIR_Y, LOW);

    if (potValX>0)  digitalWrite(DIR_X, HIGH);
    else   digitalWrite(DIR_X, LOW);

    if (abs(potValY)>NOISE){
      digitalWrite(ENABLE, LOW);
//      digitalWrite(STEP_Y, HIGH);
//      delay(1);
//      digitalWrite(STEP_Y, LOW);
      for(int x = 0; x < abs(potValY)/50; x++) {
        digitalWrite(STEP_Y,HIGH); 
        delayMicroseconds(500); 
        digitalWrite(STEP_Y,LOW); 
        delayMicroseconds(500);
      }
    }

    if (abs(potValX)>NOISE){
      digitalWrite(ENABLE, LOW);
//      digitalWrite(STEP_X, HIGH);
//      delay(1);
//      digitalWrite(STEP_X, LOW);
      for(int x = 0; x < abs(potValX)/50; x++) {
        digitalWrite(STEP_X,HIGH); 
        delayMicroseconds(500); 
        digitalWrite(STEP_X,LOW); 
        delayMicroseconds(500);
      }
    }
    

  }
  else
  {
    //potValY=0;
    //potValX=0;
  }

  if (btn_loop_cnt < 2000) btn_loop_cnt++;
  else {
    curbtn=0;
    bufbtn=0;

    potValY=0;
    potValX=0;
  }

  if (abs(potValY)<=NOISE && abs(potValX)<=NOISE){
    digitalWrite(ENABLE, HIGH);
  }

  if (curbtn != bufbtn && curbtn != 0){
    if (curbtn == BTN_ORIENT) { 
      ans[0]=0;
      myFlir.MakeCMD(0x11,ans,0); //get current orientation

      printbuf(myFlir.buf,myFlir.cmdlen);
      sendcmd(myFlir.buf,myFlir.cmdlen); 

      cur_fn_cmd = curbtn;
    }
    if (curbtn == BTN_CNTR_P) { 
      ans[0]=0;
      myFlir.MakeCMD(0x14,ans,0); //get current contrast

      printbuf(myFlir.buf,myFlir.cmdlen);
      sendcmd(myFlir.buf,myFlir.cmdlen); 

      cur_fn_cmd = curbtn;
    }    
    if (curbtn == BTN_CNTR_M) { 
      //myFlir.MakeCMD(0x15,ans,0); //get current brightness
      ans[0]=0;
      myFlir.MakeCMD(0x14,ans,0); //get current contrast

      printbuf(myFlir.buf,myFlir.cmdlen);
      sendcmd(myFlir.buf,myFlir.cmdlen); 

      cur_fn_cmd = curbtn;
    }
    if (curbtn == BTN_RST) { 
      ans[0]=0;
      myFlir.MakeCMD(0x3,ans,0); //reset to factory defaults

      printbuf(myFlir.buf,myFlir.cmdlen);
      sendcmd(myFlir.buf,myFlir.cmdlen); 
    }
    
    if (curbtn == BTN_INFO) { 
      ans[0]=0;
      myFlir.MakeCMD(0x5,ans,0); //get revision number

      printbuf(myFlir.buf,myFlir.cmdlen);
      sendcmd(myFlir.buf,myFlir.cmdlen);     
    }

    bufbtn = curbtn;
  }

  if (serial_state == SERIAL_COMMAND_SEND){
    serial_state = SERIAL_ANSWER_WAITING;
    serial_bytes_receved = 0;
    serial_loop_cnt = 0;
  }
  
  if (serial_state == SERIAL_ANSWER_WAITING) {
    serial_loop_cnt++;
    if (serial_loop_cnt>25) { 
      serial_state = SERIAL_ANSWER_READY; 
    }
  }

  if (serial_state == SERIAL_ANSWER_READY) {
    if (serial_bytes_receved) {

      printans();

      if (cur_fn_cmd == BTN_ORIENT){
        val_ori = ans[9] + 1;
        if (val_ori >= 4) val_ori = 0;
        Serial.println(val_ori);
        
        ans[0]=0;
        ans[1]=val_ori;
        myFlir.MakeCMD(0x11,ans,2); //set orientation
  
        printbuf(myFlir.buf,myFlir.cmdlen);
        sendcmd(myFlir.buf,myFlir.cmdlen); 
     
        cur_fn_cmd = 0;
      }
      if (cur_fn_cmd == BTN_CNTR_P) { 
        val_ori = ans[8];
        if (val_ori <= 255) val_ori++;
        Serial.println(val_ori);
        
        ans[0]=val_ori;
        ans[1]=0;
        myFlir.MakeCMD(0x14,ans,2); //set +1 contrast
  
        printbuf(myFlir.buf,myFlir.cmdlen);
        sendcmd(myFlir.buf,myFlir.cmdlen);
           
        cur_fn_cmd = 0;
      }    
      if (cur_fn_cmd == BTN_CNTR_M) { 
        val_ori = ans[8];
        if (val_ori > 0) val_ori--;
        Serial.println(val_ori);
        
        ans[0]=val_ori;
        ans[1]=0;
        myFlir.MakeCMD(0x14,ans,2); //set -1 contrast
  
        printbuf(myFlir.buf,myFlir.cmdlen);
        sendcmd(myFlir.buf,myFlir.cmdlen);
          
        cur_fn_cmd = 0;
      }    
      
//      myCrc.CcittGeneric(ans,6,0);
//      Serial.println(myCrc.crc,HEX);
//      Serial.println((uint16_t) *(ans+6)*256+*(ans+7),HEX);
    }
    else
      Serial.println("no answer");
    serial_state = SERIAL_IDLE;
  }
  
  if (mySerial.available()) {
    //Serial.write(mySerial.read());
    //Serial.println();    
    if (serial_bytes_receved < ANSLEN)
      ans[serial_bytes_receved] = mySerial.read();
    else
      mySerial.read();
      
    serial_bytes_receved++;
  }
  if (Serial.available()) {
    Serial.read();
    //mySerial.write(Serial.read());

    //sendcmd(cmd,CMDLEN); 
//    myFlir.MakeCMD(5,cmd,0);
//    printbuf(myFlir.buf,myFlir.cmdlen);
//    sendcmd(myFlir.buf,myFlir.cmdlen); 
  }
}
