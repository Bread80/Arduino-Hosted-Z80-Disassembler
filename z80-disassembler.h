/* 
Arduino hosted Z80 disassembler by Mike Sutton
Bread80.com
*/
#ifndef Z80Disassembler_h
#define Z80Disassembler_h

#include "Arduino.h"

class Z80Disassembler
{
	public:
		Z80Disassembler();
		/*
		* Convert an opcode data to it's text form
		* out = buffer for output text
		* prefix = IX/IY prefix (or 0)
		* opcodeset = CB/ED or 0
		* opcode = opcode (duh!)
		* valuesPassed = number (count) of parameter values passed in
		* value1, value2 = parameter values
		* trimItem = right trim the output?
		* return value = out buffer
		*/
		static int getOpcodeText(char* out, byte prefix, byte opcodeset, byte opcode, 
                    int valuesPassed, byte value1, byte value2, bool trimItem);
		//Disassemble a series of bytes (opcode).
		//buf = buffer for output text
		//opcode = array of bytes to process
		//opcodeLength = Number of bytes passed in opcode parameter
		//Returns the number of bytes consumed from opcode.
		static byte disassemble(char* buf, byte* opcode, int opcodeLength);
		
	private:
		//Copy an item out of a list of strings.
		//buf is the buffer to copy into.
		//list is a list of fixed length text items.
		//index is the index into the list.
		//lengthItem is the length of each item.
		//trimItem = right trim spaces from the item.
		//Returns buf.
		static char* copyTextListItem(char* buf, const char* list, int index, int lengthItem, bool trimItem);
		//Convert an opcode byte into an index into the tableOpcodes table.
		static int opcodeToOpcodeTableIndex(byte prefix,byte opcode);
		//Return asciiz base opcode (opbase) for given index into bases table
		//buf must be at least baseLength+1
		static char* indexOpcodesToOpcodeName(char* buf,int indexOpcode, bool trimItem);
		//Extract a parameter encoded in an opcode.
		//Parameter is then right-shifted to return values 0,1,2,...
		//Returns 255 if no parameter
		static byte getOpcodeParam(byte opcode, int indexOpcode, int paramNo);
		static char* indexOpcodesToParamText(char* buf,byte prefix, byte opcodeset, byte opcode, 
			int indexOpcode, int paramNo, bool trimItem);
		//Returns the number of format substititions needed within a string (i.e. the number of % chars).
		static int countSubstitutionsNeeded(char* buf);
		//Substitutes values in a parameter either for a value (if known) or ** if unknown.
		//inbuf is the parameter text.
		//valuesPassed is the number of parameters passed in.
		//low and high are the possible parameters (low,high: in the same order as in the source code.
		//             If we have one parameter pass it in in low.
		static char* substitute(char* outbuf, char* inbuf, int valuesPassed, byte low, byte high);
};				
			
#endif
