//
// Created by sokol on 31.03.20.
//
#include "path_finder/graphs/CHGraph.h"
#include "path_finder/helper/Static.h"
#include "path_finder/routing/CHDijkstra.h"
#include "path_finder/routing/PathFinderBase.h"
#include "path_finder/storage/DataConfig.h"
#include "vendor/oscar-routing/vendor/liboscar/include/liboscar/StaticOsmCompleter.h"
#include <chrono>
#include <fcntl.h>
#include <fstream>
#include <path_finder/routing/HubLabelCreator.h>
#include <path_finder/routing/HybridPathFinder.h>
#include <path_finder/storage/CellIdStore.h>
#include <path_finder/storage/FileLoader.h>
#include <streambuf>
#include <string>

void loop(std::vector<std::shared_ptr<pathFinder::PathFinderBase>> pathFinders) {
    while(true) {
      /*
        std::cout << "sourceLat: ";
        float sourceLat;
        std::cin >> sourceLat;
        std::cout << "sourceLng: ";
        float sourceLng;
        std::cin >> sourceLng;
        float targetLat;
        std::cout << "targetLat: ";
        std::cin >> targetLat;
        float targetLng;
        std::cout << "targetLng: ";
        std::cin >> targetLng;
        pathFinder::LatLng source = {sourceLat, sourceLng};
        pathFinder::LatLng target = {targetLat, targetLng};
        */

        for(auto pathFinder : pathFinders) {
          liboscar::Static::OsmCompleter cmp;
          std::cout << "reading oscar files..." << std::endl;
          cmp.setAllFilesFromPrefix("stgt");
          try {
            cmp.energize();
          }
          catch (std::exception const & e) {
            std::cerr << "Error: " << e.what() << std::endl;
          }


          auto start = std::chrono::high_resolution_clock::now();
          auto routingResult = pathFinder->getShortestPath(pathFinder::LatLng{48.758834048201095, 9.141569137573244}, pathFinder::LatLng{48.758940139629686, 9.141751527786257});
          std::cout << routingResult.path.size() << '\n';
          if(!routingResult.distance)
              std::cerr << "source or target not found" << std::endl;
          else
              std::cout << "Distance: " << routingResult.distance << std::endl;
          auto store = cmp.store();
          int itemCount = 0;
          for(auto cellId : routingResult.cellIds) {
            auto cell = store.geoHierarchy().cell(cellId);
            auto ptr = cell.itemPtr();
            auto size = cell.itemCount();
            auto boundary = cell.boundary();
            bool containsOnePoint = false;
            for(auto point: routingResult.path) {
              if(boundary.contains(point.lat, point.lng))
                containsOnePoint = true;
            }

            itemCount += size;
          }
          auto finish = std::chrono::high_resolution_clock::now();
          auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
        }

        /**
        int fd = ::open("/proc/sys/vm/drop_caches", O_WRONLY);
        if (2 != ::write(fd, "1\n", 2)) {
            throw std::runtime_error("Benchmarker: could not drop caches");
        }
        */


    }
};
int main(int argc, char* argv[]) {
    std::string configFolder;
    bool ram = true;
    for(int i = 1; i < argc; ++i) {
        std::string option = argv[i];
        if(option == "-f")
            configFolder = argv[++i];
        if(option == "-ram")
            ram = true;
    }


    if(!ram) {
      /*
      using namespace pathFinder;
        auto nodes = Static::getFromFileMMap<CHNode>(config.nodes, configFolder);
        auto forwardEdges = Static::getFromFileMMap<CHEdge>(config.forwardEdges, configFolder);
        auto backwardEdges = Static::getFromFileMMap<CHEdge>(config.backwardEdges, configFolder);
        auto forwardOffset = Static::getFromFileMMap<NodeId>(config.forwardOffset, configFolder);
        auto backwardOffset = Static::getFromFileMMap<NodeId>(config.backwardOffset, configFolder);
        auto forwardHubLabels = Static::getFromFileMMap<CostNode>(config.forwardHubLabels, configFolder);
        auto backwardHublabels = Static::getFromFileMMap<CostNode>(config.backwardHublabels, configFolder);
        auto forwardHublabelOffset = Static::getFromFileMMap<OffsetElement>(config.forwardHublabelOffset, configFolder);
        auto backwardHubLabelOffset = Static::getFromFileMMap<OffsetElement>(config.backwardHubLabelOffset, configFolder);
        auto cellIds = Static::getFromFileMMap<CellId_t>(config.cellIds, configFolder);
        auto cellIdsOffset = Static::getFromFileMMap<OffsetElement>(config.cellIdsOffset, configFolder);
        CHGraph chGraph(nodes, forwardEdges, backwardEdges, forwardOffset, backwardOffset, config.numberOfNodes);
        auto cellIdStore = CellIdStore(cellIds, cellIdsOffset);
        // set up grid
        for(auto gridEntry : config.gridMapEntries) {
            chGraph.gridMap[gridEntry.latLng] = gridEntry.pointerPair;
        }

        std::cout << "gridMap size: " << chGraph.gridMap.size() << std::endl;

        pathFinder::HubLabelStore hubLabelStore(forwardHubLabels, backwardHublabels, forwardHublabelOffset, backwardHubLabelOffset);
        auto hl =  std::make_shared<FileLoader::HybridPF>(HybridPathFinder(hubLabelStore, chGraph, cellIdStore, config.calculatedUntilLevel));

        loop({hl});
        */
    } else {
        auto hl = pathFinder::FileLoader::loadHubLabelsShared(configFolder);
        loop({hl});
    }

    //pathFinder::CHDijkstra  ch(chGraph);

}

