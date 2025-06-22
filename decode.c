#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define KEY_SIZE 512
#define CHUNK_SIZE 2048
#define NOVATION 1

unsigned char base64_value(char c) {
    if ('A' <= c && c <= 'Z') return c - 'A';
    if ('a' <= c && c <= 'z') return c - 'a' + 26;
    if ('0' <= c && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return 0;
}

unsigned char *base64_decode(const char *data, size_t input_length, size_t *output_length) {
    if (input_length % 4 != 0) return NULL;

    // Count padding
    size_t padding = 0;
    if (input_length >= 1 && data[input_length - 1] == '=') padding++;
    if (input_length >= 2 && data[input_length - 2] == '=') padding++;

    *output_length = (input_length / 4) * 3 - padding;

    unsigned char *decoded = (unsigned char *) malloc(*output_length);
    if (!decoded) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t sextet_a = base64_value(data[i++]);
        uint32_t sextet_b = base64_value(data[i++]);
        uint32_t sextet_c = data[i] == '=' ? 0 : base64_value(data[i]);
        i++;
        uint32_t sextet_d = data[i] == '=' ? 0 : base64_value(data[i]);
        i++;

        uint32_t triple = (sextet_a << 18) |
                          (sextet_b << 12) |
                          (sextet_c << 6) |
                          sextet_d;

        if (j < *output_length) decoded[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded[j++] = triple & 0xFF;
    }

    return decoded;
}



int main(int argc, char const *argv[])
{
    if (argc < 4) {
        printf("Usage: %s <encrypted file> <file with key> <out file path>", argv[0]);
        return 1;
    } 

    // open some files, forget to close them, eat ice cream, repeat
    FILE *encFile = fopen(argv[1], "rb");
    FILE *keyFile = fopen(argv[2], "r");
    FILE *outFile = fopen(argv[3], "wb");

    if(!encFile || !keyFile || !outFile) {
        printf("Make sure the paths you provided are correct because some files couldn't be opened...\n");
        return 1;
    }

    // move the cursor to the end to find the key length => really clever
    fseek(keyFile, 0, SEEK_END);
    long key_len = ftell(keyFile);
    rewind(keyFile);

    char * base64Key = (char *) malloc(key_len + 1);
    if (base64Key == NULL) {
        printf("Memory allocation failed...\n");
        return 2;
    }
    // here czytamy klucz
    fread(base64Key, 1, key_len, keyFile);
    base64Key[key_len] = '\0'; // again with the lodziarnia

    unsigned char *buffer = (unsigned char *) malloc(CHUNK_SIZE);
    unsigned char *output = (unsigned char *) malloc(CHUNK_SIZE);

    if (!buffer || !output) {
        printf("Memory allocation failed...\n");
        return 2;
    }

    size_t bytes_read, total_read = 0;

    // Decode the base64 key before the while-loop:
    size_t decoded_key_len;

    
    // Clean input (remove '\n' and '\r')
    size_t clean_len = 0;
    for (size_t i = 0; i < key_len; i++) {
        if (base64Key[i] != '\n' && base64Key[i] != '\r') {
            base64Key[clean_len++] = base64Key[i];
        }
    }
    base64Key[clean_len] = '\0';


    
    printf("Clean Base64 length: %zu\n", clean_len);
    printf("Base64 content:\n[%.*s]\n", (int)clean_len, base64Key);

    unsigned char *key = base64_decode(base64Key, clean_len, &decoded_key_len);
    if (!key || decoded_key_len != KEY_SIZE) {
        printf("Key decoding failed or key length mismatch... got %zu bytes (expected %d)\n", decoded_key_len, KEY_SIZE);
        return 3;
    }


    while ((bytes_read = fread(buffer, 1, CHUNK_SIZE, encFile)) > 0) {

        // do some dark magic severus snape style
        for (size_t i = 0; i < bytes_read; i++) {
            output[i] = buffer[i] ^ key[(total_read + i) % KEY_SIZE];
        }

        // write chunk to outfile
        fwrite(output, 1, bytes_read, outFile);
        total_read += bytes_read;    
    }

    printf("Task complete probably...\n");
    // close the mfs before I forget
    free(key);
    free(base64Key);
    free(buffer);
    free(output);
    fclose(encFile);
    fclose(keyFile);
    fclose(outFile);

    return 0;
}
