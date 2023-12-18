#ifndef _MLIF_H_
#define _MLIF_H_

#include <stdio.h>
#include <stddef.h>

typedef enum status
{
    MLIF_PROCESS_OK,
    MLIF_MISSMATCH,
    MLIF_MODEL_OUTPUT_NULL,
}MLIF_PROCESS_STATUS;

MLIF_PROCESS_STATUS mlif_porcess_input(void *model_input_ptr, size_t model_input_size);
MLIF_PROCESS_STATUS mlif_porcess_output(void *model_output_ptr, size_t model_output_size);

#endif
