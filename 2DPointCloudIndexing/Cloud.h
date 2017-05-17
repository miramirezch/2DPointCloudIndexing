#pragma once
#include <vector>
#include <unordered_map>

// Miguel Ramirez Chacon
// 16/05/17

// PointCloud Struct
template<typename T>
struct Cloud
{	
	Cloud(unsigned id) :ID{ id } {}

	// Fluent interface for inserting points
	Cloud& Add(T point)
	{
		Points.push_back(point);
		return *this;
	}

	std::vector<T> Points;
	unsigned ID;
};