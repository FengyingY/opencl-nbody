#include "Config.h"
#include "Body.h"
#include "position.h"

__kernel void next_move(__global Body *bodies) {
    unsigned int index = get_global_id(0);
    __global Body* target = &bodies[index];
        
    // compute the accumulated acceleration
    for (int i = 0; i < N; i++) {
        float diff_x = bodies[i].pos.x - target->pos.x;
        float diff_y = bodies[i].pos.y - target->pos.y;
        // compute the distance
        float distance = sqrt(pow(diff_x, (float)2) + pow(diff_y, (float)2));

        // compute the force
        float F = G * bodies[i].mass / (pow(distance+3, (float)3));
        // accumulate acceleration
        float x_dir = F * diff_x;
        float y_dir = F * diff_y;
        if (!isnan(x_dir))
            target->force.x += x_dir;
        if (!isnan(y_dir))
            target->force.y += y_dir;      
    }
}
