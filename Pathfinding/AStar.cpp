#include "AStar.h"
#include <queue>
#include <unordered_set>

using namespace Pathfinding;

Coroutine AStar::search(const Graph& graph, const Node& start, const Node& end, bool& incrementalSearch)
{
	using namespace std;
	using namespace std::chrono;

	nanoseconds runtime = nanoseconds::zero();
	auto startTime = high_resolution_clock().now();

	if (!getHeuristic)
	{
		searchResult = make_shared<SearchResult>();
		throw exception("AStar was run without assigning a heuristic function first.");
	}

	const Node* current = nullptr;
	unordered_map<const Node*, AStarPathData> pathData;

	auto compare = [&](const Node* left, const Node* right)
	{
		const auto& leftData = pathData[left];
		const auto& rightData = pathData[right];

		if (leftData.sortingValue != rightData.sortingValue)
			return leftData.sortingValue > rightData.sortingValue;

		return leftData.heuristicValue > rightData.heuristicValue;
	};

	vector<const Node*> vec;
	vec.reserve(graph.getNodes().size());
	priority_queue<const Node*, vector<const Node*>, decltype(compare)> discovered(compare, move(vec));
	unordered_set<const Node*> explored;
	size_t previousSearchLogSize;

	discovered.push(&start);
	pathData.insert({ &start, AStarPathData { nullptr, 0, getHeuristic(graph, start, end) } });

	while (!discovered.empty())
	{
		current = discovered.top();
		discovered.pop();
		searchLog.push_back({ current, NodeState::CURRENT, pathData[current] });

		// allowing breakpoint (not part of the algorithm)
		runtime += high_resolution_clock().now() - startTime;
		co_await suspend_if(&incrementalSearch);
		startTime = high_resolution_clock().now();
		previousSearchLogSize = searchLog.size();

		// if whole path is found -> break out of loop
		if (*current == end) break;

		for (auto& edge : current->getEdges())
		{
			const float neighbourPathWeight = pathData[current].pathWeight + edge->weight;
			const bool neighbourUnknown = !pathData.contains(edge->neighbour);			

			// discover new neighbours of current node
			if (neighbourUnknown)
			{
				const AStarPathData nodeData { current, neighbourPathWeight, getHeuristic(graph, *edge->neighbour, end) };
				pathData.insert_or_assign(edge->neighbour, nodeData);
				discovered.push(edge->neighbour);
				searchLog.push_back({ edge->neighbour, NodeState::DISCOVERED, pathData[edge->neighbour] });
			}

			// if pathWeight of neighbour is worse than current path -> replace pathData
			else if (pathData[edge->neighbour].pathWeight > neighbourPathWeight)
			{
				const AStarPathData nodeData { current, neighbourPathWeight, pathData[edge->neighbour].heuristicValue };
				pathData.insert_or_assign(edge->neighbour, nodeData);
				const bool neighbourExplored = explored.contains(edge->neighbour);
				searchLog.push_back({ edge->neighbour, neighbourExplored ? NodeState::PROCESSED : NodeState::DISCOVERED, pathData[edge->neighbour] });
			}
		}

		// allowing breakpoint (not part of the algorithm)
		runtime += high_resolution_clock().now() - startTime;
		co_await suspend_if([&]() { return incrementalSearch && searchLog.size() != previousSearchLogSize; });
		startTime = high_resolution_clock().now();

		explored.insert(current);
		searchLog.push_back({ current, NodeState::PROCESSED, pathData[current] });
	}

	runtime += high_resolution_clock().now() - startTime;

	// no path found
	if (*current != end)
	{
		searchResult = make_shared<SearchResult>(explored.size(), runtime);
		co_return;
	}

	list<const Node*> path;
	while (current != nullptr)
	{
		path.push_front(current);
		current = pathData[current].previousNode;
	}

	searchResult = make_shared<SearchResult>(true, pathData[&end].pathWeight, move(path), explored.size() + 1, runtime);

}