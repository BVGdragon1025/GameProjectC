#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SDL_MAIN_HANDLED
#include "libs/SDL2/SDL.h"
#include "libs/SDL2/SDL_image.h"
#include "libs/SDL2/SDL_ttf.h"
#include "main.h"
#include <Windows.h>

typedef unsigned char uchar;

const char fontPath[] = "fonts/FreeSansBold-Xgdd.ttf";
const int fontSize = 24;

const int horizontalRes = 1280;
const int verticalRes = 720;

const int gridWidth = 15;
const int gridHeight = 11;

const int gridElementWidth = horizontalRes / gridWidth;
const int gridElementHeight = verticalRes / gridHeight;

uchar battlefield[13][17];

int allyUnitsCount = 8;
int enemyUnitsCount = 8;

SDL_Texture* SetTexture(SDL_Surface* surface, SDL_Renderer* renderer, const char* image_path);
SDL_Renderer* publicRenderer;

//Vector2 Struct
struct Vector2 {
	int x;
	int y;
};


//Character Struct
struct Character {
	

	SDL_Texture* texture;
	SDL_Texture* textTexture;
	SDL_Rect rect;
	SDL_Rect textRect;

	Vector2 gridPosition;
	Vector2 gridDestination;
	Vector2 currentGridPosition = { 0,0 };
	Vector2 pastGridPosition = { 0,0 };

	float unitHealth = 0;
	float unitCount = 0;
	float damageValue = 0;

	bool isDead = false;
	bool disableImage = false;
	bool updateHealth = false;

	Character(Vector2 position, SDL_Surface* surface, SDL_Renderer* renderer, SDL_Surface* textSurface , const char* imagePath, TTF_Font* font, float uHealth, float uCount, float damage);
	void PlaceCharacterOnGrid(Vector2 position);
	void MoveCharacter(Vector2 destination);
	void AttackEnemy();
	void AttackEnemy(Character* enemy);
	void HealthUpdate(SDL_Renderer* renderer, SDL_Surface* textSurface, TTF_Font* font);


	~Character();
};

Character::Character(Vector2 position, SDL_Surface* surface, SDL_Renderer* renderer, SDL_Surface* textSurface , const char* imagePath, TTF_Font* font , float uHealth, float uCount, float damage)
{

	unitHealth = uHealth;
	unitCount = uCount;
	damageValue = damage;
	PlaceCharacterOnGrid(position);
	battlefield[MouseToGridPosition(gridPosition).y + 1][MouseToGridPosition(gridPosition).x + 1] = 255;
	texture = SetTexture(surface, renderer, imagePath);
	textSurface = TTF_RenderText_Solid(font, CastToArray(unitCount), { 64,0,255 });
	textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
}

void Character::PlaceCharacterOnGrid(Vector2 position) {
	gridPosition.x = ((position.x - 1) * gridElementWidth) + (gridElementWidth / 2);
	gridPosition.y = ((position.y - 1) * gridElementHeight) + (gridElementHeight/ 2);
}

void Character::MoveCharacter(Vector2 destination) {
	gridDestination = destination;
	currentGridPosition = { gridPosition.x / gridElementWidth, gridPosition.y / gridElementHeight };
	gridDestination.x += 1;
	gridDestination.y += 1;
	currentGridPosition.x += 1;
	currentGridPosition.y += 1;

	if (battlefield[gridDestination.y][gridDestination.x] != 255 && battlefield[gridDestination.y][gridDestination.x] != 200) {
		battlefield[gridDestination.y][gridDestination.x] = 1;
	}

	uchar playerDestination = battlefield[gridDestination.y][gridDestination.x];
	uchar playerPos = battlefield[currentGridPosition.y][currentGridPosition.x];
	uchar up = battlefield[currentGridPosition.y - 1][currentGridPosition.x];
	uchar down = battlefield[currentGridPosition.y + 1][currentGridPosition.x];
	uchar left = battlefield[currentGridPosition.y][currentGridPosition.x - 1];
	uchar right = battlefield[currentGridPosition.y][currentGridPosition.x + 1];

	GrassfireAlgorithm();

	if (playerDestination != 255 && playerDestination != 200) {
		if (playerPos > up) {
			if (gridPosition.y - gridElementHeight >= gridElementHeight / 2) {
				gridPosition.y -= gridElementHeight;
				pastGridPosition.y = currentGridPosition.y;
				currentGridPosition.y -= 1;
				battlefield[pastGridPosition.y][currentGridPosition.x] = 200;
			}
			Sleep(150);
		}
		else if(playerPos > down) {
			if (gridPosition.y + gridElementHeight <= (verticalRes - gridElementHeight / 2)) {
				gridPosition.y += gridElementWidth;
				pastGridPosition.y = currentGridPosition.y;
				currentGridPosition.y += 1;
				battlefield[pastGridPosition.y][currentGridPosition.x] = 200;
			}
			Sleep(150);
		}
		else if (playerPos > left) {
			if (gridPosition.x - gridElementWidth >= gridElementWidth / 2) {
				gridPosition.x -= gridElementWidth;
				pastGridPosition.x = currentGridPosition.x;
				currentGridPosition.x -= 1;
				battlefield[currentGridPosition.y][pastGridPosition.x] = 200;
			}
			Sleep(150);
		}
		else if(playerPos > right) {
			if (gridPosition.x + gridElementWidth <= (horizontalRes - gridElementWidth / 2)) {
				gridPosition.x += gridElementWidth;
				pastGridPosition.x = currentGridPosition.x;
				currentGridPosition.x += 1;
				battlefield[currentGridPosition.y][pastGridPosition.x] = 200;
			}
			Sleep(150);
		}
	}
}

