#include "Chip8.h"
#include <iostream>
#include <fstream>

// Constructor implementation
Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
    pc = START_ADDRESS; // Set the program counter to start outside of reserved memory.
    
    // Load fonts into memory
    for (unsigned int i = 0; i < FONTSET_SIZE; i++) {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    // Seed RNG
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);
}

// LoadROM method implementation
void Chip8::LoadROM(char const* filename) {
    // Open the file as a binary stream
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        // Get the size of the file
        std::streampos size = file.tellg();
        // Allocate a buffer to hold the file content
        char* buffer = new char[size];

        // Go back to the beginning of the file and read the content into the buffer
        file.seekg(0, std::ios::beg);
        file.read(buffer, size);
        file.close();

        // Load the ROM contents into Chip8 memory starting at 0x200
        for (long i = 0; i < size; ++i) {
            memory[START_ADDRESS + i] = buffer[i];
        }

        // Clean up the buffer
        delete[] buffer;
    } else {
        std::cerr << "Failed to open ROM: " << filename << std::endl;
    }
}

//OP_00E0 implementation, clears the display
void Chip8::OP_00E0(){
    memset(video, 0, sizeof(video));
}

//OP_00EE implementation, returns from a subroutine
void Chip8::OP_00EE(){
    --sp;
    pc = stack[sp];
}

//OP_1nnn implementation, sets program counter to nnn
void Chip8::OP_1nnn(){
    uint16_t address = opcode & 0x0FFFu;
}

//OP_2nnn implementation, call subroutine at nnn
void Chip8::OP_2nnn(){
    uint16_t address = opcode & 0x0FFFu;

    stack[sp] = pc;
    sp++;
    pc = address;
}

//OP_3xKK implementation, Skip next instruction if Vx = kk
void Chip8::OP_3xkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    if (registers[Vx] == byte){
        pc += 2;
    }
}

//OP_4xkk implementation, Skip next instruction if Vx != kk
void Chip8::OP_4xkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    if(registers[Vx] != byte){
        pc += 2;
    }
}

//OP_5xy0 implementation, skip next instruction if Vx = Vy
void Chip8::OP_5xy0(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vx] == registers[Vy]){
        pc += 2;
    }
}

//OP_6xkk implementation, set Vx = kk
void Chip8::OP_6xkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = byte;
}

//OP_7xkk implementation, Set Vx = Vx + kk
void Chip8::OP_7xkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] += byte;
}

//OP_8xy0 implementation, set Vx=  Vy
void Chip8::OP_8xy0(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] = registers[Vy];
}

//OP_8xy1 implementation, Set vx = Vx OR Vy
void Chip8::OP_8xy1(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] |= registers[Vy];
}

//OP_8xy2 implementation, set Vx = Vx AND Vy
void Chip8::OP_8xy2(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] &= registers[Vy];
}

//OP_8xy3 implementation, set Vx = Vx XOR Vy
void Chip8::OP_8xy3(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    registers[Vx] ^= registers[Vy];
}

//OP_8xy4 implementation, set Vx = Vx + Vy, set VF = carry
void Chip8::OP_8xy4(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    uint16_t sum = registers[Vx] + registers[Vy];

    if(sum > 255u){
        registers[0xF] = 1;
    }
    else{
        registers[0xF] = 0;
    }

    registers[Vx] = sum & 0xFFu;
}

//OP_8xy5 implementation, set Vx = Vx - Vy, set VF = NOT borrow
