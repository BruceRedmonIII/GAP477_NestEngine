#pragma once
#include "../Tiling/TileData.h"
#include <cstdint>
#include <fstream>
#include <unordered_map>
struct SDL_RWops;
struct SDL_Surface;
namespace nest
{
    class XMLFile;
#pragma pack(1)
    struct ZipHeader
    {
        uint32_t signature = 0;
        uint16_t diskIndex = 0;
        uint16_t startDisk = 0;
        uint16_t numFiles = 0;
        uint16_t totalFiles = 0;
        uint32_t dirSize = 0;
        uint32_t dirOffset = 0;
        uint16_t commentLength = 0;
    };
#pragma pack()

    static constexpr uint32_t kZipSignature = 0x06054b50;

#pragma pack(1)
    struct FileHeader
    {
        uint32_t signature = 0;
        uint16_t versionMade = 0;
        uint16_t versionNeeded = 0;
        uint16_t flag = 0;
        uint16_t compression = 0;
        uint16_t time = 0;
        uint16_t date = 0;
        uint32_t crcCode = 0;
        uint32_t compressedSize = 0;
        uint32_t uncompressedSize = 0;
        uint16_t nameLength = 0;
        uint16_t extraLength = 0;
        uint16_t commentLength = 0;
        uint16_t startDisk = 0;
        uint16_t internalAttributes = 0;
        uint32_t externalAttributes = 0;
        uint32_t dataOffset = 0;
    };
#pragma pack()

    static constexpr uint32_t kFileSignature = 0x02014b50;

#pragma pack(1)
    struct DataHeader
    {
        uint32_t signature = 0;
        uint16_t version = 0;
        uint16_t flag = 0;
        uint16_t compression = 0;
        uint16_t time = 0;
        uint16_t date = 0;
        uint32_t crcCode = 0;
        uint32_t compressedSize = 0;
        uint32_t uncompressedSize = 0;
        uint16_t nameLength = 0;
        uint16_t extraLength = 0;
    };
#pragma pack()

    static constexpr uint32_t kDataSignature = 0x04034b50;
    class Zipper
    {
        const char* m_filePath;
        std::vector<char*> m_pUncompressedData{};
        char* m_pDirData;
        uint32_t m_uncompressedSize;
        std::fstream m_fileStream;
        ZipHeader m_zipHeader;
        FileHeader m_fileHeader;
        DataHeader m_dataHeader;
        std::unordered_map<std::string, SDL_RWops*> m_pData;
    public:
        Zipper() = delete;
        Zipper(const char* filePath);
        ~Zipper();
        SDL_RWops* GetSDLRWops(const char* imagePath);
        nest::XMLFile* GetXMLFile(const char* filePath);
        const nest::TileData* GetTMXData(const char* filePath);
    private:
        void StoreAllDataFromFile();
    };
}