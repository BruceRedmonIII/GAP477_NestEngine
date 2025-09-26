#include "Tiler.h"
#include "../Configuration/Macros.h"
#include "../Engine/Engine.h"
#include "../Managers/GraphicsSystem.h"
#include "../Parsing/XMLFile.h"
#include "../Managers/LogManager.h"
#include "../lib/BleachNew/BleachNew.h"
#include <algorithm>
#include <iostream>
#include <queue>

nest::Tiler::~Tiler()
{
	for (auto& tileNode : m_tileMap)
		BLEACH_DELETE(tileNode.second);
}

nest::TileNode* nest::Tiler::GetNodeFromPos(Vec2 pos)
{
	int tileWidth = m_data.GetTileMapData().m_tileWidth;
	int tileHeight = m_data.GetTileMapData().m_tileHeight;
	int rowWidth = m_data.GetTileMapData().m_columns;
	// x moves forward columns, if tile width is 32, and x is 64. then 64/32 = column 2
	// y moves down rows
	int column = static_cast<int>(std::round(pos.x / tileWidth));
	int row = static_cast<int>(std::round(pos.y / tileHeight));
	size_t index = (row * rowWidth) + column;

	if (row >= rowWidth)
	{
		_LOG_V(LogType::kError, "Index out of range!");
		return nullptr;
	}
	return m_tileMap[index];
}

nest::TilePath nest::Tiler::ReconstructPath(CameFromMap cameFrom, TileNode* current)
{
	std::vector<TileNode*> path;
	path.push_back(current);
	while (cameFrom.find(current) != cameFrom.end())	//	while current in cameFrom.Keys:
	{
		current = cameFrom[current];//  current := cameFrom[current]
		path.push_back(current);// 	total_path.prepend(current)
	}
	std::reverse(path.begin(), path.end());	//  reverse order so its correct
	ResetNodeScores(); // reset the g and f scores of each node (I use those internally for the comparison lambda on my prio que)
	return path; // 	return total_path */
}

int nest::Tiler::DistanceBetween(TileNode* left, TileNode* right)
{
	/* https://theory.stanford.edu/~amitp/GameProgramming/Heuristics.html
	function heuristic(node) =
    dx = abs(node.x - goal.x)
    dy = abs(node.y - goal.y)
    return D * (dx + dy) + (D2 - 2 * D) * min(dx, dy)
	*/
	int dx = static_cast<int>(std::abs(left->m_tile.m_pos.x - right->m_tile.m_pos.x));
	int dy = static_cast<int>(std::abs(left->m_tile.m_pos.y - right->m_tile.m_pos.y));
	int d = right->m_tile.m_weight;
	int d2 = right->m_tile.m_weight + 1; // add a slightly more expensive assumption to moving diagonal
	int h = d * (dx + dy) + (d2 - 2 * d) * std::min(dx, dy);
	return h;
}

/* A* finds a path from start to goal.
// h is the heuristic function. h(n) estimates the cost to reach goal from node n.
function A_Star(start, goal, h)*/

