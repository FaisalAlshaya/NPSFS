//Arduino RockBlock Control code
//Communicates with Python command and control
//Owned by Faisal Alshaya and developed by Jousha Heaney (2017)
//This is an open source and free software
//Last Updated: 8July2017 , only change is from 
// "RBmsgComplete" to "RBmsgComplet" in reciving side 

#include "IridiumSBD.h"

//Constants
# define FILE_SEG_SIZE 1000
# define MAX_FILE_SIZE 3000
# define RB_MSG_SIZE 60

//Globals
IridiumSBD isbd(Serial1, 10);   // RockBLOCK SLEEP(OnOff): pin 10
 
void setup(){
  Serial.begin(9600); //laptop serial monitor
  Serial1.begin(19200);  //rockBlock
  
  //isbd.setPowerProfile(); // This is a low power application
  isbd.begin(); // Wake up the 9602 and prepare it to communicate.
  isbd.attachConsole(Serial); //Forward RB/Arduinio Serial comms to laptop
  //isbd.attachDiags(Serial); //Forward ISBD library debug to the laptop
}

///////////////////////////////
// Computer Serial Functions //
///////////////////////////////
//compPrint()
//Print line to computer serial
void compPrint(String msg){
  Serial.println(msg);
}

//compReadline()
//Non-blocking call to read 1 line from computer serial
String compReadline(){
  String readStr = ""; 
  while (Serial.available()){
    char c = Serial.read();
    readStr += c;
  }
  return readStr;
}

//compReadfile() 
//Blocking call, reads exactly the length provided as long as there is data
//  in the serial buffer
String compReadfile(){
  int filesize = 0;
  int numSegs = 0;
  String fileString = "";

  while (!Serial.available());//wait for input
  filesize = compReadline().toInt();
  compPrint("FS: " + String(filesize));
  
  while (!Serial.available());//wait for input
  numSegs = compReadline().toInt();
  compPrint("NS: " + String(numSegs));
  
  for (int i = 0; i < numSegs; i++){
    while (!Serial.available());//Wait for input
    while (Serial.available()){
      char c = Serial.read();
      if (c == '\r'){//if \r character found, serial send has finished
        return fileString;
      }else{
        fileString += c;
      }
    }
  }
  compPrint("File Transfer Complete");
  return fileString;
}

///////////////////////////////
// RockBLOCK Functions       //
///////////////////////////////
void rockSetup(){
  int sigQuality = 0;
  //int waitingMsgs = 0;
  //int stat = 0;
  //unsigned char recvBuffer[RB_MSG_SIZE];
  //size_t recvBufferSize = sizeof(recvBuffer);
  
  isbd.begin(); // Wake up the 9602 
  isbd.getSignalQuality(sigQuality);
  compPrint("RB Signal Quality: " + String(sigQuality));
  /*stat = isbd.sendReceiveSBDText(blankMsg, recvBuffer, recvBufferSize);
  if (recvBufferSize == 0){
    compPrint("No Message to receive");
    break;
  }*/

  isbd.sleep(); // Test complete; sleep 9602
  compPrint("Warmup complete");
}

void rockSendFile(String fileToSend){
  int segments = 0;
  int stat = 0;
  String compReady = "";
  String fileSeg = "";
  String targetRockblock = "";
  char segChars [RB_MSG_SIZE+1];
  String finishedMsg = "RBmsgComplete";
  int messageSize = RB_MSG_SIZE; //Start from the max size
  int headerSize = 0;

  while (!Serial.available());//wait for input
  targetRockblock = compReadline();
  if (targetRockblock != "None"){
    messageSize -= 9; //make room for RB ID in each message
    headerSize = 9;
  } else {
    targetRockblock = "";
  }
  
  isbd.begin(); // Wake up the 9602
  segments = (int) fileToSend.length()/messageSize;
  
  //Round up to nearest whole segment
  if (((int) fileToSend.length()) % messageSize != 0){
    segments++;
  }
  
  //Divide file up into message size segments and send
  for (int i = 0; i < segments; i++){
    //Create segments based on message size
	fileSeg = fileToSend.substring(i*messageSize, i*messageSize+messageSize);
    fileSeg = fileSeg + "\0";
    fileSeg = targetRockblock + fileSeg; //Add target RB ID
    
	compPrint("messageSize: " + String(messageSize));
    compPrint("fileSeg size: " + String(fileSeg.length()));
    
	//Convert String to char* for send function
	fileSeg.toCharArray(segChars, messageSize+headerSize+2);
    compPrint("segchars: " + String(segChars));
	
	//Send char*
    stat = isbd.sendSBDText(segChars);
    compPrint("Stat=" + String(stat));
    /*if (stat != ISBD_SUCCESS){
      compPrint("Error Occured");
      break;
    }*/
  }
  //If all the segments were successfully transferred, send file complete message ("RBmsgComplete")
  if (stat == ISBD_SUCCESS){
    finishedMsg = targetRockblock + finishedMsg;
    finishedMsg.toCharArray(segChars, finishedMsg.length());
    
	stat = isbd.sendSBDText(segChars);
    if (stat != ISBD_SUCCESS){
      compPrint("Error Occured");
    }
  }
  isbd.sleep(); // Op complete; sleep 9602
  
  compPrint("##Send complete##");
}

//rockRecvFile()
void rockRecvFile(){
  int stat = 0;
  bool Receiving = false;
  char recvBuffer[RB_MSG_SIZE];
  String msgSeg = "";
  size_t recvBufferSize = sizeof(recvBuffer);
  char *blankMsg = '\0';

  isbd.begin(); // Wake up the 9602

  while (1){
    stat = isbd.sendReceiveSBDText(blankMsg, (unsigned char *) recvBuffer, recvBufferSize);
    /*Serial.print("UU: ");
    for (int i = 0; i < recvBufferSize; i++){
      Serial.print(recvBuffer[i]);
    }
    Serial.println();*/
	//If message end found, stop receiving
    if (msgSeg.indexOf("RBmsgComplet") > 0){
      compPrint("RE_Recv Complete");
      Receiving = false;
      break;
    }
	//If no message received and not currently transferring a file, quit
    if (recvBufferSize == 0 && !Receiving){
      compPrint("No Message to receive");
      break;
    }
	//If message received
    compPrint("\nRB_Stat: " + String(stat));
    if (stat == ISBD_SUCCESS){
	  Receiving = true; //While still receiving a file, set to true
      recvBuffer[recvBufferSize]='\0';//null terminate received string
      msgSeg = String(recvBuffer);//convert char* to String
      compPrint("RB_Message: " + msgSeg);
      compPrint("RB_Size: " + String(recvBufferSize));
    }
  }

  isbd.sleep(); // Op complete; sleep 9602
  compPrint("##Receive complete##");
}

///////////////////////////////
// Arduino Loop
///////////////////////////////
int i = 0;
String compData = "";
String fileString = "";
void loop() {
  //Read commands from computer/python
  compData = compReadline();
  if (compData != ""){
    //compPrint(compData);
    if (compData == "RB_Warmup"){
      compPrint("Entering Setup Mode");
      //compPrint("RB_Message: This is a test send");
      rockSetup();
    } else if (compData == "RB_Send"){
      compPrint("Entering File Send Mode");
	  //Get file from computer/python
      fileString = compReadfile();
	  //Send file via RockBLOCK
      rockSendFile(fileString);
    } else if (compData == "RB_Recv"){
      compPrint("Entering File Recv Mode");
      rockRecvFile();
    } else 
      compPrint("Error: Unk Mode");
    }
}
