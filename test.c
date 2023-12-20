#include <stdio.h>
#include <inttypes.h>
#include "mlif.h"

#define SIZE 10

int main(void)
{
    int8_t a[SIZE] = {1, 2, 3, 4, 5, 6, 7, -128};

    mlif_porcess_output(a, sizeof(*a) * SIZE);

    return 0;
}
