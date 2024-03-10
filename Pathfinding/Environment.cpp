#include "Environment.h"

using namespace Pathfinding;

Environment::Environment(SDL_Window* window)
{
    renderData.window = window;
    renderData.renderer = SDL_CreateRenderer(renderData.window, -1, SDL_RENDERER_ACCELERATED);
}

Environment::~Environment()
{
    SDL_DestroyTexture(renderData.textureGraph);
    SDL_DestroyTexture(renderData.textureEdgeWeights);
    SDL_DestroyTexture(renderData.textureSearchConnections);
    SDL_DestroyTexture(renderData.textureSearchValues);
    SDL_DestroyTexture(renderData.textureCoordinates);
    SDL_DestroyRenderer(renderData.renderer);
}

void Environment::refreshWindow()
{
    if (renderData.textureGraph) SDL_DestroyTexture(renderData.textureGraph);
    if (renderData.textureEdgeWeights) SDL_DestroyTexture(renderData.textureEdgeWeights);
    if (renderData.textureSearchConnections) SDL_DestroyTexture(renderData.textureSearchConnections);
    if (renderData.textureSearchValues) SDL_DestroyTexture(renderData.textureSearchValues);
    if (renderData.textureCoordinates) SDL_DestroyTexture(renderData.textureCoordinates);

    SDL_GetRendererOutputSize(renderData.renderer, &renderData.windowWidth, &renderData.windowHeight);

    renderData.textureGraph = SDL_CreateTexture(renderData.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, renderData.windowWidth, renderData.windowHeight);
    renderData.textureEdgeWeights = SDL_CreateTexture(renderData.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, renderData.windowWidth, renderData.windowHeight);
    renderData.textureSearchConnections = SDL_CreateTexture(renderData.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, renderData.windowWidth, renderData.windowHeight);
    renderData.textureSearchValues = SDL_CreateTexture(renderData.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, renderData.windowWidth, renderData.windowHeight);
    renderData.textureCoordinates = SDL_CreateTexture(renderData.renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, renderData.windowWidth, renderData.windowHeight);

    SDL_SetTextureBlendMode(renderData.textureEdgeWeights, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(renderData.textureSearchConnections, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(renderData.textureSearchValues, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(renderData.textureCoordinates, SDL_BLENDMODE_BLEND);

    drawGraph();
    applyNodeStates();
    drawEdgeWeights();
    drawCoordinates();
    renderEnvironment();
}

void Environment::renderEnvironment() const
{
    SDL_RenderCopy(renderData.renderer, renderData.textureGraph, NULL, NULL);
    if (renderData.renderLayerMask[0]) SDL_RenderCopy(renderData.renderer, renderData.textureEdgeWeights, NULL, NULL);
    if (renderData.renderLayerMask[1]) SDL_RenderCopy(renderData.renderer, renderData.textureSearchConnections, NULL, NULL);
    if (renderData.renderLayerMask[2]) SDL_RenderCopy(renderData.renderer, renderData.textureSearchValues, NULL, NULL);
    if (renderData.renderLayerMask[3]) SDL_RenderCopy(renderData.renderer, renderData.textureCoordinates, NULL, NULL);
    SDL_RenderPresent(renderData.renderer);
}

void Environment::applyNodeStates() const
{
    for (auto nodeState : nodeStates)
    {
        drawNode(*nodeState.first, nodeState.second.nodeColor);
        drawSearchData(*nodeState.first, nodeState.second.pathData);
    }
}

void Environment::drawSearchData(const Node& node, const PathData& pathData) const
{
    drawConnections(node, pathData);
    drawPathWeights(node, pathData);
}

void Environment::searchInitialize(std::unique_ptr<Pathfinder>&& pathfinder, const Node& start, const Node& end)
{
    searchData.pathfinder = move(pathfinder);
    if (searchData.pathfinder) searchData.searchCoroutine = searchData.pathfinder->search(*graph, start, end, searchData.incrementalSearch);

    resetRenderState();
    renderEnvironment();
}

bool Environment::searchRun()
{
    // guard for not initialized
    if (!searchData.pathfinder) return false;

    // execute search step
    Coroutine& coroutine = searchData.searchCoroutine;
    if (coroutine.isDone()) return false;
    coroutine();

    // visualize search step
    if (searchData.incrementalSearch)
    {
        for (SearchData& logItem : searchData.pathfinder->searchLog)
        {
            SDL_Color nodeColor = getColorOf(logItem.state);
            drawNode(*logItem.node, nodeColor);
            drawSearchData(*logItem.node, logItem.pathData);
            nodeStates[logItem.node] = { nodeColor, logItem.pathData };
        }
        renderEnvironment();
    }
    searchData.pathfinder->searchLog.clear();

    if (!coroutine.isDone()) return true;

    auto searchResult = searchData.pathfinder->searchResult;
    if (!searchResult->pathFound) return false;

    // visualize search result
    drawPath(searchResult->path);
    renderEnvironment();

    std::cout << "PathWeight: " << searchResult->pathWeight << ", Nodes explored: " << searchResult->nodesExplored << ", Runtime: " << searchResult->runtime.count() << "ns\n";

    return false;
}