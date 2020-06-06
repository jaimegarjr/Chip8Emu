#include <cstdint>

// Chip8 class
class Chip8 {
	public:
		// some functions here later

	private:
		// Chip-8 Emulator specfications as listed here: https://austinmorlan.com/posts/chip8_emulator/
		uint8_t memory[4096]{};
		uint8_t registers[16]{};
		uint8_t stack_pointer{};
		uint8_t keys[16];
		uint8_t delayTimer{};
		uint8_t soundTimer{};
		uint16_t index{};
		uint16_t program_counter{};
		uint16_t stack[16]{};
		uint16_t opcode;
		uint32_t display[64 * 32]{};

};