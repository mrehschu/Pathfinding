#include "Pathfinding.h"

bool autoPlay = false;
Uint32 autoPlayDelayMs = 500;
const Uint32 autoPlayDelaySteps = 50;


int main(int argc, char* argv[])
{
    using namespace std;
    using namespace Pathfinding;


    if (TTF_Init() != 0) return -1;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0) return -1;
    SDL_Window* window = SDL_CreateWindow("Pathfinding.exe", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1000, 1000, SDL_WINDOW_RESIZABLE);

    unique_ptr<Environment> environment ( (Environment*) new Grid(15, 15, window) );
    unique_ptr<Pathfinder> pathfinder;

    auto& startNode = environment->getGraph()->getNode(Grid::generateNodeName(0, 0));
    auto& endNode = environment->getGraph()->getNode(Grid::generateNodeName(14, 14));


    auto pushAutoPlayEvent = [](Uint32 _, void* params)
    {
        SDL_Event sdlEvent;
        SDL_UserEvent sdlUserEvent;

        sdlUserEvent.type = SDL_USEREVENT;
        sdlEvent.type = SDL_USEREVENT;
        sdlEvent.user = sdlUserEvent;

        SDL_PushEvent(&sdlEvent);
        return autoPlay ? autoPlayDelayMs : (Uint32)0;
    };

    SDL_Event sdlEvent;
    bool currentLayerState;
    bool exitProgram = false;
    while (!exitProgram && SDL_WaitEvent(&sdlEvent))
    {
        switch (sdlEvent.type)
        {
            case SDL_KEYDOWN:
                switch (sdlEvent.key.keysym.sym)
                {
                    case SDLK_F1:
                        currentLayerState = environment->isRenderLayerActive(Environment::RenderLayer::EdgeWeights);
                        environment->setRenderLayer(Environment::RenderLayer::EdgeWeights, !currentLayerState);
                        break;

                    case SDLK_F2:
                        currentLayerState = environment->isRenderLayerActive(Environment::RenderLayer::SearchConnections);
                        environment->setRenderLayer(Environment::RenderLayer::SearchConnections, !currentLayerState);
                        break;

                    case SDLK_F3:
                        currentLayerState = environment->isRenderLayerActive(Environment::RenderLayer::SearchValues);
                        environment->setRenderLayer(Environment::RenderLayer::SearchValues, !currentLayerState);
                        break;

                    case SDLK_F4:
                        currentLayerState = environment->isRenderLayerActive(Environment::RenderLayer::Coordinates);
                        environment->setRenderLayer(Environment::RenderLayer::Coordinates, !currentLayerState);
                        break;

                    case SDLK_1:
                        autoPlay = false;
                        pathfinder = unique_ptr<Pathfinder>((Pathfinder*) new DepthFirst());
                        environment->searchInitialize(move(pathfinder), *startNode, *endNode);
                        break;

                    case SDLK_2:
                        autoPlay = false;
                        pathfinder = unique_ptr<Pathfinder>((Pathfinder*) new BreadthFirst());
                        environment->searchInitialize(move(pathfinder), *startNode, *endNode);
                        break;

                    case SDLK_3:
                        autoPlay = false;
                        pathfinder = unique_ptr<Pathfinder>((Pathfinder*) new Dijkstra());
                        environment->searchInitialize(move(pathfinder), *startNode, *endNode);
                        break;

                    case SDLK_4:
                        autoPlay = false;
                        pathfinder = unique_ptr<Pathfinder>((Pathfinder*) new AStar());
                        environment->searchInitialize(move(pathfinder), *startNode, *endNode);
                        break;

                    case SDLK_BACKSPACE:
                        autoPlay = false;
                        // pathfinder is empty, this is by design
                        environment->searchInitialize(move(pathfinder), *startNode, *endNode);
                        break;

                    case SDLK_RSHIFT:
                        autoPlay = !autoPlay;
                        environment->setIncrementalSearch(true);
                        if (autoPlay) SDL_AddTimer(autoPlayDelayMs, pushAutoPlayEvent, NULL);
                        break;

                    case SDLK_PLUS:
                        autoPlayDelayMs = max(autoPlayDelayMs - autoPlayDelaySteps, autoPlayDelaySteps);
                        break;

                    case SDLK_MINUS:
                        autoPlayDelayMs = min(autoPlayDelayMs + autoPlayDelaySteps, (Uint32)1000);
                        break;

                    case SDLK_RETURN:
                        autoPlay = false;
                        environment->setIncrementalSearch(true);
                        environment->searchRun();
                        break;

                    case SDLK_SPACE:
                        autoPlay = false;
                        environment->setIncrementalSearch(false);
                        environment->searchRun();
                        break;

                    case SDLK_ESCAPE:
                        exitProgram = true;
                        break;
                }
                break;

            case SDL_USEREVENT:
                switch (sdlEvent.user.type)
                {
                    case SDL_USEREVENT:
                        if (autoPlay) autoPlay = environment->searchRun();
                        break;
                }
                break;

            case SDL_WINDOWEVENT:
                switch (sdlEvent.window.event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                        environment->refreshWindow();
                        break;
                }
                break;

            case SDL_QUIT:
                exitProgram = true;
                break;
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}