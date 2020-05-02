#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdbool.h>

#include <CL/cl.h>
#include <SDL.h>

#include "Opencl.h"
#include "Body.h"
#include "position.h"
#include "Utility.h"


int main(int argc, char *argv[]) {
    // Alloacte memory for bodies
    body* bodies = new body[N];

    // Initialize the N bodies
    initialize_bodies(bodies);

    // initialize opencl
    opencl* cl = initialize_opencl();

    // read the bodies to the open cl buffer
    cl_mem bodies_obj = clCreateBuffer(cl->context, CL_MEM_READ_WRITE, sizeof(body) * N, NULL, &cl->err);
    if (cl->err) {
        fprintf(stderr, "clCreateBuffer error\n");
        exit(cl->err);
    }
    cl->err = clEnqueueWriteBuffer(cl->queue, bodies_obj, CL_TRUE, 0, sizeof(body) * N, bodies, 0, NULL, NULL);
    if (cl->err) {
        fprintf(stderr, "clCreateBuffer error\n");
        exit(cl->err);
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL initialize error!" << std::endl;
    }
    SDL_Window* window;
    SDL_Renderer* renderer;
    int err = SDL_CreateWindowAndRenderer(WIDTH, HEIGTH, 0, &window, &renderer);
    if (err) {
        std::cout << "SDL_CreateWindowAndRenderer error!" << std::endl;
        return err;
    }

    bool quit = false;
    SDL_Event event;
    while(!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        // execute kernels
        opencl_next_move(&bodies_obj, cl);
        opencl_update_position(&bodies_obj, cl);

        // read from the buffer and print it out
        cl->err = clEnqueueReadBuffer(cl->queue, bodies_obj, CL_TRUE, 0, sizeof(body) * N, bodies, 0, NULL, NULL);

        // SDL draw
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
            // std::cout << i << " " << x << " " << y << " " << mass << std::endl;

            if (x >= 0 && x < HEIGTH && y >= 0 && y < WIDTH) {
                // Now we can draw our point
                int err = SDL_RenderDrawPoint(renderer, x, y);//Renders on middle of screen.

                if (err < 0) {
                    std::cout << "Rendering error!" << std::endl;
                    return err;
                }
            }
        }

        // And now we present everything we draw after the clear.
        SDL_RenderPresent(renderer);
    }

    opencl_cleanup(cl);
    cl->err = clReleaseMemObject(bodies_obj);
    return 0;
}