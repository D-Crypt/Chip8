#include "Chip8.h"
#include <fstream>

const unsigned int start_address = 0x200; // CHIP-8 instructions begin in memory starting at address 0x200

void Chip8::loadROM(char const* filename)
{
	// Open file as a binary stream and move file pointer to end of stream
	std::ifstream file(filename, std::ios::binary | std::ios::ate);

	if (file.is_open())
	{
		// Get size of file, then allocate new buffer to hold its contents
		std::streampos size = file.tellg();
		char* buffer = new char[size];

		// Return to start of file, fill buffer, then close
		file.seekg(0, std::ios::beg);
		file.read(buffer, size);
		file.close();

		// Load ROM content into CHIP-8's memory
		for (long i = 0; i < size; ++i)
		{
			memory[i + start_address] = buffer[i];
		}

		delete[] buffer;
	}
}

Chip8::Chip8()
{
	
}