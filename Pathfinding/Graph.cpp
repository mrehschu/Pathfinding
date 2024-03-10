#include "Graph.h"

using namespace Pathfinding;

Graph::Graph(std::vector<std::unique_ptr<Node>>&& nodes)
{
	for (auto& node : nodes)
	{
		this->nodes.insert({node->getName(), move(node)});
	}
	nodes.clear();
}

void Graph::addNode(std::unique_ptr<Node>&& node)
{
	// automatically checks if element already contained, adds element if not
	this->nodes.insert({node->getName(), move(node)});
}

bool Graph::removeNode(const std::string name)
{
	// return false if element not contained
	auto iter = this->nodes.find(name);
	if (iter == this->nodes.end()) return false;

	// else delete element
	this->nodes.erase(iter);
	return true;
}

bool Graph::contains(const std::string name) const
{
	auto iter = this->nodes.find(name);
	return iter != this->nodes.end();
}

bool Graph::tryGetNode(const std::string name, const std::unique_ptr<Node>*& out) const
{
	if (!contains(name)) return false;

	out = &getNode(name);
	return true;
}