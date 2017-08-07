#pragma once
#include <unordered_map>
#include <vector>
// Miguel Ramirez Chacon
// 17/05/17

struct PerformanceReport
{
	std::unordered_map<unsigned, double> RecallAt;
	std::vector<int> QueriesTime;
	double AverageQueryTime;
	double SDQueryTime;
	double MaxQueryTime;
	double MinQueryTime;
};
