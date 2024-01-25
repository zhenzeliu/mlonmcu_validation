#include "mlifio.h"

#define BUFFER_SIZE 2048
#define NPY_HEADER_SIZE 128

/**
 * @brief Interface writing 2-dimensional data to file (.npy or .bin format)
 * 
 * @param fmode Either MLIF_FILE_BIN or MLIF_FILE_NPY.
 * @param file_path Path including file name and suffix to store.
 * @param config Data configuration which contains datatype, shape and further informations.
 * @param data Data pointer
 * @return MLIF_IO_STATUS Either MLIF_IO_ERROR or MLIF_IO_SUCCESS.
 */
MLIF_IO_STATUS mlifio_to_file(const mlif_file_mode fmode, const char *file_path, const mlif_data_config *config, const void *data)
{
    if ((config == NULL) || (data == NULL)) return MLIF_IO_ERROR;

    int size = 1;
    int dsize = 1;
    char type = 'V';
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
        default: type = 'V'; size = 1; break;
    }
    dsize = size;
    for (size_t i = 0; i < config->ndim; i++)
    {
        size *= config->shape[i];   // single input size in bytes
    }

    if (fmode == MLIF_FILE_NPY)
    {
        FILE *fp = NULL;
        fp = fopen(file_path, "rb+");
        if (fp == NULL)
        {
            fp = fopen(file_path, "wb+");
            const int8_t magic_string[8] = {0x93, 'N', 'U', 'M', 'P', 'Y', 0x01, 0x00};
            const uint16_t length = NPY_HEADER_SIZE - 10;
            char order[6] = "";
            if (config->order == MLIF_FORTRAN_ORDER)
                sprintf(order, "%s", "True");
            else
                sprintf(order, "%s", "False");
            // write header information to .npy file
            fwrite(magic_string, sizeof(int8_t), 8, fp);
            fwrite(&length, sizeof(int8_t), 2, fp);
            fprintf(fp, "{'descr': '<%c%d', 'fortran_order': %s, 'shape': (%zu, %zu, ", type, dsize, order, config->nbatch, config->nsample);
            for (size_t i = 0; i < config->ndim; i++)
                fprintf(fp, "%zu, ", config->shape[i]);
            fseek(fp, -2, SEEK_CUR);
            fprintf(fp, "), }");
            fprintf(fp, "%*s\n", (NPY_HEADER_SIZE - 1) - (int)ftell(fp), " ");
            fwrite(data, sizeof(int8_t), size * config->nsample, fp);     // write raw data
            fclose(fp);
        }
        else
        {
            fseek(fp, 0, SEEK_END);
            fwrite(data, sizeof(int8_t), size * config->nsample, fp);
            fclose(fp);
        }
        
    }
    else if (fmode == MLIF_FILE_BIN)
    {
        FILE *fp = NULL;
        fp = fopen(file_path, "ab+");
        fwrite(data, sizeof(int8_t), size * config->nsample, fp);
        fclose(fp);
    }
    else
    {
        return MLIF_IO_ERROR;
    }
    
    return MLIF_IO_SUCCESS;
}

/**
 * @brief Interface output 2-dimensional data via stdout (plaintext or binary)
 * 
 * @param mode Either MLIF_STDIO_BIN or MLIF_STDIO_PLAIN.
 * @param config Data configuration which contains datatype, shape and further informations.
 * @param data Data pointer.
 * @param ibatch Batch indicator to show that the interface is playing with i-th batch
 * @return MLIF_IO_STATUS Either MLIF_IO_ERROR or MLIF_IO_SUCCESS.
 */
