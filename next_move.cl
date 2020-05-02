#include "Config.h"
#include "Body.h"
#include "position.h"

__kernel void next_move(__global body *bodies) {
    unsigned int index = get_global_id(0);
    __global body* target = &bodies[index];


    /*
    if (index == 8)
        printf("body[%d] x=%.1f, y=%.1f mass=%.1f\n", index, target.pos.x, target.pos.y, target.mass);
    */
        
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

        /*
        if (i == 0) {
            printf("i=%d distance=%.4f F=%.4f mass=%.4f accumulate_x=%.4f accumulate_y=%.4f\n",
                i, distance, F, bodies[i].mass, target->force.x, target->force.y);
        }
        */        
    }
    /*
     printf("body[%d] x=%.1f, y=%.1f mass=%.1f force.x=%.1f force.y=%.1f \n", 
        index, bodies[index].pos.x, bodies[index].pos.y, bodies[index].mass,
        bodies[index].force.x, bodies[index].force.y);
    */
   

}
