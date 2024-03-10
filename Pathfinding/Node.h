#pragma once
#include <memory>
#include <string>
#include <vector>

namespace Pathfinding
{
	struct Edge;

	class Node
	{
		private:

		std::string name;
		std::vector<std::unique_ptr<Edge>> edges;

		public:

		static std::unique_ptr<Node> create(const std::string name, std::vector<std::unique_ptr<Edge>>&& edges = {});
		void addEdge(const Node& neighbour, const float weight);
		bool removeEdge(const Node& neighbour);
		void setEdgeWeight(const Node& neighbour, const float weight);

		const std::vector<std::unique_ptr<Edge>>& getEdges() const { return edges; }
		const std::string getName() const { return name; }
		~Node() { edges.clear(); }

		bool operator==(const Node& other) const;
	};

	struct Edge
	{
		const Node* neighbour;
		float weight;
	};
}