// g++ -Wall -Wextra main.cpp -lSDL2
#include <cstdio>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

int input_function (const char *text, void *data, char **output);

void quit(){
    Console_Destroy(tty);
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

int main (int argc, char **argv)
{
    SDL_Window   *window = NULL;
    SDL_GLContext glContext = NULL;
    Console_tty  *tty = NULL;
    SDL_Event     event;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <monospaced font path> <font size>\n", 
                argv[0]);
        exit(1);
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL Failed to init: %s\n", SDL_GetError());
        exit(1);
    }

    /* Create a window that is resizeable. SDL_Console can handle resizing. */
    window = SDL_CreateWindow("Console", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        640, 480, 
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

    if (!window) {
        fprintf(stderr, "Window could not be created: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        printf("%s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    tty = Console_Create(
            window,         /* SDL_Window */
            argv[1],        /* font path */
            atoi(argv[2]),  /* font size */
            SDLK_ESCAPE,    /* key that toggles console on/off */
            input_function, /* function that handles text input to console */
            NULL);          /* userdata to the above function */

    if (!tty) {
        fprintf(stderr, "Console could not init: %s\n", Console_GetError());
        SDL_GL_DeleteContext(glContext);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(1);
    }

    while(1) { 
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                goto quit;
                break;
            }
        }

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (Console_Draw(tty)) { /* handle drawing the console if toggled */
            fprintf(stderr, "%s\n", Console_GetError());
            /* handle fatal console error */
            quit();
        }

        SDL_GL_SwapWindow(window);
    }


}