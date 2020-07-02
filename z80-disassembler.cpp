/* 
Arduino hosted Z80 disassembler by Mike Sutton
Bread80.com
*/
#include "Arduino.h"
#include "z80-disassembler.h"


/*
Opcodes table.
For converting an opcode byte into a name, details, parameters etc.
BEWARE: Some opcodes will match more than one entry (i.e. LD (HL),(HL)/HALT and RETI/RETN[1]).
  The entry which comes first in the table is the one which will be found!!!!
  [1] - RETI/RETN issue is because RETN maps to multiple (undocumented) opcodes and using a simple
        mask makes decoding easier but 'overwrites' RETI.

Fields:
Prefix: opcode prefix: CB or ED (or none). IX and IY are handled separately as parameter modifiers.
Opcode: opcode byte. Any bits which are parameterised are zeroed.
Mask:   Apply the mask to an opcode byte (Opcode _AND_ Mask) and if it equals Opcode then we have a match.
        (i.e. the mask removes any parameterised bits).
OpcodeName: Index into the listOpcodeNames list.
OpBase: Index into detail tables and lists for the opcode.
Param1, Param2: Indexes into the detail tables for parameter options (or zero if no parameter).
*/
//#include "z80opcodes.csv";

#define FIELDCOUNT_OPCODES 6
//Field indexes
#define FIELDNO_OPCODES_PREFIX 0
#define FIELDNO_OPCODES_OPCODE 1
#define FIELDNO_OPCODES_MASK 2
#define FIELDNO_OPCODES_NAME 3
#define FIELDNO_OPCODES_PARAM1 4
#define FIELDNO_OPCODES_PARAM2 5
const byte tableOpcodes[] PROGMEM = {
//Prefix,Opcode,Mask,Name,Param1,Param2,,
0,0,255,41,0,0,//,NOP
0,1,207,35,9,22,//,"LD dd,nn"
0,2,255,35,16,3,//,"LD (BC),A"
0,3,207,26,9,0,//,INC rr
0,4,199,26,2,0,//,INC r
0,5,199,14,2,0,//,DEC m
0,6,199,35,2,7,//,"LD r,n"
0,7,255,57,0,0,//,RLCA
0,8,255,19,11,12,//,"EX AF,AF'"
0,9,207,1,14,9,//,"ADD HL,ss"
0,10,255,35,3,16,//,"LD A,(BC)"
0,11,207,14,9,0,//,DEC rr
0,15,255,62,0,0,//,RRCA
0,16,255,16,24,0,//,DJNZ e
0,18,255,35,17,3,//,"LD (DE),A"
0,23,255,55,0,0,//,RLA
0,24,255,34,24,0,//,JR e
0,26,255,35,3,17,//,"LD A,(DE)"
0,31,255,60,0,0,//,RRA
0,32,231,34,26,24,//,"JR cc,e"
0,34,255,35,23,14,//,"LD (nn),HL"
0,39,255,13,0,0,//,DAA
0,42,255,35,14,23,//,"LD HL,(nn)"
0,47,255,12,0,0,//,CPL
0,50,255,35,23,3,//,"LD (nn),A"
0,54,255,35,18,7,//,"LD (HL),n"
0,55,255,66,0,0,//,SCF
0,58,255,35,3,23,//,"LD A,(nn)"
0,63,255,6,0,0,//,CCF
0,118,255,21,0,0,//,HALT
0,64,192,35,2,1,//,"LD r,r'"
0,128,248,1,3,1,//,"ADD A,r"
0,136,248,0,3,1,//,"ADC A,m"
0,144,248,72,3,1,//,SUB m
0,152,248,65,3,1,//,"SBC A,m"
0,160,248,2,3,1,//,AND m
0,168,248,73,3,1,//,XOR m
0,176,248,42,3,1,//,OR m
0,184,248,7,3,1,//,CP m
0,192,199,51,27,0,//,RET cc
0,193,207,48,10,0,//,POP rr
0,194,199,33,27,25,//,"JP cc,pq"
0,195,255,33,25,0,//,JP pq
0,196,199,4,27,25,//,"CALL cc,pq"
0,197,207,49,10,0,//,PUSH rr
0,198,255,1,3,7,//,"ADD A,n"
0,199,199,64,28,0,//,RST p
0,201,255,51,0,0,//,RET
0,205,255,4,25,0,//,CALL pq
0,206,255,0,3,7,//,"ADC A,n"
0,211,255,45,8,3,//,"OUT(N),A"
0,214,255,72,3,7,//,SUB n
0,217,255,20,0,0,//,EXX
0,219,255,25,3,8,//,"IN A,N"
0,222,255,65,3,7,//,"SBC A,n"
0,227,255,19,21,14,//,"EX (SP),HL"
0,230,255,2,3,7,//,AND n
0,233,255,33,18,0,//,JP (HL)
0,235,255,19,13,14,//,"EX DE,HL"
237,74,207,0,14,9,//,"ADC HL,ss"
0,238,255,73,3,7,//,XOR n
0,243,255,15,0,0,//,DI
0,246,255,42,3,7,//,OR n
0,249,255,35,15,14,//,"LD SP,HL"
0,251,255,18,0,0,//,EI
0,254,255,7,3,7,//,CP n
237,65,199,45,4,2,//,"OUT (C ),r"
237,66,207,65,14,9,//,"SBC HL,ss"
237,67,207,35,23,9,//,"LD (nn),dd"
237,68,199,40,0,0,//,NEG
237,77,255,52,0,0,//,RETI
237,71,255,35,5,3,//,"LD I,A"
237,69,199,53,0,0,//,RETN
237,79,255,35,6,3,//,"LD R,A"
237,87,255,35,3,5,//,"LD A,I"
237,95,255,35,3,6,//,"LD A,R"
237,103,255,63,0,0,//,RRD
237,111,255,58,0,0,//,RLD
237,160,255,38,0,0,//,LDI
237,163,255,47,0,0,//,OUTI
237,168,255,36,0,0,//,LDD
237,171,255,46,0,0,//,OUTD
237,176,255,39,0,0,//,LDIR
237,179,255,44,0,0,//,OTIR
237,184,255,37,0,0,//,LDDR
237,187,255,43,0,0,//,OTDR
203,0,248,56,1,30,//,RLC m
203,8,248,61,1,30,//,RRC m
203,16,248,54,1,30,//,RL m
203,24,248,59,1,30,//,RR m
203,32,248,69,1,30,//,SLA m
203,40,248,70,1,30,//,SRA m
203,48,248,68,1,30,//,SL1 m
203,56,248,71,1,30,//,SRL m
203,128,192,50,29,1,//,"RES b,m"
237,64,199,25,2,4,//,"IN r,(C )"
237,70,215,22,0,0,//,IM 0
237,75,207,35,9,23,//,"LD dd,(nn)"
237,86,223,23,0,0,//,IM 1
237,94,223,24,0,0,//,IM 2
237,161,255,10,0,0,//,CPI
237,162,255,29,0,0,//,INI
237,178,255,30,0,0,//,INIR
237,169,255,8,0,0,//,CPD
237,170,255,27,0,0,//,IND
237,177,255,11,0,0,//,CPIR
237,185,255,9,0,0,//,CPDR
237,186,255,28,0,0,//,INDR
203,64,192,3,29,1,//,"BIT b,m"
203,192,192,67,29,1,//,"SET b,m"
0,203,255,5,0,0,//,CB Prefix
0,221,255,31,0,0,//,IX Prefix
0,253,255,32,0,0,//,IY Prefix
0,237,255,17,0,0//,ExtD

};

