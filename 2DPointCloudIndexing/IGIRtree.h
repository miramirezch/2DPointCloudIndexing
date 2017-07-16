#pragma once
#include "Cloud.h"
#include "PerformanceReport.h"
#include "UtilityFunctions.h"

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/index/parameters.hpp>

#include <unordered_map>
#include <vector>
#include <chrono>
#include <string>

// Miguel Ramirez Chacon
// 22/05/17

// Index for Point Clouds based in combination of Inverted Grid Index and RTree
// T: Point class(2D)
// Param: Rtree configuration

template<typename T = boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian>, typename Param = boost::geometry::index::rstar<20, 10>>
class IGIRtree
{
	using PointIdx = std::pair<T, int>;

private:

	std::unordered_map<int, boost::geometry::index::rtree<PointIdx, Param>> igiRtree;
	std::unordered_map<int, int> sizeClouds;
	const std::string name_;
	const int cmax_;
	const int delta_;

public:

	// Build index from vector of PointClouds
	IGIRtree(const std::vector<Cloud<T>>& pointClouds, const std::string name, const int cmax, const int delta) :name_{ name }, cmax_{ cmax }, delta_{ delta }
	{
		std::unordered_map<int, std::vector<PointIdx>> pointsWithinCell;
		PointsWithinCell(pointClouds, pointsWithinCell);

		for (const auto& pair : pointsWithinCell)
		{
			// Create a Rtree for every cell
			boost::geometry::index::rtree<PointIdx, Param> tempRtree(std::begin(pair.second), std::end(pair.second));
			igiRtree.insert({ pair.first, boost::move(tempRtree) });
		}
	}

	std::string GetName() const
	{
		return name_;
	}

	// Calculate cell for every point in pointClouds
	void PointsWithinCell(const std::vector<Cloud<T>>& pointClouds, std::unordered_map<int, std::vector<PointIdx>>& pointsWithinCell)
	{
		int totalPoints = 0;
		int px, py, cell;

		// Prepare data to Indexing - Add the Cloud ID to every Point
		for (const auto& cloud : pointClouds)
		{
			sizeClouds[cloud.ID] = cloud.Points.size();

			for (const auto& p : cloud.Points)
			{
				// Calculate cell of point
				px = static_cast<int>(std::floor(boost::geometry::get<0>(p) / delta_));
				py = static_cast<int>(std::floor(boost::geometry::get<1>(p) / delta_));
				cell = px + static_cast<int>(cmax_ / delta_)*py;

				pointsWithinCell[cell].emplace_back(std::make_pair(p, cloud.ID));
			}
		}
	}

	// KNN Query
	// 1st Parameter: Query =  PointCloud
	// 2nd Parameter: K = K Nearest Neighbors PointClouds
	// 3rd Parameter: internalK = internalK-NN queries per point in PointCloud
	std::vector<std::pair<int, int>> KNN(const Cloud<T>& queryCloud, const int k, const int internalK) const
	{
		std::unordered_map<int, int> count;
		std::vector<PointIdx> results;

		int px, py, cell;

		// For every point in the PointCloud 
		for (const auto& point : queryCloud.Points)
		{
			px = static_cast<int>(std::floor(boost::geometry::get<0>(point) / delta_));
			py = static_cast<int>(std::floor(boost::geometry::get<1>(point) / delta_));

			cell = px + static_cast<int>(cmax_ / delta_)*py;

			auto it = igiRtree.find(cell);

			// Get List from Inverted Index and count frequency of ID's
			if (it != std::end(igiRtree))
			{				
				(it->second).query(boost::geometry::index::nearest(point, internalK), std::back_inserter(results));
			}
		}

		// Count the frequencies for the Clouds ID
		for (const auto& item : results)
		{
			count[item.second]++;
		}

		auto numberResults = 0;

		if (count.size() > k)
			numberResults = k;
		else
			numberResults = count.size();


		std::vector<std::pair<int, int>> resultsID(numberResults);

		// Get only the first K Point Clouds based in ID frequency.
		std::partial_sort_copy(std::begin(count), std::end(count), std::begin(resultsID), std::end(resultsID),
			[](const std::pair<int, int>& left, const std::pair<int, int>& right) {return left.second > right.second; });

		return resultsID;
	}


	// Performance report on KNN Search
	// Obtain Recall@
	// Average query time, Standard deviation query time, max query time and min query time
	// 1st Parameter: Vector of Queries Point Clouds
	// 2nd Parameter: k = Nearest Neighbors
	// 3rd Parameter: internalK = Internal k parameter for internalK-NN 
	// 4th Parameter: recallAt = Vector for desired Recall@
	template<typename D = std::chrono::milliseconds>PerformanceReport KNNPerformanceReport(const std::vector<Cloud<T>>& queryClouds, const int k, const int internalK, const std::vector<int>& recallAt) const
	{
		PerformanceReport performance;
		performance.QueriesTime.reserve(queryClouds.size());
		performance.RecallAt.reserve(queryClouds.size());
		std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

		// For every cloud in vector of PointClouds
		for (const auto& cloud : queryClouds)
		{
			// Perform KNN search
			start = std::chrono::high_resolution_clock::now();
			auto result = KNN(cloud, k, internalK);
			end = std::chrono::high_resolution_clock::now();

			GetRecall(performance, result, recallAt, cloud.ID);

			performance.QueriesTime.push_back(std::chrono::duration_cast<D>(end - start).count());
		}
		// Calculate Recall@
		for (auto& pair : performance.RecallAt)
		{
			pair.second = pair.second / static_cast<double>(queryClouds.size());
		}

		TimePerformance(performance);

		return performance;
	}



};


