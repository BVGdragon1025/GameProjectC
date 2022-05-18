#include <stdio.h>
#include <stdlib.h>
#define SDL_MAIN_HANDLED
#include "libs/SDL2/SDL.h"
#include "libs/SDL2/SDL_image.h"
#include "main.h"
#include <Windows.h>
#include <assert.h>

int horizontalRes = 1280;
int verticalRes = 720;

int gridWidth = 15;
int gridHeight = 11;

float speed = 1000.0f;

uint32_t lastTickTime = 0;
uint32_t currentTickTime = 0;
uint32_t deltaTime = 0;

// Loading an image
char image_path[] = "img/image.png";
char obstacle_image[] = "img/obstacle.png";

struct Vector2 {
	int x;
	int y;

	Vector2(int x, int y);

	Vector2 operator +(Vector2 right) {
		return Vector2{ x + right.x, y + right.y };
	}

	Vector2 operator -(Vector2 right) {
		return Vector2{ x - right.x, y - right.y };
	}

	Vector2 operator *(float factor) {
		return Vector2{ (int)(x * factor), (int)(y * factor) };
	}

	Vector2 operator /(float factor) {
		assert(factor != 0.0f && "Factor must be non-zero");

		return Vector2{ (int)(x / factor), (int)(y / factor) };
	}

	Vector2 operator *(Vector2 right) {
		return Vector2{ x * right.x, y * right.y };
	}

	int Dot(Vector2 right) {
		return x * right.x + y * right.y;
	}

	float Length() {
		return (float)sqrt(x * x + y * y);
	}

	explicit operator bool() {
		return x || y;  //Bool zwraca true dla wartoœci nie zerowej
	}
};

struct Vector2f {
	float x;
	float y;

	Vector2f(float x, float y);

	Vector2f operator +(Vector2f right) {
		return Vector2f{ x + right.x,y + right.y };
	}
	Vector2f operator -(Vector2f right) {
		return Vector2f{ x - right.x, y - right.y };
	}

	Vector2f operator *(float factor) {
		return Vector2f{ x * factor, y * factor };
	}

	Vector2f operator /(float factor) {
		assert(factor != 0.0f && "Factor must be non-zero");

		return Vector2f{ x / factor, y / factor };
	}

	Vector2f operator *(Vector2f right) {
		return Vector2f{ x * right.x, y * right.y };
	}

	int Dot(Vector2f right) {
		return x * right.x + y * right.y;
	}

	float Length() {
		return (float)sqrt(x * x + y * y);
	}

	explicit operator bool() {
		return x || y;  //Bool zwraca true dla wartoœci nie zerowej
	}
};

struct Vector4 {
	int r;
	int g;
	int b;
	int a;

	Vector4(int red, int green, int blue, int alfa);
};

struct Obstacle {

	SDL_Texture* texture;
	Vector2 size;

	Obstacle(SDL_Texture* texture, Vector2 size);
	Obstacle(SDL_Texture* texture, int xSize, int ySize);
	Obstacle(SDL_Renderer* renderer, SDL_Surface* surface);

