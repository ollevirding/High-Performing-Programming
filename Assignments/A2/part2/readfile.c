#include <stdio.h>
#include <stdlib.h>

int main()
{
    int d;
    double lf;
    char c;
    float f;

    FILE *file;
    file = fopen("little_bin_file", "rb");
    
    fread(&d, sizeof(d),1,file);
    fread(&lf, sizeof(lf),1,file);
    fread(&c, sizeof(c),1,file);
    fread(&f, sizeof(f),1,file);

    printf("Int: %d\n", d);
    printf("Double: %lf\n", lf);
    printf("Char: %c\n", c);
    printf("Float: %f", f);

    return 0;
}