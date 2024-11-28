/* NAME : VISHNU VARDHAN.E
   DATE : 20-09-2024
   DESCRIPTION : ENCODING (encode.c) */

#include <stdio.h>
#include "encode.h"
#include "types.h"
#include <string.h>
#include "common.h"

/* Function definition for check operation type */
// Compares the command-line argument with expected flags and returns the appropriate operation type.
OperationType check_operation_type(char *argv[])
{
    if (strcmp(argv[1], "-e") == 0)
        return e_encode;
    if (strcmp(argv[1], "-d") == 0)
        return e_decode;
    else
        return e_unsupported;
}


// Function definition for read and validate encode args
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    //Checks the file extensions for BMP ,SH and TXT.

    if (strcmp(strstr(argv[2], "."), ".bmp") == 0)
        encInfo->src_image_fname = argv[2];
    else
        return e_failure;
    if (strcmp(strstr(argv[3], "."), ".txt") == 0)  //  Sets the filenames in the EncodeInfo structure. 
        encInfo->secret_fname = argv[3];
    else
        return e_failure;
    if (argv[4] != NULL)
        encInfo->stego_image_fname = argv[4];
    else
        encInfo->stego_image_fname = "stego.bmp";  //If no stego image name is provided, defaults to "stego.bmp".
    return e_success;
}


// Function definition for do encoding called in main function
Status do_encoding(EncodeInfo *encInfo)
{
    /*Opens the input files (source image and secret file) and creates the output stego image.*/
    if (open_files(encInfo) == e_success)
    {
        printf("Open files is a successfully\n");
        /*Checks if the source image has enough capacity to hold the secret data.*/
        if (check_capacity(encInfo) == e_success)
        {
            printf("Check capacity is successfully\n");
            /*Copies the BMP header from the source image to the stego image.[54 LINES]*/
            if (copy_bmp_header(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
            {
                printf("Copied bmp header successfully\n");
                /*Encodes a predefined magic string into the stego image to identify it later.
                  To encode the magic string into the image.*/
                if (encode_magic_string(MAGIC_STRING, encInfo) == e_success)
                {
                    printf("Encoded magic string successfully\n");
                   /* It then copies the substring starting from the period(.)*/
                    strcpy(encInfo->extn_secret_file, strstr(encInfo->secret_fname, "."));
                    /*Encodes the size of the secret file extension in the image*/
                    if (encode_secret_file_extn_size(strlen(encInfo->extn_secret_file), encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                    {
                        printf("Encoded secret file extn size successfully\n");
                        /* Encodes the actual file extension of the secret file*/
                        if (encode_secret_file_extn(encInfo->extn_secret_file, encInfo) == e_success)
                        {
                            printf("Encoded secret file extn successfully\n");
                            /*Encodes the size of the secret file in the image.*/
                            if (encode_secret_file_size(encInfo->size_secret_file, encInfo) == e_success)
                            {
                                printf("Encoded secret file size successfully\n");
                                /*Encodes the contents of the secret file into the stego image*/
                                if (encode_secret_file_data(encInfo) == e_success)
                                {
                                    printf("Encoded secret file data successfully\n");
                                    /* Copies any remaining data from the source image to the stego image after encoding.*/
                                    if (copy_remaining_img_data(encInfo->fptr_src_image, encInfo->fptr_stego_image) == e_success)
                                    {
                                        printf("Copied remaining data successfully\n");
                                    }
                                    else
                                    {
                                        printf("Failed to copy remaining data successfully\n");
                                        return e_failure;
                                    }
                                }
                                else
                                {
                                    printf("Failed to encode secret file data\n");
                                    return e_failure;
                                }
                            }
                            else
                            {
                                printf("Failed to encode secret file size\n");
                                return e_failure;
                            }
                        }
                        else
                        {
                            printf("Failed to encode secret file extn\n");
                            return e_failure;
                        }
                    }
                    else
                    {
                        printf("Failed to encoded secret file extn size\n");
                        return e_failure;
                    }
                }
                else
                {
                    printf("Failed to encode magic string\n");
                    return e_failure;
                }
            }
            else
            {
                printf("Failed to copy bmp header\n");
                return e_failure;
            }
        }
        else
        {
            printf("Check capacity is a failure\n");
            return e_failure;
        }
    }
    else
    {
        printf("Open files is a failure\n");
        return e_failure;
    }
    return e_success;
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
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
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
    // Stego Image file - To write the image data with the embedded secret file.
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



// Function definition for check capacity
Status check_capacity(EncodeInfo *encInfo)
{
    encInfo->image_capacity = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo->size_secret_file = get_file_size(encInfo->fptr_secret);

/* TOTAL REQUIRED BITS : 
 - strlen(MAGIC_STRING) * 8 bits for the magic string
 - 32 bits for encoding the size of the secret file extension (integer)
 - strlen(encInfo->extn_secret_file) * 8 bits for the secret file extension
 - 32 bits for encoding the size of the secret file (integer)
 - (encInfo->size_secret_file )* 8 bits for the actual secret file data */

    if (encInfo->image_capacity > ((strlen(MAGIC_STRING)*8+32+strlen(encInfo->extn_secret_file)*8+32+encInfo->size_secret_file)*8))
        return e_success;
    else
        return e_failure;
}


/* Function Definitions */
/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int) -> Located at byte offset 18-21 (4 bytes).
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int) -> Located at byte offset 22-25 (4 bytes).
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity 
    return width * height * 3;  // each pixel in a BMP image uses 3 bytes (Red, Green, Blue).
}


// Function definition for getting file size
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);
    return ftell(fptr);    //returns the current position of the file pointer.
}

