// *********************************************************
//
//			 CHIP 8 CLASS FUNCTION DECLARATIONS
//
// *********************************************************

// header inclusion
#include "chip8.h"

using namespace std;

// MEMORY RANGES:
// Total: 0x000 to 0xFFF
// Beginning: 0x000-0x1FF - reserved for the CHIP-8 interpreter
// Middle: 0x050-0x0A0 - storage space for the 16 built-in characters
// Ending: 0x200-0xFFF - instructions from the ROM

// Starting memory location for any Chip8 object
const unsigned int START_ADDRESS = 0x200;

// Chip8 constructor declaration
Chip8::Chip8() {
	program_counter = START_ADDRESS;
}

// Rom loading function declaration
void Chip8::LoadROM(char const* filename) {

	// creation of input file with name filename, and reading in binary at the end of the file
	ifstream rom_file(filename, ios::binary | ios::ate);

	if (rom_file.is_open()) {
		// allocates size and gets current character in stream
		streampos size = rom_file.tellg();

		// creates a pointer to a dynamic array of characters with size 
		char* buffer = new char[size];

		// goes back to beginning of file
		rom_file.seekg(0, ios::beg);
		
		// fills buffer array with the size of the rom file and closes 
		rom_file.read(buffer, size);
		rom_file.close();

		// loads the ROM into the respective memory slot in Chip8 interpreter
		for (long i = 0; i < size; i++) {

			// iterates as long as size and goes storing into memory 
			memory[START_ADDRESS + i] = buffer[i];

		}

		// delets dynamically allocated buffer array
		delete[] buffer;
	}
}
