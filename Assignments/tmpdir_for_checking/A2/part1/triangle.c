#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void calculate_coefficient(int row, int *coefficient)
{
    int n = row;
    int k;
    for (k = 0; k <= n; k++)
    {
        double C = 1;
        for (int i = 1; i <= k; i++)
        {
            C *= (double)(n + 1- i) / (i); //This is probably a sub-optimal solution memory wise
        }
        coefficient[k] = lround(C);
    }
}

void print_coefficient(int row, int *coefficient)
{
    for (int k = 0; k <= row; k++)
        printf("%d\t", coefficient[k]);
    printf("\n");
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
	printf("Program to calculate the coefficients of Pascal's triangle\n");
        printf("Usage: ./triangle {height of triangle}\n");
        return -1;
    }
    int height = atoi(argv[1]);                              // Read input as height
    int *coefficients = (int *)malloc(height * sizeof(int)); // Allocate maximum height but reuse the allocated memory to reduce malloc calls
    for (int row = 0; row < height; row++)
    {
        calculate_coefficient(row, coefficients);
        print_coefficient(row, coefficients);
    }
    free(coefficients);
    return 0;
}
