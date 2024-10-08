#include "chip8.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <vector>

// Constructor implementation
Chip8::Chip8() : randGen(std::chrono::system_clock::now().time_since_epoch().count()) {
    pc = START_ADDRESS; // Set the program counter to start outside of reserved memory.
    sp = 0;
    // Load fonts into memory
    for (unsigned int i = 0; i < FONTSET_SIZE; ++i) {
        memory[FONTSET_START_ADDRESS + i] = fontset[i];
    }

    // Seed RNG
    randByte = std::uniform_int_distribution<uint8_t>(0, 255U);

    

    // Create function pointer table
		table[0x0] = &Chip8::Table0;
		table[0x1] = &Chip8::OP_1nnn;
		table[0x2] = &Chip8::OP_2nnn;
		table[0x3] = &Chip8::OP_3xkk;
		table[0x4] = &Chip8::OP_4xkk;
		table[0x5] = &Chip8::OP_5xy0;
		table[0x6] = &Chip8::OP_6xkk;
		table[0x7] = &Chip8::OP_7xkk;
		table[0x8] = &Chip8::Table8;
		table[0x9] = &Chip8::OP_9xy0;
		table[0xA] = &Chip8::OP_Annn;
		table[0xB] = &Chip8::OP_Bnnn;
		table[0xC] = &Chip8::OP_Cxkk;
		table[0xD] = &Chip8::OP_Dxyn;
		table[0xE] = &Chip8::TableE;
		table[0xF] = &Chip8::TableF;

		for (size_t i = 0; i <= 0xE; i++)
		{
			table0[i] = &Chip8::OP_NULL;
			table8[i] = &Chip8::OP_NULL;
			tableE[i] = &Chip8::OP_NULL;
		}

		table0[0x0] = &Chip8::OP_00E0;
		table0[0xE] = &Chip8::OP_00EE;

		table8[0x0] = &Chip8::OP_8xy0;
		table8[0x1] = &Chip8::OP_8xy1;
		table8[0x2] = &Chip8::OP_8xy2;
		table8[0x3] = &Chip8::OP_8xy3;
		table8[0x4] = &Chip8::OP_8xy4;
		table8[0x5] = &Chip8::OP_8xy5;
		table8[0x6] = &Chip8::OP_8xy6;
		table8[0x7] = &Chip8::OP_8xy7;
		table8[0xE] = &Chip8::OP_8xyE;

		tableE[0x1] = &Chip8::OP_ExA1;
		tableE[0xE] = &Chip8::OP_Ex9E;

		for (size_t i = 0; i <= 0x65; i++)
		{
			tableF[i] = &Chip8::OP_NULL;
		}

		tableF[0x07] = &Chip8::OP_Fx07;
		tableF[0x0A] = &Chip8::OP_Fx0A;
		tableF[0x15] = &Chip8::OP_Fx15;
		tableF[0x18] = &Chip8::OP_Fx18;
		tableF[0x1E] = &Chip8::OP_Fx1E;
		tableF[0x29] = &Chip8::OP_Fx29;
		tableF[0x33] = &Chip8::OP_Fx33;
		tableF[0x55] = &Chip8::OP_Fx55;
		tableF[0x65] = &Chip8::OP_Fx65;
}

// LoadROM method implementation
void Chip8::LoadROM(char const* filename) {
    // Open the file as a binary stream, places cursor at the end of file
    std::ifstream file(filename, std::ios::binary | std::ios::ate);

    if (file.is_open()) {
        // Get the size of the file by reading what character the cursor is at
        std::streampos size = file.tellg();
        //std::cout << "size: " << size << std::endl;
        // Allocate a buffer to hold the file content
        std::vector<char> buffer(size);

        // Go back to the beginning of the file and read the content into the buffer
        file.seekg(0, std::ios::beg);
        file.read(buffer.data(), size);
        

        // Load the ROM contents into Chip8 memory starting at 0x200
        for (size_t i = 0; i < size; ++i) {
            memory[START_ADDRESS + i] = static_cast<uint8_t>(buffer[i]);
        }

        // Clean up the buffer
        buffer.clear();
        //Close file
        file.close();
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

    pc = address;
}

//OP_2nnn implementation, call subroutine at nnn
void Chip8::OP_2nnn(){
    uint16_t address = opcode & 0x0FFFu;

    stack[sp] = pc;
    ++sp;
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
void Chip8::OP_8xy5(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vx] >= registers[Vy]){
        registers[0xF] = 1;
    }
    else{
        registers[0xF] = 0;
    }

    registers[Vx] -= registers[Vy];
}

//OP_8xy6 implementation, set Vx = Vx SHR 1
void Chip8::OP_8xy6(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    //Saves LSB into VF register
    registers[0xF] = (registers[Vx] & 0x1u);
    registers[Vx] >>= 1;
}

//OP_8xy7 implementation, Set Vx = Vy - Vx, set VF = NOT borrow
void Chip8::OP_8xy7(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;

    if(registers[Vy] >= registers[Vx]){
        registers[0xF] = 1;
    }
    else{
        registers[0xF] = 0;
    }

    registers[Vx] = registers[Vy] - registers[Vx];
}

//OP_8xyE implementation, set Vx = Vx SHL 1
void Chip8::OP_8xyE(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    //Save MSB in VF
    registers[0xF] = (registers[Vx] & 0x80u) >> 7u;

    registers[Vx] <<= 1;
}

