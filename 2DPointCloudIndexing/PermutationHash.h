#pragma once

#include "Cloud.h"
#include "Rtree.h"
#include "PerformanceReport.h"
#include <boost/geometry.hpp>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <utility>
#include <chrono>
#include <cmath>
#include <string>
#include <set>
#include <iostream>
#include <boost/functional/hash.hpp>
#include <random>

struct key_hash2 : public std::unary_function<std::vector<unsigned>, std::size_t>
{
	std::size_t operator()(const std::vector<unsigned>& k) const
	{
		return boost::hash_range(std::begin(k), std::end(k));
	}
};

struct key_equal2
{
	bool operator()(const std::vector<unsigned>& lhs, const std::vector<unsigned>& rhs) const
	{
		bool flag = true;
		for (unsigned i{ 0 }; i<lhs.size(); i++)
		{
			if (lhs[i] != rhs[i])
			{
				return false;
			}
		}
		return true;
	}
};


template<typename T = boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian>, typename KeyHash = key_hash2, typename KeyEqual = key_equal2>
class PermutationHash
{
	using PointIdx = std::pair<T, unsigned>;

private:
	std::unordered_map<std::vector<unsigned>, Rtree<T>, KeyHash, KeyEqual> permutationIndex;
	std::unordered_map<unsigned, unsigned> sizeClouds;
	std::vector<unsigned> bins;//{10000,25000,50000,100000,500000,1500000,350000,500000,750000,1000000 };
	std::string name_;
	const unsigned cmax_;
	const unsigned delta_;
	const unsigned number_bins;

public:	

	PermutationHash(const std::vector<Cloud<T>>& pointClouds, std::string name, unsigned cmax, unsigned delta, unsigned numberBins):
		name_{name}, cmax_{cmax}, delta_{delta}, number_bins{numberBins}
	{
		GetBins(number_bins);

		std::unordered_map <std::vector<unsigned>, std::vector<Cloud<T>>, KeyHash, KeyEqual> permutationHashTable;

		for (const auto& cloud : pointClouds)
		{
			sizeClouds[cloud.ID] = cloud.Points.size();
			permutationHashTable[Permutation(cloud)].push_back(cloud);
		}

		for (const auto& pair : permutationHashTable)
		{
			permutationIndex[pair.first].Build(pair.second);
		}

	}

	void GetBins(const unsigned num_bins)
	{
		std::random_device rd;     // only used once to initialise (seed) engine
		std::mt19937 rng(rd());    // random-number engine used (Mersenne-Twister in this case)
		std::uniform_int_distribution<int> uni(0, 1000000); // guaranteed unbiased
		
		for (int i = 0; i < num_bins; i++)
		{
			bins.push_back(uni(rng));
		}

		std::sort(std::begin(bins), std::end(bins));		

	}

	std::vector<unsigned> Permutation(const Cloud<T>& pointCloud) const
	{		
		std::unordered_map<unsigned, unsigned> count;

		unsigned px, py, position;

		for (auto i = 0; i<bins.size();i++)
		{
			count[i]=0;
		}
		for (const auto& point : pointCloud.Points)
		{
			// Calculate cell of point
			px = static_cast<unsigned>(std::floor(boost::geometry::get<0>(point) / delta_));
			py = static_cast<unsigned>(std::floor(boost::geometry::get<1>(point) / delta_));
			position = px + static_cast<unsigned>(cmax_ / delta_)*py;

			auto it = std::upper_bound(std::begin(bins), std::end(bins), position);
			auto dist = std::distance(std::begin(bins), it);
			count[(dist + 1)]++;			
		}		

		std::vector<std::pair<unsigned, unsigned>> temp;
		temp.reserve(count.size());

		for (const auto& pair : count)
		{
			temp.push_back(pair);
		}

		std::sort(std::begin(temp), std::end(temp),
			[](const std::pair<unsigned, unsigned>& p1, const std::pair<unsigned, unsigned>& p2) {return p1.first < p2.first; });

		std::vector<unsigned> permutation;		
		auto previous = pointCloud.Points.size() / number_bins;
				
		for (const auto& pair : temp)
		{
			//permutation.push_back(pair.first);
			if (previous <= pair.second)
			{
				permutation.push_back(2);
				previous = pair.second;
			}
			else
			{
				permutation.push_back(3);
				previous = pair.second;
			}						
		}

		return permutation;
	}

	std::string GetName()
	{
		return name_;
	}

	// KNN Query
	// 1st Parameter: Query =  PointCloud
	// 2nd Parameter: K = K Nearest Neighbors PointClouds
	// 3rd Parameter: internalK = internalK-NN queries per point in PointCloud
	std::vector<std::pair<unsigned, unsigned>> KNN(const Cloud<T>& queryCloud, const unsigned k, const unsigned internalK) const
	{
		std::unordered_map<unsigned, unsigned> count;
		std::vector<std::pair<unsigned, unsigned>> resultsID;

		auto permutation = Permutation(queryCloud);	

		// Get Index from Permutation Index
		auto it = permutationIndex.find(permutation);
		if (it != std::end(permutationIndex))
		{
			resultsID = (it->second).KNN(queryCloud,k,internalK);					
		}	

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
