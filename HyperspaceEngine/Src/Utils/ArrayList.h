#pragma once
#include <iostream>
#include <Utils/Math.h>


template<typename T>
class hyArrayList
{
	T* Data;
	size_t NumElements;
	size_t AllocatedSize;

	void Realloc(size_t size)
	{
		T* dataResized = new T[size];
			
		for (size_t i = 0; i < AllocatedSize; i++)
		{
			#ifdef _MSC_VER
			#pragma warning(push)
			#pragma warning(disable : 6386)
			#endif

			dataResized[i] = Data[i];

			#ifdef _MSC_VER
			#pragma warning(pop)
			#endif
		}

		Data = dataResized;
		AllocatedSize = size;
	}
	
public:
	hyArrayList() : AllocatedSize(10), NumElements(0), Data{ new T[AllocatedSize] } {}

	hyArrayList(size_t size) : AllocatedSize(size), NumElements(0), Data{ new T[size] } {}

	~hyArrayList() { delete[] Data; }

	size_t Size() { return NumElements; }
	size_t GetAllocatedSize() { return AllocatedSize; }

	T& operator[](size_t i)             { return Data[i]; }
	const T& operator[](size_t i) const { return Data[i]; }

	T* Get() { return Data; }
	const T* Get() const { return Data; }

	void PushBack(T& object)       
	{ 
		if (Size == AllocatedSize - 1)
			Realloc(AllocatedSize + AllocatedSize/2);

		Data[Size++] = object;
	}

	void PushBack(const T& object)
	{
		if (Size == AllocatedSize - 1)
			Realloc(AllocatedSize + AllocatedSize / 2);
			
		Data[NumElements++] = object;
	}

	void Reserve(size_t size)
	{
		if(size > this->NumElements)
			Realloc(size);
	}

};

