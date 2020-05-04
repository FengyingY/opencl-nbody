#include "Config.h"
#include "Body.h"
#include "position.h"

__kernel void update_position(__global Body* bodies) {
	unsigned int index = get_global_id(0);
    __global Body* target = &bodies[index];

    /*
    printf("body[%d] x=%.1f y=%.1f force_x=%.1f force_y=%.1f speed_x=%.1f speed_y=%.1f\n",
        index, target->pos.x, target->pos.y, target->force.x, target->force.y, target->speed.x, target->speed.y);
    */
    
    // update the position
    target->speed.x += target->force.x * DT;
    target->speed.y += target->force.y * DT;
    target->pos.x += bodies[index].speed.x;
    target->pos.y += bodies[index].speed.y;

    // reset force
    target->force.x = 0;
    target->force.y = 0;

    // printf("body[%d] x=%.1f, y=%.1f mass=%.1f\n", index, bodies[index].pos.x, bodies[index].pos.y, bodies[index].mass);
}
