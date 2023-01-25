#include "shell_array.h"

#include <stdio.h>
#include <stdlib.h>

long *Array_Load_From_File(char *filename, int *size) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        *size = 0;
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    *size = ftell(fp) / sizeof(long);
    rewind(fp);

    long* arr = malloc((*size) * sizeof(long));
    if (arr == NULL) {
        return arr;
    }

    int num = fread(arr, sizeof(long), *size, fp);
    if (num != *size) {
        perror("fread");
        exit(EXIT_FAILURE);
    }
    fclose(fp);

    return arr;
}

int Array_Save_To_File(char *filename, long *array, int size) {
    FILE* fp = fopen(filename, "w");
    int num = fwrite(array, sizeof(long), size, fp);
    fclose(fp);
    return num;
}

static void Array_Insert(long *array, int size, int k, long *n_comp) {
    int i, j;
    for (j = k; j < size; ++j) {
        long temp_r = array[j];
        i = j;
        while (i >= k && array[i - k] > temp_r) {
            array[i] = array[i-k];
            i = i - k;
            *n_comp += 1;
        }
        array[i] = temp_r;
    }
}

/* static void debug_print(long *array, int size) { */
/*     int i = 0; */
/*     for (i = 0; i < size; ++i) { */
/*         printf("%d: %ld\n", i, array[i]); */
/*     } */
/*     printf("-----\n"); */
/* } */

void Array_Shellsort(long *array, int size, long *n_comp) {
    int maxk = 1;
    while ((maxk * 3 + 1) < size) {
        maxk = maxk * 3 + 1;
    }

    *n_comp = 0;

    int k = 0;
    /* debug_print(array, size); */
    for (k = maxk; k >= 1; k = (k - 1) / 3) {
        Array_Insert(array, size, k, n_comp);
        /* debug_print(array, size); */
    }
}