// 3 heuristics
// h = heuristic : estimation to target (cannot over estimate, but can underestimate)
// g = goal		 : known value so far (Dijkstra weight) (the current weight from start to where we are at)
// f = fitness	 : h + g
nest::TilePath nest::Tiler::PathFind(size_t startIndex, size_t goalIndex)
{
	// create a comparison function for my prio que to sort by fScore
	auto cmp = [](TileNode* left, TileNode* right) { return left->m_fScore > right->m_fScore; };

	/* The set of discovered nodes that may need to be(re - )expanded.
	// Initially, only the start node is known.
	// This is usually implemented as a min-heap or priority queue rather than a hash-set.
	openSet = { start } */
	auto* pNode = m_tileMap[startIndex];
	std::priority_queue<TileNode*, std::vector<TileNode*>, decltype(cmp)> openSet; // default is almost always a prio que - Rez
	openSet.emplace(pNode);

    /* For startNode, cameFrom[startNode] is the node immediately preceding it (before the current node)
	// on the cheapest path from the start
    // to startNode currently known.
    cameFrom = an empty map */
	CameFromMap cameFrom;

	/* For startNode, gScore[startNode] is the currently known cost of the cheapest path from start to startNode.
    gScore = map with default value of Infinity
    gScore[start] = 0 */
	//std::map<TileNode*, int> gScore;
	pNode->m_gScore = 0;


	/* For node startNode, fScore[n] = gScore[startNode] + h(startNode). fScore[startNode] represents our current best guess as to
    // how cheap a path could be from start to finish if it goes through startNode.
    fScore = map with default value of Infinity
    fScore[start] = h(start) */
	//std::map<TileNode*, int> fScore;
	pNode->m_fScore = HeuristicCalculation(startIndex, goalIndex);

	while (!openSet.empty())
	{
		// reset the que by emptying and readding all the nodes
		//nest::RePrioritize(openSet);
		// gather node with smallest fScore
		auto* current = openSet.top();
		// if node is goal, we have our path
		if (current->m_tile.m_index == goalIndex)
			return ReconstructPath(cameFrom, current);

		openSet.pop();

		// loop through each neighbor
		for (auto& neighbor : current->m_tiles)
		{
			if (neighbor->m_tile.m_weight < 0)
				continue;
			// d(current,neighbor) is the weight of the edge from current to neighbor
			// tentative_gScore is the distance from start to the neighbor through current
			// gather distance from current to neighbor and add that to current
			int tenativeGScore = current->m_gScore + DistanceBetween(current, neighbor);
			if (tenativeGScore < neighbor->m_gScore)
			{
				cameFrom[neighbor] = current;
				neighbor->m_gScore = tenativeGScore;
				// changes priority in que
				neighbor->m_fScore = tenativeGScore + HeuristicCalculation(neighbor->m_tile.m_index, goalIndex);

				// if openSet doesnt contain neighbor, we add
				//if (!Contains(neighbor, openSet))
				//	openSet.push(neighbor);
			}
			else
			{
#if _SAVE_PATH_SEARCH_DATA == 1
				m_tilesSearched.emplace_back(neighbor->m_tile.m_index);
#endif
			}
		}

	}
	ResetNodeScores(); // reset the nodes g and f values
	_LOG(LogType::kError, "Could not find path to destination " + goalIndex);
	return {}; // fail
}

// will attempt to connect the north, west, north east, and north west nodes
// we could improve this by adding a boolean to the connections so we dont have to check some twice
// this function is expected to be called when creating tile map and works by linking nodes to previous ones
void nest::Tiler::LinkNode(size_t index)
{
	ConnectWest(index);
	ConnectNorth(index);
	ConnectNorthEast(index);
	ConnectNorthWest(index);
}

int nest::Tiler::HeuristicCalculation(size_t start, size_t end)
{ 
	if (start == end)
		return 0;
	// x moves forward columns, if tile width is 32, and x is 64. then 64/32 = column 2
	// y moves down rows
	auto* startNode = m_tileMap[start];
	auto* endNode = m_tileMap[end];
	int x = static_cast<int>(glm::abs(startNode->m_tile.m_pos.x - endNode->m_tile.m_pos.x));
	int y = static_cast<int>(glm::abs(startNode->m_tile.m_pos.y - endNode->m_tile.m_pos.y));
	auto h = x + y; 
	return h;
}

int nest::Tiler::HeuristicCalculationUsingNodes(TileNode* current, TileNode* goal)
{
	if (current == goal)
		return 0;
	// x moves forward columns, if tile width is 32, and x is 64. then 64/32 = column 2
	// y moves down rows
	int x = static_cast<int>(glm::abs(current->m_tile.m_pos.x - goal->m_tile.m_pos.x));
	int y = static_cast<int>(glm::abs(current->m_tile.m_pos.y - goal->m_tile.m_pos.y));
	auto h = x + y;
	return h;
}

