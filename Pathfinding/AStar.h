#pragma once
#include "Pathfinder.h"

namespace Pathfinding
{
	struct AStarPathData : public PathData
	{
		float heuristicValue = 0;
		float sortingValue = 0;

		AStarPathData() {}
		AStarPathData(const Node* previousNode, float pathWeight, float heuristicValue) : PathData({ previousNode, pathWeight }), heuristicValue(heuristicValue)
		{
			sortingValue = pathWeight + heuristicValue;
		}
	};

	class AStar : public Pathfinder
	{
		public:

		std::function<float(const Graph& graph, const Node& current, const Node& target)> getHeuristic;
		Coroutine search(const Graph& graph, const Node& start, const Node& end, bool& incrementalSearch) override;
	};
}