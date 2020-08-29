#pragma once


template<typename T, size_t T_Size>
class hyArray
{
	T Data[T_Size];

public:
	/* constexpr: constant expression -> this value can be evaled at compile-time */
	constexpr size_t Size() const { return T_Size; }

	T* Get() { return Data; }
	const T* Get() const { return Data; }

	T& operator[](size_t i) { return Data[i]; }
	const T& operator[](size_t i) const { return Data[i]; }

	void Clear()
	{
		memset(&Data[0], NULL, T_Size * sizeof(int));
	}

};
