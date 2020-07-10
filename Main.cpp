#include "Platform.h"
#include "Chip8.h"
#include <chrono>
#include <iostream>
#include <string>

using std::cerr;
using std::exit;
using std::stoi;
using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::milliseconds;

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		cerr << "Usage: " << argv[0] << "<Scale> <Delay> <ROM>\n";
		exit(EXIT_FAILURE);
	}

	int videoScale = stoi(argv[1]);
	int cycleDelay = stoi(argv[2]);
	char const* romFilename = argv[3];

	Platform platform("CHIP-8 Emulator", 64 * videoScale, 32 * videoScale, 64, 32);

	Chip8 chip8;
	chip8.loadROM(romFilename);

	int videoPitch = sizeof(chip8.video[0]) * 64;

	auto lastCycleTime = high_resolution_clock::now();
	bool quit = false;

	while (!quit)
	{
		quit = platform.processInput(chip8.keypad);

		auto currentTime = high_resolution_clock::now();
		float dt = duration<float, milliseconds::period>(currentTime - lastCycleTime).count();

		if (dt > cycleDelay)
		{
			lastCycleTime = currentTime;

			chip8.cycle();

			platform.update(chip8.video, videoPitch);
		}
	}

	return 0;
}