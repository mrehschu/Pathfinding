#pragma once
#include "Pathfinder.h"

namespace Pathfinding
{
    class Dijkstra : public Pathfinder
    {
        public:

        Coroutine search(const Graph& graph, const Node& start, const Node& end, bool& incrementalSearch) override;
    };
}