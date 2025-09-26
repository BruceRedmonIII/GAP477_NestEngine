#pragma once
#include "../Math/MathAlias.h"
#include "../SDLWrappers/Color.h"
#include "Tile.h"
#include "TileData.h"
#include "../Parsing/XMLNode.h"
#include <map>
#include <unordered_map>
#include <vector>
namespace nest
{
	using CameFromMap = std::map<TileNode*, TileNode*>;
	using TilePath = std::vector<nest::TileNode*>;
	// NOTE: Tiler dynamically allocates tile nodes! TODO:: don't do that
	// we could template this to create the array, then we would just get the TileData beforehand and use that as the 
	// template parameter for the array, but we would then need to change the nodes as well to not use pointers
	// to each other. Memory pool is most likely the best option
	class Tiler
	{
		TileData m_data;
		std::unordered_map<size_t, TileNode*> m_tileMap;
		std::vector<Tile> m_tileTemplates;
#if _SAVE_PATH_SEARCH_DATA == 1
		std::vector<int> m_tilesSearched;
#endif
	public:
		Tiler(const char* tmxLocation, const char* xmlFileLocation)
			: m_data(tmxLocation)
		{
			ParseTileXML(xmlFileLocation);
		}
		~Tiler();
		TileNode* GetNodeFromPos(Vec2 pos);
		TilePath PathFind(size_t startIndex, size_t goalIndex);
		int HeuristicCalculation(size_t start, size_t end);
		int HeuristicCalculationUsingNodes(TileNode* current, TileNode* goal);
		Vec2 GetTileSize() { return { m_data.GetTileMapData().m_tileWidth, m_data.GetTileMapData().m_tileHeight }; }
	private:
		// path finding functions
		TilePath ReconstructPath(CameFromMap cameFrom, TileNode* other);
		int DistanceBetween(TileNode* left, TileNode* right);
		void LinkNode(size_t index);

		// parse functions
		void ParseTileXML(const char* xmlFileLocation);
		void CreateTileTemplate(const XMLNode node);
		void CreateTileFromPreset(size_t index, int id);
		void ConnectNorth(size_t index);
		void ConnectNorthWest(size_t index);
		void ConnectWest(size_t index);
		void ConnectNorthEast(size_t index);
		void ResetNodeScores();
	public:
		// test functions
		void DrawTile(TileNode* node, Color color, bool fill);
		void DrawIndex(int id, Color color, bool fill);
		void DrawSearched(Color color, bool fill);
		void DrawConnecting(int id, Color color, bool fill);
		void ClearSearchedNodes();
		void PrintMap();
		void TestNode(Vec2 pos);
		void TestDraw(int tileId, Color color, bool fill = false);
	};
}
