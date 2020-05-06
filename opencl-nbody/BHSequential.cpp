#include <iostream>
#include <omp.h>
#include "Body.h"
#include "Utility.h"
#include "Config.h"
#include "Simulator.h"
#include "instrument.h"

typedef struct NodeSeq {
	float central_mass;			// aggregated mass
	Position central_pos;		// weighted position
	Quad quad;					// the square region this node represent 
	bool isLeaf;				// is leaf or not
	int level;					// the level of the tree
	NodeSeq* children[4];		// 4 children. 0 = upper left, 1 = upper right, 2 = lower left, 3 = loewr right
	std::vector<Body> bodies;	// bodies in this cell, only set for the leaves
}NodeSeq;


int get_quad_index(Quad quad, Position pos) {
	float diff_x = pos.x - quad.x;
	float diff_y = pos.y - quad.y;
	float w = quad.width / 2;
	// diffs should be >= 0
	return int(diff_x / w) + int(diff_y / w) * 2;
}

Quad get_sub_quad(Quad quad, int index) {
	Quad sub_quad;
	float w = quad.width / 2;
	sub_quad.width = w;
	sub_quad.x = quad.x + w * (index % 2);
	sub_quad.y = quad.y + w * (index / 2);
	return sub_quad;
}

NodeSeq* new_node(float cm, Position cp, Quad q, bool leaf, int level) {
	NodeSeq* node = new NodeSeq();
	node->central_mass = cm;
	node->central_pos = cp;
	node->quad = q;
	node->isLeaf = leaf;
	node->level = level;
	return node;
}

/*
Compute the weighted position of two bodies
*/
Position weighted_position(Position p1, float m1, Position p2, float m2) {
	float m = m1 + m2;
	Position new_position;
	new_position.x = (m1 * p1.x + m2 * p2.x) / m;
	new_position.y = (m1 * p1.y + m2 * p2.y) / m;
	return new_position;
}

/*
Insert the body to the tree
*/
void insert_node(NodeSeq* root, Body to_insert) {
	// update the mass and position
	root->central_pos = weighted_position(root->central_pos, root->central_mass, to_insert.pos, to_insert.mass);
	root->central_mass += to_insert.mass;

	// base case, the root is a leaf
	if (root->isLeaf) {
		// the root has been the max level, simply add the body to the list
		if (root->level == MAXLEVEL) {
			root->bodies.push_back(to_insert);
		}
		// if the node has not been the MAXLEVEL, need to split the space
		else {
			if (root->bodies.size() != 1) {
				fprintf(stderr, "Leaf at depth %d has %d bodies!\n", root->level, root->bodies.size());
				exit(-1);
			}
			Body original_body = root->bodies[0];
			// update the root's state
			root->isLeaf = false;
			// clear the bodies vector for the inner node
			root->bodies.clear();

			// info for the new aggregated node
			float cm = original_body.mass + to_insert.mass;
			Position cp = weighted_position(original_body.pos, original_body.mass, to_insert.pos, to_insert.mass);

			// create the children nodes
			int quad_index1 = get_quad_index(root->quad, original_body.pos);
			int quad_index2 = get_quad_index(root->quad, to_insert.pos);

			// if two index are the same, need to create a new node
			while (quad_index1 == quad_index2 && root->level < MAXLEVEL - 1) {
				// create a new internal node
				Quad internal_quad = get_sub_quad(root->quad, quad_index1);
				NodeSeq* node = new_node(cm, cp, internal_quad, false, root->level + 1);
				root->children[quad_index1] = node;

				// update root & index with the new internal node
				root = node;
				quad_index1 = get_quad_index(root->quad, original_body.pos);
				quad_index2 = get_quad_index(root->quad, to_insert.pos);
			}

			if (quad_index1 != quad_index2) {
				// create new children for the internal node
				NodeSeq* node1 = new_node(original_body.mass, original_body.pos, get_sub_quad(root->quad, quad_index1), true, root->level + 1);
				NodeSeq* node2 = new_node(to_insert.mass, to_insert.pos, get_sub_quad(root->quad, quad_index2), true, root->level + 1);
				node1->bodies.push_back(original_body);
				node2->bodies.push_back(to_insert);
				root->children[quad_index1] = node1;
				root->children[quad_index2] = node2;
			}
			else {
				// create a new node for both of the children
				NodeSeq* node = new_node(cm, cp, get_sub_quad(root->quad, quad_index1), true, root->level + 1);
				node->bodies.push_back(original_body);
				node->bodies.push_back(to_insert);
			}
		}
	}
	else {
		// search for the leaf recursively 
		int quad_index = get_quad_index(root->quad, to_insert.pos);
		// append to the current root, update the central
		if (root->children[quad_index] == NULL) {
			// add the node to the node list
			NodeSeq* new_child = new_node(to_insert.mass, to_insert.pos, get_sub_quad(root->quad, quad_index), true, root->level + 1);
			new_child->bodies.push_back(to_insert);
			root->children[quad_index] = new_child;
		}
		// search in the next level
		else {
			// printf("NodeSeq is not child, go to child %d\n", quad_index);
			insert_node(root->children[quad_index], to_insert);
		}
	}
}

