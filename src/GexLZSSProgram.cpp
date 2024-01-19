#include "lib/GexLZSS.hpp"

#include <iostream>
#include <string>
#include <unordered_map>

using std::string;

using ArgMap_t = std::unordered_map<string, string>;

ArgMap_t FormArgs(int argc, const char** argv)
{
    ArgMap_t map;
    for(int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-' && (i + 1))
        {
            string key;
            for(int j = 0; argv[i][j] != '\0'; ++j)
            {
                if (argv[i][j] != '-')
                {
                    key = string(&argv[i][j]);
                    break;
                }
            }

            for(auto& c : key)
            {
                if ('A' <= c && c <= 'Z')
                    c += 'a' - 'A';
            }

            if (i + 1 < argc && argv[i + 1][0] != '-')
            {
                map[key] = string(argv[i + 1]);
                ++i;
            }
            else
            {
                map[key] = "";
            }
        }
    }

    return map;
}

enum ArgumentFoundType
{
    NotFound = 0,
    FoundShort = 1,
    FoundLong = 2
};

ArgumentFoundType ArgumentExists(const ArgMap_t& map, const string& keyShort, const string& keyLong)
{
    if (map.find(keyShort) != map.end())
    {
        return ArgumentFoundType::FoundShort;
    }

    if (map.find(keyLong) != map.end())
    {
        return ArgumentFoundType::FoundLong;
    }
    
    return ArgumentFoundType::NotFound;
}

bool GetArgument(const ArgMap_t& map, const string& keyShort, const string& keyLong, string& outValue)
{
    switch(ArgumentExists(map, keyShort, keyLong))
    {
        case ArgumentFoundType::FoundShort:
            outValue = map.at(keyShort);
            return true;

        case ArgumentFoundType::FoundLong:
            outValue = map.at(keyLong);
            return true;

        default:
            return false;
    }
}

void ShowHelp(const char* programName)
{
    printf("help: %s --input /input/compressed/file --output /output/uncompressed/file\n", programName);
    puts("  --input, -i: File to decompress");
    puts("  --output, -o: File to write to after decompressing");
}

int main(int argc, const char* argv[])
{
    ArgMap_t map = FormArgs(argc, argv);

    if (ArgumentExists(map, "h", "help"))
    {
        ShowHelp(argv[0]);
        return EXIT_SUCCESS;
    }

    string outputPath, inputPath;

    if ( !GetArgument(map, "o", "output", outputPath) || !GetArgument(map, "i", "input", inputPath) )
    {
        puts("Incorrect usage!");
        ShowHelp(argv[0]);
        return EXIT_FAILURE;
    }

    GexLZSS::ErrorCode_t err = GexLZSS::decompress(inputPath.c_str(), outputPath.c_str());
    if (err != GexLZSS::ErrorCode::SUCCESS)
    {
        puts("Error(s) when trying to decompress file!");
        GexLZSS::PrintErrors(err);
        return EXIT_FAILURE;
    }

    printf("Successfully decompressed file %s and written to %s.\n", inputPath.c_str(), outputPath.c_str());

    return EXIT_SUCCESS;
}