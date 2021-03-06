/*
    CELS - Framework and standard API for compression algorithms
    Copyright (C) 2017-2021, Bulat Ziganshin <Bulat.Ziganshin@gmail.com>

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

#ifndef CELS_H
#define CELS_H

#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Function types representing codecs and callbacks
typedef long long CelsResult, CelsNum;
typedef CelsResult __cdecl CelsCallback0(void);
typedef CelsResult __cdecl CelsCallback (void* self, int service, CelsNum subservice, void* inbuf, CelsNum insize, void* outbuf, CelsNum outsize, void* ud, CelsCallback0* cb);
typedef CelsResult __cdecl CelsFunction (void* self, int service, CelsNum subservice, void* inbuf, CelsNum insize, void* outbuf, CelsNum outsize, void* ud, CelsCallback* cb);
// Method registering/parsing
CelsResult CelsRegister (const char* name, void* ud, CelsFunction* CelsMain);
CelsResult CelsParseStr (const char* method_str, void* method, CelsNum method_size, void* ud, CelsCallback* cb);
CelsResult CelsParseSplitted (char const* const* parameters, void* method, CelsNum method_size, void* ud, CelsCallback* cb);
// DLL loading/unloading
CelsResult CelsRegisterModule (void* dll, const char* method_name, CelsFunction* CelsMain);
CelsResult CelsLoad();
void CelsUnload();
// Providing actual services
CelsResult Cels (const void* method, int service, CelsNum subservice, void* inbuf, CelsNum insize, void* outbuf, CelsNum outsize, void* ud, CelsCallback* cb);
const char* CelsErrorMessage (CelsResult errcode);  // English description of error code
// User-defined functions
CelsResult __cdecl CelsMain (void* self, int service, CelsNum subservice, void* inbuf, CelsNum insize, void* outbuf, CelsNum outsize, void* ud, CelsCallback* cb);


// Operation masks
const int CELS_SET                              = 0x01000000;   // Adding this value to service number converts "get param" operation into the corresponding "set param" one

// Operations that can be implemented by codec instance in CelsMain()
inline static int IS_CELS_INSTANCE_SERVICE (int service)  {return (unsigned)service < 0x04000000;}   // Family of codec instance services
const int CELS_INITIALIZE                       = 0x00000000;   // First operation called on new instance, allowing it to initialize itself
const int CELS_FREE                             = 0x00000001;   // Last operation called on the instance, allowing it to free up any resources taken
const int CELS_UNPARSE                          = 0x00000002;   // Put into (outbuf,outsize) buffer some variant of string representing the method instance, where variant is defined by the insize containing one of CELS_UNPARSE_* constants
const int CELS_COMPRESS                         = 0x00000004;   // Compress (encode) data using CELS_READ/CELS_WRITE callbacks (and optionally CELS_PROGRESS/CELS_QUASI_WRITE to inform application about operation progress). Also: Compress buffer (inbuf,insize) into buffer (outbuf,outsize) and return compressed size. When inbuf and/or outbuf is NULL, read/write data via callbacks or return CELS_ERROR_NOT_IMPLEMENTED
const int CELS_DECOMPRESS                       = 0x00000005;   // Like above but decompress (decode)
// Information requests
const int CELS_GET_EXPAND_DATA                  = 0x01000000;   // Can this compressor expand data (like precomp)?
const int CELS_GET_NUM_INPUT_STREAMS            = 0x01000001;   // Number of input streams for compression (== number of output streams for decompression)
const int CELS_GET_NUM_OUTPUT_STREAMS           = 0x01000002;   // Number of output streams for compression (== number of input streams for decompression)
const int CELS_GET_MAX_COMPRESSED_SIZE          = 0x01000003;   // Upper limit of compressed size for given insize
// Get algorithm parameters
const int CELS_GET_COMPRESSION_MEMORY           = 0x02000000;   // How much memory for compression?
const int CELS_GET_DECOMPRESSION_MEMORY         = 0x02000001;   // How much memory for decompression?
const int CELS_GET_MINIMUM_COMPRESSION_MEMORY   = 0x02000002;   // Minimal memory required for compression?
const int CELS_GET_MINIMUM_DECOMPRESSION_MEMORY = 0x02000003;   // Minimal memory required for decompression?
const int CELS_GET_DICTIONARY_SIZE              = 0x02000004;   // Dictionary size (for LZ and similar compressors)
const int CELS_GET_BLOCKSIZE                    = 0x02000005;   // Block size (for block-wise compressors like bwt)
const int CELS_GET_COMPRESSION_CPU_LOAD         = 0x02000006;   // Percents of CPU load during compression. Value of 100 = 1 hardware thread
const int CELS_GET_DECOMPRESSION_CPU_LOAD       = 0x02000007;   // Percents of CPU load during decompression. Value of 100 = 1 hardware thread
const int CELS_GET_MINIMAL_INPUT_SIZE           = 0x02000008;   // Minimum input size the method is optimized for (f.e. LZ with 64 MB dictionary is optimized for minimum 64 MB of input data). Reducing this parameter may reduce memory usage without losing compression for the specified and lower input sizes.
const int CELS_GET_CACHING                      = 0x02000009;   // 1: memory can be kept allocated between (de)compression operations, 0: memory always released
const int CELS_GET_NAMED_SERVICE                = 0x0200000A;   // Service name (C string) passed in the inbuf, allowing to implement COMPRESSION_METHOD::doit()
// Set algorithm parameters (to the value specified by insize)
inline static int IS_CELS_SET_INSTANCE_PARAM_SERVICE (int service)  {return (service&0xFF000000)==0x03000000;}   // Family of CelsMain() "set param" codec instance services
const int CELS_SET_COMPRESSION_MEMORY           = 0x03000000;   // How much memory for compression?
const int CELS_SET_DECOMPRESSION_MEMORY         = 0x03000001;   // How much memory for decompression?
const int CELS_SET_MINIMUM_COMPRESSION_MEMORY   = 0x03000002;   // Minimal memory required for compression?
const int CELS_SET_MINIMUM_DECOMPRESSION_MEMORY = 0x03000003;   // Minimal memory required for decompression?
const int CELS_SET_DICTIONARY_SIZE              = 0x03000004;   // Dictionary size (for LZ and similar compressors)
const int CELS_SET_BLOCKSIZE                    = 0x03000005;   // Block size (for block-wise compressors like bwt)
const int CELS_SET_COMPRESSION_CPU_LOAD         = 0x03000006;   // Percents of CPU load during compression. Value of 100 = 1 hardware thread
const int CELS_SET_DECOMPRESSION_CPU_LOAD       = 0x03000007;   // Percents of CPU load during decompression. Value of 100 = 1 hardware thread
const int CELS_SET_MINIMAL_INPUT_SIZE           = 0x03000008;   // Minimum input size the method is optimized for (f.e. LZ with 64 MB dictionary is optimized for minimum 64 MB of input data). Reducing this parameter may reduce memory usage without losing compression for the specified and lower input sizes.
const int CELS_SET_CACHING                      = 0x03000009;   // 1: enable caching, 0: disable caching and release previously allocated memory
const int CELS_SET_NAMED_SERVICE                = 0x0300000A;   // Service name (C string) passed in the inbuf, allowing to implement COMPRESSION_METHOD::doit()
// CELS_[DE]COMPRESS* callbacks
const int CELS_READ                             = 0x10000000;   // Read up to inbytes bytes into inbuf. Retcode: <0 - error, 0 - EOF, >0 - amount of data read
const int CELS_WRITE                            = 0x10000001;   // Write outbytes bytes from outbuf. Retcode: the same
const int CELS_QUASI_WRITE                      = 0x10000002;   // "Quasi-write" just informs application how much data (= outsize) will be written as the result of (de)compression of already read data
const int CELS_PROGRESS                         = 0x10000003;   // Informs application that input was advanced by insize bytes, and output by outsize bytes
const int CELS_RECEIVE_FILLED_INBUF             = 0x10000004;   // Receive next filled input buffer from the queue: bufsize returned as result, bufptr stored in *inbuf
const int CELS_SEND_EMPTY_INBUF                 = 0x10000005;   // Send empty input buffer (inbuf,insize) into the queue
const int CELS_RECEIVE_EMPTY_OUTBUF             = 0x10000006;   // Receive next empty output buffer from the queue: bufsize returned as result, bufptr stored in *outbuf
const int CELS_SEND_FILLED_OUTBUF               = 0x10000007;   // Send filled output buffer (outbuf,outsize) into the queue
const int CELS_MEM_ALLOC                        = 0x10000008;   // Alloc outsize memory bytes and return pointer in *outbuf
const int CELS_MEM_FREE                         = 0x10000009;   // Free memory pointed by inbuf (should be implemented if and only if CELS_MEM_ALLOC is also implemented)

// Operations that can be implemented by codec in CelsMain()
inline static int IS_CELS_CODEC_SERVICE (int service)  {return (service&0xFF000000)==0x04000000;}   // Family of codec services
const int CELS_LOAD_CODEC                       = 0x04000000;   // Called on codec registration
const int CELS_UNLOAD_CODEC                     = 0x04000001;   // Called before codec unload
const int CELS_PARSE                            = 0x04000002;   // Parse method string

// Operations that can be implemented by module in CelsMain()
inline static int IS_CELS_MODULE_SERVICE (int service)  {return (service&0xFF000000)==0x05000000;}   // Family of module services
const int CELS_LOAD_MODULE                      = 0x05000000;   // Called on DLL load, allowing to register codecs
const int CELS_UNLOAD_MODULE                    = 0x05000001;   // Called before DLL unload

// Global operations implemented by Cels() itself
inline static int IS_CELS_GLOBAL_SERVICE (int service)  {return (service&0xFF000000)==0x06000000;}   // Family of global services
const int CELS_LOAD                             = 0x06000000;   // CelsLoad() == Load cels*.dll
const int CELS_UNLOAD                           = 0x06000001;   // CelsUnload() == Deregister all codecs and free all dlls
const int CELS_REGISTER                         = 0x06000002;   // CelsRegister(inbuf,ud,cb) == Register codec

// Code ranges reserved for applications and 3rd-party libraries
const int CELS_LIBRARY_CODES                    = 0x40000000;   // Codes available for 3rd-party libraries
const int CELS_RESERVED_CODES                   = 0x80000000;   // Codes reserved for future extensions
const int CELS_APPLICATION_CODES                = 0xC0000000;   // Codes available for applications

// Service parameters
const int CELS_UNPARSE_FULL                     = 0;    // Return string with canonical representation of the compression method
const int CELS_UNPARSE_DISPLAY                  = 1;    // Return method string prepared for display, with sensitive information like encryption keys removed
const int CELS_UNPARSE_PURE                     = 2;    // Return method string prepared for storing in archive, with sensitive information and compression-specific parameters removed

// Error codes
const int CELS_OK                               =   0;  // ALL RIGHT
const int CELS_ERROR_GENERAL                    =  -1;  // Some error when (de)compressing
const int CELS_ERROR_INVALID_COMPRESSOR         =  -2;  // Invalid compression method or parameters
const int CELS_ERROR_ONLY_DECOMPRESS            =  -3;  // Program was compiled with FREEARC_DECOMPRESS_ONLY, so don't try to use compress()
const int CELS_ERROR_OUTBLOCK_TOO_SMALL         =  -4;  // Output block size in (de)compressMem is not enough for all output data
const int CELS_ERROR_NOT_ENOUGH_MEMORY          =  -5;  // Can't allocate memory needed for (de)compression
const int CELS_ERROR_READ                       =  -6;  // Error when reading data
const int CELS_ERROR_BAD_COMPRESSED_DATA        =  -7;  // Data can't be decompressed
const int CELS_ERROR_NOT_IMPLEMENTED            =  -8;  // Requested feature isn't supported
const int CELS_ERROR_NO_MORE_DATA_REQUIRED      =  -9;  // Required part of data was already decompressed
const int CELS_ERROR_OPERATION_TERMINATED       = -10;  // Operation terminated by user
const int CELS_ERROR_WRITE                      = -11;  // Error when writing data
const int CELS_ERROR_BAD_CRC                    = -12;  // File failed CRC check
const int CELS_ERROR_BAD_PASSWORD               = -13;  // Password/keyfile failed checkcode test
const int CELS_ERROR_BAD_HEADERS                = -14;  // Archive headers are corrupted
const int CELS_ERROR_INTERNAL                   = -15;  // It should never happen: implementation error. Please report this bug to developers!

// Various sizes
const int CELS_MAX_PARSED_METHOD_SIZE           = 1024;
const int CELS_MAX_METHOD_STRING_SIZE           = 1024;
const int CELS_MAX_METHOD_PARAMETERS            =  200;
const char CELS_METHOD_PARAMETERS_DELIMITER     =  ':';

// Handy operation shortcuts
inline static CelsResult CelsRead  (CelsCallback* cb, void* ud, void* buf, CelsNum size)  {return cb(ud, CELS_READ,0,  buf,size, 0,0, 0,0);}
inline static CelsResult CelsWrite (CelsCallback* cb, void* ud, void* buf, CelsNum size)  {return cb(ud, CELS_WRITE,0, 0,0, buf,size, 0,0);}
inline static CelsResult CelsProgress (CelsCallback* cb, void* ud, CelsNum insize, CelsNum outsize)    {return cb(ud, CELS_PROGRESS,0, 0,insize, 0,outsize, 0,0);}
inline static CelsResult CelsReceiveFilledInbuf (CelsCallback* cb, void* ud, void** buf)               {return cb(ud, CELS_RECEIVE_FILLED_INBUF,0,  buf,0,    0,0, 0,0);}
inline static CelsResult CelsSendEmptyInbuf     (CelsCallback* cb, void* ud, void* buf, CelsNum size)  {return cb(ud, CELS_SEND_EMPTY_INBUF,0,      buf,size, 0,0, 0,0);}
inline static CelsResult CelsReceiveEmptyOutbuf (CelsCallback* cb, void* ud, void** buf)               {return cb(ud, CELS_RECEIVE_EMPTY_OUTBUF,0,  0,0,    buf,0, 0,0);}
inline static CelsResult CelsSendFilledOutbuf   (CelsCallback* cb, void* ud, void* buf, CelsNum size)  {return cb(ud, CELS_SEND_FILLED_OUTBUF,0,    0,0, buf,size, 0,0);}

// Ask host to alloc memory for us, falling back to malloc if host doesn't implement the service
inline static void* CelsMemAlloc (CelsCallback* cb, void* ud, CelsNum size)
{
    void* ptr = NULL;
    CelsResult result = cb? cb(ud, CELS_MEM_ALLOC,0, 0,0, &ptr,size, 0,0)
                          : CELS_ERROR_NOT_IMPLEMENTED;
    if (result == CELS_ERROR_NOT_IMPLEMENTED)
        ptr = malloc(size);
    return ptr;
}

// Free memory previously allocated by host (or malloc)
inline static void CelsMemFree (CelsCallback* cb, void* ud, void* buf)
{
    CelsResult result = cb? cb(ud, CELS_MEM_FREE,0, buf,0, 0,0, 0,0)
                          : CELS_ERROR_NOT_IMPLEMENTED;
    if (result == CELS_ERROR_NOT_IMPLEMENTED)
        free(buf);
}

inline static CelsResult CelsCompress  (const void* method, void* ud, CelsCallback* cb)  {return Cels(method, CELS_COMPRESS,0,   0,0, 0,0, ud,cb);}
inline static CelsResult CelsDecompress(const void* method, void* ud, CelsCallback* cb)  {return Cels(method, CELS_DECOMPRESS,0, 0,0, 0,0, ud,cb);}

inline static CelsResult CelsCanonize (const void* method, char* outbuf)  {return Cels(method, CELS_UNPARSE,CELS_UNPARSE_FULL,    0,0, outbuf,CELS_MAX_METHOD_STRING_SIZE, 0,0);}
inline static CelsResult CelsDisplay  (const void* method, char* outbuf)  {return Cels(method, CELS_UNPARSE,CELS_UNPARSE_DISPLAY, 0,0, outbuf,CELS_MAX_METHOD_STRING_SIZE, 0,0);}
inline static CelsResult CelsPurify   (const void* method, char* outbuf)  {return Cels(method, CELS_UNPARSE,CELS_UNPARSE_PURE,    0,0, outbuf,CELS_MAX_METHOD_STRING_SIZE, 0,0);}

inline static CelsResult CelsParse (const char* method_str, void* method)
        {return CelsParseStr (method_str, method,CELS_MAX_PARSED_METHOD_SIZE, 0,0);}

inline static CelsResult CelsGetMaxCompressedSize (const void* method, CelsNum size)
        {return Cels(method, CELS_GET_MAX_COMPRESSED_SIZE,0, 0,size, 0,0, 0,0);}

inline static CelsResult CelsFree (void* method)
        {return Cels(method, CELS_FREE,0, 0,0, 0,0, 0,(CelsCallback*)Cels);}

inline static CelsResult CelsGetNamedService (const void* method, const char* serviceName, CelsNum size)
        {return Cels(method, CELS_GET_NAMED_SERVICE,0, (void*)serviceName,size, 0,0, 0,0);}

inline static CelsResult CelsSetNamedService (void* method, const char* serviceName, CelsNum size, char* outbuf)
        {return Cels(method, CELS_SET_NAMED_SERVICE,0, (void*)serviceName,size, outbuf,CELS_MAX_METHOD_STRING_SIZE, 0,0);}

#define CELS_DEFINE_GETTER(function,code)                                                               \
inline static CelsResult CelsGet##function (const void* method)                                         \
{                                                                                                       \
    return Cels(method, CELS_GET_##code,0, 0,0, 0,0, 0,0);                                              \
}

#define CELS_DEFINE_SETTER(function,code)                                                               \
inline static CelsResult CelsSet##function (void* method, CelsNum size, char* outbuf)                   \
{                                                                                                       \
    return Cels(method, CELS_SET_##code,0, 0,size, outbuf,CELS_MAX_METHOD_STRING_SIZE, 0,0);            \
}

#define CELS_DEFINE_GETTER_AND_SETTER(function,code)                                                    \
CELS_DEFINE_GETTER(function,code)                                                                       \
CELS_DEFINE_SETTER(function,code)                                                                       \
                                                                                                        \
inline static CelsResult CelsLimit##function (void* method, CelsNum limit, char* outbuf)                \
{                                                                                                       \
    CelsResult size_or_errcode = CelsGet##function (method);                                            \
    if (size_or_errcode < CELS_OK)  return size_or_errcode;                                             \
    if (size_or_errcode < limit)    return outbuf? CelsCanonize(method,outbuf) : CELS_OK;               \
    return CelsSet##function (method, limit, outbuf);                                                   \
}

CELS_DEFINE_GETTER            (NumInputStreams ,    NUM_INPUT_STREAMS);
CELS_DEFINE_GETTER            (NumOutputStreams,    NUM_OUTPUT_STREAMS);
CELS_DEFINE_GETTER_AND_SETTER (CompressionMem,      COMPRESSION_MEMORY);
CELS_DEFINE_GETTER_AND_SETTER (DecompressionMem,    DECOMPRESSION_MEMORY);
CELS_DEFINE_GETTER_AND_SETTER (MinCompressionMem,   MINIMUM_COMPRESSION_MEMORY);
CELS_DEFINE_GETTER_AND_SETTER (MinDecompressionMem, MINIMUM_DECOMPRESSION_MEMORY);
CELS_DEFINE_GETTER_AND_SETTER (Dictionary,          DICTIONARY_SIZE);
CELS_DEFINE_GETTER_AND_SETTER (Blocksize,           BLOCKSIZE);
CELS_DEFINE_GETTER_AND_SETTER (CompressionCpuLoad,  COMPRESSION_CPU_LOAD);
CELS_DEFINE_GETTER_AND_SETTER (DecompressionCpuLoad,DECOMPRESSION_CPU_LOAD);
CELS_DEFINE_GETTER_AND_SETTER (MinimalInputSize,    MINIMAL_INPUT_SIZE);
CELS_DEFINE_GETTER_AND_SETTER (Caching,             CACHING);

#undef CELS_DEFINE_GETTER
#undef CELS_DEFINE_GETTER_AND_SETTER
#undef CELS_DEFINE_SETTER


// *** Extensions (not required for CELS functioning and use only official API) *******************************************

// Compress/decompress data with CELS_COMPRESS/CELS_DECOMPRESS services from/to buffers or using callbacks.
// Allow arbitrary buffer/callback combination for input and output data.
// Provides appropriate callback for codecs that doesn't support inbuf and/or outbuf.
CelsResult CelsCompressMem   (const void* method, void* inbuf, CelsNum insize, void* outbuf, CelsNum outsize, void* ud, CelsCallback* cb);
CelsResult CelsDecompressMem (const void* method, void* inbuf, CelsNum insize, void* outbuf, CelsNum outsize, void* ud, CelsCallback* cb);


// *** Stream processing helpers ******************************************************************************************

#define CELS_RETURN(result)                     {errcode = (result);  goto finished;}
#define CELS_RETURN2(result, default_errcode)   CELS_RETURN((result) < CELS_OK? (result) : (default_errcode))

#define CELS_WRITE_EXACTLY(buf, size) \
        {CelsResult result = CelsWrite(cb,ud, (buf),(size)); \
        if (result != (size))  CELS_RETURN2(result, CELS_ERROR_WRITE);}

// Usual way to read data going to compress. Should read less than `size` bytes only at EOF
#define CELS_READ_OR_EOF(len, buf, size) \
        {CelsResult result = CelsRead(cb,ud, (buf),(size)); \
        if (result == 0)       CELS_RETURN(CELS_OK); \
        if (result < CELS_OK)  CELS_RETURN(result); \
        (len) = result;}

// Useful only to read some structured (i.e. compressed) data. If we can't read as much, this means data are corrupt
#define CELS_READ_EXACTLY(buf, size) \
        {CelsResult result = CelsRead(cb,ud, (buf),(size)); \
        if (result != (size))  CELS_RETURN2(result, CELS_ERROR_BAD_COMPRESSED_DATA);}

// Same as above, but EOF means we have no more compressed blocks
#define CELS_READ_EXACTLY_OR_EOF(buf, size) \
        {CelsResult result = CelsRead(cb,ud, (buf),(size)); \
        if (result == 0)       CELS_RETURN(CELS_OK); \
        if (result != (size))  CELS_RETURN2(result, CELS_ERROR_BAD_COMPRESSED_DATA);}

// Read an integer value W bytes wide, using Intel byte-order
#define CELS_READ_INT_EXACTLY_OR_EOF(result, W) \
        {char buf[16]; \
        CELS_READ_EXACTLY_OR_EOF(buf, (W)); \
        (result) = CelsDeserializeInt(buf, (W));}

// Read buffer prefixed with size
#define CELS_READ_WITH_SIZE_OR_EOF(size,W, buf,bufSize) \
        {CELS_READ_INT_EXACTLY_OR_EOF((size), (W)); \
        if ((size) == 0)         CELS_RETURN(CELS_OK); \
        if ((size) > (bufSize))  CELS_RETURN(CELS_ERROR_BAD_COMPRESSED_DATA); \
        CELS_READ_EXACTLY((buf), (size));}

// Write buffer prefixed with size (first W bytes of buffer should be reserved reserved for the size value)
#define CELS_WRITE_WITH_SIZE(size,W, buf) \
        {CelsSerializeInt((size), (buf), (W)); \
        CELS_WRITE_EXACTLY((buf), (size) + (W));}

// Serialize the value into W bytes of the buffer, using Intel byte-order
inline static void CelsSerializeInt(CelsResult value, char* buf, int W)
{
    unsigned char* ptr = (unsigned char*) buf;
    while (W--) {
        *ptr++ = (unsigned char) value;
        value >>= 8;
    }
}

// Deserialize an integer value W bytes wide, using Intel byte-order
inline static CelsResult CelsDeserializeInt(void* buf, int W)
{
    unsigned char* ptr = (unsigned char*) buf;
    CelsResult result = 0;
    int shift = 0;
    while (W--) {
        result += (*ptr++ << shift);
        shift += 8;
    }
    return result;
}


#ifdef __cplusplus
}       // extern "C"
#endif

#endif // CELS_H
