#include "GexLZSS.hpp"

#include <cstring>
#include <array>
#include <cstdio>
#include <vector>

typedef unsigned int uint32_t;
typedef unsigned char byte;

class FileReader
{
    byte* data;
    size_t index, size;
public:
    FileReader(unsigned char* block, size_t size, size_t index)
        : data(block), index(index), size(size) {}

    byte ReadByte()
    {
        if (eof()) return 0;

        byte b = data[index];
        ++index;
        return b;
    }

    bool eof()
    {
        return index >= size;
    }

    bool empty() { return data == nullptr || size == 0; }
};

enum struct BlockType : int
{
    LiteralByte = 0,
    Command = 1
};

byte ReverseByte(byte b)
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

std::array<BlockType, 8> GetBlockTypesFromByte(byte b)
{
    std::array<BlockType, 8> result;
    byte reversedByte = ReverseByte(b);

    for (int i = 0; i < 8; ++i)
    {
        byte temp = (reversedByte & (1U << (7 - i))) >> (7 - i);
        result[i] = (BlockType)(temp);
    }

    return result;
}

typedef std::array<BlockType, 16> layout_t;

layout_t GetBlockTypesFromBytes(byte b1, byte b2)
{
    layout_t result;
    
    std::array<BlockType, 8> r[2] = {GetBlockTypesFromByte(b1), GetBlockTypesFromByte(b2)};

    for(int i = 0; i < 2; ++i)
    {
        for(int j = 0; j < 8; ++j)
        {
            result[i * 8 + j] = r[i][j];
        }
    }
    
    return result;
}

namespace GexLZSS {

    /// Source of original code: https://github.com/TheSerioliOfNosgoth/SoulSpiral-Official/
    ErrorCode_t decompressMemory(unsigned char* blockIn, int blockSize, unsigned char*& blockOut, int* optOutSizeUncompressedBlock, unsigned int optBlockInOffset)
    {
        FileReader reader(blockIn, blockSize + optBlockInOffset, optBlockInOffset);
        if (reader.eof() || reader.empty()) return ErrorCode::READERERROR;

        std::vector<byte> uncompressedBlock;
        do
        {
            byte layoutByte1 = reader.ReadByte(), layoutByte2 = reader.ReadByte();

            layout_t currentLayout = GetBlockTypesFromBytes(layoutByte1, layoutByte2);
            
            for(size_t i = 0; i < currentLayout.size(); ++i)
            {
                if (currentLayout[i] == BlockType::Command)
                {
                    byte controlByte1 = reader.ReadByte(), controlByte2 = reader.ReadByte();
                    
                    if ((controlByte1 & 0xF0) != 0)
                    {
                        int numBytes = 0;
                        int startOffset = 0;
                        unsigned short backDist = 0;

                        numBytes = ((controlByte1 & 0x0F) + 1);
                        backDist = controlByte2 | (((unsigned short)(controlByte1 & 0xF0)) << 4);

                        startOffset = (int)uncompressedBlock.size() - backDist;
                        for (int j = 0; j < numBytes; ++j)
                        {
                            if ((startOffset + j) >= 0)
                                uncompressedBlock.push_back(uncompressedBlock[startOffset + j]);
                        }
                    }
                    else
                    {
                        int startOffset = (int)uncompressedBlock.size() - controlByte2;
                        int numBytes = controlByte1 + 1;

                        for(int j = 0; j < numBytes; ++j)
                            uncompressedBlock.push_back(uncompressedBlock[startOffset + j]);
                    }
                }
                else
                    uncompressedBlock.push_back(reader.ReadByte());
            }

        } while (!reader.eof());

        if (optOutSizeUncompressedBlock)
            *optOutSizeUncompressedBlock = uncompressedBlock.size();
            
        blockOut = new byte[uncompressedBlock.size()];
        memcpy(blockOut, uncompressedBlock.data(), uncompressedBlock.size());

        return ErrorCode::SUCCESS;
    }

    ErrorCode_t decompressFile(const char* filePathIn, const char* filePathOut)
    {
        FILE* f = fopen(filePathIn, "rb");
        if (!f) return ErrorCode::FILENOTFOUND;
        fseek(f, 0, SEEK_END);
        size_t s = ftell(f);
        rewind(f);
        unsigned char* data = new unsigned char[s];
        fread(data, s, 1, f);
        fclose(f);

        unsigned char* decompressedData = NULL;
        int sizeUncompressedFile;
        ErrorCode_t res = decompressMemory(data, s, decompressedData, &sizeUncompressedFile);
        if (decompressedData)
        {
            f = fopen(filePathOut, "wb");
            if (f)
            {
                fwrite(decompressedData, sizeUncompressedFile, 1, f);
                fclose(f);
                delete[] decompressedData;
            }
            else
                res |= ErrorCode::WRITEERROR;
        }
        delete[] data;
        return res;
    }

