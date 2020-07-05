#include "Chip8.h"
#include <fstream>
#include <chrono>

using std::ifstream;
using std::ios;
using std::streampos;
using std::chrono::system_clock;

const unsigned int startAddress = 0x200;       // CHIP-8 instructions begin in memory starting at address 0x200
const unsigned int fontsetSize = 16 * 5;       // 16 characters required, each taking up 5 bytes of memory
const unsigned int fontsetStartAddress = 0x50; // Storage space for characters begins at 0x50

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
	vf = registers[0xF];
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
	// To retrieve x, perform AND operation with opcode then shift 8 bits to the right to isolate value
	return vx = (opcode & 0x0F00) >> 8;
}

uint8_t Chip8::getKK()
{
	// Get lowest byte
	return kk = (opcode & 0x00FF);
}

uint8_t Chip8::getVY()
{
	// y is the highest 4 bits of the low byte of the instruction
	// To retrieve y, perform AND operation with opcode then shift 4 bits to the right to isolate value
	return vy = (opcode & 0x00F0) >> 4;
}

uint16_t Chip8::getSum()
{
	return sum = registers[getVX()] + registers[getVY()];
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

void Chip8::op_8XY2()
{
	registers[getVX()] &= registers[getVY()];
}

void Chip8::op_8XY3()
{
	registers[getVX()] ^= registers[getVY()];
}

void Chip8::op_8XY4()
{
	if (getSum() > 255) // Larger than a byte
	{
		registers[vf] = 1;
	}
	else
	{
		registers[vf] = 0;
	}

	registers[getVX()] = sum & 0x00FF;
}

void Chip8::op_8XY5()
{
	if (registers[getVX()] > registers[getVY()])
	{
		registers[vf] = 1;
	}
	else
	{
		registers[vf] = 0;
	}

	registers[getVX()] -= registers[getVY()];
}

void Chip8::op_8XY6()
{
	// Checks if least-significant bit of VX is 1
	registers[vf] = (registers[getVX()] & 0x1);

	// Shifting all bits to the right is equivalent to dividing by 2
	registers[getVX()] >>= 1;
}

void Chip8::op_8XY7()
{
	if (registers[getVY()] > registers[getVX()])
	{
		registers[vf] = 1;
	}
	else
	{
		registers[vf] = 0;
	}

	registers[getVX()] = registers[getVY()] - registers[getVX()];
}

void Chip8::op_8XYE()
{
	// 0x80 in binary form is 10000000, which can be used to retrieve the most-significant bit
	registers[vf] = (registers[getVX()] & 0x80) >> 7;

	// Shifting all bits to the left is equivalent to multiplying by 2
	registers[getVX()] <<= 1;
}

void Chip8::op_9XY0()
{
	if (registers[getVX()] != registers[getVY()])
	{
		skipInstruction();
	}
}

void Chip8::op_ANNN()
{
	// NNN = 12 bits
	address = opcode & 0x0FFF;

	index = address;
}

void Chip8::op_BNNN()
{
	address = opcode & 0X0FFF;

	progCounter = address + registers[0];
}

void Chip8::op_CXKK()
{
	registers[getVX()] = randByte(randGen) & getKK();
}

void Chip8::op_DXYN()
{
	int width = 8; // All sprites are 8 pixels wide
	int height = opcode & 0X000F; // Value set to last nibble (4 bits) of opcode, as specified in documentation
	int videoWidth = 64;
	int videoHeight = 32;

	// Used to wrap around screen boundaries
	uint8_t xPos = registers[getVX()] % videoWidth;
	uint8_t yPos = registers[getVY()] % videoHeight;

	registers[vf] = 0; // Will be set to 1 if pixels are erased

	for (int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = memory[index + row]; // Reference states start at index when reading sprites from memory

		for (int col = 0; col < width; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80 >> col);
			uint32_t* screenPixel = &video[(yPos + row) * videoWidth + (xPos + col)];

			if (spritePixel != 0) // If sprite pixel is on
			{
				if (*screenPixel == 0xFFFFFFFF) // If screen pixel is on, there is a collision
				{
					registers[vf] = 1;
				}

				*screenPixel ^= 0xFFFFFFFF; // XOR with sprite pixel
			}
		}
	}
}

void Chip8::op_EX9E()
{
	uint8_t key = registers[getVX()];

	if (keypad[key])
	{
		skipInstruction();
	}
}