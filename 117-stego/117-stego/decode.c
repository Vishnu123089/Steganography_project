/* NAME : VISHNU VARDHAN.E
   DATE : 30-09-2024
   DESCRIPTION : DECODING (decode.c) */

#include <stdio.h>
#include "decode.h"
#include "types.h"
#include <string.h>
#include "common.h"
#include <stdlib.h>

Status_d read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // Check if the necessary arguments are provided
    if (argv[2] == NULL) // Ensure the source image is provided
    {
        printf("Error: Source image file is not provided.\n");
        return d_failure;
    }

    if (strstr(argv[2], ".bmp")) // Validate encoded file is .bmp
    {
        decInfo->d_src_image_fname = argv[2];
        printf("Source image file: %s\n", decInfo->d_src_image_fname);
    }
    else
    {
        printf("Error: Source file is not a .bmp file\n");
        return d_failure;
    }

    // If argv[3] is not provided, use "decode.txt" as default
    if (argv[3] != NULL) // Check if the secret file name is provided
    {
        decInfo->d_secret_fname = strtok(argv[3], "."); // Removes the file extension
    }
    else
    {
        decInfo->d_secret_fname = "decode.txt"; // Default value
    }

    printf("Secret file (output): %s\n", decInfo->d_secret_fname);
    return d_success;
}

// Function definition for do decoding
Status_d do_decoding(DecodeInfo *decInfo)
{
    /* attempts to open the source BMP image and output secret file */
    if (open_files_dec(decInfo) == d_success)
    {
        printf("Files opened successfully\n");

        /* to check for a predefined string in the image */
        if (decode_magic_string(decInfo) == d_success)
        {
            printf("Magic string decoded successfully\n");

            /* reads the size of the secret file extension and checks it */
            if (decode_file_extn_size(strlen(".txt"), decInfo->fptr_d_src_image) == d_success)
            {
                /* decodes the file extension of the secret file and verifies it */
                if (decode_secret_file_extn(decInfo->d_extn_secret_file, decInfo) == d_success)
                {
                    /* It reads 32 bits (4 bytes) and decodes the size using LSB */
                    if (decode_secret_file_size(decInfo) == d_success)
                    {
                        printf("Secret file size decoded successfully\n");
                        // printf("Secret file data size: %d bytes\n", decInfo->size_secret_file);

                        /* Decode the secret file data */
                        if (decode_secret_file_data(decInfo) == d_success)
                        {
                            printf("Secret file data decoded successfully\n");
                        }
                        else
                        {
                            printf("Error: Failed to decode secret file data\n");
                        }
                    }
                    else
                    {
                        printf("Error: Failed to decode secret file size\n");
                    }
                }
                else
                {
                    printf("Error: Failed to decode secret file extension\n");
                }
            }
            else
            {
                printf("Error: Failed to decode file extension size\n");
            }
        }
        else
        {
            printf("Error: Failed to decode magic string\n");
        }
    }
    else
    {
        printf("Error: Failed to open files\n");
    }
    return d_success;
}

// Function definition for open files for decoding
Status_d open_files_dec(DecodeInfo *decInfo)
{
    // Open the source stego image file for reading.
    decInfo->fptr_d_src_image = fopen(decInfo->d_src_image_fname, "r");
    
    // Check if the file was opened successfully.
    if (decInfo->fptr_d_src_image == NULL)
    {
        perror("fopen"); // Print the error message
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->d_src_image_fname);
        return d_failure;
    }
    
    // Indicate successful opening of the source image file.
    printf("Source image file opened successfully: %s\n", decInfo->d_src_image_fname);

    // Open the destination file for writing the decoded secret data.
    decInfo->fptr_d_secret = fopen(decInfo->d_secret_fname, "w");
    
    // Check if the destination file was opened successfully.
    if (decInfo->fptr_d_secret == NULL)
    {
        perror("fopen"); // Print the error message from the system.
        fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo->d_secret_fname); 
        return d_failure; 
    }
    
    // Indicate successful opening of the output file.
    printf("Output file opened successfully: %s\n", decInfo->d_secret_fname);
    

    return d_success;
}

// Function definition for decode magic string
Status_d decode_magic_string(DecodeInfo *decInfo)
{
    // Move the fptr - skipping the BMP header (54 bytes).
    fseek(decInfo->fptr_d_src_image, 54, SEEK_SET);

    // Determine the length of the magic string.
    int i = strlen(MAGIC_STRING);

    /* The size allocated is the length of the MAGIC_STRING + null terminator. */
    decInfo->magic_data = malloc(strlen(MAGIC_STRING) + 1);

    // Decode the data from the image into magic_data.
    decode_data_from_image(strlen(MAGIC_STRING), decInfo->fptr_d_src_image, decInfo);
    
    // Null-terminate the decoded magic string.
    decInfo->magic_data[i] = '\0';

    // Compare the decoded magic string with the predefined MAGIC_STRING.
    if (strcmp(decInfo->magic_data, MAGIC_STRING) == 0)
    {
     
        printf("Magic string decoded successfully: %s\n", decInfo->magic_data);
        return d_success;
    }
    else
    {
        
        printf("Error: Magic string decoding failed\n");
        return d_failure;
    }
}



