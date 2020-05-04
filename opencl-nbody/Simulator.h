#pragma once
#include "Body.h"
#include "Opencl.h"
#include "BH.h"

__interface Simulator {
	Body* next_move();
};

class NaiveCL : public Simulator {
public:
	NaiveCL(Body* init_bodies);
	Body* next_move();
	~NaiveCL();

private:
	Body* bodies;
	opencl* cl;
	cl_mem bodies_obj;
};

class BHSequential : public Simulator {
public:
	BHSequential(Body* init_bodies);
	Body* next_move();
	~BHSequential();

private:
	Body* bodies;
};

class BHParallel : public Simulator {
public:
	BHParallel(Body* init_bodies);
	Body* next_move();
	~BHParallel();

private:
	Body* bodies;
	Node* nodes;
	opencl* cl;

	cl_mem bodies_obj;
	cl_mem nodes_obj;

	// helper arrays
	std::vector<std::vector<int> > nodes_at_level;
	int** nodes_at_level_arr;
	int offset_at_level[MAXLEVEL + 1];
	
	void init_quads_dfs(Quad quad, int index, int offset, int level);
	void init_quads_bfs(); 
	void reset_node(int node_index);
	void construct_tree();
	void compute_force();
	void update_position();
	int get_quad_index(Quad quad, Position pos);
	void add_body(Node* node, Body* new_body);
	void setupChildren(int index, int level);
};