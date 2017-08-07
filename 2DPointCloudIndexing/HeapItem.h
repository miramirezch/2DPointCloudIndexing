#pragma once
template<typename T>
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