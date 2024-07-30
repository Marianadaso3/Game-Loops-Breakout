#define SDL_MAIN_HANDLED
#include "inc/SDL.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <unordered_map>


const int SCREEN_WIDTH = 750;
const int SCREEN_HEIGHT = 480;
const int MAX_FPS = 60;
const int BALL_SPEED = 200;
const int BALL_SIZE = 20;
const int PADDLE_WIDTH = 100;
const int PADDLE_HEIGHT = 20;
const int BLOCK_WIDTH = 60;
const int BLOCK_HEIGHT = 20;
const int BLOCK_ROWS = 5;
const int BLOCK_COLUMNS = 10;
const int PADDLE_SPEED = 300;

// Estructuro los componentes
struct Position {
    float x, y;
};

struct Velocity {
    float vx, vy;
};

struct Color {
    SDL_Color color;
};

struct Paddle {};
struct Ball {};
struct Block { bool active; };

// Clase para entidades
class Entity {
public:
    int id;
    Entity(int id) : id(id) {}
};

// Clase ECS para gestionar componentes
class ECS {
public:
    std::unordered_map<int, Position> positions;
    std::unordered_map<int, Velocity> velocities;
    std::unordered_map<int, Color> colors;
    std::unordered_map<int, Paddle> paddles;
    std::unordered_map<int, Ball> balls;
    std::unordered_map<int, Block> blocks;

    int createEntity() {
        static int id = 0;
        return id++;
    }
};

SDL_Color getRandomColor() {
    return { static_cast<Uint8>(rand() % 256), static_cast<Uint8>(rand() % 256), static_cast<Uint8>(rand() % 256), 0xFF };
}

// Inicializo bloques con ECS
void initializeBlocks(ECS &ecs) {
    for (int i = 0; i < BLOCK_ROWS; ++i) {
        for (int j = 0; j < BLOCK_COLUMNS; ++j) {
            int block = ecs.createEntity();
            ecs.positions[block] = { j * (BLOCK_WIDTH + 10) + 35.0f, i * (BLOCK_HEIGHT + 10) + 30.0f };
            ecs.colors[block] = { getRandomColor() };
            ecs.blocks[block] = { true };
        }
    }
}

// Inicializo entidades ECS
void initializeEntities(ECS &ecs) {
    int paddle = ecs.createEntity();
    ecs.positions[paddle] = { (SCREEN_WIDTH - PADDLE_WIDTH) / 2.0f, SCREEN_HEIGHT - PADDLE_HEIGHT - 10.0f };
    ecs.velocities[paddle] = { 0.0f, 0.0f };
    ecs.colors[paddle] = { {0xFF, 0xFF, 0xFF, 0xFF} };
    ecs.paddles[paddle] = {};

    int ball = ecs.createEntity();
    ecs.positions[ball] = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    ecs.velocities[ball] = { BALL_SPEED, BALL_SPEED };
    ecs.colors[ball] = { {0xFF, 0xFF, 0xFF, 0xFF} };
    ecs.balls[ball] = {};

    initializeBlocks(ecs);
}

// Manejo de la entrada
void handleInput(ECS &ecs, SDL_Event& e) {
    const Uint8* ks = SDL_GetKeyboardState(NULL);

    for (auto& paddle : ecs.paddles) {
        ecs.velocities[paddle.first].vx = 0.0f;

        if (ks[SDL_SCANCODE_LEFT]) {
            ecs.velocities[paddle.first].vx = -PADDLE_SPEED;
        }
        if (ks[SDL_SCANCODE_RIGHT]) {
            ecs.velocities[paddle.first].vx = PADDLE_SPEED;
        }
    }
}

// Verifico colisiones
bool checkCollision(Position& aPos, Position& bPos, int bWidth, int bHeight) {
    return aPos.x < bPos.x + bWidth && aPos.x + BALL_SIZE > bPos.x && aPos.y < bPos.y + bHeight && aPos.y + BALL_SIZE > bPos.y;
}

