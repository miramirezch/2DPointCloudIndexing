#include "Cloud.h"
#include "RTreePC.h"
#include "GetCloudsCSV.h"
#include <iostream>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/index/parameters.hpp>

// Miguel Ramirez Chacon
// Example for 2D Point Cloud Indexing
// Index based in RTree
// 16/05/17

// Namespaces
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

// Type Alias
using Point = bg::model::point<float, 2, boost::geometry::cs::cartesian>;


int main()
{
	// PointCloud (ID = 100)
	Cloud<Point> cloud(100);

	// Add some points to the PointCloud 
	cloud.Add(Point(1, 1)).Add(Point(2, 2)).Add(Point(3, 3)).Add(Point(4, 4));
	
	// Vector of PointClouds
	std::vector<Cloud<Point>> clouds{ cloud };

	// Declaration of Rtree for PointCloud
	RtreePC<Point> rtree;

	// Index Construction
	rtree.Build(clouds);

	// KNN Query
	// First Parameter: Query =  PointCloud
	// Second Parameter: K = Nearest Neighbors
	auto resultKNN = rtree.KNN(cloud,20,1);

	// Print Results of KNN Query
	for (auto r : resultKNN)
	{
		std::cout << "Cloud: " << r.first << " - Matches: " << r.second << '\n';
	}

	// Intersection Query
	// 1st Parameter: Query = PointCloud
	// 2nd Parameter: Intersection Window (Box centered in  point)
	// 3rd Parameter: Epsilon = Retrieve all Point Clouds with support > epsilon
	auto resultIntersection = rtree.Intersection(cloud, 1, 0.5);

	// Print Results
	for (auto r : resultIntersection)
	{
		std::cout << "Cloud: " << r.first << " - Support: " << r.second << '\n';
	}

	// Test for ReadCSV Function - Get PointClouds from File
	std::string queriesFileName = "C:\\Users\\Miguel\\Source\\Repos\\2DPointCloudIndexing\\2DPointCloudIndexing\\QueryClouds.csv";
	std::string indexingFileName = "C:\\Users\\Miguel\\Source\\Repos\\2DPointCloudIndexing\\2DPointCloudIndexing\\IndexingClouds.csv";
	auto cloudsQuery = ReadCSV<Point>(queriesFileName, 0, 10000, false);
	auto cloudsIndexing = ReadCSV<Point>(indexingFileName, 0, 10000, false);

	// Declaration of Rtree for PointCloud
	RtreePC<Point> rtree2;

	// Index Construction
	rtree2.Build(cloudsIndexing);

	// Define wanted Recall@ 
	std::vector<unsigned> recall{ 1,2 };

	// Test for function KNNPerformanceReport
	auto report = rtree2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 3, 1, recall);

	return 0;
}