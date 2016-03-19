#ifndef __RAYTRACING_H
#define __RAYTRACING_H

#include "objects.h"
#include <stdint.h>

typedef struct __THREAD_ARG {
    uint8_t *pixels;
    color background_color;
    rectangular_node rectangulars;
    sphere_node spheres;
    light_node lights;
    const viewpoint *view;
    int width;
    int height;
    int wstart;
    int hstart;
} thread_arg;

void raytracing(void *pArg);

thread_arg *setThread_arg(uint8_t *pixels, color background_color,
                          rectangular_node rectangulars, sphere_node spheres,
                          light_node lights, const viewpoint *view,
                          int width, int height, int wstart, int hstart);

#endif
