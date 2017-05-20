#include "Cloud.h"
#include "RTreePC.h"
#include "IGIPC.h"
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
	
	std::string queriesFileName = "C:\\Succinct Index\\nubes_noise1k.csv";	
	std::string indexingFileName = "C:\\Succinct Index\\nubes_1k.csv";

	std::cout << "Inicia lectura de archivo de consulta" << std::endl;
	auto cloudsQuery = ReadCSV<Point>(queriesFileName, 0, 10000, true);
	std::cout << "Inicia lectura de archivo de indexado" << std::endl;
	auto cloudsIndexing = ReadCSV<Point>(indexingFileName, 0, 10000, true);

	// Declaration of Rtree for PointCloud
	RtreePC<Point> rtree2;

	// Index Construction
	std::cout << "Inicia construccion de indice" << std::endl;
	rtree2.Build(cloudsIndexing);

	// Define wanted Recall@ 
	std::vector<unsigned> recall{1,5,10};

	// Test for function KNNPerformanceReport
	std::cout << "Inicia seccion de consulta" << std::endl;
	auto report = rtree2.KNNPerformanceReport<std::chrono::milliseconds>(cloudsQuery, 20, 1, recall);

	std::cout << "Average Query Time: " << report.AverageQueryTime << '\n';

	for (auto pair : report.RecallAt)
	{
		std::cout << "Recall@" << pair.first << " :" << pair.second << '\n';
	}

	getchar();

	return 0;
}