//Opcode name strings. Indexed from the Name field in tableOpcodes
#define LENGTH_OPCODENAME 4
const char listOpcodeNames[] PROGMEM =
"ADC "//  ADC
"ADD "//  ADD
"AND "//  AND
"BIT "//  BIT
"CALL"//  CALL
"CB.."//  CB..
"CCF "//  CCF
"CP  "//  CP
"CPD "//  CPD
"CPDR"//  CPDR
"CPI "//  CPI
"CPIR"//  CPIR
"CPL "//  CPL
"DAA "//  DAA
"DEC "//  DEC
"DI  "//  DI
"DJNZ"//  DJNZ
"ED.."//  ED..
"EI  "//  EI
"EX  "//  EX
"EXX "//  EXX
"HALT"//  HALT
"IM_0"//  IM 0
"IM_1"//  IM 1
"IM_2"//  IM 2
"IN  "//  IN
"INC "//  INC
"IND "//  IND
"INDR"//  INDR
"INI "//  INI
"INIR"//  INIR
"IX.."//  IX..
"IY.."//  IY..
"JP  "//  JP
"JR  "//  JR
"LD  "//  LD
"LDD "//  LDD
"LDDR"//  LDDR
"LDI "//  LDI
"LDIR"//  LDIR
"NEG "//  NEG
"NOP "//  NOP
"OR  "//  OR
"OTDR"//  OTDR
"OTIR"//  OTIR
"OUT "//  OUT
"OUTD"//  OUTD
"OUTI"//  OUTI
"POP "//  POP
"PUSH"//  PUSH
"RES "//  RES
"RET "//  RET
"RETI"//  RETI
"RETN"//  RETN
"RL  "//  RL
"RLA "//  RLA
"RLC "//  RLC
"RLCA"//  RLCA
"RLD "//  RLD
"RR  "//  RR
"RRA "//  RRA
"RRC "//  RRC
"RRCA"//  RRCA
"RRD "//  RRD
"RST "//  RST
"SBC "//  SBC
"SCF "//  SCF
"SET "//  SET
"SL1 "//  SL1
"SLA "//  SLA
"SRA "//  SRA
"SRL "//  SRL
"SUB "//  SUB
"XOR "//  XOR
;

