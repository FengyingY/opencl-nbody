#include "Body.h"
#include "Opencl.h"
#include "Simulator.h"
#include "instrument.h"

NaiveCL::NaiveCL(Body* init_bodies) {
	// initialize opencl
	cl = initialize_opencl();
	bodies = init_bodies;

	if (cl == NULL) {
		exit(-1);
	}

	// read the bodies to the open cl buffer
	bodies_obj = clCreateBuffer(cl->context, CL_MEM_READ_WRITE, sizeof(Body) * N, NULL, &cl->err);
	if (cl->err) {
		fprintf(stderr, "NaiveCL::NaiveCL clCreateBuffer error\n");
		exit(cl->err);
	}
	cl->err = clEnqueueWriteBuffer(cl->queue, bodies_obj, CL_TRUE, 0, sizeof(Body) * N, init_bodies, 0, NULL, NULL);
	if (cl->err) {
		fprintf(stderr, "NaiveCL::NaiveCL clEnqueueWriteBuffer error\n");
		exit(cl->err);
	}
}

Body* NaiveCL::next_move() {
	// execute kernels
	START_ACTIVITY(ACTIVITY_FORCE);
	opencl_next_move(&bodies_obj, cl);
	FINISH_ACTIVITY(ACTIVITY_FORCE);

	START_ACTIVITY(ACTIVITY_POSITION);
	opencl_update_position(&bodies_obj, cl);
	FINISH_ACTIVITY(ACTIVITY_POSITION);

	// read from the buffer and print it out
	START_ACTIVITY(ACTIVITY_DEVICE_TO_HOST);
	cl->err = clEnqueueReadBuffer(cl->queue, bodies_obj, CL_TRUE, 0, sizeof(Body) * N, bodies, 0, NULL, NULL);
	if (cl->err) {
		fprintf(stderr, "NaiveCL::next_move clEnqueueReadBuffer error\n");
		exit(cl->err);
	}
	FINISH_ACTIVITY(ACTIVITY_DEVICE_TO_HOST);

	return bodies;
}

NaiveCL::~NaiveCL() {
	opencl_cleanup(cl);
	cl->err = clReleaseMemObject(bodies_obj);
	if (cl->err) {
		fprintf(stderr, "NaiveCL::~NaiveCL clReleaseMemObject error\n");
		exit(cl->err);
	}
}