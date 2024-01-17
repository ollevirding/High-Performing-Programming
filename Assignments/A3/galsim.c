#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "graphics.h"

const double epsilon = 1e-3;
const float baseRadius = 0.005, circleColor = 0;
const int windowWidth = 800, windowHeight = 800;
const int blockSz = 150;

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

static inline Link calculate_forces(Link __restrict particles, Link __restrict target, const int N, const double factor)
{
    double radius_x, radius_y, r_ij_square, target_pos_x, target_pos_y, inc_x, inc_y;
    double inverse_square;
    int i, i_block, j, j_block;

    for (i_block = 0; i_block < N; i_block += blockSz)
        for (j_block = 0; j_block < N; j_block += blockSz)
            for (i = i_block; i < i_block + blockSz; i++)
            {
                target = particles + i;
                target_pos_x = target->pos_x;
                target_pos_y = target->pos_y;
                inc_x = 0;
                inc_y = 0;
                for (j = j_block; j < j_block + blockSz; j++)
                {
                    /* In this for-loop with don't disregard the case where i=j since it will lead
                    to radius_x and radius_y = 0 and the increment will therefor be zero

                    this will lead to us always doing one extra set of calculation each time
                    we call "calculate_forces" function. I figured it was better (and confirmed
                    with time test) than always evaluating an if-statement */

                    radius_x = ((particles + j)->pos_x) - target_pos_x;
                    radius_y = ((particles + j)->pos_y) - target_pos_y;
                    r_ij_square = (radius_x * radius_x + radius_y * radius_y);
                    inverse_square = 1 / (sqrt(r_ij_square) + epsilon);
                    inc_x += factor * (particles + j)->mass * inverse_square * inverse_square * inverse_square * radius_x;
                    inc_y += factor * (particles + j)->mass * inverse_square * inverse_square * inverse_square * radius_y;
                }
                target->vel_x += inc_x;
                target->vel_y += inc_y;
            }

    return particles;
}

static inline __attribute__((always_inline)) Link calculate_position(Link __restrict particles, const int N, const double delta_t)
{
    for (int i = 0; i < N; i++)
    {
        (particles + i)->pos_x = (particles + i)->pos_x + (particles + i)->vel_x * delta_t;
        (particles + i)->pos_y = (particles + i)->pos_y + (particles + i)->vel_y * delta_t;
    }
    return particles;
}
void update_graphics(Link particles, const int N)
{
    ClearScreen();
    for (int i = 0; i < N; i++)
    {
        DrawCircle((particles + i)->pos_x, (particles + i)->pos_y, 1, 1, baseRadius * (particles + i)->mass, circleColor);
    }
    Refresh();
    usleep(3000);
}

int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        printf("Usage: ./galsim N filename nsteps delta_t graphics ");
        return -1;
    }

    /* Reading inputs */
    const int N = atoi(argv[1]);
    char *filename = argv[2];
    const int nsteps = atoi(argv[3]);
    const double delta_t = atof(argv[4]);
    const int graphics = atoi(argv[5]);
    const int rest = nsteps % 5;

    const double G = (double)100.0 / (double)N;
    const double factor = G * delta_t;

    /* Read starting values and set */
    FILE *input_file;
    Link particles = (Link)malloc(N * sizeof(Node));
    Link buffer = (Link)malloc(sizeof(Node));
    input_file = fopen(filename, "rb");
    for (int i = 0; i < N; i++)
        fread(particles + i, sizeof(Node), 1, input_file);
    fclose(input_file);

    if (graphics)
    { /* Calculate force and new velocity (ORDO N²)*/
        InitializeGraphics(argv[0], windowWidth, windowHeight);
        SetCAxes(0, 1);
        update_graphics(particles, N);
        int j = 0;
        while (!CheckForQuit() && j < nsteps)
        {

            for (int i = 0; i < N; i++)
            {
                particles = calculate_forces(particles, buffer, N, factor);
                particles = calculate_position(particles, N, delta_t);
            }
            update_graphics(particles, N);
            j++;
        }

        FlushDisplay();
        CloseDisplay();
    }
    else
    {
        for (int j = 0; j < nsteps; j += 5)
        {

            particles = calculate_forces(particles, buffer, N, factor);
            particles = calculate_position(particles, N, delta_t);

            particles = calculate_forces(particles, buffer, N, factor);
            particles = calculate_position(particles, N, delta_t);

            particles = calculate_forces(particles, buffer, N, factor);
            particles = calculate_position(particles, N, delta_t);

            particles = calculate_forces(particles, buffer, N, factor);
            particles = calculate_position(particles, N, delta_t);

            particles = calculate_forces(particles, buffer, N, factor);
            particles = calculate_position(particles, N, delta_t);
        }
        for (int j = 0; j < rest; j++)
        {

            particles = calculate_forces(particles, buffer, N, factor);
            particles = calculate_position(particles, N, delta_t);
        }
    }

    FILE *output_file = fopen("result.gal", "wb");
    for (int i = 0; i < N; i++)
        fwrite(particles + i, 1, sizeof(Node), output_file);
    fclose(output_file);
    free(particles);
    free(buffer);
    return 0;
}
