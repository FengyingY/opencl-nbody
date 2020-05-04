#include "Config.h"
#include "Body.h"
#include "position.h"
#include "BH.h"

__kernel void compute_force_kernel(__global Body* bodies, __global Node* nodes) {
	unsigned int index = get_global_id(0);
	__global Body* b = &bodies[index];

	// offset array
	int MAXNODE = 0;
	int offset_at_level[MAXLEVEL + 1];
	int offset = 1;
	for (int i = MAXLEVEL; i >= 0; i--) {
		MAXNODE += offset;
		offset_at_level[i] = offset;
		offset = offset * 4 + 1;
	}

	// dfs the tree
	for (int i = 0; i < MAXNODE;) {
		Node root = nodes[i];
		float diff_x = root.central_pos.x - b->pos.x;
		float diff_y = root.central_pos.y - b->pos.y;

		float d = sqrt(pow(diff_x, (float)2) + pow(diff_y, (float)2));
		if (root.bodies_count <= 1 || root.level == MAXLEVEL ||		// is leaf or NULL
			root.quad.width / d < THETA) {		// too far away, treat this cell as a single body

			float F = G * root.central_mass / (pow(d + 3, (float)3));
			float x_dir = F * diff_x;
			float y_dir = F * diff_y;

			if (!isnan(x_dir))
				b->force.x += x_dir;
			if (!isnan(y_dir))
				b->force.y += y_dir;

			// skip all the children under this root
			i += offset_at_level[root.level];
		}
		else {
			// go to next level (if it's not leaf) or go the neighbor leaves / upper level root (if it's leaf)
			i++;
		}
	}
	/*
	if (index == 0)
		printf("%.4f %.4f\n", b->force.x, b->force.y);
	*/
	
	
	
}