/*
#pragma once
#include "Cloud.h"
#include "PerformanceReport.h"
#include "UtilityFunctions.h"
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/index/parameters.hpp>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <string>

// Miguel Ramirez Chacon
// 22/05/17

// Index for Point Clouds based in combination of Inverted Grid Index and RTree
// T: Point class(2D)
// Param: Rtree configuration

template<typename T = boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian>, typename Param = boost::geometry::index::rstar<20, 10>>
class IGIRtree
{
using PointIdx = std::pair<T, unsigned>;

private:

std::unordered_map<unsigned, boost::geometry::index::rtree<PointIdx, Param>> igiRtree;
std::unordered_map<unsigned, unsigned> sizeClouds;
std::string name_;
const unsigned cmax_;
const unsigned delta_;

public:

// Build index from vector of PointClouds
IGIRtree(const std::vector<Cloud<T>>& pointClouds, std::string name, const unsigned cmax, const unsigned delta) :name_{ name }, cmax_{ cmax }, delta_{ delta }
{
std::unordered_map<unsigned, std::vector<PointIdx>> pointsWithinCell;
PointsWithinCell(pointClouds, pointsWithinCell);

for (const auto& pair : pointsWithinCell)
{
// Create a Rtree for every cell
boost::geometry::index::rtree<PointIdx, Param> tempRtree(std::begin(pair.second), std::end(pair.second));
igiRtree.insert({ pair.first, boost::move(tempRtree) });
}
}

std::string GetName()
{
return name_;
}

// Calculate cell for every point in pointClouds
void PointsWithinCell(const std::vector<Cloud<T>>& pointClouds, std::unordered_map<unsigned, std::vector<PointIdx>>& pointsWithinCell)
{
int totalPoints = 0;
unsigned px, py, cell;

// Prepare data to Indexing - Add the Cloud ID to every Point
for (const auto& cloud : pointClouds)
{
sizeClouds[cloud.ID] = cloud.Points.size();

for (const auto& p : cloud.Points)
{
// Calculate cell of point
px = static_cast<unsigned>(std::floor(boost::geometry::get<0>(p) / delta_));
py = static_cast<unsigned>(std::floor(boost::geometry::get<1>(p) / delta_));
cell = px + static_cast<unsigned>(cmax_ / delta_)*py;

pointsWithinCell[cell].push_back(std::make_pair(p, cloud.ID));
}
}
}

// KNN Query
// 1st Parameter: Query =  PointCloud
// 2nd Parameter: K = K Nearest Neighbors PointClouds
// 3rd Parameter: internalK = internalK-NN queries per point in PointCloud
std::vector<std::pair<unsigned, unsigned>> KNN(const Cloud<T>& queryCloud, const unsigned k, const unsigned internalK) const
{
std::unordered_map<unsigned, unsigned> count;
std::vector<PointIdx> results;

unsigned px, py, cell;

// For every point in the PointCloud
for (const auto& point : queryCloud.Points)
{
px = static_cast<unsigned>(std::floor(boost::geometry::get<0>(point) / delta_));
py = static_cast<unsigned>(std::floor(boost::geometry::get<1>(point) / delta_));

cell = px + static_cast<unsigned>(cmax_ / delta_)*py;

auto it = igiRtree.find(cell);

// Get List from Inverted Index and count frequency of ID's
if (it != std::end(igiRtree))
{
(it->second).query(boost::geometry::index::nearest(point, internalK), std::back_inserter(results));
}
}

// Count the frequencies for the Clouds ID
for (const auto& item : results)
{
count[item.second]++;
}

auto numberResults = 0;

if (count.size() > k)
numberResults = k;
else
numberResults = count.size();


std::vector<std::pair<unsigned, unsigned>> resultsID(numberResults);

// Get only the first K Point Clouds based in ID frequency.
std::partial_sort_copy(std::begin(count), std::end(count), std::begin(resultsID), std::end(resultsID),
[](const std::pair<unsigned, unsigned>& left, const std::pair<unsigned, unsigned>& right) {return left.second > right.second; });

return resultsID;
}


// Performance report on KNN Search
// Obtain Recall@
// Average query time, Standard deviation query time, max query time and min query time
// 1st Parameter: Vector of Queries Point Clouds
// 2nd Parameter: k = Nearest Neighbors
// 3rd Parameter: internalK = Internal k parameter for internalK-NN
// 4th Parameter: recallAt = Vector for desired Recall@
template<typename D = std::chrono::milliseconds>PerformanceReport KNNPerformanceReport(const std::vector<Cloud<T>>& queryClouds, const unsigned k, const unsigned internalK, const std::vector<unsigned>& recallAt) const
{
PerformanceReport performance;
performance.QueriesTime.reserve(queryClouds.size());
performance.RecallAt.reserve(queryClouds.size());
std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

// For every cloud in vector of PointClouds
for (const auto& cloud : queryClouds)
{
// Perform KNN search
start = std::chrono::high_resolution_clock::now();
auto result = KNN(cloud, k, internalK);
end = std::chrono::high_resolution_clock::now();

GetRecall(performance, result, recallAt, cloud.ID);

performance.QueriesTime.push_back(std::chrono::duration_cast<D>(end - start).count());
}
// Calculate Recall@
for (auto& pair : performance.RecallAt)
{
pair.second = pair.second / static_cast<double>(queryClouds.size());
}

TimePerformance(performance);

return performance;
}



};


*/
