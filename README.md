# Chip8

Chip-8 emulator implemented in C++.

## How to run

1. Open project in Visual Studio.
2. Debug -> Chip8VS Debug Properties
3. Configuration Properties -> Debugging
4. Command Arguments -> "10 1 test_opcode.ch8"

Replace the command arguments with a different .ch8 ROM name to test other ROMs.

![Tetris](https://user-images.githubusercontent.com/12638378/116236031-76f92e80-a756-11eb-9384-6775fa414595.png)

## Currently bugged

* Despite the opcode test ROM passing (as shown below), games are currently not functional.
* There seems to be an issue with the draw opcode, specifically that each object has no hit detection and moves erratically compared to expected behaviour. 
* The actual draw function seems to be implemented correctly; most likely the issue stems from incorrect variable/pointer management.

![Opcode-test](https://user-images.githubusercontent.com/12638378/116235403-bb37ff00-a755-11eb-8cae-61c98b69ee85.png)
