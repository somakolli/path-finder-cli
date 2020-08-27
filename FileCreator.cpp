//
// Created by sokol on 31.03.20.
//
#include "path_finder/graphs/CHGraph.h"
#include "path_finder/storage/GraphReader.h"
#include <liboscar/StaticOsmCompleter.h>
#include <liboscar/routing/support/Edge2CellIds.h>
#include <path_finder/helper/OscarIntegration.h>
#include <path_finder/routing/HubLabelCreator.h>
#include <path_finder/storage/FileWriter.h>
#include <string>

int main(int argc, char *argv[]) {
  std::string filepath;
  std::string oscarFilePath;
  std::string outPutPath = "data";
  int level = -1;
  bool gridReorder = false;
  for (int i = 1; i < argc; ++i) {
    std::string option = argv[i];
    if (option == "-f")
      filepath = argv[++i];
    else if (option == "-l")
      level = std::stoi(argv[++i]);
    else if (option == "-gridReorder")
      gridReorder = true;
    else if (option == "-oscarFiles")
      oscarFilePath = argv[++i];
    else if (option == "-out")
      outPutPath = argv[++i];
  }


  liboscar::Static::OsmCompleter cmp;
  // setup stores
  auto chGraph = std::make_shared<pathFinder::CHGraph>();

  pathFinder::GraphReader::readCHFmiFile(chGraph, filepath, gridReorder);

  // read cellIds
  auto cellIdStore = std::make_shared<pathFinder::CellIdStore>(chGraph->getNumberOfEdges());
  if (!oscarFilePath.empty()) {
    std::cout << "reading oscar files..." << std::endl;
    cmp.setAllFilesFromPrefix(oscarFilePath);
    try {
      cmp.energize();
    } catch (std::exception const &e) {
      std::cerr << "Error: " << e.what() << std::endl;
      return -1;
    }
    pathFinder::OscarIntegrator::writeCellIdsForEdges<sserialize::spatial::GeoPoint>(
        *chGraph, liboscar::routing::support::Edge2CellIds(cmp.store()), pathFinder::CellIdDiskWriter(*cellIdStore),
        cmp.store());
  }

  if(oscarFilePath.empty())
    cellIdStore = nullptr;


  // construct hub labels
  auto hlStore =  std::make_shared<pathFinder::HubLabelStore>(chGraph->getNumberOfNodes());
  if(level > -1) {
    pathFinder::HubLabelCreator hubLabelCreator(chGraph, hlStore);
    hubLabelCreator.create(level);
  }
  pathFinder::FileWriter::writeAll(chGraph, hlStore, cellIdStore, outPutPath);

  return 0;
}
