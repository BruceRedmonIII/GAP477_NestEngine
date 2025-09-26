#define ZLIB_WINAPI
#include "Zipper.h"
#include "../Parsing/XMLFile.h"
#include "../Tiling/TileData.h"
#include "../Configuration/LogMacro.hint"
#include "../lib/zlib/zlib.h"
#include "../lib/BleachNew/BleachNew.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <SDL_rwops.h>
#include <ranges>
//-----------------------------------------------------------------------------------------------
// Zipper constructor
//-----------------------------------------------------------------------------------------------
nest::Zipper::Zipper(const char* filePath)
    : m_filePath(filePath)
    , m_pDirData(nullptr)
    , m_uncompressedSize(0)
    , m_zipHeader({})
    , m_fileHeader({})
    , m_dataHeader({})
    , m_pData({})
{
    m_pUncompressedData.reserve(200);
    StoreAllDataFromFile();
}

nest::Zipper::~Zipper()
{
    for (auto& data : std::views::values(m_pData))
    {
        if (data)
        {
            if (SDL_RWclose(data) != 0)
            {
                _LOG(nest::LogType::kError, SDL_GetError());
            }
        }
    }
    BLEACH_DELETE_ARRAY(m_pDirData);
    for (auto& uncompressedData : m_pUncompressedData)
    {
        BLEACH_DELETE(uncompressedData);
    }
}

//-----------------------------------------------------------------------------------------------
// Sets all the data by using the image path given, and then creates the image from the data if
// the headers name matches the file
//-----------------------------------------------------------------------------------------------
SDL_RWops* nest::Zipper::GetSDLRWops(const char* imagePath)
{
    auto it = m_pData.find(imagePath);
    // I need to copy the data from the zip file or else I cant gather multiple copies of the same image
    // EXAMPLE: If I load a button image, I want that button image to change colors and be manipulated without
    // affecting the other button images
    if (it != m_pData.end())
    {
        SDL_RWops* original = it->second;

        // Get the size of the original SDL_RWops
        size_t size = SDL_RWsize(original);

        // Allocate memory for the new buffer
        char* pBuffer = BLEACH_NEW(char[size]);

        // Reset the original SDL_RWops to the beginning (assuming SEEK_SET is supported)
        SDL_RWseek(original, 0, RW_SEEK_SET);

        // Read the data from the original SDL_RWops into the buffer
        SDL_RWread(original, pBuffer, 1, size);

        // Create a new SDL_RWops from the copied buffer
        SDL_RWops* newRwOps = SDL_RWFromConstMem(pBuffer, static_cast<int>(size));

        return newRwOps;
    }
    _LOG(LogType::kError, GenerateString("Failure to load path from zip!: ", imagePath));
    return nullptr;
}

nest::XMLFile* nest::Zipper::GetXMLFile(const char* filePath)
{
    auto it = m_pData.find(filePath);
    if (it != m_pData.end())
    {
        return BLEACH_NEW(XMLFile{ filePath });
    }
    _LOG(LogType::kError, GenerateString("Zipper failed to find XML File!: ", filePath));
    return nullptr;
}

const nest::TileData* nest::Zipper::GetTMXData(const char* filePath)
{
    auto it = m_pData.find(filePath);
    if (it != m_pData.end())
    {
        size_t size = SDL_RWsize(it->second);
        char* pBuffer = BLEACH_NEW(char[size]);
        SDL_RWread(it->second, pBuffer, 1, size);

        TileData* data = BLEACH_NEW(TileData( pBuffer, size ));
        return data;
    }
    return nullptr;
}