    const unsigned char* FindBestMatch(const unsigned char* startOfMatch, int maxBacksearchSize, int& matchSize)
    {
        const auto matchLength = [](const unsigned char* a, const unsigned char* b, const int maxLength) -> int
        {
            for(int i = 0; i < maxLength; ++i)
            {
                if (a[i] != b[i]) return i;
            }
            return maxLength;
        };

        const unsigned char* bestPtr = nullptr;
        matchSize = 0;
        const unsigned char* const backBuffer = startOfMatch;

        for (int i = 0; i < maxBacksearchSize; ++i)
        {
            const int n = matchLength(startOfMatch, backBuffer - i, std::min(i, 0xF));
            if (n > 0 && n > matchSize)
            {
                bestPtr = backBuffer - i;
                matchSize = n;
            }
        }

        return matchSize > 2 ? bestPtr : nullptr; // If it takes two byte to write one or two bytes, we might as well just copy them raw
    }

    ErrorCode_t compressMemory(unsigned char* blockIn, int blockSize, unsigned char*& blockOut, int* outSizeCompressedBlock)
    {
        std::vector<unsigned char> buffer;
        unsigned char* current = blockIn;

        auto eof = [&current, &blockSize, &blockIn]() -> bool
        {
            return (size_t)(current - blockIn) >= (size_t)blockSize;
        };

        while (!eof())
        {
            const size_t sectionIndex = buffer.size();
            buffer.push_back(0);
            buffer.push_back(0);

            for(int byteIndex = 0; byteIndex < 2; ++byteIndex)
            {
                for(int bitIndex = 0; bitIndex < 8; ++bitIndex)
                {
                    if (eof())
                    {
                        // Fill end with zeroes so the algorithm works properly
                        buffer.push_back(0);
                        buffer[sectionIndex + byteIndex] >>= 1;
                        continue;
                    }

                    int len = 0;
                    const unsigned char* ptr = FindBestMatch(current, std::min((size_t)0xFFF, (size_t)(current - blockIn)), len);
                    if (ptr == nullptr)
                    {
                        buffer.push_back(*current);
                        ++current;
                        buffer[sectionIndex + byteIndex] >>= 1;
                    }
                    else
                    {
                        buffer.push_back((len - 1) | ((current - ptr) & 0xF00) >> 4);
                        buffer.push_back((current - ptr) & 0xFF);
                        current += len;
                        buffer[sectionIndex + byteIndex] = (buffer[sectionIndex + byteIndex] >> 1) | 0b10000000;
                    }
                }
            }
        }

        blockOut = new unsigned char[buffer.size()];
        if (!blockOut)
        {
            return ErrorCode::OUTOFMEMORY;
        }

        if (outSizeCompressedBlock)
            *outSizeCompressedBlock = (int)buffer.size();

        memcpy(blockOut, buffer.data(), buffer.size());

        return ErrorCode::SUCCESS;
    }

    ErrorCode_t compressFile(const char* filePathIn, const char* filePathOut)
    {
        FILE* f = fopen(filePathIn, "rb");
        if (!f) return ErrorCode::FILENOTFOUND;
        fseek(f, 0, SEEK_END);
        size_t s = ftell(f);
        rewind(f);
        unsigned char* data = new unsigned char[s];
        fread(data, s, 1, f);
        fclose(f);

        unsigned char* compressedData = NULL;
        int sizeCompressedFile;
        ErrorCode_t res = compressMemory(data, s, compressedData, &sizeCompressedFile);
        if (compressedData)
        {
            f = fopen(filePathOut, "wb");
            if (f)
            {
                fwrite(compressedData, sizeCompressedFile, 1, f);
                fclose(f);
                delete[] compressedData;
            }
            else
                res |= ErrorCode::WRITEERROR;
        }
        delete[] data;
        return res;
    }

    const char* ERROR_MESSAGES[] = {
        "No error.",
        "Input file can not be opened.",
        "Reader failed to initialize with given data.",
        "Output file could not be written to.",
        "Memory could not be allocated for output buffer."
    };

    void PrintErrors(ErrorCode_t error)
    {
        if (error == ErrorCode::SUCCESS)
        {
            puts(ERROR_MESSAGES[0]);
            return;
        }
        
        for(unsigned int i = 0; i < 8 * sizeof(ErrorCode_t); ++i)
        {
            if (error & (1U << i))
                puts(ERROR_MESSAGES[i + 1]);
        }
    }
}