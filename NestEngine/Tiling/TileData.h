#pragma once
#include "../Tiling/LayerData.h"
#include "TileMapData.h"
#include "TileSetData.h"

namespace nest
{
    class TileData
    {

        struct Context
        {
            int currentPosition = 0;
            char startAtDelim = '"';
            char endAtDelim = '"';
            std::string tempValue = {};
        };
        char* m_data;
        size_t m_dataSize;
        std::string m_encoding;
        std::string m_gameMap;

        LayerData m_layerData;
        TileSetData m_tileSetData;
        TileMapData m_tileMapData;

        // does not currently support multiple layers
        int m_layerCount = 1;
        float m_xmlVersion;

    public:
        TileData(const char* filePath);
        TileData(char*& data, size_t size);
        ~TileData();
        TileData(const TileData& copy) = delete;
        TileData& operator=(const TileData& copy) = delete;

        const std::string& GetEncoding() const { return m_encoding; }
        const std::string& GetGameMap() const { return m_gameMap; }
        LayerData GetLayerData() const { return m_layerData; }
        TileSetData GetTileSetData() const { return m_tileSetData; }
        TileMapData GetTileMapData() const { return m_tileMapData; }
        float GetXmlVersion() const { return m_xmlVersion; }
        int GetLayerCount() const { return m_layerCount; }

    private:
        void AllocateTileData(const char* filePath);
        void ParseAllocatedData();
        const std::string& FindNextElement(Context& context);
        void ProcessHeaderData(Context& context);
        void ProcessMapData(Context& context);
        void ProcessTilesetData(Context& context);
        void ProcessLayerData(Context& context);
        void ProcessGameMapData(Context& context);
    };
}
