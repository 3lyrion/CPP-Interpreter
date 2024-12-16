#pragma once

#include <string>
#include <list>
#include <vector>
#include <unordered_set>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace std;

namespace fs = filesystem;

template <typename T, typename Del = default_delete<T> >
using uptr = unique_ptr<T, Del>;

template <typename T>
using sptr = shared_ptr<T>;

template <typename T>
using wptr = weak_ptr<T>;

namespace util
{
	// Безопасное удаление
	template <typename T>
	void dispose(T** object) 
	{
		if (object && *object)
		{
			delete *object;
			*object = nullptr;
		}
	}
}