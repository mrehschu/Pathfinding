#pragma once
#include <chrono>
#include "Coroutine.h"
#include "Graph.h"

namespace Pathfinding
{
	enum class NodeState
	{
		DEFAULT,
		CURRENT,
		DISCOVERED,
		PROCESSED
	};

	struct PathData
	{
		const Node* previousNode = nullptr;
		float pathWeight = 0;

		PathData() {}
		PathData(const Node* previousNode, float pathWeight) : previousNode(previousNode), pathWeight(pathWeight) {}
		virtual ~PathData() {}
	};

	struct SearchData
	{
		const Node* const node;
		const NodeState state;
		const PathData& pathData;
	};

	struct SearchResult
	{
		const bool pathFound = false;
		const float pathWeight = 0;
		const std::list<const Node*> path;

		const size_t nodesExplored = 0;
		const std::chrono::nanoseconds runtime = std::chrono::nanoseconds::zero();

		SearchResult() {}
		SearchResult(const size_t nodesExplored, const std::chrono::nanoseconds runtime) : nodesExplored(nodesExplored), runtime(runtime) {}
		SearchResult(const bool pathFound, const float pathWeight, std::list<const Node*>&& path, const size_t nodesExplored, const std::chrono::nanoseconds runtime)
			: pathFound(pathFound), pathWeight(pathWeight), path(move(path)), nodesExplored(nodesExplored), runtime(runtime) {}
	};

	class Pathfinder
	{
		public:

		std::list<SearchData> searchLog;
		std::shared_ptr<SearchResult> searchResult;

		virtual Coroutine search(const Graph& graph, const Node& start, const Node& end, bool& incrementalSearch) = 0;
	};
}