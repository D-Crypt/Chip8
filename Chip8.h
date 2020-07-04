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
	uint16_t address;
	uint8_t vx;              // V = register, x = variable which is 4 bits (nibble)
	uint8_t kk;              // lowest 8 bits (byte) of an instruction
	const uint16_t endRAM = 0x0FFF; // 0x0FFF is end of CHIP-8 RAM
	default_random_engine randGen; // Random Number Generator
	uniform_int_distribution<uint8_t> randByte; // Random byte

	void loadROM(char const* filename); // filename is a pointer to a const char
	uint8_t getVX();
	uint8_t getKK();

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
	void op_8XY1(); // Set VX = VX | VY
	void op_8XY2(); // Set VX = VX & VY
	void op_8XY3(); // Set VX = VX ^ (XOR) VY
	void op_8XY4(); // Set VX += VY, set VF = carry
	void op_8XY5(); // Set VX -= VY, set VF = NOT borrow
	void op_8XY6(); // Set VX = VX SHR 1
	void op_8XY7(); // Set VX = VY - VX, set VF = NOT borrow
	void op_8XYE(); // Set VX = VX SHL 1
	void op_9XY0(); // Skip next instruction if VX != VY
	void op_ANNN(); // Set I = NNN
	void op_BNNN(); // Jump to location NNN + V0
	void op_CXKK(); // Set VX = random byte AND KK
	void op_DXYN(); // Display n-byte sprite starting at memory location I at (VX, VY), set VF = collision
	void op_EX9E(); // Skip next instruction if key with the value of VX is pressed
	void op_EXA1(); // Skip next instruction if key with the value of VX is not pressed
	void op_FX07(); // Set VX = delay timer value
	void op_FX0A(); // Wait for a key press, store the value of the key in VX
	void op_FX15(); // Set delay timer = VX
	void op_FX18(); // Set sound timer = VX
	void op_FX1E(); // Set I += VX
	void op_FX29(); // Set I = location of sprite for digit VX
	void op_FX33(); // Store BCD representation of VX in memory locations I, I+1 and I+2
	void op_FX55(); // Store registers V0 through VX in memory starting at location I
	void op_FX65(); // Read registers V0 through VX from memory starting at location I

};