/*
Given a list of bodies, contstruct a barnes-hut tree
Return the root of the tree
*/
NodeSeq* constrcut_tree(Body* bodies) {
	Quad quad;
	quad.x = 0;
	quad.y = 0;
	quad.width = WIDTH;
	Position pos;
	pos.x = 0;
	pos.y = 0;
	NodeSeq* root = new_node(0, pos, quad, false, 0);
	for (int i = 0; i < N; i++) {
		if (is_in_bound(bodies[i])) {
			insert_node(root, bodies[i]);
		}
	}
	return root;
}

void compute_force(NodeSeq* root, Body *b) {
	if (root == NULL) 
		return;

	float d = distance(root->central_pos, b->pos);

	// leaf: compute pair wise force / big theta: treat the internal node as a single body
	if (root->isLeaf || root->quad.width / d < THETA || !is_in_bound(*b)) {
		float F = G * (root->central_mass / (pow(d + 3, (float)3)));
		float x_dir = F * (root->central_pos.x - b->pos.x);
		float y_dir = F * (root->central_pos.y - b->pos.y);
		if (!isnan(x_dir))
			b->force.x += x_dir;
		if (!isnan(y_dir))
			b->force.y += y_dir;
		return;
	}

	// not-leaf and within resonable rage recursively 
	for (int i = 0; i < 4; i++) {
		compute_force(root->children[i], b);
	}
	
}
 
void update_position(Body* bodies) {
	#pragma omp parallel for schedule(static)
	for (int i = 0; i < N; i++) {
		Body* bp = &bodies[i];
		// update the position
		bp->speed.x += bp->force.x * DT;
		bp->speed.y += bp->force.y * DT;
		bp->pos.x += bp->speed.x;
		bp->pos.y += bp->speed.y;

		// reset the force
		bp->force.x = 0;
		bp->force.y = 0;

	}
}


BHSequential::BHSequential(Body* init_bodies) {
	bodies = init_bodies;
}

Body* BHSequential::next_move() {
	// coonstruct tree
	START_ACTIVITY(ACTIVITY_CONSTRUCT_TREE);
	NodeSeq* root = constrcut_tree(bodies);
	FINISH_ACTIVITY(ACTIVITY_CONSTRUCT_TREE);

	// compute force
	START_ACTIVITY(ACTIVITY_FORCE);
	#pragma omp parallel for schedule(static)
	for (int i = 0; i < N; i++) {
		compute_force(root, &bodies[i]);
	}
	FINISH_ACTIVITY(ACTIVITY_FORCE);

	// next move
	START_ACTIVITY(ACTIVITY_POSITION);
	update_position(bodies);
	FINISH_ACTIVITY(ACTIVITY_POSITION);
	return bodies;
}

BHSequential::~BHSequential() {
	free(bodies);
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

	NodeSeq* root = constrcut_tree(bodies);

	for (auto& b : bodies) {
		compute_force(root, &b);
	}

	next_move(bodies, (float)0.1);

	return 0;
}
*/
