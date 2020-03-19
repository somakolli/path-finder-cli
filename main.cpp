#include <iostream>
#include <chrono>
#include <fcntl.h>
#include <fcntl.h>
#include "vendor/path_finder/include/CHGraph.h"
#include "vendor/path_finder/include/GraphReader.h"
#include "vendor/path_finder/include/CHDijkstra.h"
#include "vendor/path_finder/include/HubLabels.h"
#include "vendor/path_finder/include/ChHlBenchmarker.h"

template<typename ShopaProvider, typename ShopaProvider2>
void loop(ShopaProvider shopa, ShopaProvider2 shopa2){
    while(true) {
        std::cout << "source: ";
        pathFinder::NodeId source;
        std::cin >> source;
        pathFinder::NodeId target;
        std::cout << "target: ";
        std::cin >> target;
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "Distance: " <<  shopa.getShortestDistance(source, target).value() << std::endl;
        auto finish = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
        std::cout << "Elapsed time: " << elapsed.count() << " µs\n";
        start = std::chrono::high_resolution_clock::now();
        std::cout << "Distance: " <<  shopa2.getShortestDistance(source, target).value() << std::endl;
        finish = std::chrono::high_resolution_clock::now();
        elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
        std::cout << "Elapsed time: " << elapsed.count() << " µs\n";
        /*
        int fd = ::open("/proc/sys/vm/drop_caches", O_WRONLY);
        if (2 != ::write(fd, "1\n", 2)) {
            throw std::runtime_error("Benchmarker: could not drop caches");
        }
         */
    }
};

int main(int argc, char* argv[]) {
    std::string filepath;
    int level = 0;
    enum methodEnum{
        hl = 0,
        ch = 1,
        bench = 2
    };
    methodEnum method = hl;
    for(int i = 1; i < argc; ++i) {
        std::string option = argv[i];
        if(option == "-f")
            filepath = argv[++i];
        if(option == "-l")
            level = std::stoi(argv[++i]);
        if(option == "-m") {
            std::string methodStr = argv[++i];
            if(methodStr == "ch")
                method = ch;
            else if(methodStr == "hl"){
                method = hl;
            } else if(methodStr == "bench")
                method = bench;
        }

    }
    std::cout << filepath << std::endl;
    pathFinder::CHGraph chGraph;
    pathFinder::GraphReader::readCHFmiFile(chGraph, filepath);
    pathFinder::ChHlBenchmarker bm(chGraph);
    pathFinder::Timer timer;

    switch(method){
        case hl:{
            pathFinder::HubLabels hl(chGraph, level, timer);
            pathFinder::CHDijkstra ch(chGraph);
            //hl.writeToFile("stgtregbz.hl");
            hl.setLabelsUntilLevel(20);
            loop(hl, ch);
        }
            break;
        case ch:{
            pathFinder::HubLabels  hl(chGraph, level, timer);
            auto& ramHlStore = hl.getHublabelStore();

            auto mmapNodes = pathFinder::MmapVector(chGraph.getNodes(), "nodes");
            auto mmapForwardEdges = pathFinder::MmapVector(chGraph.getForwardEdges(), "forwardEdges");
            auto mmapBackwardEdges = pathFinder::MmapVector(chGraph.getBackEdges(), "backwardEdges");

            auto diskGraph = pathFinder::CHGraph(mmapNodes, mmapForwardEdges, mmapBackwardEdges, chGraph.getForwardOffset(), chGraph.getBackOffset(), chGraph.numberOfNodes);
            auto mmapForwardLabels = pathFinder::MmapVector(ramHlStore.getForwardLabels(), "forwardLabels");
            auto mmapBackwardLabels = pathFinder::MmapVector(ramHlStore.getBackwardLabels(), "backwardLabels");
            pathFinder::HubLabelStore diskHlStore(mmapForwardLabels, mmapBackwardLabels, ramHlStore.getForwardOffset(), ramHlStore.getBackwardOffset());
            pathFinder::HubLabels diskHl(diskGraph, level, diskHlStore, timer);
            ramHlStore.getBackwardLabels().clear();
            ramHlStore.getBackwardLabels().shrink_to_fit();
            ramHlStore.getForwardLabels().clear();
            ramHlStore.getForwardLabels().shrink_to_fit();
            /*
            int fd = ::open("/proc/sys/vm/drop_caches", O_WRONLY);
            if (2 != ::write(fd, "1\n", 2)) {
                throw std::runtime_error("Benchmarker: could not drop caches");
            }
             */
            loop(diskHl, pathFinder::CHDijkstra(diskGraph));
            }

            break;
        case bench:
            bm.compareSpeed("hl-ram.bench", level);
            break;
    }
    return 0;
}
