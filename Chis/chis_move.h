#ifndef _CHIS_MOVE
#define _CHIS_MOVE
#include <vector>
namespace chis {
	class Board;
	struct Point;
	struct _point_with_value;
	///////////////////////////�ŷ�/�������//////////////////////////////
	int max_min_search(Board &b, int alpha, int beta);
	//����Chis������b��������߷�
	Point chis_move(Board &b);
	void safe_prune(Board &b, std::vector<_point_with_value> &moves);
	//////////////////////////////////////////////////////////////////////
	size_t time();
	void get_move(int(&ps)[30][30], Board &b);
	void get_move_in_root(int(&ps)[30][30], Board &b);
}
#endif