#define SDL_MAIN_HANDLED
#include "inc/SDL.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL_image.h>

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
        for (auto& star : stars) {
            star.y += STAR_SPEED * dT;  // Desplazar las estrellas hacia abajo

            if (star.y > SCREEN_HEIGHT) {
                star.y = 0;  // Si la estrella sale de la pantalla, la muevo nuevamente arriba
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

// Clase para gestionar al personaje (Sprite)
class Character {
public:
    SDL_Texture* texture;
    SDL_Rect srcRect, destRect;
    int speed;
    int x, y;

    // Variables para la animación
    int frameWidth;          // Ancho de un frame en pigi.png
    int frameHeight;         // Alto de un frame en pigi.png
    int totalFrames;         // Número total de frames en la animación
    int currentFrame;        // Frame actual de la animación
    float animationSpeed;    // Velocidad de cambio de frames
    float frameTime;         // Tiempo acumulado desde el último cambio de frame

    Character(SDL_Renderer* renderer)
        : frameWidth(612), frameHeight(1224), totalFrames(3), currentFrame(0), animationSpeed(0.2f), frameTime(0) {

        texture = IMG_LoadTexture(renderer, "pigi.png");
        if (!texture) {
            std::cerr << "Error al cargar pigi.png: " << IMG_GetError() << std::endl;
            exit(1);
        } else {
            std::cout << "pigi.png cargado correctamente" << std::endl;
        }

        // Configura el rectángulo fuente (srcRect) para el primer frame
        srcRect = { 0, 0, frameWidth, frameHeight }; // Primer frame en la fila 1, columna 1
        
        // Configura el rectángulo destino (destRect) para la posición inicial del personaje
        destRect = { 100, 100, frameWidth / 2, frameHeight / 2 };
        
        speed = 5;
        x = 100;
        y = 100;
    }

    void handleInput() {
        const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);

        if (currentKeyStates[SDL_SCANCODE_UP]) {
            y -= speed;
        }
        if (currentKeyStates[SDL_SCANCODE_DOWN]) {
            y += speed;
        }
        if (currentKeyStates[SDL_SCANCODE_LEFT]) {
            x -= speed;
        }
        if (currentKeyStates[SDL_SCANCODE_RIGHT]) {
            x += speed;
        }

        destRect.x = x;
        destRect.y = y;
    }

    void update(float dT) {
        frameTime += dT;
        if (frameTime >= animationSpeed) {
            currentFrame = (currentFrame + 1) % totalFrames;
            switch (currentFrame) {
                case 0:
                    srcRect.x = 0;  // Primer fotograma
                    srcRect.y = 0;
                    break;
                case 1:
                    srcRect.x = frameWidth;  // Segundo fotograma
                    srcRect.y = 0;
                    break;
                case 2:
                    srcRect.x = 0;  // Tercer fotograma
                    srcRect.y = frameHeight;  // 'frameHeight' posición 
                    break;
            }
            frameTime = 0;
        }
    }

    void render(SDL_Renderer* renderer) {
        SDL_RenderCopy(renderer, texture, &srcRect, &destRect);
    }

    ~Character() {
        SDL_DestroyTexture(texture);
    }
};

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);  // Inicializar SDL_image para cargar imágenes PNG

    SDL_Window* window = SDL_CreateWindow("Entre Estrellas", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Background background;
    Character character(renderer);
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

        // Actualización de la lógica
        background.update(dT);
        character.handleInput();
        character.update(dT);  // Actualiza el personaje para animarlo

        // Renderizado
        SDL_SetRenderDrawColor(renderer, 0, 0, 20, 255);  // Fondo negro azulado
        SDL_RenderClear(renderer);

        background.render(renderer);  // Renderiza el fondo primero
        character.render(renderer);   // Luego renderiza el personaje encima del fondo

        SDL_RenderPresent(renderer);
        SDL_Delay(1000 / MAX_FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    IMG_Quit();  // Cerrar SDL_image

    return 0;
}
