#ifndef OCLBHGS_KERNEL_OCL_H
#define OCLBHGS_KERNEL_OCL_H

#include <CL/cl.hpp>
#include "Config.h"

typedef struct opencl {
	cl_int err;
	cl_platform_id platform;
	cl_device_id device;
	cl_context context;
	cl_program program[KERNEL_COUNT];
	cl_command_queue queue;
	cl_event event;
	cl_kernel kernels[KERNEL_COUNT];
} opencl;

opencl* initialize_opencl();
void opencl_next_move(cl_mem* bodies, opencl* cl);
void opencl_update_position(cl_mem* bodies, opencl* cl);
void opencl_cleanup(opencl* cl);
void opencl_compute_force(cl_mem* bodies, cl_mem* nodes, opencl* cl);

#endif