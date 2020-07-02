/* 
Arduino hosted Z80 disassembler by Mike Sutton
Bread80.com
*/
#include "Arduino.h"
#include "z80-disassembler.h"

//(For testing)
//Output a tab of opcodes for a the given prefix and opcodeset

//Change this value to simulate a partial opcode (i.e. if you only have one byte of a 3 byte opcode)
//and unknown parameters will be replaced with appropriate mnemonics, e.g. LD A,nn or LD A,nFF instead of LD A,FFFF
#define MAXDATALENGTH 1

//Z80Disassembler Z80 = Z80Disassembler();

void disassembleTable(byte prefix, byte opcodeset) {
  for (byte opcode = 0;true;opcode++) {
//  byte opcode = 255;  
 
  byte data[] = {0x01, 0x02, 0x03, 0x04, 0x05};
  int ptr = 0;
  if (prefix) {
    data[ptr] = prefix;
    ptr++;
  }
  if (opcodeset) {
    data[ptr] = opcodeset;
    ptr++;
  }

  if (opcodeset == 0xCB && prefix) {
    data[ptr + 1] = opcode; //Should always be data[3]
    ptr += 2;
  } else {
    data[ptr] = opcode;
    ptr++;
  }

/*  int dataLength = 5;
  if (dataLength > MAXDATALENGTH) {
    dataLength = MAXDATALENGTH;
  }
*/
  char buf[20];
  int bytesUsed = Z80Disassembler::disassemble(buf, data, MAXDATALENGTH);//dataLength);
  Serial.print(bytesUsed);
  Serial.print(": ");

  for (ptr = 0; ptr < 5; ptr++) {
    if (ptr < bytesUsed) {
      char bbuf[4];
      sprintf(bbuf, "%02x ", data[ptr]);
      Serial.print(bbuf);
    } else {
      Serial.print("   ");
    }
  }
//  Serial.print(bytesUsed);
  Serial.print(buf);
  Serial.println();

  if (opcode == 255) {
    return;
  }
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
 
  byte prefixes[] = {0,0xdd,0xfd};
  byte opcodesets[] = {0,0xed,0xcb};
  for (byte prefix = 0;prefix < 2;prefix++) {
    for (byte opcodeset = 0;opcodeset < 3;opcodeset++) {
    disassembleTable(prefixes[prefix], opcodesets[opcodeset]);
    Serial.println();
    }
    Serial.println();
    Serial.println();
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  pinMode(8, OUTPUT);

  digitalWrite(8, HIGH);
  delay(100);
  digitalWrite(8, LOW);
  delay(100);
}
