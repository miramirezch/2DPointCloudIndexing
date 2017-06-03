#pragma once
#include "Cloud.h"
#include "PerformanceReport.h"
#include "UtilityFunctions.h"
#include <boost/geometry.hpp>
#include <vector>
#include <unordered_map>
#include <utility>
#include <chrono>
#include <cmath>
#include <string>

// Miguel Ramirez Chacon
// 19/05/17

// Index for Point Clouds based in Inverted Grid Index
// T: Point class(2D)

template<typename T = boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian>>
class IGI
{
private:
	std::unordered_map<unsigned, std::vector<unsigned>> IGI_index;
	std::unordered_map<unsigned, unsigned> sizeClouds;
	std::string name_;
	const unsigned cmax_;
	const unsigned delta_;

public:

	// Build index from vector of Point Clouds
	IGI(const std::vector<Cloud<T>>& pointClouds, std::string name, const unsigned cmax, const unsigned delta) :name_{ name }, cmax_{ cmax }, delta_{ delta }
	{
		for (const auto& cloud : pointClouds)
		{
			// Get size of all PointCloud for calculate support
			sizeClouds[cloud.ID] = cloud.Points.size();

			// Add cloud to index
			Add(cloud);
		}
	}

	std::string GetName()
	{
		return name_;
	}

	// Add PointCloud to Index
	IGI& Add(const Cloud<T>& pointCloud)
	{
		unsigned px, py, cell;
		for (const auto& point : pointCloud.Points)
		{
			// Calculate cell of point
			px = static_cast<unsigned>(std::floor(boost::geometry::get<0>(point) / delta_));
			py = static_cast<unsigned>(std::floor(boost::geometry::get<1>(point) / delta_));
			cell = px + static_cast<unsigned>(cmax_ / delta_)*py;

			// Inverted Index
			IGI_index[cell].push_back(pointCloud.ID);
		}

		return *this;
	}

	// KNN Query
	// First Parameter: queryCloud =  PointCloud
	// Second Parameter: k = Nearest Neighbors
	std::vector<std::pair<unsigned, unsigned>> KNN(const Cloud<T>& queryCloud, unsigned k) const
	{
		std::unordered_map<unsigned, unsigned> count;

		unsigned px, py, cell;

		// For every point in the PointCloud 
		for (const auto& point : queryCloud.Points)
		{
			px = static_cast<unsigned>(std::floor(boost::geometry::get<0>(point) / delta_));
			py = static_cast<unsigned>(std::floor(boost::geometry::get<1>(point) / delta_));

			cell = px + static_cast<unsigned>(cmax_ / delta_)*py;

			auto it = IGI_index.find(cell);

			// Get List from Inverted Index and count frequency of ID's
			if (it != std::end(IGI_index))
			{
				auto list = it->second;
				std::for_each(std::begin(list), std::end(list), [&count](unsigned val) { count[val]++; });
			}
		}

		auto numberResults = 0;

		if (count.size() > k)
			numberResults = k;
		else
			numberResults = count.size();

		std::vector<std::pair<unsigned, unsigned>> resultsID(numberResults);

		// Get k approximate nearest neighbors
		std::partial_sort_copy(std::begin(count), std::end(count), std::begin(resultsID), std::end(resultsID),
			[](const std::pair<unsigned, unsigned>& left, const std::pair<unsigned, unsigned>& right) {return left.second>right.second; });

		return resultsID;
	}

	// Performance report on KNN Search
	// Obtain Recall@
	// Average query time, Standard deviation query time, max query time and min query time
	// 1st Parameter: Vector of Queries Point Clouds
	// 2nd Parameter: k = Nearest Neighbors	
	// 3rd Parameter: recallAt = Vector for desired Recall@
	template<typename D = std::chrono::milliseconds>PerformanceReport KNNPerformanceReport(const std::vector<Cloud<T>>& queryClouds, unsigned k, const std::vector<unsigned>& recallAt) const
	{
		PerformanceReport performance;
		performance.QueriesTime.reserve(queryClouds.size());
		performance.RecallAt.reserve(queryClouds.size());
		std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

		// For every cloud in vector of PointClouds
		for (const auto cloud : queryClouds)
		{
			// Perform KNN search
			start = std::chrono::high_resolution_clock::now();
			auto result = KNN(cloud, k);
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