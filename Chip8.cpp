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

	randByte = uniform_int_distribution<int>(0, 255U); // Initialise random byte to a number between 0 and 255
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

void Chip8::op_EXA1()
{
	uint8_t key = registers[getVX()];

	if (!keypad[key])
	{
		skipInstruction();
	}
}

void Chip8::op_FX07()
{
	registers[getVX()] = delayTimer;
}

void Chip8::op_FX0A()
{
	if (keypad[0])
	{
		registers[getVX()] = 0;
	}
	else if (keypad[1])
	{
		registers[getVX()] = 1;
	}
	else if (keypad[2])
	{
		registers[getVX()] = 2;
	}
	else if (keypad[3])
	{
		registers[getVX()] = 3;
	}
	else if (keypad[4])
	{
		registers[getVX()] = 4;
	}
	else if (keypad[5])
	{
		registers[getVX()] = 5;
	}
	else if (keypad[6])
	{
		registers[getVX()] = 6;
	}
	else if (keypad[7])
	{
		registers[getVX()] = 7;
	}
	else if (keypad[8])
	{
		registers[getVX()] = 8;
	}
	else if (keypad[9])
	{
		registers[getVX()] = 9;
	}
	else if (keypad[10])
	{
		registers[getVX()] = 10;
	}
	else if (keypad[11])
	{
		registers[getVX()] = 11;
	}
	else if (keypad[12])
	{
		registers[getVX()] = 12;
	}
	else if (keypad[13])
	{
		registers[getVX()] = 13;
	}
	else if (keypad[14])
	{
		registers[getVX()] = 14;
	}
	else if (keypad[15])
	{
		registers[getVX()] = 15;
	}
	else
	{
		progCounter -= 2; // Decrementing program counter by 2 is equivalent to running the same instruction repeatedly
	}
}

void Chip8::op_FX15()
{
	delayTimer = registers[getVX()];
}

void Chip8::op_FX18()
{
	soundTimer = registers[getVX()];
}

void Chip8::op_FX1E()
{
	index += registers[getVX()];
}

void Chip8::op_FX29()
{
	uint8_t digit = registers[getVX()];

	// The index is initialised using the start address (0x50) of the fontset as an offset; digit is multiplied by 5 as each sprite is 5 bytes large
	index = fontsetStartAddress + (digit * 5);
}

void Chip8::op_FX33()
{
	uint8_t value = registers[getVX()];

	// Using modulo operator allows isolation of right-most digit
	// Dividing by 10 after each iteration will truncate any decimal numbers

	memory[index + 2] = value % 10; // ones digit
	value /= 10;

	memory[index + 1] = value % 10; // tens digit
	value /= 10;

	memory[index] = value % 10;     // hundreds digit
}

void Chip8::op_FX55()
{
	for (int i = 0; i <= getVX(); ++i)
	{
		memory[index + i] = registers[i];
	}
}

void Chip8::op_FX65()
{
	for (int i = 0; i <= getVX(); ++i)
	{
		registers[i] = memory[index + i];
	}
}

void Chip8::executeOpcode()
{
	switch (opcode & 0xF000)
	{
	case 0x0000:
		switch (opcode)
		{
		case 0x00E0:
			op_00E0();
			break;
		case 0x00EE:
			op_00EE();
			break;
		}
		
		break;
	case 0x1000:
		op_1NNN();
		break;
	case 0x2000:
		op_2NNN();
		break;
	case 0x3000:
		op_3XKK();
		break;
	case 0x4000:
		op_4XKK();
		break;
	case 0x5000:
		op_5XY0();
		break;
	case 0x6000:
		op_6XKK();
		break;
	case 0x7000:
		op_7XKK();
		break;
	case 0x8000:
		switch (opcode & 0xF)
		{
		case 0x0:
			op_8XY0();
			break;
		case 0x1:
			op_8XY1();
			break;
		case 0x2:
			op_8XY2();
			break;
		case 0x3:
			op_8XY3();
			break;
		case 0x4:
			op_8XY4();
			break;
		case 0x5:
			op_8XY5();
			break;
		case 0x6:
			op_8XY6();
			break;
		case 0x7:
			op_8XY7();
			break;
		case 0xE:
			op_8XYE();
			break;
		}

		break;
	case 0x9000:
		op_9XY0();
		break;
	case 0xA000:
		op_ANNN();
		break;
	case 0xB000:
		op_BNNN();
		break;
	case 0xC000:
		op_CXKK();
		break;
	case 0xD000:
		op_DXYN();
		break;
	case 0xE000:
		switch (opcode & 0xFF)
		{
		case 0x9E:
			op_EX9E();
			break;
		case 0xA1:
			op_EXA1();
			break;
		}

		break;
	case 0xF000:
		switch (opcode & 0xFF)
		{
		case 0x07:
			op_FX07();
			break;
		case 0x0A:
			op_FX0A();
			break;
		case 0x15:
			op_FX15();
			break;
		case 0x18:
			op_FX18();
			break;
		case 0x1E:
			op_FX1E();
			break;
		case 0x29:
			op_FX29();
			break;
		case 0x33:
			op_FX33();
			break;
		case 0x55:
			op_FX55();
			break;
		case 0x65:
			op_FX65();
			break;
		}

		break;
	}
}

void Chip8::cycle()
{
	// Fetch the next instruction in the form of an opcode
	// Shifting the first piece of memory to the left by 8 bits makes it 8 bytes long
	// By being 8 bytes long, the second piece of memory can now be added to the first with inclusive OR |
	// e.g. 0x10 << 8 = 0x1000; 0x1000 | 0xF0 = 0x10F0
	opcode = (memory[progCounter] << 8) | memory[progCounter + 1];

	skipInstruction();

	executeOpcode();

	if (delayTimer > 0)
	{
		--delayTimer;
	}

	if (soundTimer > 0)
	{
		--soundTimer;
	}
};