MLIF_IO_STATUS mlifio_to_stdout(const mlif_stdio_mode iomode, const mlif_data_config *config, const void *data, const size_t ibatch)
{
    if ((config == NULL) || (data == NULL)) return MLIF_IO_ERROR;

    size_t size = 1;
    switch (config->dtype)
    {
        case MLIF_DTYPE_INT8: size = 1; break;
        case MLIF_DTYPE_INT16: size = 2; break;
        case MLIF_DTYPE_INT32: size = 4; break;
        case MLIF_DTYPE_UINT8: size = 1; break;
        case MLIF_DTYPE_UINT16: size = 2; break;
        case MLIF_DTYPE_UINT32: size = 4; break;
        case MLIF_DTYPE_FLOAT: size = 4; break;
        case MLIF_DTYPE_RAW:
        default: size = 1; break;
    }
    for (size_t i = 0; i < config->ndim; i++)
    {
        size *= config->shape[i];   // single input size in bytes
    }

    if (iomode == MLIF_STDIO_PLAIN)
    {
        fprintf(stdout, "Batch[%zu]:\n", ibatch);
        for (size_t i = 0; i < config->nsample; i++)
        {
            fprintf(stdout, "Output[%zu]:", config->nsample * ibatch + i);
            for (size_t j = 0; j < size; j++)
            {
                fprintf(stdout, " 0x%02x", ((uint8_t *)data)[i*size+j]);
            }
            fprintf(stdout, "\n");
            fflush(stdout);
        }
    }
    else if (iomode == MLIF_STDIO_BIN)
    {
        for (size_t i = 0; i < config->nsample; i++)
        {
            fprintf(stdout, "-?-");
            fwrite(data+i*size, sizeof(uint8_t), size, stdout);
            fprintf(stdout, "-!-");
            fflush(stdout);
        }
    }
    else
    {
        return MLIF_IO_ERROR;
    }
    
    return MLIF_IO_SUCCESS;
}

/**
 * @brief Interface get 2-dimensional input data via file system (.npy or .bin)
 * 
 * @param fmode Either MLIF_FILE_BIN or MLIF_FILE_NPY.
 * @param file_path Path including file name and suffix to store.
 * @param config Data configuration which contains datatype, shape and further informations.
 * @param data Data pointer
 * @param idx Index of inputs
 * @return MLIF_IO_STATUS Either MLIF_IO_ERROR or MLIF_IO_SUCCESS.
 */
