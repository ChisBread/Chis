#ifndef _CHIS_PAT_HASH
#define _CHIS_PAT_HASH
namespace chis{
	extern int pattern_map[(1 << 13)];
	extern unsigned pattern_rev[1 << 7];//用于加快棋型匹配速度
}
#endif