#ifndef DETECTOR_H
#define DETECTOR_H
#include "FlexCAN.h"

class Detector{
   public:
    void detect(); // runs detector using the msgList
    void buildMsgList(CAN_message_t &frame);// builds a msgList from fresh AIDs
    void updateMsgList(CAN_message_t &frame);// updates the count for each message in the msgList
    void clearMsgCount();// clear the count for each message in the msgList
    void calcLimit(float margin);// calculate the max and min limits for a given margin (threshold)
    int averageCounts(int countAvg[]);// calculate the avg num of messages seen over a window

    typedef struct{
      uint16_t aid; //arbitration id of message
      int count; //num times a msg is seen
      int countAvg[3];// array of 3 runs to determine the average num of messages seen
      int minLimit; // min acceptable threshold for can messages
      int maxLimit; // max acceptable threshold for can messages
      } MsgInfo;
      
    MsgInfo msgList[4096]; //4096 total arbids
};

/*
detect: Uses the metadata inside of msgList to determine if a possible intrusion occurs. Should be called in a loop.
 */
void Detector::detect(){
  for(int i = 0; i < 4096; i++){
    if(msgList[i].aid != 0){
      if(msgList[i].count < msgList[i].minLimit || msgList[i].count > msgList[i].maxLimit){
      Serial.print("DANGER!!! on AID "); Serial.print(i); 
      Serial.println("Count: "); Serial.print(msgList[i].count);
      Serial.println("Expected min: "); Serial.print(msgList[i].minLimit);
      Serial.print(" or max: "); Serial.print(msgList[i].maxLimit);
      Serial.println();
      } else{
      Serial.print(msgList[i].aid);
      Serial.println(msgList[i].minLimit); Serial.print(" min limit ");
      Serial.println(msgList[i].maxLimit); Serial.print(" max limit ");
      Serial.println(msgList[i].count); Serial.print(" count ");
      Serial.print("all ok...");
      }
    }  
  }
  clearMsgCount();
}

/*
buildMsgList: accepts an incoming CAN frame and adds it to the msgList if never seen. Increments count for the msg if seen.
 */
void Detector::buildMsgList(CAN_message_t &frame){
  if(msgList[frame.id].aid == 0){
    msgList[frame.id].aid = frame.id;
    Serial.println("wrote new message");
  }
  Serial.println("updated count");
  msgList[frame.id].count++;
}

/*
updateMsgList: Only increments msg count if it's already on the list. Disregards if the message has never been seen.
 */
void Detector::updateMsgList(CAN_message_t &frame){
  if(msgList[frame.id].aid == frame.id){
    msgList[frame.id].count++;
  }
}

/*
calcLimits: calculates min or max limit given a particular margin. 0.2 was used for initial testing.
 */
void Detector::calcLimit(float margin){
  for(int i = 0; i < 4096; i++){
    if(msgList[i].aid != 0){
      msgList[i].maxLimit = ceil(msgList[i].count * margin + msgList[i].count);
      msgList[i].minLimit = floor(msgList[i].count - msgList[i].count * margin);
      if(msgList[i].minLimit <= 2) msgList[i].minLimit += 1;
      if(msgList[i].maxLimit <= 2) msgList[i].maxLimit += 1;
      Serial.print("AID: "); Serial.print(msgList[i].aid);
      Serial.println(" min limit: "); Serial.print(msgList[i].minLimit);
    }
  }
}

/*
clarMsgCount: Clears the count for each msg in msgList.
 */
void Detector::clearMsgCount(){
  for(int i = 0; i < 4096; i++){
    msgList[i].count = 0;
  }
}

/*
averageCounts: Accepts an array of ints (will process 3). This calculates and returns the average
for the number of times seen in windows.
 */
int Detector::averageCounts(int countAvg[]){
  int sum = 0;
  for(int i = 0; i < 3; i++){
    sum += countAvg[i];
  }
  return sum / 3;
}
#endif
