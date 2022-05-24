#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SDL_MAIN_HANDLED
#include "libs/SDL2/SDL.h"
#include "libs/SDL2/SDL_image.h"
#include "main.h"
#include <Windows.h>

typedef unsigned char uchar;

const int horizontalRes = 1920;
const int verticalRes = 1080;

const int gridWidth = 15;
const int gridHeight = 11;

const int gridElementWidth = horizontalRes / gridWidth;
const int gridElementHeight = verticalRes / gridHeight;

uchar battlefield[17][13];

SDL_Texture* SetTexture(SDL_Surface* surface, SDL_Renderer* renderer, const char* image_path);

//Vector2 Struct
struct Vector2 {
	int x;
	int y;
};

//Character Struct
struct Character {
	SDL_Texture* texture;
	Vector2 gridPosition;
	Vector2 gridDestination;
	Vector2 currentGridPosition = { 0,0 };
	Vector2 pastGridPosition = { 0,0 };
	Character(Vector2 position, SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath);
	void PlaceCharacterOnGrid(Vector2 position);
	void MoveCharacter(Vector2 destination);
};

Character::Character(Vector2 position, SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath) {
	PlaceCharacterOnGrid(position);
	battlefield[MouseToGridPosition(position).x + 1][MouseToGridPosition(position).y + 1] = 255;
	texture = SetTexture(surface, renderer, imagePath);
}

void Character::PlaceCharacterOnGrid(Vector2 position) {
	position.x = ((position.x - 1) * gridElementWidth) + (gridElementWidth / 2);
	position.y = ((position.y - 1) * gridElementHeight) + (gridElementHeight/ 2);
}

void Character::MoveCharacter(Vector2 destination) {
	gridDestination = destination;
	currentGridPosition = { gridPosition.x / gridElementWidth,gridPosition.y / gridElementHeight };
	gridDestination.x += 1;
	gridDestination.y += 1;
	currentGridPosition.x += 1;
	currentGridPosition.y += 1;

	if (battlefield[gridDestination.x][gridDestination.y] != 255 && battlefield[gridDestination.x][gridDestination.y] != 200) {
		battlefield[gridDestination.x][gridDestination.y] = 1;
	}

	uchar playerDestination = battlefield[gridDestination.x][gridDestination.y];
	uchar playerPos = battlefield[currentGridPosition.x][currentGridPosition.y];
	uchar up = battlefield[currentGridPosition.x][currentGridPosition.y - 1];
	uchar down = battlefield[currentGridPosition.x][currentGridPosition.y + 1];
	uchar left = battlefield[currentGridPosition.x - 1][currentGridPosition.y];
	uchar right = battlefield[currentGridPosition.x + 1][currentGridPosition.y];

	GrassfireAlgorithm();

	if (playerDestination != 255 && playerDestination != 200) {
		if (playerPos > up) {
			if (gridPosition.y - gridElementHeight >= gridElementHeight / 2) {
				gridPosition.y -= gridElementHeight;
				pastGridPosition.y = currentGridPosition.y;
				currentGridPosition.y -= 1;
				battlefield[currentGridPosition.x][pastGridPosition.y] = 200;
			}
			Sleep(150);
		}
		else if(playerPos > down) {
			if (gridPosition.y + gridElementHeight <= (verticalRes - gridElementHeight / 2)) {
				gridPosition.y += gridElementWidth;
				pastGridPosition.y = currentGridPosition.y;
				currentGridPosition.y += 1;
				battlefield[currentGridPosition.x][pastGridPosition.y] = 200;
			}
			Sleep(150);
		}
		else if (playerPos > left) {
			if (gridPosition.x - gridElementWidth >= gridElementWidth / 2) {
				gridPosition.x -= gridElementWidth;
				pastGridPosition.x = currentGridPosition.x;
				currentGridPosition.x -= 1;
				battlefield[pastGridPosition.x][currentGridPosition.y] = 200;
			}
			Sleep(150);
		}
		else if(playerPos > right) {
			if (gridPosition.x + gridElementWidth <= (horizontalRes - gridElementWidth / 2)) {
				gridPosition.x += gridElementWidth;
				pastGridPosition.x = currentGridPosition.x;
				currentGridPosition.x += 1;
				battlefield[pastGridPosition.x][currentGridPosition.y] = 200;
			}
			Sleep(150);
		}
	}
}

