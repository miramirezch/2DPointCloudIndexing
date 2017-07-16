#pragma once
#include <vector>
#include <functional>
#include <cmath>
#include <random>
#include <algorithm>
#include <queue>
#include <stack>

// Miguel Ramirez Chacon
// 25/06/17
// Vantage Point Tree -  Array Based / No recursion
// Paper: Data Structures and Algorithms for Nearest Neighbor Search in General Metric Spaces
// Author: Yianilos, P.N.
// Based on https://github.com/gregorburger/vp-tree (Pointer based)

template<typename T>
class VpTree
{
private:

	struct Node
	{
		T Data;
		double Threshold;
	};

	struct StackItem
	{
		StackItem(int position, int lower, int upper) :Position{ position }, Lower{ lower }, Upper{ upper } {}
		int Position;
		int Lower;
		int Upper;
	};

	struct HeapItem
	{
		HeapItem(T data, double distance) :Data{ data }, Distance{ distance } {}
		T Data;
		double Distance;

		bool operator<(const HeapItem& o) const
		{
			return Distance < o.Distance;
		}
	};

	std::vector<Node> tree_;
	std::vector<bool> flags_;
	std::function<double(const T&, const T&)> distance;

public:

	VpTree() {}

	void Build(const std::vector<T>& db, std::function<double(const T&, const T&)> dist)
	{
		auto data = db;
		distance = dist;

		const auto power = static_cast<unsigned>(std::floor(std::log2(data.size()) + 1));
		const auto size = 1 << power;

		std::vector<Node> temp(size - 1);
		tree_ = std::move(temp);

		flags_.reserve(size - 1);

		for (auto i = 0; i < (size - 1); i++)
		{
			flags_.emplace_back(false);
		}

		Build(data, 1, 0, data.size());
	}

	void Build(std::vector<T>& data, const int position, const int lower, const int upper)
	{
		std::stack<StackItem> snapshotStack;
		snapshotStack.push(StackItem(position, lower, upper));

		while (snapshotStack.size() != 0)
		{
			const auto currentItem = snapshotStack.top();
			snapshotStack.pop();

			if (currentItem.Lower == currentItem.Upper)
				continue;

			Node node;
			node.Threshold = 0;
			node.Data = data[currentItem.Lower];

			tree_[currentItem.Position - 1] = node;
			flags_[currentItem.Position - 1] = true;

			if (currentItem.Upper - currentItem.Lower > 1)
			{
				std::mt19937 gen(0);
				std::uniform_int_distribution<> dis(currentItem.Lower, currentItem.Upper - 1);
				std::swap(data[currentItem.Lower], data[dis(gen)]);

				const auto pivot = data[currentItem.Lower];
				auto d = distance;

				const int median = (currentItem.Upper + currentItem.Lower) / 2;

				std::nth_element(
					std::begin(data) + currentItem.Lower + 1,
					std::begin(data) + median,
					std::begin(data) + currentItem.Upper,
					[pivot, d](const T& a, const T& b)
				{
					return d(pivot, a) < d(pivot, b);
				});

				node.Threshold = distance(pivot, data[median]);
				node.Data = pivot;

				tree_[currentItem.Position - 1] = node;
				flags_[currentItem.Position - 1] = true;

				snapshotStack.push(StackItem(2 * currentItem.Position, currentItem.Lower + 1, median + 1));
				snapshotStack.push(StackItem(2 * currentItem.Position + 1, median + 1, currentItem.Upper));
			}
		}

	}

