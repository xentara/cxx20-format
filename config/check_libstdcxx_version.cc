#include <version>

int main()
{
	#if defined __GLIBCXX__ && _GLIBCXX_RELEASE == 12
	static constexpr auto kHaveCompatibleLibCxxVersion { true };
	#else
	static constexpr auto kHaveCompatibleLibCxxVersion { false };
	#endif

	static_assert(kHaveCompatibleLibCxxVersion, "could not find compatible libstdc++ version");

	return 1;
}
