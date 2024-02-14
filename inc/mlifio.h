#ifndef _MLIFIO_H_
#define _MLIFIO_H_

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>

typedef enum
{
    MLIF_IO_SUCCESS,
    MLIF_IO_UNSUPPORTED,
    MLIF_IO_FILE_NOT_EXIST,
    MLIF_IO_ERROR
}MLIF_IO_STATUS;

typedef enum
{
    MLIF_DTYPE_INT8,
    MLIF_DTYPE_INT16,
    MLIF_DTYPE_INT32,
    MLIF_DTYPE_UINT8,
    MLIF_DTYPE_UINT16,
    MLIF_DTYPE_UINT32,
    MLIF_DTYPE_FLOAT32,
    MLIF_DTYPE_RAW
}MLIF_DATATYPE;

// data storage order
typedef enum
{
    MLIF_C_ORDER,       // row first
    MLIF_FORTRAN_ORDER  // column first
}MLIF_DATA_ORDER;

// data configuration
typedef struct
{
    size_t nbatch;
    size_t nsample;
    size_t ndim;
    size_t *shape;
    MLIF_DATATYPE dtype;
    MLIF_DATA_ORDER order;
}mlif_data_config;

typedef enum
{
    MLIF_STDIO_BIN,
    MLIF_STDIO_PLAIN
}MLIF_STDIO_MODE;

typedef enum
{
    MLIF_FILE_NPY,
    MLIF_FILE_BIN
}MLIF_FILE_MODE;

MLIF_IO_STATUS mlifio_to_file(const MLIF_FILE_MODE fmode, const char *npy_file_path, const mlif_data_config *config, const void *data);
MLIF_IO_STATUS mlifio_to_stdout(const MLIF_STDIO_MODE iomode, const mlif_data_config *config, const void *data, const size_t ibatch);
MLIF_IO_STATUS mlifio_from_file(const MLIF_FILE_MODE fmode, const char *file_path, mlif_data_config *config, void *data, const size_t ibatch);
MLIF_IO_STATUS mlifio_from_stdin(const MLIF_STDIO_MODE iomode, mlif_data_config *config, void *data);

#endif
