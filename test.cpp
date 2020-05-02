#include <iostream>
#include "position.h"
#include "Body.h"

#define UPPER_RIGHT (0)
#define UPPER_LEFT	(1)
#define LOWER_RIGHT (2)
#define LOWER_LEFT	(3)

typedef struct Quad {
	float x;
	float y;
	float width;
} Quad;

typedef struct Node {
	body aggregate;
	Quad quad;		// the square region this node represent 
	bool isLeaf;	// is leaf or not
	Node* children[4]; // 4 children. 0 = upper left, 1 = upper right, 2 = lower left, 3 = loewr right
} Node;

int get_quad_index_1(Quad quad, position pos) {
	float diff_x = pos.x - quad.x;
	float diff_y = pos.y - quad.y;
	float w = quad.width / 2;
	// diffs should be >= 0
	return int(diff_x / w) + int(diff_y / w) * 2;
}

int main()
{
    float a = 4;
    float b = 3;

    std::cout << int(b / a) << std::endl;

    Quad quad;
    position pos;

    quad.width = 8;
    quad.x = 0;
    quad.y = 0;
    pos.x = 5;
    pos.y = 3;

    std::cout << get_quad_index_1(quad, pos) << std::endl;


	Node *node = new Node();
	std::cout << (node->children[0] == NULL) << std::endl;


    return 0;
}