void nest::Tiler::ParseTileXML(const char* xmlFileLocation)
{
	// rows are equal to height / tileheight
	int rows = m_data.GetTileMapData().m_rows;
	// columns are equal to width / tilewidth
	int columns = m_data.GetTileMapData().m_columns;
	// parse the TMX for tile data
	XMLFile file{ xmlFileLocation };
	// check to make sure the file is valid
	if (file.Check())
	{
		// while the file still has unsearched children, we keep the loop active
		while (true)
		{
			// gather the current child node in the file
			XMLNode node = file.GetChildNode();
			// create a tile template from the xml
			CreateTileTemplate(node);
			// advance to next child
			if (!file.MoveToNextChild())
				break;
		}
		// gather tile map for parsing
		std::string map{ m_data.GetGameMap() };
		// advance position once to get past the endline
		size_t pos = 1;
		int tileWidth = m_data.GetTileMapData().m_tileWidth;
		int tileHeight = m_data.GetTileMapData().m_tileHeight;
		// this will populate moving forward columns,
		// then back to the start of a row
		for (int i = 0; i < rows; ++i)
		{
			for (int ii = 0; ii < columns; ++ii)
			{
				int index = (i * columns) + ii;
				int id = std::stoi(FindNextWord(pos, map.data(), ','));
				// emplace a new tile node based on the Id
				// if Id exists as a tile template, copy construct the new node
				CreateTileFromPreset(index, id);
				m_tileMap[index]->m_tile.m_pos = { ii * tileWidth, i * tileHeight};

			}
		}
	}
	else
		_LOG_V(LogType::kError, "XML file is invalid!");
}

//-------------------------------------------------------------------
// Tile parsing functions
void nest::Tiler::CreateTileTemplate(const XMLNode node)
{
	int id = 0;
	int weight = 0;
	// if tile has an id
	if (node.GetIntFromRoot("Id", id))
	{
		if (node.GetIntFromRoot("Weight", weight))
		{
			m_tileTemplates.emplace_back(Tile{ id, weight });
			return;
		}
		else // lets not demand the weight as we could use 0 weight as default
		{
			m_tileTemplates.emplace_back(Tile{ id });
		}
	}
}


void nest::Tiler::CreateTileFromPreset(size_t index, int id)
{
	// search tile templates for a matching ID to copy
	for (auto& tile : m_tileTemplates)
	{
		if (tile.m_id == id)
		{
			m_tileMap[index] = BLEACH_NEW(TileNode(Tile{ tile }));
			m_tileMap[index]->m_tile.m_index = index;
			LinkNode(index);
			return;
		}
	}
	// if no valid template was found, create a tile with index 0
	// TODO:: we could potentially create a default tile in this case as well
	m_tileMap[index] = BLEACH_NEW(TileNode(Tile{ 0 }));
	LinkNode(index);
}

// will connect index to north node if possible, and vice versa
void nest::Tiler::ConnectNorth(size_t index)
{
	size_t columns = m_data.GetTileMapData().m_columns;
	if (index >= columns /* not in first row*/) // we can always connect upwards so long as we are not on the first row
	{
		size_t northIndex = index - columns;
		m_tileMap[index]->AddNode(m_tileMap[northIndex]);
		m_tileMap[northIndex]->AddNode(m_tileMap[index]);
	}
}

// will connect index to north-west node if possible, and vice versa
void nest::Tiler::ConnectNorthWest(size_t index)
{
	size_t columns = m_data.GetTileMapData().m_columns;
	if (index >= columns /* not in first row*/ && index % columns > 0 /* not on left edge*/)
	{
		size_t northWestIndex = index - columns - static_cast<size_t>(1);
		m_tileMap[index]->AddNode(m_tileMap[northWestIndex]);
		m_tileMap[northWestIndex]->AddNode(m_tileMap[index]);
	}
}

// will connect index to west node if possible, and vice versa
void nest::Tiler::ConnectWest(size_t index)
{
	size_t columns = m_data.GetTileMapData().m_columns;
	// the left edge will always be 0 when % with column count
	if (index % columns > 0 /* not in first column*/)
	{
		size_t westIndex = index - static_cast<size_t>(1);
		m_tileMap[index]->AddNode(m_tileMap[westIndex]);
		m_tileMap[westIndex]->AddNode(m_tileMap[index]);
	}
}

