#pragma once
#include "Body.h"
#include "position.h"
#include "Opencl.h"

float distance(Position a, Position b);
void initialize_bodies_two_galaxies(Body* bodies);
void initialize_bodies(Body* bodies);
void update_force(float mass, Position pos, Body* target, float distance);
int max_node(int level);
bool is_in_bound(Body b);