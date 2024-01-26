#include "lib/GexLZSS.hpp"

#include <iostream>

unsigned char* readFile(const char* path, size_t* outSize = nullptr)
{
    FILE* f = fopen(path, "rb");
    if (f)
    {
        fseek(f, 0, SEEK_END);
        size_t s = ftell(f);
        rewind(f);
        unsigned char* b = new unsigned char[s];
        fread(b, s, 1, f);
        fclose(f);
        if (outSize)
            *outSize = s;
        return b;
    }
    return nullptr;
}

struct BIGFILE_t
{
    using uint32_t = unsigned int;
    uint32_t nEntries;
    struct BIGFILEEntry_t
    {
        uint32_t pathHash, uncompressed_size, compressed_size, fileOffset;
        uint32_t _[2];
    } entries[];
};

using ull = unsigned long long;
void GenerateFileInfo(unsigned char* file, ull fileLength, ull& outXor, ull& outSum)
{
    outXor = outSum = 0;
    for(ull i = 0; i < fileLength; ++i)
    {
        outXor ^= (i * file[i]);
        outSum += file[i];
    }
}

bool TestFile(const char* path, size_t hashToMatch)
{
    size_t size = 0;
    unsigned char* data = readFile(path, &size);
    if (!data) return false;
    
    ull outXor, outSum;
    GenerateFileInfo(data, size, outXor, outSum);
    bool result = hashToMatch == (outXor ^ outSum);
    delete[] data;
    return result;
}

int main()
{
    constexpr size_t hashToMatch = 0x01358066;
    
    GexLZSS::decompressFile("./warning.tim.bin", "./warning1.tim");

    BIGFILE_t* bigfile = (BIGFILE_t*)readFile("./bigfile.dat");
    
    unsigned char* b = NULL;
    int uncompressed_size;
    GexLZSS::decompressMemory((unsigned char*)bigfile, bigfile->entries[1].compressed_size, b, &uncompressed_size, bigfile->entries[1].fileOffset);

    if (b == nullptr)
    {
        printf("Decompression failed!");
        exit(EXIT_FAILURE);
    }
    
    FILE* f = fopen("./warning2.tim", "wb");
    fwrite(b, uncompressed_size, 1, f);
    fclose(f);
    delete[] b;
        
    delete[] bigfile;

    bool results[] = {
        TestFile("./warning1.tim", hashToMatch), TestFile("./warning2.tim", hashToMatch)
    };

    switch((results[0] ? 0b01 : 0) | (results[1] ? 0b10 : 0))
    {
        case 0b00:
            puts("All tests failed!");
            break;

        case 0b01:
            puts("In-Memory test failed!");
            break;

        case 0b10:
            puts("From-file test failed!");
            break;

        case 0b11:
            puts("All tests passed!");
            return EXIT_SUCCESS;
    }

    return EXIT_SUCCESS;
}