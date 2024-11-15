#define SDL_MAIN_HANDLED
#include "inc/SDL.h"
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h> 


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
    Uint32 lastCollisionTime; // Variable para controlar el tiempo de la última colisión
    const Uint32 collisionCooldown = 500; // Tiempo de "enfriamiento" en milisegundos (500 ms = 0.5 segundos)
   // bool checkCollisionWithCooldown(const SDL_Rect& targetRect);

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

    bool checkCollisionWithCooldown(const SDL_Rect& targetRect) {
    SDL_Rect collider = getCollider(); // Guardamos el resultado en una variable temporal
    return SDL_HasIntersection(&collider, &targetRect); // Pasamos la dirección de la variable temporal
}



    ~Character() {
        SDL_DestroyTexture(texture);
    }
};

bool checkCollision(const SDL_Rect& characterRect, const SDL_Rect& targetRect) {
    return SDL_HasIntersection(&characterRect, &targetRect);
}

class TileMap {
public:
    SDL_Texture* tileTexture;
    std::vector<std::vector<int>> map;
    int mapWidth, mapHeight;
    int tileWidth, tileHeight;

    TileMap(SDL_Renderer* renderer, const std::string& texturePath, int mapWidth, int mapHeight, int tileWidth = 60, int tileHeight = 60)
        : mapWidth(mapWidth), mapHeight(mapHeight), tileWidth(tileWidth), tileHeight(tileHeight) {
        // Cargar la textura de los tiles
        tileTexture = IMG_LoadTexture(renderer, texturePath.c_str());
        if (!tileTexture) {
            std::cerr << "Error al cargar la textura de tiles: " << IMG_GetError() << std::endl;
            exit(1);
        }

        // Crear un mapa simpl
        map = std::vector<std::vector<int>>(mapHeight, std::vector<int>(mapWidth, 0));
        // Personalizacion del mapa 
        map[10][10] = 10; // Asignar un tile específico
       
    }

    void render(SDL_Renderer* renderer) {
    for (int y = 0; y < mapHeight; ++y) {
        for (int x = 0; x < mapWidth; ++x) {
            int tileIndex = map[y][x]; // Usamos un valor en el mapa para determinar qué tile mostrar

            // Determinar la fuente del tile dentro de la textura
            // Calculamos las coordenadas dentro de la textura, asumiendo 10 tiles por fila.
            int tileX = (tileIndex % 10) * tileWidth;  // Asumimos 10 tiles por fila en la textura
            int tileY = (tileIndex / 10) * tileHeight; // Asumimos 10 tiles por columna en la textura

            // Ajustar el tamaño de cada tile
            SDL_Rect srcRect = { tileX, tileY, tileWidth, tileHeight }; // Rectángulo de la textura
            SDL_Rect destRect = { x * tileWidth, y * tileHeight, tileWidth, tileHeight }; // Posición en el mapa

            // Renderizar el tile
            SDL_RenderCopy(renderer, tileTexture, &srcRect, &destRect);
        }
    }
}


    ~TileMap() {
        SDL_DestroyTexture(tileTexture);
    }
};


int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // Inicializar SDL con soporte de audio
    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);
    TTF_Init(); // Inicializar SDL_ttf

    // Inicializar SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "Error al inicializar SDL_mixer: " << Mix_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Entre Estrellas", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Cargar fuente
    //TTF_Font* font = TTF_OpenFont("Arial.ttf", 24);
    TTF_Font* font = TTF_OpenFont("/usr/share/fonts/truetype/msttcorefonts/Arial.ttf", 24);

    if (!font) {
        std::cerr << "Error al cargar la fuente: " << TTF_GetError() << std::endl;
        return 1;
    }

    // Cargar música de fondo
    Mix_Music* backgroundMusic = Mix_LoadMUS("background_music.mp3");
    if (!backgroundMusic) {
        std::cerr << "Error al cargar la música de fondo: " << Mix_GetError() << std::endl;
        return 1;
    }

    // Cargar efecto de sonido
    Mix_Chunk* collisionSound = Mix_LoadWAV("collision_sound.wav");
    if (!collisionSound) {
        std::cerr << "Error al cargar el efecto de sonido: " << Mix_GetError() << std::endl;
        return 1;
    }

    // Reproducir música de fondo en bucle
    Mix_PlayMusic(backgroundMusic, -1);

    Background background;
    Character character(renderer);

    TileMap tileMap(renderer, "tile3.jpg", 14, 14); // 14 tiles en X y 14 en Y

    std::vector<Cloud> clouds;
    srand(static_cast<unsigned int>(time(0)));

    Uint32 startTime = SDL_GetTicks(); // Tiempo de inicio del juego

    bool quit = false;
    SDL_Event e;
    int collisionCount = 0; // Contador de colisiones
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

         // Comprobar colisiones
        for (auto& cloud : clouds) {
            if (checkCollision(character.getCollider(), cloud.getCollider())) {
                collisionCount++; // Incrementar el contador de colisiones
                Mix_PlayChannel(-1, Mix_LoadWAV("collision_sound.wav"), 0); // Sonido de colisión
                std::cout << "Colisión #" << collisionCount << std::endl;
                if (collisionCount >= 10) {
                    std::cout << "Game Over" << std::endl;
                    quit = true;
                    break;
                }
            }
        }

        // Actualizar objetos
        character.update(0.016f); // Actualización del personaje
        for (auto& cloud : clouds) {
            cloud.update(dT); // Actualización de las nubes
        }

        // for (auto& cloud : clouds) {
        //     cloud.update(dT);
        // }

        SDL_SetRenderDrawColor(renderer, 0, 0, 68, 68);
        SDL_RenderClear(renderer);

       
        tileMap.render(renderer);

        background.render(renderer);
        
        

        for (auto& cloud : clouds) {
            cloud.render(renderer);
        }

        character.update(dT);
        character.render(renderer);

         // Calcular el tiempo transcurrido
        Uint32 elapsedTime = (SDL_GetTicks() - startTime) / 1000; // En segundos
        std::string timerText = "Tiempo: " + std::to_string(elapsedTime) + "s";
        std::cout << "Tiempo transcurrido: " << elapsedTime << " segundos." << std::endl;

        // Renderizar el texto del temporizador
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, timerText.c_str(), white);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_Rect textRect = {10, 10, textSurface->w, textSurface->h};
        SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);


        //Colisión con la nube
        for (auto& cloud : clouds) {
            if (checkCollision(character.getCollider(), cloud.getCollider())) {
                std::cout << "Colisión detectada con una nube!" << std::endl;
                Mix_PlayChannel(-1, collisionSound, 0); // Reproducir efecto de sonido al colisionar
            }
        }

        SDL_RenderPresent(renderer);

        SDL_Delay(1000 / MAX_FPS);
    }
    
    
    // Liberar recursos
    Mix_FreeMusic(backgroundMusic);
    Mix_FreeChunk(collisionSound);
    Mix_CloseAudio();

    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();
    IMG_Quit();

    return 0;
}
