#include <vector>
#include <iostream>
#include "Body.h"
#include "Opencl.h"
#include "Utility.h"

#define THETA (0.5)

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

int get_quad_index(Quad *quad, position *pos) {
	float diff_x = pos->x - quad->x;
	float diff_y = pos->y - quad->y;
	float w = quad->width / 2;
	// diffs should be >= 0
	return int(diff_x / w) + int(diff_y / w) * 2;
}

Quad get_sub_quad(Quad *quad, int index) {
	Quad sub_quad;
	float w = quad->width / 2;
	sub_quad.width = w;
	sub_quad.x = quad->x + w * (index % 2);
	sub_quad.y = quad->y + w * (index / 2);
	return sub_quad;
}

Node* new_node(body *agg, Quad q, bool leaf) {
	Node* node = new Node();
	node->aggregate = *agg;
	node->quad = q;
	node->isLeaf = leaf;
	return node;
}

/*
Compute the weighted position of two bodies
*/
position weighted_position(body b1, body b2) {
	float m = b1.mass + b2.mass;
	position new_position;
	new_position.x = (b1.mass * b1.pos.x + b2.mass * b2.pos.x) / m;
	new_position.y = (b1.mass * b1.pos.y + b2.mass * b2.pos.y) / m;
	return new_position;
}

/*
Insert the body to the tree
*/
void insert_node(Node* root, body to_insert) {
	// base case, the root is a leaf, need to split the space
	if (root->isLeaf) {
		// get the original body
		body original_body = root->aggregate;
		// update the root's state
		root->isLeaf = false;
		// update the mass and position
		root->aggregate.pos = weighted_position(original_body, to_insert);
		root->aggregate.mass += to_insert.mass;
		// ignore the velocity of the center of mass

		// create the children nodes
		int quad_index1 = get_quad_index(&(root->quad), &(original_body.pos));
		int quad_index2 = get_quad_index(&(root->quad), &(to_insert.pos));

		// if two index are the same, need to create a new node
		while (quad_index1 == quad_index2) {
			printf("Creating new internal node: both nodes has index %d\n", quad_index1);

			// create a new internal node
			Quad internal_quad = get_sub_quad(&(root->quad), quad_index1);
			printf("New internal sub quad: x[%.1f] y[%.1f] w[%.1f]\n", internal_quad.x, internal_quad.y, internal_quad.width);
			Node* node = new_node(&(root->aggregate), internal_quad, false);
			root->children[quad_index1] = node;

			// update root & index with the new internal node
			root = node;
			quad_index1 = get_quad_index(&(root->quad), &(original_body.pos));
			quad_index2 = get_quad_index(&(root->quad), &(to_insert.pos));
		}

		// create new children for the internal node
		printf("Appending new children %d %d to internal node\t", quad_index1, quad_index2);
		printf("Internal quad: x[%.1f] y[%.1f] w[%.1f]\n", root->quad.x, root->quad.y, root->quad.width);

		Node *node1 = new_node(&original_body, get_sub_quad(&(root->quad), quad_index1), true);
		Node *node2 = new_node(&to_insert, get_sub_quad(&(root->quad), quad_index2), true);
		root->children[quad_index1] = node1;
		root->children[quad_index2] = node2;
	}
	else {
		// update the mass and position
		root->aggregate.pos = weighted_position(root->aggregate, to_insert);
		root->aggregate.mass += to_insert.mass;
		// ignore the velocity of the center of mass

		// search for the leaf recursively 
		int quad_index = get_quad_index(&(root->quad), &(to_insert.pos));
		// append to the current root, update the central
		if (root->children[quad_index] == NULL) {
			printf("Appending new child %d to internal node\t", quad_index);
			printf("Internal quad: x[%.1f] y[%.1f] w[%.1f]\n", root->quad.x, root->quad.y, root->quad.width);
			// add the node to the node list
			Node* new_child = new_node(&to_insert, get_sub_quad(&(root->quad), quad_index), true);
			root->children[quad_index] = new_child;
		}
		// search in the next level
		else {
			printf("Node is not child, go to child %d\n", quad_index);
			insert_node(root->children[quad_index], to_insert);
		}
	}
}

/*
Given a list of bodies, contstruct a barnes-hut tree
Return the root of the tree
*/
Node* constrcut_tree(std::vector<body> bodies) {
	Quad quad;
	body b;
	quad.x = 0;
	quad.y = 0;
	quad.width = WIDTH;
	b.mass = 0;
	Node* root = new_node(&b, quad, false);
	for (auto const &body : bodies) {
		insert_node(root, body);
		printf("\n");
	}
	return root;
}

void compute_force(Node* root, body *b) {
	if (root == NULL) 
		return;
	
	float d = distance(root->aggregate.pos, b->pos);
	// leaf: compute pair wise force / big theta: treat the internal node as a single body
	if (root->isLeaf || root->quad.width / d >= THETA) {
		update_force(&(root->aggregate), b, 0.1);
		return;
	}
	// not-leaf and within resonable rage recursively 
	if (root->quad.width / d > THETA) {
		for (int i = 0; i < 4; i++) {
			compute_force(root->children[i], b);
		}
	}
}

void next_move(std::vector<body>bodies, float dt) {
	for (auto &b : bodies) {
		body* bp = &b;
		// update the position
		bp->speed.x += b.force.x * dt;
		bp->speed.y += b.force.y * dt;
		bp->pos.x += b.speed.x;
		bp->pos.y += b.speed.y;

		// reset the force
		bp->force.x = 0;
		bp->force.y = 0;
	}
}


// for test only
/*
int main()
{
	// create the test cases
	std::vector<body> bodies;
	float x[5] = { 20, 77, 76, 96, 55 };
	float y[5] = { 30, 5, 13, 13, 90 };
	float m[5] = { 1, 2, 3, 4, 5 };
	for (int i = 0; i < 5; i++) {
		body body;
		body.mass = m[i];
		body.pos.x = x[i];
		body.pos.y = y[i];
		bodies.push_back(body);
	}

	Node* root = constrcut_tree(bodies);

	for (auto& b : bodies) {
		compute_force(root, &b);
	}

	next_move(bodies, (float)0.1);

	return 0;
}
*/
