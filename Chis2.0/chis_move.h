#ifndef _CHIS_MOVE
#define _CHIS_MOVE
#include "chis_config.h"

namespace chis {
	class _board;
	struct Point;
	struct _point_with_value;
	///////////////////////////�ŷ�/�������//////////////////////////////
	int max_min_search(_board &b, int alpha, int beta, U8 ply);
	//����Chis������b��������߷�
	Point chis_move(_board &b);
	//////////////////////////////////////////////////////////////////////
	time_t time();
}
#endif