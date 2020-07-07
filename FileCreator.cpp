//
// Created by sokol on 31.03.20.
//
#include "path_finder/CHGraph.h"
#include "path_finder/DataConfig.h"
#include "path_finder/GraphReader.h"
#include "path_finder/Static.h"
#include <liboscar/StaticOsmCompleter.h>
#include <liboscar/routing/support/Edge2CellIds.h>
#include <path_finder/HubLabelCreator.h>
#include <path_finder/OscarIntegration.h>
#include <path_finder/Timer.h>
#include <string>

int main(int argc, char* argv[]) {
    std::string filepath;
    std::string oscarFilePath;
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
        else if (option == "-oscarFiles")
          oscarFilePath = argv[++i];
    }

  liboscar::Static::OsmCompleter cmp;

    // read graph
    pathFinder::CHGraph chGraph;
    pathFinder::GraphReader::readCHFmiFile(chGraph, filepath, gridReorder);

    // read cellIds
  pathFinder::CellIdStore<std::vector> cellIdStore(chGraph.edges.size());
  if(!oscarFilePath.empty()) {
    std::cout << "reading oscar files..." << std::endl;
    cmp.setAllFilesFromPrefix(oscarFilePath);
    try {
      cmp.energize();
    }
    catch (std::exception const & e) {
      std::cerr << "Error: " << e.what() << std::endl;
      return -1;
    }
    pathFinder::OscarIntegrator::writeCellIdsForEdges<sserialize::spatial::GeoPoint>
        (chGraph, liboscar::routing::support::Edge2CellIds(cmp.store()), pathFinder::CellIdDiskWriter(cellIdStore));
  }
    std::cout << "read graph file" << std::endl;
    std::string folderName = "data";
    std::string command = "mkdir " + folderName;
    system(command.c_str());

    // set graph config values
    pathFinder::DataConfig dataConfig;
    dataConfig.graphName = filepath;
    dataConfig.numberOfNodes = chGraph.getNodes().size();
    dataConfig.numberOfEdges = chGraph.getForwardEdges().size();

    dataConfig.nodes = {"nodes", chGraph.getNodes().size(), true};
    dataConfig.forwardEdges = {"forwardEdges", chGraph.getForwardEdges().size(), true};
    dataConfig.forwardOffset = {"forwardOffset", chGraph.getForwardOffset().size(), false};
    dataConfig.backwardEdges = {"backwardEdges", chGraph.getBackEdges().size(), true};
    dataConfig.backwardOffset = {"backwardOffset", chGraph.getBackOffset().size(), false};
    dataConfig.cellIds = {"cellIds", cellIdStore.cellIdSize()};
    dataConfig.cellIdsOffset = {"cellIdsOffset", cellIdStore.offsetSize()};
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

    // write cell ids
    Static::writeVectorToFile(cellIdStore.cellIdsVec(), dataConfig.cellIds.path.c_str());
    Static::writeVectorToFile(cellIdStore.offsetVec(), dataConfig.cellIdsOffset.path.c_str());

    // construct hub labels
    pathFinder::HubLabelStore hlStore(dataConfig.numberOfNodes);
    pathFinder::HubLabelCreator hubLabelCreator(chGraph, hlStore);
    hubLabelCreator.create(level);
    std::cout << hlStore.getForwardLabels().size() << std::endl;
    // set hub label config values
    dataConfig.calculatedUntilLevel = level;
    dataConfig.forwardHublabels = {"forwardHubLabels", hlStore.getForwardLabels().size(), true};
    dataConfig.backwardHublabels = { "backwardHubLabels", hlStore.getBackwardLabels().size(), true};
    dataConfig.forwardHublabelOffset = { "forwardHubLabelOffset", hlStore.getForwardLabels().size(), false};
    dataConfig.backwardHublabelOffset = {"backwardHubLabelOffset", hlStore.getBackwardOffset().size(), false};

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
