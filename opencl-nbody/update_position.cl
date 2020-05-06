#include "Config.h"
#include "Body.h"
#include "position.h"

__kernel void update_position(__global Body* bodies) {
	unsigned int index = get_global_id(0);
    __global Body* target = &bodies[index];

    // update the position
    target->speed.x += target->force.x * DT;
    target->speed.y += target->force.y * DT;
    target->pos.x += bodies[index].speed.x;
    target->pos.y += bodies[index].speed.y;

    // reset force
    target->force.x = 0;
    target->force.y = 0;
}
