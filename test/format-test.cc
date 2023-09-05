#include <c++20-format.h>

#include <string>
#include <string_view>
#include <iostream>

using namespace std::literals;

int main()
{
	std::cout << std::format("{}\n{}\n{}\n{}\n{}\n{}\n{}\n", -122, 342u, "const char[]", "std::string"s, "std::string_view"sv, 12.4, 8.3f) << std::flush;

	return 0;
}
