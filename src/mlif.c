#include "mlif.h"

MLIF_PROCESS_STATUS mlif_porcess_input(void *model_input_ptr, size_t model_input_size)
{
    return MLIF_PROCESS_OK;
}

MLIF_PROCESS_STATUS mlif_porcess_output(void *model_output_ptr, size_t model_output_size)
{
    // check model outputs
    if (model_output_ptr == NULL)
    {
        printf("ERROR - Model outputs NULL\n");
        return MLIF_MODEL_OUTPUT_NULL;
    }

    return MLIF_PROCESS_OK;
}
