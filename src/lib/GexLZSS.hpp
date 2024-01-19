#pragma once

namespace GexLZSS
{
    using ErrorCode_t = unsigned char;
    struct ErrorCode
    {
        static const ErrorCode_t
            FILENOTFOUND = 0x01,
            READERERROR = 0x02,
            WRITEERROR = 0x04,


            SUCCESS = 0x00;
    };

    void PrintErrors(ErrorCode_t error);

    ErrorCode_t decompress(const char* filePathIn, const char* filePathOut);
    ErrorCode_t decompress(unsigned char* blockIn, int blockSize, unsigned char*& blockOut, int* outSizeUncompressedBlock = nullptr, unsigned int optBlockInOffset = 0);
}