#pragma once
#include <utility>
#include "UtilityFunctions.h"
#include <unordered_map>
#include <string>
#include <chrono>
#include <functional>
#include "vp-tree.h"
#include "PerformanceReport.h"
#include "Cloud.h"
#include <boost/geometry.hpp>

// Miguel Ramirez Chacon
// 21/05/17

// Index for Point Clouds based in Vantage Point Tree
// T: Point class(2D)
// D: Distance Function (Metric)

template<typename T>
class VPT
{
	using PointIdx = std::pair<T, unsigned>;

private:
	VpTree<PointIdx> vpt;
	std::unordered_map<unsigned, unsigned> sizeClouds;
	std::string name_;

public:

	VPT(std::string name) :name_{ name } {}

	// Build index from vector of Point Clouds
	void Build(const std::vector<Cloud<T>>& pointClouds, std::function<double(const std::pair<T, unsigned>&, const std::pair<T, unsigned>&)> dist)
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

		// Generate Index
		vpt.Build(data, dist);
	}

	std::string GetName() { return name_; }

	// KNN Query
	// 1st Parameter: Query =  PointCloud
	// 2nd Parameter: K = K Nearest Neighbors PointClouds
	// 3rd Parameter: internalK = internalK-NN queries per point in PointCloud
	std::vector<std::pair<unsigned, unsigned>> KNN(const Cloud<T>& queryCloud, const unsigned k, const unsigned internalK) const
	{
		std::vector<PointIdx> results;
		std::vector<double> distances;
		std::unordered_map<unsigned, unsigned> count;
		auto i = 0;
		// K queries for every point in the PointCloud
		for (const auto& point : queryCloud.Points)
		{
			vpt.KNN(std::make_pair(point, 1), internalK, results, distances);

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
			[](const std::pair<unsigned, unsigned>& left, const std::pair<unsigned, unsigned>& right) {return left.second>right.second; });

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
