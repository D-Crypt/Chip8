#pragma once
#include <cstdint>
#include <random>

using std::default_random_engine;
using std::uniform_int_distribution;

class Chip8
{
public:
	Chip8();                 // Class contructor
	uint8_t registers[16];   // 16 8-bit registers named V0 to VF
	uint8_t memory[4096];    // 4k bytes of memory
	uint16_t index;	         // 16-bit index register
	uint16_t progCounter;    // 16-bit program counter that holds the address of the next instruction to execute
	uint16_t stack[16];      // 16 levels of stack required
	uint8_t stackPointer;    // 8-bit pointer to track where values are placed in the stack
	uint8_t delayTimer;      // 8-bit timer used for emulation cycles
	uint8_t soundTimer;      // 8-bit timer used for sound emission
	uint8_t keypad[16];      // 16 input keys for controlling the system
	uint32_t video[64 * 32]; // Memory buffer used for storing graphics (64 pixels wide, 32 pixels tall)
	uint16_t opcode;         // Operation code that specifies what instruction to be performed	
	default_random_engine randGen; // Random Number Generator
	uniform_int_distribution<uint8_t> randByte; // Random byte

	void loadROM(char const* filename); // filename is a pointer to a const char

	// Below is a list of the 34 instructions executed by CHIP-8 with their accompanying hexadecimal values:
	void op_00E0(); // CLS: Clear the display
	void op_00EE(); // RET: Return from a subroutine
	void op_1NNN(); // JUMP: Jump to location NNN
	void op_2NNN(); // CALL: Call a subroutine
	void op_3XKK(); // Skip next instruction if VX == KK
	void op_4XKK(); // Skip next instruction if VX != KK
	void op_5XY0(); // Skip next instruction if VX == VY
	void op_6XKK(); // Set VX = KK
	void op_7XKK(); // Set VX += KK
	void op_8XY0(); // Set VX = VY
	void op_8XY1(); // Set VX = VX || VY
	void op_8XY2(); // Set VX = VX && VY
	void op_8XY3(); // Set VX = VX ^ (XOR) VY
	void op_8XY4(); // Set VX += VY, set VF = carry
	void op_8XY5(); // Set VX -= VY, set VF = NOT borrow
	void op_8XY6(); // 
};

