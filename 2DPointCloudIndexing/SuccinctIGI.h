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
#include <set>
//#include <sdsl/bit_vectors.hpp>

// Miguel Ramirez Chacon
// 23/05/17

// Succinct Inverted Grid Index for Point Clouds
// T: Point class(2D)

template<typename T = boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian>>
class SuccinctIGI
{
private:
	std::unordered_map<unsigned, sdsl::sd_vector<>> succinctIGI;
	std::unordered_map<unsigned, unsigned> sizeClouds;
	std::unordered_map<unsigned, unsigned> onesPerBitmap;
	const std::string name_;
	const unsigned cmax_;
	const unsigned delta_;

public:

	SuccinctIGI(const std::vector<Cloud<T>>& pointClouds, std::string name, const unsigned cmax, const unsigned delta) :name_{ name }, cmax_{ cmax }, delta_{ delta }
	{
		std::unordered_map<unsigned, std::set<unsigned>> tempIGI;

		for (const auto& cloud : pointClouds)
		{
			// Get size of all PointCloud for calculate support
			sizeClouds[cloud.ID] = cloud.Points.size();

			// Add cloud to index
			Add(cloud, tempIGI);
		}

		// For every cell
		for (const auto& pair : tempIGI)
		{
			// Generate bitmap
			sdsl::bit_vector bitmap = sdsl::bit_vector(pointClouds.size(), 0);

			// For every ID in set 
			for (const auto& id : pair.second)
			{
				bitmap[id] = 1;
			}

			// Generate SArray from bitmap
			sdsl::sd_vector<> sarray(bitmap);
			succinctIGI[pair.first] = sarray;
			// Number or 1's per bitmap
			sdsl::sd_vector<>::rank_1_type sarray_rank(&sarray);
			onesPerBitmap[pair.first] = sarray_rank(sarray.size());
		}
	}

	std::string GetName()
	{
		return name_;
	}

	// Add PointCloud to Index
	void Add(const Cloud<T>& pointCloud, std::unordered_map<unsigned, std::set<unsigned>>& tempIGI)
	{
		unsigned px, py, cell;
		for (const auto& point : pointCloud.Points)
		{
			// Calculate cell of point
			px = static_cast<unsigned>(std::floor(boost::geometry::get<0>(point) / delta_));
			py = static_cast<unsigned>(std::floor(boost::geometry::get<1>(point) / delta_));
			cell = px + static_cast<unsigned>(cmax_ / delta_)*py;

			// Inverted Index
			tempIGI[cell].insert(pointCloud.ID);
		}
	}

	// KNN Query
	// First Parameter: queryCloud =  PointCloud
	// Second Parameter: k = Nearest Neighbors
	std::vector<std::pair<unsigned, unsigned>> KNN(const Cloud<T>& queryCloud, const unsigned k) const
	{
		std::unordered_map<unsigned, unsigned> count;

		unsigned px, py, cell;

		std::unordered_map<unsigned, sdsl::sd_vector<>> listSarrays;
		listSarrays.reserve(queryCloud.Points.size());

		// For every point in the PointCloud
		for (const auto& point : queryCloud.Points)
		{
			px = static_cast<unsigned>(std::floor(boost::geometry::get<0>(point) / delta_));
			py = static_cast<unsigned>(std::floor(boost::geometry::get<1>(point) / delta_));

			cell = px + static_cast<unsigned>(cmax_ / delta_)*py;

			auto it = succinctIGI.find(cell);

			// Get Sarray from Inverted Index
			if (it != std::end(succinctIGI))
			{
				sdsl::sd_vector<>::select_1_type select_sarray(&(it->second));
				auto it = onesPerBitmap.find(cell);
				auto ones = it->second;
				unsigned i{ 1 };
				while (i<ones)
				{
					count[select_sarray(i)]++;
					i++;
				}
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
	template<typename D = std::chrono::milliseconds>PerformanceReport KNNPerformanceReport(const std::vector<Cloud<T>>& queryClouds, const unsigned k, const std::vector<unsigned>& recallAt) const
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