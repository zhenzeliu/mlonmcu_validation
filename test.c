#include <stdio.h>
#include <inttypes.h>
#include "mlif.h"

#define SIZE 10

int main(void)
{
    uint8_t a[SIZE] = {1, 2, 3, 4, 5, 6, 7, -128};

    mlif_data_config cfg;

    cfg.dtype = MLIF_DTYPE_UINT8;
    cfg.row = 1;
    cfg.col = 10;

    mlif_porcess_output(a, sizeof(*a) * SIZE);

    mlif_io_write_npy("./a.npy", &cfg, a);

    return 0;
}
