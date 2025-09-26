#include "TileData.h"
#include "../Configuration/LogMacro.hint"
#include "../lib/BleachNew/BleachNew.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <assert.h>
nest::TileData::TileData(const char* filePath)
    : m_encoding({})
{
    AllocateTileData(filePath);
    if (m_data)
        ParseAllocatedData();
}

nest::TileData::TileData(char*& data, size_t size)
    : m_encoding({})
    , m_dataSize(size)
{
    memmove(&m_data, &data, sizeof(data));
    ParseAllocatedData();
}

nest::TileData::~TileData()
{
    BLEACH_DELETE(m_data);
}

void nest::TileData::AllocateTileData(const char* filePath)
{
    std::ifstream fs{ filePath, std::ios::in };
    if (fs.is_open())
    {
        uintmax_t fileSize = std::filesystem::file_size(filePath);
        m_data = BLEACH_NEW(char[fileSize]);
        fs.read(m_data, static_cast<std::streamsize>(fileSize));
        m_dataSize = static_cast<int>(fileSize);
        fs.close();
    }
    else
        _LOG(LogType::kError, GenerateString("File failed! : ", filePath));
}

void nest::TileData::ParseAllocatedData()
{
    Context context;

    ProcessHeaderData(context);
    ProcessMapData(context);
    ProcessTilesetData(context);
    ProcessLayerData(context);
    ProcessGameMapData(context);


#if _TEST_NOTES_LOGGING == 1
    _LOG(GenerateString("Logging parsed tile file (everything but the enums) - | xml-version=",
        m_xmlVersion," | xml-encoding=", m_encoding, " | map-version=", m_tileMapData.m_version, " | tiled-version=",
        m_tileMapData.m_tiledVersion, " | map-width=",
        m_tileMapData.m_width, " | nmap-height=", m_tileMapData.m_height, " | tile-width=", m_tileMapData.m_tileWidth, " | tile-height=",
        m_tileMapData.m_tileHeight, " | map-infnite=", m_tileMapData.m_infinite, "\nnext-layerId=", m_tileMapData.m_nextLayerId, " | next-ObjectId=",
        m_tileMapData.m_nextObjectId, " | map-firstGrid=", m_tileSetData.m_firstGrid, " | tile-source=", m_tileSetData.m_source, " | layer-id=",
        m_layerData.m_id, " | layer-name=", m_layerData.m_name, " | layer-width=", m_layerData.m_width, " | layer-height=", m_layerData.m_height));
#endif
        
    return;
}

const std::string& nest::TileData::FindNextElement(Context& context)
{
    assert(static_cast<uint64_t>(context.currentPosition) < m_dataSize);
    //clear any remaining temp values
    context.tempValue.clear();

    int counter = 1;
    char* pPointer = m_data;
    pPointer += context.currentPosition;

    // look for this delim to start the data collection
    while (*pPointer != context.startAtDelim)
    {
        ++pPointer;
        ++counter;
    }

    // after found, iterate once to get to the data
    ++pPointer;
    ++counter;

    //while not pointing at the end delim, 
    // add to the counter to store position,
    // and add the value to temp value
    while (*pPointer != context.endAtDelim)
    {
        if (*pPointer != context.endAtDelim)
        {
            context.tempValue += *pPointer;
            ++counter;
        }
        ++pPointer;
    }
    // add the incremented amount to the current position
    context.currentPosition += counter;

    return context.tempValue;
}

void nest::TileData::ProcessHeaderData(Context& context)
{
    m_xmlVersion = std::stof(FindNextElement(context));
    m_encoding = FindNextElement(context);
}

void nest::TileData::ProcessMapData(Context& context)
{
    m_tileMapData.m_version = std::stof(FindNextElement(context));
    m_tileMapData.m_tiledVersion = std::stof(FindNextElement(context));

    FindNextElement(context);
    m_tileMapData.m_orientation =
        context.tempValue[0] == 'o' ? TileMapData::OrientationFlags::kOrthogonal :
        context.tempValue[0] == 'i' ? TileMapData::OrientationFlags::kIsometric :
        TileMapData::OrientationFlags::kHexagonal;

    FindNextElement(context);
    m_tileMapData.m_renderOrder =
        context.tempValue[0] == 'r' && context.tempValue[6] == 'd' ? TileMapData::RendOrderFlags::kRightDown :
        context.tempValue[0] == 'r' && context.tempValue[6] == 'u' ? TileMapData::RendOrderFlags::kRightDown :
        context.tempValue[0] == 'l' && context.tempValue[6] == 'd' ? TileMapData::RendOrderFlags::kLeftDown :
        TileMapData::RendOrderFlags::kLeftUp;

    m_tileMapData.m_columns = static_cast<uint16_t>(std::stoi(FindNextElement(context)));
    m_tileMapData.m_rows = static_cast<uint16_t>(std::stoi(FindNextElement(context)));
    m_tileMapData.m_tileWidth = static_cast<uint16_t>(std::stoi(FindNextElement(context)));
    m_tileMapData.m_tileHeight = static_cast<uint16_t>(std::stoi(FindNextElement(context)));
    m_tileMapData.m_infinite = static_cast<bool>(std::stoi(FindNextElement(context)));
    m_tileMapData.m_nextLayerId = static_cast<uint16_t>(std::stoi(FindNextElement(context)));
    m_tileMapData.m_nextObjectId = static_cast<uint16_t>(std::stoi(FindNextElement(context)));
}

void nest::TileData::ProcessGameMapData(Context& context)
{
    FindNextElement(context);
    m_tileMapData.m_encoding =
        context.tempValue[0] == 'c' ? TileMapData::DataEncodingFlags::kCsv :
        context.tempValue[0] == 'x' ? TileMapData::DataEncodingFlags::kXml :
        TileMapData::DataEncodingFlags::kBase64;

    context.startAtDelim = '>';
    context.endAtDelim = '<';
    m_gameMap = static_cast<std::string>(FindNextElement(context));
}

void nest::TileData::ProcessTilesetData(Context& context)
{
    m_tileSetData.m_firstGrid = static_cast<uint16_t>(std::stoi(FindNextElement(context)));
    m_tileSetData.m_source = FindNextElement(context);
}

void nest::TileData::ProcessLayerData(Context& context)
{
    m_layerData.m_id = static_cast<uint16_t>(std::stoi(FindNextElement(context)));
    m_layerData.m_name = FindNextElement(context);
    m_layerData.m_width = static_cast<uint16_t>(std::stoi(FindNextElement(context)));
    m_layerData.m_height = static_cast<uint16_t>(std::stoi(FindNextElement(context)));
}
