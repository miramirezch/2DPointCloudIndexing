#pragma once
#include "vp-tree.h"
#include "UtilityFunctions.h"
#include "PerformanceReport.h"
#include "Cloud.h"

#include <boost/geometry.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <chrono>

// Miguel Ramirez Chacon
// 22/05/17

// Index for Point Clouds based in combination of Inverted Grid Index and Vantage Point Tree
// T: Point class(2D)
// D: Distance Function (Metric)

template<typename T>
class IGIVpt
{
	using PointIdx = std::pair<T, int>;

private:	
	std::unordered_map<int, VpTree<PointIdx>> igiVPT;
	std::unordered_map<int, int> sizeClouds;
	const std::string name_;
	const int cmax_;
	const int delta_;

public:

	IGIVpt(std::vector<Cloud<T>>& pointClouds, std::function<double(const T&, const T&)> dist, const std::string name, const int cmax, const int delta) :name_{ name }, cmax_{ cmax }, delta_{ delta }
	{
		std::unordered_map<int, std::vector<PointIdx>> pointsWithinCell;
		PointsWithinCell(pointClouds, pointsWithinCell);

		auto distance = [d = dist](const PointIdx& e1, const PointIdx& e2) { return d(e1.first, e2.first); };

		for (auto& pair : pointsWithinCell)
		{
			// Create a VPT for every cell						
			igiVPT[pair.first].Build(pair.second, distance);
		}
	}

	std::string GetName()
	{
		return name_;
	}

	// Calculate cell for every point in pointClouds
	void PointsWithinCell(std::vector<Cloud<T>>& pointClouds, std::unordered_map<int, std::vector<PointIdx>>& pointsWithinCell)
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

				pointsWithinCell[cell].push_back(std::make_pair(p, cloud.ID));
			}
		}
	}

	// KNN Query
	// 1st Parameter: Query =  PointCloud
	// 2nd Parameter: K = K Nearest Neighbors PointClouds
	// 3rd Parameter: internalK-NN
	std::vector<std::pair<int, int>> KNN(const Cloud<T>& queryCloud, const int k, const int internalK) const
	{
		std::unordered_map<int, int> count;
		std::vector<PointIdx> results;
		std::vector<double> distances;

		int px, py, cell;

		// For every point in the PointCloud 
		for (const auto& point : queryCloud.Points)
		{
			px = static_cast<int>(std::floor(boost::geometry::get<0>(point) / delta_));
			py = static_cast<int>(std::floor(boost::geometry::get<1>(point) / delta_));

			cell = px + static_cast<int>(cmax_ / delta_)*py;

			auto it = igiVPT.find(cell);

			// Get List from Inverted Index and count frequency of ID's
			if (it != std::end(igiVPT))
			{				
				(it->second).KNN(std::make_pair(point, 0), internalK, results, distances);

				// Count the frequencies for the Clouds ID
				for (const auto& item : results)
				{
					count[item.second]++;
				}

				results.clear();
				distances.clear();				
			}
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
	// 3rd Parameter: internalK-NN
	// 3rd Parameter: recallAt = Vector for desired Recall@
	template<typename Duration = std::chrono::milliseconds>PerformanceReport KNNPerformanceReport(const std::vector<Cloud<T>>& queryClouds, const int k,const int internalK ,const std::vector<int>& recallAt) const
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

			performance.QueriesTime.push_back(std::chrono::duration_cast<Duration>(end - start).count());
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
#include "vp-tree.h"
#include "UtilityFunctions.h"
#include "PerformanceReport.h"
#include "Cloud.h"
#include <boost/geometry.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <chrono>

// Miguel Ramirez Chacon
// 22/05/17

// Index for Point Clouds based in combination of Inverted Grid Index and Vantage Point Tree
// T: Point class(2D)
// D: Distance Function (Metric)

template<typename T>
class IGIVpt
{
using PointIdx = std::pair<T, unsigned>;

private:
std::unordered_map<unsigned, VpTree<PointIdx>> igiVPT;
std::unordered_map<unsigned, unsigned> sizeClouds;
std::string name_;
const unsigned cmax_;
const unsigned delta_;

public:

IGIVpt(std::vector<Cloud<T>>& pointClouds, std::function<double(const PointIdx&, const PointIdx&)> dist,std::string name, const unsigned cmax, const unsigned delta) :name_{ name }, cmax_{ cmax }, delta_{ delta }
{
std::unordered_map<unsigned, std::vector<PointIdx>> pointsWithinCell;
PointsWithinCell(pointClouds, pointsWithinCell);

for (auto& pair : pointsWithinCell)
{
// Create a VPT for every cell
igiVPT[pair.first].Build(pair.second, dist);
}
}

std::string GetName()
{
return name_;
}

// Calculate cell for every point in pointClouds
void PointsWithinCell(std::vector<Cloud<T>>& pointClouds, std::unordered_map<unsigned, std::vector<PointIdx>>& pointsWithinCell)
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
// 3rd Parameter: internalK-NN
std::vector<std::pair<unsigned, unsigned>> KNN(const Cloud<T>& queryCloud, const unsigned k, const unsigned internalK) const
{
std::unordered_map<unsigned, unsigned> count;
std::vector<PointIdx> results;
std::vector<double> distances;

unsigned px, py, cell;

// For every point in the PointCloud
for (const auto& point : queryCloud.Points)
{
px = static_cast<unsigned>(std::floor(boost::geometry::get<0>(point) / delta_));
py = static_cast<unsigned>(std::floor(boost::geometry::get<1>(point) / delta_));

cell = px + static_cast<unsigned>(cmax_ / delta_)*py;

auto it = igiVPT.find(cell);

// Get List from Inverted Index and count frequency of ID's
if (it != std::end(igiVPT))
{
(it->second).KNN(std::make_pair(point, 0), internalK, results, distances);

// Count the frequencies for the Clouds ID
for (const auto& item : results)
{
count[item.second]++;
}

results.clear();
distances.clear();
}
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
// 3rd Parameter: internalK-NN
// 3rd Parameter: recallAt = Vector for desired Recall@
template<typename Duration = std::chrono::milliseconds>PerformanceReport KNNPerformanceReport(const std::vector<Cloud<T>>& queryClouds, const unsigned k,const unsigned internalK ,const std::vector<unsigned>& recallAt) const
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

performance.QueriesTime.push_back(std::chrono::duration_cast<Duration>(end - start).count());
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