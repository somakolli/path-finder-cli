#include "path_finder/CHGraph.h"
#include "path_finder/ChHlBenchmarker.h"
#include "path_finder/GraphReader.h"
#include <iostream>
int main(int argc, char* argv[]) {
  std::string filepath;
  int level = 0;
  bool gridReorder = false;
  bool ram = false;
  for(int i = 1; i < argc; ++i) {
    std::string option = argv[i];
    if(option == "-f")
      filepath = argv[++i];
    else if(option == "-l")
      level = std::stoi(argv[++i]);
    else if(option == "-gridReorder")
      gridReorder = true;
    else if(option == "-ram")
      ram = true;
  }

  std::cout << filepath << std::endl;
  pathFinder::CHGraph chGraph;
  pathFinder::GraphReader::readCHFmiFile(chGraph, filepath, gridReorder);
  pathFinder::ChHlBenchmarker bm(chGraph);
  bm.compareSpeed("hl-ram.bench", level, ram);
  return 0;
}