
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "base64.h"

static void
encblock(unsigned char *out, const unsigned char *in)
{
    assert(out);
    assert(in);

    static const unsigned char enc[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    out[0] = enc[  (in[0] >> 2) ];
    out[1] = enc[ ((in[0]&0x3) << 4)  | ((in[1]&0xf0) >> 4) ];
    out[2] = enc[ ((in[1]&0x0f) << 2) | ((in[2]&0xc0) >> 6) ];
    out[3] = enc[  (in[2]&0x3f) ];
}

unsigned char *
encode64(const unsigned char *in, size_t inlen, size_t *outlen)
{
    size_t len = inlen * 4 / 3;

    unsigned char *out = malloc(len+4);
    unsigned char *out2 = out;

    size_t outlen2 = 0;

    /* Encode 3-byte blocks */

    while (inlen >= 3) {
        encblock(out2, in);
        out2 += 4;
        in += 3;
        inlen -= 3;
        outlen2 += 4;
    }

    /* Encode rest */

    if (inlen) {
        unsigned char intmp[3];
        memcpy(intmp, in, inlen);
        memset(intmp+inlen, 0, 3-inlen);
        encblock(out2, intmp);

        const size_t blocklen[] = {0, 2, 3};
        assert(inlen < sizeof(blocklen)/sizeof(blocklen[0]));
        assert(blocklen[inlen]);

        out2 = memset(out2+blocklen[inlen], '=', 4-blocklen[inlen]);

        outlen2 += 4;
    }

    if (outlen) {
        *outlen = outlen2;
    }

    return out;
}

static void
decblock(unsigned char *out, const unsigned char *in)
{
    assert(out);
    assert(in);

    static const unsigned char dec[] = {
        '~','~','~','~','~','~','~','~','~','~','~','~','~','~','~','~',
        '~','~','~','~','~','~','~','~','~','~','~','~','~','~','~','~',
        '~','~','~','~','~','~','~','~','~','~','~','>','~','~','~','?',
        '4','5','6','7','8','9',':',';','<','=','~','~','~',255,'~','~',
        '~', 0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10, 11, 12, 13, 14,
         15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25,'~','~','~','~','~',
        '~', 26, 27, 28, 29, 30, 31, 32,'!', 34,'#','$','%','&', 39,'(',
        ')','*','+',',','-','.','/','0','1','2','3','~','~','~','~','~'};

    assert(dec[in[0]] != '~');
    assert(dec[in[1]] != '~');

    out[0] =  (dec[in[0]] << 2) | (dec[in[1]] >> 4);
    out[1] =  (dec[in[1]] << 4) | (dec[in[2]] >> 2);
    out[2] = ((dec[in[2]]&0x03) << 6) | dec[in[3]];
}

unsigned char *
decode64(const unsigned char *in, size_t inlen, size_t *outlen)
{
    if (inlen%4) {
        return NULL;
    }

    size_t len = inlen * 3 / 4;

    unsigned char *out = malloc(len+3);
    unsigned char *out2 = out;

    size_t outlen2 = 0;

    /* Decode 4-character blocks */

    while (inlen > 4) {
        decblock(out2, in);
        outlen2 += 3;
        out2 += 3;
        in += 4;
        inlen -= 4;
    }

    /* Decode rest */

    if (inlen) {
        unsigned char outtmp[3];

        decblock(outtmp, in);

        size_t blocklen;

        const unsigned char *pos = memchr(in, '=', 4);

        if (pos) {
            ptrdiff_t diff = pos-in;
        
            if ((diff == 0) || (diff == 1)) {
                free(out);
                return NULL;
            } else if (diff == 2) {
                blocklen = 1;
            } else if (diff == 3) {
                blocklen = 2;
            }
        } else {
            blocklen = 3;
        }

        memcpy(out2, outtmp, blocklen);
        outlen2 += blocklen;
    }

    if (outlen) {
        *outlen = outlen2;
    }

    return out;
}
#include <stdio.h>
void test_base64(void)
{
    /*size_t len = strlen(enc);
    size_t i;

    for (i = 0; i < len; ++i) {
        fprintf(stdout, "%c", enc[i]);
        assert( i == dec[enc[i]] );

    }*/
/*
    static const unsigned char test[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    size_t len = strlen(test)+1;

    fprintf(stdout, "\n>%s< %d\n", test, len);

    unsigned char *str = encode64(test, 65, &len);

    fprintf(stdout, "\n>%.*s<\n", len, str);
    fprintf(stdout, "\n%d\n", len);

    str = decode64(str, len, &len);

    fprintf(stdout, "\n>%s< %d\n", str, len);

    assert(!memcmp(test, str, len));*/
}

