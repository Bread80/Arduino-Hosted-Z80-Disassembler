Z80 disassembler for Arduino by Mike Sutton
===========================================
http://bread80.com
Twitter: @WingsUnfurling

Written as part of the Bread-80 Project.

Introduction
------------
This project contains C++ source code of a Z80 disassembler ready to run on an Arduino.
This could would be useful if connecting a Z80 to an Arduino, either to feed code to the Z80, to test and analyse the Z80 or to act as a monitor for the Z80. This code is purely the disassembler and does not include any code to talk to a Z80 nor to define how any such connection would be made.

This code works on and outputs single data for single opcodes only. It make to attempt, for example, to generate target link addresses for relative jumps or to create labels for the targets of jumps and calls.

Usage
-----
The main public facing function is disassemble()
There are two ways to use this function
a) Feed it a long(ish) array of bytes. It will consume as many as are needed by 
   the opcode and return the textual opcode and the number of bytes consumed.
b) Feed it a partial array of bytes plus the count of bytes in the array, e.g 
   from a stream of data being read from or written to a data source or a data 
   bus.
   If there are enough bytes for a complete opcode then you will get the same 
   outputs as method a).
   If not it will return a partial opcode with memonics instead of parameters, 
   e.g LD A,** or LD HL,**34, or a suitable message if, for example, you only 
   sent a prefix.
   Again it will return the number of bytes consumed. If this is less than the 
   number of bytes sent in then the opcode has been completely disassembled 
   (and yes, there could be a better way to handle this).
   
   
Notes
-----
This should work for every instruction including all undocumented ones.
What follows has not been 100% extensively tested so do not rely on it.
The names and formats for some undocumented instructions is different to that 
published elsewhere, namely
(Name changes (SLL/SL1) are because I prefer this style, format changes 
(IX/IY CB) are because doing it properly would need a major rewrite which I 
can't be bothered doing for some obscure, undocumented, stuff which hardy 
anybody uses anyway).
Also IM 0/1/2 instructions contain an underscore because they aren't actually 
parameterised and something here can't handle a space in an opcode name.
 
I always welcome comments, bug fixes and constructive criticism.
I'm not normally a C/C++ coder so please be gentle with any inefficient code, 
bad coding style or general crapness in what follows.
 
Follow the link above for related information, original sources and other stuff 
such as the CSV and Excel files used to generate the data tables included below.


   
WARNING: When the Z80 encouters an opcode with an IX or IY prefix which does 
not use the IX/IY registers (technically an illegal opcode) it processes it 
as per the normal, non-prefixed, opcode. This code duplicates that behaviour 
and returns the opcode from the base opcode table.
However, illegal opcodes in the ED extended instruction set are simply returned 
as illegal opcode. This may or may not mirror actual Z80 behaviour (i.e whether 
it exectues an effective NOP for these instructions).
The code doesn't have a specific mechanism for return an 'illegal opcode' 
status, but you can test for this by testing whether the first char in the 
returned buffer is a '<'.
 
Files
-----
z80-disassembler.cpp and .h are the actual code
z80-arduino-disassembler is the example sketch which outputs lists of opcodes.
SampeOutput text files show typical output of z80-arduino-disassembler. Take a 
look if you want to see what this code does.
the Data directory contains:
- an Excel spreadsheet which was used to analyse the Z80 opcodes and produce 
the data tables used by the code.
- CSV files exported from the above Excel spreadsheet ready to be cut and pasted
into the code (or #included - couldn't get that part working).