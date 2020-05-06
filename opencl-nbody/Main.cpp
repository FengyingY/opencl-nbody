#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <stdbool.h>

#include <CL/cl.h>
#include <SDL.h>
#include <omp.h>

#include "Opencl.h"
#include "Body.h"
#include "position.h"
#include "Utility.h"
#include "Simulator.h"
#include "SDLDraw.h"
#include "instrument.h"


int main(int argc, char *argv[]) {
    track_activity(true);

    omp_set_dynamic(0);
    //omp_set_num_threads(2);
    
    START_ACTIVITY(ACTIVITY_STARTUP);
    // Alloacte memory for bodies
    Body* bodies = (Body*)calloc(N, sizeof(Body));
    // Initialize the N bodies
    initialize_bodies(bodies);

    // Initialize the simulator and the renderer
    Simulator* sim = new NaiveCL(bodies);
    SDL_Renderer* render = SDLinit();
    FINISH_ACTIVITY(ACTIVITY_STARTUP);

    bool quit = false;
    SDL_Event event;

    int count = 0;

    while(count < 1000 && !quit) {
        
        START_ACTIVITY(ACTIVITY_RENDER);
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }
        FINISH_ACTIVITY(ACTIVITY_RENDER);
        
        // execute kernels
        bodies = sim->next_move();

        // SDL draw
        START_ACTIVITY(ACTIVITY_RENDER);
        SDLDraw(render, bodies);
        FINISH_ACTIVITY(ACTIVITY_RENDER);

        count++;
    }

    show_activity(stderr, true);

    return 0;
}