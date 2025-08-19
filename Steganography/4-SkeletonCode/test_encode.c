#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include<string.h>
// OperationType check_operation_type(char *argv[]);
int main(int argc, char *argv[])
{
    
    Status option = check_operation_type(argv);
    if(option  == e_encode)
    {
        EncodeInfo encInfo;
        Status  validation =  read_and_validate_encode_args(argv, &encInfo);
        if(validation == e_success)
        {
            Status encoding = do_encoding(&encInfo);
            if(encoding == e_success)
            {
                printf("INFO: Encoding Done Successfully!\n");
                return e_success;
            } 
            else{
                printf("INFO: Encoding Failed?\n");
                return e_failure;
            }
        }
        else {
            printf("Encoding Validation is Failed?\n");
        }

    }
    else if(option == e_decode)
    {
        DecodeInfo decInfo;
        Status  validation =  read_and_validate_decode_args(argv, &decInfo);
        if(validation == e_success)
        {
            Status decoding = do_decoding(&decInfo);
            if(decoding == e_success)
            {
                printf("INFO: Decoding Done Successfully!\n");
                return e_success;
            } 
            else{
                printf("INFO: Decoding Failed?\n");
                return e_failure;
            }
        }
    }
    else{
        printf("ERROR: Read and Validate function failed\n");
        return e_failure;
    }

}

OperationType check_operation_type(char *argv[])
{
    if( argv[1] != NULL)
    {
        if(strcmp(argv[1], "-e") == 0)
        {
            return e_encode;
        }
        else if(strcmp(argv[1], "-d") == 0)
        {
            return e_decode;
        }
        else{
            return e_unsupported;
        }
    }
    else{
        return e_unsupported;
    }

}


