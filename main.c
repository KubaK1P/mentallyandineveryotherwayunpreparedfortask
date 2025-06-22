#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define KEY_SIZE 512
#define CHUNK_SIZE 2048
#define ARTURIA 0

const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Wonder what this does. I will write this myself one day 
char *base64_encode(const unsigned char *data, size_t input_length, size_t *output_length) {
    static const char encoding_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    static const char padding_char = '=';
    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = (char *) malloc(*output_length + 1);
    if (encoded_data == NULL) return NULL;

    size_t i, j;
    for (i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? data[i++] : 0;
        uint32_t octet_b = i < input_length ? data[i++] : 0;
        uint32_t octet_c = i < input_length ? data[i++] : 0;

        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        encoded_data[j++] = encoding_table[(triple >> 18) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 12) & 0x3F];
        encoded_data[j++] = (i >= input_length + 1) ? padding_char : encoding_table[(triple >> 6) & 0x3F];
        encoded_data[j++] = (i >= input_length) ? padding_char : encoding_table[triple & 0x3F];
    }

    encoded_data[j] = '\0';
    return encoded_data;
}





// Generates the KEY of given length
unsigned char * generateKey(size_t length) {

    /* If I had a Polski Złoty every time I forgot malloc returns a void *, 
    which I have to cast for it to be usable, I would have 2 Polskie Złote. */
    unsigned char * key = (unsigned char *)malloc(length);
    if (key == NULL) {
        return NULL;
    }

    for (size_t i = 0; i < length; i++)
    {
        key[i] = rand() % 256; // very random, very secure (very mindful, very demure reference)
    }
    
    return key;
}

int main(int argc, char const *argv[])
{
    srand(time(NULL)); // cryptographically secure radomness generatore inicio-mente (latin)
    if (argc < 3) {
        printf("Too few arguments. Usage: %s <input> <output>\n", argv[0]);
        return 1;
    } else {
        const char * file_path = argv[1];
        const char * out_file_path = argv[2];

        printf("Encoding the file: %s\n", file_path);

        FILE * inputFile = fopen(file_path, "rb");

        if (inputFile == NULL) {
            printf("Failed to open file: %s\n", file_path);
            return 1;
        }

        char * buffer = (char *) malloc(CHUNK_SIZE + 1);
        if(buffer == NULL) {
            printf("Malloc failed miserably...\n");
            fclose(inputFile);
            return 2;
        }

        

        // printf("%s\n", buffer);

        unsigned char * encodeKey = generateKey(KEY_SIZE);
        if (encodeKey == NULL) {
            printf("The key struggled to generate itself...\n");
            fclose(inputFile);
            return 3;
        }

        //shit's useless
        size_t output_length;
        char * base64EncodeKey = base64_encode(encodeKey, KEY_SIZE, &output_length);
        if(base64EncodeKey == NULL) {
            printf("We couldn't change the flavour of your key to base64...\n");
            fclose(inputFile);
            return 4;
        }

        printf("--BASE64--\n\n%s\n\n--END OF BASE64 XD--", base64EncodeKey);

        // File that hopefully creates itself, Add custom save location later
        FILE * outputFile = fopen(out_file_path, "wb");
        if (outputFile == NULL) {
            printf("Output file could not be opened...\n");
            fclose(inputFile);
            return 1;
        }

        // DIDN'T forget this time yayy :3
        char * outputFileBuffer = (char *) malloc(CHUNK_SIZE + 1); 
        if (outputFileBuffer == NULL) {
            printf("Memory allocation failed...\n");
            fclose(inputFile);
            fclose(outputFile);
            return 2;
        }

        size_t bytes_read;
        size_t total_read = 0;

        while ( (bytes_read = fread(buffer, 1, CHUNK_SIZE, inputFile)) > 0) {
        // buffer[bytes_read] = '\0';  what is this C shyt
            // The encoding happens (hopefully) here
            for (int i = 0; i < bytes_read; i++) {
                outputFileBuffer[i] = buffer[i] ^ encodeKey[(total_read + i) % KEY_SIZE]; // The magic line of code
            }
            //these fockers should be outside the for, makes sense

            // move in the file
        total_read += bytes_read;
                
                // write the chunk to the output file
        fwrite(outputFileBuffer, 1, bytes_read, outputFile);
        }

        

        // Save the key to a file, You don't want this to get lost hihi
        FILE * keyOutputFile = fopen("key.txt", "w");
        if (keyOutputFile == NULL) {
            printf("KeyOutput file could not be opened...\n");
            fclose(inputFile);
            fclose(outputFile);
            return 1;
        }

        fwrite(base64EncodeKey, 1, output_length, keyOutputFile);
        
        // Freeing everything because C
        free(base64EncodeKey);
        free(encodeKey);
        free(buffer);
        free(outputFileBuffer);
        fclose(outputFile);
        fclose(inputFile);
        fclose(keyOutputFile);
    }
    
    return 0;
}
