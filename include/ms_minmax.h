#ifndef MS_MINMAX_775703B8_617C_482e_9915_0016BFF9FBC9
#define MS_MINMAX_775703B8_617C_482e_9915_0016BFF9FBC9

#ifdef _MSC_VER 
// needed to cope with bug in MS library: 
// it fails to define min/max 
namespace std
{

	template <class T> inline T max(const T& a, const T& b) 
	{ 
		return (a > b) ? a : b;
	} 

	template <class T> inline T min(const T& a, const T& b) 
	{ 
		return (a < b) ? a : b;
	} 
}

#endif 

#endif 
