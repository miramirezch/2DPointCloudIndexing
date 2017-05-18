#pragma once
#include "Cloud.h"
#include "Results.h"
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/index/parameters.hpp>
#include <chrono>
#include <utility>
#include <algorithm>
#include <unordered_map>
#include <numeric>

// Miguel Ramirez Chacon
// 17/05/17

// Index for Point Clouds based in RTree
// T: Point class(2D)
// Param: Rtree configuration
template<typename T, typename Param = boost::geometry::index::rstar<20, 10>>
class RtreePC
{
	using PointIdx =  std::pair<T, unsigned>;
	using Box = boost::geometry::model::box<T>;

public:

	RtreePC() {}

	// Build index from vector of Point Clouds
	void Build(std::vector<Cloud<T>> pointClouds)
	{
		std::vector<PointIdx> data;
		int totalPoints = 0;

		// Get the total of points from all Point Clouds
		for (auto& cloud : pointClouds)
		{
			totalPoints += cloud.Points.size();
		}

		
		data.reserve(totalPoints);

		// Prepare data to Indexing - Add the Cloud ID to every Point
		for (auto& cloud : pointClouds)
		{
			sizeClouds[cloud.ID] = cloud.Points.size();

			for (auto& p : cloud.Points)
			{
				data.push_back(std::make_pair(p, cloud.ID));
			}
		}

		// Generate Index
		boost::geometry::index::rtree<PointIdx, Param> tempRtree(std::begin(data), std::end(data));						
		rtree = boost::move(tempRtree);
	}

	// KNN Query
	// First Parameter: Query =  PointCloud
	// Second Parameter: K = Nearest Neighbors
	std::vector<std::pair<unsigned,unsigned>> KNN(const Cloud<T>& queryCloud, unsigned k, unsigned internalK) const
	{		
		std::vector<PointIdx> results;
		std::unordered_map<unsigned, unsigned> count;

		// K queries for every point in the PointCloud
		for (const auto& point : queryCloud.Points)
		{
			rtree.query(boost::geometry::index::nearest(point, internalK), std::back_inserter(results));
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
			

		std::vector<std::pair<unsigned,unsigned>> resultsID(numberResults);		

		// Get only the first K Point Clouds based in ID frequency.
		std::partial_sort_copy(std::begin(count), std::end(count), std::begin(resultsID), std::end(resultsID),
			[](const std::pair<unsigned,unsigned>& left, const std::pair<unsigned, unsigned>& right) {return left.second>right.second; });

		return resultsID;
	}

	// Intersection Query
	// 1st Parameter: Query = PointCloud
	// 2nd Parameter: Intersection Window (Box centered in  point)
	// 3rd Parameter: Epsilon = Retrieve all Point Clouds with support > epsilon
	std::vector<std::pair<unsigned, float>> Intersection(const Cloud<T>& queryCloud, const float delta, const float epsilon) const
	{
		std::vector<PointIdx> results;
		std::unordered_map<unsigned, unsigned> count;

		// Intersection query for every point in the PointCloud
		for (const auto& point : queryCloud.Points)
		{
			// Create a Box for the intersection query
			auto cx = point.get<0>();
			auto cy = point.get<1>();

			// Box centered in the point 
			Box query_box(T(cx - (delta / 2), cy - (delta / 2)), T(cx + (delta / 2), cy + (delta / 2)));

			// Intersection query
			rtree.query(boost::geometry::index::intersects(query_box), std::back_inserter(results));
		}

		// Count ID's frequencies
		for (const auto& item : results)
		{
			count[item.second]++;
		}		

		std::vector<std::pair<unsigned, float>> resultsPrelim(count.size());

		for (const auto& pair : count)
		{
			unsigned id = pair.first;
			auto it = sizeClouds.find(pair.first);
			resultsPrelim.push_back(std::make_pair(pair.first, static_cast<float>(pair.second) / it->second));

		}

		std::sort(std::begin(resultsPrelim), std::end(resultsPrelim),
			[](const std::pair<unsigned, unsigned>& left, const std::pair<unsigned, unsigned>& right) {return left.second>right.second; });

		std::vector<std::pair<unsigned, float>> resultsID;
		resultsID.reserve(resultsPrelim.size());

		// Select only the PointCloud with support greater than epsilon
		for (const auto& r : resultsPrelim)
		{
			if (r.second > epsilon)
			{
				resultsID.push_back(r);
			}			
		}		
		return resultsID;
	}

	// Performance report on KNN Search
	// Obtain Recall@
	// Average query time, Standard deviation query time, max query time and min query time
	template<typename D>Results KNNPerformanceReport(const std::vector<Cloud<T>>& queryClouds, unsigned k, unsigned internalK ,std::vector<unsigned> recallAt) const
	{
		Results results;
		results.QueriesTime.reserve(queryClouds.size());
		results.RecallAt.reserve(queryClouds.size());
		std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

		// For every cloud in vector of PointClouds
		for (const auto cloud : queryClouds)
		{
			// Perform KNN search
			start = std::chrono::high_resolution_clock::now();
			auto result = KNN(cloud, k, internalK);
			end = std::chrono::high_resolution_clock::now();

			// Calculate position for Recall@
			auto it = find_if(result.begin(), result.end(),
				[ind = cloud.ID](const std::pair<unsigned,unsigned>& elemento) {return elemento.first == ind; });

			if (it != result.end())
			{
				auto position = std::distance(result.begin(), it);

				for (const auto& at : recallAt)
				{
					if (position <= (at-1))
					{
						results.RecallAt[at] = results.RecallAt[at] + 1;
					}
				}
			}
			results.QueriesTime.push_back(std::chrono::duration_cast<D>(end - start).count());
		}
		// Calculate Recall@
		for (auto& pair : results.RecallAt)
		{
			pair.second = pair.second / static_cast<double>(queryClouds.size());
		}

		// Calculate Min and Max query time
		auto max_min = std::minmax_element(std::begin(results.QueriesTime), std::end(results.QueriesTime));
		results.MinQueryTime = *max_min.first;
		results.MaxQueryTime = *max_min.second;

		// Calculate average query time
		auto sum = std::accumulate(std::begin(results.QueriesTime), std::end(results.QueriesTime), 0);
		auto average = sum / results.QueriesTime.size();
		results.AverageQueryTime = average;

		// Calculate standard deviation - query time		
		double var = 0;
		for (auto i = 0; i < results.QueriesTime.size(); i++)
		{
			var += (results.QueriesTime[i] - average) * (results.QueriesTime[i] - average);
		}
		var = var / (results.QueriesTime.size() - 1);
		
		results.SDQueryTime = std::sqrt(var);
		
		return results;
	}




private:
	boost::geometry::index::rtree<PointIdx, Param> rtree;
	std::unordered_map<unsigned, unsigned> sizeClouds;
};