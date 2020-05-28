#include <boost/iostreams/write.hpp>
#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <path_finder/ChHlBenchmarker.h>
#include <path_finder/GraphReader.h>
#include <path_finder/PathFinderBase.h>
#include <vector>

void loop(std::vector<pathFinder::PathFinderBase*> pathFinders){
    while(true) {
        std::cout << "source: ";
        pathFinder::NodeId source;
        std::cin >> source;
        pathFinder::NodeId target;
        std::cout << "target: ";
        std::cin >> target;
        for(auto pathFinder : pathFinders) {
            auto start = std::chrono::high_resolution_clock::now();
            std::cout << "Distance: " <<  pathFinder->getShortestDistance(source, target).value() << std::endl;
            auto finish = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
            std::cout << "Elapsed time: " << elapsed.count() << " µs\n";
            int fd = ::open("/proc/sys/vm/drop_caches", O_WRONLY);
            if (2 != ::write(fd, "1\n", 2)) {
                throw std::runtime_error("Benchmarker: could not drop caches");
            }
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
    std::string filepath;
    int level = 0;
    bool gridReorder = false;
    enum AlgorithmType {
        pureCH = 0,
        hybrid = 1,
        bench = 3
    };
    enum MemoryType{
        ram = 0,
        disk = 1
    };
    AlgorithmType algorithmType = hybrid;
    MemoryType memoryType = ram;
    for(int i = 1; i < argc; ++i) {
        std::string option = argv[i];
        if(option == "-f")
            filepath = argv[++i];
        else if(option == "-l")
            level = std::stoi(argv[++i]);
        else if(option == "-a") {
            std::string algString = argv[++i];
            if(algString == "ch")
                algorithmType = pureCH;
            else if(algString == "hybrid") {
                algorithmType = hybrid;
            }
            else if(algString == "bench") {
                algorithmType = bench;
            }
        }
        else if(option == "-m") {
            std::string memoryString = argv[++i];
            if(memoryString == "ram")
                memoryType = ram;
            else if(memoryString == "disk")
                memoryType = disk;
        }
        else if(option == "-gridReorder")
          gridReorder = true;
    }
    std::cout << filepath << std::endl;
    pathFinder::CHGraph chGraph;
    pathFinder::GraphReader::readCHFmiFile(chGraph, filepath, gridReorder);
    pathFinder::ChHlBenchmarker bm(chGraph);
    pathFinder::Timer timer;
    pathFinder::CellIdStore cellIdStore(chGraph.edges.size());

    switch(algorithmType){
        case hybrid:{
            pathFinder::HubLabels hl(chGraph, level, timer, cellIdStore);
            if(memoryType == disk) {
                std::cout << "writing to disk" << std::endl;
                auto& ramHlStore = hl.getHublabelStore();

                auto mmapNodes = pathFinder::MmapVector(chGraph.getNodes(), "nodes");
                auto mmapForwardEdges = pathFinder::MmapVector(chGraph.getForwardEdges(), "forwardEdges");
                auto mmapBackwardEdges = pathFinder::MmapVector(chGraph.getBackEdges(), "backwardEdges");
                auto diskGraph = pathFinder::CHGraph(mmapNodes, mmapForwardEdges, mmapBackwardEdges, chGraph.getForwardOffset(), chGraph.getBackOffset(), chGraph.numberOfNodes);
                auto mmapForwardLabels = pathFinder::MmapVector(ramHlStore.getForwardLabels(), "forwardLabels");
                auto mmapBackwardLabels = pathFinder::MmapVector(ramHlStore.getBackwardLabels(), "backwardLabels");
                pathFinder::HubLabelStore diskHlStore(mmapForwardLabels, mmapBackwardLabels, ramHlStore.getForwardOffset(), ramHlStore.getBackwardOffset());
                pathFinder::HubLabels diskHl(diskGraph, level, diskHlStore, timer, cellIdStore);
                ramHlStore.getBackwardLabels().clear();
                ramHlStore.getBackwardLabels().shrink_to_fit();
                ramHlStore.getForwardLabels().clear();
                ramHlStore.getForwardLabels().shrink_to_fit();
                chGraph.deleteEdges();
                chGraph.deleteNodes();

                int fd = ::open("/proc/sys/vm/drop_caches", O_WRONLY);


                loop({&diskHl});
            } else
                loop({&hl});
        }
            break;
        case pureCH:{
            if(memoryType == disk) {
                auto mmapNodes = pathFinder::MmapVector(chGraph.getNodes(), "nodes");
                auto mmapForwardEdges = pathFinder::MmapVector(chGraph.getForwardEdges(), "forwardEdges");
                auto mmapBackwardEdges = pathFinder::MmapVector(chGraph.getBackEdges(), "backwardEdges");
                auto diskGraph = pathFinder::CHGraph(mmapNodes, mmapForwardEdges, mmapBackwardEdges, chGraph.getForwardOffset(), chGraph.getBackOffset(), chGraph.numberOfNodes);
                pathFinder::CHDijkstra  ch(diskGraph);
                chGraph.deleteNodes();
                chGraph.deleteEdges();
                int fd = ::open("/proc/sys/vm/drop_caches", O_WRONLY);
                if (2 != ::write(fd, "1\n", 2)) {
                    throw std::runtime_error("Benchmarker: could not drop caches");
                }
                loop({&ch});
            } else if(memoryType == ram) {
                pathFinder::CHDijkstra  ch(chGraph);
                loop({&ch});
            }

            }

            break;
        case bench:
            bm.compareSpeed("hl-ram.bench", level, true);
            break;
    }
    return 0;
}