void Character::AttackEnemy(Character* enemy) {
	int totalEnemyHealth = enemy->unitHealth * enemy->unitCount;
	int totalHealth = unitHealth * unitCount;
	int totalDamage = damageValue * unitCount;
	int updatedEnemyHealth = totalEnemyHealth -= totalDamage;

	if (updatedEnemyHealth > 0) {
		int enemyUnitsDead = totalDamage / enemy->unitHealth;
		enemy->unitCount = enemy->unitCount - enemyUnitsDead;
		enemy->updateHealth = true;

		int totalEnemyDamage = enemy->damageValue * enemy->unitCount;
		int updatedHealth = totalHealth -= totalEnemyDamage;
		if (updatedHealth > 0) {
			int unitsDead = totalEnemyDamage / unitHealth;
			unitCount -= unitsDead;
			updateHealth = true;
		}
		else {
			isDead = true;
			disableImage = true;
			battlefield[currentGridPosition.y][currentGridPosition.x] = 100;
		}
	}
	else {
		enemy->isDead = true;
		enemy->disableImage = true;
		battlefield[currentGridPosition.y][currentGridPosition.x] = 100;
	}
}

void Character::HealthUpdate(SDL_Renderer* renderer, SDL_Surface* textSurface, TTF_Font* font) {
	const char* unitCountText = CastToArray(unitCount);

	if (!isDead) {
		textSurface = TTF_RenderText_Solid(font, unitCountText, { 0,0,255 });
		textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
	}
}

Character::~Character(){}

//Obstacle Struct
struct Obstacle {
	SDL_Texture* obstacleTexture;
	SDL_Rect obstacleRect;
	Vector2 obstaclePosition;
	Obstacle(Vector2 position, SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath);
	void PlaceObstacle();
};

Obstacle::Obstacle(Vector2 position, SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath) {
	obstaclePosition = position;
	PlaceObstacle();
	battlefield[MouseToGridPosition(obstaclePosition).y + 1][MouseToGridPosition(obstaclePosition).x + 1] = 255;
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
	Vector2 gridPos = { mousePos.x / gridElementWidth, mousePos.y / gridElementHeight };
	return gridPos;
}

void GrassfireAlgorithm() {
	bool s = true;

	while (s) {

		s = false;

		for (int i = 0; i < (gridHeight + 2); i++) {

			for (int j = 0; j < (gridWidth + 2); j++) {

				uchar a = battlefield[i][j];

				if (a != 0 && a != 255) {
					uchar b = a + 1;
					uchar gridUp = battlefield[i - 1][j];
					uchar gridDown = battlefield[i + 1][j];
					uchar gridLeft = battlefield[i][j - 1];
					uchar gridRight = battlefield[i][j + 1];

					if (i > 0 && (gridUp != 255 && gridUp < b)) {
						if (battlefield[i - 1][j] == 0) {
							battlefield[i - 1][j] = b;
							s = true;
						}
					}
					if (j < gridHeight && (gridDown != 255 && gridDown < b)) {
						if (battlefield[i + 1][j] == 0) {
							battlefield[i + 1][j] = b;
							s = true;
						}
					}
					if (i > 0 && (gridLeft != 255 && gridLeft != b)) {
						if (battlefield[i][j - 1] == 0) {
							battlefield[i][j - 1] = b;
							s = true;
						}
					}
					if (i < gridWidth && (gridRight != 255 && gridRight != b)) {
						if (battlefield[i][j + 1] == 0) {
							battlefield[i][j + 1] = b;
							s = true;
						}
					}
				}
			}
		}
	}
}