// will connect index to north east node if possible, and vice versa
void nest::Tiler::ConnectNorthEast(size_t index)
{
	if (index == 79)
		std::cout << "st";
	size_t columns = m_data.GetTileMapData().m_columns;
	auto r  = std::fmod(index + static_cast<size_t>(1), columns);
	if (index >= columns /* not in first column*/ && r != 0)
	{
		size_t northEastIndex = index - columns + static_cast<size_t>(1);
		m_tileMap[index]->AddNode(m_tileMap[northEastIndex]);
		m_tileMap[northEastIndex]->AddNode(m_tileMap[index]);
	}
}
//
//-------------------------------------------------------------------
//-------------------------------------------------------------------
// Test Functions
void nest::Tiler::TestDraw(int tileId, [[maybe_unused]] Color color, [[maybe_unused]] bool fill)
{
	// rows are equal to height / tileheight
	int rows = m_data.GetTileMapData().m_rows;
	// columns are equal to width / tilewidth
	int columns = m_data.GetTileMapData().m_columns;
	// rows are equal to height / tileheight
	int tileW = m_data.GetTileMapData().m_tileWidth;
	// columns are equal to width / tilewidth
	int tileH = m_data.GetTileMapData().m_tileHeight;

	for (size_t i = 0; i < rows; ++i)
	{
		for (size_t ii = 0; ii < columns; ++ii)
		{
			size_t index = (i * columns) + ii;
			if (m_tileMap[index]->m_tile.m_id == tileId)
			{
				[[maybe_unused]] float x = (static_cast<float>(ii) * static_cast<float>(tileW));
				[[maybe_unused]] float y = static_cast<float>(i) * static_cast<float>(tileH);
				[[maybe_unused]] float w = static_cast<float>(tileW) - .5f; // subtracting a small portion so we dont get overlapping lines
				[[maybe_unused]] float h = static_cast<float>(tileH) - .5f;
				//nest::Engine::GetGraphics()->DrawRectangleF({ x, y, w, h }, color, fill);
			}
		}
	}
}

void nest::Tiler::PrintMap()
{
	size_t columns = m_data.GetTileMapData().m_columns;
	size_t rows = m_data.GetTileMapData().m_rows;
	for (size_t i = 0; i < rows; ++i)
	{
		for (size_t ii = 0; ii < columns; ++ii)
		{
			std::cout << m_tileMap[(i * columns) + ii]->m_tile.m_id << std::endl;
		}
	}
	return;
}

void nest::Tiler::DrawTile( [[maybe_unused]] TileNode* node, [[maybe_unused]] Color color, [[maybe_unused]] bool fill)
{
	//nest::Engine::GetGraphics()->DrawRectangle(
	//	{ node->m_tile.m_pos.x, node->m_tile.m_pos.y, m_data.GetTileMapData().m_tileWidth, m_data.GetTileMapData().m_tileHeight },
	//	color, fill);
}

void nest::Tiler::DrawIndex(int index, Color color, bool fill)
{
	DrawTile(m_tileMap[index], color, fill);
}

void nest::Tiler::DrawSearched([[maybe_unused]] Color color, [[maybe_unused]] bool fill)
{
#if _SAVE_PATH_SEARCH_DATA == 1
	for (auto& i : m_tilesSearched)
	{
		DrawIndex(i, color, fill);
	}
#endif
}

void nest::Tiler::DrawConnecting(int id, Color color, bool fill)
{
	for (auto connecting : m_tileMap[id]->m_tiles)
	{
		DrawTile(connecting, color, fill);
	}
}
//
//-------------------------------------------------------------------


void nest::Tiler::ClearSearchedNodes()
{
#if _SAVE_PATH_SEARCH_DATA == 1
	for (auto i = m_tilesSearched.begin(); i != m_tilesSearched.end(); /* */)
	{
		i = m_tilesSearched.erase(i);
	}
#endif
}

void nest::Tiler::ResetNodeScores()
{
	for (auto& tile : m_tileMap)
	{
		tile.second->m_gScore = std::numeric_limits<int>::max();
		tile.second->m_fScore = std::numeric_limits<int>::max();
	}
}

void nest::Tiler::TestNode(Vec2 pos)
{
	uint16_t tileWidth = m_data.GetTileMapData().m_tileWidth;
	uint16_t tileHeight = m_data.GetTileMapData().m_tileHeight;
	uint16_t rowWidth = m_data.GetTileMapData().m_columns;
	int column = static_cast<int>(std::round(pos.x / tileWidth));
	int row = static_cast<int>(std::round(pos.y / tileHeight));
	int index = (row * rowWidth) + column;

	auto* node = m_tileMap[index];
	for (size_t i = 0; i < 8; ++i)
	{
		std::cout << node->m_tiles[i]->m_tile.m_id << std::endl;
	}
}