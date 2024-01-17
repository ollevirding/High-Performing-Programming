#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include "graphics.h"

const double epsilon = 1e-3;
const float baseRadius = 0.005, circleColor = 0;
const int windowWidth = 800, windowHeight = 800;
const int blockSz = 100;

int waiting = 0;
int state = 0;

pthread_mutex_t mutex;
pthread_cond_t cond;

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

typedef struct arg_struct
{
    int start;
    int end;
    Link *particles;
    double factor;
    int N;
    double delta_t;
    int num_threads;
} arg_t;

void barrier(int NUM_THREADS)
{ /* CODE USED FROM LAB 9.3*/
    int mystate;
    pthread_mutex_lock(&mutex);
    mystate = state;
    waiting++;
    if (waiting == NUM_THREADS)
    {
        waiting = 0;
        state = 1 - mystate;
        pthread_cond_broadcast(&cond);
    }
    while (mystate == state)
    {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

void *calculate_forces(void *args)
{
    arg_t *arguments = (arg_t *)args;
    const int start = arguments->start;
    const int end = arguments->end;
    Link *particle_list = arguments->particles;
    const double factor = arguments->factor;
    const double delta_t = arguments->delta_t;
    const int N = arguments->N;
    const int num_threads = arguments->num_threads;

    Link target;
    Link source = *(particle_list);
    double radius_x, radius_y, r_ij_square, target_pos_x, target_pos_y, inc_x, inc_y;
    double inverse_square, tmp;
    int i, i_block, j, j_block;

    for (i_block = start; i_block < end; i_block += blockSz)
        for (j_block = 0; j_block < N; j_block += blockSz)
            for (i = i_block; i < min(i_block + blockSz, end); i++)
            {
                target = *(particle_list) + i;
                target_pos_x = target->pos_x;
                target_pos_y = target->pos_y;
                inc_x = 0;
                inc_y = 0;
                for (j = j_block; j < min(j_block + blockSz, N); j++)
                {
                    radius_x = ((source + j)->pos_x) - target_pos_x;
                    radius_y = ((source + j)->pos_y) - target_pos_y;
                    r_ij_square = (radius_x * radius_x + radius_y * radius_y);
                    inverse_square = 1 / (sqrt(r_ij_square) + epsilon);
                    tmp = factor * (source + j)->mass * inverse_square * inverse_square * inverse_square;
                    inc_x += tmp * radius_x;
                    inc_y += tmp * radius_y;
                }
                (*(particle_list) + i)->vel_x += inc_x;
                (*(particle_list) + i)->vel_y += inc_y;
            }

    barrier(num_threads);

    for (int i = start; i < end; i++)
    {
        (*(particle_list) + i)->pos_x = (*(particle_list) + i)->pos_x + (*(particle_list) + i)->vel_x * delta_t;
        (*(particle_list) + i)->pos_y = (*(particle_list) + i)->pos_y + (*(particle_list) + i)->vel_y * delta_t;
    }

    return NULL;
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
    if (argc != 7)
    {
        printf("Usage: ./galsim N filename nsteps delta_t graphics threads ");
        return -1;
    }

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    /* Reading inputs */
    const int N = atoi(argv[1]);
    char *filename = argv[2];
    const int nsteps = atoi(argv[3]);
    const double delta_t = atof(argv[4]);
    const int graphics = atoi(argv[5]);
    const int num_threads = atoi(argv[6]);

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

    const int splits = N / num_threads;
    const int rest = N % num_threads;

    pthread_t threads[num_threads];
    arg_t arguments[num_threads];

    for (int i = 0; i < num_threads; i++)
    {
        (arguments + i)->start = i * splits;
        (arguments + i)->end = (i + 1) * splits;
        (arguments + i)->factor = factor;
        (arguments + i)->delta_t = delta_t;
        (arguments + i)->N = N;
        (arguments + i)->particles = &particles;
        (arguments + i)->num_threads = num_threads;
    }
    if (rest > 0)
        (arguments + num_threads - 1)->end = N;
    

    if (graphics)
    { /* Calculate force and new velocity (ORDO N²)*/
        InitializeGraphics(argv[0], windowWidth, windowHeight);
        SetCAxes(0, 1);
        update_graphics(particles, N);
        int j = 0;
        while (!CheckForQuit() && j < nsteps)
        {

            for (int t = 0; t < num_threads; t++)
                pthread_create(&threads[t], NULL, calculate_forces, (void *)(arguments + t));

            for (int t = 0; t < num_threads; t++)
                pthread_join(threads[t], NULL);

            update_graphics(particles, N);
            j++;
        }

        FlushDisplay();
        CloseDisplay();
    }
    else
    {
        for (int i = 0; i < nsteps; i++)
        {
            for (int t = 0; t < num_threads; t++)
                pthread_create(&threads[t], NULL, calculate_forces, (void *)(arguments + t));

            for (int t = 0; t < num_threads; t++)
                pthread_join(threads[t], NULL);
        }
    }

    FILE *output_file = fopen("result.gal", "wb");
    for (int i = 0; i < N; i++)
        fwrite(particles + i, 1, sizeof(Node), output_file);
    fclose(output_file);
    free(particles);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
