#include "mlif.h"

MLIF_PROCESS_STATUS mlif_porcess_input(void *model_input_ptr, size_t model_input_size)
{
    return MLIF_PROCESS_OK;
}

MLIF_PROCESS_STATUS mlif_porcess_output(const void *model_output_ptr, size_t model_output_size)
{
    // check model outputs pointer and size
    if ((model_output_ptr == NULL) || (model_output_size == 0))
    {
        printf("ERROR - Model outputs NULL\n");
        return MLIF_MODEL_OUTPUT_NULL;
    }

    printf("Model outputs: ");
    for (size_t i = 0; i < model_output_size; i++)
    {
        printf("0x%02x ", ((uint8_t *)model_output_ptr)[i]);
    }

    printf("\n");

    return MLIF_PROCESS_OK;
}
