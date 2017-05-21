#pragma once
#include "Cloud.h"
#include "FingerPrint.h"
#include "PerformanceReport.h"
#include "UtilityFunctions.h"
#include "ShazamHashParameters.h"
#include <boost/geometry.hpp>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <chrono>
#include <string>
#include <boost/functional/hash.hpp>


// Miguel Ramirez Chacon
// 20/05/17

// Index for Point Clouds based in the Paper An Industrial-Strength Audio Search Algorithm - Wang 2003
// T: Point class(2D)

struct key_hash : public std::unary_function<FingerPrint, std::size_t>
{
	std::size_t operator()(const FingerPrint& k) const
	{		
		std::size_t seed = 0;
		boost::hash_combine(seed, k.Y1);
		boost::hash_combine(seed, k.Y2);
		boost::hash_combine(seed, k.DX);
		return seed;
	}
};

struct key_equal
{
	bool operator()(const FingerPrint& lhs, const FingerPrint& rhs) const
	{
		return lhs.Y1 == rhs.Y1 && lhs.Y2 == rhs.Y2 && lhs.DX == rhs.DX;
	}
};


template<typename T = boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian>, typename KeyHash = key_hash, typename KeyEqual = key_equal>
class ShazamHash
{
private:
	std::unordered_map<FingerPrint, std::vector<unsigned>, KeyHash, KeyEqual> invertedIndex;
	std::unordered_map<unsigned, unsigned> sizeClouds;
	std::string name_;
	ShazamHashParameters parameters;

	std::vector<FingerPrint> GetFingerPrints(const Cloud<T>& pointCloud, ShazamHashParameters param) const
	{
		std::vector<FingerPrint> fingerPrints;
		fingerPrints.reserve(pointCloud.Points.size()*param.CombinationLimit);

		std::vector<std::pair<unsigned, unsigned>> discretePointCloud;
		discretePointCloud.reserve(pointCloud.Points.size());

		for (const auto& point : pointCloud.Points)
		{
			auto dx = static_cast<unsigned>(std::floor(boost::geometry::get<0>(point) / param.Delta));
			auto dy = static_cast<unsigned>(std::floor(boost::geometry::get<1>(point) / param.Delta));
			discretePointCloud.push_back(std::make_pair(dx, dy));
		}

		unsigned low_x, high_x, low_y, high_y;
		unsigned pointX, pointY;


		for (const auto& anchor : discretePointCloud)
		{

			low_x = anchor.first + param.DelayX;
			high_x = low_x + param.DeltaX;
			low_y = anchor.second - static_cast<unsigned>(param.DeltaY / 2);
			high_y = anchor.second + static_cast<unsigned>(param.DeltaY / 2);

			std::vector<std::pair<unsigned, unsigned>> tempPoints;

			for (const auto& point : discretePointCloud)
			{
				if (point.first > low_x && point.first < high_x && point.second >low_y && point.second < high_y)
				{
					tempPoints.push_back(point);
				}
			}

			auto length = 0;
			if (param.CombinationLimit < tempPoints.size())
			{
				length = param.CombinationLimit;
			}
			else
			{
				length = tempPoints.size();
			}

			std::vector<std::pair<unsigned, unsigned>> tempPoints2(length);

			std::partial_sort_copy(std::begin(tempPoints), std::end(tempPoints), std::begin(tempPoints2), std::end(tempPoints2),
				[](const std::pair<unsigned, unsigned>& p1, const std::pair<unsigned, unsigned>& p2) {
				return p1.first < p2.first;
			});

			for (const auto& p : tempPoints2)
			{

				FingerPrint fingerPrint(anchor.second, p.second, p.first - anchor.first);
				fingerPrints.push_back(fingerPrint);
			}
		}
		return fingerPrints;
	}

public:
	// Build index from vector of Point Clouds
	ShazamHash(const std::vector<Cloud<T>>& pointClouds, std::string name, ShazamHashParameters param) :name_{ name }, parameters{ param }
	{
		for (const auto& cloud : pointClouds)
		{
			// Get size of all PointCloud for calculate support
			sizeClouds[cloud.ID] = cloud.Points.size();

			// Add cloud to index
			Add(cloud, param);
		}
	}

	std::string GetName()
	{
		return name_;
	}

	

	// Add PointCloud to Index
	ShazamHash& Add(const Cloud<T>& pointCloud, ShazamHashParameters param)
	{
		auto fingerPrints = GetFingerPrints(pointCloud, param);

		for (const auto& fingerPrint : fingerPrints)
		{
			invertedIndex[fingerPrint].push_back(pointCloud.ID);
		}	
		
		return *this;
	}

	std::vector<std::pair<unsigned, unsigned>> KNN(const Cloud<T>& queryCloud, const unsigned k, ShazamHashParameters param) const
	{
		std::unordered_map<unsigned, unsigned> count;		

		auto fingerPrints = GetFingerPrints(queryCloud, param);

		for (const auto& fingerPrint : fingerPrints)
		{
			auto it = invertedIndex.find(fingerPrint);

			// Get List from Inverted Index and count frequency of ID's
			if (it != std::end(invertedIndex))
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

	template<typename D = std::chrono::milliseconds>PerformanceReport KNNPerformanceReport(const std::vector<Cloud<T>>& queryClouds, unsigned k, const std::vector<unsigned>& recallAt, ShazamHashParameters param) const
	{
		PerformanceReport performance;
		performance.QueriesTime.reserve(queryClouds.size());
		performance.RecallAt.reserve(queryClouds.size());
		std::chrono::time_point<std::chrono::high_resolution_clock> start, end;

		for (const auto& cloud : queryClouds)
		{
			start = std::chrono::high_resolution_clock::now();
			auto result = KNN(cloud, k,param);
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
