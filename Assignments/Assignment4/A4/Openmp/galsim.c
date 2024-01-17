#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "graphics.h"

#include <omp.h>

const double epsilon = 1e-3;
const float baseRadius = 0.005, circleColor = 0;
const int windowWidth = 800, windowHeight = 800;
const int blockSz = 100;

static inline int min(const int a, const int b)
{
    if (a < b)
    {
        return a;
    }
    else
        return b;
}
__attribute__((const)) // Pure

struct Node
{ /* Struct for celestial body */
    double pos_x;
    double pos_y;
    double mass;
    double vel_x;
    double vel_y;
    double brightness;
} __attribute__((packed));

typedef struct Node Node, *Link;

static inline void calculate_forces(Link *particles, const int N, const double factor, int start, int end, const double delta_t)
{
    double radius_x, radius_y, r_ij_square, target_pos_x, target_pos_y, inc_x, inc_y;
    double inverse_square, tmp;
    int i, i_block, j, j_block;

    for (i_block = start; i_block < end; i_block += blockSz)
        for (j_block = 0; j_block < N; j_block += blockSz)
            for (i = i_block; i < min(i_block + blockSz, end); i++)
            {
                target_pos_x = (*(particles) + i)->pos_x;
                target_pos_y = (*(particles) + i)->pos_y;
                inc_x = 0;
                inc_y = 0;
                for (j = j_block; j < min(j_block + blockSz, N); j++)
                {
                    /* In this for-loop with don't disregard the case where i=j since it will lead
                    to radius_x and radius_y = 0 and the increment will therefor be zero

                    this will lead to us always doing one extra set of calculation each time
                    we call "calculate_forces" function. I figured it was better (and confirmed
                    with time test) than always evaluating an if-statement */

                    radius_x = ((*(particles) + j)->pos_x) - target_pos_x;
                    radius_y = ((*(particles) + j)->pos_y) - target_pos_y;
                    r_ij_square = (radius_x * radius_x + radius_y * radius_y);
                    inverse_square = 1 / (sqrt(r_ij_square) + epsilon);
                    tmp = factor * (*(particles) + j)->mass * inverse_square * inverse_square * inverse_square;
                    inc_x += tmp * radius_x;
                    inc_y += tmp * radius_y;
                }
                (*(particles) + i)->vel_x += inc_x;
                (*(particles) + i)->vel_y += inc_y;
            }

#pragma omp barrier
    for (int i = start; i < end; i++)
    {
        (*(particles) + i)->pos_x = (*(particles) + i)->pos_x + (*(particles) + i)->vel_x * delta_t;
        (*(particles) + i)->pos_y = (*(particles) + i)->pos_y + (*(particles) + i)->vel_y * delta_t;
    }
}

void update_graphics(Link particles, const int N, int start, int end)
{
    ClearScreen();
    for (int i = start; i < end; i++)
    {
        DrawCircle((particles + i)->pos_x, (particles + i)->pos_y, 1, 1, baseRadius * (particles + i)->mass, circleColor);
    }
    Refresh();
}

int main(int argc, char *argv[])
{
    if (argc != 7)
    {
        printf("Usage: ./galsim N filename nsteps delta_t graphics number_of_threads");
        return -1;
    }

    /* Reading inputs */
    const int N = atoi(argv[1]);
    char *filename = argv[2];
    const int nsteps = atoi(argv[3]);
    const double delta_t = atof(argv[4]);
    const int graphics = atoi(argv[5]);
    const int nThreads = atoi(argv[6]);

    const int split_size = N / nThreads;
    int interval_splits[nThreads + 1];
    for (int i = 0; i < nThreads; i++)
        interval_splits[i] = i * split_size;
    interval_splits[nThreads] = N; /*  This makes sure that even if our N is not evenly divisible by nThreads
                                       All work will be done. However, this may cause the last thread to to
                                       more work (but as most nThreads more, for example 100, 100, 100, 107, if nThreads = 8)*/

    int start;
    int end; /*These will be made private*/

    const double G = (double)100.0 / (double)N;
    const double factor = G * delta_t;

    /* Read starting values and set */
    FILE *input_file;
    Link particles = (Link)malloc(N * sizeof(Node));
    input_file = fopen(filename, "rb");
    for (int i = 0; i < N; i++)
        if (!fread(particles + i, sizeof(Node), 1, input_file))
        {
            printf("Error reading input file...\\Exiting...");
            return -1;
        }
    fclose(input_file);

    if (graphics)
    { /* Calculate force and new velocity (ORDO N²)*/
        InitializeGraphics(argv[0], windowWidth, windowHeight);
        SetCAxes(0, 1);
        update_graphics(particles, N, 0, N);

        int j = 0;
        while (!CheckForQuit() && j < nsteps)
        {
#pragma omp parallel num_threads(nThreads)
            {
                int id = omp_get_thread_num();
                int start = interval_splits[id];
                int end = interval_splits[id + 1];
                for (int i = 0; i < nsteps; i++)
                {
                    calculate_forces(&particles, N, factor, start, end, delta_t);
#pragma omp single
                    {
                        update_graphics(particles, N, 0, N);
                        j++;
                    }
                }
            }
        }

        FlushDisplay();
        CloseDisplay();
    }
    else
    {
#pragma omp parallel num_threads(nThreads)
        {
            int id = omp_get_thread_num();
            int start = interval_splits[id];
            int end = interval_splits[id + 1];
            for (int j = 0; j < nsteps; j++)
            {
                calculate_forces(&particles, N, factor, start, end, delta_t);
            }
        }
    }

    FILE *output_file = fopen("result.gal", "wb");
    for (int i = 0; i < N; i++)
        fwrite(particles + i, 1, sizeof(Node), output_file);
    fclose(output_file);
    free(particles);
    return 0;
}
