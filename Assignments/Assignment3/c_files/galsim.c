#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "graphics.h" // Unedited from Oliver Fringer, Stanford University

const double epsilon = pow(10, -3);
const float baseRadius = 0.005, circleColor = 0;
const int windowWidth = 800, windowHeight = 800;
const char* foldername = "../input";

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

void print_all_velocity(Link particles, const int N)
{
    for (int i = 0; i < N; i++)
    {
        printf("Velocity X: %lf\n", (particles + i)->vel_x);
        printf("Velocity Y: %lf\n\n", (particles + i)->vel_y);
    }
}

void print_absvel(double velx, double vely)
{
    double abs = sqrt(velx*velx + vely*vely);
    printf("Absolut velocity: %lf\n", abs);
}

float I_rsqrt(float number)
{
    float y = 1 / sqrtf(number);
    return y;
}

Link calculate_forces(Link particles, const int N, const double G, const double delta_t)
{
    double radius_x, radius_y, r_ij_square, factor, target_pos_x, target_pos_y, inc_x, inc_y;
    double inverse_square;
    double sum;
    Link target;
    // int fast = 1;

    for (int i = 0; i < N; i++)
    {
        target = particles + i;
        factor = delta_t * G * 1 / target->mass;
        target_pos_x = target->pos_x;
        target_pos_y = target->pos_y;
        inc_x = 0;
        inc_y = 0;

        for (int j = 0; j < N; j++)
        {
            if (i != j)
            {

                radius_x = ((particles + j)->pos_x) - target_pos_x;
                radius_y = ((particles + j)->pos_y) - target_pos_y;
                r_ij_square = (radius_x * radius_x + radius_y * radius_y);
                inverse_square = 1 / (sqrt(r_ij_square) + epsilon);
                sum = (particles + j)->mass * inverse_square * inverse_square * inverse_square;

                inc_x += factor * sum * radius_x;
                inc_y += factor * sum * radius_y;
            }
        }
        target->vel_x += inc_x;
        target->vel_y += inc_y;
        print_absvel(target->vel_x, target->vel_y);
    }

    return particles;
}

void print_all_positions(Link particles, const int N)
{
    for (int i = 0; i < N; i++)
    {
        printf("Position X: %lf\n", (particles + i)->pos_x);
        printf("Position Y: %lf\n\n", (particles + i)->pos_y);
    }
}

Link calculate_position(Link particles, const int N, const double delta_t)
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
    const double delta_t = atoi(argv[4]);
    const int graphics = atoi(argv[5]);


    // const int N = 500;
    // char *filename = "ellipse_N_00500.gal";

    // const int N = 10;
    // char *filename = "ellipse_N_00010.gal";

    // const int N = 100;
    // char *filename = "ellipse_N_00100.gal";

    // const int N = 2;
    // char *filename = "circles_N_2.gal";

    // const int nsteps = 200;
    // const double delta_t = pow(10, -5);
    // const int graphics = 0;
    const double G = 100 / N;

    /* Read starting values and set */
    FILE *input_file;
    Link particles = (Link)malloc(N * sizeof(Node));
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
                particles = calculate_forces(particles, N, G, delta_t);
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
        for (int j = 0; j < nsteps; j++)
        {

            particles = calculate_forces(particles, N, G, delta_t);
            particles = calculate_position(particles, N, delta_t);
        }
    }

    FILE *output_file = fopen("results.gal", "wb");
    for (int i = 0; i < N; i++)
        fwrite(particles + i, 1, sizeof(Node), output_file);
    free(particles);
    return 0;
}
