#include <iostream>
#include <chrono>
#include "vendor/path_finder/include/CHGraph.h"
#include "vendor/path_finder/include/GraphReader.h"
#include "vendor/path_finder/include/CHDijkstra.h"
#include "vendor/path_finder/include/HubLabels.h"

int main(int argc, char* argv[]) {
    std::string filepath;
    int level = 0;
    for(int i = 1; i < argc; ++i) {
        std::string option = argv[i];
        if(option == "-f") {
            filepath = argv[++i];
        }
        if(option == "-l")
            level = std::stoi(argv[++i]);
    }
    std::cout << filepath << std::endl;
    pathFinder::CHGraph chGraph;
    pathFinder::GraphReader::readCHFmiFile(chGraph, filepath);
    pathFinder::CHDijkstra hl(chGraph);
    while(true) {
        std::cout << "source: ";
        pathFinder::NodeId source;
        std::cin >> source;
        pathFinder::NodeId target;
        std::cout << "target: ";
        std::cin >> target;
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "Distance: " <<  hl.getShortestDistance(source, target).value() << std::endl;
        auto finish = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(finish - start);
        std::cout << "Elapsed time: " << elapsed.count() << " µs\n";
    }

    return 0;
}
