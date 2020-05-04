#include "Body.h"
#include "Utility.h"
#include "Config.h"
#include "Simulator.h"
#include "instrument.h"

#define MAXNODE max_node(MAXLEVEL)

BHParallel::BHParallel(Body* init_bodies) {
	bodies = init_bodies;
	nodes = (Node*) calloc(MAXNODE, sizeof(Node));

	printf("Bodies size: %d\n", sizeof(Body) * N);
	printf("Nodes size: %d\n", sizeof(Node) * MAXNODE);

	// initialize the root node
	for (int i = 0; i < N; i++) {
		add_body(&nodes[0], &bodies[i]);
	}

	// initialize the helper arrays
	int offset = MAXNODE;
	for (int i = 0; i < MAXLEVEL; i++) {
		nodes_at_level.push_back(std::vector<int>());
		offset_at_level[i] = offset;
		offset = (offset - 1) / 4;
	}

	// dfs
	Quad quad;
	quad.x = 0;
	quad.y = 0;
	quad.width = WIDTH;
	init_quads_dfs(quad, 0, MAXNODE, 0);

	// initialize the array
	nodes_at_level_arr = (int**)malloc(sizeof(int*) * (MAXLEVEL));
	for (int i = 0; i < MAXLEVEL; i++) {
		int size = nodes_at_level[i].size();
		nodes_at_level_arr[i] = (int*)malloc(sizeof(int) * size);
		for (int ii = 0; ii < size; ii++) {
			nodes_at_level_arr[i][ii] = nodes_at_level[i][ii];
		}
	}

	cl = initialize_opencl();
	if (cl == NULL) {
		fprintf(stderr, "BHParallel::BHParallelL initialize_opencl error\n");
		exit(-1);
	}

	// read the bodies to the open cl buffer
	bodies_obj = clCreateBuffer(cl->context, CL_MEM_READ_WRITE, sizeof(Body) * N, NULL, &cl->err);
	if (cl->err) {
		fprintf(stderr, "BHParallel::BHParallelL clCreateBuffer error\n");
		exit(cl->err);
	}

	// Create the Nodes buffer 
	nodes_obj = clCreateBuffer(cl->context, CL_MEM_READ_WRITE, sizeof(Node) * MAXNODE, NULL, &cl->err);
	if (cl->err) {
		fprintf(stderr, "BHParallel::BHParallelL clCreateBuffer for nodes error\n");
		exit(cl->err);
	}
}

Body* BHParallel::next_move() {
	START_ACTIVITY(ACTIVITY_CONSTRUCT_TREE);
	// construct tree - CPU
	construct_tree();
	FINISH_ACTIVITY(ACTIVITY_CONSTRUCT_TREE);
	// compute next move - GPU
	compute_force();
	// update position - GPU
	update_position();

	return bodies;
}

BHParallel::~BHParallel() {
	free(nodes);
	opencl_cleanup(cl);
}


/*
Reset the node array
*/
void BHParallel::reset_node(int node_index) {
	Node* n = &nodes[node_index];
	n->central_mass = 0;
	n->central_pos.x = 0;
	n->central_pos.y = 0;
	n->body_start_idx = 0;
	n->bodies_count = 0;
}

void BHParallel::setupChildren(int index, int level) {
	//START_ACTIVITY(ACTIVITY_RESET_CHILD);
	Node* parent_node = &nodes[index];
	// reset children info
	int child_index = index + 1;
	for (int ii = 0; ii < 4; ii++) {
		reset_node(child_index);
		child_index += offset_at_level[level + 1];
	}
	//FINISH_ACTIVITY(ACTIVITY_RESET_CHILD);

	if (parent_node->bodies_count > 1) {

		//START_ACTIVITY(ACTIVITY_SPLIT_NODE);
		// loop over all the bodies at this node
		int start_index = parent_node->body_start_idx;
		int end_index = start_index + parent_node->bodies_count;
		for (int j = start_index; j < end_index; j++) {
			Body* b = &bodies[j];
			// the body is still in the zone
			if (is_in_bound(*b)) {
				// get quad index and then map to the child
				int quad_index = get_quad_index(parent_node->quad, b->pos);
				int node_index = index + 1 + quad_index * offset_at_level[level + 1];		// child level
				// update the body's node_index
				b->node_idx = node_index;
				// add the body to the child's node
				add_body(&nodes[node_index], b);
			}
			else {
				b->node_idx = MAXNODE;
			}
		}
		//FINISH_ACTIVITY(ACTIVITY_SPLIT_NODE);

		//START_ACTIVITY(ACTIVITY_SETUP_CHILD);
		// sort the nodes by its node index
		Body* start = bodies + parent_node->body_start_idx;
		qsort(start, parent_node->bodies_count, sizeof(Body), compareBody);

		// set the body start index for the children
		nodes[index + 1].body_start_idx = parent_node->body_start_idx;
		Node* prev_node = &nodes[index + 1];
		child_index = index + 1 + offset_at_level[level + 1];
		for (int ii = 1; ii < 4; ii++) {
			nodes[child_index].body_start_idx = prev_node->body_start_idx + prev_node->bodies_count;
			// go to the next child
			prev_node = &nodes[child_index];
			child_index += offset_at_level[level + 1];
		}
		//FINISH_ACTIVITY(ACTIVITY_SETUP_CHILD);
	}
}

void BHParallel::construct_tree() {
#if DEBUG
	printf("constructing tree\n");
#endif
	int node_count = 1;
	for (int level = 0; level < MAXLEVEL; level++) {					// parent level
		// loop over all nodes at this level
		//for (std::vector<int>::iterator it = nodes_at_level[level].begin(); it != nodes_at_level[level].end(); it++) {		// this 

//#pragma omp parallel for num_threads(4) schedule(static) 
		for (int i = 0; i < node_count; i++) {
			// int index = *it; 
			int index = nodes_at_level_arr[level][i];
			setupChildren(index, level);
		}
		node_count *= 4;
	}
}

