#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>


uint32_t a0 = 0x67452301;  // A
uint32_t b0 = 0xefcdab89;  // B
uint32_t c0 = 0x98badcfe;  // C
uint32_t d0 = 0x10325476;

int shifts[64] = {
     7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22,  7, 12, 17, 22 ,
     5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20,  5,  9, 14, 20 ,
     4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23,  4, 11, 16, 23 ,
     6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21,  6, 10, 15, 21 
};


uint32_t keys[64] = {
     0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee ,
     0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501 ,
     0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be ,
     0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821 ,
     0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa ,
     0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8 ,
     0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed ,
     0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a ,
     0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c ,
     0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70 ,
     0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05 ,
     0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665 ,
     0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039 ,
     0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1 ,
     0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1 ,
     0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391 
};


uint32_t leftRotate(uint32_t x, int d)
{

    return (x << d) | (x >> (32 - d));
}

void print_state(uint8_t *state, int length) {

  for(int i = 0; i < length;i++) {
    if(i && i%4==0) printf(" ");

    printf("%x", state[i]);
  }
  printf("\n");

}

uint8_t* complete_pad(uint8_t* value, int* bytes_number) {
    uint64_t message_length = *bytes_number * 8; // the length of the message in bits
    int pad_length = 0;

    while (((*bytes_number + pad_length) % 64) != 56) {
        pad_length++;
    }


    uint8_t* tmp = (uint8_t*) calloc((*bytes_number + pad_length + 8), sizeof(uint8_t));
   
    for (int i = 0; i < *bytes_number; i++) {
        tmp[i] = value[i];
    }
   

    for (int j = 0; j < pad_length; j++) {
        
        if (j == 0) {
          tmp[*bytes_number+j] = (uint8_t)0x80;
        } else {
          tmp[*bytes_number+j] = (uint8_t)0x00;
        }
        
    }

    // handle endianess adding the length at the end

    for (int i =8; i >0;i--)
				tmp[*bytes_number + pad_length + 8 -i]=(uint8_t) (message_length>>((8-i)*8) & 0x00000000000000ff);

    *bytes_number = *bytes_number + pad_length + 8;
    return tmp;

}

uint8_t* pad(uint8_t* value, int* bytes_number) {
    int pad_length = 1;

    while (((*bytes_number + pad_length) % 64) != 0) {
        pad_length++;
    }

    uint8_t* tmp = (uint8_t*) calloc((*bytes_number + pad_length), sizeof(uint8_t));

    for (int i = 0; i < *bytes_number; i++) {
        tmp[i] = value[i];
    }
    for (int j = 0; j < pad_length; j++) {
        
        if (j == 0) {
          tmp[*bytes_number+j] = (uint8_t)0x80;
        } else {
          tmp[*bytes_number+j] = (uint8_t)0x00;
        }
        
    }
        *bytes_number = *bytes_number + pad_length;

    return tmp;
}

void divide_value(uint32_t** container, uint8_t* value, int chunks) {
    for (int i = 0; i < chunks; i++) {
        int base_index = i*64;
        for (int j = 0; j < 16; j++) {
            uint32_t val = 0;
            val ^= value[base_index+ 4*j+3] << 24;
            val ^= value[base_index + 4*j+ 2] << 16;
            val ^= value[base_index + 4*j + 1] << 8;
            val ^= value[base_index + 4*j + 0];

            container[i][j] = val;
        }
    }
}

void hash(uint8_t* value, int byte_length, int padding_type) {
    int chunks = 0;
    uint32_t state[4] = {0,0,0,0}; // the container of the current state (first a||b||c||d)
    
    // initialize state with IV
    state[0] = a0;
    state[1] = b0;
    state[2] = c0;
    state[3] = d0;
    
    uint8_t* val;
    
    // divide value in chunks of 512 bits (64 bytes) (padding if needed)

    //if ((byte_length % 64) != 0) {
        if (padding_type == 0) {
            val = complete_pad(value, &byte_length);    
        } else if (padding_type == 1) {
            val = pad(value, &byte_length);    
        }
        chunks = (int)(byte_length / 64);
    //}

    // debug: print_state(val,byte_length);

    for (int i_chunk = 0; i_chunk < chunks; i_chunk++) {
        int offset = 64*i_chunk;

        // by doing so, we don't need to handle the endianess which will be handled by the conversion itself
        uint32_t *container = (uint32_t *) (val + offset);

        uint32_t a = state[0];
        uint32_t b = state[1];
        uint32_t c = state[2];
        uint32_t d = state[3];

        for (int i = 0; i < 64; i++) { // 64 iterations
            uint32_t F;
            uint32_t index_message;

            if (i < 16) {
                F = (b & c) | ((~b) & d);
                index_message = i;
            } else if (i < 32) {
                F = (b & d) | (c & (~d));
                index_message = (5*i + 1) % 16;
            } else if (i < 48) {
                F = b ^ c ^ d;
                index_message = (3*i + 5) % 16;
            } else if (i < 64) {
                F = c ^ (b | (~d));
                index_message = (7*i) % 16;
            }

            
            uint32_t temp = d;
            d = c;
            c = b;            
            b = b + leftRotate((a + F + keys[i] + container[index_message]), shifts[i]);
            a = temp;
        }

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;


    }

    for (int i = 0; i < 4; i++) {
        uint8_t* new_array_of_bytes = (uint8_t*)&state[i];
        printf("%2.2x%2.2x%2.2x%2.2x", new_array_of_bytes[0], new_array_of_bytes[1], new_array_of_bytes[2], new_array_of_bytes[3]);
    }
    
    

}


void run(char* input, int padding_type) {
    int length = strlen(input);
    uint8_t* s = (uint8_t*) calloc(length,sizeof(uint8_t));

    for (int i = 0; i < length; i++) {
        s[i] = (uint8_t)input[i];
    }

    hash(s, length, padding_type);
}
void main(int argc, char** argv) {
    
    if (argc < 3) {
        printf("usage: %s <string> <0: complete padding (0x80, zeros and length)  | 1: only 0x80 and zeros>\n", argv[0]);
        return;
    }

    run(argv[1], atoi(argv[2]));
    

}