/* Parameters table, including any substitutions needed
 *
 * This table is referenced by the Param1 and Param2 fields in tableOpcodes
 * Indexing is the same as for listParamFormats
 *
 * Mask = bitwise mask into opcode for the relevant bits
 * Count = The number of substitutions necessary. E.g. n = one byte to be subbed, nn = two bytes to be subbed
 *         but not merely bytes from opcode. E.g. s = bits from opcode itself which encode a register
 * Comments show: Parameter, Substitution text, index (used for creating other tables) and binary version of Mask column
 */
#define FIELDCOUNT_PARAMS 2
#define FIELDNO_PARAMS_MASK 0
#define FIELDNO_PARAMS_SUBCOUNT = 1
const byte tableParams[] PROGMEM = {
//Mask,ParamCount,,,,Index,MaskBinary
0,0,//,,,0,
7,1,//,r210,%s,1,111
56,1,//,r543,%s,2,111000
0,0,//,A,A,3,
0,0,//,(C),(C),4,
0,0,//,I,I,5,
0,0,//,R,R,6,
0,1,//,n,%s,7,
0,1,//,(n),(%s),8,
48,1,//,rp,%s,9,110000
48,1,//,ra,%s,10,110000
0,0,//,AF,AF,11,
0,0,//,AF',AF',12,
0,0,//,DE,DE,13,
0,0,//,HL,HL,14,
0,0,//,SP,SP,15,
0,0,//,(BC),(BC),16,
0,0,//,(DE),(DE),17,
0,0,//,(HL),(HL),18,
0,1,//,(IX+d),(IX+%s),19,
0,1,//,(IY+d),(IY+%s),20,
0,0,//,(SP),(SP),21,
0,2,//,nn,%s%s,22,
0,2,//,(nn),(%s%s),23,
0,1,//,e,%s,24,
0,1,//,pq,%s,25,
24,1,//,cc2,%s,26,11000
56,1,//,cc3,%s,27,111000
56,1,//,p,%s,28,111000
56,1,//,b,%s,29,111000
0,0//,xycb,xycb,30,
  
};

/* Parameter text prior to substitutions
 * Indexing is the same as for tableParams
 * This table is referenced by the Param1 and Param2 fields in tableOpcodes
 */
#define LENGTH_PARAMFORMAT 6
const char listParamFormats[] PROGMEM =
"      "
"%s    "
"%s    "
"A     "
"(C)   "
"I     "
"R     "
"%s    "
"(%s)  "
"%s    "
"%s    "
"AF    "
"AF'   "
"DE    "
"HL    "
"SP    "
"(BC)  "
"(DE)  "
"(HL)  "
".     "
".     "
"(SP)  "
"%s%s  "
"(%s%s)"
"%s    "
"%s%s  "
"%s    "
"%s    "
"%s    "
"%s    "
"xycb  "

;

#define NO_PARAM 255

Z80Disassembler::Z80Disassembler()
{}

