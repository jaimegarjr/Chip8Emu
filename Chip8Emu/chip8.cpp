// *********************************************************
//
//			 CHIP 8 CLASS FUNCTION DECLARATIONS
//
// *********************************************************

// header inclusion
#include "chip8.h"
#include <random>
#include <chrono>

using namespace std;

// MEMORY RANGES:
// Total: 0x000 to 0xFFF
// Beginning: 0x000-0x1FF - reserved for the CHIP-8 interpreter
// Middle: 0x050-0x0A0 - storage space for the 16 built-in characters
// Ending: 0x200-0xFFF - instructions from the ROM

const unsigned int START_ADDRESS = 0x200;	// starting memory location for any Chip8 object
const unsigned int FONT_SIZE = 80;			// 5 bytes per character, 16 characters
const unsigned int FONT_START_ADDRESS = 0x50;

// array of fontset (characters created in terms of bytes in hexadecimal)
// Example of the letter 'F'
// 11110000 = (0x)F0
// 10000000 = (0x)80
// 11110000 = (0x)F0
// 10000000 = (0x)80
// 10000000 = (0x)80

uint8_t fontset[FONT_SIZE] = {
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

// Chip8 constructor declaration
Chip8::Chip8()
	: randGen(std::chrono::system_clock::now().time_since_epoch().count())
{
	// Sets the program counter to the starting address in memory
	program_counter = START_ADDRESS;

	// loads in fontset into memory
	for (unsigned int i = 0; i < FONT_SIZE; i++) {
		memory[FONT_START_ADDRESS + i] = fontset[i];
	}

	// initializes the random number generator
	randByte = std::uniform_int_distribution<uint16_t>(0, 255U);

	// DECLARATION OF FUNCTION POINTER TABLE
	// First unique set of opcodes
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

	// Second set of opcodes
	table0[0x0] = &Chip8::OP_00E0;
	table0[0xE] = &Chip8::OP_00EE;

	// Third set of opcodes
	table8[0x0] = &Chip8::OP_8xy0;
	table8[0x1] = &Chip8::OP_8xy1;
	table8[0x2] = &Chip8::OP_8xy2;
	table8[0x3] = &Chip8::OP_8xy3;
	table8[0x4] = &Chip8::OP_8xy4;
	table8[0x5] = &Chip8::OP_8xy5;
	table8[0x6] = &Chip8::OP_8xy6;
	table8[0x7] = &Chip8::OP_8xy7;
	table8[0xE] = &Chip8::OP_8xyE;

	// Fourth set of opcodes
	tableE[0x1] = &Chip8::OP_ExA1;
	tableE[0xE] = &Chip8::OP_Ex9E;

	// Fifth set of opcodes
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

void Chip8::Table0() {
	((*this).*(table0[opcode & 0x000Fu]))();
}

void Chip8::Table8() {
	((*this).*(table8[opcode & 0x000Fu]))();
}

void Chip8::TableE() {
	((*this).*(tableE[opcode & 0x000Fu]))();
}

void Chip8::TableF() {
	((*this).*(tableF[opcode & 0x000Fu]))();
}

// Save function in case no opcode is found
void Chip8::OP_NULL() {}

// Function to clear the screen when ROM calls it
void Chip8::OP_00E0() {
	// clears the screen with memset
	memset(display, 0, sizeof(display));
}

// Function to return from a subroutine
void Chip8::OP_00EE()
{
	// decremements the stack pointer
	--stack_pointer;

	// assigns the program_counter from the stack
	program_counter = stack[stack_pointer];
}

// Function that jumps to a particular location
void Chip8::OP_1nnn() {

	// creates address with location
	uint16_t address = opcode & 0x0FFFu;

	// makes the program counter point to address
	program_counter = address;
}

// Function that calls a subroutine
void Chip8::OP_2nnn()
{
	// creates address with location
	uint16_t address = opcode & 0x0FFFu;

	// access the index of stack usiing the pointer then assign to program counter
	stack[stack_pointer] = program_counter;

	// increment stack pointer
	++stack_pointer;

	// make the program counter point to address
	program_counter = address;
}

// Function to skip instruction if Vx == kk
void Chip8::OP_3xkk()
{

	// declare 2 variables for different registers
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	// if the previous 2 variables have the same component add 2
	if (registers[Vx] == byte)
	{
		program_counter += 2;
	}
}

// Function to skip next instruction if Vx != kk
void Chip8::OP_4xkk()
{
	// declare 2 variables for different registers
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	// if the previous 2 variables dont have the same component add 2
	if (registers[Vx] != byte)
	{
		program_counter += 2;
	}
}

// Function to skip next instruction if Vx != Vy
void Chip8::OP_5xy0() {

	// declare variables Vx and Vy
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	// check if they are equal, if so then add two to program_counter
	if (registers[Vx] == registers[Vy])
	{
		program_counter += 2;
	}
}

// Function to set Vx equal to kk, or byte
void Chip8::OP_6xkk() {
	
	// declares 8 bit variables
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	// sets Vx register to byte
	registers[Vx] = byte;

}

// Function to set Vx = Vx + kk
void Chip8::OP_7xkk() {

	// declares 8 bit variables
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	// increments Vx register by byte
	registers[Vx] += byte;
}

// Function to set Vx = Vy
void Chip8::OP_8xy0() {

	// creates Vx and Vy variables
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	// sets Vx to Vy
	registers[Vx] = registers[Vy];
}

// Function to logical OR Vx and Vy, and then store into Vx
void Chip8::OP_8xy1() {

	// creates Vx and Vy variables
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	// logical OR and assigns it to Vx
	registers[Vx] |= registers[Vy];
}

// Function to logical AND Vx and Vy, and then store into Vx
void Chip8::OP_8xy2() {

	// creates Vx and Vy variables
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	// logical AND and assigns it to Vx
	registers[Vx] &= registers[Vy];
}

// Function to logical XOR Vx and Vy, and then store into Vx
void Chip8::OP_8xy3() {

	// creates Vx and Vy variables
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	// logical OR and assigns it to Vx
	registers[Vx] ^= registers[Vy];
}

// Function to set Vx = Vx + Vy, set VF = carry.
void Chip8::OP_8xy4() {
	// creates Vx and Vy variables
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	// sums Vx and Vy
	uint16_t sum = registers[Vx] + registers[Vy];

	// if sum is greater than 8 bits, set flag
	if (sum > 255U) {
		registers[0xF] = 1;
	}

	// else, don't set flag
	else {
		registers[0xF] = 0;
	}

	// keeps lower 8 bits of sum and stores it in Vx
	registers[Vx] = sum & 0xFFu;
}

// Function to set Vx = Vx - Vy, set VF = NOT borrow
void Chip8::OP_8xy5() {

	// creates Vx and Vy variables
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	// if Vx is bigger, set VF
	if (registers[Vx] > registers[Vy])
	{
		registers[0xF] = 1;
	}

	// else, don't set VF
	else
	{
		registers[0xF] = 0;
	}

	// subtract the two registers and store in Vx
	registers[Vx] -= registers[Vy];
}

// Function to set Vx = Vx SHR 1
void Chip8::OP_8xy6() {

	// creates Vx variable
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// Save LSB in VF
	registers[0xF] = (registers[Vx] & 0x1u);

	// shifts bits one to right and reassigns
	registers[Vx] >>= 1;
}

//Function  to Set Vx = Vy - Vx, set VF = NOT borrow.
void Chip8::OP_8xy7()
{ 
	//Creates Vx and Vy variable
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	//If Vy is bigger set VF 
	if (registers[Vy] > registers[Vx])
	{
		registers[0xF] = 1;
	}
	// else don't set VF
	else
	{
		registers[0xF] = 0;
	}
	//Subtract Vy register with Vx and assign to Vx register
	registers[Vx] = registers[Vy] - registers[Vx];
}

// Set Vx = Vx SHR 1.
void Chip8::OP_8xyE()
{
	// create Vx variable
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// save MSB in VF
	registers[0xF] = (registers[Vx] & 0x80u) >> 7u;
	
	// Vx is multiplied by 2
	registers[Vx] <<= 1;
}

// Skip next instruction if Vx != Vy.
void Chip8::OP_9xy0()
{
	// declare variables Vx and Vy
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;

	// if Vx does not equal Vy increment pc by 2
	if (registers[Vx] != registers[Vy])
	{
		program_counter += 2;
	}
}

// Set I = nnn
void Chip8::OP_Annn()
{
	// declare variable address
	uint16_t address = opcode & 0x0FFFu;
	
	// index assigned to the address declared
	index = address;
}

// Jump to location nnn + V0.
void Chip8::OP_Bnnn()
{
	// declare variable address
	uint16_t address = opcode & 0x0FFFu;

	// program_counter equals the register at index 0 plus address declared
	program_counter = registers[0] + address;
}

// Set Vx = random byte AND kk.
void Chip8::OP_Cxkk()
{
	//declare variables Vx and Vy
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t byte = opcode & 0x00FFu;

	// generate a random byte and assign it to the register
	registers[Vx] = randByte(randGen) & byte;
}

// Function to display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision
void Chip8::OP_Dxyn() {
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t Vy = (opcode & 0x00F0u) >> 4u;
	uint8_t height = opcode & 0x000Fu;

	uint8_t xPos = registers[Vx] % VIDEO_WIDTH;
	uint8_t yPos = registers[Vy] % VIDEO_HEIGHT;

	// VF register, register 15
	registers[0xF] = 0;

	for (unsigned int row = 0; row < height; ++row) {
		uint8_t spriteByte = memory[index + row];

		for (unsigned int col = 0; col < 8; ++col) {
			uint8_t spritePixel = spriteByte & (0x80u >> col);
			uint32_t* screenPixel = &display[(yPos + row) * VIDEO_WIDTH + (xPos + col)];

			// if sprite exists
			if (spritePixel) {

				// if this is true, it means collision is occuring 
				if (*screenPixel == 0xFFFFFFFF) {
					// sets VF to 1
					registers[0xF] = 1;
				}

				// XOR sprite
				*screenPixel ^= 0xFFFFFFFF;
			}
		}
	}
}

// Function to skip next instruction if key with the value of Vx is pressed.
void Chip8::OP_Ex9E() {

	// declare Vx
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// declare key and set it to register Vx
	uint8_t key = registers[Vx];

	// if keys at key is True
	if (keys[key])
	{
		program_counter += 2; // skips instruction
	}
}

// Function to skip next instruction if key with the value of Vx is not pressed
void Chip8::OP_ExA1() {
	// declare Vx
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// declare key and set it to register Vx
	uint8_t key = registers[Vx];

	// if keys at key is True
	if (!keys[key])
	{
		program_counter += 2; // skips instruction
	}
}

// Function to set register Vx to delayTimer
void Chip8::OP_Fx07() {
	// declare Vx
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// assigns the delay time to register Vx
	registers[Vx] = delayTimer;
}

// Function to wait for a key press, and store the value of the key in Vx
void Chip8::OP_Fx0A() {

	// declare Vx
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// if statements for respective key (maybe implement a loop?)
	if (keys[0]) {
		registers[Vx] = 0;
	}
	else if (keys[1]) {
		registers[Vx] = 1;
	}
	else if (keys[2]) {
		registers[Vx] = 2;
	}
	else if (keys[3]) {
		registers[Vx] = 3;
	}
	else if (keys[4]) {
		registers[Vx] = 4;
	}
	else if (keys[5]) {
		registers[Vx] = 5;
	}
	else if (keys[6]) {
		registers[Vx] = 6;
	}
	else if (keys[7]) {
		registers[Vx] = 7;
	}
	else if (keys[8]) {
		registers[Vx] = 8;
	}
	else if (keys[9]) {
		registers[Vx] = 9;
	}
	else if (keys[10]) {
		registers[Vx] = 10;
	}
	else if (keys[11]) {
		registers[Vx] = 11;
	}
	else if (keys[12]) {
		registers[Vx] = 12;
	}
	else if (keys[13]) {
		registers[Vx] = 13;
	}
	else if (keys[14]) {
		registers[Vx] = 14;
	}
	else if (keys[15]) {
		registers[Vx] = 15;
	}
	else {
		program_counter -= 2; // else, "wait" until key is pressed
	}
}

// Function to set delay timer = Vx
void Chip8::OP_Fx15() {

	// declare Vx
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// sets delayTimer to registers[Vx]
	delayTimer = registers[Vx];
}

// Function to set sound timer = Vx
void Chip8::OP_Fx18() {

	// declare Vx
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// sets soundTimer to registers[Vx]
	soundTimer = registers[Vx];
}

// Function to set I = I + Vx.
void Chip8::OP_Fx1E() {

	// declare Vx
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// adds registers[Vx] to index
	index += registers[Vx];
}

// Function to set I = location of sprite for digit Vx
void Chip8::OP_Fx29() {

	// declare Vx and digit
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t digit = registers[Vx];

	// gets location of sprite by multiplying digit by 5
	index = FONT_START_ADDRESS + (5 * digit);
}

// Function to store BCD representation of Vx in memory locations I, I+1, and I+2
void Chip8::OP_Fx33()
{
	// declare Vx and valuea
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;
	uint8_t value = registers[Vx];

	// Ones-place
	memory[index + 2] = value % 10;
	value /= 10;

	// Tens-place
	memory[index + 1] = value % 10;
	value /= 10;

	// Hundreds-place
	memory[index] = value % 10;
}


// Function to store registers V0 through Vx in memory starting at location I
void Chip8::OP_Fx55()
{
	// declare variable Vx
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// makes memory equal the register index +1
	for (uint8_t i = 0; i <= Vx; ++i)
	{
		memory[index + i] = registers[i];
	}
}

// Function to read registers V0 through Vx from memory starting at location I
void Chip8::OP_Fx65()
{
	// declare variable Vx
	uint8_t Vx = (opcode & 0x0F00u) >> 8u;

	// makes register equal memory index +1
	for (uint8_t i = 0; i <= Vx; ++i)
	{
		registers[i] = memory[index + i];
	}
}

//Fetch, Decode, Execute
void Chip8::Cycle()
{
	// Fetch
	opcode = (memory[program_counter] << 8u) | memory[program_counter + 1];

	// Increment the PC before we execute anything
	program_counter += 2;

	// Decode and Execute
	((*this).*(table[(opcode & 0xF000u) >> 12u]))();

	// Decrement the delay timer if it's been set
	if (delayTimer > 0)
	{
		--delayTimer;
	}

	// Decrement the sound timer if it's been set
	if (soundTimer > 0)
	{
		--soundTimer;
	}
}