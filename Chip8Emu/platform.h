#pragma once
#include <cstdint>

class SDL_Window;
class SDL_Renderer;
class SDL_Texture;

class Platform
{
	public:
		// constructor
		Platform(char const* title, int windowWidth, int windowHeight, int textureWidth, int textureHeigh);
		
		// update function
		void Update(void const* buffer, int pitch);
		
		// input for keys function
		bool ProcessInput(uint8_t* keys);
		
		// destructor
		~Platform();

	private:
		SDL_Window* window{};
		SDL_Renderer* renderer{};
		SDL_Texture* texture{};
};