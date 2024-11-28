/* NAME : VISHNU VARDHAN.E
   DATE : 30-09-2024
   DESCRIPTION : DECODING  (decode.h)*/

#ifndef DECODE_H
#define DECODE_H

#include "types.h" //Include user-defined type

/*
 * Structure to store information required for
 * decoding the secret file from the source image.
 * This structure also holds information about output
 * and intermediate data used during decoding.
 */

#define MAX_SECRET_BUF_SIZE 1                         // Maximum size for the secret data buffer
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8) // Image buffer size (8 bits for each secret byte)

// Structure -> decoding information

typedef struct _DecodeInfo
{
    /* Stego image information */
    char *d_src_image_fname;                 // Filename - source stego image
    FILE *fptr_d_src_image;                 //File pointer - reading the stego image

    char d_image_data[MAX_IMAGE_BUF_SIZE];  // Buffer to hold - image data
    char *magic_data;                       // Pointer to hold - decoded magic string
    char *d_extn_secret_file;               // Pointer to hold - decoded file extn of secret file

    int size_secret_file;                   // Size - decoded secret file
    FILE *fptr_d_dest_image;                // File pointer - destination (o/p)file

    char *d_secret_fname; // Pointer to hold the name of the secret file (output)
    FILE *fptr_d_secret; // File pointer for the secret file where decoded data will be stored
} DecodeInfo; 

/* Decoding Function Prototypes */

/* Function to read and validate command line arguments for decoding */
Status_d read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo);

/* Function to perform the decoding process */
Status_d do_decoding(DecodeInfo *decInfo);

/* Function to open input and output files for decoding */
Status_d open_files_dec(DecodeInfo *decInfo);

/* Function to decode the magic string from the image */
Status_d decode_magic_string(DecodeInfo *decInfo);

/* Function to decode data from the image */
Status_d decode_data_from_image(int size, FILE *fptr_d_src_image, DecodeInfo *decInfo);

/* Function to decode a byte from the least significant bit (LSB) of the image */
Status_d decode_byte_from_lsb(char *data, char *image_buffer);

/* Function to decode the size of the file extension */
Status_d decode_file_extn_size(int size, FILE *fptr_d_src_image);

/* Function to decode a size value from the least significant bit */
Status_d decode_size_from_lsb(char *buffer, int *size);

/* Function to decode the secret file's extension */
Status_d decode_secret_file_extn(char *file_ext, DecodeInfo *decInfo);

/* Function to decode the extension data from the image */
Status_d decode_extension_data_from_image(int size, FILE *fptr_d_src_image, DecodeInfo *decInfo);

/* Function to decode the size of the secret file */
Status_d decode_secret_file_size(DecodeInfo *decInfo);

/* Function to decode the actual secret file data */
Status_d decode_secret_file_data(DecodeInfo *decInfo);

#endif 