//OP_9xy0 implementation, skip next instruction if Vx != Vy
void Chip8::OP_9xy0(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    
    if(registers[Vx] != registers[Vy]){
        pc += 2;
    }
}

//OP_Annn implementation, set I = nnn
void Chip8::OP_Annn(){
    uint16_t address = opcode & 0x0FFFu;

    index = address;
}

//OP_Bnnn implementation, jump to location nnn + V0
void Chip8::OP_Bnnn(){
    uint8_t address = opcode & 0x0FFFu;

    pc = registers[0] + address;
}

//OP_Cxkk implementation, set Vx = random byte AND kk
void Chip8::OP_Cxkk(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t byte = opcode & 0x00FFu;

    registers[Vx] = randByte(randGen) & byte;
}

//OP_Dxyn implementation, display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
void Chip8::OP_Dxyn(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t Vy = (opcode & 0x00F0u) >> 4u;
    uint8_t height = opcode & 0x000Fu;

    //Wrap around if beyond screen boundaries
    uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
    uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

    registers[0xF] = 0;
    for(unsigned int row = 0; row < height; ++row){
        uint8_t spriteByte = memory[index + row];

        for(unsigned int col = 0; col < 8; ++col){
            uint8_t spritePixel = spriteByte & (0x80u >> col);
            uint32_t* screenPixel = &video[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

            //Sprite pixel is on
            if(spritePixel){
                //Screen pixel also on - indicating collision
                if(*screenPixel == 0xFFFFFFFF){
                    registers[0xF] = 1;
                }

                //XOR with the sprite pixel
                *screenPixel ^= 0xFFFFFFFF;
            }
        }
    }
}

//OP_Ex9E implementation, skip next instruction if key with the value of Vx is pressed
void Chip8::OP_Ex9E(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t key = registers[Vx];
    
    if(keypad[key]){
        pc += 2;
    }
}

//OP_ExA1 implementation, skip next instruction if key with the value of Vx is not pressed.
void Chip8::OP_ExA1(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    uint8_t key = registers[Vx];

    if(!keypad[key]){
        pc += 2;
    }
}

//OP_Fx07 implementation, set Vx = delay timer value
void Chip8::OP_Fx07(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    registers[Vx] = delayTimer;
}

//OP_Fx0A implementation, wait for a key press and store the value of the key in Vx
void Chip8::OP_Fx0A(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    bool keyFound = false;
    for(uint8_t i = 0; i < 16; i++){
        if(keypad[i]){
            registers[Vx] = i;
            keyFound = true;
            break;
        }
    }
    if(!keyFound){
        pc -= 2;
    }
}

//OP_Fx15 implementation, set delay timer = Vx
void Chip8::OP_Fx15(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    delayTimer = registers[Vx];
}

//OP_Fx18 implementation, set sound timer = Vx
void Chip8::OP_Fx18(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    soundTimer = registers[Vx];
}

//OP_Fx1E implementation, set I = I + Vx
void Chip8::OP_Fx1E(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    index += registers[Vx];
}

//OP_Fx29 implementation, set I = location of sprite for digit Vx
void Chip8::OP_Fx29(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t digit = registers[Vx];

    index = FONTSET_START_ADDRESS + (5 * digit);
}

//OP_Fx33 implementation, Store BCD representation of Vx in memory locations I, I=1, and I+2.
void Chip8::OP_Fx33(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    uint8_t value = registers[Vx];

    //Ones place
    memory[index + 2] = value % 10;
    value /= 10;

    //Tens place
    memory[index + 1] = value % 10;
    value /= 10;

    //Hundredths place
    memory[index] = value % 10;
}

//OP_Fx55 implementation, store registers V0 through Vx in memory starting at location I
void Chip8::OP_Fx55(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;

    for(uint8_t i = 0; i <= Vx; ++i){
        memory[index + i] = registers[i];
    }
}

//OP_Fx65 implementation, read registers V0 through Vx from memory starting at location I
void Chip8::OP_Fx65(){
    uint8_t Vx = (opcode & 0x0F00u) >> 8u;
    for(uint8_t i = 0; i <= Vx; ++i){
        registers[i] = memory[index + i];
    }
}

/*
FUNCTIONS FOR FUNCTION POINTER TABLE
*/

void Chip8::Table0(){
	((*this).*(table0[opcode & 0x000Fu]))();
}
void Chip8::Table8(){
    ((*this).*(table8[opcode & 0x000Fu]))();
}

void Chip8::TableE(){
    ((*this).*(tableE[opcode & 0x000Fu]))();
}

void Chip8::TableF(){
    ((*this).*(tableF[opcode & 0x00FFu]))();
}

void Chip8::OP_NULL(){
    //For Debugging
    //std::cout << "Unimplemented Opcode: " << std::hex << opcode << std::endl;
}

void Chip8::Cycle(){
    //Fetch opcode
    opcode = (memory[pc] << 8u) | memory[pc+1];

    //Debugging PC position
    //std::cout << "Opcode: " << std::hex << opcode;
    //std::cout << " | pc: " << pc << std::endl;
    //assert(pc % 2 == 0);
    //assert(pc >= START_ADDRESS && pc < 4096);
    
    //Increment the PC before execution
    pc += 2;

    //Decode and Execute
    ((*this).*(table[(opcode & 0xF000u) >> 12u]))();

    //Decrement the timer delay if it is set
    if(delayTimer > 0){
        --delayTimer;
    }

    //Decrement sound timer if it is set
    if(soundTimer > 0){
        --soundTimer;
    }
}