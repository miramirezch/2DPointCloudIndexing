#pragma once
#include "bk-tree.h"

#include <utility>
#include <unordered_map>
#include <string>
#include <chrono>
#include <functional>
#include <boost/geometry.hpp>

#include "UtilityFunctions.h"
#include "PerformanceReport.h"
#include "Cloud.h"

// Miguel Ramirez Chacon
// 05/06/17

// Index for Point Clouds based in BKT
// T: Point class(2D)
// D: Discrete Distance Function (Metric)

template<typename T>
class BKT
{
	// Pair<Data, CloudID>
	using PointIdx = std::pair<T, int>;

private:
	BKTree<PointIdx> bkt;
	std::unordered_map<int, int> sizeClouds;
	std::string name_;

public:

	BKT() {}

	BKT(std::string name) :name_{ name } {}

	// Build index from vector of Point Clouds
	void Build(const std::vector<Cloud<T>>& pointClouds, std::function<int(const T&, const T&)> dist)
	{
		std::vector<PointIdx> data;
		int totalPoints = 0;

		// Get the total of points from all Point Clouds
		for (const auto& cloud : pointClouds)
		{
			totalPoints += cloud.Points.size();
		}

		data.reserve(totalPoints);

		// Prepare data to Indexing - Add the Cloud ID to every Point
		for (const auto& cloud : pointClouds)
		{
			sizeClouds[cloud.ID] = cloud.Points.size();

			for (const auto& p : cloud.Points)
			{
				data.emplace_back(std::make_pair(p, cloud.ID));
			}
		}

		auto distance = [d = dist](const PointIdx& e1, const PointIdx& e2) {
			return d(e1.first, e2.first);
		};

		// Generate Index
		bkt.Build(data, distance);
	}

	std::string GetName() { return name_; } const

	// KNN Query
	// 1st Parameter: Query =  PointCloud
	// 2nd Parameter: K = K Nearest Neighbors PointClouds
	// 3rd Parameter: internalK = internalK-NN queries per point in PointCloud
	std::vector<std::pair<int, int>> KNN(const Cloud<T>& queryCloud, const int k, const int internalK) const
	{
		std::vector<PointIdx> results;
		std::vector<int> distances;
		std::unordered_map<int, int> count;
		int numberResults{ 0 };

		// K queries for every point in the PointCloud
		for (const auto& point : queryCloud.Points)
		{
			bkt.KNN(std::make_pair(point, 1), internalK, results, distances);

			// Count the frequencies for the Clouds ID
			for (const auto& item : results)
			{
				count[item.second]++;
			}
			results.clear();
			distances.clear();
		}

		numberResults = 0;

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
	// 3rd Parameter: internalK = internalK-NN
	// 3rd Parameter: recallAt = Vector for desired Recall@
	template<typename Duration = std::chrono::milliseconds>PerformanceReport KNNPerformanceReport(const std::vector<Cloud<T>>& queryClouds, const int k, const int internalK, const std::vector<int>& recallAt) const
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

			performance.QueriesTime.emplace_back(std::chrono::duration_cast<Duration>(end - start).count());
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
#include "bk-tree.h"

#include <utility>
#include <unordered_map>
#include <string>
#include <chrono>
#include <functional>
#include <boost/geometry.hpp>

#include "UtilityFunctions.h"
#include "PerformanceReport.h"
#include "Cloud.h"

// Miguel Ramirez Chacon
// 05/06/17

// Index for Point Clouds based in BKT
// T: Point class(2D)
// D: Discrete Distance Function (Metric)

template<typename T>
class BKT
{
using PointIdx = std::pair<T, unsigned>;

private:
BKTree<PointIdx> bkt;
std::unordered_map<unsigned, unsigned> sizeClouds;
std::string name_;

public:

BKT(std::string name) :name_{ name } {}

// Build index from vector of Point Clouds
void Build(const std::vector<Cloud<T>>& pointClouds, std::function<unsigned(const T&, const T&)> dist)
{
std::vector<PointIdx> data;
int totalPoints = 0;

// Get the total of points from all Point Clouds
for (const auto& cloud : pointClouds)
{
totalPoints += cloud.Points.size();
}

data.reserve(totalPoints);

// Prepare data to Indexing - Add the Cloud ID to every Point
for (const auto& cloud : pointClouds)
{
sizeClouds[cloud.ID] = cloud.Points.size();

for (const auto& p : cloud.Points)
{
data.push_back(std::make_pair(p, cloud.ID));
}
}

auto distance = [d = dist](const PointIdx& e1, const PointIdx& e2) {
return d(e1.first, e2.first);
};

// Generate Index
bkt.Build(data, distance);
}

std::string GetName() { return name_; }

// KNN Query
// 1st Parameter: Query =  PointCloud
// 2nd Parameter: K = K Nearest Neighbors PointClouds
// 3rd Parameter: internalK = internalK-NN queries per point in PointCloud
std::vector<std::pair<unsigned, unsigned>> KNN(const Cloud<T>& queryCloud, const unsigned k, const unsigned internalK) const
{
std::vector<PointIdx> results;
std::vector<unsigned> distances;
std::unordered_map<unsigned, unsigned> count;
auto i = 0;
// K queries for every point in the PointCloud
for (const auto& point : queryCloud.Points)
{
bkt.KNN(std::make_pair(point, 1), internalK, results, distances);

// Count the frequencies for the Clouds ID
for (const auto& item : results)
{
count[item.second]++;
}
results.clear();
distances.clear();
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
// 3rd Parameter: internalK = internalK-NN
// 3rd Parameter: recallAt = Vector for desired Recall@
template<typename Duration = std::chrono::milliseconds>PerformanceReport KNNPerformanceReport(const std::vector<Cloud<T>>& queryClouds, const unsigned k, const unsigned internalK, const std::vector<unsigned>& recallAt) const
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