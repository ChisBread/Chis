#ifndef _CHIS_PAT_MAP
#define _CHIS_PAT_MAP
namespace chis{
	extern const int pattern_map[(1 << 13)];
	extern const unsigned pattern_rev[1 << 7];//用于加快棋型匹配速度
}
#endif