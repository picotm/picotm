
#ifndef BASE64_H
#define BASE64_H

unsigned char *
encode64(const unsigned char *in, size_t inlen, size_t *outlen);

unsigned char *
decode64(const unsigned char *in, size_t inlen, size_t *outlen);

#endif

