//
// Created by sokol on 11.08.20.
//
#include <fstream>
#include <iostream>
#include <path_finder/helper/Benchmarker.h>
#include "CLI11.hpp"
auto main(int argc, char* argv[]) -> int {
  CLI::App app("routing-benchmarker");

  std::string routingDataPath;
  app.add_option("-f, --routingData", routingDataPath,
                 "path to routing data")->required();
  int numberOfQueries;
  app.add_option("-n, --numberOfQueries", numberOfQueries,
                 "number of queries run per level")->required();
  std::string outputPath;
  app.add_option("-o, --outPutPathForOctave", outputPath,
                 "output file; the result will be printed in octave syntax");
  bool hybrid = false;
  app.add_flag("-a, --hybrid", hybrid, "benchmark all level and print the result to outPutPath");
  bool chDijkstra = false;
  app.add_flag("-d, --chDijkstra", chDijkstra, "benchmark only dijkstra and print the result to output");
  bool interactive = false;
  app.add_flag("-i, --interactive", interactive, "do an interactive benchmark");
  CLI11_PARSE(app, argc, argv);

  if(!hybrid && !chDijkstra && !interactive) {
    std::cout << "please add either the chDijkstra, the hybrid or the interactive flag" << '\n';
  }

  if(hybrid && outputPath.empty() || chDijkstra && outputPath.empty()) {
    std::cout << "please add an output path" << '\n';
    return 0;
  }

  if(interactive) {
    std::cout << "loading files: " << routingDataPath << '\n';
    auto hubLabels = pathFinder::FileLoader::loadHubLabelsShared(routingDataPath);
    while(true) {
      auto dijkstra = pathFinder::FileLoader::loadCHDijkstraShared(routingDataPath);
      std::cout << "enter level: " << '\n';
      std::string in;
      std::cin >> in;
      pathFinder::Level level = std::stoi(in);
      pathFinder::Benchmarker::benchmarkLevel(*hubLabels, level, numberOfQueries);
      auto additionalSpace = hubLabels->m_spaceMeasurer.getSpaceConsumption(level);
      std::cout << "additional Space Required: " << pathFinder::Static::humanReadable(additionalSpace) << '\n';
      pathFinder::Benchmarker::benchmarkCHDijkstra(*dijkstra, numberOfQueries);
    }
  } else if(chDijkstra) {
    std::cout << "loading files: " << routingDataPath << '\n';
    auto chDijkstraData = pathFinder::FileLoader::loadCHDijkstraShared(routingDataPath);
    pathFinder::Benchmarker::benchmarkCHDijkstra(*chDijkstraData, numberOfQueries);
  } else if(hybrid) {
    std::cout << "loading files: " << routingDataPath << '\n';
    auto hubLabels = pathFinder::FileLoader::loadHubLabelsShared(routingDataPath);
    auto result = pathFinder::Benchmarker::benchmarkAllLevel(*hubLabels, numberOfQueries);
    std::ofstream plotOutputFile;
    plotOutputFile.open (outputPath);
    pathFinder::Benchmarker::printRoutingResultForOctave(plotOutputFile, result);
    plotOutputFile.close();
  }

  return 0;
}