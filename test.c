#include <stdio.h>
#include <inttypes.h>

#include "mlifio.h"

#define SIZE 20

int main(void)
{
    int16_t a[SIZE] = {0x61, 0x62, 0x63, 0x64, 0x65,
                        0x66, 0x67, 0x68, 0x69, 0x6A,
                        0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
                        0x70, 0x71, 0x72, 0x73, 0x74};

    mlif_data_config cfg;
    cfg.row = 1;
    cfg.col = 20;
    cfg.dtype = MLIF_DTYPE_INT16;
    cfg.order = MLIF_C_ORDER;

    mlifio_to_stdout(MLIF_STDIO_BIN, &cfg, a);

    mlifio_to_file(MLIF_FILE_BIN, "out.bin", &cfg, a);

    return 0;
}