//Obstacle Struct
struct Obstacle {
	SDL_Texture* obstacleTexture;
	Vector2 obstaclePosition;
	Obstacle(Vector2 position, SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath);
	void PlaceObstacle();
};

Obstacle::Obstacle(Vector2 position, SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath) {
	obstaclePosition = position;
	PlaceObstacle();
	battlefield[MouseToGridPosition(obstaclePosition).x + 1][MouseToGridPosition(obstaclePosition).y + 1] = 255;
	obstacleTexture = SetTexture(surface, renderer, imagePath);
}

void Obstacle::PlaceObstacle() {
	obstaclePosition.x = ((obstaclePosition.x - 1) * gridElementWidth) + (gridElementWidth / 2);
	obstaclePosition.y = ((obstaclePosition.y - 1) * gridElementHeight) + (gridElementHeight / 2);
}

uint32_t CalculateDeltaTime(uint32_t* lastTick, uint32_t* currentTick) {
	*currentTick = SDL_GetTicks();
	uint32_t deltaTime = *currentTick - *lastTick;
	*lastTick = *currentTick;
	return deltaTime;
}

Vector2 MouseToGridPosition(Vector2 mousePos) {
	Vector2 gridPosition = { mousePos.x / gridElementWidth, mousePos.y / gridElementHeight };
	return gridPosition;
}

void GrassfireAlgorithm() {
	bool s = true;

	while (s) {

		s = false;

		for (int i = 0; i < 17; i++) {

			for (int j = 0; j < 13; j++) {

				uchar a = battlefield[i][j];

				if (a != 0 && a != 255) {
					uchar b = a + 1;
					uchar gridUp = battlefield[i][j - 1];
					uchar gridDown = battlefield[i][j + 1];
					uchar gridLeft = battlefield[i - 1][j];
					uchar gridRight = battlefield[i + 1][j];

					if (j > 0 && (gridUp != 255 && gridUp < b)) {
						if (battlefield[i][j - 1] == 0) {
							battlefield[i][j - 1] = b;
							s = true;
						}
					}
					if (j > gridHeight && (gridDown != 255 && gridDown < b)) {
						if (battlefield[i][j + 1] == 0) {
							battlefield[i][j + 1] = b;
							s = true;
						}
					}
					if (i > 0 && (gridLeft != 255 && gridLeft != b)) {
						if (battlefield[i - 1][j] == 0) {
							battlefield[i - 1][j] = b;
							s = true;
						}
					}
					if (i < gridWidth && (gridRight != 255 && gridRight != b)) {
						if (battlefield[i + 1][j] == 0) {
							battlefield[i + 1][j] = b;
							s = true;
						}
					}
				}
			}
		}
	}
}

void SetArraySides() {
	for (int i = 0; i < gridWidth; i++) {
		battlefield[i][0] = 255;
	}
	for (int i = 0; i < gridHeight; i++) {
		battlefield[0][i] = 255;
	}
	for (int i = 0; i < gridWidth; i++) {
		battlefield[i][gridHeight + 1] = 255;
	}
	for (int i = 0; i < gridHeight; i++) {
		battlefield[gridWidth + 1][i] = 255;
	}
}

void SetAllPositionsToZero() {
	for (int i = 0; i < (gridWidth + 2); i++) {
		for(int j = 0; j < (gridHeight + 2); i++){
			if (battlefield[i][j] != 255) {
				battlefield[i][j] = 0;
			}
		}
	}
}

int GetRandomXColumn() {
	//int randomX = (rand() % (13 - 3 + 1)) + 3;
	return (rand() % (13 - 3 + 1)) + 3;
}

int GetRandomYColumn() {
	return (rand() % (9 - 2 + 1)) + 2;
}

Vector2 GetRandomGridPosition() {
	int randomX = GetRandomYColumn();
	int randomY = GetRandomYColumn();

	if (battlefield[randomX][randomY] != 255) {
		Vector2 tempVector = { randomX, randomY };
		return tempVector;
	}
	else {
		GetRandomGridPosition();
	}

}

