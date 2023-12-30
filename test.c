#include <stdio.h>
#include <inttypes.h>

#include "mlifio.h"

#define SIZE 25

int main(void)
{
    uint16_t a[SIZE] = {0xff, 0xaa, 0xbb, 0xcc, 0xdd,
                        0xee, 0x17, 0x18, 0x19, 0x20,
                        0x64, 0x78, 0x08, 0x07, 0x06,
                        0x05, 0x04, 0x03, 0x02, 0x01,
                        0x88, 0x88, 0x88, 0x88, 0x88};

    mlif_data_config cfg;
    cfg.row = 5;
    cfg.col = 5;
    cfg.dtype = MLIF_DTYPE_UINT16;
    cfg.order = MLIF_FORTRAN_ORDER;

    // mlifio_to_npy("./out.npy", &cfg, a);
    mlifio_to_stdout(&cfg, a);

    return 0;
}
