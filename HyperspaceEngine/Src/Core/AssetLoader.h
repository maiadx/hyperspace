#pragma once
#include <vector>
#include <string>

class hyAssetLoader
{

public:
	static std::vector<char> ReadFile(const std::string& filepath);
};

