#ifndef DECODE_H
#define DECODE_H
#include <stdio.h>
#include "types.h"

#define MAX_SECRET_BUF_SIZE 1
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)
#define MAX_FILE_SUFFIX 4

typedef struct _DecodeInfo 
{
    /* Source Image info */
    char *src_image_fname;
    FILE *fptr_src_image;
    char magic_str[3];

    int extn_size;
    char file_extn[10];

    /* destination Image Info */
    char dest_image_fname[20];
    FILE *fptr_dest_image;
    char *data;

} DecodeInfo;

/* Decoding function prototypes */
OperationType check_operation_type(char *argv[]);
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);
Status do_decoding(DecodeInfo *decInfo);
Status open_file(DecodeInfo *decInfo);
Status skip_bmp_header(FILE *fptr_src_image);
Status decode_magic_string(DecodeInfo *decInfo);
char decode_byte_from_lsb(char *buffer);
Status decode_secret_file_extn_size(int *extn_size, DecodeInfo *decInfo);
Status decode_secret_file_extn(char *file_extn, DecodeInfo *decInfo);
Status decode_secret_file_size(int *file_size, DecodeInfo *decInfo);
Status decode_secret_file_data(DecodeInfo *decInfo);
Status ecode_secret_file_extn_concatinate(DecodeInfo *decInfo);
#endif


