#include "hypch.h"
#include "AssetLoader.h"
#include <fstream>
#include <iostream>

std::vector<char> hyAssetLoader::ReadFile(const std::string& filepath)
{
	/* start reading at ending of file: allows us to allocate memory according to read position to get size of file */
	std::ifstream file(filepath, std::ios::ate | std::ios::binary);
	
	if (!file.is_open())
		std::cerr << "Failed to open file : " << filepath << "!\n";

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	/* seek to beginning of file */
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	
	file.close();
	return buffer; 
}