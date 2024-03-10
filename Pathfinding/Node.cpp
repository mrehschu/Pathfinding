#include "Node.h"

using namespace Pathfinding;

std::unique_ptr<Node> Node::create(const std::string name, std::vector<std::unique_ptr<Edge>>&& edges)
{
	auto node = std::make_unique<Node>();
	node->name = name;
	node->edges = move(edges);

	return node;
}

void Node::addEdge(const Node& neighbour, const float weight)
{
	// check if node is already contained
	auto iter = std::find_if(this->edges.begin(), this->edges.end(), [&](auto& item) { return *item->neighbour == neighbour; });
	if (iter != this->edges.end()) return;

	// add new edge
	this->edges.push_back(std::make_unique<Edge>(&neighbour, weight));
}

bool Node::removeEdge(const Node& neighbour)
{
	// try to remove element, if success return true, else return false
	auto iter = std::find_if(this->edges.begin(), this->edges.end(), [&](auto& item) { return *item->neighbour == neighbour; });
	if (iter == this->edges.end()) return false;

	this->edges.erase(iter);
	return true;
}

void Node::setEdgeWeight(const Node& neighbour, const float weight)
{
	// find edge and change its weight
	auto iter = std::find_if(this->edges.begin(), this->edges.end(), [&](auto& item) { return *item->neighbour == neighbour; });
	if (iter != this->edges.end()) (*iter)->weight = weight;
}

bool Node::operator==(const Node& other) const
{
	return this->name == other.name;
}