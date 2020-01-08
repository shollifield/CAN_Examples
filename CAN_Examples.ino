#include <FlexCAN.h>
#include "CANHandler.h"

static CAN_message_t msg;
elapsedMicros sinceStart;

CANHandler canHandler;
// -------------------------------------------------------------

/*
sendMessage: Builds and sends a CAN message with a fixed length of 8 bytes using the global msg variable
 */
void sendMessage(int aid, int buf0, int buf1, int buf2, int buf3, int buf4, int buf5, int buf6, int buf7){
  msg.ext = 0;
  msg.id = aid;
  msg.len = 8;
  msg.buf[0] = buf0;
  msg.buf[1] = buf1;
  msg.buf[2] = buf2;
  msg.buf[3] = buf3;
  msg.buf[4] = buf4;
  msg.buf[5] = buf5;
  msg.buf[6] = buf6;
  msg.buf[7] = buf7;
  Can0.write(msg);
}

/*
sendProfileDiag: Uses sendMessage() to request available diagnostic services from a vehicle using OBD-II. 
 */
void sendProfileDiag() {
  //set up msg to ask for available diagnostics
  sendMessage(0x7DF, 2, 1, 0, 0, 0, 0, 0, 0);
}

/*
queryDiag: Accepts a CANHandler object with an attached message handler and int declaring rate in microseconds. First, the function 
sets the filteredID of the CANHandler object then iterates through OBD standard queries to ask for available diagnostic services. 
Once the CANHandler returns as profiled, queryDiag() will send requests for diagnostic data that the vehicle claims to support.
 */
void queryDiag(CANHandler &canHandler, int rate) {
  canHandler.setFilteredID(0x7E8);
  while(!canHandler.isProfiled()){
    sendProfileDiag();
    delayMicroseconds(500000);
    
  }
  if(canHandler.isProfiled()){
    for(int i = 0; i < 32; i++){
      if(canHandler.diagAry[i]){
        msg.ext = 0;
        msg.id = 0x7DF;
        msg.len = 8;
        msg.buf[0] = 2;
        msg.buf[1] = 1;
        msg.buf[2] = i+1; //because array index starts at 0, but PIDs start at 1
        msg.buf[3] = 0;
        msg.buf[4] = 0;
        msg.buf[5] = 0;
        msg.buf[6] = 0;
        msg.buf[7] = 0;
        Can0.write(msg);
        delayMicroseconds(rate);
      }
    }
  }
}

// -------------------------------------------------------------
void setup() {
  Can0.begin(500000);
  Can0.attachObj(&canHandler);
  canHandler.attachGeneralHandler();

}

// -------------------------------------------------------------
void loop(void) {
    queryDiag(canHandler, 125000);
}
