#include <stdio.h>
#include <stdlib.h>
#define SDL_MAIN_HANDLED
#include "libs/SDL2/SDL.h"
#include "libs/SDL2/SDL_image.h"
#include "main.h";

int horizontalRes = 1920;
int verticalRes = 1080;

int gridWidth = 15;
int gridHeight = 11;

float xPos = horizontalRes / 2;
float yPos = verticalRes / 2;

int currentXPos = xPos;
int currentYPos = yPos;

float speed = 1.0f;

uint32_t lastTickTime = 0;
uint32_t currentTickTime = 0;
uint32_t deltaTime = 0;

int x = xPos;
int y = yPos;

// Loading an image
char image_path[] = "img/image.png";
char obstacle_image[] = "img/obstacle.png";

struct Vector2 {
	int x;
	int y;
};

struct Vector4 {
	int r;
	int g;
	int b;
	int a;
	void Init(int redValue, int greenValue, int blueValue, int alphaValue);
};

void Vector4::Init(int redValue, int greenValue, int blueValue, int alphaValue) {
	r = redValue;
	g = greenValue;
	b = blueValue;
	a = alphaValue;

}

struct Obstacle {
	SDL_Texture* texture;
	Vector2 size;
	void Init(SDL_Texture* textureData, int textureWidth, int textureHeight);
	void RenderObstacle(SDL_Renderer* renderer, float x, float y);
	void Destroy();
};

void Obstacle::Init(SDL_Texture* textureData, int textureWidth, int textureHeight) {
	texture = textureData;
	size.x = textureWidth;
	size.y = textureHeight;
}

void Obstacle::RenderObstacle(SDL_Renderer* renderer, float x, float y) {
	SDL_Rect rect;
	rect.x = round(x * horizontalRes / gridWidth);
	rect.y = round(y * verticalRes / gridHeight);
	rect.w = size.x;
	rect.h = size.y;

	SDL_RenderCopyEx(renderer, texture, nullptr, &rect, 0, nullptr, SDL_FLIP_NONE);
}

void Obstacle::Destroy() {
	SDL_DestroyTexture(texture);
}

struct Player {
	Obstacle obstacle;
	Vector2 position;
	void Init(SDL_Texture* texture, int width, int height, float xPosition, float yPosition);
	void Render(SDL_Renderer* renderer);
	
};

void Player::Init(SDL_Texture* texture, int width, int height, float xPosition, float yPosition) {
	obstacle.Init(texture, width, height);
	position.x = xPosition;
	position.y = yPosition;
}

void Player::Render(SDL_Renderer* renderer) {
	obstacle.RenderObstacle(renderer, position.x, position.y);
}

struct List {
	List* previousElement;
	unsigned char x;
	unsigned char y;
};

struct Stack {
	List* lastElement = nullptr;
	void AddElement(int xPos, int yPos);
	void DeleteLastElement();
	void Clear();
};

void Stack::AddElement(int xPos, int yPos) {
	List* newElement = (List*)malloc(sizeof(List));
	newElement->previousElement = lastElement;
	newElement->x = xPos;
	newElement->y = yPos;
	lastElement = newElement;
}

void Stack::DeleteLastElement() {
	if (lastElement) {
		List* prevElement = lastElement->previousElement;
		free(lastElement);
		lastElement = prevElement;
	}
	else {
		printf("You tried to delete something non-existent... You failed. Aborting...");
		abort();
	}
}

void Stack::Clear() {
	while (lastElement) {
		DeleteLastElement();
	}
}

uint32_t CalculateDeltaTime(uint32_t* lastTickTime, uint32_t* currentTickTime) {
	*currentTickTime = SDL_GetTicks();
	uint32_t deltaTime = *currentTickTime - *lastTickTime;
	*lastTickTime = *currentTickTime;
	return deltaTime;
}


