#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node
{
    int in_use;
    int day;
    double min;
    double max;
    struct Node *next_day;
} Node, *Link;

Link find_day(int day, Link current_day)
{
    if (current_day->day == day)
        return current_day;
    else if (current_day->day == 31)
        return NULL;
    return find_day(day, current_day->next_day);
}

void add_data(double min, double max, Link day)
{
    day->in_use = 1;
    day->min = min;
    day->max = max;
}

void delete_day(Link day)
{
    day->in_use = 0;
}

void print_data(Link first)
{
    if (first->in_use == 1)
        printf("Day: %d\tMin: %lf\tMax: %lf\n", first->day, first->min, first->max);
    if (first->next_day != NULL)
        print_data(first->next_day);
}

void build_database(int day, Link Node)
{
    Node->day = day;
    Node->in_use = 0;
    if (day != 31)
    {
        Node->next_day = Node + 1;
        build_database(day + 1, Node + 1);
    }
    else
        Node->next_day = NULL;
}

int main()
{
    Link first = (Link)malloc(31 * sizeof(Node));
    build_database(1, first);
    char *option = (char *)malloc(sizeof(char));
//    char input_buffer[100];
    double min;
    double max;
    int day;
    int state = 1;
    Link day_ptr;
    while (state)
    {
        printf("Enter command: ");
        scanf(" %c", option);
        switch (*option)
        {
        case 'A':
            scanf("%d %lf %lf", &day, &min, &max);
            day_ptr = find_day(day, first);
            add_data(min, max, day_ptr);
            break;

        case 'D':
            scanf("%d", &day);
            day_ptr = find_day(day, first);
            delete_day(day_ptr);
            break;

        case 'P':
            print_data(first);
            break;

        case 'Q':
            state = 0;
            break;

        default:
            printf("Invalid option!");
            break;
        }
    }

    free(first);
    free(option);
    return 0;
}
