#pragma once
#include "vp-tree.h"
#include <vector>
#include <random>
#include <functional>
#include <unordered_map>
#include <map>
#include <iostream>


// Reverse Nearest Neighbor List of Clusters(Rev - LC)

/*
@inproceedings{SaditTellez:2012:LCR:2367875.2367898,
author = {Sadit Tellez, Eric and Ch\'{a}vez, Edgar},
title = {The List of Clusters Revisited},
booktitle = {Proceedings of the 4th Mexican Conference on Pattern Recognition},
series = {MCPR'12},
year = {2012},
isbn = {978-3-642-31148-2},
location = {Huatulco, Mexico},
pages = {187--196},
numpages = {10},
url = {http://dx.doi.org/10.1007/978-3-642-31149-9_19},
doi = {10.1007/978-3-642-31149-9_19},
acmid = {2367898},
publisher = {Springer-Verlag},
address = {Berlin, Heidelberg},
}
*/

template<typename T>
struct Cluster
{
	T Center;
	double CoveringRadii;
	std::vector<T> Bucket;
};


template<typename T>
class ReverseLC
{
private:
	std::vector<Cluster<T>> listClusters;
	std::function<double(const T&, const T&)> distance;
public:

	ReverseLC() {}

	void Build(const std::vector<T>& db, std::function<double(const T&, const T&)> dist, const unsigned m)
	{
		distance = dist;
		std::vector<T> data = db;
		std::vector<std::pair<unsigned, T>>  centers(m);

		//Select random centers
		for (auto i = 0; i < m; i++)
		{
			std::mt19937 gen(0);
			std::uniform_int_distribution<> dis(0, data.size() - 1);
			std::swap(data[dis(gen)], data[data.size() - 1]);
			auto pivot = data[data.size() - 1];
			centers[i] = std::make_pair(i, pivot);
			data.pop_back();

			Cluster<T> cluster;
			cluster.Center = pivot;
			cluster.CoveringRadii = 0;
			listClusters.push_back(cluster);
		}

		// Index for nearest neighbors
		VpTree<std::pair<unsigned, T>> vpt;
		auto d = distance;
		auto dist2 = [d](const std::pair<unsigned, T>& p1, const std::pair<unsigned, T>& p2)
		{return d(p1.second, p2.second); };

		vpt.Build(centers, dist2);

		std::vector<std::pair<unsigned, T>> results;
		std::vector<double> distances;

		for (auto& element : data)
		{
			vpt.KNN(std::make_pair(0, element), 1, results, distances);
			auto neighborID = results[0].first;
			auto neighborDist = distances[0];

			listClusters[neighborID].CoveringRadii = std::max(listClusters[neighborID].CoveringRadii, neighborDist);
			listClusters[neighborID].Bucket.push_back(element);
		}

	}

	void KNN(const T& target, const unsigned k, std::vector<T>& results, std::vector<double>& distances) const
	{
		std::priority_queue<HeapItem<T>> heap;
		double tau = std::numeric_limits<double>::max();

		Search(target, k, heap, tau);

		while (heap.size() > 0)
		{
			results.push_back(heap.top().Data);
			distances.push_back(heap.top().Distance);
			heap.pop();
		}

		std::reverse(std::begin(results), std::end(results));
		std::reverse(std::begin(distances), std::end(distances));

	}

	void Search(const T& target, const unsigned k, std::priority_queue<HeapItem<T>>& heap, double& tau) const
	{
		for (const auto& cluster : listClusters)
		{			
			auto d = distance(target, cluster.Center);

			if (d < tau)
			{
				if (heap.size() == k)
				{
					heap.pop();
				}

				heap.push(HeapItem<T>(cluster.Center, d));

				if (heap.size() == k)
				{
					tau = heap.top().Distance;
				}
			}

			if (d <= cluster.CoveringRadii + tau)
			{				
				for (const auto& item : cluster.Bucket)
				{
					d = distance(target, item);
					
					if (d < tau)
					{
						if (heap.size() == k)
						{
							heap.pop();
						}

						heap.push(HeapItem<T>(item, d));

						if (heap.size() == k)
						{
							tau = heap.top().Distance;
						}

					}
				}

			}
		}
	}

};