void PlayTurn(Character* player, Character* ai, bool* isPlayerMoving, bool* isPlayerFinishedMoving, bool* isAiMoving, int* turn, int nextTurn, Vector2 mousePosition) {
	if (*isPlayerMoving) {

		player->MoveCharacter(MouseToGridPosition(mousePosition));

		if (player->currentGridPosition.x == player->gridDestination.x && player->currentGridPosition.y == player->gridDestination.y) {
			player->gridDestination.x = 0;
			player->gridDestination.y = 0;
			*isPlayerMoving = false;
			*isPlayerFinishedMoving = true;
			*isAiMoving = true;
			SetAllPositionsToZero();
			battlefield[player->currentGridPosition.x][player->currentGridPosition.y] = 255;
		}
	}
	if (*isPlayerFinishedMoving && *isAiMoving) {

		ai->MoveCharacter(GetRandomGridPosition());

		if (ai->currentGridPosition.x == ai->gridDestination.x && ai->currentGridPosition.y == ai->gridDestination.y) {
			ai->gridDestination.x = 0;
			ai->gridDestination.y = 0;
			*isAiMoving = false;
			SetAllPositionsToZero();
			battlefield[ai->currentGridPosition.x][ai->currentGridPosition.y] = 255;
			*turn = nextTurn;
		}
	}
}

SDL_Texture* SetTexture(SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath) {
	surface = IMG_Load(imagePath);
	if (!surface)
	{
		printf("Unable to load an image %s. Error: %s", imagePath, IMG_GetError());
		return NULL;
	}

	// Now we use the renderer and the surface to create a texture which we later can draw on the screen.
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!texture)
	{
		printf("Unable to create a texture. Error: %s", SDL_GetError());
		return NULL;
	}
	SDL_FreeSurface(surface);

	return texture;
}

void SetRect(SDL_Rect* rect, Vector2 position) {
	rect->x = (int)round(position.x - gridElementWidth / 2); // Counting from the image's center but that's up to you
	rect->y = (int)round(position.y - gridElementHeight / 2); // Counting from the image's center but that's up to you
	rect->w = (int)gridElementWidth;
	rect->h = (int)gridElementHeight;

}

void DrawImage(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect rect)
{
	SDL_RenderCopyEx(renderer, // Already know what is that
		texture, // The image
		nullptr, // A rectangle to crop from the original image. Since we need the whole image that can be left empty (nullptr)
		&rect, // The destination rectangle on the screen.
		0, // An angle in degrees for rotation
		nullptr, // The center of the rotation (when nullptr, the rect center is taken)
		SDL_FLIP_NONE); // We don't want to flip the image
}

bool InitializeSDL(SDL_Renderer** renderer, SDL_Window** window) {
	// Init SDL libraries
	SDL_SetMainReady(); // Just leave it be
	int result = 0;
	result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // Init of the main SDL library
	if (result) // SDL_Init returns 0 (false) when everything is OK
	{
		printf("Can't initialize SDL. Error: %s", SDL_GetError()); // SDL_GetError() returns a string (as const char*) which explains what went wrong with the last operation
		return false;
	}

	result = IMG_Init(IMG_INIT_PNG); // Init of the Image SDL library. We only need to support PNG for this project
	if (!(result & IMG_INIT_PNG)) // Checking if the PNG decoder has started successfully
	{
		printf("Can't initialize SDL image. Error: %s", SDL_GetError());
		return false;
	}

	// Creating the window 1920x1080 (could be any other size)
	*window = SDL_CreateWindow("HeroesOfMM",
		0, 0,
		horizontalRes, verticalRes,
		SDL_WINDOW_SHOWN);

	if (!*window)
		return false;

	// Creating a renderer which will draw things on the screen
	*renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
	if (!*renderer)
		return false;

	// Setting the color of an empty window (RGBA). You are free to adjust it.
	SDL_SetRenderDrawColor(*renderer, 153, 255, 102, 255);

	return true;
}

