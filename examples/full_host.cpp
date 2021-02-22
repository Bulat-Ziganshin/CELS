#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CELS.h"

// Internal structure keeping read/write buffer positions for in-memory (de)compression operations
typedef struct
{
    char    *readPtr;           // current read position in inbuf (or NULL for redirecting reads to the original callback)
    size_t   readLeft;          // remaining bytes in the inbuf
    char    *writePtr;          // current write position in outbuf (or NULL for redirecting writes to the original callback)
    size_t   writeLeft;         // remaining bytes in the outbuf
} CelsMemBuf;

// Callback emulating CELS_READ/CELS_WRITE for in-memory (de)compression operations
static CelsResult __cdecl CelsReadWriteMem (void* self, int service, CelsNum subservice, void* inbuf, CelsNum insize, void* outbuf, CelsNum outsize, void* ud, CelsCallback0* cb)
{
    CelsMemBuf *membuf = (CelsMemBuf*)self;
    if (service==CELS_READ  &&  membuf->readPtr)
    {
        // Copy data from readPtr to inbuf and advance the read pointer
        size_t read_bytes = membuf->readLeft<insize ? membuf->readLeft : insize;
        memcpy (inbuf, membuf->readPtr, read_bytes);
        membuf->readPtr  += read_bytes;
        membuf->readLeft -= read_bytes;
        return read_bytes;
    }
    else if (service==CELS_WRITE  &&  membuf->writePtr)
    {
        // Copy data from outbuf to writePtr and advance the write pointer
        if (outsize > membuf->writeLeft)  return CELS_ERROR_OUTBLOCK_TOO_SMALL;
        memcpy (membuf->writePtr, outbuf, outsize);
        membuf->writePtr  += outsize;
        membuf->writeLeft -= outsize;
        return outsize;
    }
    else
    {
        printf("Unexpected request for service %d\n", service);
        return CELS_ERROR_NOT_IMPLEMENTED;
    }
}

int main (int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: program METHOD INFILE\n");
        return 1;
    }

    char* method = argv[1];
    FILE* infile = fopen(argv[2], "rb");
    if (infile == NULL) {
        printf("Can't find %s\n", argv[2]);
        return 1;
    }

    fseek(infile, 0, SEEK_END); 
    size_t origSize = ftell(infile); 
    fseek(infile, 0, SEEK_SET); 

    char* origBuf = (char*) malloc(origSize);
    origSize = fread(origBuf, 1, origSize, infile);

    size_t comprBufSize = size_t(origSize)*2 + 1024;
    char* comprBuf = (char*) malloc(comprBufSize);
    char* decomprBuf = (char*) malloc(origSize);


    CelsLoad();

    // Test memory compression
    CelsResult compressedSize = CelsCompressMem(method, origBuf, origSize, comprBuf, comprBufSize, NULL, NULL);
    if (compressedSize < CELS_OK)  {printf("Buffer compression failed: %s\n", CelsErrorMessage(compressedSize)); return 2;}
    printf("Buffer compressed %d to %d bytes\n", int(origSize), int(compressedSize));

    CelsResult decompressedSize = CelsDecompressMem(method, comprBuf, compressedSize, decomprBuf, origSize, NULL, NULL);
    if (decompressedSize < CELS_OK)  {printf("Buffer decompression failed: %s\n", CelsErrorMessage(decompressedSize)); return 3;}
    printf("Buffer decompressed to %d bytes\n", int(decompressedSize));

    if (decompressedSize != origSize  ||  memcmp(origBuf, decomprBuf, origSize) != 0)
        {printf("Buffer decompressed data doesn't match the original\n"); return 4;}

    printf("Buffer compression: data restored correctly\n\n");


    // Test stream compression
    CelsMemBuf comprMembuf = {origBuf, origSize, comprBuf, comprBufSize};
    CelsResult result = CelsCompress(method, &comprMembuf, CelsReadWriteMem);
    if (result < CELS_OK)  {printf("Stream compression failed: %s\n", CelsErrorMessage(result)); return 5;}
    compressedSize = comprBufSize - comprMembuf.writeLeft;
    printf("Stream compressed %d to %d bytes\n", int(origSize), int(compressedSize));

    CelsMemBuf decomprMembuf = {comprBuf, size_t(compressedSize), decomprBuf, origSize};
    result = CelsDecompress (method, &decomprMembuf, CelsReadWriteMem);
    if (result < CELS_OK)  {printf("Stream decompression failed: %s\n", CelsErrorMessage(result)); return 6;}
    decompressedSize = origSize - decomprMembuf.writeLeft;
    printf("Stream decompressed to %d bytes\n", int(decompressedSize));

    if (decompressedSize != origSize  ||  memcmp(origBuf, decomprBuf, origSize) != 0)
        {printf("Stream decompressed data doesn't match the original\n"); return 7;}

    printf("Stream compression: data restored correctly\n");


    return 0;
}