//Copy an item out of a list of strings.
//buf is the buffer to copy into.
//list is a list of fixed length text items.
//index is the index into the list.
//lengthItem is the length of each item.
//trimItem = right trim spaces from the item.
//Returns buf.
char* Z80Disassembler::copyTextListItem(char* buf, const char* list, int index, int lengthItem, bool trimItem) {
  int ptrEntry = index * lengthItem;
  for (int offset = 0;offset < lengthItem;offset++) {
    if (!trimItem || (pgm_read_byte_near(list + ptrEntry + offset) != ' ')) {
      buf[offset] = pgm_read_byte_near(list + ptrEntry + offset);
    } else {
      buf[offset] = 0;
    }
  }
  buf[lengthItem] = 0;
  return(buf);  
}

//Convert an opcode byte into an index into the tableOpcodes table.
int Z80Disassembler::opcodeToOpcodeTableIndex(byte prefix,byte opcode) {
  for (int i = 0 ; i < sizeof(tableOpcodes); i += FIELDCOUNT_OPCODES) {
    if (pgm_read_byte_near(tableOpcodes + i + FIELDNO_OPCODES_PREFIX) == prefix) {
      if (pgm_read_byte_near(tableOpcodes + i + FIELDNO_OPCODES_OPCODE) == (opcode & pgm_read_byte_near(tableOpcodes + i + FIELDNO_OPCODES_MASK))) {
        return(i / FIELDCOUNT_OPCODES);
      }
    }
  }

  //Not found
  return(-1);
}

//Return asciiz base opcode (opbase) for given index into bases table
//buf must be at least baseLength+1
char* Z80Disassembler::indexOpcodesToOpcodeName(char* buf,int indexOpcode, bool trimItem){
  //Pointer into Opcodes table for OpcodeName of indexed entry.
  int ptrOpcodes = indexOpcode * FIELDCOUNT_OPCODES + FIELDNO_OPCODES_NAME;
  return(copyTextListItem(buf, listOpcodeNames, pgm_read_byte_near(tableOpcodes + ptrOpcodes), LENGTH_OPCODENAME, trimItem));
}

//Extract a parameter encoded in an opcode.
//Parameter is then right-shifted to return values 0,1,2,...
//Returns 255 if no parameter
byte Z80Disassembler::getOpcodeParam(byte opcode, int indexOpcode, int paramNo) {
  //Pointer into Opcodes table for OpcodeName of indexed entry.
  int ptrOpcodes = indexOpcode * FIELDCOUNT_OPCODES + FIELDNO_OPCODES_PARAM1;
  //Index into params table
  int indexParams = pgm_read_byte_near(tableOpcodes + ptrOpcodes + paramNo);
  //Get mask for (possible) parameter in opcode
  byte mask = pgm_read_byte_near(tableParams + indexParams * FIELDCOUNT_PARAMS + FIELDNO_PARAMS_MASK);
  if (!mask) { return(NO_PARAM); }
  else {
    for (; (mask & 1) == 0; mask >>= 1, opcode >>= 1);
    
    return(opcode & mask);
  } 
}

//Lists of parameterised opcodes and the length of each entry
//8-bit registers
const char listReg8[] PROGMEM = "BCDEHLxA";
#define LENGTH_REG8 1
//16-bit registers
const char listRP[] PROGMEM = "BCDEHLSPAFIXIY";
#define LENGTH_REG16 2
//Indexed registers
const char listIXIYd[] PROGMEM = "(IX+%s)(IY+%s)";
#define LENGTH_IXIYD 7
//Flags (and no, that's not a POP it's PO and PE flags!)
const char listCC[] PROGMEM = "NZZ NCC POPEP M ";
#define LENGTH_FLAGS 2

//Indexes into tableParams
#define REG8_IMPLICIT_210 1  //B/C/D/E/H/L/(HL)/A in bits 2,1,0
#define REG8_IMPLICIT_543 2  //As above in bits 5,4,3
#define REG16_IMPLICIT_SP 9  //BC/DE/HL/SP
#define REG16_IMPLICIT_AF 10 //BC/DE/HL/AF
#define HL_IMPLICIT 14 //HL
#define HL_INDIRECT 18 //(HL)
#define FLAGS_2BIT 26
#define FLAGS_3BIT 27
#define PARAM_RST 28
#define PARAM_BITNO 29
#define CB_INDEXED 30 //

#define MASK_IXIY 32

#define OPCODE_EXDEHL 0xEB
#define OPCODE_IN_C 0x70
#define OPCODE_OUT_C 0x71

#define VALUE_REG8_INDIRECT_HL 6 //(HL) in REG8_IMPLICIT

