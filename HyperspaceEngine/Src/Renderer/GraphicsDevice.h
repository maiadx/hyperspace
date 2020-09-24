#pragma once

class hyGraphicsDevice
{

public:
	virtual void SelectPhysicalDevice() = 0;
	virtual void CreateLogicalDevice() = 0;
	virtual void DestroyLogicalDevice() = 0;
};
