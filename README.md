# 2DPointCloudIndexing

## Why 2D Point Cloud Indexing?
TODO

## Applications
TODO

## Current Indexes
* RTree for PointClouds
* Inverted Grid Index for PointClouds
* ShazamHash: PointCloud index based on the paper: An Industrial-Strength Audio Search Algorithm - Wang 2003

## Requirements
* A C++11/C++14 compiler such as g++ (4.7 or higher), Visual Studio C++ Compiler.
* Tested on Linux and Windows(64 bits).
* Boost Library (1.61 or higher)

## Simple Demo
Check out 2DPointCloudIndexing/Example.cpp

## Documentation
TODO

## Getting Started
TODO

## Benchmarks
TODO

## Licencing
TODO

## Recomended libraries

For Similarity Search - Metric Space

Library | URL | Language 
--- | --- | ---
SISAP | http://sisap.org/metricspaceslibrary.html | C, Java
Natix | https://github.com/sadit/natix | C# (.NET Framework)
Vantage Point Tree | https://github.com/gregorburger/vp-tree | C++

For Similarity Search - Vector Space

Library | URL | Language 
--- | --- | --- 
Boost RTree | http://www.boost.org/ | C++
annoy | https://github.com/spotify/annoy | C++
FLANN | https://github.com/mariusmuja/flann | C++
nanoflann | https://github.com/jlblancoc/nanoflann | C++
FALCONN | https://github.com/FALCONN-LIB/FALCONN | C++

For Succinct Data Structures

Library | URL | Language 
--- | --- | --- 
SDSL 2.0 | https://github.com/simongog/sdsl-lite | C++
