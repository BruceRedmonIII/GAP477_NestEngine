#pragma once
#include <string>

namespace nest
{
    struct TileMapData
    {
        enum OrientationFlags
        {
            kOrthogonal,
            kIsometric,
            kHexagonal
        };
        enum RendOrderFlags
        {
            kRightDown,
            kRightUp,
            kLeftDown,
            kLeftUp
        };
        enum DataEncodingFlags
        {
            kCsv,
            kXml, // deprecated!
            kBase64,
        };
        enum DataEncodingCompression
        {
            kNothing,
            kUncompressed,
            kGZipCompressed, // deprecated!
            kZLibCompressed,
            kZStandardCompressed
        };
        bool m_infinite;
        float m_version;
        float m_tiledVersion;
        uint16_t m_columns;
        uint16_t m_rows;
        uint16_t m_tileWidth;
        uint16_t m_tileHeight;
        uint16_t m_nextLayerId;
        uint16_t m_nextObjectId;
        DataEncodingFlags m_encoding;
        OrientationFlags m_orientation;
        RendOrderFlags m_renderOrder;
        DataEncodingCompression m_compression;
    };
}