char* Z80Disassembler::indexOpcodesToParamText(char* buf,byte prefix, byte opcodeset, byte opcode, int indexOpcode, int paramNo, bool trimItem) {
  //If the instruction has an index prefix (DD/FD) usesIXIY will end up true if the instruction is one which can
  //be and has been modified to use an index register. Currently the result is ignored since if a z80 encounters 'invalid' instructions
  //(instructions which can't have a prefix) it (from my tests) simply ignores the prefix and executes as a non-prefixed opcode.
  bool usesIXIY = false;
  
  //Convert IX or IY to a 0,1 index
  int prefixindex = (prefix & MASK_IXIY) >> 5;
  //Pointer into Opcodes table for OpcodeName of indexed entry.
  int ptrOpcodes = indexOpcode * FIELDCOUNT_OPCODES + FIELDNO_OPCODES_PARAM1;
  //Index into params table
  int indexParams = pgm_read_byte_near(tableOpcodes + ptrOpcodes + paramNo);

  byte paramValue = getOpcodeParam(opcode, indexOpcode, paramNo);
  
  if (paramValue == NO_PARAM) {
    //Non parameterised
    copyTextListItem(buf, listParamFormats, indexParams, LENGTH_PARAMFORMAT, trimItem);
    
    //And a few exceptions:
    switch (indexParams) {
      case HL_IMPLICIT:
        if (prefix && !opcodeset) {
          if (opcode != OPCODE_EXDEHL) { //Exclude EX DE,HL
            copyTextListItem(buf, listRP, prefixindex + 5, LENGTH_REG16, false);
            usesIXIY = true;               
          }
        }
        break;
      case HL_INDIRECT: 
        if (prefix && !opcodeset) {
            copyTextListItem(buf+1, listRP, prefixindex + 5, LENGTH_REG16, false);
            //Reinstate the close brace
            buf[3] = ')';
            usesIXIY = true;               
        }
        break;
       //CB shift/rotate opcodes
      case CB_INDEXED:
        //If no prefix we only have first parameter
        //Also if we have a prefix but the other parameter is the (ix/iy+d) one then only one parameter
        if (!prefix || (paramNo = 1 && getOpcodeParam(opcode, indexOpcode, 1-paramNo) == VALUE_REG8_INDIRECT_HL )) {
          buf[0] = 0;
        }
        //Secret: Other cases: second parameter is indexed one
        else {
          copyTextListItem(buf, listIXIYd, prefixindex, LENGTH_IXIYD, false);
          usesIXIY = true;
        }
        break;
      } //switch
    } else { //Parameterised value (ie coded into the opcode)
      
    if (opcodeset == 0xED) {// && !prefix) {
      //Secret: IN -,(C)
      if (opcode == OPCODE_IN_C && paramNo == 0) { strcpy(buf, "-"); return(buf); }
      //Secret: OUT(C),0
      if (opcode == OPCODE_OUT_C && paramNo == 1) { strcpy(buf, "0"); return(buf); }
    }
    
    switch (indexParams) {
      case REG8_IMPLICIT_210: //8 bit register or (HL)
      case REG8_IMPLICIT_543:
        //Convert (HL) to (IX+d) or (IY+d)...
        //unless ED opcodeset
        //Secret: Also, if we have a CB BIT instruction with a prefix then we ignore the parameterised 
        //register and always use (IX+d)/(IY+d).  If we have SET or RES then we have both the register and
        //the index register, as in 'SET 5,D,(IX+d)'. I.e. we have three parameters!
        //Note that most sources display the register before the but other instructions have the bit before
        //the register, so that's what this code does. It's also far easier to code it that way!
        if (((opcodeset != 0xED) && (paramValue == VALUE_REG8_INDIRECT_HL)) || (prefix && (opcodeset == 0xCB) && (opcode >= 0x40))) {
          if (prefix) {
            int bufOffset = 0;
            if (prefix && opcodeset == 0xCB && opcode >= 0x80 && paramValue != VALUE_REG8_INDIRECT_HL) {
              copyTextListItem(buf, listReg8, paramValue, LENGTH_REG8, true);
              buf[1] = ',';
              bufOffset = 2;
            }
            copyTextListItem(buf+bufOffset, listIXIYd, prefixindex, LENGTH_IXIYD, false);
            usesIXIY = true;
          } else {
             strcpy(buf, "(HL)");
          }
        } else
        {
          //Get the value for the other parameter since we can use index modifier twice!
          byte otherParam =  getOpcodeParam(opcode, indexOpcode, 1-paramNo);
    
          //Basic register, but we sub IXH/IXL/IYH/IYL for H/L unless the other parameter is a (IX+d)/(IY+d)
          //Note: No prefix for ED set, and CB set uses indexing for every instruction so can't do this there.
          if ((opcodeset == 0x00) && (prefix && (paramValue == 4 || paramValue == 5) && otherParam != VALUE_REG8_INDIRECT_HL)) {
            //Put the IX/IY in
            copyTextListItem(buf, listRP, prefixindex + 5, LENGTH_REG16, false);
            //Then copy the H/L after
            copyTextListItem(buf + 2, listReg8, paramValue, LENGTH_REG8, true);
            usesIXIY = true;
          } else {
            copyTextListItem(buf, listReg8, paramValue, LENGTH_REG8, true);
          }
        }
        break;
      //16 bit register pair
      case REG16_IMPLICIT_SP: //BC/DE/HL/SP (or IX/IY)
      case REG16_IMPLICIT_AF:  //BC/DE/HL/AF
        //For SP/AF: Add 1 if case 10
        if (paramValue == 3) { paramValue += indexParams-9; }
        //Handle substituting IX/IY for HL
        //(Not if ED opcodeset)
        if ((opcodeset != 0xED) && (paramValue == 2) && prefix) { 
          paramValue = prefixindex + 5;
          usesIXIY = true;
        }
        copyTextListItem(buf, listRP, paramValue, LENGTH_REG16, false);
        break;
      //CC flags for conditional jumps/calls and returns
      case FLAGS_2BIT:  //2 bits
      case FLAGS_3BIT:  //3 bits
        copyTextListItem(buf, listCC, paramValue, LENGTH_FLAGS, true);
        break;
      //RST instructions
      case PARAM_RST:
        sprintf(buf, "%02xh", paramValue * 8);
        break;
      //BIT, RES and SET first parameter: 0..7
      case PARAM_BITNO:
        sprintf(buf, "%d", paramValue);
        break;
      } //switch
  }
/*  if (prefix && !usesIXIY) {
 *  //Invalid prefix, if it matters to you.
    //...
  }
*/  return(buf);
}

