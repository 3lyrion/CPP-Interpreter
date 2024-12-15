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