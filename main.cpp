#include <iostream>
#include <chrono>
#include "vendor/path_finder/include/CHGraph.h"
#include "vendor/path_finder/include/GraphReader.h"
#include "vendor/path_finder/include/CHDijkstra.h"
#include "vendor/path_finder/include/HubLabels.h"

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
    }
};

int main(int argc, char* argv[]) {
    std::string filepath;
    int level = 0;
    enum methodEnum{
        hl = 0,
        ch = 1
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
            if(methodStr == "hl"){
                method = hl;
            }
        }

    }
    std::cout << filepath << std::endl;
    pathFinder::CHGraph chGraph;
    pathFinder::GraphReader::readCHFmiFile(chGraph, filepath);
    switch(method){
        case hl:{
            pathFinder::HubLabels hl(chGraph, level);
            pathFinder::CHDijkstra ch(chGraph);
            //hl.writeToFile("stgtregbz.hl");
            loop(hl, ch);
        }
            break;
        case ch:
            pathFinder::HubLabels hl(chGraph, level);
            loop(hl, pathFinder::CHDijkstra(chGraph));
            break;
    }
    return 0;
}
