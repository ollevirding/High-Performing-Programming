#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include "graphics.h" // Unedited from Oliver Fringer, Stanford University

const double epsilon = 1e-3;
const float baseRadius = 0.005, circleColor = 0;
const int windowWidth = 800, windowHeight = 800;

typedef struct Node
{ /* Struct for celestial body */
    int particle_nr;
    double pos_x;
    double pos_y;
    double vel_x;
    double vel_y;
    double mass;
    double brightness;
    struct Node *next;
} Node, *Link;

Link build_linked_list(FILE *input, int N, Link root)
{
    if (root != NULL)
    {
        root = malloc(sizeof(Node));
        root->particle_nr = N;
        fread(&root->pos_x, sizeof(double), 1, input);
        fread(&root->pos_y, sizeof(double), 1, input);
        fread(&root->mass, sizeof(double), 1, input);
        fread(&root->vel_x, sizeof(double), 1, input);
        fread(&root->vel_y, sizeof(double), 1, input);
        fread(&root->brightness, sizeof(double), 1, input);
        root->next = build_linked_list(input, N - 1, root->next);
        // printf("Mass: %lf\n", root->pos_y);
    }

    return root;
}

Link calculate_forces(Link root, Link target, const int N, const double G, const double delta_t)
{
    if (target != NULL)
    {
        double radius_x, radius_y, acc_x, acc_y, r_ij;
        double sum_x = 0;
        double sum_y = 0;
        double sum;
        Link source = root;

        for (int i = 0; i < N; i++)
        {
            if (source->particle_nr != target->particle_nr)
            {
                // printf("Particle Number: %d\n", target->particle_nr);
                radius_x = (source->pos_x) - (target->pos_x);
                // printf("Radius_x: %lf\n", radius_x);
                radius_y = (source->pos_y) - (target->pos_y);
                // printf("Radius_y: %lf\n", radius_y);
                r_ij = sqrt(radius_x * radius_x + radius_y * radius_y);
                sum = source->mass / pow((r_ij + epsilon), 3); // Due to change
                // printf("Sum: %lf\n", sum);
                sum_x += sum * (radius_x); // Due to change
                // printf("Sum_x: %lf\n", sum_x);
                sum_y += sum * (radius_y); // Due to change
                // printf("Sum_y: %lf\n\n\n", sum_y);
            }
            source = source->next;
        }
        target->next = calculate_forces(root, target->next, N, G, delta_t);
        acc_x = G * sum_x / target->mass;
        acc_y = G * sum_y / target->mass;

        // printf("Particle Number: %d\n", target->particle_nr);
        // printf("Acc_x: %lf\nAcc_y: %lf\n\n\n", acc_x, acc_y);
        target->vel_x += acc_x * delta_t;
        target->vel_y += acc_y * delta_t;
    }
    return target;
}

void print_all_positions(Link root)
{
    if (root == NULL)
    {
        return;
    }
    printf("Particle: %d\n", root->particle_nr);
    printf("Position X: %lf\n", root->pos_x);
    printf("Position Y: %lf\n\n", root->pos_y);
    print_all_positions(root->next);
}

void print_all_velocity(Link root)
{
    if (root == NULL)
    {
        return;
    }
    printf("Particle: %d\n", root->particle_nr);
    printf("Velocity X: %lf\n", root->vel_x);
    printf("Velocity Y: %lf\n\n", root->vel_y);
    print_all_velocity(root->next);
}

Link calculate_position(Link root, const double delta_t)
{
    if (root != NULL)
    {
        root->pos_x = root->pos_x + root->vel_x * delta_t;
        root->pos_y = root->pos_y + root->vel_y * delta_t;
        root->next = calculate_position(root->next, delta_t);
    }
    return root;
}

void update_graphics(Link root, const int N)
{
    ClearScreen();
    for (int i = 0; i < N; i++)
    {
        DrawCircle(root->pos_x, root->pos_y, 1, 1, baseRadius * root->mass, circleColor);
        root = root->next;
    }
    Refresh();
    usleep(3000);
}

void delete_linked_list(Link root)
{
    if (root == NULL)
        ;
    return;
    delete_linked_list(root->next);
    free(root);
}

int main(int argc, char *argv[])
{
    // if (argc != 6)
    // {
    //     printf("Usage: ./galsim N filename nsteps delta_t graphics ");
    //     return -1;
    // }

    // /* Reading inputs */
    // const int N = atoi(argv[1]);
    // char *filename = argv[2];
    // const int nsteps = atoi(argv[3]);
    // const double delta_t = atoi(argv[4]);
    // const int graphics = atoi(argv[5]);

    const int N = 20;
    char *filename = "ellipse_N_00020.gal";

    // const int N = 4;
    // char *filename = "circles_N_4.gal";

    const int nsteps = 100;
    const double delta_t = 1e-5;
    const int graphics = 1;

    const double G = 100 / N;
    Link root = NULL;

    /* Read starting values and set */
    FILE *input_file;
    input_file = fopen(filename, "rb");
    root = build_linked_list(input_file, N, root);
    fclose(input_file);

    if (graphics)
    { /* Calculate force and new velocity (ORDO N²)*/
        InitializeGraphics(argv[0], windowWidth, windowHeight);
        SetCAxes(0, 1);
        update_graphics(root, N);
        for (int j = 0; j < nsteps; j++)
        {
            for (int i = 0; i < N; i++)
            {
                root = calculate_forces(root, root, N, G, delta_t);
                root = calculate_position(root, delta_t);
            }
            // print_all_velocity(root);
            // print_all_positions(root);
            update_graphics(root, N);
        }

        while (!CheckForQuit())
            ;
        FlushDisplay();
        CloseDisplay();
    }
    else
    {
        for (int j = 0; j < nsteps; j++)
        {
            for (int i = 0; i < N; i++)
            {
                root = calculate_forces(root, root, N, G, delta_t);
                root = calculate_position(root, delta_t);
            }
        }
    }
    delete_linked_list(root);
    return 0;
    // print_all_positions(root);
    // print_all_velocity(root);
}
