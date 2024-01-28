#include <stdio.h>
#include <inttypes.h>
#include "mlifio.h"

int main(void)
{
    mlif_data_config cfg;
    int8_t output_batch_1[9] = {0x41, 0x42, 0x43,
                                0x44, 0x45, 0x46,
                                0x47, 0x48, 0x49};

    int8_t output_batch_2[9] = {0x01, 0x02, 0x03,
                                0xff, 0xff, 0xff,
                                0xff, 0xff, 0xff};

    int8_t output_batch_3[9] = {0x00, 0x11, 0xaa,
                                0x00, 0x11, 0xaa,
                                0x00, 0x11, 0xaa};
    
    int8_t *output_batches[] = {output_batch_1, output_batch_2, output_batch_3};      // test data has 3 batches, each batch has 3 inputs

    size_t shape[2] = {1, 3};
    cfg.nbatch = 3;                 // number of batches
    cfg.nsample = 9;                // number of total inputs/outputs
    cfg.ndim = 2;                   // dimension of a single input, in this case is {1, 3}
    cfg.shape = shape;              // data shape for each input is 1x3
    cfg.dtype = MLIF_DTYPE_INT8;    // dtype int8
    cfg.order = MLIF_C_ORDER;       // row first

    // test on output via .bin file/stdout
    for (size_t i = 0; i < (cfg.nsample / cfg.nbatch); i++)
    {
        mlifio_to_file(MLIF_FILE_BIN, "out.bin", &cfg, output_batches[i]);
        mlifio_to_stdout(MLIF_STDIO_BIN, &cfg, output_batches[i], i);
    }
    
    // test on output via .npy file/stdout
    for (size_t i = 0; i < (cfg.nsample / cfg.nbatch); i++)
    {
        mlifio_to_file(MLIF_FILE_NPY, "out.npy", &cfg, output_batches[i]);
        mlifio_to_stdout(MLIF_STDIO_PLAIN, &cfg, output_batches[i], i);
    }

    int8_t input_batch_1[9] = {};
    int8_t input_batch_2[9] = {};
    int8_t input_batch_3[9] = {};

    int8_t *input_batches[] = {input_batch_1, input_batch_2, input_batch_3};

    // test on input via .npy/.bin file
    for (size_t i = 0; i < (cfg.nsample / cfg.nbatch); i++)
    {
        // mlifio_from_file(MLIF_FILE_BIN, "out.bin", &cfg, input_batches[i], i);
        mlifio_from_file(MLIF_FILE_NPY, "out.npy", &cfg, input_batches[i], i);
        for (size_t j = 0; j < sizeof(input_batch_1); j++)
        {
            printf("0x%02hhx ", input_batches[i][j]);
        }
        printf("\n");
    }

    // test on input via stdin plain or bin
    for (size_t i = 0; i < (cfg.nsample / cfg.nbatch); i++)
    {
        // mlifio_from_stdin(MLIF_STDIO_PLAIN, &cfg, input_batches[i]);
        mlifio_from_stdin(MLIF_STDIO_BIN, &cfg, input_batches[i]);
        printf("Input batch[%zu]: ", i);
        for (size_t j = 0; j < sizeof(input_batch_1); j++)
        {
            printf("%d ", input_batches[i][j]);
        }
        printf("\n");
    }
    
    return 0;
}