//Returns the number of format substititions needed within a string (i.e. the number of % chars).
int Z80Disassembler::countSubstitutionsNeeded(char* buf) {
  int subs = 0;
  for (int i=strlen(buf);i>= 0; i--) {
    if (buf[i] == '%') {
      subs++;
    }
  }
  //Serial.print(subs);
  return(subs);
}

//Substitutes values in a parameter either for a value (if known) or ** if unknown.
//inbuf is the parameter text.
//valuesPassed is the number of parameters passed in.
//low and high are the possible parameters (low,high: in the same order as in the source code.
//             If we have one parameter pass it in in low.
char* Z80Disassembler::substitute(char* outbuf, char* inbuf, int valuesPassed, byte low, byte high) {
  char strlow[4] = "**";
  char strhigh[4] = "**";

  if (valuesPassed >= 1) {
    sprintf(strlow, "%02Xh", low); 
  }
  if (valuesPassed >= 2) {
    sprintf(strhigh, "%02X", high); 
  }

  //How many parameters is our parameter expecting?
  switch (countSubstitutionsNeeded(inbuf)) {
  case 0:
    strcpy(outbuf, inbuf);
    break;
  case 1: 
    sprintf(outbuf, inbuf, strlow);
    break;
  case 2:
    sprintf(outbuf, inbuf, strhigh, strlow);
  }
  return(outbuf);
}

/*
 * Convert an opcode data to it's text form
 * out = buffer for output text
 * prefix = IX/IY prefix (or 0)
 * opcodeset = CB/ED or 0
 * opcode = opcode (duh!)
 * valuesPassed = number (count) ot parameter values passed in
 * value1, value2 = parameter values
 * trimItem = right trim the output?
 * return value = out buffer
 */
