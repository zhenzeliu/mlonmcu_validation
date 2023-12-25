#include "mlif.h"

MLIF_PROCESS_STATUS mlif_porcess_input(void *model_input_ptr, size_t model_input_size)
{
    return MLIF_PROCESS_SUCCESS;
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

    return MLIF_PROCESS_SUCCESS;
}

MLIF_IO_STATUS mlif_io_write_npy(const char *npy_file_path, const mlif_data_config *config, const void *data)
{
    const char mode[] = "wb+";
    const char magic_string[6] = {0x93, 'N', 'U', 'M', 'P', 'Y'};
    const char major_version[1] = {0x01};
    const char minor_version[1] = {0x00};
    const char length[2] = {0x76, 0x00};

    // const uint8_t header[] = "{'descr': '<i1', 'fortran_order': False, 'shape': (2, 3), }";
    char type = 'V';
    int size = 1;
    switch (config->dtype)
    {
        case MLIF_DTYPE_INT8: type = 'i'; size = 1; break;
        case MLIF_DTYPE_INT16: type = 'i'; size = 2; break;
        case MLIF_DTYPE_INT32: type = 'i'; size = 4; break;
        case MLIF_DTYPE_UINT8: type = 'u'; size = 1; break;
        case MLIF_DTYPE_UINT16: type = 'u'; size = 2; break;
        case MLIF_DTYPE_UINT32: type = 'u'; size = 4; break;
        case MLIF_DTYPE_FLOAT: type = 'f'; size = 4; break;
        case MLIF_DTYPE_RAW:
        default: type = 'V'; size = '1'; break;
    }
    char order[6] = "";
    if (config->order == MLIF_FORTRAN_ORDER)
        sprintf(order, "%s", "True");
    else
        sprintf(order, "%s", "False");
    
    char buffer[128] = "";
    char header[128] = "";
    sprintf(buffer, "{'descr': '<%c%d', 'fortran_order': %s, 'shape': (%lu, %lu), }", type, size, order, config->row, config->col);
    sprintf(header, "%-*s", 118, buffer);
    header[117] = '\n';

    // write header information to .npy file
    FILE *file_ptr = NULL;
    file_ptr = fopen(npy_file_path, mode);
    if (file_ptr == NULL) return MLIF_IO_ERROR;
    fwrite(magic_string, sizeof(char), 6, file_ptr);
    fwrite(major_version, sizeof(char), 1, file_ptr);
    fwrite(minor_version, sizeof(char), 1, file_ptr);
    fwrite(length, sizeof(char), 2, file_ptr);
    fwrite(header, sizeof(char), 118, file_ptr);
    fwrite(data, sizeof(char), (size * config->row * config->col), file_ptr);
    fclose(file_ptr);

    return MLIF_IO_SUCCESS;
}
