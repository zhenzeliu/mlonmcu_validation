#ifndef _MLIFIO_H_
#define _MLIFIO_H_

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <inttypes.h>

typedef enum process_status
{
    MLIF_PROCESS_SUCCESS,
    MLIF_MISSMATCH,
    MLIF_MODEL_OUTPUT_NULL
}MLIF_PROCESS_STATUS;

typedef enum io_status
{
    MLIF_IO_SUCCESS,
    MLIF_IO_ERROR
}MLIF_IO_STATUS;

typedef enum datatype
{
    MLIF_DTYPE_INT8,
    MLIF_DTYPE_INT16,
    MLIF_DTYPE_INT32,
    MLIF_DTYPE_UINT8,
    MLIF_DTYPE_UINT16,
    MLIF_DTYPE_UINT32,
    MLIF_DTYPE_FLOAT,
    MLIF_DTYPE_RAW
}MLIF_DATATYPE;

// data storage order
typedef enum data_order
{
    MLIF_C_ORDER,       // row first
    MLIF_FORTRAN_ORDER  // column first
}MLIF_DATA_ORDER;

// data configuration
typedef struct data_config
{
    size_t row;
    size_t col;
    MLIF_DATATYPE dtype;
    MLIF_DATA_ORDER order;
}mlif_data_config;

MLIF_IO_STATUS mlifio_to_npy(const char *npy_file_path, const mlif_data_config *config, const void *data);
MLIF_IO_STATUS mlifio_to_stdout(const mlif_data_config *config, const void *data);
MLIF_IO_STATUS mlifio_to_uart(const mlif_data_config *config, const void *data);

#endif