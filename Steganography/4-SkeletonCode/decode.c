#include <stdio.h>
#include <string.h>
#include "decode.h"
#include "types.h"
#include "common.h"

//My password is SECRET ;)
/* Function to read and validate command-line arguments */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{

if(argv[2] != NULL || argv[3] != NULL)
{
    if (argv[2] == NULL)
    {
        return e_failure;
    }
    if (strstr(argv[2], ".bmp") == NULL)
    {
        return e_failure;
    }
    decInfo->src_image_fname = argv[2];

    if (argv[3] != NULL)
    {
        if (strchr(argv[3], '.') == NULL)
        {
            strncpy(decInfo->dest_image_fname, argv[3], sizeof(decInfo->dest_image_fname) - 1);
        } 
        else 
        {
            strcpy(decInfo->dest_image_fname, "output");
        }
    }
    else 
    {
        strcpy(decInfo->dest_image_fname, "output");
    }    
}

    return e_success;
}

/* Function to perform decoding */
Status do_decoding(DecodeInfo *decInfo) 
{
    decInfo->fptr_src_image = fopen(decInfo->src_image_fname, "r");
    if (!decInfo->fptr_src_image) 
    {
        perror("ERROR: Unable to open source BMP file");
        return e_failure;
    }
    
    if (skip_bmp_header(decInfo->fptr_src_image) != e_success) 
    {
        return e_failure;
    }

    char user_magic_string[10];
    printf("Enter expected magic string: ");
    scanf("%9s", user_magic_string);

    if (decode_magic_string(decInfo) != e_success) 
    {
        return e_failure;
    }
    if (strcmp(decInfo->magic_str,user_magic_string) != 0) 
    {
        printf("ERROR: Magic string mismatch?.\n");
        return e_failure;
    }
    if (decode_secret_file_extn_size(&(decInfo->extn_size), decInfo) != e_success) 
    {
        return e_failure;
    }
    if (decode_secret_file_extn(decInfo->file_extn, decInfo) != e_success) 
    {
        return e_failure;
    }
    if(ecode_secret_file_extn_concatinate(decInfo) != e_success)
    {
        return e_failure;
    }
    
    decInfo->fptr_dest_image = fopen(decInfo->dest_image_fname, "w");
    if (!decInfo->fptr_dest_image) 
    {
        perror("ERROR: Unable to create destination file");
        fclose(decInfo->fptr_src_image);
        return e_failure;
    }

    if (decode_secret_file_size(&(decInfo->extn_size), decInfo) != e_success) 
    {
        return e_failure;
    }
    if (decode_secret_file_data(decInfo) != e_success) 
    {
        return e_failure;
    }
    
    fclose(decInfo->fptr_src_image);
    fclose(decInfo->fptr_dest_image);
    
    return e_success;
}

/* Function to skip the bmp header */
Status skip_bmp_header(FILE *fptr) 
{
    if (fseek(fptr, 54, SEEK_SET) != 0) 
    {
        return e_failure;
    }
    return e_success;
}

/* Function to decode magic string */
Status decode_magic_string(DecodeInfo *decInfo ) 
{
    char buffer[8];
    for (int i = 0; i < 2; i++) 
    {
        if (fread(buffer, 1, 8, decInfo->fptr_src_image) != 8) 
        {
            return e_failure;
        }
        decInfo->magic_str[i] = decode_byte_from_lsb(buffer);
    }
    decInfo->magic_str[2] = '\0';

    return e_success;
}

/* Function to decode a byte from LSBs */
char decode_byte_from_lsb(char *buffer) 
{
    char decoded_byte = 0; 
    // int index=0;
    for (int i = 0; i < 8; i++) 
    {
        decoded_byte |= ((buffer[i] & 1) << i);
    }
    return decoded_byte;
}

/* Function to decode the size of the secret file extension */
Status decode_secret_file_extn_size(int *extn_size, DecodeInfo *decInfo) 
{
    char buffer[32];
    if (fread(buffer, 1, 32, decInfo->fptr_src_image) != 32) 
    {
        return e_failure;
    }

    *extn_size = 0;
    for (int i = 0; i < 32; i++) 
    {
        *extn_size |= ((buffer[i] & 1) << i);
    }

    return e_success;
}

/* Function to decode the file extension */
Status decode_secret_file_extn(char *file_extn, DecodeInfo *decInfo) 
{
    char buffer[8];
    for (int i = 0; i < decInfo->extn_size; i++) 
    {
        if (fread(buffer, 1, 8, decInfo->fptr_src_image) != 8) 
        {
            return e_failure;
        }
        file_extn[i] = decode_byte_from_lsb(buffer);
    }
    file_extn[decInfo->extn_size] = '\0';

    return e_success;
}

/*Function to concantinate the extention file*/
Status ecode_secret_file_extn_concatinate(DecodeInfo *decInfo)
{
    strcat(decInfo->dest_image_fname, decInfo->file_extn);
    return e_success;
}

/* Function to decode the secret file size */
Status decode_secret_file_size(int *file_size, DecodeInfo *decInfo) 
{
    char buffer[32];
    if (fread(buffer, 1, 32, decInfo->fptr_src_image) != 32) 
    {
        return e_failure;
    }
    *file_size = 0;
    for (int i = 0; i < 32; i++) 
    {
        *file_size |= ((buffer[i] & 1) << i);
    }

    return e_success;
}

/* Function to decode the secret file data */
Status decode_secret_file_data(DecodeInfo *decInfo) 
{
    char buffer[8];
    for (int i = 0; i < decInfo->extn_size; i++) 
    {
        if (fread(buffer, 1, 8, decInfo->fptr_src_image) != 8) 
        {
            return e_failure;
        }
        char decoded_char = decode_byte_from_lsb(buffer);
        fputc(decoded_char, decInfo->fptr_dest_image);
    }
    return e_success;
}
