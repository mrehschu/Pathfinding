#include "Grid.h"
#include "AStar.h"
#include <PerlinNoise/PerlinNoise.hpp>

using namespace Pathfinding;

constexpr int HEIGHTMAP_STEPS = 8;
const std::string FONT_NAME = "C:/Windows/Fonts/ariblk.ttf";

Grid::Grid(const int width, const int height, SDL_Window* window) : Environment(window)
{
	using namespace std;

    if (width <= 0 || height <= 0) throw exception("Width and height of grid have to be greater than 0.");
    gridData.gridWidth = width;
    gridData.gridHeight = height;

    gridData.heightMap.resize((size_t)width * height);
	graph = make_shared<Graph>();

    constexpr double noiseScale = 0.4;
    const siv::PerlinNoise::seed_type seed = (unsigned int)time(NULL);

    const siv::PerlinNoise perlin { seed };
    auto edgeWeight = [&](int x1, int y1, int x2, int y2)
    {
        int gradient = gridData.heightMap[(size_t)x2 * gridData.gridWidth + y2] - gridData.heightMap[(size_t)x1 * gridData.gridWidth + y1];
        if (gradient < 0) gradient /= 2;
        return (float)(gradient + HEIGHTMAP_STEPS);
    };

    for (int x = 0; x < gridData.gridWidth; x++)
    {
        for (int y = 0; y < gridData.gridHeight; y++)
        {
            auto node = Node::create(generateNodeName(x, y));
            gridData.heightMap[(size_t)x * gridData.gridWidth + y] = (int)(perlin.noise2D_01(x * noiseScale, y * noiseScale) * HEIGHTMAP_STEPS);

            const unique_ptr<Node>* neighbour;
            if (x > 0 && graph->tryGetNode(generateNodeName(x - 1, y), neighbour)) { node->addEdge(*neighbour->get(), edgeWeight(x, y, x - 1, y)); (*neighbour)->addEdge(*node.get(), edgeWeight(x - 1, y, x, y)); }
            if (y > 0 && graph->tryGetNode(generateNodeName(x, y - 1), neighbour)) { node->addEdge(*neighbour->get(), edgeWeight(x, y, x, y - 1)); (*neighbour)->addEdge(*node.get(), edgeWeight(x, y - 1, x, y)); }

            graph->addNode(move(node));
        }
    }

    refreshWindow();
}

void Grid::refreshWindow()
{
    SDL_GetRendererOutputSize(renderData.renderer, &renderData.windowWidth, &renderData.windowHeight);

    constexpr int borderScale = 5;
    gridData.borderWidth = renderData.windowWidth / ((gridData.gridWidth * borderScale) + (gridData.gridWidth + 1));
    gridData.borderHeight = renderData.windowHeight / ((gridData.gridHeight * borderScale) + (gridData.gridHeight + 1));

    gridData.nodeWidth = (renderData.windowWidth - (gridData.borderWidth * (gridData.gridWidth + 1))) / gridData.gridWidth;
    gridData.nodeHeight = (renderData.windowHeight - (gridData.borderHeight * (gridData.gridHeight + 1))) / gridData.gridHeight;

    gridData.marginWidth = (renderData.windowWidth - (gridData.borderWidth * (gridData.gridWidth - 1)) - (gridData.nodeWidth * gridData.gridWidth)) / 2;
    gridData.marginHeight = (renderData.windowHeight - (gridData.borderHeight * (gridData.gridHeight - 1)) - (gridData.nodeHeight * gridData.gridHeight)) / 2;

    Environment::refreshWindow();
}

void Grid::resetRenderState()
{
    nodeStates.clear();

    const SDL_Color color = GridColor::TRANSPARENT;
    SDL_SetRenderDrawColor(renderData.renderer, color.r, color.g, color.b, color.a);

    SDL_SetRenderTarget(renderData.renderer, renderData.textureSearchConnections);
    SDL_RenderClear(renderData.renderer);

    SDL_SetRenderTarget(renderData.renderer, renderData.textureSearchValues);
    SDL_RenderClear(renderData.renderer);
    
    drawGraph();
}

