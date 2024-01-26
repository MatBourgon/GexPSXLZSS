#pragma once

#if defined(GEXLZSS_SHARED)
    #define GEXLZSS_API __stdcall __declspec(dllexport)
    int __stdcall DllMain(void*,unsigned int,void*) { return 1; }
#else
    #define GEXLZSS_API
#endif

namespace GexLZSS
{

    using ErrorCode_t = unsigned char;
    struct ErrorCode
    {
        static const ErrorCode_t
            FILENOTFOUND = 0x01,
            READERERROR = 0x02,
            WRITEERROR = 0x04,
            OUTOFMEMORY = 0x08,

            SUCCESS = 0x00;
    };

    void PrintErrors(ErrorCode_t error);

    extern "C" {
        ErrorCode_t GEXLZSS_API decompressMemory(unsigned char* blockIn, int blockSize, unsigned char*& blockOut, int* outSizeUncompressedBlock = nullptr, unsigned int optBlockInOffset = 0);
        ErrorCode_t GEXLZSS_API decompressFile(const char* filePathIn, const char* filePathOut);
    
        ErrorCode_t GEXLZSS_API compressMemory(unsigned char* blockIn, int blockSize, unsigned char*& blockOut, int* outSizeCompressedBlock = nullptr);
        ErrorCode_t GEXLZSS_API compressFile(const char* filePathIn, const char* filePathOut);
    }
}