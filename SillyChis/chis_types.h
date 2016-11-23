#ifndef CHIS_TYPES
#define CHIS_TYPES
#include <vector>
#include <algorithm>
#include "pattern_map.h"

namespace chis {
	using U64 = unsigned __int64;
	using U32 = unsigned __int32;
	using U16 = unsigned __int16;
	using U8 = unsigned char;
	struct Parameters;
	struct _depth_with_value_;
	struct _point_with_value;
	struct Point;
	struct chis_config;
	class _board;
	const U64 EMP = (0); //00
	const U64 BLK = (1); //01
	const U64 WHI = (2); //10
	const U64 SID = (3); //11
	const int NEGA_WON = (INT_MAX - 1);
	const int NEGA_LOS = (INT_MIN + 2);
	const U8 HASH_USELESS = 0;
	const U8 HASH_GAMEOVER = 1;
	const U8 HASH_ALPHA = 2;
	const U8 HASH_PV = 3;
	const U8 HASH_BETA = 4;
	struct config {
		size_t SIZE = 15;//棋盘大小
		int RULE = 0;//规则类型（只支持freestyle
		//time ms
		time_t search_time = 29900;//一次搜索时间
		time_t timeout_match = 180000;//总搜索时间上限
		time_t time_left = 180000;//剩下的时间

		size_t hash_max_size = 1024*100;//100MB
		size_t search_depth = 8;
	};
}
#endif