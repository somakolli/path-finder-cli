//
// Created by sokol on 31.03.20.
//
#include "path_finder/CHDijkstra.h"
#include "path_finder/CHGraph.h"
#include "path_finder/DataConfig.h"
#include "path_finder/PathFinderBase.h"
#include "path_finder/Static.h"
#include <chrono>
#include <fcntl.h>
#include <fstream>
#include <path_finder/CellIdStore.h>
#include <streambuf>
#include <string>

void loop(std::vector<pathFinder::PathFinderBase*> pathFinders) {
    while(true) {
        std::cout << "source: ";
        pathFinder::NodeId source;
        std::cin >> source;
        pathFinder::NodeId target;
        std::cout << "target: ";
        std::cin >> target;
        for(auto pathFinder : pathFinders) {
            auto start = std::chrono::high_resolution_clock::now();
            auto distance = pathFinder->getShortestDistance(source, target);
            std::vector<unsigned int> cellIds;
            auto path = pathFinder->getShortestPath(source, target, &cellIds);
            std::cout << cellIds.size() << '\n';
            if(!distance.has_value())
                std::cerr << "source or target not found" << std::endl;
            else
                std::cout << "Distance: " << distance.value() << std::endl;
            auto finish = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
            std::cout << "Elapsed time: " << elapsed.count() << " µs\n";
        }


        int fd = ::open("/proc/sys/vm/drop_caches", O_WRONLY);
        if (2 != ::write(fd, "1\n", 2)) {
            throw std::runtime_error("Benchmarker: could not drop caches");
        }

    }
};
int main(int argc, char* argv[]) {
    std::string configFolder;
    bool ram = false;
    for(int i = 1; i < argc; ++i) {
        std::string option = argv[i];
        if(option == "-f")
            configFolder = argv[++i];
        if(option == "-ram")
            ram = true;
    }

    // read config
    std::ifstream t(configFolder + "/config.json");
    std::string str((std::istreambuf_iterator<char>(t)),
                    std::istreambuf_iterator<char>());
    auto config = pathFinder::DataConfig::getFromFile(str);
    std::cout << "read config" << std::endl;


    if(!ram) {
      using namespace pathFinder;
        auto nodes = Static::getFromFileMMap<CHNode>(config.nodes, configFolder);
        auto forwardEdges = Static::getFromFileMMap<CHEdge>(config.forwardEdges, configFolder);
        auto backwardEdges = Static::getFromFileMMap<CHEdge>(config.backwardEdges, configFolder);
        auto forwardOffset = Static::getFromFileMMap<NodeId>(config.forwardOffset, configFolder);
        auto backwardOffset = Static::getFromFileMMap<NodeId>(config.backwardOffset, configFolder);
        auto forwardHublabels = Static::getFromFileMMap<CostNode>(config.forwardHublabels, configFolder);
        auto backwardHublabels = Static::getFromFileMMap<CostNode>(config.backwardHublabels, configFolder);
        auto forwardHublabelOffset = Static::getFromFileMMap<OffsetElement>(config.forwardHublabelOffset, configFolder);
        auto backwardHublabelOffset = Static::getFromFileMMap<OffsetElement>(config.backwardHublabelOffset, configFolder);
        auto cellIds = Static::getFromFileMMap<CellId_t>(config.cellIds, configFolder);
        auto cellIdsOffset = Static::getFromFileMMap<OffsetElement>(config.cellIdsOffset, configFolder);
        CHGraph chGraph(nodes, forwardEdges, backwardEdges, forwardOffset, backwardOffset, config.numberOfNodes);
        auto cellIdStore = CellIdStore(cellIds, cellIdsOffset);
        // set up grid
        for(auto gridEntry : config.gridMapEntries) {
            chGraph.gridMap[gridEntry.latLng] = gridEntry.pointerPair;
        }

        std::cout << "gridMap size: " << chGraph.gridMap.size() << std::endl;

        pathFinder::HubLabelStore hubLabelStore(forwardHublabels, backwardHublabels, forwardHublabelOffset, backwardHublabelOffset);
        pathFinder::Timer timer;
        pathFinder::HubLabels hl(chGraph, config.calculatedUntilLevel, hubLabelStore, timer, cellIdStore);

        loop({&hl});
    } else {
        auto nodes = pathFinder::Static::getFromFile<pathFinder::CHNode>(config.nodes, configFolder);
        auto forwardEdges = pathFinder::Static::getFromFile<pathFinder::CHEdge>(config.forwardEdges, configFolder);
        auto backwardEdges = pathFinder::Static::getFromFile<pathFinder::CHEdge>(config.backwardEdges, configFolder);
        auto forwardOffset = pathFinder::Static::getFromFile<pathFinder::NodeId>(config.forwardOffset, configFolder);
        auto backwardOffset = pathFinder::Static::getFromFile<pathFinder::NodeId>(config.backwardOffset, configFolder);
        auto forwardHublabels = pathFinder::Static::getFromFile<pathFinder::CostNode>(config.forwardHublabels, configFolder);
        auto backwardHublabels = pathFinder::Static::getFromFile<pathFinder::CostNode>(config.backwardHublabels, configFolder);
        auto forwardHublabelOffset = pathFinder::Static::getFromFile<pathFinder::OffsetElement>(config.forwardHublabelOffset, configFolder);
        auto backwardHublabelOffset = pathFinder::Static::getFromFile<pathFinder::OffsetElement>(config.backwardHublabelOffset, configFolder);
        auto cellIds = pathFinder::Static::getFromFileMMap<pathFinder::CellId_t>(config.cellIds, configFolder);
        auto cellIdsOffset = pathFinder::Static::getFromFileMMap<pathFinder::OffsetElement>(config.cellIdsOffset, configFolder);
        pathFinder::CHGraph chGraph(nodes, forwardEdges, backwardEdges, forwardOffset, backwardOffset, config.numberOfNodes);

        // set up grid
        for(auto gridEntry : config.gridMapEntries) {
            chGraph.gridMap[gridEntry.latLng] = gridEntry.pointerPair;
        }

        std::cout << "gridMap size: " << chGraph.gridMap.size() << std::endl;

        pathFinder::HubLabelStore hubLabelStore(forwardHublabels, backwardHublabels, forwardHublabelOffset, backwardHublabelOffset);
        pathFinder::CellIdStore cellIdStore(cellIds, cellIdsOffset);
        pathFinder::Timer timer;
        pathFinder::HubLabels hl(chGraph, config.calculatedUntilLevel, hubLabelStore, timer, cellIdStore);

        loop({&hl});
    }

    //pathFinder::CHDijkstra  ch(chGraph);

}

