#pragma once
#include <cstdint>

class Chip8
{
public:
	uint8_t registers[16];   //16 8-bit registers named V0 to VF
	uint8_t memory[4096];    //4k bytes of memory
	uint16_t index;		     //16-bit index register
	uint16_t progCounter;    //16-bit program counter that holds the address of the next instruction to execute
	uint16_t stack[16];      //16 levels of stack required
	uint8_t stackPointer;    //8-bit pointer to track where values are placed in the stack
	uint8_t delayTimer;      //8-bit timer used for emulation cycles
	uint8_t soundTimer;      //8-bit timer used for sound emission
	uint8_t keypad[16];      //16 input keys for controlling the system
	uint32_t video[64 * 32]; //memory buffer used for storing graphics (64 pixels wide, 32 pixels tall)
	uint16_t opcode;         //operation code that specifies what instruction to be performed
};

