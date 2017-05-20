#pragma once
#include "PerformanceReport.h"
#include <algorithm>
#include <numeric>
// Miguel Ramirez Chacon
// 19/05/17

// Function to Get recall@k1,k2,...kn
void GetRecall(PerformanceReport& results, const std::vector<std::pair<unsigned, unsigned>>& result, const std::vector<unsigned>& recallAt, unsigned cloudID)
{
	// Calculate position for Recall@
	auto it = find_if(result.begin(), result.end(),
		[ind = cloudID](const std::pair<unsigned, unsigned>& elemento) {return elemento.first == ind; });

	if (it != result.end())
	{
		auto position = std::distance(result.begin(), it);

		for (const auto& at : recallAt)
		{
			if (position <= (at - 1))
			{
				results.RecallAt[at] = results.RecallAt[at] + 1;
			}
		}
	}
}

// Function to get Average, Max, Min and SD query time
void TimePerformance(PerformanceReport& results)
{
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
}
