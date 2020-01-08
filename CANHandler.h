#ifndef CANHANDLER_H
#define CANHANDLER_H
#include "Detector.h"

class CANHandler : public CANListener{
  private:
    bool profiled = false;
    uint16_t filteredID = 0;
    elapsedMicros sinceStart;
        
  public:
    void printFrame(CAN_message_t &frame); //prints frame with filter (if > 0) to serial
    void gotFrame(CAN_message_t &frame, int mailbox); //overrides the parent version so we can actually do something
    void filterMsg(CAN_message_t &frame); //sets filtered id. 0 = no filter.
    void profileDiag(CAN_message_t &frame); //requests available diagnostic services and writes them to the diagAry array.
    void setProfile(bool b){ profiled = b;} //sets bool profiled
    bool isProfiled(){ return profiled; } //gets bool profiled
    void setFilteredID(uint16_t filter){ filteredID = filter; }
    uint16_t getFilteredID(){ return filteredID; } 

    bool diagAry[32]; //array of available diagnostic messages. This is populated from profileDiag().
    Detector detector; //detector for the IDS.

};

/*
printFrame: accepts an incoming frame and prints it to the Serial console. This function is made to be compatible with can-utils & savvyCAN.
 */
void CANHandler::printFrame(CAN_message_t &frame){
  Serial.print("("); Serial.print(sinceStart / 1000.0 / 1000.0, 6); Serial.print(") "); //cast to 6 decimals to make compatible with savvyCAN
  Serial.print("can0 "); // to identify a CAN channel for can-utils
  Serial.print(frame.id, HEX); // hex is always better
  Serial.print("#");
  for ( uint8_t i = 0; i < frame.len; i++ ) {
    if(frame.buf[i] < 15){
      Serial.print("0"); Serial.print(frame.buf[i], HEX);
    } else {
      Serial.print(frame.buf[i], HEX);
    }
  } 
  Serial.println();
}

/*
profileDiag: accepts an incoming frame, expecting the filter to be set to 0x7E8. The message should contain the OBD services 
a vehicle will respond to. It then populates the diagAry variable to reflect the appropriate service status.
 */
void CANHandler::profileDiag(CAN_message_t &frame) {
  int k = 0;
  for(int i = 3; i < 7; i++){ // diagnostic really starts at 4th buffer. 1st is message length, 2nd is service + 40, 3rd is PID
    Serial.print(frame.buf[i], HEX);
    for(int j = 0; j < 8; j++){
     diagAry[k++] = bitRead(frame.buf[i], j);
    }
  }
  for (int i = 0; i < 32; i++){
    if(diagAry[i]){
      if(i < 15) Serial.print("0");
      Serial.print(i + 1, HEX);
      Serial.print(" ");
      profiled = true;
    }
  }
}

/*
filterMsg: Accepts an incoming frame and prints based on filter. We can use this function to complete diagnostic profiling.
 */

void CANHandler::filterMsg(CAN_message_t &frame) {
  if (frame.id == filteredID) {
    printFrame(frame);
    if (filteredID == 0x7E8 && !profiled){
        profileDiag(frame);
    }
   }
}

/*
gotFrame: Interrupt-driven function which is called when a new frame arrives in a CAN mailbox.
 */
void CANHandler::gotFrame(CAN_message_t &frame, int mailbox){
  if(filteredID > 0){
    filterMsg(frame);
   } else {
    printFrame(frame);
  }
  // Not very elegent...comment out either detector or printer
  /* 
  if(!profiled){
    detector.buildMsgList(frame);
  }else{
    detector.updateMsgList(frame);
  }
  */
}

#endif
