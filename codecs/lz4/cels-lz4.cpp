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

const int LZ4_CHUNKSIZE_WIDTH = 4;        // Width of the size fields in the compressed stream
const int LZ4_STREAM_CHUNKSIZE = 1<<20;   // Stream compression splits input data into chunks of this size

// Structure representing the parsed codec
struct Lz4Codec
{
    int acceleration;           // compression speed AKA the last parameter of LZ4_compress_fast*
    double MinCompression;      // minimal compression ratio, 0.99 means that data should be reduced by 1% at least
    size_t StreamChunkSize;     // size of chunks in the stream compression
};


// Stream compression employing callbacks for I/O
CelsResult CELS_LZ4_compress (Lz4Codec* codec, void* ud, CelsCallback* cb)
{
    size_t origBufSize = codec->StreamChunkSize;
    size_t compressedBufSize = LZ4_compressBound(origBufSize);

    char* buf = (char*) CelsMemAlloc(cb,ud, 2*origBufSize + LZ4_sizeofState() + LZ4_CHUNKSIZE_WIDTH + compressedBufSize);
    if (buf == NULL)  return CELS_ERROR_NOT_ENOUGH_MEMORY;

    char* origBuf[2] = {buf, buf + origBufSize};
    char* LZ4_state = buf + 2*origBufSize;
    char* compressedBuf = LZ4_state + LZ4_sizeofState();

    CelsResult errcode = CELS_OK;
    LZ4_stream_t* lz4Stream = LZ4_initStream(LZ4_state, LZ4_sizeofState());
    if (lz4Stream == NULL)  CELS_RETURN(CELS_ERROR_INTERNAL);

    for(int i=0; ; i^=1)
    {
        CelsResult origSize;
        CELS_READ_OR_EOF(origSize, origBuf[i], origBufSize);

        int compressedSize = LZ4_compress_fast_continue(lz4Stream,
            origBuf[i], compressedBuf + LZ4_CHUNKSIZE_WIDTH, origSize, compressedBufSize, codec->acceleration);
        if(compressedSize <= 0)   CELS_RETURN(CELS_ERROR_GENERAL);

        CELS_WRITE_WITH_SIZE(compressedSize,LZ4_CHUNKSIZE_WIDTH, compressedBuf);
    }

finished:
    CelsMemFree(cb,ud, buf);
    return errcode;
}


// Stream decompression employing callbacks for I/O
CelsResult CELS_LZ4_decompress (Lz4Codec* codec, void* ud, CelsCallback* cb)
{
    size_t origBufSize = codec->StreamChunkSize;
    size_t compressedBufSize = LZ4_compressBound(origBufSize);

    char* buf = (char*) CelsMemAlloc(cb,ud, 2*origBufSize + compressedBufSize);
    if (buf == NULL)  return CELS_ERROR_NOT_ENOUGH_MEMORY;

    char* origBuf[2] = {buf, buf + origBufSize};
    char* compressedBuf = buf + 2*origBufSize;

    CelsResult errcode = CELS_OK;
    LZ4_streamDecode_t lz4Stream[1];
    if (1 != LZ4_setStreamDecode(lz4Stream, NULL, 0))  CELS_RETURN(CELS_ERROR_INTERNAL);

    for(int i=0; ; i^=1)
    {
        CelsResult compressedSize;
        CELS_READ_WITH_SIZE_OR_EOF(compressedSize,LZ4_CHUNKSIZE_WIDTH, compressedBuf,compressedBufSize);

        int origSize = LZ4_decompress_safe_continue(lz4Stream,
            compressedBuf, origBuf[i], compressedSize, origBufSize);
        if(origSize <= 0)   CELS_RETURN(CELS_ERROR_BAD_COMPRESSED_DATA);

        CELS_WRITE_EXACTLY(origBuf[i], origSize);
    }

finished:
    CelsMemFree(cb,ud, buf);
    return errcode;
}


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
            codec->StreamChunkSize = LZ4_STREAM_CHUNKSIZE;
            return sizeof(Lz4Codec);
        }

    case CELS_GET_DICTIONARY_SIZE:
        return (LZ4_DISTANCE_MAX + 128) & ~255;  // round in order to avoid odd values

    case CELS_GET_MAX_COMPRESSED_SIZE:
        {
            CelsNum full_chunks = insize / codec->StreamChunkSize;
            return full_chunks * LZ4_compressBound(codec->StreamChunkSize)
                 + LZ4_compressBound(insize % codec->StreamChunkSize)
                 + (full_chunks + 1 + 1) * LZ4_CHUNKSIZE_WIDTH;  // +1 for possible extra zero word at the end of compressed stream
        }

    case CELS_GET_COMPRESSION_MEMORY:
    case CELS_GET_DECOMPRESSION_MEMORY:
        return 2 * codec->StreamChunkSize + LZ4_compressBound(codec->StreamChunkSize)
             + (service==CELS_GET_COMPRESSION_MEMORY? LZ4_sizeofState() + LZ4_CHUNKSIZE_WIDTH : 0);

    case CELS_COMPRESS:
        if (inbuf && outbuf)    // Memory buffer compression: from inbuf to outbuf
        {
            // LZ4_compress*() returns compressed size, or 0 if compression failed for any reason
            outsize = LZ4_compress_fast((const char*)inbuf, (char*)outbuf, insize, outsize, codec->acceleration);

            if (codec->MinCompression > 0  &&  outsize > insize * codec->MinCompression)
                return CELS_ERROR_OUTBLOCK_TOO_SMALL;

            return (outsize > 0 ?  outsize : CELS_ERROR_GENERAL);
        }

        if (!inbuf && !outbuf)
        {
            return CELS_LZ4_compress(codec, ud,cb);
        }

        return CELS_ERROR_NOT_IMPLEMENTED;

    case CELS_DECOMPRESS:
        if (inbuf && outbuf)    // Memory buffer decompression: from inbuf to outbuf
        {
            // LZ4_decompress_safe() returns output size, or negative value if decompression failed
            outsize = LZ4_decompress_safe((const char*)inbuf, (char*)outbuf, insize, outsize);

            return (outsize >= 0 ?  outsize : CELS_ERROR_BAD_COMPRESSED_DATA);
        }

        if (!inbuf && !outbuf)
        {
            return CELS_LZ4_decompress(codec, ud,cb);
        }

        return CELS_ERROR_NOT_IMPLEMENTED;

    default:
        return CELS_ERROR_NOT_IMPLEMENTED;
    }
}


#ifdef CELS_REGISTER_CODECS
static CelsResult dummy = CelsRegister ("lz4", NULL, CelsMain);
#endif
