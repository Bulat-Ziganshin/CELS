/*
    LZ4 codec for CELS - Framework and standard API for compression algorithms
    Copyright (C) 2021, Bulat Ziganshin <Bulat.Ziganshin@gmail.com>

    MIT License (https://opensource.org/licenses/MIT)

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    You can contact the author at:
       - CELS repository: https://github.com/Bulat-Ziganshin/CELS
*/

#include "lz4/lib/lz4.c"
#include "CELS.h"

// Structure representing the parsed codec
struct Lz4Codec
{
    int acceleration;
    double MinCompression;  // minimal compression ratio
};


CelsResult __cdecl CelsMain (void* self, int service, CelsNum subservice, void* inbuf, CelsNum insize, void* outbuf, CelsNum outsize, void* ud, CelsCallback* cb)
{
    Lz4Codec *codec = (Lz4Codec*)self;

    switch (service)
    {
    case CELS_PARSE:
        {
            codec = (Lz4Codec*)outbuf;
            codec->acceleration = 1;
            codec->MinCompression = 0;
            return sizeof(Lz4Codec);
        }

    case CELS_GET_MAX_COMPRESSED_SIZE:
        return LZ4_compressBound(insize);

    case CELS_COMPRESS:
        if (inbuf && outbuf)    // Memory buffer compression: from inbuf to outbuf
        {
            // LZ4_compress*() returns compressed size, or 0 if compression failed for any reason
            outsize = LZ4_compress_fast((const char*)inbuf, (char*)outbuf, insize, outsize, codec->acceleration);

            if (codec->MinCompression > 0  &&  outsize > insize * codec->MinCompression)
                return CELS_ERROR_OUTBLOCK_TOO_SMALL;

            return (outsize > 0 ?  outsize : CELS_ERROR_GENERAL);
        }
        return CELS_ERROR_NOT_IMPLEMENTED;

    case CELS_DECOMPRESS:
        if (inbuf && outbuf)    // Memory buffer decompression: from inbuf to outbuf
        {
            // LZ4_decompress_safe() returns output size, or negative value if decompression failed
            outsize = LZ4_decompress_safe((const char*)inbuf, (char*)outbuf, insize, outsize);

            return (outsize >= 0 ?  outsize : CELS_ERROR_BAD_COMPRESSED_DATA);
        }
        return CELS_ERROR_NOT_IMPLEMENTED;

    default:
        return CELS_ERROR_NOT_IMPLEMENTED;
    }
}

#ifdef CELS_REGISTER_CODECS
static CelsResult dummy = CelsRegister ("lz4", NULL, CelsMain);
#endif
