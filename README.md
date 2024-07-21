# Game-Loops-Breakout


Características de primera entrega: 

El jugador debe de manejar un “paddle” que se puede mover solo de izquierda a derecha en la pantalla.

Se debe controlar con el teclado.

Utilicen movimiento time-based (delta time).

Debe haber un rectángulo “pelota” que continuamente esté moviéndose en la pantalla.

Cuando la pelota toque la “paddle” debe invertir su movimiento en X y aumentar su velocidad. 

Cuando la pelota toque la pared de arriba o de los lados debe invertir su movimiento. 

Si toca la pared de abajo, el juego debe cerrarse y se debe mostrar un mensaje en la terminal que diga “Game Over”.

En la pantalla, coloquen algunos rectángulos para representar los “bloques”. Si la pelota toca un bloque, eliminen el bloque.

Si se eliminan todos los bloques, cierren el juego y muestren en la terminal un mensaje que diga algo como “You Win!”


Link del video en ejecución: 

https://www.youtube.com/watch?v=yeeu2qO0hVA


Comandos:

Compilar

g++ tarea.cpp -o tarea -I"C:/Users/Mariana/Downloads/SDL2-devel-2.30.5-mingw (1)/SDL2-2.30.5/i686-w64-mingw32/includei686-w64-mingw32/include" -L"C:/Users/Mariana/Downloads/SDL2-devel-2.30.5-mingw (1)/SDL2-2.30.5/i686-w64-mingw32/lib" -lSDL2 -lmingw32

Ejecutar

.\tarea.exe