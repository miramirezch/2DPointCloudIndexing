#pragma once
#include "PerformanceReport.h"
#include <algorithm>
#include <numeric>
#include <iostream>
#include <string>
#include <cmath>
// Miguel Ramirez Chacon
// 19/05/17

// Function to Get recall@k1,k2,...kn
void GetRecall(PerformanceReport& results, const std::vector<std::pair<int, int>>& result, const std::vector<int>& recallAt, int cloudID)
{
	// Calculate position for Recall@
	auto it = find_if(result.begin(), result.end(),
		[ind = cloudID](const std::pair<int, int>& elemento) {return elemento.first == ind; });

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

// Function to Get recall@k1,k2,...kn
void GetRecall(PerformanceReport& results, const std::vector<std::pair<int, double>>& result, const std::vector<int>& recallAt, int cloudID)
{
	// Calculate position for Recall@
	auto it = find_if(result.begin(), result.end(),
		[ind = cloudID](const std::pair<int, double>& elemento) {return elemento.first == ind; });

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
void TimePerformance(PerformanceReport& report)
{
	// Calculate Min and Max query time
	auto max_min = std::minmax_element(std::begin(report.QueriesTime), std::end(report.QueriesTime));
	report.MinQueryTime = *max_min.first;
	report.MaxQueryTime = *max_min.second;

	// Calculate average query time
	auto sum = std::accumulate(std::begin(report.QueriesTime), std::end(report.QueriesTime), 0);
	auto average = static_cast<double>(sum) / report.QueriesTime.size();
	report.AverageQueryTime = average;

	// Calculate standard deviation - query time		
	double var = 0;
	for (auto i = 0; i < report.QueriesTime.size(); i++)
	{
		var += (report.QueriesTime[i] - average) * (report.QueriesTime[i] - average);
	}
	var = var / (report.QueriesTime.size() - 1);

	report.SDQueryTime = std::sqrt(var);
}

void PrintPerformanceReport(PerformanceReport& report,std::string name ,std::string timeUnits)
{
	std::cout << "-------------------------------------------------------------" << '\n';
	std::cout << name << '\n';
	std::cout <<"Query Time: Performance" << '\n';	
	std::cout << "Query Time - Average: " << report.AverageQueryTime << " " << timeUnits << std::endl;
	std::cout << "Query Time - Standard Deviation: " << report.SDQueryTime << " " << timeUnits << '\n';
	std::cout << "Query Time - Maximum: " << report.MaxQueryTime << " " << timeUnits << '\n';
	std::cout << "Query Time - Minimum: " << report.MinQueryTime << " " << timeUnits << '\n';
	std::cout << "-------------------------------------------------------------" << '\n';
	std::cout << name << '\n';
	std::cout << "Recall - Performance" << '\n';
	for (const auto pair : report.RecallAt)
	{
		std::cout << "Recall@" << pair.first << " :" << pair.second << '\n';
	}
}



/*

#pragma once
#include "PerformanceReport.h"
#include <algorithm>
#include <numeric>
#include <iostream>
#include <string>
#include <cmath>
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

// Function to Get recall@k1,k2,...kn
void GetRecall(PerformanceReport& results, const std::vector<std::pair<unsigned, double>>& result, const std::vector<unsigned>& recallAt, unsigned cloudID)
{
// Calculate position for Recall@
auto it = find_if(result.begin(), result.end(),
[ind = cloudID](const std::pair<unsigned, double>& elemento) {return elemento.first == ind; });

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
void TimePerformance(PerformanceReport& report)
{
// Calculate Min and Max query time
auto max_min = std::minmax_element(std::begin(report.QueriesTime), std::end(report.QueriesTime));
report.MinQueryTime = *max_min.first;
report.MaxQueryTime = *max_min.second;

// Calculate average query time
auto sum = std::accumulate(std::begin(report.QueriesTime), std::end(report.QueriesTime), 0);
auto average = static_cast<double>(sum) / report.QueriesTime.size();
report.AverageQueryTime = average;

// Calculate standard deviation - query time
double var = 0;
for (auto i = 0; i < report.QueriesTime.size(); i++)
{
var += (report.QueriesTime[i] - average) * (report.QueriesTime[i] - average);
}
var = var / (report.QueriesTime.size() - 1);

report.SDQueryTime = std::sqrt(var);
}

void PrintPerformanceReport(PerformanceReport& report,std::string name ,std::string timeUnits)
{
std::cout << "-------------------------------------------------------------" << '\n';
std::cout << name << '\n';
std::cout <<"Query Time: Performance" << '\n';
std::cout << "Query Time - Average: " << report.AverageQueryTime << " " << timeUnits << std::endl;
std::cout << "Query Time - Standard Deviation: " << report.SDQueryTime << " " << timeUnits << '\n';
std::cout << "Query Time - Maximum: " << report.MaxQueryTime << " " << timeUnits << '\n';
std::cout << "Query Time - Minimum: " << report.MinQueryTime << " " << timeUnits << '\n';
std::cout << "-------------------------------------------------------------" << '\n';
std::cout << name << '\n';
std::cout << "Recall - Performance" << '\n';
for (const auto pair : report.RecallAt)
{
std::cout << "Recall@" << pair.first << " :" << pair.second << '\n';
}
}

*/