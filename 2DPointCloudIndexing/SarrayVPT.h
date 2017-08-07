#pragma once
#include <utility>
#include <unordered_map>
#include "UtilityFunctions.h"
#include <string>
#include <chrono>
#include <functional>
#include "vp-tree.h"
#include "PerformanceReport.h"
#include "Cloud.h"
#include "vptPointers.h"
#include <boost/geometry.hpp>
#include <set>
#include <sdsl/bit_vectors.hpp>

// Miguel Ramirez Chacon
// 28/05/17

// Index for Point Clouds using Sarray and VPT
// T: Point class(2D)

template<typename T, double(*distance)(const std::pair<sdsl::sd_vector<>, unsigned>&, const std::pair<sdsl::sd_vector<>, unsigned>&)>
class SarrayVPT
{
	using PointIdx = std::pair<sdsl::sd_vector<>, unsigned>;

private:
	VptPointers<PointIdx, distance> vpt;
	std::unordered_map<unsigned, unsigned> sizeClouds;
	std::string name_;
	const unsigned cmax_;
	const unsigned delta_;

public:

	SarrayVPT(std::string name, unsigned cmax, unsigned delta) :name_{ name }, cmax_{ cmax }, delta_{ delta } {}

	void Build(const std::vector<Cloud<T>>& pointClouds)
	{
		int totalPoints{ 0 };
		std::vector<PointIdx> data;

		// Get the total of points from all Point Clouds
		for (const auto& cloud : pointClouds)
		{
			sizeClouds[cloud.ID] = cloud.Points.size();
			totalPoints += cloud.Points.size();
		}

		data.reserve(pointClouds.size());

		// Prepare data to Indexing - Add the Cloud ID to every Point
		for (const auto& cloud : pointClouds)
		{
			data.push_back(std::make_pair(GenerateSarray(cloud), cloud.ID));
		}

		// Generate Index
		vpt.create(data);
	}

	sdsl::sd_vector<> GenerateSarray(const Cloud<T>& pointCloud) const
	{
		unsigned px, py, position;
		std::set<unsigned> positions;

		for (const auto& point : pointCloud.Points)
		{
			// Calculate cell of point
			px = static_cast<unsigned>(std::floor(boost::geometry::get<0>(point) / delta_));
			py = static_cast<unsigned>(std::floor(boost::geometry::get<1>(point) / delta_));
			position = px + static_cast<unsigned>(cmax_ / delta_)*py;

			positions.insert(position);
		}

		// Generate bitmap
		auto size_bitmap = (cmax_ / delta_)*(cmax_ / delta_);
		sdsl::bit_vector bitmap = sdsl::bit_vector(size_bitmap, 0);

		// For every position in Positions 
		for (const auto& p : positions)
		{
			bitmap[p] = 1;
		}

		// Generate SArray from bitmap
		sdsl::sd_vector<> sarray(bitmap);

		return sarray;
	}

	std::string GetName() { return name_; }

	// KNN Query
	// 1st Parameter: Query =  PointCloud
	// 2nd Parameter: K = K Nearest Neighbors PointClouds
	// 3rd Parameter: internalK = internalK-NN queries per point in PointCloud
	std::vector<std::pair<unsigned, double>> KNN(const Cloud<T>& queryCloud, const unsigned k, const unsigned internalK) const
	{
		std::vector<PointIdx> results;
		std::vector<double> distances;

		vpt.search(std::make_pair(GenerateSarray(queryCloud), 0), internalK, &results, &distances);

		std::vector<std::pair<unsigned, double>> neighbors;
		neighbors.reserve(k);

		for (int i{ 0 }; i<results.size(); i++)
		{
			neighbors.push_back(std::make_pair(results[i].second, distances[i]));
		}

		return neighbors;
	}

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
