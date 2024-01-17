#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

int solution_found = 0;

typedef struct unassigned_t
{
    int row;
    int column;
    int possibilities;
    int list_of_possibilities[36]; // 36 is semi arbitrary, it is designed to be compatible 36x36 boards. For bigger boards, use the side length as length of the array
} unassigned_t;

void clear_lines(const int n) // https://stackoverflow.com/questions/1348563/clearing-output-of-a-terminal-program-linux-c-c
{
    for (int i = 0; i < n; i++)
        printf("\033[A\033[2K"); // '\033[A' is go up a line '\033[2K' is clear line
    usleep(50000);
}

void printBoard(int **board, const int size, const int sub_size)
{
    int i, j;
    const int num_width = (size >= 10) ? 3 : 2; // calculate the width of the number format specifier (can be read as if size is >= 10 num_width = 3 else 2)
    const int length = size + size * num_width + sub_size - 2;
    for (i = 0; i < size; i++)
    {
        if (i % sub_size == 0 && i != 0)
        {
            // Print horizontal separator line
            for (j = 0; j < length; j++)
                printf("-"); // print a single hyphen to align with the numbers
            printf("\n");
        }
        for (j = 0; j < size; j++)
        {
            if (j % sub_size == 0 && j != 0)
                printf("|");
            printf("%*d ", num_width, board[i][j]); // use the width of the number format specifier
        }
        printf("\n");
    }
}

int ValidBoard(int **board, int row, int col, const int size, const int sub_size)
{
    int i, j;
    int number_to_check = board[row][col];

    // Check if number to check is in row, column, or sub-square
    for (i = 0; i < size; i++)
        if ((board[i][col] == number_to_check && i != row) ||
            (board[row][i] == number_to_check && i != col))
            return 0;

    // Acquire sub-square based on column and row
    const int sub_square_row = row / sub_size;
    const int sub_square_col = col / sub_size;

    // Check if number to check is in sub-square
    for (i = sub_square_row * sub_size; i < sub_square_row * sub_size + sub_size; i++)
        for (j = sub_square_col * sub_size; j < sub_square_col * sub_size + sub_size; j++)
            if (board[i][j] == number_to_check && (i != row && j != col))
                return 0;

    return 1;
}

int find_n_unassigned(int **board, const int size, const int sub_size) // Finding how many unassigned cells there are
{
    int n_unassigned = 0;

    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            if (board[i][j] == 0)
                n_unassigned++;

    return n_unassigned;
}

void sort_unassigned(int **board, unassigned_t *unassigned_indicies, int n_unassigned, const int size, const int sub_size)
{
    int i, j;
    int n = n_unassigned;
    unassigned_t unassigned_indicies_buffer[n_unassigned];

    for (i = 0; i < size; i++)
        for (j = 0; j < size; j++)
            if (board[i][j] == 0) // If the number is unassigned
            {
                unassigned_indicies_buffer[n_unassigned - 1].row = i; // save the row and column number of the unassigned number and place it at the last empty position
                unassigned_indicies_buffer[n_unassigned - 1].column = j;
                unassigned_indicies_buffer[n_unassigned - 1].possibilities = 0; // Inizialize the number of possibilities

                n_unassigned--; // We know how many unassigned numbers there are, so decrement the counter

                for (int val = 1; val <= size; val++) // For each number
                {
                    board[i][j] = val;
                    if (ValidBoard(board, i, j, size, sub_size)) // Check if the number is a valid placement
                    {
                        unassigned_indicies_buffer[n_unassigned].list_of_possibilities[unassigned_indicies_buffer[n_unassigned].possibilities] = val; // Place the number in the list of possibilities
                        unassigned_indicies_buffer[n_unassigned].possibilities++;                                                                     // Increment the number of possibilities
                    }
                }
                board[i][j] = 0; // Reset the number to unassigned
            }
    for (i = 0; i < n; i++) // Selection Sort algorithm ()
    {
        int max = -1; // Initialize the max value to -1 to ensure that we always find a bigger number
        int index = i;
        for (j = i; j < n; j++)
            if (unassigned_indicies_buffer[j].possibilities > max) // If the number of possibilities is greater than the current greatest value
            {
                max = unassigned_indicies_buffer[j].possibilities; // Set the max value
                index = j;                                         // Save the index of the unassigned number
            }
        unassigned_indicies[i] = unassigned_indicies_buffer[index];        // Place the unassigned number in the list of unassigned numbers
        unassigned_indicies_buffer[index] = unassigned_indicies_buffer[i]; // Switch places of the unassigned numbers
        unassigned_indicies_buffer[i] = unassigned_indicies_buffer[index];
    }
}

int Is_solvable(int **board, const int size, const int sub_size)
{
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            if (board[i][j] != 0)
                if (!ValidBoard(board, i, j, size, sub_size)) // Check each cell if it has any conflict
                    return 0;
    return 1;
}