	void KNN(const T& target, const int k, std::vector<T>& results, std::vector<double>& distances) const
	{
		std::priority_queue<HeapItem> heap;

		double _tau = std::numeric_limits<double>::max();
		Search(1, target, k, heap, _tau);

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

	void Search(const int position, const T& target, const int k,
		std::priority_queue<HeapItem>& heap, double& _tau) const
	{
		std::stack<int> snapshotStack;
		snapshotStack.push(position);

		while (snapshotStack.size() != 0)
		{
			const auto currentPosition = snapshotStack.top();
			snapshotStack.pop();

			if (currentPosition > tree_.size() || flags_[currentPosition - 1] == false)
			{
				continue;
			}

			double dist = distance(tree_[currentPosition - 1].Data, target);

			if (dist <= _tau)
			{
				if (heap.size() == k)
					heap.pop();

				heap.push(HeapItem(tree_[currentPosition - 1].Data, dist));

				if (heap.size() == k)
					_tau = heap.top().Distance;
			}

			double dm = tree_[currentPosition - 1].Threshold;

			if (dist < dm)
			{
				if (dist <= dm + _tau)
				{
					snapshotStack.push(2 * currentPosition);
				}

				if (dist + _tau >= dm)
				{
					snapshotStack.push(2 * currentPosition + 1);
				}

			}
			else
			{
				if (dist + _tau >= dm)
				{
					snapshotStack.push(2 * currentPosition + 1);
				}

				if (dist <= dm + _tau)
				{
					snapshotStack.push(2 * currentPosition);
				}
			}
		}
	}

};



/*
#pragma once
#include <vector>
#include <functional>
#include <cmath>
#include <random>
#include <algorithm>
#include <queue>
#include "HeapItem.h"

// Miguel Ramirez Chacon
// 28/05/17

// Array based Vantage Point Tree
// Based on https://github.com/gregorburger/vp-tree (Pointer based)
// T: Point class(2D)

template<typename T>
struct Node
{
	T Data;
	double Threshold;
};

template<typename T>
class VpTree
{
private:
	std::vector<Node<T>> tree_;
	std::vector<bool> flags_;
	std::function<double(const T&, const T&)> distance;

public:

	VpTree() {}

	void Build(std::vector<T>& data, std::function<double(const T&, const T&)> dist)
	{
		distance = dist;
		auto power = static_cast<unsigned>(std::floor(std::log2(data.size()) + 1));
		auto size =  1 << power ;

		std::vector<Node<T>> temp(size - 1);
		tree_ = std::move(temp);

		flags_.reserve(size - 1);

		for (auto i = 0; i < (size - 1); i++)
		{
			flags_.push_back(false);
		}

		auto lower = 0;
		auto upper = data.size() - 1;
		auto position = 1;

		std::random_device rd;
		std::mt19937 gen(0);
		std::uniform_real_distribution<> dis(0, 1);

		Build(data, position, dis, gen);
	}

	void Build(std::vector<T>& data, int position, std::uniform_real_distribution<> dis, std::mt19937 gen)
	{
		if (data.size() == 1)
		{
			Node<T> node;
			node.Threshold = 0;
			node.Data = data[0];

			tree_[position - 1] = node;
			flags_[position - 1] = true;
			return;
		}

		if (data.size() == 0)
			return;

		int i = static_cast<int>(dis(gen)*(data.size() - 1));
		std::swap(data[data.size() - 1], data[i]);
		auto pivot = data[data.size() - 1];
		data.pop_back();

		auto d = distance;

		int median = data.size() / 2;

		std::nth_element(
			std::begin(data),
			std::begin(data) + median,
			std::end(data),
			[pivot, d](const T& a, const T& b)
		{
			return d(pivot, a) < d(pivot, b);
		});

		Node<T> node;

		node.Threshold = distance(pivot, data[median]);
		node.Data = pivot;

		tree_[position - 1] = node;
		flags_[position - 1] = true;

		std::vector<T> leftSubTreeData(std::begin(data), std::begin(data) + median);
		std::vector<T> rightSubTreeData(std::begin(data) + median, std::end(data));

		Build(leftSubTreeData, 2 * position, dis, gen);
		Build(rightSubTreeData, 2 * position + 1, dis, gen);
	}

	void KNN(const T& target, const unsigned k, std::vector<T>& results, std::vector<double>& distances) const
	{
		std::priority_queue<HeapItem<T>> heap;

		double _tau = std::numeric_limits<double>::max();
		Search(1, target, k, heap, _tau);

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

	void Search(unsigned position, const T& target, size_t k,
		std::priority_queue<HeapItem<T>>& heap, double& _tau) const
	{
		if (position > tree_.size() || flags_[position - 1] == false)
		{
			return;
		}		

		double dist = distance(tree_[position - 1].Data, target);
		
		if (dist <= _tau)
		{
			if (heap.size() == k)
				heap.pop();

			heap.push(HeapItem<T>(tree_[position - 1].Data, dist));

			if (heap.size() == k)
				_tau = heap.top().Distance;
		}			

		double dm = tree_[position - 1].Threshold;

		if (dist < dm)
		{
			if (dist <= dm + _tau)
			{
				Search(2 * position, target, k, heap, _tau);
			}

			if (dist + _tau >= dm)
			{
				Search(2 * position + 1, target, k, heap, _tau);
			}

		}
		else
		{
			if (dist + _tau >= dm)
			{
				Search(2 * position + 1, target, k, heap, _tau);
			}

			if (dist <= dm + _tau)
			{
				Search(2 * position, target, k, heap, _tau);
			}
		}		
	}
};
*/