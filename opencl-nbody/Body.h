#pragma once
#include "position.h"

typedef struct Body {
	Position pos;
	Position speed;
	Position force;
	float mass;
	int node_idx;				// the index of the node
} Body;

int compareBody(const void* a, const void* b);