// Function definition for decode file extn size
Status_d decode_file_extn_size(int size, FILE *fptr_d_src_image)
{
    // Array to hold the encoded data read from the source image.
    char str[32];
    int length; // Variable to store the decoded file extension size.

    // Read 32 bytes of data from the source image into the str array.
    fread(str, 32, sizeof(char), fptr_d_src_image);

    // Decode the size of the file extension from LSB of the read-data.
    decode_size_from_lsb(str, &length);
    
    // Print the decoded file extension size 
    printf("Decoded file extension size: %d\n", length);

    // Check if the decoded length matches the size.
    if (length == size)
        return d_success; 
    else
        return d_failure; 
}



// Function definition for decoding the secret file extension
Status_d decode_secret_file_extn(char *file_ext, DecodeInfo *decInfo)
{
    // Set expected file extn(e.g., ".txt").
    file_ext = ".txt";
    int i = strlen(file_ext); // Get the length of the file extn.

    // Allocate memory in the DecodeInfo structure - hold the dec-file etxn.
    decInfo->d_extn_secret_file = malloc(i + 1); 

    // Decode the file extension data from the src image.
    decode_extension_data_from_image(strlen(file_ext), decInfo->fptr_d_src_image, decInfo);

    // Null-terminate decode file extn.
    decInfo->d_extn_secret_file[i] = '\0';

    // Compare the decoded file extension with the expected extension.
    if (strcmp(decInfo->d_extn_secret_file, file_ext) == 0)
    {
        
        printf("Secret file extension decoded successfully: %s\n", decInfo->d_extn_secret_file);
        return d_success; 
    }
    else
    {
        
        printf("Error: Secret file extension decoding failed\n");
        return d_failure; 
    }
}

// Function definition for decode secret file size
Status_d decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];  // Buffer to hold 32 bits (4 bytes) for the size
    int file_size;

    // Read 32 bits (4 bytes) from the image for the secret file size
    fread(buffer, 32, sizeof(char), decInfo->fptr_d_src_image);
    decode_size_from_lsb(buffer, &file_size);

    decInfo->size_secret_file = file_size;  // Store the decoded file size

    // Print the decoded size
    printf("Decoded secret file size: %d bytes\n", decInfo->size_secret_file);
    
    return d_success;
}

// Function definition for decode secret file data
Status_d decode_secret_file_data(DecodeInfo *decInfo)
{
    char str[8];  // Buffer to hold 8 bits (1 byte) for decoding
    char ch;      // Variable to hold the decoded character
    int stego_file_size = decInfo->size_secret_file; // Use the stored size
    int i;

    printf("Decoded secret file data:\n");
    // Decode the secret file data
    for (i = 0; i < stego_file_size; i++)
    {
        // Read 8 bits from the source image
        fread(str, 8, 1, decInfo->fptr_d_src_image);
        // Decode the byte from the least significant bit
        decode_byte_from_lsb(&ch, str);
        // Write the decoded byte to the secret file
        fwrite(&ch, 1, 1, decInfo->fptr_d_secret);

        // Display - decoded character
        printf("%c", ch);
    }

    return d_success;
}

// Function definition for decoding data from image
Status_d decode_data_from_image(int size, FILE *fptr_d_src_image, DecodeInfo *decInfo)
{
    int i;
    char str[8];
    for (i = 0; i < size; i++)
    {
        fread(str, 8, sizeof(char), fptr_d_src_image);
        decode_byte_from_lsb(&decInfo->magic_data[i], str);
    }
    return d_success;
}

// Function definition for decode byte from lsb
Status_d decode_byte_from_lsb(char *data, char *image_buffer)
{
    int bit = 7;
    unsigned char ch = 0x00;
    for (int i = 0; i < 8; i++)
    {
        ch = ((image_buffer[i] & 0x01) << bit--) | ch;
    }
    *data = ch;
    return d_success;
}

// Function definition decode size from lsb
Status_d decode_size_from_lsb(char *buffer, int *size)
{
    int j = 31;
    int value = 0;
    for (int i = 0; i < 32; i++)
    {
        value |= (buffer[i] & 0x01) << j--;
    }
    *size = value; // Assign the decoded value to the size
    return d_success;
}

// Function definition for decoding the file extension data from the image
Status_d decode_extension_data_from_image(int size, FILE *fptr_d_src_image, DecodeInfo *decInfo)
{
    char str[8];  // Buffer to hold 8 bits (1 byte) read from - image.
    char ch;      // Variable to hold the decoded character.
    int i;        

    
    for (i = 0; i < size; i++)
    {
        // Read 8 bits (1 byte) from the source image into the buffer.
        fread(str, 8, 1, fptr_d_src_image);

        // Decode the byte from the least significant bit (LSB) and store it in 'ch'.
        decode_byte_from_lsb(&ch, str);

        // Store the decoded character - file extn buffer 
        decInfo->d_extn_secret_file[i] = ch;
    }

    return d_success; 
}
