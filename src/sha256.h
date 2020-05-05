#ifndef _SHA256_H
#define _SHA256_H

#ifndef uint8
#define uint8  unsigned char
#endif

#ifndef uint32
#define uint32 unsigned long int
#endif


typedef struct
{
    unsigned long int total[2];
    unsigned long int state[8];
    unsigned char buffer[64];
}
sha256_context;

void sha256_starts( sha256_context *ctx );
void sha256_update( sha256_context *ctx, unsigned char *input, unsigned long int length );
void sha256_finish( sha256_context *ctx, unsigned char digest[32] );

#endif /* sha256.h */

