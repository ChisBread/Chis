#ifndef SILLY_CHIS
#define SILLY_CHIS
#include "chis_board.h"
#include <array>
#define HASH_MB(x) ((1 << 20)*(x)/(sizeof(hash_item)))
namespace chis {
	struct hash_item {
		U64 key;
		U8 type;
		U8 ply;
		int value;
		hash_item() :type(HASH_USELESS) {};
		hash_item(U8 t):type(t) {};
		hash_item(U64 k, U8 t, U8 p, int v):key(k), type(t), ply(p), value(v) {};
	};

	template<const size_t MAP_SIZE>
	class silly_chis {
		
	public:
		_board bod;
		Point chis_move();
		silly_chis() :bod(15) {}
		silly_chis(size_t bsize, size_t hash_size = 1024*1024*128): bod(bsize) {
			conf.SIZE = bsize;
		}
		inline void hash_insert(U64 key, U8 type, U8 ply, int value) {
			auto &bk = hash_map[key % MAP_SIZE];
			//ply first
			if(bk[0].ply <= ply) {
				bk[0].key = key;
				bk[0].type = type;
				bk[0].ply = ply;
				bk[0].value = value;
			}
			bk[1].key = key;
			bk[1].type = type;
			bk[1].ply = ply;
			bk[1].value = value;
		}
		inline hash_item hash_find(U64 key) {
			auto &bk = hash_map[key % MAP_SIZE];
			//ply first
			if(bk[0].key == key) {
				return bk[0];
			}
			if(bk[1].key == key) {
				return bk[1];
			}
			return hash_item();
		}
	private:
		bool vctf_search(bool p, U8 vc_depth);
		int max_min_search(int alpha, int beta, U8 ply, bool zero_window = false);
		static std::array<std::array<hash_item, 2>, MAP_SIZE> hash_map;
		config conf;
	};
	template<const size_t MAP_SIZE>
	std::array<std::array<hash_item, 2>, MAP_SIZE> silly_chis<MAP_SIZE>::hash_map;
	template<const size_t MAP_SIZE>
	bool silly_chis<MAP_SIZE>::vctf_search(bool p, U8 vc_depth) {
		if(bod.have_winner()) {
			if(p) {//算杀方被杀了.....
				return false;
			}
			return true;//算杀成功
		}
		auto hi = hash_find(bod.hash_value());
		if(hi.type == HASH_GAMEOVER) {
			if(p) {
				if(hi.value == NEGA_WON) {
					return true;
				}
				else {
					return false;
				}
			}
			else {
				if(hi.value == NEGA_LOS) {
					return true;
				}
				else {
					return false;
				}
			}
		}
		if(vc_depth <= 0) {
			return false;
		}
		std::vector<_point_with_value> moves;
		//int ps[30][30] = {};
		if(p) {
			if(vc_depth > 8) {
				std::swap(moves,
					bod.get_turn() == BLK ?
					bod.get_pruned_moves_black() : bod.get_pruned_moves_white());
			}
			else if(vc_depth > 4) {
				std::swap(moves,
					bod.get_turn() == BLK ?
					bod.get_pruned_moves_black_forvcf() : bod.get_pruned_moves_white_forvcf());
			}
			else {
				std::swap(moves,
					bod.get_turn() == BLK ?
					bod.get_pruned_moves_black_forvct() : bod.get_pruned_moves_white_forvct());
			}
		}
		else {
			std::swap(moves,
				bod.get_turn() == BLK ?
				bod.get_pruned_moves_black_fordefend() : bod.get_pruned_moves_white_fordefend());
		}
		if(moves.empty()) {
			return false;
		}
		bool pn = !p;//算杀方从0起算，防守方从1起算
		for(auto&i : moves) {
			const Point &po = i.first;
			bod.make_move({ po.x, po.y }, bod.get_turn());
			if(p) {//算杀方，OR
				pn = pn || vctf_search(!p, vc_depth - 1);
				bod.unmove();
				if(pn) {
					break;//如果有算杀方有一处杀，返回算杀成功
				}
			}
			else {//防守方，AND
				pn = pn && vctf_search(!p, vc_depth - 1);
				bod.unmove();
				if(!pn) {
					break;//如果防守方有一处活，返回算杀失败
				}
			}

		}
		if(pn) {//算到了杀
			hash_insert(bod.hash_value(), HASH_GAMEOVER, conf.search_depth, p ? NEGA_WON : NEGA_LOS);
		}
		return pn;
	}
	template<const size_t MAP_SIZE>
	int silly_chis<MAP_SIZE>::max_min_search(int alpha, int beta, U8 ply, bool zero_window) {

		if(bod.have_winner()) {//上家已经赢了
			hash_insert(bod.hash_value(), HASH_GAMEOVER, ply, NEGA_LOS);
			return NEGA_LOS;
		}
		auto hi = hash_find(bod.hash_value());
		if(hi.type == HASH_GAMEOVER) {
			return hi.value;
		}
		if(hi.ply >= ply) {
			if(hi.type == HASH_PV) {
				return hi.value;
			}
			else if(hi.type == HASH_ALPHA && hi.value < alpha) {
				return alpha;
			}
			else if(hi.type == HASH_BETA && hi.value >= beta) {
				return beta;
			}
		}
		if(ply <= 0) {
			auto v = bod.evaluation();
			if(v > alpha && v < beta) {
				if(vctf_search(false, 11)) {
					hash_insert(bod.hash_value(), HASH_GAMEOVER, ply, NEGA_LOS);
					return NEGA_LOS;
				}
				if(vctf_search(true, 10)) {
					hash_insert(bod.hash_value(), HASH_GAMEOVER, ply, NEGA_WON);
					return NEGA_WON;
				}
				hash_insert(bod.hash_value(), HASH_PV, ply, v);
			}
			return v;
		}
		std::vector<_point_with_value> moves(bod.get_turn() == BLK
			? bod.get_pruned_moves_black()
			: bod.get_pruned_moves_white());

		auto max_v = NEGA_LOS;
		for(auto &i : moves) {
			const Point &p = i.first;
			bod.make_move(p, bod.get_turn());
			int child_value;
			//pvs
			child_value = -max_min_search(-alpha - 1, -alpha, ply - 1, true);
			if(child_value >= alpha && child_value < beta) {
				child_value = -max_min_search(-beta, -alpha, ply - 1, zero_window);
			}
			bod.unmove();
			if(child_value > max_v) {
				max_v = child_value;
				if(max_v > alpha) {
					alpha = max_v;
					if(alpha >= beta) {//m持续变大而last_m取小，剪枝
						break;
					}
				}
			}
		}
		if(!zero_window) { //排除零窗口
			if(max_v == NEGA_LOS || max_v == NEGA_WON) {
				hash_insert(bod.hash_value(), HASH_GAMEOVER, ply, max_v);
			}
			else if(max_v >= beta) {
				hash_insert(bod.hash_value(), HASH_BETA, ply, max_v);
			}
			else if(max_v < alpha) {
				hash_insert(bod.hash_value(), HASH_ALPHA, ply, max_v);
			}
			else {
				hash_insert(bod.hash_value(), HASH_PV, ply, max_v);
			}
		}
		return max_v;
	}
	template<const size_t MAP_SIZE>
	Point silly_chis<MAP_SIZE>::chis_move() {
		if(!bod.moves_size()) {
			return Point((U8)conf.SIZE / 2 + 5, conf.SIZE / 2 + 5);
		}
		std::vector<_point_with_value> moves(bod.get_turn() == BLK
			? bod.get_pruned_moves_black_root()
			: bod.get_pruned_moves_white_root());
		Point good_move;
		for(int depth = 2, alpha = NEGA_LOS; depth == 2 ||
			depth <= conf.search_depth; ++depth, alpha = NEGA_LOS) {

			std::sort(moves.begin(), moves.end());
			good_move = moves[0].first;
			while(moves.size() > 1 && moves.back().value == NEGA_LOS) {
				moves.pop_back();
			}
			if(moves[0].value == (NEGA_WON)) {
				return good_move;
			}
			for(auto &i : moves) {
				const Point &p = i.first;
				bod.make_move({ p.x, p.y }, bod.get_turn());
				int child_value;
				if(alpha == NEGA_LOS) {
					child_value = i.value = -max_min_search(NEGA_LOS, -alpha, depth, false);
				}
				else {
					child_value = i.value = -max_min_search(-alpha - 1, -alpha, depth, true);
					if(child_value >= alpha && child_value != NEGA_WON) {
						child_value = i.value = -max_min_search(NEGA_LOS, -alpha, depth, false);
					}
				}
				bod.unmove();
				if(child_value > alpha) {
					if(child_value == NEGA_WON) {
						return p;
					}
					alpha = child_value;
					good_move = p;

				}
			}
		}
		return good_move;
	}
}
#endif