// Actualizo el estado del juego
void update(ECS &ecs, float dT) {
    for (auto& paddle : ecs.paddles) {
        auto& pos = ecs.positions[paddle.first];
        auto& vel = ecs.velocities[paddle.first];

        pos.x += vel.vx * dT;
        if (pos.x < 0) pos.x = 0;
        if (pos.x + PADDLE_WIDTH > SCREEN_WIDTH) pos.x = SCREEN_WIDTH - PADDLE_WIDTH;
    }

    for (auto& ball : ecs.balls) {
        auto& pos = ecs.positions[ball.first];
        auto& vel = ecs.velocities[ball.first];

        pos.x += vel.vx * dT;
        pos.y += vel.vy * dT;

        if (pos.x < 0 || pos.x + BALL_SIZE > SCREEN_WIDTH) {
            vel.vx *= -1;
        }
        if (pos.y < 0) {
            vel.vy *= -1;
        }
        if (pos.y + BALL_SIZE > SCREEN_HEIGHT) {
            std::cout << "Game Over" << std::endl;
            SDL_Quit();
            exit(0);
        }

        for (auto& paddle : ecs.paddles) {
            if (checkCollision(pos, ecs.positions[paddle.first], PADDLE_WIDTH, PADDLE_HEIGHT)) {
                vel.vy *= -1;
                pos.y = ecs.positions[paddle.first].y - BALL_SIZE;
            }
        }

        for (auto& block : ecs.blocks) {
            if (block.second.active && checkCollision(pos, ecs.positions[block.first], BLOCK_WIDTH, BLOCK_HEIGHT)) {
                vel.vy *= -1;
                block.second.active = false;
            }
        }

        bool allDestroyed = true;
        for (const auto& block : ecs.blocks) {
            if (block.second.active) {
                allDestroyed = false;
                break;
            }
        }

        if (allDestroyed) {
            std::cout << "You Win!" << std::endl;
            SDL_Delay(2000);
            SDL_Quit();
            exit(0);
        }
    }
}

// Renderizo el juego con ECS
void render(ECS &ecs, SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    for (const auto& paddle : ecs.paddles) {
        auto& pos = ecs.positions[paddle.first];
        auto& color = ecs.colors[paddle.first];
        SDL_SetRenderDrawColor(renderer, color.color.r, color.color.g, color.color.b, color.color.a);
        SDL_Rect rect = { static_cast<int>(pos.x), static_cast<int>(pos.y), PADDLE_WIDTH, PADDLE_HEIGHT };
        SDL_RenderFillRect(renderer, &rect);
    }

    for (const auto& ball : ecs.balls) {
        auto& pos = ecs.positions[ball.first];
        auto& color = ecs.colors[ball.first];
        SDL_SetRenderDrawColor(renderer, color.color.r, color.color.g, color.color.b, color.color.a);
        SDL_Rect rect = { static_cast<int>(pos.x), static_cast<int>(pos.y), BALL_SIZE, BALL_SIZE };
        SDL_RenderFillRect(renderer, &rect);
    }

    for (const auto& block : ecs.blocks) {
        if (block.second.active) {
            auto& pos = ecs.positions[block.first];
            auto& color = ecs.colors[block.first];
            SDL_SetRenderDrawColor(renderer, color.color.r, color.color.g, color.color.b, color.color.a);
            SDL_Rect rect = { static_cast<int>(pos.x), static_cast<int>(pos.y), BLOCK_WIDTH, BLOCK_HEIGHT };
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);
}

// Función principal
int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Game Loops: Breakout", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    ECS ecs; //Usando ECS para inicializar
    initializeEntities(ecs);

    bool quit = false;
    SDL_Event e;

    Uint32 frameStartTimestamp;
    Uint32 frameEndTimestamp;
    Uint32 lastFrameTime = SDL_GetTicks();
    Uint32 lastUpdateTime = 0;
    float frameDuration = (1.0f / MAX_FPS) * 1000.0f;
    float actualFrameDuration;
    int FPS = MAX_FPS;

    while (!quit) {
        frameStartTimestamp = SDL_GetTicks();

        Uint32 currentFrameTime = SDL_GetTicks();
        float dT = (currentFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentFrameTime;

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            handleInput(ecs, e);
        }

        update(ecs, dT);
        render(ecs, renderer);

        frameEndTimestamp = SDL_GetTicks();
        actualFrameDuration = frameEndTimestamp - frameStartTimestamp;

        if (actualFrameDuration < frameDuration) {
            SDL_Delay(static_cast<Uint32>(frameDuration - actualFrameDuration));
        }

        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsedTime = currentTime - lastUpdateTime;
        if (elapsedTime > 1000) {
            FPS = (float)lastFrameTime / (elapsedTime / 1000.0f);
            lastUpdateTime = currentTime;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
