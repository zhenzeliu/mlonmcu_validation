#include "mlifio.h"

#define BUFFER_SIZE 4096
#define NPY_HEADER_SIZE 128

inline static int dtype2size(const mlif_data_config *config, size_t *size);
inline static int dtype2type(const mlif_data_config *config, char *type);
inline static int get_data_size(const mlif_data_config *config, size_t *size);

/**
 * @brief Interface writing 2-dimensional data to file (.npy or .bin format)
 * 
 * @param fmode Either MLIF_FILE_BIN or MLIF_FILE_NPY.
 * @param file_path Path including file name and suffix.
 * @param config Data configuration which contains datatype, shape and further informations.
 * @param data Data pointer.
 * @return MLIF_IO_STATUS Indicates execution status.
 */
MLIF_IO_STATUS mlifio_to_file(const MLIF_FILE_MODE fmode, const char *file_path, const mlif_data_config *config, const void *data)
{
    if ((config == NULL) || (data == NULL) || (strlen(file_path) == 0)) return MLIF_IO_ERROR;

    size_t size = 1;
    size_t dsize = 1;
    char type = 'V';
    dtype2size(config, &size);
    dtype2type(config, &type);
    dsize = size;
    get_data_size(config, &size);   // get how many bytes in a single input/output

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
            fprintf(fp, "{'descr': '<%c%zu', 'fortran_order': %s, 'shape': (%zu, %zu, ", type, dsize, order, config->nbatch, config->nsample / config->nbatch);
            for (size_t i = 0; i < config->ndim; i++)
                fprintf(fp, "%zu, ", config->shape[i]);
            fseek(fp, -2, SEEK_CUR);
            fprintf(fp, "), }");
            fprintf(fp, "%*s\n", (NPY_HEADER_SIZE - 1) - (int)ftell(fp), " ");
            fwrite(data, sizeof(int8_t), size * (config->nsample / config->nbatch), fp);     // write raw data
            fclose(fp);
        }
        else
        {
            fseek(fp, 0, SEEK_END);
            fwrite(data, sizeof(int8_t), size * (config->nsample / config->nbatch), fp);
            fclose(fp);
        }
    }
    else if (fmode == MLIF_FILE_BIN)
    {
        FILE *fp = NULL;
        fp = fopen(file_path, "ab+");
        fwrite(data, sizeof(int8_t), size * (config->nsample / config->nbatch), fp);
        fclose(fp);
    }
    else
    {
        return MLIF_IO_UNSUPPORTED;
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
 * @return MLIF_IO_STATUS Indicates execution status.
 */
MLIF_IO_STATUS mlifio_to_stdout(const MLIF_STDIO_MODE iomode, const mlif_data_config *config, const void *data, const size_t ibatch)
{
    if ((config == NULL) || (data == NULL)) return MLIF_IO_ERROR;

    size_t size = 1;
    dtype2size(config, &size);
    get_data_size(config, &size);

    if (iomode == MLIF_STDIO_PLAIN)
    {
        fprintf(stdout, "Batch[%zu]:\n", ibatch);
        for (size_t i = 0; i < (config->nsample / config->nbatch); i++)
        {
            fprintf(stdout, "Output[%zu]:", (config->nsample / config->nbatch) * ibatch + i);
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
        for (size_t i = 0; i < (config->nsample / config->nbatch); i++)
        {
            fprintf(stdout, "-?-");
            fwrite(data+i*size, sizeof(uint8_t), size, stdout);
            fprintf(stdout, "-!-");
            fflush(stdout);
        }
    }
    else
    {
        return MLIF_IO_UNSUPPORTED;
    }
    
    return MLIF_IO_SUCCESS;
}

/**
 * @brief Interface get 2-dimensional input data via file system (.npy or .bin)
 * 
 * @param fmode Either MLIF_FILE_BIN or MLIF_FILE_NPY.
 * @param file_path Path including file name and suffix.
 * @param config Data configuration which contains datatype, shape and further informations.
 * @param data Data pointer.
 * @param ibatch Batch indicator to show that the interface is playing with i-th batch
 * @return MLIF_IO_STATUS Indicates execution status.
 */
MLIF_IO_STATUS mlifio_from_file(const MLIF_FILE_MODE fmode, const char *file_path, mlif_data_config *config, void *data, const size_t ibatch)
{
    if ((config == NULL) || (data == NULL) || (strlen(file_path) == 0)) return MLIF_IO_ERROR;
    
    // get input file size
    struct stat stat_buffer = {};
    stat(file_path, &stat_buffer);
    
    const char mode[] = "rb";
    char dtype[3];

    if (fmode == MLIF_FILE_NPY)
    {
        if (stat_buffer.st_size <= NPY_HEADER_SIZE) return MLIF_IO_ERROR;

        int cnt;
        char tmp;
        char buffer[10] = {};
        size_t size = 1;
        size_t read_size = 0;
        size_t read_ptr = 0;
        short offset = 0;
        char header_length[2] = {};

        FILE *fp = NULL;
        fp = fopen(file_path, mode);
        if (fp == NULL) return MLIF_IO_FILE_NOT_EXIST;
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
        else if (!strcmp(dtype, "f4")) {config->dtype = MLIF_DTYPE_FLOAT32; size = 4;}
        else {config->dtype = MLIF_DTYPE_RAW; size = 1;}
        
        get_data_size(config, &size);
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
                config->nsample = atoi(buffer) * config->nbatch;
        }
        
        read_size = size * (config->nsample / config->nbatch);
        read_ptr = offset + 10 + ibatch * read_size;
        // check if this batch can be succcessfully read out
        if (read_ptr >= stat_buffer.st_size) return MLIF_IO_ERROR;
        if ((read_ptr + read_size) > stat_buffer.st_size)
        {
            read_size = stat_buffer.st_size - read_ptr;
        }

        fseek(fp, read_ptr, SEEK_SET);
        fread(data, sizeof(int8_t), read_size, fp);
        fclose(fp);
    }
    else if (fmode == MLIF_FILE_BIN)
    {
        size_t size = 1;
        size_t read_size = 0;
        size_t read_ptr = 0;
        dtype2size(config, &size);
        get_data_size(config, &size);
        FILE *fp = NULL;
        fp = fopen(file_path, mode);
        if (fp == NULL) return MLIF_IO_FILE_NOT_EXIST;

        read_size = size * (config->nsample / config->nbatch);
        read_ptr = ibatch * read_size;
        // check if this batch can be succcessfully read out
        if (read_ptr >= stat_buffer.st_size) return MLIF_IO_ERROR;
        if ((read_ptr + read_size) > stat_buffer.st_size)
        {
            read_size = stat_buffer.st_size - read_ptr;
        }

        fseek(fp, read_ptr, SEEK_SET);
        fread(data, sizeof(int8_t), read_size, fp);
        fclose(fp);
    }
    else
    {
        return MLIF_IO_UNSUPPORTED;
    }

    return MLIF_IO_SUCCESS;
}

/**
 * @brief Interface get 2-dimensional input data via stdin (plaintext or binary)
 * 
 * @param mode Either MLIF_STDIO_BIN or MLIF_STDIO_PLAIN.
 * @param config config Data configuration which contains datatype, shape and further informations.
 * @param data Data pointer.
 * @return MLIF_IO_STATUS Indicates execution status.
 */
MLIF_IO_STATUS mlifio_from_stdin(const MLIF_STDIO_MODE iomode, mlif_data_config *config, void *data)
{
    if ((config == NULL) || (data == NULL)) return MLIF_IO_ERROR;
    
    static char buffer[BUFFER_SIZE];
    char *token;
    int num = 0;
    int cnt = 0;
    int ptr = 0;
    size_t size = 1;
    dtype2size(config, &size);
    get_data_size(config, &size);
    // plain text in decimal should looks like:
    // 10,20,30,40,15,25,10,23,255,255,10
    // 13,15,12,98,22,33,95,69,0,0,122,243
    if (iomode == MLIF_STDIO_PLAIN)
    {
        memset(buffer, 0, sizeof(buffer));
        while (fgets(buffer, sizeof(buffer), stdin) != NULL)
        {
            if (strcmp(buffer, "\n") == 0) break;           // If a blank line or '\n' fed to stdin then quit
            ptr = 0;
            token = strtok(buffer, ",");
            while (token != NULL)
            {
                num = atoi(token);
                num = num < INT8_MIN ? INT8_MIN : num;
                num = num > INT8_MAX ? INT8_MAX : num;      // Limits input number into [-128, 127)
                ((int8_t *)data)[cnt * size + ptr] = (int8_t)num;
                token = strtok(NULL, ",");
                ptr++;
            }
            cnt++;
            if (cnt == config->nsample / config->nbatch) break; // Or if input number has already reach the limit then quit
        }
        fflush(stdin);
    }
    else if (iomode == MLIF_STDIO_BIN)
    {
        // read all input at once
        fread(data, sizeof(uint8_t), size * (config->nsample / config->nbatch), stdin);
        fflush(stdin);
    }
    else
    {
        return MLIF_IO_UNSUPPORTED;
    }

    return MLIF_IO_SUCCESS;
}

inline static int dtype2size(const mlif_data_config *config, size_t *size)
{
    if ((size == NULL) || (config == NULL)) return 1;
    switch (config->dtype)
    {
        case MLIF_DTYPE_INT8: *size = 1; break;
        case MLIF_DTYPE_INT16: *size = 2; break;
        case MLIF_DTYPE_INT32: *size = 4; break;
        case MLIF_DTYPE_UINT8: *size = 1; break;
        case MLIF_DTYPE_UINT16: *size = 2; break;
        case MLIF_DTYPE_UINT32: *size = 4; break;
        case MLIF_DTYPE_FLOAT32: *size = 4; break;
        case MLIF_DTYPE_RAW:
        default: *size = 1; break;
    }
    return 0;
}

inline static int dtype2type(const mlif_data_config *config, char *type)
{
    if ((type == NULL) || (config == NULL)) return 1;
    switch (config->dtype)
    {
        case MLIF_DTYPE_INT8: *type = 'i'; break;
        case MLIF_DTYPE_INT16: *type = 'i'; break;
        case MLIF_DTYPE_INT32: *type = 'i'; break;
        case MLIF_DTYPE_UINT8: *type = 'u'; break;
        case MLIF_DTYPE_UINT16: *type = 'u'; break;
        case MLIF_DTYPE_UINT32: *type = 'u'; break;
        case MLIF_DTYPE_FLOAT32: *type = 'f'; break;
        case MLIF_DTYPE_RAW:
        default: *type = 'V'; break;
    }
    return 0;
}

inline static int get_data_size(const mlif_data_config *config, size_t *size)
{
    for (size_t i = 0; i < config->ndim; i++)
    {
        *size *= config->shape[i];   // single input size in bytes
    }
    return 0;
}
