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

/*uint8_t Chip8::getVX()
{
	// x is the lowest 4 bits of the high byte of the instruction
	// To retrieve x, perform AND operation with opcode then shift 8 bits to the right to isolate value
	return vx = (opcode & 0x0F00) >> 8;
}*/

/*uint8_t Chip8::getKK()
{
	// Get lowest byte
	return kk = (opcode & 0x00FF);
}*/

/*uint8_t Chip8::getVY()
{
	// y is the highest 4 bits of the low byte of the instruction
	// To retrieve y, perform AND operation with opcode then shift 4 bits to the right to isolate value
	return vy = (opcode & 0x00F0) >> 4;
}*/

/*uint16_t Chip8::getSum()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	return sum = registers[vx] + registers[vy];
}*/

/*void Chip8::skipInstruction()
{
	progCounter += 2;
}*/

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
	uint16_t address = opcode & 0x0FFF;

	progCounter = address;
}

void Chip8::op_2NNN()
{
	uint16_t address = opcode & 0x0FFF;

	stack[stackPointer] = progCounter;
	++stackPointer;
	progCounter = address;
}

void Chip8::op_3XKK()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t kk = (opcode & 0x00FF);

	if (registers[vx] == kk)
	{
		progCounter += 2;
	}
}

void Chip8::op_4XKK()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t kk = (opcode & 0x00FF);

	if (registers[vx] != kk)
	{
		progCounter += 2;
	}
}

void Chip8::op_5XY0()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	if (registers[vx] == registers[vy])
	{
		progCounter += 2;
	}
}

void Chip8::op_6XKK()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t kk = (opcode & 0x00FF);

	registers[vx] = kk;
}

void Chip8::op_7XKK()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t kk = (opcode & 0x00FF);

	registers[vx] += kk;
}

void Chip8::op_8XY0()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	registers[vx] = registers[vy];
}

void Chip8::op_8XY1()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	registers[vx] |= registers[vy];
}

void Chip8::op_8XY2()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	registers[vx] &= registers[vy];
}

void Chip8::op_8XY3()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	registers[vx] ^= registers[vy];
}

void Chip8::op_8XY4()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	uint16_t sum = registers[vx] + registers[vy];


	if (sum > 255) // Larger than a byte
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[vx] = sum & 0xFF;
}

void Chip8::op_8XY5()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	if (registers[vx] > registers[vy])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[vx] -= registers[vy];
}

void Chip8::op_8XY6()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;

	// Checks if least-significant bit of VX is 1
	registers[0xF] = (registers[vx] & 0x1);

	// Shifting all bits to the right is equivalent to dividing by 2
	registers[vx] >>= 1;
}

void Chip8::op_8XY7()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	if (registers[vy] > registers[vx])
	{
		registers[0xF] = 1;
	}
	else
	{
		registers[0xF] = 0;
	}

	registers[vx] = registers[vy] - registers[vx];
}

void Chip8::op_8XYE()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	// 0x80 in binary form is 10000000, which can be used to retrieve the most-significant bit
	registers[0xF] = (registers[vx] & 0x80) >> 7;

	// Shifting all bits to the left is equivalent to multiplying by 2
	registers[vx] <<= 1;
}

void Chip8::op_9XY0()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;

	if (registers[vx] != registers[vy])
	{
		progCounter += 2;
	}
}

void Chip8::op_ANNN()
{
	// NNN = 12 bits
	uint16_t address = opcode & 0x0FFF;

	index = address;
}

void Chip8::op_BNNN()
{
	uint16_t address = opcode & 0X0FFF;

	progCounter = registers[0] + address;
}

void Chip8::op_CXKK()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t kk = (opcode & 0x00FF);

	registers[vx] = randByte(randGen) & kk;
}

void Chip8::op_DXYN()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t vy = (opcode & 0x00F0) >> 4;
	uint8_t height = opcode & 0X000Fu; // Value set to last nibble (4 bits) of opcode, as specified in documentation

	//const int width = 8; // All sprites are 8 pixels wide
	const unsigned int videoWidth = 64;
	const unsigned int videoHeight = 32;

	// Used to wrap around screen boundaries
	uint8_t xPos = registers[vx] % videoWidth;
	uint8_t yPos = registers[vy] % videoHeight;

	registers[0xF] = 0; // Will be set to 1 if pixels are erased

	for (int row = 0; row < height; ++row)
	{
		uint8_t spriteByte = memory[index + row]; // Reference states start at index when reading sprites from memory

		for (int col = 0; col < 8; ++col)
		{
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &video[(yPos + row) * videoWidth + (xPos + col)];

			if (spritePixel) // If sprite pixel is on
			{
				if (*screenPixel == 0xFFFFFFFF) // If screen pixel is on, there is a collision
				{
					registers[0xF] = 1;
				}

				*screenPixel ^= 0xFFFFFFFF; // XOR with sprite pixel
			}
		}
	}
}

void Chip8::op_EX9E()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;

	uint8_t key = registers[vx];

	if (keypad[key])
	{
		progCounter += 2;
	}
}

void Chip8::op_EXA1()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;

	uint8_t key = registers[vx];

	if (!keypad[key])
	{
		progCounter += 2;
	}
}

void Chip8::op_FX07()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;

	registers[vx] = delayTimer;
}

void Chip8::op_FX0A()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;

	if (keypad[0])
	{
		registers[vx] = 0;
	}
	else if (keypad[1])
	{
		registers[vx] = 1;
	}
	else if (keypad[2])
	{
		registers[vx] = 2;
	}
	else if (keypad[3])
	{
		registers[vx] = 3;
	}
	else if (keypad[4])
	{
		registers[vx] = 4;
	}
	else if (keypad[5])
	{
		registers[vx] = 5;
	}
	else if (keypad[6])
	{
		registers[vx] = 6;
	}
	else if (keypad[7])
	{
		registers[vx] = 7;
	}
	else if (keypad[8])
	{
		registers[vx] = 8;
	}
	else if (keypad[9])
	{
		registers[vx] = 9;
	}
	else if (keypad[10])
	{
		registers[vx] = 10;
	}
	else if (keypad[11])
	{
		registers[vx] = 11;
	}
	else if (keypad[12])
	{
		registers[vx] = 12;
	}
	else if (keypad[13])
	{
		registers[vx] = 13;
	}
	else if (keypad[14])
	{
		registers[vx] = 14;
	}
	else if (keypad[15])
	{
		registers[vx] = 15;
	}
	else
	{
		progCounter -= 2; // Decrementing program counter by 2 is equivalent to running the same instruction repeatedly
	}
}

void Chip8::op_FX15()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;

	delayTimer = registers[vx];
}

void Chip8::op_FX18()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;

	soundTimer = registers[vx];
}

void Chip8::op_FX1E()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;

	index += registers[vx];
}

void Chip8::op_FX29()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t digit = registers[vx];

	// The index is initialised using the start address (0x50) of the fontset as an offset; digit is multiplied by 5 as each sprite is 5 bytes large
	index = fontsetStartAddress + (5 * digit);
}

void Chip8::op_FX33()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;
	uint8_t value = registers[vx];

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
	uint8_t vx = (opcode & 0x0F00) >> 8;

	for (int i = 0; i <= vx; ++i)
	{
		memory[index + i] = registers[i];
	}
}

void Chip8::op_FX65()
{
	uint8_t vx = (opcode & 0x0F00) >> 8;

	for (int i = 0; i <= vx; ++i)
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

	progCounter += 2;

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