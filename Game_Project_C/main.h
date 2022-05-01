#pragma once

uint32_t CalculateDeltaTime(uint32_t* lastTickTime, uint32_t* currentTickTime);
struct ScreenResolution;
struct Player;
struct Obstacle;
struct Vector2;
struct Vector4;
struct Stack;
void MoveTo(Player& player, Stack& path, SDL_Renderer* renderer);
void FindPath(Stack* currentCheck, unsigned char** gridArray, Stack* nextCheck, Stack& path, bool& isPathGenerated);
SDL_Window* GetWindow(SDL_Window* window);
void DrawObstacles(unsigned char** gridArray, Obstacle& RedBlock, SDL_Renderer* renderer);
void Process(SDL_Event& sdl_event, bool& isDone, Stack* Check, Player& player, Vector2& EndPosition, int Width, int Height, unsigned char** gridArray, bool& isFinding);
