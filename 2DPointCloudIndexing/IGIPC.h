#pragma once
#include "Cloud.h"
#include "PerformanceReport.h"
#include <boost/geometry.hpp>
#include <vector>
#include <unordered_map>
#include <utility>
#include <chrono>
#include <cmath>

// Miguel Ramirez Chacon
// 19/05/17

// Index for Point Clouds based in Inverted Grid Index
// T: Point class(2D)

template<typename T = boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian>>
class IGIPC
{
private:
	std::unordered_map<unsigned, std::vector<unsigned>> IGI;
	std::unordered_map<unsigned, unsigned> sizeClouds;
	unsigned cmax_;
	unsigned delta_;

public:
	
	// Build index from vector of Point Clouds
	IGIPC(std::vector<Cloud<T>> pointClouds, unsigned cmax, unsigned delta) :cmax_{ cmax }, delta_{ delta }
	{
		for (const auto& cloud : pointClouds)
		{
			// Get size of all PointCloud for calculate support
			sizeClouds[cloud.ID] = cloud.Points.size();

			// Add cloud to index
			Add(cloud);			
		}
	}
	
	// Add PointCloud to Index
	IGIPC& Add(Cloud<T> pointCloud)
	{
		unsigned px, py, cell;
		for (const auto& point : pointCloud)
		{
			// Calculate cell of point
			px = static_cast<unsigned>(std::floor(boost::geometry::get<0>(point) / delta_));
			py = static_cast<unsigned>(std::floor(boost::geometry::get<1>(point) / delta_));
			cell = px + static_cast<unsigned>(cmax_ / delta_)*py;
			
			// Inverted Index
			IGI[cell].push_back(point.ID);
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
		for (const auto& point : queryCloud)
		{
			px = static_cast<unsigned>(std::floor(boost::geometry::get<0>(point) / delta_));
			py = static_cast<unsigned>(std::floor(boost::geometry::get<1>(point) / delta_));

			cell = px + static_cast<unsigned>(cmax_ / delta_)*py;			

			// Get List from Inverted Index and count frequency of ID's
			if (IGI.find(cell) != std::end(IGI))
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
	template<typename D = std::chrono::milliseconds>PerformanceReport KNNPerformanceReport(const std::vector<Cloud<T>>& queryClouds, unsigned k, std::vector<unsigned> recallAt) const
	{
		PerformanceReport results;
		results.QueriesTime.reserve(queryClouds.size());
		results.RecallAt.reserve(queryClouds.size());
		std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

		// For every cloud in vector of PointClouds
		for (const auto cloud : queryClouds)
		{
			// Perform KNN search
			start = std::chrono::high_resolution_clock::now();
			auto result = KNN(cloud, k);
			end = std::chrono::high_resolution_clock::now();

			GetRecall(results, result, recallAt, cloud.ID);

			results.QueriesTime.push_back(std::chrono::duration_cast<D>(end - start).count());
		}
		// Calculate Recall@
		for (auto& pair : results.RecallAt)
		{
			pair.second = pair.second / static_cast<double>(queryClouds.size());
		}

		TimePerformance(results);

		return results;
	}

};