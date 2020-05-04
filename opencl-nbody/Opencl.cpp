#include "Opencl.h"
#include "Config.h"
#include "Body.h"
#include <iostream>

const char* kernel_file_names[] = {
    "next_move.cl",
    "update_position.cl",
    "compute_force_kernel.cl"
};

const char* kernel_names[] = {
    "next_move",
    "update_position",
    "compute_force_kernel"
};

// Execute the OpenCL kernel on the list
size_t global_item_size = N; // Process the entire lists
size_t local_item_size = 64; // Divide work items into groups of 64


opencl* initialize_opencl() {
    opencl* cl = new opencl();

    // Get platform and device information
    cl_uint ret_num_devices;
    cl_uint ret_num_platforms;

    cl->err = clGetPlatformIDs(1, &cl->platform, &ret_num_platforms);
    if (cl->err) {
        fprintf(stderr, "clGetPlatformIDs error\n");
        exit(cl->err);
    }
    cl->err = clGetDeviceIDs(cl->platform, CL_DEVICE_TYPE_DEFAULT, 1, &cl->device, &ret_num_devices);
    if (cl->err) {
        fprintf(stderr, "clGetDeviceIDs error\n");
        exit(cl->err);
    }

    char* version_info[100];
    clGetDeviceInfo(cl->device, CL_DEVICE_VERSION, 100, version_info, NULL);
    printf("%s\n", version_info);

    // Create an OpenCL context
    cl->context = clCreateContext(NULL, 1, &cl->device, NULL, NULL, &cl->err);
    if (cl->err) {
        fprintf(stderr, "clCreateContext error\n");
        exit(cl->err);
    }

    // Create a command queue
    cl->queue = clCreateCommandQueue(cl->context, cl->device, 0, &cl->err);
    if (cl->err) {
        fprintf(stderr, "clCreateCommandQueue error\n");
        exit(cl->err);
    }

    char* source_str = (char*)malloc(MAX_SOURCE_SIZE);
    // read the kernel files, build the programs and create kernels
    for (int i = 0; i < KERNEL_COUNT; i++) {
        // Read the kernel source code and load into the array source_str
        FILE* fp = fopen(kernel_file_names[i], "r");
        if (!fp) {
            fprintf(stderr, "Failed to open file [%s] \n", kernel_file_names[i]);
            exit(1);
        }
        size_t source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
        fclose(fp);

        // Build a program from the kernel source file
        cl->program[i] = clCreateProgramWithSource(cl->context, 1, (const char**)&source_str, &source_size, &cl->err);
        if (cl->err) {
            fprintf(stderr, "clCreateProgramWithSource [%s] error\n", kernel_file_names[i]);
            exit(cl->err);
        }

        cl->err = clBuildProgram(cl->program[i], 0, NULL, "", NULL, NULL);
        if (cl->err) {
            fprintf(stderr, "clCreateProgramWithSource [%s] error\n", kernel_file_names[i]);
            size_t size;
            clGetProgramBuildInfo(cl->program[i], cl->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &size);
            char* buildlog = (char*)malloc(size);
            clGetProgramBuildInfo(cl->program[i], cl->device, CL_PROGRAM_BUILD_LOG, size, buildlog, NULL);
            printf("\n\nBuildlog:   %s\n\n", buildlog);
            exit(cl->err);
        }

        // create kernel
        cl->kernels[i] = clCreateKernel(cl->program[i], kernel_names[i], &cl->err);
        if (cl->err) {
            fprintf(stderr, "clCreateKernel [%s] error\n", kernel_names[i]);
            exit(cl->err);
        }
    }
    free(source_str);
    return cl;
}

void opencl_next_move(cl_mem* bodies, opencl* cl) {
    int idx = 0;
    // set argument for the kernel
    cl->err = clSetKernelArg(cl->kernels[idx], 0, sizeof(cl_mem), bodies);
    if (cl->err) {
        fprintf(stderr, "clSetKernelArg [%s] error\n", kernel_file_names[idx]);
        exit(cl->err);
    }

    // execute the kernel
    cl->err = clEnqueueNDRangeKernel(cl->queue, cl->kernels[idx], 1, NULL, &global_item_size, NULL, 0, NULL, NULL);
    if (cl->err) {
        fprintf(stderr, "clEnqueueNDRangeKernel [%s] error\n", kernel_file_names[idx]);
        exit(cl->err);
    }

    cl->err = clFinish(cl->queue);
    if (cl->err) {
        fprintf(stderr, "clFinish [%s] error\n", kernel_file_names[idx]);
        exit(cl->err);
    }
}

void opencl_update_position(cl_mem* bodies, opencl* cl) {
    int idx = 1;
    // set argument for the kernel
    cl->err = clSetKernelArg(cl->kernels[idx], 0, sizeof(cl_mem), bodies);
    if (cl->err) {
        fprintf(stderr, "clSetKernelArg [%s] error\n", kernel_file_names[idx]);
        exit(cl->err);
    }

    // execute the kernel
    cl->err = clEnqueueNDRangeKernel(cl->queue, cl->kernels[idx], 1, NULL, &global_item_size, NULL, 0, NULL, NULL);
    if (cl->err) {
        fprintf(stderr, "clEnqueueNDRangeKernel [%s] error\n", kernel_file_names[idx]);
        exit(cl->err);
    }

    cl->err = clFinish(cl->queue);
    if (cl->err) {
        fprintf(stderr, "clFinish [%s] error\n", kernel_file_names[idx]);
        exit(cl->err);
    }
}

void opencl_compute_force(cl_mem* bodies, cl_mem* nodes, opencl* cl) {
    int idx = 2;
    // set arguments for the kernel
    cl->err = clSetKernelArg(cl->kernels[idx], 0, sizeof(cl_mem), bodies);
    if (cl->err) {
        fprintf(stderr, "clSetKernelArg for bodies [%s] error\n", kernel_file_names[idx]);
        exit(cl->err);
    }

    cl->err = clSetKernelArg(cl->kernels[idx], 1, sizeof(cl_mem), nodes);
    if (cl->err) {
        fprintf(stderr, "clSetKernelArg [%s] for nodes error\n", kernel_file_names[idx]);
        exit(cl->err);
    }

    // execute the kernel
    cl->err = clEnqueueNDRangeKernel(cl->queue, cl->kernels[idx], 1, NULL, &global_item_size, NULL, 0, NULL, NULL);
    if (cl->err) {
        fprintf(stderr, "clEnqueueNDRangeKernel [%s] error\n", kernel_file_names[idx]);
        exit(cl->err);
    }

    cl->err = clFinish(cl->queue);
    if (cl->err) {
        fprintf(stderr, "clFinish [%s] error\n", kernel_file_names[idx]);
        exit(cl->err);
    }

}

void opencl_cleanup(opencl* cl) {
    // Clean up
    cl->err = clFlush(cl->queue);
    cl->err = clFinish(cl->queue);
    for (int i = 0; i < KERNEL_COUNT; i++) {
        cl->err = clReleaseKernel(cl->kernels[i]);
        cl->err = clReleaseProgram(cl->program[i]);
    }
    cl->err = clReleaseCommandQueue(cl->queue);
    cl->err = clReleaseContext(cl->context);
}