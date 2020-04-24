#include "vendor/path_finder/include/CHGraph.h"
#include "vendor/path_finder/include/ChHlBenchmarker.h"
#include "vendor/path_finder/include/GraphReader.h"
#include <iostream>
int main(int argc, char* argv[]) {
  std::string filepath;
  int level = 0;

  for(int i = 1; i < argc; ++i) {
    std::string option = argv[i];
    if(option == "-f")
      filepath = argv[++i];
    else if(option == "-l")
      level = std::stoi(argv[++i]);
  }

  std::cout << filepath << std::endl;
  pathFinder::CHGraph chGraph;
  pathFinder::GraphReader::readCHFmiFile(chGraph, filepath);
  pathFinder::ChHlBenchmarker bm(chGraph);
  bm.compareSpeed("hl-ram.bench", level);
  return 0;
}