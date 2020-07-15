// *********************************************************
//
//				  CHIP 8 CLASS DECLARATION
//
// *********************************************************

// Libraries
#include <iostream>
#include <fstream>
#include <cstdint>
#include <random>
#include <chrono>

using namespace std;

// Chip8 class
class Chip8 {
	public:

		// Chip8 constructor
		Chip8();

		// Chip8 function to load in a given ROM from a filename
		void LoadROM(char const* filename);

	private:

		// random number generator components
		std::default_random_engine randGen;
		std::uniform_int_distribution<uint16_t> randByte; // FIX ME: changed from uint8_t to uint16_t due to error, why?

		// INSTRUCTION SET FUNCTIONS
		// Clears the screen
		void OP_00E0();

		// Returns from a subroutine
		void OP_00EE();

		// Jumps to a particular location
		void OP_1nnn();

		// Calls a subroutine
		void OP_2nnn();

		// Skip instruction if Vx == kk
		void OP_3xkk();

		// Skip next instruction if Vx != kk
		void OP_4xkk();

		// Skip next instruction if Vx != Vy
		void OP_5xy0();

		// Set Vx = kk
		void OP_6xkk();

		// Set Vx = Vx + kk
		void OP_7xkk();

		// Sets the value of Vy to Vx
		void OP_8xy0();

		// Performs logical OR and stores in Vx
		void OP_8xy1();

		// Performs logical AND and stores it in Vx
		void OP_8xy2();

		// Performs logical XOR and stores it in Vx
		void OP_8xy3();

		// Sets Vx = Vx + Vy, set VF = carry
		void OP_8xy4();

		// Sets Vx = Vx - Vy, set VF = NOT borrow
		void OP_8xy5();

		// Sets Vx = Vx SHR 1 (bits shifted right)
		void OP_8xy6();

		// Chip-8 emulator specfications as listed here: https://austinmorlan.com/posts/chip8_emulator/
		uint8_t memory[4096]{};			// creates memory array composed of 8-bit elements
		uint8_t registers[16]{};		// creates 16 8-bit registers for the emulator
		uint8_t stack_pointer{};		// 8-bit variable for stack pointer
		uint8_t keys[16];				// 8-bit array for key inputs
		uint8_t delayTimer{};			// 8-bit delay timer
		uint8_t soundTimer{};			// 8-bit sound timer
		uint16_t index{};				// 16-bit index variable
		uint16_t program_counter{};		// 16-bit program_counter variable
		uint16_t stack[16]{};			// creates 16-bit memory stack array
		uint16_t opcode;				// 16-bit opcode instruction
		uint32_t display[64 * 32]{};	// 32-bit display for output

};