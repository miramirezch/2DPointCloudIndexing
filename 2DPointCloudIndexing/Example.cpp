#include "Cloud.h"
#include "Rtree.h"
#include "IGI.h"
#include "VPT.h"
#include "IGIRtree.h"
#include "IGIVpt.h"
#include "BKT.h"
#include "ShazamHash.h"
#include "SuccinctIGI.h"
#include "GetCloudsCSV.h"
#include "SarrayVPT.h"
#include "SarrayMetrics.h"
#include <iostream>
#include <cmath>
#include <boost/functional/hash.hpp>
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
auto DistL2 = [](const PointIdx& p1, const PointIdx& p2)
{
	auto dx = std::pow(boost::geometry::get<0>(p1.first) - boost::geometry::get<0>(p2.first), 2);
	auto dy = std::pow(boost::geometry::get<1>(p1.first) - boost::geometry::get<1>(p2.first), 2);

	return static_cast<double>(std::sqrt(dx + dy));
};

// Metric function for Vantage Point Tree Index
auto DiscreteDistL2 = [](const PointIdx& p1, const PointIdx& p2)
{
	auto dx = std::pow(boost::geometry::get<0>(p1.first) - boost::geometry::get<0>(p2.first), 2);
	auto dy = std::pow(boost::geometry::get<1>(p1.first) - boost::geometry::get<1>(p2.first), 2);

	return static_cast<unsigned>(std::floor(std::sqrt(dx + dy)));
};

int main()
{
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
	std::string indexingFileName = "nubes_1k.csv";
	std::string queriesFileName = "nubes_noise1k.csv";

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
	IGI<Point> igi2(cloudsIndexing, "IGI", 10000, 10);

	// ShazamHash
	ShazamHashParameters param2(1, 0, 500, 500, 10);
	ShazamHash<Point> shazam2(cloudsIndexing, "Shazam", param2);

	// VPT
	VPT<Point> vpt2("VPT");
	vpt2.Build(cloudsIndexing, DistL2);

	// BKT
	VPT<Point> bkt2("BKT");
	bkt2.Build(cloudsIndexing, DiscreteDistL2);

	// IGIRtree
	IGIRtree<Point> igiRtree2(cloudsIndexing, "IGIRtree", 10000, 10);

	// IGIVpt
	IGIVpt<Point> igiVPT2(cloudsIndexing, DistL2, "IGIVpt", 10000, 10);

	// SuccinctIGI
	SuccinctIGI<Point> sIGI2(cloudsIndexing, "SuccinctIGI", 10000, 10);

	// SarrayVPT
	SarrayVPT<Point, HammingDistance> sarrayVPT2("SarrayVPT", 10000, 10);
	sarrayVPT2.Build(cloudsIndexing);

	// Define wanted recall: In this case, Recall@1, Recall@5, Recall@10 
	std::vector<unsigned> recall{ 1,10,30 };
	std::cout << "Starting Queries" << std::endl;

	// Performance Test	
	auto reportRtree = rtree2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 1, 1, recall);
	auto reportIGI = igi2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 1, recall);
	auto reportShazam = shazam2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 1, recall, param2);
	auto reportVPT = vpt2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 1, 1, recall);
	auto reportBKT = bkt2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 1, 1, recall);
	auto reportIGIVpt = igiVPT2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 1, 1, recall);
	auto reportIGIRtree = igiRtree2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 1, 1, recall);
	auto reportSuccinctIGI = sIGI2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 1, recall);
	auto reportSarrayVPT = sarrayVPT2.KNNPerformanceReport<std::chrono::microseconds>(cloudsQuery, 1, 1, recall);


	// Print Performance Report
	PrintPerformanceReport(reportRtree, rtree2.GetName(), "us");
	PrintPerformanceReport(reportIGI, igi2.GetName(), "us");
	PrintPerformanceReport(reportShazam, shazam2.GetName(), "us");
	PrintPerformanceReport(reportVPT, vpt2.GetName(), "us");
	PrintPerformanceReport(reportBKT, bkt2.GetName(), "us");
	PrintPerformanceReport(reportIGIRtree, igiRtree2.GetName(), "us");
	PrintPerformanceReport(reportIGIVpt, igiVPT2.GetName(), "us");
	PrintPerformanceReport(reportSuccinctIGI, sIGI2.GetName(), "us");
	PrintPerformanceReport(reportSarrayVPT, sarrayVPT2.GetName(), "us");

	getchar();

	return 0;
}


