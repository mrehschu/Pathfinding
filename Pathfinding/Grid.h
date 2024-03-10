#pragma once
#include <format>
#include "Environment.h"

namespace Pathfinding
{
	namespace GridColor
	{
		constexpr SDL_Color BACKGROUND = { 40, 40, 40, 255 };			// black
		constexpr SDL_Color NODE_DEFAULT = { 255, 255, 255, 255 };		// white
		constexpr SDL_Color NODE_CURRENT = { 0, 255, 0, 255 };			// green
		constexpr SDL_Color	NODE_DISCOVERED = { 255, 165, 0, 255 };		// orange
		constexpr SDL_Color NODE_PROCESSED = { 0, 0, 255, 255 };		// blue
		constexpr SDL_Color TRANSPARENT = { 255, 255, 255, 0 };			// transparent
		constexpr SDL_Color OVERLAY = { 255, 0, 0, 255 };				// red
		constexpr SDL_Color TEXT = { 0, 0, 0, 255 };					// black
	};

	class Grid : public Environment
	{
		private:

		struct GridData
		{
			int gridWidth, gridHeight;
			int marginWidth, marginHeight;
			int borderWidth, borderHeight;
			int nodeWidth, nodeHeight;

			std::vector<int> heightMap;
		};

		GridData gridData;

		void resetRenderState() override;
		void drawGraph() const override;
		void drawEdgeWeights() const override;
		void drawPath(const std::list<const Node*> path) override;
		void drawNode(const Node& node, const SDL_Color color) const override;
		void drawConnections(const Node& node, const PathData& pathData) const override;
		void drawPathWeights(const Node& node, const PathData& pathData) const override;
		void drawCoordinates() const override;

		public:

		Grid(const int width, const int height, SDL_Window* window);

		void refreshWindow() override;
		void searchInitialize(std::unique_ptr<Pathfinder>&& pathfinder, const Node& start, const Node& end) override;
		const SDL_Color getColorOf(NodeState nodeState) const override;

		static const SDL_Point getGridCoordinates(const Node& node);
		const SDL_Point getScreenCoordinates(const Node& node) const;
		const SDL_Point getScreenCoordinates(const int gridX, const int gridY) const;

		static std::string generateNodeName(const int x, const int y) { return std::format("{}, {}", x, y); }
	};	
}