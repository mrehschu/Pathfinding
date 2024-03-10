#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "Pathfinder.h"

namespace Pathfinding
{
	class Environment
	{
		protected:

		struct SDL_Data
		{
			SDL_Window* window;
			SDL_Renderer* renderer;
			SDL_Texture* textureGraph;
			SDL_Texture* textureEdgeWeights;
			SDL_Texture* textureSearchConnections;
			SDL_Texture* textureSearchValues;
			SDL_Texture* textureCoordinates;

			int windowWidth, windowHeight;

			bool renderLayerMask[4]{ true, true, false, false };
		};

		struct PathfindingData
		{
			bool incrementalSearch = false;
			std::unique_ptr<Pathfinder> pathfinder;
			Coroutine searchCoroutine;
		};

		struct NodeData
		{
			SDL_Color nodeColor;
			PathData pathData;
		};

		SDL_Data renderData;
		PathfindingData searchData;
		std::shared_ptr<Graph> graph;
		std::unordered_map<const Node*, NodeData> nodeStates;


		Environment(SDL_Window* window);

		void applyNodeStates() const;
		void drawSearchData(const Node& node, const PathData& pathData) const;

		virtual void resetRenderState() = 0;
		virtual void drawGraph() const = 0;
		virtual void drawEdgeWeights() const = 0;
		virtual void drawPath(const std::list<const Node*> path) = 0;
		virtual void drawNode(const Node& node, const SDL_Color color) const = 0;
		virtual void drawConnections(const Node& node, const PathData& pathData) const = 0;
		virtual void drawPathWeights(const Node& node, const PathData& pathData) const = 0;
		virtual void drawCoordinates() const = 0;

		public:

		enum class RenderLayer
		{
			EdgeWeights = 0,
			SearchConnections = 1,
			SearchValues = 2,
			Coordinates = 3
		};

		~Environment();

		void setIncrementalSearch(const bool value) { searchData.incrementalSearch = value; }
		virtual void searchInitialize(std::unique_ptr<Pathfinder>&& pathfinder, const Node& start, const Node& end);
		virtual bool searchRun();

		virtual void refreshWindow();
		virtual void renderEnvironment() const;
		virtual const SDL_Color getColorOf(const NodeState nodeState) const = 0;

		const std::shared_ptr<Graph>& getGraph() const { return graph; }
		bool isRenderLayerActive(RenderLayer layer) const { return renderData.renderLayerMask[(int)layer]; }
		void setRenderLayer(RenderLayer layer, bool value)
		{
			renderData.renderLayerMask[(int)layer] = value;
			renderEnvironment();
		}
	};
}