//
// Created by sokol on 31.03.20.
//
#include <string>
#include "vendor/path_finder/include/Static.h"
#include "vendor/path_finder/include/CHGraph.h"
#include "vendor/path_finder/include/GraphReader.h"
#include "vendor/path_finder/include/DataConfig.h"
#include "vendor/path_finder/include/HubLabels.h"

int main(int argc, char* argv[]) {
    std::string filepath;
    int level = 0;
    bool gridReorder = false;
    for(int i = 1; i < argc; ++i) {
        std::string option = argv[i];
        if (option == "-f")
            filepath = argv[++i];
        else if (option == "-l")
            level = std::stoi(argv[++i]);
        else if (option == "-gridReorder")
          gridReorder = true;
    }

    // read graph
    pathFinder::CHGraph chGraph;
    pathFinder::GraphReader::readCHFmiFile(chGraph, filepath, gridReorder);
    std::cout << "read file" << std::endl;
    std::string folderName = "data";
    std::string command = "mkdir " + folderName;
    system(command.c_str());

    // set graph config values
    pathFinder::DataConfig dataConfig;
    dataConfig.graphName = filepath;
    dataConfig.numberOfNodes = chGraph.getNodes().size();
    dataConfig.numberOfEdges = chGraph.getForwardEdges().size();

    dataConfig.nodes = {folderName + "/nodes", chGraph.getNodes().size(), true};
    dataConfig.forwardEdges = {folderName + "/forwardEdges", chGraph.getForwardEdges().size(), true};
    dataConfig.forwardOffset = {folderName + "/forwardOffset", chGraph.getForwardOffset().size(), false};
    dataConfig.backwardEdges = {folderName + "/backwardEdges", chGraph.getBackEdges().size(), true};
    dataConfig.backwardOffset = {folderName + "/backwardOffset", chGraph.getBackOffset().size(), false};

    // populate config grid map
    for(auto [latLng, pointer]: chGraph.gridMap) {
      dataConfig.gridMapEntries.emplace_back(pathFinder::GridMapEntry{latLng, pointer});

    }
    std::cout << "gridMap length " << chGraph.gridMap.size() << std::endl;

    // write graph files
    using namespace pathFinder;
    Static::writeVectorToFile(chGraph.getNodes(), dataConfig.nodes.path.c_str());
    Static::writeVectorToFile(chGraph.getForwardEdges(), dataConfig.forwardEdges.path.c_str());
    Static::writeVectorToFile(chGraph.getForwardOffset(), dataConfig.forwardOffset.path.c_str());
    Static::writeVectorToFile(chGraph.getBackEdges(), dataConfig.backwardEdges.path.c_str());
    Static::writeVectorToFile(chGraph.getBackOffset(), dataConfig.backwardOffset.path.c_str());

    // construct hub labels
    pathFinder::Timer timer;
    pathFinder::HubLabels hubLabels(chGraph, level, timer);
    auto& hlStore = hubLabels.getHublabelStore();
    std::cout << hlStore.getForwardLabels().size() << std::endl;
    // set hub label config values
    dataConfig.calculatedUntilLevel = level;
    dataConfig.forwardHublabels = {folderName + "/forwardHubLabels", hlStore.getForwardLabels().size(), true};
    dataConfig.backwardHublabels = { folderName + "/backwardHubLabels", hlStore.getBackwardLabels().size(), true};
    dataConfig.forwardHublabelOffset = { folderName + "/forwardHubLabelOffset", hlStore.getForwardLabels().size(), false};
    dataConfig.backwardHublabelOffset = {folderName + "/backwardHubLabelOffset", hlStore.getBackwardOffset().size(), false};

    // write hub label files
    Static::writeVectorToFile(hlStore.getForwardLabels(), dataConfig.forwardHublabels.path.c_str());
    Static::writeVectorToFile(hlStore.getBackwardLabels(), dataConfig.backwardHublabels.path.c_str());
    Static::writeVectorToFile(hlStore.getForwardOffset(), dataConfig.forwardHublabelOffset.path.c_str());
    Static::writeVectorToFile(hlStore.getBackwardOffset(), dataConfig.backwardHublabelOffset.path.c_str());

    //write config to file
    std::ofstream out(folderName + "/config.json");
    out << dataConfig.toJson();
    out.close();

    return 0;
}
