#pragma once
#include "position.h"

typedef struct body {
	position pos;
	position speed;
	position force;
	float mass;
} body;