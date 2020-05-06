#include "Body.h"

typedef struct Quad {
	float x;		// upper left corner position
	float y;
	float width;
}Quad;

typedef struct Node {
	float central_mass;			// aggregated mass
	Position central_pos;		// weighted position
	Quad quad;					// the square region this node represent 
	int level;					// the level of the tree
	int body_start_idx;			// the start index at the bodies array
	int bodies_count;			// the number of bodies at this node or below this node
} Node;

/*
typedef struct NodeSeq {
	float central_mass;			// aggregated mass
	Position central_pos;		// weighted position
	Quad quad;					// the square region this node represent 
	bool isLeaf;				// is leaf or not
	int level;					// the level of the tree
	NodeSeq* children[4];		// 4 children. 0 = upper left, 1 = upper right, 2 = lower left, 3 = loewr right
	std::vector<Body> bodies;	// bodies in this cell, only set for the leaves
}NodeSeq;

__interface BHTree {
	void construct_tree(Body* bodies);
	void compute_force();
	void update_position();
};
*/