// Function definition for copying 1st 54 bytes header file
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    char str[54];  // holds the header data read from the src-image before writing it to the dest-image. 

    // Setting pointer to point to 0th position
    fseek(fptr_src_image, 0, SEEK_SET);

    //Reading 54 bytes from beautiful.bmp - Size of each element (54 bytes for BMP header)
    //Reading larger chunks of data is more efficient because it reduces function calls and I/O operations.
    fread(str, 54, 1, fptr_src_image);

    // Writing 54 bytes to str
    fwrite(str, 54, 1, fptr_dest_image);
    return e_success;
}



// Function definition for encoding magic string
Status encode_magic_string(char *magic_string, EncodeInfo *encInfo)
{
    encode_data_to_image(magic_string, 2, encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}

// Function definition for encode secret file extn size
Status encode_secret_file_extn_size(int size, FILE *fptr_src_image, FILE *fptr_stego_image)
{
    char str[32];
    fread(str, 32, 1, fptr_src_image);
    encode_size_to_lsb(size, str);
    fwrite(str, 32, 1, fptr_stego_image);
    return e_success;
}


// Function definition to encode secret file extn
Status encode_secret_file_extn(char *file_extn, EncodeInfo *encInfo)
{
    encode_data_to_image(file_extn, strlen(file_extn), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);
    return e_success;
}


// Function definition for encoding the size of the secret file into the source image
Status encode_secret_file_size(int size, EncodeInfo *encInfo)
{
    char str[32];  // Temporary buffer to hold data read from the source image

    // Read 32 bytes from the source image file
    fread(str, 32, 1, encInfo->fptr_src_image);

    // Encode the size of the secret file into the least significant bits of the read data
    encode_size_to_lsb(size, str);

    // Write the modified data back to the stego image file
    fwrite(str, 32, 1, encInfo->fptr_stego_image);

    return e_success; 
}

// Function definition for encoding the secret file data into the image
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    // Set the file pointer of the secret file to the beginning
    fseek(encInfo->fptr_secret, 0, SEEK_SET);

    // Create a buffer to hold the secret file data
    char str[encInfo->size_secret_file];

    // Read the secret file data into the buffer
    fread(str, encInfo->size_secret_file, 1, encInfo->fptr_secret);

    // Encode the secret file data into the source image, modifying the stego image
    encode_data_to_image(str, strlen(str), encInfo->fptr_src_image, encInfo->fptr_stego_image, encInfo);

    return e_success; 
}

// Function definition for encoding data into the image
Status encode_data_to_image(char *data, int size, FILE *fptr_src_image, FILE *fptr_stego_image, EncodeInfo *encInfo)
{
    // Loop through each byte of the data to be encoded
    for (int i = 0; i < size; i++)
    {
        // Read 8 bits (1 byte) from the source image file into the image_data buffer
        fread(encInfo->image_data, 8, 1, fptr_src_image);

        // Encode the current byte of data into the least significant bits of the image data
        encode_byte_to_lsb(data[i], encInfo->image_data);

        // Write the modified image data back to the stego image file
        fwrite(encInfo->image_data, 8, 1, fptr_stego_image);
    }

    return e_success; 
}

// Function definition for encode byte to lsb
Status encode_byte_to_lsb(char data, char *image_buffer) {
    // Define a mask that will help isolate each bit of 'data'
    unsigned int mask = 0x80;  // Start with the highest bit (1000 0000)
    unsigned int i;            // Loop variable

    // Loop through each bit of the byte 'data'
    for (i = 0; i < 8; i++) {
        // Update the corresponding bit in the image_buffer:
        // 1. Clear the least significant bit (LSB) of the current pixel
        //    by ANDing with 0xFE (1111 1110).
        // 2. Set the LSB to the bit from 'data' using bitwise OR.
        //    The bit from 'data' is obtained by shifting it down using the mask.
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((data & mask) >> (7 - i));

        // Shift the mask one position to the right to get the next bit in 'data'
        mask = mask >> 1;
    }

    return d_success;  // Return success status
}



// Function definition to encode size to lsb
Status encode_size_to_lsb(int size, char *image_buffer) {
    // Define a mask to isolate each bit of 'size'.
    unsigned int mask = 1 << 31;  // Start with the highest bit (1000 0000 0000 0000 0000 0000 0000 0000)
    unsigned int i;               // Loop variable

    // Loop through each bit of the integer 'size'
    for (i = 0; i < 32; i++) {
        // Update the corresponding bit in the image_buffer:
        // 1. Clear the least significant bit (LSB) of the current pixel
        //    by ANDing with 0xFE (1111 1110).
        // 2. Set the LSB to the bit from 'size' using bitwise OR.
        image_buffer[i] = (image_buffer[i] & 0xFE) | ((size & mask) >> (31 - i));

       
        mask = mask >> 1;  // Move to the next lower bit
    }

    return d_success;  // Return success status
}



// Function definition for copying remaining data as it is
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{
    char ch;
    /* read bytes from the source file (fptr_src) until there are no more bytes left to read.  
    Once it reaches the end of the file (where no more bytes can be read), fread will terminate*/
    while ((fread(&ch, 1, 1, fptr_src)) > 0)
    {
        fwrite(&ch, 1, 1, fptr_dest);
    }
    return e_success;
}