void Grid::drawGraph() const
{
    SDL_Color color = GridColor::BACKGROUND;
    SDL_SetRenderTarget(renderData.renderer, renderData.textureGraph);
    SDL_SetRenderDrawColor(renderData.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderData.renderer);

    std::vector<SDL_Rect> rects;
    for (int x = 0; x < gridData.gridWidth; x++)
    {
        for (int y = 0; y < gridData.gridHeight; y++)
        {
            const auto [xPos, yPos] = getScreenCoordinates(x, y);
            rects.push_back({ xPos, yPos, gridData.nodeWidth, gridData.nodeHeight });
        }
    }

    color = GridColor::NODE_DEFAULT;
    SDL_SetRenderDrawColor(renderData.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRects(renderData.renderer, &rects[0], (int)rects.size());

    SDL_SetRenderTarget(renderData.renderer, NULL);
}

void Grid::drawPath(const std::list<const Node*> path)
{
    resetRenderState();

    std::vector<SDL_Rect> rects;
    for (const Node* node : path)
    {
        const auto [xPos, yPos] = getScreenCoordinates(*node);
        rects.push_back({ xPos, yPos, gridData.nodeWidth, gridData.nodeHeight });
        nodeStates[node] = { GridColor::NODE_CURRENT, {} };
    }

    const SDL_Color color = GridColor::NODE_CURRENT;
    SDL_SetRenderTarget(renderData.renderer, renderData.textureGraph);
    SDL_SetRenderDrawColor(renderData.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRects(renderData.renderer, &rects[0], (int)rects.size());

    SDL_SetRenderTarget(renderData.renderer, NULL);
}

void Grid::drawNode(const Node& node, const SDL_Color color) const
{
    const auto [xPos, yPos] = getScreenCoordinates(node);
    const SDL_Rect rect = { xPos, yPos, gridData.nodeWidth, gridData.nodeHeight };

    SDL_SetRenderTarget(renderData.renderer, renderData.textureGraph);
    SDL_SetRenderDrawColor(renderData.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderData.renderer, &rect);

    SDL_SetRenderTarget(renderData.renderer, NULL);
}

void Grid::drawEdgeWeights() const
{
    SDL_Color color = GridColor::TRANSPARENT;
    SDL_SetRenderTarget(renderData.renderer, renderData.textureEdgeWeights);
    SDL_SetRenderDrawColor(renderData.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderData.renderer);

    constexpr int outlineScale = 4;
    const int outlineWidth = (gridData.nodeWidth / 2) / outlineScale;
    const int outlineHeight = (gridData.nodeHeight / 2) / outlineScale;

    std::vector<SDL_Rect> centerPieces;
    for (int x = 0; x < gridData.gridWidth; x++)
    {
        for (int y = 0; y < gridData.gridHeight; y++)
        {
            const auto [xPos, yPos] = getScreenCoordinates(x, y);
            const SDL_Rect outline = { xPos, yPos, gridData.nodeWidth, gridData.nodeHeight };
            const SDL_Rect center = { xPos + outlineWidth, yPos + outlineHeight, gridData.nodeWidth - (outlineWidth * 2), gridData.nodeHeight - (outlineHeight * 2) };

            const Uint8 greyValue = 250 - ((200 / HEIGHTMAP_STEPS) * gridData.heightMap[(size_t)x * gridData.gridWidth + y]);
            color = { greyValue, greyValue, greyValue, 255 };
            SDL_SetRenderDrawColor(renderData.renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(renderData.renderer, &outline);
            centerPieces.push_back(center);
        }
    }

    color = GridColor::TRANSPARENT;
    SDL_SetRenderDrawColor(renderData.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRects(renderData.renderer, &centerPieces[0], (int)centerPieces.size());

    SDL_SetRenderTarget(renderData.renderer, NULL);
}

void Grid::drawConnections(const Node& node, const PathData& pathData) const
{
    if (!pathData.previousNode) return;

    auto sign = [](int value) { return (value > 0) - (value < 0); };

    constexpr int lineScale = 1;    
    const SDL_Point halfNode = { gridData.nodeWidth / 2, gridData.nodeHeight / 2 };

    SDL_SetRenderTarget(renderData.renderer, renderData.textureSearchConnections);
    
    struct Connection
    {
        const Node& node;
        const Node& previousNode;
        const SDL_Color color;
    };

    std::vector<Connection> connections;
    if (nodeStates.contains(&node) && *nodeStates.at(&node).pathData.previousNode != *pathData.previousNode)
    {
        // clear old connection
        connections.push_back({ node, *nodeStates.at(&node).pathData.previousNode, GridColor::TRANSPARENT });
    }
    // draw new connection
    connections.push_back({ node, *pathData.previousNode, GridColor::OVERLAY });

    for (auto& connection : connections)
    {
        const SDL_Point thisNodeOrigin = getScreenCoordinates(connection.node);
        const SDL_Point otherNodeOrigin = getScreenCoordinates(connection.previousNode);
        const SDL_Point nodeDiff = { sign(thisNodeOrigin.x - otherNodeOrigin.x), sign(thisNodeOrigin.y - otherNodeOrigin.y) };

        const SDL_Point p1
        {
            thisNodeOrigin.x + halfNode.x + (-nodeDiff.x * (halfNode.x - (halfNode.x / lineScale))),
            thisNodeOrigin.y + halfNode.y + (-nodeDiff.y * (halfNode.y - (halfNode.y / lineScale)))
        };
        const SDL_Point p2
        {
            otherNodeOrigin.x + halfNode.x + (nodeDiff.x * (halfNode.x - (halfNode.x / lineScale))),
            otherNodeOrigin.y + halfNode.y + (nodeDiff.y * (halfNode.y - (halfNode.y / lineScale)))
        };

        const SDL_Color color = connection.color;
        SDL_SetRenderDrawColor(renderData.renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawLine(renderData.renderer, p1.x, p1.y, p2.x, p2.y);
    }

    SDL_SetRenderTarget(renderData.renderer, NULL);
}

void Grid::drawPathWeights(const Node& node, const PathData& pathData) const
{
    SDL_SetRenderTarget(renderData.renderer, renderData.textureSearchValues);

    constexpr float textScale = 4;

    constexpr float FONTSIZE_TO_PIXEL = 1.125;
    const int textHeight = (gridData.nodeHeight <= gridData.nodeWidth) ? gridData.nodeHeight : (int)(gridData.nodeWidth * 0.9);
    const int fontSize = (int)((textHeight / textScale) / FONTSIZE_TO_PIXEL);
    TTF_Font* font = TTF_OpenFont(FONT_NAME.data(), fontSize);

    std::vector<std::string> labels;    
    const AStarPathData* pathDataAsAStar = dynamic_cast<const AStarPathData*>(&pathData);
    if (!pathDataAsAStar)
    {
        labels.push_back(std::format("f:{}", pathData.pathWeight));
    }
    else
    {
        labels.push_back(std::format("f:{}", pathDataAsAStar->sortingValue));
        labels.push_back(std::format("g:{}", pathDataAsAStar->pathWeight));
        labels.push_back(std::format("h:{}", pathDataAsAStar->heuristicValue));        
    }
    
    auto [xPos, yPos] = getScreenCoordinates(node);

    for (int i = 0; i < labels.size(); i++)
    {
        SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, labels[i].data(), GridColor::TEXT);
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderData.renderer, textSurface);

        SDL_Point offset;
        switch (i)
        {
            case 0: offset = { gridData.nodeWidth - textSurface->w, gridData.nodeHeight - textSurface->h }; break;
            case 1: offset = { 0, gridData.nodeHeight - textSurface->h }; break;
            case 2: offset = { gridData.nodeWidth - textSurface->w, 0 }; break;
        }
        const SDL_Rect rect = { xPos + offset.x, yPos + offset.y, textSurface->w, textSurface->h };

        SDL_Color color = GridColor::TRANSPARENT;
        SDL_SetRenderDrawColor(renderData.renderer, color.r, color.g, color.b, color.a);
        SDL_RenderFillRect(renderData.renderer, &rect);

        color = GridColor::TEXT;
        SDL_SetRenderDrawColor(renderData.renderer, color.r, color.g, color.b, color.a);
        SDL_RenderCopy(renderData.renderer, textTexture, NULL, &rect);

        SDL_FreeSurface(textSurface);
        SDL_DestroyTexture(textTexture);
    }

    TTF_CloseFont(font);
    SDL_SetRenderTarget(renderData.renderer, NULL);
}

void Grid::drawCoordinates() const
{
    SDL_Color color = GridColor::TRANSPARENT;
    SDL_SetRenderTarget(renderData.renderer, renderData.textureCoordinates);
    SDL_SetRenderDrawColor(renderData.renderer, color.r, color.g, color.b, color.a);
    SDL_RenderClear(renderData.renderer);

    constexpr float textScale = 2.5;

    constexpr float FONTSIZE_TO_PIXEL = 1.125;
    const int textHeight = (gridData.nodeHeight <= gridData.nodeWidth) ? gridData.nodeHeight : (int)(gridData.nodeWidth * 0.9);
    const int fontSize = (int)((textHeight / textScale) / FONTSIZE_TO_PIXEL);
    TTF_Font* font = TTF_OpenFont(FONT_NAME.data(), fontSize);

    for (int x = 0; x < gridData.gridWidth; x++)
    {
        for (int y = 0; y < gridData.gridHeight; y++)
        {
            SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, Grid::generateNodeName(x, y).data(), GridColor::TEXT);
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderData.renderer, textSurface);

            auto [xPos, yPos] = getScreenCoordinates(x, y);
            xPos += (gridData.nodeWidth - textSurface->w) / 2;
            yPos += (gridData.nodeHeight - textSurface->h) / 2;
            SDL_Rect rect = { xPos, yPos, textSurface->w, textSurface->h };
            SDL_RenderCopy(renderData.renderer, textTexture, NULL, &rect);

            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);
        }
    }

    TTF_CloseFont(font);
    SDL_SetRenderTarget(renderData.renderer, NULL);
}