void BHParallel::compute_force() {
#if (GPU)
	START_ACTIVITY(ACTIVITY_HOST_TO_DEVICE);
#if DEBUG
	printf("Compute force: copying bodies to kernel\n");
#endif
	// copy the bodies and nodes array to the device
	cl->err = clEnqueueWriteBuffer(cl->queue, bodies_obj, CL_TRUE, 0, sizeof(Body) * N, bodies, 0, NULL, NULL);
	if (cl->err) {
		fprintf(stderr, "BHParallel::compute_force clEnqueueWriteBuffer for bodies error\n");
		exit(cl->err);
	}

#if DEBUG
	printf("Compute force: copying nodes to kernel\n");
#endif
	cl->err = clEnqueueWriteBuffer(cl->queue, nodes_obj, CL_TRUE, 0, sizeof(Node) * MAXNODE, nodes, 0, NULL, NULL);
	if (cl->err) {
		fprintf(stderr, "BHParallel::compute_force clEnqueueWriteBuffer for nodes error\n");
		exit(cl->err);
	}
	FINISH_ACTIVITY(ACTIVITY_HOST_TO_DEVICE);
	
	// launch the kernel 
	START_ACTIVITY(ACTIVITY_FORCE);
#if DEBUG
	printf("Compute force: executed kernel\n");
#endif
	opencl_compute_force(&bodies_obj, &nodes_obj, cl);
	FINISH_ACTIVITY(ACTIVITY_FORCE);

#else
	for (int index = 0; index < N; index++) {
		Body* b = &bodies[index];
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
				else
				{
					continue;
				}

				// skip all the children under this root
				i += offset_at_level[root.level];
			}
			else {
				// go to next level (if it's not leaf) or go the neighbor leaves / upper level root (if it's leaf)
				i++;
			}
		}
	}
#endif
}

void BHParallel::update_position() {
#if (GPU)
	// launch the kernel
	START_ACTIVITY(ACTIVITY_POSITION);
#if DEBUG
	printf("Update position: executed kernel\n");
#endif // DEBUG

	opencl_update_position(&bodies_obj, cl);
	FINISH_ACTIVITY(ACTIVITY_POSITION);
	// write the bodies back to the host

	START_ACTIVITY(ACTIVITY_DEVICE_TO_HOST);
#if DEBUG
	printf("Update position: copyinboides back \n");
#endif
	cl->err = clEnqueueReadBuffer(cl->queue, bodies_obj, CL_TRUE, 0, sizeof(Body) * N, bodies, 0, NULL, NULL);
	if (cl->err) {
		fprintf(stderr, "NaiveCL::next_move clEnqueueReadBuffer error\n");
		exit(cl->err);
	}
	FINISH_ACTIVITY(ACTIVITY_DEVICE_TO_HOST);
#else
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
#endif
}


void BHParallel::init_quads_dfs(Quad quad, int index, int offset, int level) {
	// base case
	if (level > MAXLEVEL)
		return;

	// set the quad and level
	nodes[index].quad = quad;
	nodes[index].level = level;
	if (level < MAXLEVEL)
		nodes_at_level[level].push_back(index);
	
	// printf("n[%d]\tx=%.4f\ty=%.4f\tw=%.4f\tlevel=%d\toffset=%d\n", index, quad.x, quad.y, quad.width, level, offset);
	
	// update the offset
	offset = (offset - 1) / 4;

	// create quad for children
	Quad child_quad;
	int child_index = index + 1;
	child_quad.width = quad.width / 2;
	for (int r = 0; r < 2; r++) {
		child_quad.y = quad.y + r * child_quad.width;
		for (int c = 0; c < 2; c++) {
			child_quad.x = quad.x + c * child_quad.width;
			init_quads_dfs(child_quad, child_index, offset, level + 1);
			child_index += offset;
		}
	}
}


void BHParallel::init_quads_bfs() {
	// initialize quads for the nodes - bfs
	// SCHEMA 1 hard to parallelize tree construction
	//        / -------- | ------ - |
	// 	|0| |1 | 2 | 3 | 4| |5 | 6 | 7 | 8 | 9 | 10 | 11 | 12 | 13 | ...
	// 	\__ | _______ |

	int arr_ptr = 0;	// pointer of the nodes array
	float w = WIDTH;	// the width of quad at this level
	Quad quad;
	for (int i = 0; i <= MAXLEVEL; i++) {
		int n = WIDTH / w;
		quad.width = w;
		for (int r = 0; r < n; r++) {
			quad.y = r * w;
			for (int c = 0; c < n; c++) {
				quad.x = c * w;
				nodes[arr_ptr++].quad = quad;
			}
		}
		// update w for next level
		w /= 2;
	}
}

int BHParallel::get_quad_index(Quad quad, Position pos) {
	float diff_x = pos.x - quad.x;
	float diff_y = pos.y - quad.y;
	float w = quad.width / 2;
	// diffs should be >= 0
	return int(diff_x / w) + int(diff_y / w) * 2;
}


/*
Add the new body to a node, update the node's info
*/
void BHParallel::add_body(Node* node, Body* new_body) {
	node->central_mass += new_body->mass;
	if (node->central_mass > 0) {
		node->central_pos.x = (node->central_mass * node->central_pos.x + new_body->mass * new_body->pos.x) / node->central_mass;
		node->central_pos.y = (node->central_mass * node->central_pos.y + new_body->mass * new_body->pos.y) / node->central_mass;
	}
	node->bodies_count += 1;
}