Vector2 SetEnemyDestination(Vector2 location) {
	if (battlefield[location.y + 1][location.x + 2] != 255 && battlefield[location.y + 1][location.x + 2] != 200) {
		return Vector2{ location.x + 1, location.y };
	}
	else if (battlefield[location.y + 1][location.x] != 255 && battlefield[location.y + 1][location.x] != 200) {
		return Vector2{ location.x - 1,location.y };
	}
	else if (battlefield[location.y + 2][location.x + 1] != 255 && battlefield[location.y + 2][location.x + 1] != 200) {
		return Vector2{ location.x, location.y + 1 };
	}
	else if (battlefield[location.y][location.x + 1] != 255 && battlefield[location.y][location.x + 1] != 200) {
		return Vector2{ location.x, location.y - 1 };
	}
}

void SetArraySides() {
	for (int i = 0; i < gridWidth; i++) {
		battlefield[0][i] = 255;
	}
	for (int i = 0; i < gridHeight; i++) {
		battlefield[i][0] = 255;
	}
	for (int i = 0; i < gridWidth; i++) {
		battlefield[gridHeight + 1][i] = 255;
	}
	for (int i = 0; i < gridHeight; i++) {
		battlefield[i][gridWidth + 1] = 255;
	}
}

void SetAllPositionsToZero() {
	for (int i = 0; i < (gridHeight + 2); i++) {
		for(int j = 0; j < (gridWidth + 2); j++){
			if (battlefield[i][j] != 255) {
				battlefield[i][j] = 0;
			}
		}
	}
}

char* CastToArray(int number) {
	int n = log10(number) + 1;
	char* numberArray = (char*)calloc(n, sizeof(char));
	numberArray[n] = '\0';
	for (int i = n - 1; i >= 0; --i, number /= 10) {
		numberArray[i] = (number % 10) + '0';
	}
	return numberArray;
}

int GetRandomX() {
	//int randomX = (rand() % (13 - 3 + 1)) + 3;
	return (rand() % (13 - 3 + 1)) + 3;
}

int GetRandomY() {
	return (rand() % (9 - 2 + 1)) + 2;
}

int GetRandomEnemy(Character* characters[]) {
	int random = (rand() % (7 - 0 + 1)) + 0;
	if (!characters[random]->isDead) {
		return random;
	}
	else {
		GetRandomEnemy(characters);
	}
}

Vector2 GetRandomGridPosition() {
	int randomX = GetRandomY();
	int randomY = GetRandomY();

	if (battlefield[randomX][randomY] != 255) {
		Vector2 tempVector = { randomX, randomY };
		return tempVector;
	}
	else {
		GetRandomGridPosition();
	}

}

