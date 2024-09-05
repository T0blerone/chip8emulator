
# Chip8 Interpreter / Emulator

A Chip 8 emulator written using C++, and using SDL for graphics. 

Chip 8 is a simple, interpreted programming languaged that was created in 1977. It was designed to be run on early microcomputers with limited resources, and allowed users to run games with as little as 4 KB of memory. This is a near-complete interpreter/emulator for the Chip 8, although it is currently missing sound.

## Compiling and Running

This build requires SDL2:\
Linux:
```
$ sudo apt-get install libsdl2-dev
```

Compile:
```
$ cd chip8emulator
$ make 
```
Clean Old Files:
```
make clean
```
Run:\
This build requires 3 arguments, the video scale, cycle delay, and ROM file. You can expirement to see what works best for your system. For most systems a video scale of 10-20 is best, and a cycle delay of 1 or 2 is best.
```
./chip8 <Video Scale> <Cycle Delay> <ROM file>
```

Also included in this repo, is 15 public domain ROMs for the system to test with, in the `roms` directory.

## References
Resources that I used while creating this.
- https://austinmorlan.com/posts/chip8_emulator/
- https://tobiasvl.github.io/blog/write-a-chip-8-emulator/
- https://en.wikipedia.org/wiki/CHIP-8