void Grid::searchInitialize(std::unique_ptr<Pathfinder>&& pathfinder, const Node& start, const Node& end)
{
    Environment::searchInitialize(move(pathfinder), start, end);

    // set heuristic in case of AStar
    AStar* pathfinderAsAStar = dynamic_cast<AStar*>(searchData.pathfinder.get());
    if (pathfinderAsAStar) pathfinderAsAStar->getHeuristic = [](const Graph& graph, const Node& current, const Node& target)
    {
        // Manhattan Distance
        const auto [currentX, currentY] = Grid::getGridCoordinates(current);
        const auto [targetX, targetY] = Grid::getGridCoordinates(target);
        return (float)(abs(targetX - currentX) + abs(targetY - currentY));
    };
}

const SDL_Point Grid::getGridCoordinates(const Node& node)
{
    using namespace std;

    int x, y;
    const string name = node.getName();
    const size_t midPoint = name.find(",");

    const string tmp1 = name.substr(0, midPoint);
    from_chars(tmp1.data(), tmp1.data() + tmp1.size(), x);

    const string tmp2 = name.substr(midPoint + 2);
    from_chars(tmp2.data(), tmp2.data() + tmp2.size(), y);
    
    return SDL_Point{ x, y };
}

const SDL_Point Grid::getScreenCoordinates(const Node& node) const
{
    const auto [x, y] = getGridCoordinates(node);
    return getScreenCoordinates(x, y);
}

const SDL_Point Grid::getScreenCoordinates(const int gridX, const int gridY) const
{
    const int xPos = (gridData.nodeWidth * gridX) + (gridData.borderWidth * gridX) + gridData.marginWidth;
    const int yPos = (gridData.nodeHeight * gridY) + (gridData.borderHeight * gridY) + gridData.marginHeight;
    return SDL_Point{ xPos, yPos };
}

const SDL_Color Grid::getColorOf(NodeState nodeState) const
{
    switch (nodeState)
    {
        case NodeState::DEFAULT: return GridColor::NODE_DEFAULT;
        case NodeState::CURRENT: return GridColor::NODE_CURRENT;
        case NodeState::DISCOVERED: return GridColor::NODE_DISCOVERED;
        case NodeState::PROCESSED: return GridColor::NODE_PROCESSED;
    }

    throw std::exception("Unmatched node state");
}