SDL_Renderer* InitializeSDL(SDL_Window* window) {
	Vector4 backgroundColor;
	backgroundColor.Init(20, 150, 39, 255);

	// Init SDL libraries
	SDL_SetMainReady(); // Just leave it be
	int result = 0;
	result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // Init of the main SDL library
	if (result) // SDL_Init returns 0 (false) when everything is OK
	{
		printf("Can't initialize SDL. Error: %s", SDL_GetError()); // SDL_GetError() returns a string (as const char*) which explains what went wrong with the last operation
		abort();
	}

	result = IMG_Init(IMG_INIT_PNG); // Init of the Image SDL library. We only need to support PNG for this project
	if (!(result & IMG_INIT_PNG)) // Checking if the PNG decoder has started successfully
	{
		printf("Can't initialize SDL image. Error: %s", SDL_GetError());
		abort();
	}

	// Creating a renderer which will draw things on the screen
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
		abort();

	// Setting the color of an empty window (RGBA). You are free to adjust it.
	SDL_SetRenderDrawColor(renderer, backgroundColor.r, backgroundColor.g, backgroundColor.b, backgroundColor.a);

	// Here the surface is the information about the image. It contains the color data, width, height and other info.
	SDL_Surface* surface = IMG_Load(image_path);
	if (!surface)
	{
		printf("Unable to load an image %s. Error: %s", image_path, IMG_GetError());
		abort();
	}

	// Now we use the renderer and the surface to create a texture which we later can draw on the screen.
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!texture)
	{
		printf("Unable to create a texture. Error: %s", SDL_GetError());
		abort();
	}

	// In a moment we will get rid of the surface as we no longer need that. But let's keep the image dimensions.
	int tex_width = surface->w;
	int tex_height = surface->h;

	// Bye-bye the surface
	SDL_FreeSurface(surface);
	return renderer;
}

SDL_Window* GetWindow() {

	// Creating the window 1920x1080 (could be any other size)
	SDL_Window* window = SDL_CreateWindow("FirstSDL",
		0, 0,
		horizontalRes, verticalRes,
		SDL_WINDOW_SHOWN);


	if (!window)
		abort();

	return window;
}

int main()
{
	SDL_Window* window = GetWindow();
	SDL_Renderer* renderer = InitializeSDL(window);

	bool done = false;
	SDL_Event sdl_event;

	// The main loop
	// Every iteration is a frame
	while (!done)
	{
		// Polling the messages from the OS.
		// That could be key downs, mouse movement, ALT+F4 or many others
		while (SDL_PollEvent(&sdl_event))
		{
			if (sdl_event.type == SDL_QUIT) // The user wants to quit
			{
				done = true;
			}
			else if (sdl_event.type == SDL_KEYDOWN) // A key was pressed
			{
				switch (sdl_event.key.keysym.sym) // Which key?
				{
				case SDLK_ESCAPE: // Posting a quit message to the OS queue so it gets processed on the next step and closes the game
					SDL_Event event;
					event.type = SDL_QUIT;
					event.quit.type = SDL_QUIT;
					event.quit.timestamp = SDL_GetTicks();
					SDL_PushEvent(&event);
					break;
					// Other keys here
				default:
					break;
				}
			}
			else if (sdl_event.type == SDL_MOUSEBUTTONDOWN)
				switch (sdl_event.button.button)
				{
					case SDL_BUTTON_LEFT:
						printf("Left mouse button pressed\n");
						SDL_GetMouseState(&currentXPos, &currentYPos);
						speed = 1.0f;
						break;
				}
					
			// More events here?
		}

		// Clearing the screen
		SDL_RenderClear(renderer);

		// All drawing goes here

		// Let's draw a sample image

		// Delta Time
		deltaTime = CalculateDeltaTime(&lastTickTime, &currentTickTime);

		// The coordinates (could be anything)
		double imgX = tex_width / 2;  //Moved the equation here, to not have it hardcoded as much :)
		double imgY = tex_height / 2;  //As well, as this equation...

		if (currentXPos < x && (x - imgX) > 0) {
			x -= speed * deltaTime;
		}
		else if (currentXPos > x && (x + imgX) < horizontalRes) {
			x += speed * deltaTime;
		}
		if (currentYPos < y && (y - imgY) > 0) {
			y -= speed * deltaTime;
		}
		else if (currentYPos > y && (y + imgY) < verticalRes) {
			y += speed * deltaTime;
		}


		// Here is the rectangle where the image will be on the screen
		SDL_Rect rect;

		rect.x = (int)round(x - imgX); // Counting from the image's center but that's up to you
		rect.y = (int)round(y - imgY); // Counting from the image's center but that's up to you
		rect.w = (int)imgX * 2;
		rect.h = (int)imgY * 2;

		SDL_RenderCopyEx(renderer, // Already know what is that
			texture, // The image
			nullptr, // A rectangle to crop from the original image. Since we need the whole image that can be left empty (nullptr)
			&rect, // The destination rectangle on the screen.
			0, // An angle in degrees for rotation
			nullptr, // The center of the rotation (when nullptr, the rect center is taken)
			SDL_FLIP_NONE); // We don't want to flip the image

// Showing the screen to the player
		SDL_RenderPresent(renderer);

		// next frame...
	}

	// If we reached here then the main loop stoped
	// That means the game wants to quit

	// Shutting down the renderer
	SDL_DestroyRenderer(renderer);

	// Shutting down the window
	SDL_DestroyWindow(window);

	// Quitting the Image SDL library
	IMG_Quit();
	// Quitting the main SDL library
	SDL_Quit();

	// Done.
	return 0;
}