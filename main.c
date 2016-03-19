#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>

#include "primitives.h"
#include "raytracing.h"

#define OUT_FILENAME "out.ppm"

#define ROWS 512
#define COLS 512
#define NUMBER_OF_THREADS 4

pthread_t callThd[NUMBER_OF_THREADS];

static void write_to_ppm(FILE *outfile, uint8_t *pixels,
                         int width, int height)
{
    fprintf(outfile, "P6\n%d %d\n%d\n", width, height, 255);
    fwrite(pixels, 1, height * width * 3, outfile);
}

static double diff_in_second(struct timespec t1, struct timespec t2)
{
    struct timespec diff;
    if (t2.tv_nsec-t1.tv_nsec < 0) {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec - 1;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec + 1000000000;
    } else {
        diff.tv_sec  = t2.tv_sec - t1.tv_sec;
        diff.tv_nsec = t2.tv_nsec - t1.tv_nsec;
    }
    return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

int main()
{
    uint8_t *pixels;
    light_node lights = NULL;
    rectangular_node rectangulars = NULL;
    sphere_node spheres = NULL;
    color background = { 0.0, 0.1, 0.1 };
    struct timespec start, end;
    pthread_attr_t attr;
    void *status;
#include "use-models.h"
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    /* allocate by the given resolution */
    pixels = malloc(sizeof(unsigned char) * ROWS * COLS * 3);
    thread_arg *arg_lefttop = setThread_arg(pixels, background,
                                            rectangulars, spheres, lights,
                                            &view, ROWS/2, COLS/2, 0, 0);
    thread_arg *arg_leftdown = setThread_arg(pixels, background,
                               rectangulars, spheres, lights,
                               &view, ROWS, COLS/2, ROWS/2, 0);
    thread_arg *arg_righttop = setThread_arg(pixels, background,
                               rectangulars, spheres, lights,
                               &view, ROWS/2, COLS, 0, COLS/2);
    thread_arg *arg_rightdown = setThread_arg(pixels, background,
                                rectangulars, spheres, lights,
                                &view, ROWS, COLS, ROWS/2, COLS/2);

    pthread_attr_destroy(&attr);
    if (!pixels) exit(-1);

    printf("# Rendering scene\n");
    /* do the ray tracing with the given geometry */
    clock_gettime(CLOCK_REALTIME, &start);
    pthread_create(&callThd[0], &attr, raytracing, (void *)arg_lefttop);
    pthread_create(&callThd[1], &attr, raytracing, (void *)arg_leftdown);
    pthread_create(&callThd[2], &attr, raytracing, (void *)arg_righttop);
    pthread_create(&callThd[3], &attr, raytracing, (void *)arg_rightdown);

    //raytracing(pixels, background,
    //           rectangulars, spheres, lights, &view, ROWS, COLS);
    pthread_attr_destroy(&attr);

    pthread_join(callThd[0], &status);
    pthread_join(callThd[1], &status);
    pthread_join(callThd[2], &status);
    pthread_join(callThd[3], &status);

    clock_gettime(CLOCK_REALTIME, &end);
    {
        FILE *outfile = fopen(OUT_FILENAME, "wb");
        write_to_ppm(outfile, pixels, ROWS, COLS);
        fclose(outfile);
    }

    delete_rectangular_list(&rectangulars);
    delete_sphere_list(&spheres);
    delete_light_list(&lights);
    free(pixels);

    printf("Done!\n");
    printf("Execution time of raytracing() : %lf sec\n", diff_in_second(start, end));
    return 0;
}
