# 2DPointCloudIndexing

## Why 2D Point Cloud Indexing?
In recent years there has been a significant increase in the size of multimedia databases, and new databases of large size are created continuously. It is, therefore, of utmost importance the design of indexes and algorithms allowing efficient similarity queries. 

A large number of multimedia objects can be represented as a point cloud:
* Images
* Audio
* Information of geographic systems
* Points and polygons in CAD systems
* Time series

We propose data structures based on two solution schemes: indexes for point clouds based on inverted indexes and based on metric indexes. 

## Current Indexes
* R*-Tree for PointClouds
* Inverted Grid Index for PointClouds
* Inverted Grid Index for PointClouds - Apache Spark (Pyspark - Spark SQL)
* ShazamHash: PointCloud index based on the paper: An Industrial-Strength Audio Search Algorithm - Wang 2003
* Vantage Point Tree for PointClouds
* Inverted Grid Index + R*-Tree for PointClouds
* Inverted Grid Index + Vantage Point Tree for PointClouds
* Succinct Inverted Grid Index for PointClouds
* Succinct Sarray + Vantage Point Tree for PointClouds

## Requirements
* A C++11/C++14 compiler such as g++ (4.7 or higher), Visual Studio Community 2017 C++ Compiler.
* Tested on Linux and Windows(64 bits).
* Boost Library (1.61 or higher)

## Simple Demo
Check out 2DPointCloudIndexing/Example.cpp

## Recomended libraries for similarity search

Metric Space

Library | URL | Language 
--- | --- | ---
SISAP | http://sisap.org/metricspaceslibrary.html | C, Java
Natix | https://github.com/sadit/natix | C# (.NET Framework)
Vantage Point Tree | https://github.com/gregorburger/vp-tree | C++

Vector Space

Library | URL | Language 
--- | --- | --- 
Boost RTree | http://www.boost.org/ | C++
annoy | https://github.com/spotify/annoy | C++
FLANN | https://github.com/mariusmuja/flann | C++
nanoflann | https://github.com/jlblancoc/nanoflann | C++
FALCONN | https://github.com/FALCONN-LIB/FALCONN | C++

Succinct Data Structures

Library | URL | Language 
--- | --- | --- 
SDSL 2.0 | https://github.com/simongog/sdsl-lite | C++
