#include <stdio.h>
#include <inttypes.h>

#include "mlifio.h"

#define SIZE 20

int main(void)
{
    mlif_data_config cfg;
    char batch_1[9] = {0x41, 0x42, 0x43,
                        0x44, 0x45, 0x46,
                        0x47, 0x48, 0x49};

    char batch_2[9] = {0xff, 0xff, 0xff,
                        0xff, 0xff, 0xff,
                        0xff, 0xff, 0xff};

    char batch_3[9] = {0x00, 0x11, 0xaa,
                        0x00, 0x11, 0xaa,
                        0x00, 0x11, 0xaa};
    
    char *batches[] = {batch_1, batch_2, batch_3};      // test data has 3 batches, each batch has 3 inputs

    size_t shape[2] = {1, 3};
    cfg.nbatch = 3;                 // number of batches
    cfg.nsample = 3;                 // number of inputs/outputs for each batch
    cfg.ndim = 2;                   // dimension of a single input, in this case is {1, 3}
    cfg.shape = shape;              // data shape for each input is 1x3
    cfg.dtype = MLIF_DTYPE_INT8;    // dtype int8
    cfg.order = MLIF_C_ORDER;       // row first

    // test on output via .bin file
    for (size_t i = 0; i < cfg.nbatch; i++)
    {
        // mlifio_to_file(MLIF_FILE_BIN, "out.bin", &cfg, batches[i]);
        mlifio_to_stdout(MLIF_STDIO_BIN, &cfg, batches[i], i);
    }
    
    // test on output via .npy file
    for (size_t i = 0; i < cfg.nbatch; i++)
    {
        // mlifio_to_file(MLIF_FILE_NPY, "out.npy", &cfg, batches[i]);
        mlifio_to_stdout(MLIF_STDIO_PLAIN, &cfg, batches[i], i);
    }
    return 0;
}