	void RenderObstacle(SDL_Renderer* renderer, float x, float y);
	void Destroy();
};

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
	Player(Obstacle obstacle, Vector2* textureParams, Vector2* position);
	Player(Obstacle obstacle, int xSize, int ySize, Vector2 position);
	Player(Obstacle* obstacle, int xSize, int ySize, Vector2 position);
	void Render(SDL_Renderer* renderer);
	
};

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
	Vector4* backgroundColor = new Vector4(20, 150, 39, 255);

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
	SDL_SetRenderDrawColor(renderer, backgroundColor->r, backgroundColor->g, backgroundColor->b, backgroundColor->a);

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

	int width = horizontalRes / gridWidth;
	int height = verticalRes / gridHeight;

	Vector2* surfaceParams = new Vector2(width, height);

	Vector2 currentPosition{ 5, 10 };
	Vector2 endGoal = currentPosition;
	Vector2 currentBlockPosition = currentPosition;

	SDL_Surface* surface;

	surface = SDL_CreateRGBSurface(0, width, height, 32, 0, 0, 0, 0);

	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 255, 255, 255));
	Obstacle* playerCollider = new Obstacle(renderer, IMG_Load(image_path));
	Player* player = new Player(playerCollider, width, height, currentPosition);

	SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 64, 255, 0));
	Obstacle* obstacle = new Obstacle(SDL_CreateTextureFromSurface(renderer, IMG_Load(obstacle_image)), width, height);
	

	SDL_FreeSurface(surface);

	unsigned char** gridArray = (unsigned char**)malloc(sizeof(unsigned char*) * gridHeight);

	for (int i = 0; i < gridHeight; i++) {
		gridArray[i] = (unsigned char*)malloc(sizeof(unsigned char) * gridWidth);
		for (int j = 0; j < gridWidth; j++) {
			gridArray[i][j] = 1;
		}
	}

	Stack firstStack, secondStack, path;
	Stack* current = &firstStack;
	Stack* next = &secondStack;


	bool isFinding = false;
	bool isPathExists = false;
	bool done = false;
	SDL_Event sdl_event;

	// Delta Time
	deltaTime = CalculateDeltaTime(&lastTickTime, &currentTickTime);

	// The main loop
	// Every iteration is a frame
	while (!done)
	{
		// Polling the messages from the OS.
		// That could be key downs, mouse movement, ALT+F4 or many others
		while (SDL_PollEvent(&sdl_event))
		{
			Process(sdl_event, done, current, player, endGoal, width, height, gridArray, isFinding);
		}

		// Clearing the screen
		SDL_RenderClear(renderer);

		DrawObstacles(gridArray, obstacle, renderer);

		// All drawing goes here

		// Let's draw a sample image

		while (isFinding && !isPathExists) {
			FindPath(current, gridArray, next, path, isPathExists);
			current->DeleteLastElement();
			if (!current->lastElement) {
				Stack* temp = current;
				current = next;
				next = temp;
			}

			while (isPathExists) {
				unsigned char x = path.lastElement->x;
				unsigned char y = path.lastElement->y;
				if (x - 1 >= 0 && gridArray[y][x - 1] == gridArray[y][x] - 1) {
					path.AddElement(x - 1, y);
				}
				else if (x + 1 < gridWidth && gridArray[y][x + 1] == gridArray[y][x] - 1)
				{
					path.AddElement(x + 1, y);
				}
				else if (y - 1 >= 0 && gridArray[y - 1][x] == gridArray[y][x] - 1)
				{
					path.AddElement(x, y - 1);
				}
				else if (y + 1 < gridHeight && gridArray[y + 1][x] == gridArray[y][x] - 1)
				{
					path.AddElement(x, y + 1);
				}
				if (gridArray[y][x] == 3)
				{
					for (int i = 0; i < gridHeight; i++)
					{
						for (int j = 0; j < gridWidth; j++)
						{
							gridArray[i][j] = 1;
						}
					}
					current->Clear();
					next->Clear();
					isFinding = false;
					isPathExists = false;
					break;

				}
			}
		}

		if (path.lastElement) {
			player->position.x = path.lastElement->x;
			player->position.y = path.lastElement->y;
			path.DeleteLastElement();
			Sleep(speed * deltaTime);
		}

		
		player->Render(renderer);

// Showing the screen to the player
		SDL_RenderPresent(renderer);

		// next frame...
	}

	// If we reached here then the main loop stoped
	// That means the game wants to quit

	for (int i = 0; i < gridHeight; i++) {
		free(gridArray[i]);
	}

	free(gridArray);

	obstacle->Destroy();

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

