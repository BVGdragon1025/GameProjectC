#pragma once

//Structs
struct Vector2;
struct Character;

//Functions
void GrassfireAlgorithm();
uint32_t CalculateDeltaTime(uint32_t* lastTick, uint32_t* currentTick);
SDL_Texture* SetTexture(SDL_Surface* surface, SDL_Renderer* renderer, const char* imagePath);
void SetRect(SDL_Rect* rect, Vector2 position);
void DrawImage(SDL_Renderer* renderer, SDL_Texture* texture, SDL_Rect rect);
Vector2 GetRandomGridPosition();
void PlayTurn(Character* player, Character* ai, bool* isPlayerMoving, bool* isPlayerFinishedMoving, bool* isAiMoving, int* turn, int nextTurn, Vector2 mousePosition, Character* enemyTarget, bool* playerMarkedEnemy, Character* playerTarget, int randomInt);
Vector2 MouseToGridPosition(Vector2 mousePos);
char* CastToArray(int number);
Vector2 SetEnemyDestination(Vector2 location);
TTF_Font* GetCurrentFont();
int GetRandomEnemy(Character* characters[]);