void solveBoard(int ***board, unassigned_t *unassigned_indicies, int n_unassigned, const int size, const int sub_size, const int graphics, int *solution_found)
{
    if (n_unassigned == 0) // No unassigned numbers meaning the solution is found
    {
        *solution_found = 1;
        printf("\n\n");
        printBoard(*board, size, sub_size);      // Print the solved board
        if (Is_solvable(*board, size, sub_size)) // Verify that the solution is valid
            printf("Solution is valid\n");
        else
            printf("Solution is invalid\n");
        return;
    }

    if (*solution_found) // Solution found so all other threads should stop
        return;

    int index[2];

    if (unassigned_indicies[n_unassigned - 1].possibilities != 1) // If there are more than one possible placement in all cells
    {
        sort_unassigned(*board, unassigned_indicies, n_unassigned, size, sub_size); // Update the unassigned numbers to see if new single possibilities are found
        if (unassigned_indicies[n_unassigned - 1].possibilities == 0)               // If there are zero possible placements in any cells, we have reach a dead end
            return;                                                                 // We should therefore stop the search
    }

    index[0] = unassigned_indicies[n_unassigned - 1].row;
    index[1] = unassigned_indicies[n_unassigned - 1].column;

    if (unassigned_indicies[n_unassigned - 1].possibilities > 1) // If there are more than one possible placement in all cells (else row 210)
    {
        for (int pos = 0; pos < unassigned_indicies[n_unassigned - 1].possibilities; pos++) // For each possible placement in all cells

#pragma omp task // Threads are given one of the possible placements as tasks
        {
            if (!(*solution_found)) // If the solution is found we should stop the search
            {

                int **local_board = (int **)malloc(size * sizeof(int *)); // Allocate memory for the local board that is unique for each thread
                for (int i = 0; i < size; i++)
                {
                    local_board[i] = (int *)malloc(size * sizeof(int));
                    memcpy(local_board[i], (*board)[i], size * sizeof(int)); // Copy the existing board to the local board
                }

                unassigned_t *local_unassigned_indicies = (unassigned_t *)malloc(n_unassigned * sizeof(unassigned_t)); // Each thread has its own local unassigned numbers
                sort_unassigned(local_board, local_unassigned_indicies, n_unassigned, size, sub_size);                 // Sort the local unassigned numbers

                local_board[index[0]][index[1]] = local_unassigned_indicies[n_unassigned - 1].list_of_possibilities[pos];
                if (ValidBoard(local_board, index[0], index[1], size, sub_size))
                {
                    if (graphics)
                    {
                        clear_lines(size + sub_size - 1);
                        printBoard(local_board, size, sub_size);
                    }

                    solveBoard(&local_board, local_unassigned_indicies, n_unassigned - 1, size, sub_size, graphics, solution_found); // Go deeper with the local board
                }
                free(local_unassigned_indicies); // Free the local unassigned numbers
                for (int i = 0; i < size; i++)
                    free(local_board[i]); // Free the local board
                free(local_board);
            }
        }
    }
    else // Else (If only one possible placement in any cell)
    {
        (*board)[index[0]][index[1]] = unassigned_indicies[n_unassigned - 1].list_of_possibilities[0];

        if (ValidBoard(*board, index[0], index[1], size, sub_size))
        {
            if (graphics)
            {
                clear_lines(size + sub_size - 1);
                printBoard(*board, size, sub_size);
            }

            solveBoard(board, unassigned_indicies, n_unassigned - 1, size, sub_size, graphics, solution_found);
        }
    }
#pragma omp taskwait // We want to wait for all task to finish before returning, else SEGFAULT
    return;
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        printf("Usage: %s input_file size graphics number_of_threads\n\nNote: Graphics is not available when running more than one thread", argv[0]);
        return 1;
    }
    const int size = atoi(argv[2]);
    const int sub_size = sqrt(size);
    const int graphics = atoi(argv[3]);
    const int nThreads = atoi(argv[4]);
    if (nThreads > 1 && graphics)
    {
        printf("Graphics is not available when running more than one thread\n");
        return -1;
    }
    FILE *input_file = fopen(argv[1], "r");
    printf("Input file: %s\n", argv[1]);
    if (input_file == NULL)
    {
        printf("Cannot open input file: %s\n", argv[1]);
        return -1;
    }
    // Allocate memory for the board
    int **board = (int **)malloc(size * sizeof(int *));
    for (int i = 0; i < size; i++)
        board[i] = (int *)malloc(size * sizeof(int));

    // Read the CSV input file and store it in board
    for (int i = 0; i < size; i++)
        for (int j = 0; j < size; j++)
            fscanf(input_file, "%d,", &board[i][j]);

    // Close the input file
    fclose(input_file);

    // find n_unassigned and unassigned_indicies
    int n_unassigned = find_n_unassigned(board, size, sub_size);
    unassigned_t *unassigned_indicies = (unassigned_t *)malloc(n_unassigned * sizeof(unassigned_t));
    sort_unassigned(board, unassigned_indicies, n_unassigned, size, sub_size);

    printf("Initialized board:\n");
    printBoard(board, size, sub_size);

    if (!Is_solvable(board, size, sub_size)) // Verify if the board is solvable i.e valid
    {
        printf("No solution exists\n");
        return -1;
    }
    int solution_found = 0;
#pragma omp parallel num_threads(nThreads)
    {
#pragma omp single
        solveBoard(&board, unassigned_indicies, n_unassigned, size, sub_size, graphics, &solution_found);
    }
    if (solution_found)
    {
        printf("Solution found!\n");
    }
    else
    {
        printf("Solution not found!\n");
    }
    free(unassigned_indicies);
    for (int i = 0; i < size; i++)
        free(board[i]);
    free(board);

    return 0;
}
