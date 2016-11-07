#ifndef CHIS_FOR_PAOFAN
#define CHIS_FOR_PAOFAN
#include "chis_gomoku.h"
#include <array>
#ifdef CHIS_ENABLE_MTB
#undef CHIS_ENABLE_MTB
#endif
namespace chis {
	using raw_board = std::array<std::array<char, 15>, 15>;
	extern _board board;
	//1 black (01)
	//2 white (10)
	//0 empty (00)
	//3 illegal (11)

	raw_board get_board() {
		raw_board rb;
		for(int i = 0; i < 15; ++i) {
			for(int j = 0; j < 15; ++j) {
				rb[i][j] = board[i + 5][j + 5];
			}
		}
		return rb;
	}
	//5 <= p.x, p.y < 20
	raw_board move(Point p) {
		board.make_move(p, board.get_turn());
		p = chis_move(board);
		board.make_move(p, board.get_turn());
		return get_board();
	}
	// 0 <= x, y < 15
	inline raw_board move(U8 x, U8 y) {
		return move(Point(x + 5, y + 5));
	}
	inline raw_board move(std::pair<int, int> p) {
		return move(Point((U8)p.first + 5, (U8)p.second + 5));
	}
	//步时，局时
	void set_time(time_t turn, time_t match) {
		SEARCH_TIME = turn;
		timeout_match = match;
		CHIS_CONFIG.SEARCH_TIME = SEARCH_TIME;
	}
	
	//1 black (01)
	//2 white (10)
	//0 no
	int winner() {
		return board.have_winner();
	}
	void reset() {
		board.clear();
		clear_hash();
	}
}
#endif