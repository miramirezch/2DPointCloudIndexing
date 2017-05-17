// Miguel Ramirez Chacon
// Example for 2D Point Cloud Indexing
// Index based in RTree
// 16/05/17

//Includes
#include "Cloud.h"
#include "RTreePC.h"
#include <iostream>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/index/parameters.hpp>

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

	//KNN Query
	//First Parameter: Query =  PointCloud
	//Second Parameter: K = Nearest Neighbors
	auto resultKNN = rtree.KNN(cloud,4);

	//Print Results of KNN Query
	for (auto r : resultKNN)
	{
		std::cout << "Cloud: " << r.first << " - Matches: " << r.second << '\n';
	}

	//Intersection Query
	//1st Parameter: Query = PointCloud
	//2nd Parameter: Intersection Window (Box centered in  point)
	//3rd Parameter: Epsilon = Retrieve all Point Clouds with support > epsilon
	auto resultIntersection = rtree.Intersection(cloud, 1, 0.5);

	//Print Results
	for (auto r : resultIntersection)
	{
		std::cout << "Cloud: " << r.first << " - Support: " << r.second << '\n';
	}
	return 0;
}