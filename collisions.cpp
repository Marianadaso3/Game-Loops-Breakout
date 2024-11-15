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
const int STAR_COUNT = 100;
const int STAR_SPEED = 50;
const int CLOUD_COUNT = 5;

struct Star {
    float x, y;
    int size;
    SDL_Color color;
};

class Background {
public:
    std::vector<Star> stars;

    Background() {
        for (int i = 0; i < STAR_COUNT; ++i) {
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
            star.y += STAR_SPEED * dT;
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
    SDL_Rect collider;  // Para el collider
    float speed;
    int colliderWidth;  // Nuevo: Ajustar ancho del collider
    int colliderHeight; // Nuevo: Ajustar alto del collider
    float pivotX, pivotY;  // Pivote en X y Y

    Cloud(SDL_Renderer* renderer, const std::string& texturePath, int x, int y, float speed, float pivotX = 1.0f, float pivotY = 0.5f)
    : speed(speed), colliderWidth(240), colliderHeight(80), pivotX(pivotX), pivotY(pivotY) {
    
    texture = IMG_LoadTexture(renderer, texturePath.c_str());
    if (!texture) {
        std::cerr << "Error al cargar la textura " << texturePath << ": " << IMG_GetError() << std::endl;
        exit(1);
    }

    // Tamaño de la imagen
    destRect = { x, y, 300, 100 };

    // Ajustar la posición de la textura con base en el pivote (esto mueve la imagen)
    destRect.x -= (pivotX - 1.0f) * destRect.w;

    // Ajustar la posición del collider con base en el pivote
    collider.x = destRect.x + (pivotX - 1.0f) * colliderWidth;

    // Si deseas mover el collider aún más a la derecha (por ejemplo, 10 posiciones),
    // puedes agregar un desplazamiento extra como este:
    collider.x += 10;  // Mueve el collider 10 unidades a la derecha

    // Mantener la posición vertical del collider
    collider.y = destRect.y;

    // Establecer el tamaño del collider
    collider.w = colliderWidth;
    collider.h = colliderHeight;
}




    void update(float dT) {
        destRect.x += static_cast<int>(speed * dT);
        if (destRect.x > SCREEN_WIDTH) {
            destRect.x = -destRect.w;
            destRect.y = rand() % (SCREEN_HEIGHT - destRect.h);
        }
    }

    void render(SDL_Renderer* renderer) const {
        SDL_Rect pivotedRect = { static_cast<int>(destRect.x - destRect.w * pivotX), 
                                 static_cast<int>(destRect.y - destRect.h * pivotY), 
                                 destRect.w, destRect.h };
        SDL_RenderCopy(renderer, texture, nullptr, &pivotedRect);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Dibujar collider
        SDL_Rect collider = { static_cast<int>(pivotedRect.x + pivotedRect.w / 2 - colliderWidth/ 2.3f), 
                              static_cast<int>(pivotedRect.y + pivotedRect.h / 2 - colliderHeight / 1.8), 
                              colliderWidth, colliderHeight };
        SDL_RenderDrawRect(renderer, &collider);
    }

    SDL_Rect getCollider() const {
        return { destRect.x + destRect.w / 2 - colliderWidth / 2, 
                 destRect.y + destRect.h / 2 - colliderHeight / 2, 
                 colliderWidth, colliderHeight };
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
    int frameWidth;
    int frameHeight;
    int totalFrames;
    int currentFrame;
    float animationSpeed;
    float frameTime;
    int colliderWidth;  // Nuevo: Ajustar ancho del collider
    int colliderHeight; // Nuevo: Ajustar alto del collider
    float pivotX, pivotY;  // Pivote en X y Y

    Character(SDL_Renderer* renderer, float pivotX = 0.5f, float pivotY = 0.5f)
        : frameWidth(302), frameHeight(245), totalFrames(2), currentFrame(0),
          animationSpeed(0.2f), frameTime(0), colliderWidth(120), colliderHeight(80),
          pivotX(pivotX), pivotY(pivotY) {

        texture = IMG_LoadTexture(renderer, "pigi2.png");
        if (!texture) {
            std::cerr << "Error al cargar pigi2.png: " << IMG_GetError() << std::endl;
            exit(1);
        }

        // Ajustar el srcRect con la nueva imagen
        srcRect = { 0, 0, frameWidth, frameHeight };  // Inicializa el primer frame
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

            // Aquí definimos los frames. Frame 0 está a la izquierda, y frame 1 está a la derecha
            if (currentFrame == 0) {
                srcRect.x = 0;  // Primer frame (izquierda)
            } else if (currentFrame == 1) {
                srcRect.x = frameWidth;  // Segundo frame (derecha)
            }
            
            srcRect.y = 0;  // Ya que solo tenemos un único tipo de animación en esta imagen (solo un rango vertical)
            frameTime = 0;
        }
    }

    void render(SDL_Renderer* renderer) {
        SDL_Rect pivotedRect = { static_cast<int>(x - destRect.w * pivotX), 
                                 static_cast<int>(y - destRect.h * pivotY), 
                                 destRect.w, destRect.h };
        SDL_RenderCopy(renderer, texture, &srcRect, &pivotedRect);

        // Dibujar collider
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); 
        SDL_Rect collider = { static_cast<int>(pivotedRect.x + pivotedRect.w / 2 - colliderWidth / 2), 
                              static_cast<int>(pivotedRect.y + pivotedRect.h / 2 - colliderHeight/2), 
                              colliderWidth, colliderHeight };
        SDL_RenderDrawRect(renderer, &collider);
    }

    SDL_Rect getCollider() {
        return { x + destRect.w / 2 - colliderWidth / 2, 
                 y + destRect.h / 2 - colliderHeight / 2, 
                 colliderWidth, colliderHeight };
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
    for (int i = 0; i < CLOUD_COUNT; ++i) {
        int x = -(rand() % SCREEN_WIDTH);
        int y = rand() % (SCREEN_HEIGHT - 100);
        float speed = 100 + rand() % 100;
        clouds.emplace_back(renderer, "nube.png", x, y, speed);
    }

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

        character.handleInput();
        character.update(dT);
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

        character.render(renderer);

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
