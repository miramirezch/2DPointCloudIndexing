#pragma once
#include "vp-tree.h"
#include "PerformanceReport.h"
#include "Cloud.h"
#include <boost/geometry.hpp>
#include <unordered_map>
#include <vector>
#include <memory>
#include <chrono>


template<typename T, double(*distance)(const std::pair<T, unsigned>&, const std::pair<T, unsigned>&)>
class IGIVpt
{
	using PointIdx = std::pair<T, unsigned>;

private:
	std::unordered_map<unsigned, std::unique_ptr<VpTree<PointIdx, distance>>> igiVPT;
	std::unordered_map<unsigned, unsigned> sizeClouds;
	std::string name_;
	const unsigned cmax_;
	const unsigned delta_;

public:

	IGIVpt(const std::vector<Cloud<T>>& pointClouds, std::string name, const unsigned cmax, const unsigned delta) :name_{ name }, cmax_{ cmax }, delta_{ delta }
	{
		std::unordered_map<unsigned, std::vector<PointIdx>> pointsWithinCell;
		PointsWithinCell(pointClouds, pointsWithinCell);

		for (const auto& pair : pointsWithinCell)
		{
			// Create a VPT for every cell
			auto tempVPT = std::make_unique<VpTree<PointIdx, distance>>();
			tempVPT->create(pair.second);
			igiVPT[pair.first] = std::move(tempVPT);					
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
	std::vector<std::pair<unsigned, unsigned>> KNN(const Cloud<T>& queryCloud, const unsigned k) const
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
				(it->second)->search(std::make_pair(point, 0), k, &results, &distances);

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
	// 3rd Parameter: recallAt = Vector for desired Recall@
	template<typename Duration = std::chrono::milliseconds>PerformanceReport KNNPerformanceReport(const std::vector<Cloud<T>>& queryClouds, const unsigned k, const std::vector<unsigned>& recallAt) const
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
			auto result = KNN(cloud, k);
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