MLIF_IO_STATUS mlifio_from_file(const mlif_file_mode fmode, const char *file_path, mlif_data_config *config, void *data, const size_t idx)
{
    const char mode[] = "rb";
    char dtype[3];

    if (fmode == MLIF_FILE_NPY)
    {
        int cnt;
        char tmp;
        char buffer[10] = {};
        size_t size = 1;
        short offset = 0;
        char header_length[2] = {};

        FILE *fp = NULL;
        fp = fopen(file_path, mode);
        if (fp == NULL) return MLIF_IO_ERROR;
        fseek(fp, 8, SEEK_SET);         // jump over the dummy bytes
        fread(header_length, sizeof(char), 2, fp);
        offset = header_length[0] + 256 * header_length[1];

        fseek(fp, 12, SEEK_CUR);
        fread(dtype, sizeof(char), 2, fp);
        if (!strcmp(dtype, "i1")) {config->dtype = MLIF_DTYPE_INT8; size = 1;}
        else if (!strcmp(dtype, "i2")) {config->dtype = MLIF_DTYPE_INT16; size = 2;}
        else if (!strcmp(dtype, "i4")) {config->dtype = MLIF_DTYPE_INT32; size = 4;}
        else if (!strcmp(dtype, "u1")) {config->dtype = MLIF_DTYPE_UINT8; size = 1;}
        else if (!strcmp(dtype, "u2")) {config->dtype = MLIF_DTYPE_UINT16; size = 2;}
        else if (!strcmp(dtype, "u4")) {config->dtype = MLIF_DTYPE_UINT32; size = 4;}
        else if (!strcmp(dtype, "f4")) {config->dtype = MLIF_DTYPE_FLOAT; size = 4;}
        else {config->dtype = MLIF_DTYPE_RAW; size = 1;}
        
        for (size_t i = 0; i < config->ndim; i++)
        {
            size *= config->shape[i];   // single input size in bytes
        }
        fseek(fp, 20, SEEK_CUR);
        if (fgetc(fp) == 'F') config->order = MLIF_C_ORDER;
        else config->order = MLIF_FORTRAN_ORDER;
        fseek(fp, 16, SEEK_CUR);
        // get number of batch and number of inputs per batch
        for (size_t i = 0; i < 2; i++)
        {
            cnt = 0;
            memset(buffer, 0, sizeof(buffer));
            tmp = fgetc(fp);
            while (tmp != ',')
            {
                if ((tmp >= '0') && (tmp <= '9'))
                {
                    buffer[cnt] = tmp;
                    cnt++;
                }
                tmp = fgetc(fp);
            }
            fseek(fp, 1, SEEK_CUR);
            if (!i)
                config->nbatch = atoi(buffer);
            else
                config->nsample = atoi(buffer);
        }
        fseek(fp, offset + 10 + idx * size, SEEK_SET);
        fread(data, sizeof(int8_t), size * config->nsample, fp);
        fclose(fp);
    }
    else if (fmode == MLIF_FILE_BIN)
    {
        size_t size = 1;
        switch (config->dtype)
        {
            case MLIF_DTYPE_INT8: size = 1; break;
            case MLIF_DTYPE_INT16: size = 2; break;
            case MLIF_DTYPE_INT32: size = 4; break;
            case MLIF_DTYPE_UINT8: size = 1; break;
            case MLIF_DTYPE_UINT16: size = 2; break;
            case MLIF_DTYPE_UINT32: size = 4; break;
            case MLIF_DTYPE_FLOAT: size = 4; break;
            case MLIF_DTYPE_RAW:
            default: size = 1; break;
        }
        for (size_t i = 0; i < config->ndim; i++)
        {
            size *= config->shape[i];   // single input size in bytes
        }
        FILE *fp = NULL;
        fp = fopen(file_path, mode);
        if (fp == NULL) return MLIF_IO_ERROR;
        fseek(fp, 0, SEEK_END);
        config->nsample = ftell(fp) / (size * config->nbatch);
        fseek(fp, idx * size, SEEK_SET);
        fread(data, sizeof(int8_t), size, fp);
    }
    else
    {
        return MLIF_IO_ERROR;
    }
    return MLIF_IO_SUCCESS;
}

/**
 * @brief Interface get 2-dimensional input data via stdin (plaintext or binary)
 * 
 * @param mode Either MLIF_STDIO_BIN or MLIF_STDIO_PLAIN.
 * @param config config Data configuration which contains datatype, shape and further informations.
 * @param data Data pointer.
 * @return MLIF_IO_STATUS Either MLIF_IO_ERROR or MLIF_IO_SUCCESS.
 */
MLIF_IO_STATUS mlifio_from_stdin(const mlif_stdio_mode iomode, mlif_data_config *config, void *data)
{
    char buffer[BUFFER_SIZE];
    char *token;
    int cnt = 0;
    int ptr = 0;
    size_t col = config->col;
    // plain text should looks like:
    // 10,20,30,40,15,25,10,23,255,255,10
    // 13,15,12,98,22,33,95,69,0,0,122,243
    if (iomode == MLIF_STDIO_PLAIN)
    {
        while (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            if (strcmp(buffer, "\n") == 0) break;
            ptr = 0;
            token = strtok(buffer, ",");
            while (token != NULL)
            {
                ((char *)data)[cnt*col+ptr] = (char)atoi(token);
                token = strtok(NULL, ",");
                ptr++;
            }
            cnt++;
        }
        config->row = (cnt > 0) ? cnt : 1;
        fflush(stdin);
    }
    else if (iomode == MLIF_STDIO_BIN)
    {
        // can only process one input each invokation
        // for multi-input need more information: end of input...
        size_t length = 0;
        length = fread(buffer, sizeof(char), col, stdin);
        memcpy(data, buffer, length);
        fflush(stdin);
    }
    else
    {
        return MLIF_IO_ERROR;
    }
    return MLIF_IO_SUCCESS;
}
