#include <SDL.h>
#include <stdio.h>

#include "Config.h"
#include "Body.h"

SDL_Renderer* SDLinit() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL initialize error!\n");
    }
    SDL_Window* window;
    SDL_Renderer* renderer;
    int err = SDL_CreateWindowAndRenderer(WIDTH, HEIGTH, 0, &window, &renderer);
    if (err) {
        fprintf(stderr, "SDL_CreateWindowAndRenderer error!\n");
        exit(err);
    }
    return renderer;
}

void SDLDraw(SDL_Renderer* renderer, Body* bodies) {
    // Set our color for the draw functions
    SDL_SetRenderDrawColor(renderer, 0x0, 0x0, 0x0, 0xFF);
    // We clear what we draw before
    SDL_RenderClear(renderer);
    // the first one
    float x = bodies[0].pos.x;
    float y = bodies[0].pos.y;
    // Set our color for the draw functions
    SDL_SetRenderDrawColor(renderer, 0xFF, 0, 0, 0xFF);
    // Now we can draw our point
    SDL_RenderDrawPoint(renderer, x, y);

    // Set our color for the draw functions
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    for (int i = 1; i < N; i++) {
        float x = bodies[i].pos.x;
        float y = bodies[i].pos.y;
        float mass = bodies[i].mass;

        if (x >= 0 && x < HEIGTH && y >= 0 && y < WIDTH) {
            // Now we can draw our point
            int err = SDL_RenderDrawPoint(renderer, x, y);  //Renders on middle of screen.

            if (err < 0) {
                fprintf(stderr, "Rendering error!\n");
                exit(err);
            }
        }
    }

    // And now we present everything we draw after the clear.
    SDL_RenderPresent(renderer);
}