#pragma once

#include <Precompiled.h>

namespace util
{
	inline void throwError(function<void()>&& body)
	{
		cout << "-------------------- ERROR --------------------\n";
		body();
		cout << "-----------------------------------------------\n";

		system("pause");
		exit(EXIT_FAILURE);
	}
}