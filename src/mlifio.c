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
            fprintf(fp, "{'descr': '<%c%d', 'fortran_order': %s, 'shape': (%zu, %zu, ", type, dsize, order, config->nbatch, config->ninput);
            for (size_t i = 0; i < config->ndim; i++)
                fprintf(fp, "%zu, ", config->shape[i]);
            fseek(fp, -2, SEEK_CUR);
            fprintf(fp, "), }");
            fprintf(fp, "%*s\n", (NPY_HEADER_SIZE - 1) - (int)ftell(fp), " ");
            fwrite(data, sizeof(int8_t), size * config->ninput, fp);     // write raw data
            fclose(fp);
        }
        else
        {
            fseek(fp, 0, SEEK_END);
            fwrite(data, sizeof(int8_t), size * config->ninput, fp);
            fclose(fp);
        }
        
    }
    else if (fmode == MLIF_FILE_BIN)
    {
        FILE *fp = NULL;
        fp = fopen(file_path, "ab+");
        fwrite(data, sizeof(int8_t), size * config->ninput / config->nbatch, fp);
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
 * @return MLIF_IO_STATUS Either MLIF_IO_ERROR or MLIF_IO_SUCCESS.
 */
MLIF_IO_STATUS mlifio_to_stdout(const mlif_stdio_mode mode, const mlif_data_config *config, const void *data)
{
    if ((config == NULL) || (data == NULL)) return MLIF_IO_ERROR;
    
    size_t row = config->row;
    size_t col = config->col;

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

    if (mode == MLIF_STDIO_PLAIN)
    {
        char header[20] = "";
        char num_char[10] = "";
        const char tail[] = "\n";
        for (size_t i = 0; i < row; i++)
        {
            sprintf(header, "Output[%zu]:", i);
            fwrite(header, sizeof(char), strlen(header), stdout);
            for (size_t j = 0; j < col*size; j++)
            {
                sprintf(num_char, " 0x%02x", ((unsigned char *)data)[col*size*i+j]);
                fwrite(num_char, sizeof(char), strlen(num_char), stdout);
            }
            fwrite(tail, sizeof(char), 1, stdout);
            fflush(stdout);
        }
    }
    else if (mode == MLIF_STDIO_BIN)
    {
        fwrite(data, sizeof(char), row * col * size, stdout);
        fflush(stdout);
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
 * @return MLIF_IO_STATUS Either MLIF_IO_ERROR or MLIF_IO_SUCCESS.
 */
MLIF_IO_STATUS mlifio_from_file(const mlif_file_mode fmode, const char *file_path, mlif_data_config *config, void *data)
{
    const char mode[] = "rb";
    char dtype[3];

    if (fmode == MLIF_FILE_NPY)
    {
        char tmp;
        char buffer[10] = {};
        size_t size = 1;
        size_t row = 0;
        size_t col = 0;
        int cnt = 0;
        short offset = 0;
        char header_length[2] = {};
        FILE *fp = NULL;
        fp = fopen(file_path, mode);
        if (fp == NULL) return MLIF_IO_ERROR;
        fseek(fp, 8, SEEK_SET);
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

        fseek(fp, 20, SEEK_CUR);
        if (fgetc(fp) == 'F') config->order = MLIF_C_ORDER;
        else config->order = MLIF_FORTRAN_ORDER;

        fseek(fp, 16, SEEK_CUR);
        tmp = fgetc(fp);
        while (tmp != ',')
        {
            buffer[cnt] = tmp;
            tmp = fgetc(fp);
            cnt++;
        }
        row = atoi(buffer);

        fseek(fp, 1, SEEK_CUR);
        cnt = 0;
        memset(buffer, 0, sizeof(buffer));
        tmp = fgetc(fp);
        while (tmp != ')')
        {
            buffer[cnt] = tmp;
            tmp = fgetc(fp);
            cnt++;
        }
        col = atoi(buffer);
        config->row = row;
        config->col = col;

        fseek(fp, offset+10, SEEK_SET);
        fread(data, sizeof(char), row*col*size, fp);
        fclose(fp);
    }
    else if (fmode == MLIF_FILE_BIN)
    {
        FILE *fp = NULL;
        fp = fopen(file_path, mode);
        if (fp == NULL) return MLIF_IO_ERROR;
        fread(data, sizeof(char), config->col, fp);
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
MLIF_IO_STATUS mlifio_from_stdin(const mlif_stdio_mode mode, mlif_data_config *config, void *data)
{
    char buffer[BUFFER_SIZE];
    char *token;
    int cnt = 0;
    int ptr = 0;
    size_t col = config->col;
    // plain text should looks like:
    // 10,20,30,40,15,25,10,23,255,255,10
    // 13,15,12,98,22,33,95,69,0,0,122,243
    if (mode == MLIF_STDIO_PLAIN)
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
    else if (mode == MLIF_STDIO_BIN)
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