int main() {
	int turn = 0;
	bool isPlayerMoving = false;
	bool hasPlayerHinishedMove = false;
	bool isAiMoving = false;

	srand(time(nullptr));

	Vector2 mousePosition = { 0, 0 };

	SDL_Event sdlEvent;
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;
	SDL_Surface* surface = nullptr;

	bool isSDLInitialized = InitializeSDL(&renderer, &window);

	if (!isSDLInitialized) {
		return -1;
	}

	//Ally block
	Character char1({ 1,2 }, surface, renderer, "img/image.png");
	Character char2({ 1,3 }, surface, renderer, "img/image.png");
	Character char3({ 1,4 }, surface, renderer, "img/image.png");
	Character char4({ 1,5 }, surface, renderer, "img/image.png");
	Character char5({ 1,6 }, surface, renderer, "img/image.png");
	Character char6({ 1,7 }, surface, renderer, "img/image.png");
	Character char7({ 1,8 }, surface, renderer, "img/image.png");
	Character char8({ 1,9 }, surface, renderer, "img/image.png");

	//Enemy block
	Character enemy1({ 15,2 }, surface, renderer, "img/image.png");
	Character enemy2({ 15,3 }, surface, renderer, "img/image.png");
	Character enemy3({ 15,4 }, surface, renderer, "img/image.png");
	Character enemy4({ 15,5 }, surface, renderer, "img/image.png");
	Character enemy5({ 15,6 }, surface, renderer, "img/image.png");
	Character enemy6({ 15,7 }, surface, renderer, "img/image.png");
	Character enemy7({ 15,8 }, surface, renderer, "img/image.png");
	Character enemy8({ 15,9 }, surface, renderer, "img/image.png");

	Character allUnits[]{
		char1, enemy1, char2, enemy2, char3, enemy3, char4, enemy4, char5, enemy5, char6, enemy6, char7, enemy7, char8, enemy8
	};

	Obstacle obstacle1({ GetRandomXColumn(), GetRandomYColumn() }, surface, renderer, "img/obstacle.png");
	Obstacle obstacle2({ GetRandomXColumn(), GetRandomYColumn() }, surface, renderer, "img/obstacle.png");
	Obstacle obstacle3({ GetRandomXColumn(), GetRandomYColumn() }, surface, renderer, "img/obstacle.png");
	Obstacle obstacle4({ GetRandomXColumn(), GetRandomYColumn() }, surface, renderer, "img/obstacle.png");
	Obstacle obstacle5({ GetRandomXColumn(), GetRandomYColumn() }, surface, renderer, "img/obstacle.png");
	Obstacle obstacle6({ GetRandomXColumn(), GetRandomYColumn() }, surface, renderer, "img/obstacle.png");
	Obstacle obstacle7({ GetRandomXColumn(), GetRandomYColumn() }, surface, renderer, "img/obstacle.png");
	Obstacle obstacle8({ GetRandomXColumn(), GetRandomYColumn() }, surface, renderer, "img/obstacle.png");

	SDL_FreeSurface(surface);

	int texWidth = gridElementWidth;
	int texHeight = gridElementHeight;

	SetArraySides();
	// The main loop
	// Every iteration is a frame
	bool done = false;
	while (!done)
	{
		// Polling the messages from the OS.
		// That could be key downs, mouse movement, ALT+F4 or many others
		while (SDL_PollEvent(&sdlEvent))
		{
			if (sdlEvent.type == SDL_QUIT) // The user wants to quit
			{
				done = true;
			}
			else if (sdlEvent.type == SDL_KEYDOWN) // A key was pressed
			{
				switch (sdlEvent.key.keysym.sym) // Which key?
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
			else if (sdlEvent.type == SDL_MOUSEBUTTONDOWN)
			{
				if (sdlEvent.button.button == SDL_BUTTON_RIGHT)
				{
					isPlayerMoving = true;
					SetAllPositionsToZero();

					SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
				}
			}
		}

		// Clearing the screen
		SDL_RenderClear(renderer);

		switch (turn) {
		case 0:
			PlayTurn(&char1, &enemy1, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 1, mousePosition);
			break;
		case 1:
			PlayTurn(&char2, &enemy2, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 2, mousePosition);
			break;
		case 2:
			PlayTurn(&char3, &enemy3, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 3, mousePosition);
			break;
		case 3:
			PlayTurn(&char4, &enemy4, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 4, mousePosition);
			break;
		case 4:
			PlayTurn(&char5, &enemy5, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 5, mousePosition);
			break;
		case 5:
			PlayTurn(&char6, &enemy6, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 6, mousePosition);
			break;
		case 6:
			PlayTurn(&char7, &enemy7, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 7, mousePosition);
			break;
		case 7:
			PlayTurn(&char8, &enemy8, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 8, mousePosition);
			break;
		default:
			printf("Something went wrong here...");
			break;
		}

		// Here is the rectangle where the image will be on the screen
		SDL_Rect rectChar1;
		SDL_Rect rectChar2;
		SDL_Rect rectChar3;
		SDL_Rect rectChar4;
		SDL_Rect rectChar5;
		SDL_Rect rectChar6;
		SDL_Rect rectChar7;
		SDL_Rect rectChar8;

		SDL_Rect rectEnemy1;
		SDL_Rect rectEnemy2;
		SDL_Rect rectEnemy3;
		SDL_Rect rectEnemy4;
		SDL_Rect rectEnemy5;
		SDL_Rect rectEnemy6;
		SDL_Rect rectEnemy7;
		SDL_Rect rectEnemy8;

		SDL_Rect rectObstacle1;
		SDL_Rect rectObstacle2;
		SDL_Rect rectObstacle3;
		SDL_Rect rectObstacle4;
		SDL_Rect rectObstacle5;
		SDL_Rect rectObstacle6;
		SDL_Rect rectObstacle7;
		SDL_Rect rectObstacle8;

		SetRect(&rectChar1, char1.gridPosition);
		SetRect(&rectChar2, char2.gridPosition);
		SetRect(&rectChar3, char3.gridPosition);
		SetRect(&rectChar4, char4.gridPosition);
		SetRect(&rectChar5, char5.gridPosition);
		SetRect(&rectChar6, char6.gridPosition);
		SetRect(&rectChar7, char7.gridPosition);
		SetRect(&rectChar8, char8.gridPosition);

		SetRect(&rectEnemy1, enemy1.gridPosition);
		SetRect(&rectEnemy2, enemy2.gridPosition);
		SetRect(&rectEnemy3, enemy3.gridPosition);
		SetRect(&rectEnemy4, enemy4.gridPosition);
		SetRect(&rectEnemy5, enemy5.gridPosition);
		SetRect(&rectEnemy6, enemy6.gridPosition);
		SetRect(&rectEnemy7, enemy7.gridPosition);
		SetRect(&rectEnemy8, enemy8.gridPosition);

		SetRect(&rectObstacle1, obstacle1.obstaclePosition);
		SetRect(&rectObstacle2, obstacle2.obstaclePosition);
		SetRect(&rectObstacle3, obstacle3.obstaclePosition);
		SetRect(&rectObstacle4, obstacle4.obstaclePosition);
		SetRect(&rectObstacle5, obstacle5.obstaclePosition);
		SetRect(&rectObstacle6, obstacle6.obstaclePosition);
		SetRect(&rectObstacle7, obstacle7.obstaclePosition);
		SetRect(&rectObstacle8, obstacle8.obstaclePosition);

		DrawImage(renderer, char1.texture, rectChar1);
		DrawImage(renderer, char2.texture, rectChar2);
		DrawImage(renderer, char3.texture, rectChar3);
		DrawImage(renderer, char4.texture, rectChar4);
		DrawImage(renderer, char5.texture, rectChar5);
		DrawImage(renderer, char6.texture, rectChar6);
		DrawImage(renderer, char7.texture, rectChar7);
		DrawImage(renderer, char8.texture, rectChar8);

		DrawImage(renderer, enemy1.texture, rectEnemy1);
		DrawImage(renderer, enemy2.texture, rectEnemy2);
		DrawImage(renderer, enemy3.texture, rectEnemy3);
		DrawImage(renderer, enemy4.texture, rectEnemy4);
		DrawImage(renderer, enemy5.texture, rectEnemy5);
		DrawImage(renderer, enemy6.texture, rectEnemy6);
		DrawImage(renderer, enemy7.texture, rectEnemy7);
		DrawImage(renderer, enemy8.texture, rectEnemy8);

		DrawImage(renderer, obstacle1.obstacleTexture, rectObstacle1);
		DrawImage(renderer, obstacle2.obstacleTexture, rectObstacle2);
		DrawImage(renderer, obstacle3.obstacleTexture, rectObstacle3);
		DrawImage(renderer, obstacle4.obstacleTexture, rectObstacle4);
		DrawImage(renderer, obstacle5.obstacleTexture, rectObstacle5);
		DrawImage(renderer, obstacle6.obstacleTexture, rectObstacle6);
		DrawImage(renderer, obstacle7.obstacleTexture, rectObstacle7);
		DrawImage(renderer, obstacle8.obstacleTexture, rectObstacle8);

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
