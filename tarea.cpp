#define SDL_MAIN_HANDLED
#include "inc/SDL.h"
#include <iostream>
#include <vector>
#include <cstdlib> // Para std::rand y std::srand
#include <ctime>   // Para std::time

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
const int PADDLE_SPEED = 300;  // velocidad del paddle

// Declaración global de variables
struct Rect {
    SDL_Rect rect;
    int vx;
    int vy;
    SDL_Color color;
};

struct Block {
    SDL_Rect rect;
    bool active;
    SDL_Color color;
};

Rect ball = {{SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, BALL_SIZE, BALL_SIZE}, BALL_SPEED, BALL_SPEED, {0xFF, 0xFF, 0xFF, 0xFF}};
Rect paddle = {{(SCREEN_WIDTH - PADDLE_WIDTH) / 2, SCREEN_HEIGHT - PADDLE_HEIGHT - 10, PADDLE_WIDTH, PADDLE_HEIGHT}, 0, 0, {0xFF, 0xFF, 0xFF, 0xFF}};
std::vector<Block> blocks;

SDL_Color getRandomColor() {
    SDL_Color color;
    color.r = std::rand() % 256;
    color.g = std::rand() % 256;
    color.b = std::rand() % 256;
    color.a = 0xFF; // Opacidad completa
    return color;
}

void renderRect(SDL_Renderer* renderer, const Rect& rect) {
    SDL_SetRenderDrawColor(renderer, rect.color.r, rect.color.g, rect.color.b, rect.color.a);
    SDL_RenderFillRect(renderer, &rect.rect);
}

void renderBlock(SDL_Renderer* renderer, const Block& block) {
    if (block.active) {
        SDL_SetRenderDrawColor(renderer, block.color.r, block.color.g, block.color.b, block.color.a);
        SDL_RenderFillRect(renderer, &block.rect);
    }
}

bool checkCollision(const SDL_Rect& a, const SDL_Rect& b) {
    return (
        a.x < b.x + b.w &&
        a.x + a.w > b.x &&
        a.y < b.y + b.h &&
        a.y + a.h > b.y
    );
}

void handleInput(SDL_Event& e) {
    const Uint8* ks = SDL_GetKeyboardState(NULL);

    paddle.vx = 0;

    if (ks[SDL_SCANCODE_LEFT]) {
        paddle.vx = -PADDLE_SPEED;
    }
    if (ks[SDL_SCANCODE_RIGHT]) {
        paddle.vx = PADDLE_SPEED;
    }
}

void update(float dT) {
    // Update paddle position
    paddle.rect.x += paddle.vx * dT;

    if (paddle.rect.x < 0) {
        paddle.rect.x = 0;
    }
    if (paddle.rect.x + paddle.rect.w > SCREEN_WIDTH) {
        paddle.rect.x = SCREEN_WIDTH - paddle.rect.w;
    }

    // Update ball position
    ball.rect.x += ball.vx * dT;
    ball.rect.y += ball.vy * dT;

    if (ball.rect.x < 0 || ball.rect.x + ball.rect.w > SCREEN_WIDTH) {
        ball.vx *= -1;
    }
    if (ball.rect.y < 0) {
        ball.vy *= -1;
    }
    if (ball.rect.y + ball.rect.h > SCREEN_HEIGHT) {
        std::cout << "Game Over" << std::endl;
        SDL_Quit();
        exit(0);
    }

    if (checkCollision(ball.rect, paddle.rect)) {
        ball.vy *= -1;
        ball.rect.y = paddle.rect.y - ball.rect.h; // Adjust ball position
    }

    for (auto& block : blocks) {
        if (block.active && checkCollision(ball.rect, block.rect)) {
            ball.vy *= -1;
            block.active = false;
        }
    }

    // Check if all blocks are destroyed
    bool allDestroyed = true;
    for (const auto& block : blocks) {
        if (block.active) {
            allDestroyed = false;
            break;
        }
    }

    if (allDestroyed) {
        std::cout << "You Win!" << std::endl;
        SDL_Delay(2000); // Esperar 2 segundos para mostrar el mensaje antes de cerrar
        SDL_Quit();
        exit(0);
    }
}

void initializeBlocks() {
    blocks.clear();
    std::srand(static_cast<unsigned>(std::time(0))); // Inicializar la semilla para generación aleatoria
    for (int i = 0; i < BLOCK_ROWS; ++i) {
        for (int j = 0; j < BLOCK_COLUMNS; ++j) {
            Block block = {
                {j * (BLOCK_WIDTH + 10) + 35, i * (BLOCK_HEIGHT + 10) + 30, BLOCK_WIDTH, BLOCK_HEIGHT},
                true,
                getRandomColor() // Asignar un color aleatorio a cada bloque
            };
            blocks.push_back(block);
        }
    }
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Game Loops: Breakout", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    initializeBlocks();

    bool quit = false;
    SDL_Event e;

    Uint32 frameStartTimestamp;
    Uint32 frameEndTimestamp;
    Uint32 lastFrameTime = SDL_GetTicks();
    Uint32 lastUpdateTime = 0;
    float frameDuration = (1.0 / MAX_FPS) * 1000.0;
    float actualFrameDuration;
    int FPS = MAX_FPS;

    while (!quit) {
        frameStartTimestamp = SDL_GetTicks();

        // delta time
        Uint32 currentFrameTime = SDL_GetTicks();
        float dT = (currentFrameTime - lastFrameTime) / 1000.0;
        lastFrameTime = currentFrameTime;

        // poll events
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
            handleInput(e);
        }

        // update
        update(dT);

        // render
        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(renderer);

        renderRect(renderer, ball);
        renderRect(renderer, paddle);
        for (const auto& block : blocks) {
            renderBlock(renderer, block);
        }

        SDL_RenderPresent(renderer);

        frameEndTimestamp = SDL_GetTicks();
        actualFrameDuration = frameEndTimestamp - frameStartTimestamp;

        if (actualFrameDuration < frameDuration) {
            SDL_Delay(frameDuration - actualFrameDuration);
        }

        // fps calculation
        Uint32 currentTime = SDL_GetTicks();
        Uint32 elapsedTime = currentTime - lastUpdateTime;
        if (elapsedTime > 1000) {
            FPS = (float)lastFrameTime / (elapsedTime / 1000.0);
            lastUpdateTime = currentTime;
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
