#pragma once
#include "../Math/MathAlias.h"
#include <limits>
#include <vector>
namespace nest
{
	struct Tile
	{
		size_t m_index = 0;
		int m_id = 0;
		int m_weight = 0;
		Vec2 m_pos{};
		Tile(int id)
			: m_id(id)
			, m_weight(0)
		{}
		Tile(int id, int weight)
			: m_id(id)
			, m_weight(weight)

		{}
		Tile(const Tile& copy, int index)
		{
			m_index = index;
			m_id = copy.m_id;
			m_weight = copy.m_weight;
		}
		Tile() = default;
	};
	struct TileNode
	{
		Tile m_tile{};
		//TileNode* m_tiles[kTotalDirectionSize] = { nullptr }; // connecting nodes
		std::vector<TileNode*> m_tiles{};
		int m_fScore = std::numeric_limits<int>::max();
		int m_gScore = std::numeric_limits<int>::max();
		// copy construct tiles
		TileNode(Tile tile)
			: m_tile(tile) { }
		~TileNode() = default;
		TileNode(const TileNode& copy) = delete;
		TileNode& operator=(const TileNode& copy) = delete;
		TileNode(const TileNode&& copy) = delete;
		TileNode& operator=(const TileNode&& copy) = delete;
		// adds a connecting node in the direction given
		void AddNode(TileNode* node)
		{
			m_tiles.emplace_back(node);
		}
		TileNode* GetCheapest()
		{
			TileNode* cheapestTile = nullptr;
			int cheapestCost = std::numeric_limits<int>::max();
			for (auto& tile : m_tiles)
			{
				if (tile == nullptr)
					continue;
				if (tile->m_tile.m_weight < cheapestCost)
				{
					cheapestTile = tile;
					cheapestCost = tile->m_tile.m_weight;
				}
			}
			return cheapestTile;
		}
	};
}