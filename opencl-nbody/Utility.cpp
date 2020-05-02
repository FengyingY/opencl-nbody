#include "Utility.h"
#include <time.h>

float distance(position a, position b)
{
	return (float)sqrt(pow((a.x - b.x), (float)2.0) + pow(a.y - b.y, (float)2.0));
}

// update the force of target, d is distance
void update_force(body* b, body* target, float d) {
    float F = G * b->mass / (pow(d + 3, (float)3));
    float x_dir = F * (b->pos.x - target->pos.x);
    float y_dir = F * (b->pos.y - target->pos.y);
    if (!isnan(x_dir))
        target->force.x += x_dir;
    if (!isnan(y_dir))
        target->force.y += y_dir;
    return;
}


// initialize the position and speed of the bodies
void initialize_bodies_two_galaxies(body* bodies) {
    srand(15618);
    for (int i = 0; i < N / 2; i++) {
        float x = rand() % WIDTH / 4 + WIDTH / 8 + WIDTH / 2;
        float y = rand() % HEIGTH / 4 + HEIGTH / 8;
        float dist = sqrt(pow(x - (WIDTH / 4 + WIDTH / 2), (float)2) + pow(y - HEIGTH / 4, (float)2));
        if (dist > WIDTH / 8) {
            --i;
            continue;
        }

        body tmp;
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

        body tmp;
        tmp.pos.x = x;
        tmp.pos.y = y;
        tmp.speed.x = 0.2;
        tmp.speed.y = -0.1;
        tmp.mass = 0;
        bodies[i] = tmp;
    }
    bodies[N - 1].mass = 100000000;
}

void initialize_bodies(body * bodies) {
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
