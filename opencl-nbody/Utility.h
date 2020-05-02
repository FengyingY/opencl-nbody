#pragma once
#include <stdlib.h>
#include <math.h>

#include "Body.h"
#include "position.h"
#include "Opencl.h"

float distance(position a, position b);
void initialize_bodies_two_galaxies(body* bodies);
void initialize_bodies(body* bodies);
void update_force(body *b1, body *b2, float d);