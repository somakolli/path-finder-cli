#### This is a command line interface to create routing files for https://github.com/somakolli/path_finder.

### Building:

```
git clone --recursive https://github.com/somakolli/path-finder-cli
cd path-finder-cli
mkdir build && cd build
CMAKE_GCC_VERSION_FOR_LTO=9 cmake ../ -DCMAKE_BUILD_TYPE=ultra
make
```
If you use gcc-10 you have to set your GCC_VERSION_FOR_LTO to 10.
### Generating Files:

The executable to create the files is called file_creator.
You can print the usage instructions with `./file_creator -h`.

The output will look like this:

```
routing-file-creator
Usage: ./file_creator [OPTIONS]

Options:
  -h,--help                   Print this help message and exit
  -f,--graph TEXT REQUIRED    Path to a graph file in ch-fmi format.
  -s,--oscarFilePath TEXT     Path to oscar data for cell id store generation.
  -l,--level INT              CH-Level at which the hub label generation will break.
  -o,--out TEXT REQUIRED      Output path.
 ```
Example :

```
./file_creator -f /path/to/germany.chfmi -s /path/to/germany-search-files/ -l 10 -o routing-data
```

The lower you set the level the faster the query will be, but it will also need more memory.
For larger graphs it is usually not feasible to compute until level 0 as it requires hundreds 
or thousands of gigabytes of ram.
Because of that I recommend to set the level to 10.

The integration with oscar works by calculating the cell ids for each edge.
But keep in mind that the calculation for graphs with many edges can take a long time (i.e. 
europe 1 billion edges 2-3 days).

If you just want to generate routing files, you can just omit the oscarFilePath (-s).

### Benchmarking

Note: You have to run the benchmarking tool with sudo to allow caches to be dropped. 

A benchmarking tool called DistanceBenchmark is located in the benchmark folder.
Run it with ```benchmarks/DistanceBenchmark -h```.

You have the choice between hybrid(-a), chDijkstra(-d) and interactive(-i).
If you chose hybrid or chDijkstra you also have to supply a path to an out put file where the benchmarking result will 
be written in octave syntax.

If you chose interactive, the cli will prompt you to input a level for which the benchmark will be executed.
The result will be presented on screen.

Example: 
```
sudo benchmarks/DistanceBenchmark -f <path/to/previously/generated/files> -n 100 -a -o out
```
This will generate an out file with the benchmarking data of the hybrid approach.

### Needed data:
For the programm to work a graph in ch-fmi format is required.
To generate a graph in ch-fmi format you first need to download the osm-data in osm-pbf format.
Then you have to use https://github.com/fmi-alg/OsmGraphCreator to create a graph in fmi format.
To then construct the contraction hierarchies and get a file in ch-fmi format use https://github.com/chaot4/ch_constructor.

In the root folder of this repository you can find a script called osmpbf-to-chfmi.sh which makes the generation more comfortable.
You need to put it in the same directory where your ch_constructor and OsmGraphCreator are.
Then call it like this:

```
./osmpbf-to-chfmi.sh /path/to/germany.osmpbf /path/to/intermediate.fmi /path/to/final.chfmi
```

The intermediate file is the output of OsmGraphCreator and the final file is the output of the ch_constructor.

If you want to also generate the needed data to integrate the path finder with oscar you need oscar search files which are generated with https://github.com/dbahrdt/oscar.