void PlayTurn(Character* player, Character* ai, bool* isPlayerMoving, bool* isPlayerFinishedMoving, bool* isAiMoving, int* turn, int nextTurn, Vector2 mousePosition, Character* enemyTarget, bool* playerMarkedEnemy, Character* playerTarget, int randomInt) {
	if (*isPlayerMoving) {
		if (!player->isDead) {
			if (*playerMarkedEnemy) {
				player->MoveCharacter(SetEnemyDestination({playerTarget->currentGridPosition.x -1, playerTarget->currentGridPosition.y -1}));

				if (player->currentGridPosition.x == player->gridDestination.x && player->currentGridPosition.y == player->gridDestination.y) {
					player->gridDestination.x = 0;
					player->gridDestination.y = 0;
					*isPlayerMoving = false;
					*isPlayerFinishedMoving = true;
					*isAiMoving = true;
					*playerMarkedEnemy = false;
					SetAllPositionsToZero();
					battlefield[player->currentGridPosition.y][player->currentGridPosition.x] = 255;
				}
			}
			else {
				player->MoveCharacter(MouseToGridPosition(mousePosition));

				if (player->currentGridPosition.x == player->gridDestination.x && player->currentGridPosition.y == player->gridDestination.y) {
					player->gridDestination.x = 0;
					player->gridDestination.y = 0;
					*isPlayerMoving = false;
					*isPlayerFinishedMoving = true;
					*isAiMoving = true;
					SetAllPositionsToZero();
					battlefield[player->currentGridPosition.y][player->currentGridPosition.x] = 255;
				}
			}
		}
		else if (player->isDead) {
			player->gridDestination.x = 0;
			player->gridDestination.y = 0;
			*isPlayerMoving = false;
			*isPlayerFinishedMoving = true;
			*isAiMoving = true;
			*playerMarkedEnemy = false;
			SetAllPositionsToZero();
			battlefield[player->currentGridPosition.y][player->currentGridPosition.x] = 100;
		}

		
	}
	if (*isPlayerFinishedMoving && *isAiMoving) {
		if (!ai->isDead) {
			if (randomInt < 6) {
				ai->MoveCharacter(SetEnemyDestination({ ai->currentGridPosition.x - 1, ai->currentGridPosition.y - 1 }));
			}
			else {
				ai->MoveCharacter(GetRandomGridPosition());
			}

			if (ai->currentGridPosition.x == ai->gridDestination.x && ai->currentGridPosition.y == ai->gridDestination.y) {
				if (randomInt < 6) {
					ai->AttackEnemy(enemyTarget);
				}

				ai->gridDestination.x = 0;
				ai->gridDestination.y = 0;
				*isAiMoving = false;
				SetAllPositionsToZero();
				battlefield[ai->currentGridPosition.y][ai->currentGridPosition.x] = 255;
				*turn = nextTurn;
			}
		}
		else if (ai->isDead) {
			ai->gridDestination.x = 0;
			ai->gridDestination.y = 0;
			*isAiMoving = false;
			SetAllPositionsToZero();
			battlefield[ai->currentGridPosition.y][ai->currentGridPosition.x] = 100;
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

void SetTextRect(SDL_Rect* rect, Vector2 position) {
	rect->x = (int)round(position.x - gridElementWidth / 2); // Counting from the image's center but that's up to you
	rect->y = (int)round(position.y - gridElementHeight / 2); // Counting from the image's center but that's up to you
	rect->w = fontSize;
	rect->h = fontSize;
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

TTF_Font* GetCurrentFont() {
	if (TTF_Init() < 0) {
		abort();
	}
	TTF_Font* font = TTF_OpenFont(fontPath, fontSize);
	if (font) {
		return font;
	}
	else {
		abort();
	}
		
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
	*window = SDL_CreateWindow("HOMM",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
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
	int randomInteger = 0;
	int aiTarget = 0;

	int turn = 0;
	bool isPlayerMoving = false;
	bool hasPlayerHinishedMove = false;
	bool isAiMoving = false;
	bool playerMarkedEnemy = false;

	int enemyUnitIndex = 0;

	srand(time(nullptr));

	Vector2 mousePosition = { 0, 0 };

	SDL_Event sdlEvent;
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;
	SDL_Surface* surface = nullptr;

	SDL_Surface* textSurface = nullptr;
	TTF_Font* font = GetCurrentFont();

	bool isSDLInitialized = InitializeSDL(&renderer, &window);

	if (!isSDLInitialized) {
		return -1;
	}

	//Ally block
	Character char1({ 1,2 }, surface, renderer, textSurface, "img/ally/cyclops.png", font, 30, 5, 20);
	Character char2({ 1,3 }, surface, renderer, textSurface, "img/ally/double-dragon.png", font, 15, 5, 30);
	Character char3({ 1,4 }, surface, renderer, textSurface, "img/ally/evil-minion.png", font, 5, 30, 3);
	Character char4({ 1,5 }, surface, renderer, textSurface, "img/ally/hydra.png", font, 20, 6, 15);
	Character char5({ 1,6 }, surface, renderer, textSurface, "img/ally/imp-laugh.png", font, 3, 40, 4);
	Character char6({ 1,7 }, surface, renderer, textSurface, "img/ally/minotaur.png", font, 25, 4, 32);
	Character char7({ 1,8 }, surface, renderer, textSurface, "img/ally/sea-dragon.png", font, 16, 5, 28);
	Character char8({ 1,9 }, surface, renderer, textSurface, "img/ally/wyvern.png", font, 18, 6, 26);

	//Enemy block
	Character enemy1({ 15,2 }, surface, renderer, textSurface, "img/enemy/brute.png", font, 15, 10, 8);
	Character enemy2({ 15,3 }, surface, renderer, textSurface, "img/enemy/bully-minion.png", font, 6, 25, 6);
	Character enemy3({ 15,4 }, surface, renderer, textSurface, "img/enemy/centaur.png", font, 18, 10, 5);
	Character enemy4({ 15,5 }, surface, renderer, textSurface, "img/enemy/fairy.png", font, 6, 25, 4);
	Character enemy5({ 15,6 }, surface, renderer, textSurface, "img/enemy/griffin-symbol.png", font, 12, 25, 12);
	Character enemy6({ 15,7 }, surface, renderer, textSurface, "img/enemy/mermaid.png", font, 8, 15, 9);
	Character enemy7({ 15,8 }, surface, renderer, textSurface, "img/enemy/spiked-dragon-head.png", font, 18, 6, 26);
	Character enemy8({ 15,9 }, surface, renderer, textSurface, "img/enemy/unicorn.png", font, 14, 16, 8);

	/*Character allUnits[]{
		char1, enemy1, char2, enemy2, char3, enemy3, char4, enemy4, char5, enemy5, char6, enemy6, char7, enemy7, char8, enemy8
	};*/

	Character* allyUnits[8] = { &char1, &char2, &char3, &char4, &char5, &char6, &char7, &char8 };
	Character* enemyUnits[8] = { &enemy1, &enemy2, &enemy3, &enemy4, &enemy5, &enemy6, &enemy7, &enemy8 };

	Obstacle obstacle1({ GetRandomX(), GetRandomY() }, surface, renderer, "img/obstacle.png");
	Obstacle obstacle2({ GetRandomX(), GetRandomY() }, surface, renderer, "img/obstacle.png");
	Obstacle obstacle3({ GetRandomX(), GetRandomY() }, surface, renderer, "img/obstacle.png");
	Obstacle obstacle4({ GetRandomX(), GetRandomY() }, surface, renderer, "img/obstacle.png");
	Obstacle obstacle5({ GetRandomX(), GetRandomY() }, surface, renderer, "img/obstacle.png");
	Obstacle obstacle6({ GetRandomX(), GetRandomY() }, surface, renderer, "img/obstacle.png");
	Obstacle obstacle7({ GetRandomX(), GetRandomY() }, surface, renderer, "img/obstacle.png");
	Obstacle obstacle8({ GetRandomX(), GetRandomY() }, surface, renderer, "img/obstacle.png");

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
				if (sdlEvent.button.button == SDL_BUTTON_LEFT)
				{
					isPlayerMoving = true;
					SetAllPositionsToZero();

					SDL_GetMouseState(&mousePosition.x, &mousePosition.y);
					aiTarget = GetRandomEnemy(allyUnits);
					randomInteger = GetRandomY();

					Vector2 grid = MouseToGridPosition(mousePosition);

					for (int i = 0; i < 8; i++) {
						if (enemyUnits[i]->currentGridPosition.x == grid.x + 1 && enemyUnits[i]->currentGridPosition.y == grid.y + 1) {
							playerMarkedEnemy = true;
							enemyUnitIndex = i;
						}
					}
				}
			}
		}

		// Clearing the screen
		SDL_RenderClear(renderer);

		switch (turn) {
		case 0:
			PlayTurn(&char1, &enemy1, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 1, mousePosition, allyUnits[aiTarget], &playerMarkedEnemy, enemyUnits[enemyUnitIndex], randomInteger);
			break;
		case 1:
			PlayTurn(&char2, &enemy2, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 2, mousePosition, allyUnits[aiTarget], &playerMarkedEnemy, enemyUnits[enemyUnitIndex], randomInteger);
			break;
		case 2:
			PlayTurn(&char3, &enemy3, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 3, mousePosition, allyUnits[aiTarget], &playerMarkedEnemy, enemyUnits[enemyUnitIndex], randomInteger);
			break;
		case 3:
			PlayTurn(&char4, &enemy4, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 4, mousePosition, allyUnits[aiTarget], &playerMarkedEnemy, enemyUnits[enemyUnitIndex], randomInteger);
			break;
		case 4:
			PlayTurn(&char5, &enemy5, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 5, mousePosition, allyUnits[aiTarget], &playerMarkedEnemy, enemyUnits[enemyUnitIndex], randomInteger);
			break;
		case 5:
			PlayTurn(&char6, &enemy6, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 6, mousePosition, allyUnits[aiTarget], &playerMarkedEnemy, enemyUnits[enemyUnitIndex], randomInteger);
			break;
		case 6:
			PlayTurn(&char7, &enemy7, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 7, mousePosition, allyUnits[aiTarget], &playerMarkedEnemy, enemyUnits[enemyUnitIndex], randomInteger);
			break;
		case 7:
			PlayTurn(&char8, &enemy8, &isPlayerMoving, &hasPlayerHinishedMove, &isAiMoving, &turn, 8, mousePosition, allyUnits[aiTarget], &playerMarkedEnemy, enemyUnits[enemyUnitIndex], randomInteger);
			break;
		default:
			break;
		}

		// Here is the rectangle where the image will be on the screen

		for (int i = 0; i < 8; i++) {
			SetRect(&allyUnits[i]->rect, allyUnits[i]->gridPosition);
			SetRect(&enemyUnits[i]->rect, enemyUnits[i]->gridPosition);
			DrawImage(renderer, allyUnits[i]->texture, allyUnits[i]->rect);
			DrawImage(renderer, enemyUnits[i]->texture, enemyUnits[i]->rect);

			SetTextRect(&allyUnits[i]->textRect, allyUnits[i]->gridPosition);
			SetTextRect(&enemyUnits[i]->textRect, enemyUnits[i]->gridPosition);
			DrawImage(renderer, allyUnits[i]->textTexture, allyUnits[i]->textRect);
			DrawImage(renderer, enemyUnits[i]->textTexture, enemyUnits[i]->textRect);
		}

		for (int i = 0; i < 8; i++) {
			if (enemyUnits[i]->updateHealth && !enemyUnits[i]->isDead) {
				enemyUnits[i]->HealthUpdate(renderer, textSurface, font);
				enemyUnits[i]->updateHealth = false;
			}
			if (allyUnits[i]->updateHealth && !allyUnits[i]->isDead) {
				allyUnits[i]->HealthUpdate(renderer, textSurface, font);
				allyUnits[i]->updateHealth = false;
			}
		}

		for (int i = 0; i < 8; i++) {
			if (allyUnits[i]->isDead && allyUnits[i]->disableImage) {
				allyUnits[i]->texture = SetTexture(surface, renderer, "img/ally/tombstone.png");
				SDL_DestroyTexture(allyUnits[i]->textTexture);
				allyUnits[i]->textTexture = SetTexture(surface, renderer, "img/no_health.png");
				allyUnits[i]->disableImage = false;
			}
			if (enemyUnits[i]->isDead && enemyUnits[i]->disableImage) {
				enemyUnits[i]->texture = SetTexture(surface, renderer, "img/ally/tombstone.png");
				SDL_DestroyTexture(enemyUnits[i]->textTexture);
				enemyUnits[i]->textTexture = SetTexture(surface, renderer, "img/no_health.png");
				enemyUnits[i]->disableImage = false;
			}
		}

		SetRect(&obstacle1.obstacleRect, obstacle1.obstaclePosition);
		SetRect(&obstacle2.obstacleRect, obstacle2.obstaclePosition);
		SetRect(&obstacle3.obstacleRect, obstacle3.obstaclePosition);
		SetRect(&obstacle4.obstacleRect, obstacle4.obstaclePosition);
		SetRect(&obstacle5.obstacleRect, obstacle5.obstaclePosition);
		SetRect(&obstacle6.obstacleRect, obstacle6.obstaclePosition);
		SetRect(&obstacle7.obstacleRect, obstacle7.obstaclePosition);
		SetRect(&obstacle8.obstacleRect, obstacle8.obstaclePosition);


		DrawImage(renderer, obstacle1.obstacleTexture, obstacle1.obstacleRect);
		DrawImage(renderer, obstacle2.obstacleTexture, obstacle2.obstacleRect);
		DrawImage(renderer, obstacle3.obstacleTexture, obstacle3.obstacleRect);
		DrawImage(renderer, obstacle4.obstacleTexture, obstacle4.obstacleRect);
		DrawImage(renderer, obstacle5.obstacleTexture, obstacle5.obstacleRect);
		DrawImage(renderer, obstacle6.obstacleTexture, obstacle6.obstacleRect);
		DrawImage(renderer, obstacle7.obstacleTexture, obstacle7.obstacleRect);
		DrawImage(renderer, obstacle8.obstacleTexture, obstacle8.obstacleRect);

		// Showing the screen to the player
		SDL_RenderPresent(renderer);

		// next frame...

	}
	// If we reached here then the main loop stoped
	// That means the game wants to quit

	SDL_FreeSurface(textSurface);

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
