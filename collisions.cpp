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
const int CLOUD_COUNT = 1; // Solo una nube en pantalla

struct Star {
    float x, y;
    int size;
    SDL_Color color;
};

class Background {
public:
    std::vector<Star> stars;

    Background() {
        for (int i = 0; i < 100; ++i) {
            stars.push_back(createRandomStar());
        }
    }

    Star createRandomStar() {
        Star star;
        star.x = static_cast<float>(rand() % SCREEN_WIDTH);
        star.y = static_cast<float>(rand() % SCREEN_HEIGHT);
        star.size = rand() % 3 + 1;
        star.color = { static_cast<Uint8>(200 + rand() % 56), 
                       static_cast<Uint8>(200 + rand() % 56), 
                       255, 
                       255 };
        return star;
    }

    void update(float dT) {
        for (auto& star : stars) {
            star.y += 50 * dT;
            if (star.y > SCREEN_HEIGHT) {
                star.y = 0;
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

class Cloud {
public:
    SDL_Texture* texture;
    SDL_Rect destRect;
    SDL_Rect collider;
    float speed;

    Cloud(SDL_Renderer* renderer, const std::string& texturePath, int x, int y, float speed)
        : speed(speed) {

        texture = IMG_LoadTexture(renderer, texturePath.c_str());
        if (!texture) {
            std::cerr << "Error al cargar la textura " << texturePath << ": " << IMG_GetError() << std::endl;
            exit(1);
        }

        // Obtener las dimensiones de la textura cargada
        int textureWidth, textureHeight;
        SDL_QueryTexture(texture, NULL, NULL, &textureWidth, &textureHeight);

        // Establecer las dimensiones del rectángulo de destino de la nube
        destRect = { x, y, textureWidth - 120, textureHeight -8 };

        // Calculamos el centro de la textura
        int centerX = destRect.x + destRect.w / 2;
        int centerY = destRect.y + destRect.h / 2;

        // Reducir el tamaño del collider
        int newWidth = textureWidth -120 ;
        int newHeight = textureHeight - 28;

        // Establecer el collider centrado respecto al pivote de la textura
        collider = {
            centerX - newWidth / 2, // Centrado en X
            centerY - newHeight / 2, // Centrado en Y
            newWidth, 
            newHeight
        };
        
    }

    void update(float dT) {
        // Actualizar la posición de la nube
        destRect.x += static_cast<int>(speed * dT);
        if (destRect.x > SCREEN_WIDTH) {
            destRect.x = -destRect.w; // Volver a la izquierda
            destRect.y = rand() % (SCREEN_HEIGHT - destRect.h); // Nueva posición aleatoria en Y
        }

        // Actualizar la posición del collider con la de la nube
        collider.x = destRect.x;
        collider.y = destRect.y;
    }

    void render(SDL_Renderer* renderer) const {
        SDL_RenderCopy(renderer, texture, nullptr, &destRect);

        //Dibujar collider para debug
        //SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); 
        //SDL_RenderDrawRect(renderer, &collider);
    }

    SDL_Rect getCollider() const {
        return collider;
    }

    ~Cloud() {
        SDL_DestroyTexture(texture);
    }
};



class Character {
public:
    SDL_Texture* texture;
    SDL_Rect srcRect, destRect;
    int speed;
    int x, y;
    int currentFrame;
    int frameWidth, frameHeight;
    Uint32 lastFrameTime;

    Character(SDL_Renderer* renderer)
        : speed(5), x(100), y(100), currentFrame(0), lastFrameTime(0) {

        texture = IMG_LoadTexture(renderer, "pigi2.png");
        if (!texture) {
            std::cerr << "Error al cargar pigi2.png: " << IMG_GetError() << std::endl;
            exit(1);
        }

        frameWidth = 302;  // Ancho de un fotograma
        frameHeight = 245; // Altura de un fotograma
        srcRect = { 0, 0, frameWidth, frameHeight };
        destRect = { x, y, 150, 122 };
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
    // Controlar el cambio de fotograma para la animación
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastFrameTime > 200) { // Cambio de fotograma cada 200ms
        currentFrame++;
        if (currentFrame >= 4) { // 4 fotogramas en la animación
            currentFrame = 0;
        }
        srcRect.x = currentFrame * frameWidth; // Desplazamiento del fotograma
        lastFrameTime = currentTime;
    }
}


    void render(SDL_Renderer* renderer) {
        SDL_RenderCopy(renderer, texture, &srcRect, &destRect);
    }

    SDL_Rect getCollider() {
        return { x + destRect.w / 2 - 60, y + destRect.h / 2 - 40, 120, 80 };
    }

    ~Character() {
        SDL_DestroyTexture(texture);
    }
};

bool checkCollision(const SDL_Rect& characterRect, const SDL_Rect& targetRect) {
    return SDL_HasIntersection(&characterRect, &targetRect);
}

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow("Entre Estrellas", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Background background;
    Character character(renderer);

    std::vector<Cloud> clouds;
    srand(static_cast<unsigned int>(time(0)));

    bool quit = false;
    SDL_Event e;
    Uint32 lastFrameTime = SDL_GetTicks();
    Uint32 lastCloudSpawnTime = SDL_GetTicks(); // Tiempo para generar una nueva nube

    while (!quit) {
        Uint32 currentFrameTime = SDL_GetTicks();
        float dT = (currentFrameTime - lastFrameTime) / 1000.0f;
        lastFrameTime = currentFrameTime;

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Generar nueva nube cada 5 segundos
        if (currentFrameTime - lastCloudSpawnTime >= 5000) {
            int x = -200; // Las nubes salen desde el principio de la ventana en X
            int y = rand() % (SCREEN_HEIGHT - 100);
            float speed = 100 + rand() % 100;
            clouds.clear(); // Limpiar las nubes anteriores
            clouds.emplace_back(renderer, "nube.png", x, y, speed); // Agregar nueva nube
            lastCloudSpawnTime = currentFrameTime;
        }

        character.handleInput();
        background.update(dT);

        for (auto& cloud : clouds) {
            cloud.update(dT);
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 68, 68);
        SDL_RenderClear(renderer);

        background.render(renderer);

        for (auto& cloud : clouds) {
            cloud.render(renderer);
        }

        character.update(dT);
        character.render(renderer);

        // Revisar colisión con la nube
        for (auto& cloud : clouds) {
            if (checkCollision(character.getCollider(), cloud.getCollider())) {
                std::cout << "Colisión detectada con una nube!" << std::endl;
            }
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(1000 / MAX_FPS);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    IMG_Quit();

    return 0;
}
