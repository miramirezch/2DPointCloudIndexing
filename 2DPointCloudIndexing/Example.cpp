#include "Cloud.h"
#include "RTree.h"
#include "IGI.h"
#include "VPT.h"
#include "IGIRtree.h"
#include "ShazamHash.h"
#include "GetCloudsCSV.h"
#include <iostream>
#include <cmath>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/index/parameters.hpp>

// Miguel Ramirez Chacon
// Example for 2D Point Cloud Indexing

// Namespaces
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

// Type Alias
using Point = bg::model::point<float, 2, boost::geometry::cs::cartesian>;
using PointIdx = std::pair<Point, unsigned>;

// Metric function for Vantage Point Tree Index
double DistL2(const PointIdx& p1, const PointIdx& p2)
{
	auto dx = std::pow(boost::geometry::get<0>(p1.first) - boost::geometry::get<0>(p2.first), 2);
	auto dy = std::pow(boost::geometry::get<1>(p1.first) - boost::geometry::get<1>(p2.first), 2);

	return std::sqrt(dx + dy);
}

int main()
{
	//---------------------------------------------------------------------------
	// Getting Started
	// Examples for Generating a PointCloud and creating a Index on this data	

	// PointCloud (ID = 100)
	Cloud<Point> cloud(100);

	// Add some points to the PointCloud 
	cloud.Add(Point(1, 1)).Add(Point(2, 2)).Add(Point(3, 3)).Add(Point(4, 4));
	
	// Vector of PointClouds
	std::vector<Cloud<Point>> clouds{ cloud };

	// Rtree for PointClouds -Declaration and construction
	// 1st Parameter - Vector of PointClouds
	Rtree<Point> rtree("Rtree");	
	rtree.Build(clouds);
	
	// Inverted Grid Index for PointClouds -Declaration and construction
	// 1st Parameter - Vector of PointClouds
	// 2nd Parameter - cmax: Upper bound of valid coordinate for the two axis
	// 3rd Parameter - delta: Dimension of uniform cell
	IGI<Point> igi(clouds,"IGI",10000,10);

	// ShazamHash for PointClouds -Declaration and construction
	// Parameter for ShazamHash
	// 1st Parameter - delta: Dimension of uniform cell
	// 2nd Parameter - DelayX: Delay for Target Zone in X axis
	// 3rd Parameter - DeltaX: Dimension for Target Zone in X axis
	// 4th Parameter - DeltaY: Dimension for Target Zone in Y Axis
	// 5th Parameter - CombinationLimit: Maximum number of tuples for an anchor point and a Target Zone
	ShazamHashParameters param(1, 0, 100, 100, 1);
	ShazamHash<Point> shazam(clouds,"Shazam",param);	

	// Vantage Point Tree for PointClouds -Declaration and construction	
	VPT<Point, DistL2> vpt("VPT");
	// 1st Parameter - Vector of PointClouds
	vpt.Build(clouds);

	// IGIRtree for PointClouds -Declaration and construction	
	// 1st Parameter - Vector of PointClouds
	// 2nd Parameter  - Name of Index
	// 3rd Parameter - cmax: Upper bound of valid coordinate for the two axis
	// 4th Parameter - delta: Dimension of uniform cell
	IGIRtree<Point> igiRtree(clouds, "IGIRtree", 10000, 10);

	//---------------------------------------------------------------------------
	// Query Example

	// Rtree - KNN Query
	// 1st Parameter: Query =  PointCloud
	// 2nd Parameter: K = K Nearest Neighbors PointClouds
	// 3rd Parameter: internalK = internalK-NN queries per point in PointCloud
	auto resultKNN = rtree.KNN(cloud,1,1);

	std::cout << "Query Example - Rtree Section" << '\n';

	// Print Results for KNN Query
	for (auto r : resultKNN)
	{
		std::cout << "Cloud: " << r.first << " - Matches: " << r.second << '\n';
	}

	// Intersection Query
	// 1st Parameter: Query = PointCloud
	// 2nd Parameter: Dimension for Intersection Window (Box centered in  point)
	// 3rd Parameter: Epsilon = Retrieve all PointClouds with support > epsilon
	auto resultIntersection = rtree.Intersection(cloud, 1, 0.5);

	// Print Results
	for (auto r : resultIntersection)
	{
		std::cout << "Cloud: " << r.first << " - Support: " << r.second << '\n';
	}

	std::cout << "--------------------------------------------------" << '\n';

	//---------------------------------------------------------------------------
	// Obtain PointClouds from CSV File
	// Test for ReadCSV Function - Get PointClouds from File

	// Expected schema
	// Example for file with 2 PointClouds
	// PointCloud 0: 2 2D Points
	// PointCloud 1: 2 3D Poins
	//
	// File.csv
	// ID, X, Y
	// 0, 1238.323, 1234.75
	// 0, 495.234, 423.734
	// 1, 432.734, 756.856
	// 1, 7564.234, 543.789
	// 1, 2342.7345, 7546.231

	std::cout << "Loading PointClouds from CSV File" << '\n';
	
	// FileName - Fullpath to CSV File
	std::string queriesFileName = "C:\\Succinct Index\\nubes_noise1k.csv";	
	std::string indexingFileName = "C:\\Succinct Index\\nubes_1k.csv";
	
	// Loading PointClouds from CSV Fiels
	auto cloudsQuery = ReadCSV<Point>(queriesFileName, 0, 10000, true);	
	auto cloudsIndexing = ReadCSV<Point>(indexingFileName, 0, 10000, true);

	std::cout << "--------------------------------------------------" << '\n';

	//---------------------------------------------------------------------------
	// Performance Comparation
	// Data: Indexing 1K PointClouds with ~1000 Point/PointCloud
	// Query: Same PointClouds from Indexing with ~10% of Noise (Insertions/Deletions)
	
	std::cout << "Performance Comparation Section" << '\n';

	std::cout << "Building Indexes" << '\n';
	// Building Indexes on same data

	// Rtree
	Rtree<Point> rtree2("Rtree");
	rtree2.Build(cloudsIndexing);

	// Inverted Grid Index
	IGI<Point> igi2(cloudsIndexing, "IGI",10000, 10);

	// ShazamHash
	ShazamHashParameters param2(1, 0, 500, 500, 10);
	ShazamHash<Point> shazam2(cloudsIndexing, "Shazam",param2);

	// VPT
	VPT<Point, DistL2> vpt2("Vantage Point Tree");
	vpt2.Build(cloudsIndexing);

	// IGIRtree
	IGIRtree<Point> igiRtree2(cloudsIndexing, "IGIRtree", 10000, 10);

	// Define wanted recall: In this case, Recall@1, Recall@5, Recall@10 
	std::vector<unsigned> recall{1,5,10};

	// Performance Test	
	std::cout << "Starting Queries" << std::endl;
	auto reportRtree = rtree2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 1, 1, recall);	
	auto reportIGI = igi2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 1, recall);
	auto reportShazam = shazam2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 1, recall, param2);
	auto reportVPT = vpt2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 1, recall);
	auto reportIGIRtree = igiRtree2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 1, 1, recall);

	// Print Performance Report
	PrintPerformanceReport(reportRtree, rtree2.GetName() ,"us");
	PrintPerformanceReport(reportIGI,igi2.GetName(),"us");
	PrintPerformanceReport(reportShazam,shazam2.GetName(), "us");
	PrintPerformanceReport(reportVPT, vpt2.GetName(), "us");
	PrintPerformanceReport(reportIGIRtree, igiRtree2.GetName(), "us");

	getchar();

	return 0;
}