int Z80Disassembler::getOpcodeText(char* out, byte prefix, byte opcodeset, byte opcode, 
                    int valuesPassed, byte value1, byte value2, bool trimItem) {
  //Get table entry for our opcode
  int indexOpcodes = opcodeToOpcodeTableIndex(opcodeset, opcode);

  if (indexOpcodes == -1) {
    out[0] = 0;
    return(0);
  } else {
    char opcodeText[5];
    char param1Text[8];
    char param2Text[10];
    indexOpcodesToOpcodeName(opcodeText, indexOpcodes, trimItem);
    indexOpcodesToParamText(param1Text, prefix, opcodeset, opcode, indexOpcodes, 0, trimItem);
    indexOpcodesToParamText(param2Text, prefix, opcodeset, opcode, indexOpcodes, 1, trimItem);

    int subs1 = countSubstitutionsNeeded(param1Text);
    int subs2 = countSubstitutionsNeeded(param2Text);
    char param1out[8];
    char param2out[10];
    substitute(param1out, param1Text, valuesPassed, value1, value2);
    int param2valuesPassed = valuesPassed - subs1;
    if (param2valuesPassed < 0) {
      param2valuesPassed = 0;
    }
    
    switch (subs1) {
      case 0: substitute(param2out, param2Text, param2valuesPassed, value1, value2);
        break;
      case 1: substitute(param2out, param2Text, param2valuesPassed, value2, 0);
        break;
      default:
        substitute(param2out, param2Text, 0, 0, 0);
    }

    char comma = param2Text[0] == 0 ? ' ' : ',';
    sprintf(out, "%s %s%c%s", opcodeText, param1out, comma, param2out);
    return(subs1 + subs2);
  }
}

//Disassemble a series of bytes (opcode).
//buf = buffer for output text
//opcode = array of bytes to process
//opcodeLength = Number of bytes passed in opcode parameter
//Returns the number of bytes consumed from opcode.
byte Z80Disassembler::disassemble(char* buf, byte* opcode, int opcodeLength) {
  byte prefix = 0;
  byte opcodeset = 0;
  byte opcodebyte = 0;
  int opcodePtr = 0;
  char prefixStr[4];
  prefixStr[0] = 0;
  
  if (opcodeLength == 0) {
    strcpy(buf, "No opcode!");
    return(0);
  }
  
  //IX or IY prefix
  if (opcode[opcodePtr] == 0xDD || opcode[opcodePtr] == 0xFD) {
    prefix = opcode[opcodePtr];
    opcodePtr++;

    switch (prefix) {
    case 0xDD: strcpy(prefixStr, "IX+");
               break;
    case 0xFD: strcpy(prefixStr, "IY+");
    }
  }

  if (opcodePtr >= opcodeLength || opcode[opcodePtr] == 0xDD || opcode[opcodePtr] == 0xFD) {
    sprintf(buf, "<%sNo opcode!>", prefixStr);
    return(opcodePtr);
  }

  //Opcodeset CB, ED or none (default)
  if (opcode[opcodePtr] == 0xCB || opcode[opcodePtr] == 0xED) {
    opcodeset = opcode[opcodePtr];
    opcodePtr++;
  }
  
  if (opcodePtr >= opcodeLength) {
    switch (opcodeset) {
      case 0xCB: sprintf(buf, "<%sCB opcodeset>", prefixStr);
                 break;
      case 0xED: sprintf(buf, "<%sED opcodeset>", prefixStr);
                 break;
      default: 
        sprintf(buf, "<%sNo opcode!>", prefixStr);
    }
    return(opcodePtr);
  }

  //CB wth a prefix = fixed length and offset before opcode!!
  if (opcodeset == 0xCB && prefix) {
    //All opcodes are 4 bytes
    if (opcodeLength < 4) {
      sprintf(buf, "<%sCB+d Incomplete opcode!>", prefixStr);
    }
    else {
      getOpcodeText(buf, prefix, opcodeset, opcode[opcodePtr + 1], 1, opcode[opcodePtr], 0, true);
    }
    return(min(4, opcodeLength));
  }

  //Other opcodes
  int values = 0;
  byte value1 = 0;
  byte value2 = 0;
  
    opcodebyte = opcode[opcodePtr];
    opcodePtr++;
    if (opcodePtr < opcodeLength) {
      values++;
      value1 = opcode[opcodePtr];
    }
    if (opcodePtr+1 < opcodeLength) {
      values++;
      value2 = opcode[opcodePtr+1];
    }    
    int valuesUsed = getOpcodeText(buf, prefix, opcodeset, opcodebyte, values, value1, value2, true);
    if (!buf[0]) {
      strcpy(buf, "<No opcode>");
    }

    return(min(opcodePtr + valuesUsed, opcodeLength));
}
