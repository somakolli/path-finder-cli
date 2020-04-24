//
// Created by sokol on 31.03.20.
//
#include <string>
#include <string>
#include <fstream>
#include <streambuf>
#include <chrono>
#include <fcntl.h>
#include "vendor/path_finder/include/DataConfig.h"
#include "vendor/path_finder/include/Static.h"
#include "vendor/path_finder/include/CHGraph.h"
#include "vendor/path_finder/include/PathFinderBase.h"
#include "vendor/path_finder/include/CHDijkstra.h"

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
            if(!distance.has_value())
                std::cerr << "source or target not found" << std::endl;
            else
                std::cout << "Distance: " << distance.value() << std::endl;
            auto finish = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
            std::cout << "Elapsed time: " << elapsed.count() << " µs\n";
        }

        /*
        int fd = ::open("/proc/sys/vm/drop_caches", O_WRONLY);
        if (2 != ::write(fd, "1\n", 2)) {
            throw std::runtime_error("Benchmarker: could not drop caches");
        }
         */
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
        auto nodes = pathFinder::Static::getFromFileMMap<pathFinder::CHNode>(config.nodes);
        auto forwardEdges = pathFinder::Static::getFromFileMMap<pathFinder::Edge>(config.forwardEdges);
        auto backwardEdges = pathFinder::Static::getFromFileMMap<pathFinder::Edge>(config.backwardEdges);
        auto forwardOffset = pathFinder::Static::getFromFileMMap<pathFinder::NodeId>(config.forwardOffset);
        auto backwardOffset = pathFinder::Static::getFromFileMMap<pathFinder::NodeId>(config.backwardOffset);
        auto forwardHublabels = pathFinder::Static::getFromFileMMap<pathFinder::CostNode>(config.forwardHublabels);
        auto backwardHublabels = pathFinder::Static::getFromFileMMap<pathFinder::CostNode>(config.backwardHublabels);
        auto forwardHublabelOffset = pathFinder::Static::getFromFileMMap<pathFinder::OffsetElement>(config.forwardHublabelOffset);
        auto backwardHublabelOffset = pathFinder::Static::getFromFileMMap<pathFinder::OffsetElement>(config.backwardHublabelOffset);
        pathFinder::CHGraph chGraph(nodes, forwardEdges, backwardEdges, forwardOffset, backwardOffset, config.numberOfNodes);

        // set up grid
        for(auto gridEntry : config.gridMapEntries) {
            chGraph.gridMap[gridEntry.latLng] = gridEntry.pointerPair;
        }

        std::cout << "gridMap size: " << chGraph.gridMap.size() << std::endl;

        pathFinder::HubLabelStore hubLabelStore(forwardHublabels, backwardHublabels, forwardHublabelOffset, backwardHublabelOffset);
        pathFinder::Timer timer;
        pathFinder::HubLabels hl(chGraph, config.calculatedUntilLevel, hubLabelStore, timer);

        loop({&hl});
    } else {
        auto nodes = pathFinder::Static::getFromFile<pathFinder::CHNode>(config.nodes);
        auto forwardEdges = pathFinder::Static::getFromFile<pathFinder::Edge>(config.forwardEdges);
        auto backwardEdges = pathFinder::Static::getFromFile<pathFinder::Edge>(config.backwardEdges);
        auto forwardOffset = pathFinder::Static::getFromFile<pathFinder::NodeId>(config.forwardOffset);
        auto backwardOffset = pathFinder::Static::getFromFile<pathFinder::NodeId>(config.backwardOffset);
        auto forwardHublabels = pathFinder::Static::getFromFile<pathFinder::CostNode>(config.forwardHublabels);
        auto backwardHublabels = pathFinder::Static::getFromFile<pathFinder::CostNode>(config.backwardHublabels);
        auto forwardHublabelOffset = pathFinder::Static::getFromFile<pathFinder::OffsetElement>(config.forwardHublabelOffset);
        auto backwardHublabelOffset = pathFinder::Static::getFromFile<pathFinder::OffsetElement>(config.backwardHublabelOffset);
        pathFinder::CHGraph chGraph(nodes, forwardEdges, backwardEdges, forwardOffset, backwardOffset, config.numberOfNodes);

        // set up grid
        for(auto gridEntry : config.gridMapEntries) {
            chGraph.gridMap[gridEntry.latLng] = gridEntry.pointerPair;
        }

        std::cout << "gridMap size: " << chGraph.gridMap.size() << std::endl;

        pathFinder::HubLabelStore hubLabelStore(forwardHublabels, backwardHublabels, forwardHublabelOffset, backwardHublabelOffset);
        pathFinder::Timer timer;
        pathFinder::HubLabels hl(chGraph, config.calculatedUntilLevel, hubLabelStore, timer);

        loop({&hl});
    }

    //pathFinder::CHDijkstra  ch(chGraph);

}

