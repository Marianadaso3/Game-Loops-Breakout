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
const int STAR_COUNT = 100;  // Número de estrellas en el fondo
const int STAR_SPEED = 50;   // Velocidad de desplazamiento de las estrellas

// Estructura de las estrellas
struct Star {
    float x, y;
    int size;
    SDL_Color color;
};

// Clase para gestionar el fondo animado
class Background {
public:
    std::vector<Star> stars;

    Background() {
        // Inicializa las estrellas con posiciones y colores aleatorios
        for (int i = 0; i < STAR_COUNT; ++i) {
            stars.push_back(createRandomStar());
        }
    }

    Star createRandomStar() {
        Star star;
        star.x = static_cast<float>(rand() % SCREEN_WIDTH);
        star.y = static_cast<float>(rand() % SCREEN_HEIGHT);
        star.size = rand() % 3 + 1; // Tamaños de estrellas entre 1 y 3
        star.color = { static_cast<Uint8>(200 + rand() % 56), 
                       static_cast<Uint8>(200 + rand() % 56), 
                       255, 
                       255 }; // Colores azulados
        return star;
    }

    void update(float dT) {
        // Actualiza la posición de cada estrella
        for (auto& star : stars) {
            star.y += STAR_SPEED * dT;  // Mueve hacia abajo
            if (star.y > SCREEN_HEIGHT) {
                star = createRandomStar();  // Reinicia la estrella en la parte superior
                star.y = 0; // Restablece su posición
            }
        }
    }

    void render(SDL_Renderer* renderer) {
        for (const auto& star : stars) {
            SDL_SetRenderDrawColor(renderer, star.color.r, star.color.g, star.color.b, star.color.a);
            SDL_Rect rect = { static_cast<int>(star.x), static_cast<int>(star.y), star.size, star.size };
            SDL_RenderFillRect(renderer, &rect);
        }
    }
};

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow("Entre Estrellas", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Background background;
    bool quit = false;
    SDL_Event e;
    Uint32 lastFrameTime = SDL_GetTicks();

    while (!quit) {
        Uint32 currentFrameTime = SDL_GetTicks();
        float dT = (currentFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentFrameTime;

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        background.update(dT);

        SDL_SetRenderDrawColor(renderer, 0, 0, 20, 255);  // Fondo negro azulado
        SDL_RenderClear(renderer);

        background.render(renderer);

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / MAX_FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
