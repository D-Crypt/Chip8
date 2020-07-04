#include "Chip8.h"
#include <fstream>
#include <chrono>

using std::ifstream;
using std::ios;
using std::streampos;
using std::chrono::system_clock;

const unsigned int startAddress = 0x200;      // CHIP-8 instructions begin in memory starting at address 0x200
const unsigned int fontsetSize = 16 * 5;       // 16 characters required, each taking up 5 bytes of memory
const unsigned int fontsetStartAddress = 0x50; // storage space for characters begins at 0x50

uint8_t fontset[fontsetSize] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
	0x20, 0x60, 0x20, 0x20, 0x70, // 1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
	0x90, 0x90, 0xF0, 0x10, 0x10, // 4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
	0xF0, 0x10, 0x20, 0x40, 0x40, // 7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
	0xF0, 0x90, 0xF0, 0x90, 0x90, // A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
	0xF0, 0x80, 0x80, 0x80, 0xF0, // C
	0xE0, 0x90, 0x90, 0x90, 0xE0, // D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
	0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

Chip8::Chip8() : randGen(system_clock::now().time_since_epoch().count()) // Initialise RNG with a seed based on the current time
{
	progCounter = startAddress; // Initialise PC to 0x200

	// Load fonts into memory
	for (unsigned int i = 0; i < fontsetSize; ++i)
	{
		memory[fontsetStartAddress + i] = fontset[i];
	}

	randByte = uniform_int_distribution<uint8_t>(0, 255U); // Initialise random byte to a number between 0 and 255
}

void Chip8::loadROM(char const* filename)
{
	// Open file as a binary stream and move file pointer to end of stream
	ifstream file(filename, ios::binary | ios::ate);

	if (file.is_open())
	{
		// Get size of file, then allocate new buffer to hold its contents
		streampos size = file.tellg();
		char* buffer = new char[size];

		// Return to start of file, fill buffer, then close
		file.seekg(0, ios::beg);
		file.read(buffer, size);
		file.close();

		// Load ROM content into CHIP-8's memory
		for (long i = 0; i < size; ++i)
		{
			memory[i + startAddress] = buffer[i];
		}

		delete[] buffer; // Free up memory from buffer
	}
}

uint8_t Chip8::getVX()
{
	// x is the lowest 4 bits of the high byte of the instruction
	// to retrieve x, perform AND operation with opcode then shift 8 bits to the right to isolate value
	return vx = (opcode & 0x0F00) >> 8;
}

uint8_t Chip8::getKK()
{
	// get lowest byte
	return kk = (opcode & 0x00FF);
}

uint8_t Chip8::getVY()
{
	// y is the highest 4 bits of the low byte of the instruction
	// to retrieve y, perform AND operation with opcode then shift 4 bits to the right to isolate value
	return vy = (opcode & 0x00F0) >> 4;
}

void Chip8::skipInstruction()
{
	progCounter += 2;
}

void Chip8::op_00E0()
{
	memset(video, 0, sizeof(video));
}

void Chip8::op_00EE()
{
	--stackPointer;
	progCounter = stack[stackPointer];
}

void Chip8::op_1NNN()
{
	// Bitwise & used to evaluate both opcode and 0xFFF in binary format 
	// Used to modify address value based on AND operation to prevent writing outside of memory range
	address = opcode & endRAM;

	progCounter = address;
}

void Chip8::op_2NNN()
{
	address = opcode & endRAM;

	stack[stackPointer] = progCounter;
	++stackPointer;
	progCounter = address;
}

void Chip8::op_3XKK()
{
	if (registers[getVX()] == getKK())
	{
		skipInstruction();
	}
}

void Chip8::op_4XKK()
{
	if (registers[getVX()] != getKK())
	{
		skipInstruction();
	}
}

void Chip8::op_5XY0()
{
	if (registers[getVX()] == registers[getVY()])
	{
		skipInstruction();
	}
}

void Chip8::op_6XKK()
{
	registers[getVX()] = getKK();
}

void Chip8::op_7XKK()
{
	registers[getVX()] += getKK();
}

void Chip8::op_8XY0()
{
	registers[getVX()] = registers[getVY()];
}

void Chip8::op_8XY1()
{
	registers[getVX()] |= registers[getVY()];
}