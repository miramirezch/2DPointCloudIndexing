#pragma once
#include <fstream>
#include <vector>
#include <unordered_map>
#include "Cloud.h"

// Miguel Ramirez Chacon
// 17/05/17

// Function to get PointClouds from CSV file
// File must comply with the following schema:
// ID, X, Y
// 1st Parameter: fileName - Full path of CSV file
// 2nd Parameter: cmin - Minimum valid coordinate value
// 3rd Parameter: cmax - Maximum valid coordinate value
// 4th Parameter: header - Flag to indicate that file has a header row
template<typename T> std::vector<Cloud<T>> ReadCSV(std::string fileName,int cmin ,int cmax, bool header)
{	
	std::unordered_map<unsigned, std::vector<T>> pointCloudsMap;

	std::ifstream inputFile{ fileName };
	std::string line;
	std::string item;

	std::vector<double> temp;
	temp.reserve(3);

	if (inputFile)
	{
		while (getline(inputFile, line) && !inputFile.eof())
		{
			temp.clear();

			// Skip header
			if (header)
			{
				header = false;
				continue;
			}

			std::stringstream lineStream{ line };

			while (getline(lineStream, item, ','))
			{
				temp.push_back(std::stof(item));
			}

			if (temp[0] >= 0 && temp[1] >= cmin && temp[2] >= cmin && temp[1]<=cmax && temp[2]<=cmax)
			{
				pointCloudsMap[static_cast<unsigned>(temp[0])].push_back(T(temp[1], temp[2]));				
			}

		}
		inputFile.close();
	}

	std::vector<Cloud<T>> pointClouds;
	pointClouds.reserve(pointCloudsMap.size());

	for (auto& pair : pointCloudsMap)
	{
		Cloud<T> cloud{ pair.first };
		cloud.Points = std::move(pair.second);
		pointClouds.push_back(cloud);
	}

	return pointClouds;
}