void MoveTo(Player& player, Stack& path, SDL_Renderer* renderer) {
	while (player.position.x < path.lastElement->x || player.position.x > path.lastElement->x || player.position.y < path.lastElement->y || player.position.y > path.lastElement->y) {
		
		if (player.position.x < path.lastElement->x) {
			player.position.x += speed * deltaTime;
		}
		else if (player.position.x > path.lastElement->x) {
			player.position.x -= speed * deltaTime;
		}

		if (player.position.y < path.lastElement->y) {
			player.position.y += speed * deltaTime;
		}
		else if (player.position.y > path.lastElement->y) {
			player.position.y -= speed * deltaTime;
		}
	}
}

void FindPath(Stack* currentCheck, unsigned char** gridArray, Stack* nextCheck, Stack& path, bool& isPathGenerated){
	unsigned char x = currentCheck->lastElement->x;
	unsigned char y = currentCheck->lastElement->y;

	if (x - 1 >= 0 && (gridArray[y][x - 1] == 1 || gridArray[y][x - 1] == 255)) {
		if (gridArray[y][x - 1] != 255) {
			gridArray[y][x - 1] = gridArray[y][x] + 1;
			nextCheck->AddElement(x - 1, y);
		}
		else {
			gridArray[y][x - 1] = gridArray[y][x] + 1;
			path.AddElement(x - 1, y);
			isPathGenerated = true;
		}
	}

	if (y - 1 >= 0 && (gridArray[y - 1][x] == 1 || gridArray[y - 1][x] == 255))
	{
		if (gridArray[y - 1][x] != 255)
		{
			gridArray[y - 1][x] = gridArray[y][x] + 1;
			nextCheck->AddElement(x, y - 1);
		}
		else
		{
			gridArray[y - 1][x] = gridArray[y][x] + 1;
			path.AddElement(x, y - 1);
			isPathGenerated = true;
		}

	}

	if (x + 1 < gridWidth && (gridArray[y][x + 1] == 1 || gridArray[y][x + 1] == 255))
	{
		if (gridArray[y][x + 1] != 255)
		{
			gridArray[y][x + 1] = gridArray[y][x] + 1;
			nextCheck->AddElement(x + 1, y);
		}
		else
		{
			gridArray[y][x + 1] = gridArray[y][x] + 1;
			path.AddElement(x + 1, y);
			isPathGenerated = true;
		}
	}

	if (y + 1 < gridHeight && (gridArray[y + 1][x] == 1 || gridArray[y + 1][x] == 255))
	{
		if (gridArray[y + 1][x])
		{
			if (gridArray[y + 1][x] != 255)
			{
				gridArray[y + 1][x] = gridArray[y][x] + 1;
				nextCheck->AddElement(x, y + 1);
			}
			else
			{
				gridArray[y + 1][x] = gridArray[y][x] + 1;
				path.AddElement(x, y + 1);
				isPathGenerated = true;
			}
		}
	}


}

void DrawObstacles(unsigned char** gridArray, Obstacle* RedBlock, SDL_Renderer* renderer) {
	for (int i = 1; i < gridWidth - 1; i++) {
		gridArray[gridHeight / 2][i] = 0;
		RedBlock->RenderObstacle(renderer, i, gridHeight / 2);
	}
}

void Process(SDL_Event& sdl_event, bool& isDone, Stack* check, Player* player, Vector2 endPosition, int width, int height, unsigned char** gridArray, bool& isFinding) {
	if (sdl_event.type == SDL_QUIT) // The user wants to quit
	{
		isDone = true;
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
	else if (sdl_event.type == SDL_MOUSEBUTTONDOWN) {
		switch (sdl_event.button.button) {
			case SDL_BUTTON_LEFT:
			{
				int x, y;
				printf("Left mouse button pressed\n");
				SDL_GetMouseState(&x, &y);
				speed = 1.0f;
				check->AddElement((int)player->position.x, (int)player->position.y);
				endPosition.x = (int)(x / width);
				endPosition.y = (int)(y / height);
				gridArray[(int)player->position.y][(int)player->position.x] = 2;
				gridArray[(int)endPosition.y][(int)endPosition.x] = 255;
				isFinding = true;
				break;
			}
			default:
				break;
		}

	}
		
	// More events here?
}
