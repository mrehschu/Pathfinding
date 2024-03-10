#pragma once
#include <unordered_map>
#include "Node.h"

namespace Pathfinding
{
	class Graph
	{
		private:

		std::unordered_map<std::string, std::unique_ptr<Node>> nodes;

		public:

		Graph(std::vector<std::unique_ptr<Node>> && = {});
		void addNode(std::unique_ptr<Node>&& node);
		bool removeNode(const std::string name);
		bool contains(const std::string name) const;
		bool tryGetNode(const std::string name, const std::unique_ptr<Node>*& out) const;

		~Graph() { clear(); }
		void clear() { nodes.clear(); }
		const std::unique_ptr<Node>& getNode(const std::string name) const { return nodes.at(name); }
		const std::unordered_map<std::string, std::unique_ptr<Node>>& getNodes() const { return nodes; }
	};
}