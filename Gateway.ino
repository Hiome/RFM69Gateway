#include <RFM69_ATC.h>
#include <RFM69_OTA.h>
#include <SPIFlash.h>

#define FIRMWARE_VERSION "V20.4.30"
#define R2

#define NODEID        1   //unique for each node on same network
#define NETWORKID     27  //the same on all nodes that talk to each other
#define ENCRYPTKEY    "smarterisbetters" //exactly the same 16 characters/bytes on all nodes!
#define SERIAL_BAUD   19200
#define DEBUG_MODE    false
#define ACK_TIME      150 // # of ms to wait for an ack
#define TIMEOUT       3000
#define SERIAL_TIME   10  // # of ms to wait for serial input
#define MAX_DATA_LEN  64

// ASCII conversions
#define NEW_LINE      10
#define SEMICOLON     59

RFM69_ATC radio;
SPIFlash flash(8, 0xEF30); //EF30 for windbond 4mbit flash

char input[MAX_DATA_LEN];
byte targetID=0;
uint32_t targettedAt = 0;

void printVer() {
  Serial.write(NODEID+48);
  Serial.write(SEMICOLON);
  Serial.print(F(FIRMWARE_VERSION));
  Serial.print(F(";01")); // no battery level to report
  Serial.write(NEW_LINE);
  Serial.flush();
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  radio.initialize(RF69_915MHZ, NODEID, NETWORKID);
  #ifdef R2
    radio.setHighPower(); //must include this only for RFM69HW/HCW!
  #endif
  radio.encrypt(ENCRYPTKEY);

  if (flash.initialize()) flash.sleep();

  delay(100);
  printVer();
}

void loop() {
  if (targetID==0 && radio.receiveDone() && radio.DATALEN > 3) {
    // Serial.print("rx: ");
    // Serial.println(millis());

    // delineate this message from previous potential garbage
    Serial.write(NEW_LINE);
    // send real message
    Serial.print(radio.SENDERID);
    Serial.write(SEMICOLON);
    for (uint8_t i = 0; i < radio.DATALEN; i++) {
      if (radio.DATA[i] != NEW_LINE) Serial.write(radio.DATA[i]);
    }
    Serial.write(NEW_LINE);

    if (radio.ACKRequested()) radio.sendACK();

    // Serial.print("ack: ");
    // Serial.println(millis());
  } else if (targetID > 0 && (millis() - targettedAt) > 500) {
    // if Core sent a TO command and then didn't follow up with FLX within 0.5s,
    // stop waiting and get back to listening to radio
    targetID = 0;
  } else if (Serial.available()) { // Core sent us something! <3
    byte inputLen = readSerialLine(input, NEW_LINE, MAX_DATA_LEN, SERIAL_TIME);
    if (inputLen==4 && input[0]=='F' && input[1]=='L' && input[2]=='X' && input[3]=='?') {
      // update firmware
      if (targetID==0) {
        Serial.println(F("TO?"));
      } else {
        CheckForSerialHEX((byte*)input, inputLen, radio, targetID, TIMEOUT, ACK_TIME, DEBUG_MODE);
        Serial.write(NEW_LINE);
        targetID = 0;
      }
    } else if (inputLen>3 && inputLen<=6 && input[0]=='T' && input[1]=='O' && input[2]==':') {
      // prep for firmware update
      targetID = 0;
      for (byte i = 3; i<inputLen; i++) { //up to 3 characters for target ID
        if (input[i] >= 48 && input[i] <= 57) {
          targetID = targetID*10+input[i]-48;
        } else {
          targetID=0;
          break;
        }
      }
      if (targetID>0) {
        Serial.print(F("TO:"));
        Serial.print(targetID);
        Serial.println(F(":OK"));
        targettedAt = millis();
      } else {
        Serial.print(input);
        Serial.println(F(":INV"));
        targettedAt = 0;
      }
    } else if (inputLen==3 && input[0]=='V' && input[1]=='E' && input[2]=='R') {
      // report my firmware version
      printVer();
      targetID = 0;
    }
  }
}
