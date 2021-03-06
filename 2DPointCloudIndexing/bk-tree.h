#pragma once
#include <utility>
#include <memory>
#include <vector>
#include <unordered_map>
#include <random>
#include <functional>
#include <stack>
#include <algorithm>
#include <queue>
#include "HeapItem.h"

// Miguel Ramirez Chacon
// 28/05/17

/* Burkhard Keller Tree
@article{Burkhard1973,
author = { Burkhard, W.A. and Keller, R.M. },
doi = { 10.1145 / 362003.362025 },
issn = { 00010782 },
journal = { Communications of the ACM },
keywords = { best match,file searching,file structuring,heuristics,matching },
month = { apr },
number = { 4 },
pages = { 230--236 },
publisher = { ACM },
title = { { Some Approaches to Best - Match File Searching } },
url = { http://portal.acm.org/citation.cfm?doid=362003.362025},
volume = { 16 },
year = { 1973 }
}
// Based on BKT from https://github.com/mkarlesky/csharp-bk-tree */


// T: Point class(2D)
template<typename T>
struct BKNode
{
	BKNode() {}
	BKNode(T data) : Data{ data } {}

	T Data;
	std::unordered_map<unsigned, BKNode<T>> children;
};

template<typename T>
class BkTree
{
private:
	BKNode<T> root;	
	std::function<unsigned(const T&, const T&)> distance;

public:
	BkTree() {}

	void Add(const T& data)
	{
		Add(BKNode<T>(data), root);		
	}

	void Add(BKNode<T>& data, BKNode<T>& node)
	{
		std::stack < BKNode<T>*> nodeStack;
		nodeStack.push(&node);

		while (nodeStack.size() != 0)
		{
			auto currentPair = nodeStack.top();
			nodeStack.pop();

			auto d = distance(data.Data, currentPair->Data);

			for (auto& child : currentPair->children)
			{
				if (d == child.first)
				{
					nodeStack.push(&(child.second));
					continue;
				}
			}
			currentPair->children.insert({ d, data });
		}
	}

	void Build(const std::vector<T>& Data, std::function<unsigned(const T&, const T&)> dist)
	{
		distance = dist;

		auto data = Data;
		while (data.size()>0)
		{
			std::mt19937 gen(0);
			std::uniform_int_distribution<> dis(0, data.size() - 1);
			int i = dis(gen);
			std::swap(data[data.size() - 1], data[i]);
			auto pivot = data[data.size() - 1];
			data.pop_back();

			Add(pivot);
		}
	}

	void KNN(T target, unsigned k, std::vector<T>& results, std::vector<unsigned>& distances) const
	{
		std::priority_queue<HeapItem<T>> heap;

		unsigned _tau = std::numeric_limits<unsigned>::max();
		Search(root, target, k, heap, _tau);

		results.clear();
		distances.clear();

		while (!heap.empty())
		{
			results.push_back(heap.top().Data);
			distances.push_back(heap.top().Distance);
			heap.pop();
		}

		std::reverse(std::begin(results), std::end(results));
		std::reverse(std::begin(distances), std::end(distances));
	}

	void Search(BKNode<T>& node, T& target, unsigned k,
		std::priority_queue<HeapItem<T>>& heap, unsigned& _tau) const
	{
		std::stack<BKNode<T>*> snapshotStack;
		snapshotStack.push(&node);

		while (snapshotStack.size() != 0)
		{
			auto currentNode = snapshotStack.top();
			snapshotStack.pop();

			auto dist = distance(currentNode->Data, target);

			if (dist <= _tau)
			{
				if (heap.size() == k)
					heap.pop();

				heap.push(HeapItem<T>(currentNode->Data, dist));

				if (heap.size() == k)
					_tau = heap.top().Distance;
			}

			for (int d = dist - _tau; d <= dist + _tau; d++)
			{
				if (currentNode->children.find(d) != currentNode->children.end())
				{
					snapshotStack.push(&(currentNode->children[d]));
				}
			}
		}
	}
};

