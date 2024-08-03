#include <fstream>
#include "chip8.cpp"

void Chip8::LoadROM(char const* filename){
    //Open file as binary and move file pointer to the end
    std::ifstream rom(filename, std::ios::binary | std::ios::ate);

    if(rom.is_open()){
        //Get size of the file and allocate buffer to hold it
        std::streampos size = rom.tellg();
        char* buffer = new char[size];

        //Return to beginning of the file and fill buffer
        rom.seekg(0, std::ios::beg);
        rom.read(buffer, size);
        rom.close();

        //Load the ROM into Chip8's memory, starting at 0x200
        for(long i = 0; i < size; i++){
            memory[START_ADRESS +i] = buffer[i];
        }

        //Free the buffer
        delete[] buffer;
    }
}