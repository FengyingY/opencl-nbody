#include "Utility.h"
#include "Config.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

float distance(Position a, Position b)
{
	return (float)sqrt(pow((a.x - b.x), (float)2.0) + pow(a.y - b.y, (float)2.0));
}

// update the force of target, d is distance
void update_force(float mass, Position pos, Body* target, float distance) {
    float F = G * mass / (pow(distance + 3, (float)3));
    float x_dir = F * (pos.x - target->pos.x);
    float y_dir = F * (pos.y - target->pos.y);
    if (!isnan(x_dir))
        target->force.x += x_dir;
    if (!isnan(y_dir))
        target->force.y += y_dir;
    return;
}


// initialize the position and speed of the bodies
void initialize_bodies_two_galaxies(Body* bodies) {
    srand(15618);
    for (int i = 0; i < N / 2; i++) {
        float x = rand() % WIDTH / 4 + WIDTH / 8 + WIDTH / 2;
        float y = rand() % HEIGTH / 4 + HEIGTH / 8;
        float dist = sqrt(pow(x - (WIDTH / 4 + WIDTH / 2), (float)2) + pow(y - HEIGTH / 4, (float)2));
        if (dist > WIDTH / 8) {
            --i;
            continue;
        }

        Body tmp;
        tmp.pos.x = x;
        tmp.pos.y = y;
        tmp.speed.x = 0.1;
        tmp.speed.y = 0.2;
        tmp.mass = 3;
        bodies[i] = tmp;
    }


    bodies[N / 2 - 1].mass = 100000000;

    for (int i = N / 2; i < N; i++) {
        float x = rand() % WIDTH / 4 + WIDTH / 8 + WIDTH / 2;
        float y = rand() % HEIGTH / 4 + HEIGTH / 8 + HEIGTH / 2;
        float dist = sqrt(pow((float)(x - (WIDTH / 4 + WIDTH / 2)), (float)2) +
            pow((float)(y - (HEIGTH / 4 + WIDTH / 2)), (float)2));
        if (dist > WIDTH / 8) {
            --i;
            continue;
        }

        Body tmp;
        tmp.pos.x = x;
        tmp.pos.y = y;
        tmp.speed.x = 0.2;
        tmp.speed.y = -0.1;
        tmp.mass = 0;
        bodies[i] = tmp;
    }
    bodies[N - 1].mass = 100000000;
}

void initialize_bodies(Body * bodies) {
    srand(15618);

    bodies[0].pos.x = WIDTH / 2;
    bodies[0].pos.y = HEIGTH / 2;
    bodies[0].speed.x = 0;
    bodies[0].speed.y = 0;
    bodies[0].force.x = 0;
    bodies[0].force.y = 0;
    bodies[0].mass = 150000000;

    for (int i = 1; i < N; i++) {
        float x = rand() % WIDTH / 4 + WIDTH / 8;
        float y = rand() % HEIGTH / 4 + HEIGTH / 8;
        float dist = sqrt(pow((float)(x - WIDTH / 4), (float)2) + pow((float)(y - HEIGTH / 4), (float)2));
        if (dist > WIDTH / 8) {
            --i;
            continue;
        }

        bodies[i].pos.x = x;
        bodies[i].pos.y = y;
        bodies[i].speed.x = 2;
        bodies[i].speed.y = -1;
        bodies[i].force.x = 0;
        bodies[i].force.y = 0;
        bodies[i].mass = 0;
    }

}

int max_node(int level) {
    if (level == 0)
        return 1;
    else
        return max_node(level - 1) * 4 + 1;
}

int compareBody(const void* a, const void* b) {
    Body b1 = *(Body*)a;
    Body b2 = *(Body*)b;
    return b1.node_idx - b2.node_idx;
}

bool is_in_bound(Body b) {
    return (b.pos.x > 0 && b.pos.x < HEIGTH&& b.pos.y > 0 && b.pos.y < WIDTH);
}