//------------------------------------------------------------------------------------------------
// Sets all the data inside the zipper to the data found within the header located at the fileName
//------------------------------------------------------------------------------------------------
void nest::Zipper::StoreAllDataFromFile()
{
    // Read the directory data
    // it's basically an array of file headers.
    // Open the assets zip.
    if (!m_filePath)
    {
        _LOG(LogType::kError, "No file path set!");
        return;
    }
    m_fileStream.open(m_filePath, std::ios::in | std::ios::binary);

    if (!m_fileStream.is_open())
    {
        _LOG(LogType::kError, "Open failed");
        return;
    }


    std::streamoff zipHeaderOffset = -static_cast<std::streamoff>(sizeof(m_zipHeader));
    m_fileStream.seekg(zipHeaderOffset, std::ios::end);

    m_fileStream.read(reinterpret_cast<char*>(&m_zipHeader), sizeof(m_zipHeader));

    if (m_zipHeader.signature != kZipSignature)
    {
        _LOG(LogType::kError, "Zip signature failed.");
        m_fileStream.close();
        return;
    }

    m_pDirData = BLEACH_NEW(char[m_zipHeader.dirSize]);
    memset(m_pDirData, 0, m_zipHeader.dirSize);

    std::streamoff dirOffset = m_zipHeader.dirOffset;
    m_fileStream.seekg(dirOffset, std::ios::beg);

    m_fileStream.read(m_pDirData, m_zipHeader.dirSize);

    // Read the file headers.
    FileHeader* pFileHeader = reinterpret_cast<FileHeader*>(m_pDirData);

    for (int iFile = 0; iFile < m_zipHeader.numFiles; ++iFile)
    {
        if (pFileHeader->signature != kFileSignature)
        {
            _LOG(LogType::kError, "File signature failed.");
            return;
        }

        char* pNameData = reinterpret_cast<char*>(pFileHeader + 1);
        char* pFileName = BLEACH_NEW(char[pFileHeader->nameLength + 1]);

        memcpy(pFileName, pNameData, pFileHeader->nameLength);
        pFileName[pFileHeader->nameLength] = NULL;

        // Get the data header and unzip the file.
        m_fileStream.seekg(pFileHeader->dataOffset, std::ios::beg);

        m_fileStream.read(reinterpret_cast<char*>(&m_dataHeader), sizeof(m_dataHeader));

        if (m_dataHeader.signature != kDataSignature)
        {
            _LOG(LogType::kError, "Data signature failed.");
            return;
        }

        m_fileStream.seekg(m_dataHeader.nameLength, std::ios::cur);

        // Allocate enough memory for the uncompressed data
        // plus one more byte to add a null-terminator just in case the file is text.

        m_uncompressedSize = m_dataHeader.uncompressedSize;
        char* pUncompressedData = BLEACH_NEW(char[m_dataHeader.uncompressedSize + 1]);

        if (m_dataHeader.compression == Z_DEFLATED)
        {
            char* pCompressedData = BLEACH_NEW(char[m_dataHeader.compressedSize]);
            m_fileStream.read(pCompressedData, m_dataHeader.compressedSize);

            z_stream zStream;

            zStream.next_in = reinterpret_cast<Bytef*>(pCompressedData);
            zStream.avail_in = m_dataHeader.compressedSize;

            zStream.next_out = reinterpret_cast<Bytef*>(pUncompressedData);
            zStream.avail_out = m_dataHeader.uncompressedSize;

            zStream.zalloc = static_cast<alloc_func>(0);
            zStream.zfree = static_cast<free_func>(0);

            int zStatus = inflateInit2(&zStream, -MAX_WBITS);

            if (zStatus == Z_OK)
            {
                inflate(&zStream, Z_FINISH);

                inflateEnd(&zStream);
            }
            BLEACH_DELETE_ARRAY(pCompressedData);
        }
        else
        {
            // Data is already uncompressed so read it as-is
            m_fileStream.read(pUncompressedData, m_dataHeader.uncompressedSize);
        }

        // Append the null-terminator just in case the data is text.
        pUncompressedData[m_dataHeader.uncompressedSize] = NULL;
        SDL_RWops* rwOps = SDL_RWFromConstMem(pUncompressedData, static_cast<int>(m_dataHeader.uncompressedSize));
        std::string name = pFileName;

        m_pData.try_emplace(name, rwOps);
        BLEACH_DELETE_ARRAY(pFileName);
        pFileName = nullptr;
        // Continue to the next file header
        pFileHeader = reinterpret_cast<FileHeader*>(pNameData + pFileHeader->nameLength);
        m_pUncompressedData.emplace_back(pUncompressedData);
    }
    m_fileStream.close();
}
