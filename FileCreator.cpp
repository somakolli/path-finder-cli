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
#include "CLI11.hpp"

int main(int argc, char *argv[]) {
  CLI::App app("routing-file-creator");

  std::string graphFilePath;
  app.add_option("-f, --graph", graphFilePath, "Path to a graph file in ch-fmi format.")->required();

  std::string oscarFilePath;
  app.add_option("-s, --oscarFilePath", oscarFilePath, "Path to oscar data for cell id store generation.");

  int level = -1;
  app.add_option("-l, --level",level, "CH-Level at which the hub label generation will break.");

  std::string outPutPath;
  app.add_option("-o, --out", outPutPath, "Output path.")->required();

  bool gridReorder = true;

  CLI11_PARSE(app, argc, argv);

  std::cout << graphFilePath << '\n';
  liboscar::Static::OsmCompleter cmp;
  // setup stores
  auto chGraph = std::make_shared<pathFinder::CHGraph>();

  pathFinder::GraphReader::readCHFmiFile(chGraph, graphFilePath, gridReorder);

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
        *chGraph, liboscar::routing::support::Edge2CellIds(cmp.store()), *cellIdStore,
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
