#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "encode.h"
#include "types.h"
#include "common.h"


uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}
// Get the size of the file
uint get_file_size(FILE *fptr)
{
    fseek(fptr,0,SEEK_END);
    uint size=ftell(fptr);
    rewind(fptr);
    return size;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo -> fptr_src_image = fopen(encInfo->src_image_fname, "r");
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}
/* Function to read and validate encode arguments */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo) 
{
    if (argv[2] == NULL || argv[3] == NULL) 
    {    
        return e_failure;
    }

    if (strstr(argv[2], ".bmp") == NULL) 
    {
        return e_failure;
    }

    encInfo->src_image_fname = argv[2];
    encInfo->secret_fname = argv[3];

    /* Check output file  */
    if (argv[4] == NULL) 
    {
        strcpy(encInfo->stego_image_fname, "stego.bmp");
    } 
    else 
    {
        if (strstr(argv[4], ".bmp") == NULL) 
        {
            return e_failure;
        }
        strcpy(encInfo->stego_image_fname, argv[4]);
    }

    char *str=strchr(argv[3] , '.');
    if((strchr(argv[3] , '.')==NULL))
    {
        return e_failure;
    }
    strcpy(encInfo->extn_secret_file,str);
    //encInfo->secret_fname = argv[3];

    return e_success;
}

/* Function to check  image has enough space to hide data */
Status check_capacity(EncodeInfo *encInfo) 
{
    /* Get the image size */
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);

    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

     /*finding extended length(.txt)*/
    strcpy((encInfo->extn_secret_file),strchr((encInfo->secret_fname),'.'));

    /*Finding total required esimated size*/
    int required_estimated_size = (strlen(MAGIC_STRING) * 8) + 32 + (strlen(encInfo->extn_secret_file) * 8) + 32 + (encInfo->size_secret_file * 8) + 54;
    printf("Total Estimated size: %d\n",required_estimated_size);

    if (encInfo->image_capacity >= required_estimated_size) 
    {
        return e_success;
    } 
    else 
    {
        return e_failure;
    }
}

Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    fseek(fptr_src_image, 0, SEEK_SET);
    char header[54];
    /*read from source image to header*/
    fread(header, 54, 1, fptr_src_image);
    /*write to header to destination image*/
    fwrite(header, 54, 1, fptr_dest_image);
    return e_success;
}

/* Function to encode a string into LSB of BMP */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo) 
{
    for (int i = 0; i < strlen(magic_string); i++) 
    {
        char buffer[8];
        /*read source image to buffer*/
        fread(buffer, 1, 8, encInfo->fptr_src_image);
        /*call byte to lsb function*/
        encode_byte_to_lsb(magic_string[i], buffer);
        /*write buffer to stego image*/
        fwrite(buffer, 1, 8, encInfo->fptr_stego_image);
    }
    return e_success;
}

/* Function to encode integer into LSB */
Status encode_int_to_lsb(int data, FILE *fptr_src, FILE *fptr_dest) 
{
    char buffer[32];
    fread(buffer, 1, 32, fptr_src);
    for (int i = 0; i < 32; i++) 
    {
        buffer[i] = (buffer[i] & ~1) | ((data >> i) & 1);
    }
    fwrite(buffer, 1, 32, fptr_dest);
    return e_success;
}

/* Function to encode file extension */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo) 
{
    for (int i = 0; i < strlen(file_extn); i++) 
    {
        char buffer[8];
        fread(buffer, 1, 8, encInfo->fptr_src_image);
        encode_byte_to_lsb(file_extn[i], buffer);
        fwrite(buffer, 1, 8, encInfo->fptr_stego_image);
    }
    return e_success;
}

/* Function to encode file data */
Status encode_secret_file_data(EncodeInfo *encInfo) 
{
    char buffer[8];
    for (int i = 0; i < encInfo->size_secret_file; i++) 
    {
        fread(buffer, 1, 8, encInfo->fptr_src_image);
        char ch;
        fread(&ch, 1, 1, encInfo->fptr_secret);
        encode_byte_to_lsb(ch, buffer);
        fwrite(buffer, 1, 8, encInfo->fptr_stego_image);
    }
    return e_success;
}

/* Function to encode a single byte using LSB technique */
Status encode_byte_to_lsb(char data, char *image_buffer) 
{
    for (int i = 0; i < 8; i++) 
    {
        image_buffer[i] = (image_buffer[i] & ~1) | ((data >> i) & 1);
    }
    return e_success;
}

/* Function to copy remaining BMP data */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest) 
{
    char buffer[1024];
    int bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), fptr_src)) > 0) 
    {
        fwrite(buffer, 1, bytes, fptr_dest);
    }
    return e_success;
}

/* Function to perform encoding */
Status do_encoding(EncodeInfo *encInfo) 
{
    if (open_files(encInfo) != e_success)
    {
        return e_failure;
    } 
    if (check_capacity(encInfo) != e_success) 
    {
        return e_failure;
    }
    /* Copy BMP header */
    if(copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image)!= e_success)
    {
        return e_failure;
    }

    /* Encode Magic String */
    if(encode_magic_string(MAGIC_STRING, encInfo) != e_success)
    {
        return e_failure;
    }

    /* Encode secret file extension size */
    if(encode_int_to_lsb(strlen(encInfo->extn_secret_file), encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        return e_failure;
    }

    /* Encode file extension */
    if(encode_secret_file_extn(encInfo->extn_secret_file, encInfo) != e_success)
    {
        return e_failure;
    }

    /* Encode secret file size */
    if(encode_int_to_lsb(encInfo->size_secret_file, encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        return e_failure;
    }

    /* Encode secret file data */
    if(encode_secret_file_data(encInfo) != e_success)
    {
        return e_failure;
    }

    /* Copy remaining image data */
    if(copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) != e_success)
    {
        return e_failure;
    }

    fclose(encInfo->fptr_src_image);
    fclose(encInfo->fptr_secret);
    fclose(encInfo->fptr_stego_image);

    return e_success;
}
