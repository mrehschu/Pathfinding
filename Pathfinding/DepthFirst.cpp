#include "DepthFirst.h"
#include <stack>
#include <set>

using namespace Pathfinding;

Coroutine DepthFirst::search(const Graph& graph, const Node& start, const Node& end, bool& incrementalSearch)
{
	using namespace std;
	using namespace std::chrono;

	nanoseconds runtime = nanoseconds::zero();
	auto startTime = high_resolution_clock().now();

	const Node* current = nullptr;
	stack<const Node*> discovered;
	set<const Node*> explored;
	unordered_map<const Node*, PathData> pathData;
	size_t previousSearchLogSize;

	discovered.push(&start);
	pathData.insert({ &start, PathData { nullptr, 0 } });

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
			const bool neighbourUnknown = pathData.find(edge->neighbour) == pathData.end();
			const PathData nodeData{ current, neighbourPathWeight };

			// discover new neighbours of current node
			if (neighbourUnknown)
			{
				discovered.push(edge->neighbour);
				pathData.insert_or_assign(edge->neighbour, nodeData);
				searchLog.push_back({ edge->neighbour, NodeState::DISCOVERED, pathData[edge->neighbour] });
			}

			// if pathWeight of neighbour is worse than current path -> replace pathData
			else if (pathData[edge->neighbour].pathWeight > neighbourPathWeight)
			{
				pathData.insert_or_assign(edge->neighbour, nodeData);
				const bool neighbourExplored = explored.find(edge->neighbour) != explored.end();
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
