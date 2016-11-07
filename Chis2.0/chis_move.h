#ifndef _CHIS_MOVE
#define _CHIS_MOVE
#include "chis_config.h"

namespace chis {
	class _board;
	struct Point;
	struct _point_with_value;
	///////////////////////////着法/搜索相关//////////////////////////////
	int max_min_search(_board &b, int alpha, int beta, U8 ply);
	//返回Chis对棋盘b计算出的走法
	Point chis_move(_board &b);
	//////////////////////////////////////////////////////////////////////
}
#endif