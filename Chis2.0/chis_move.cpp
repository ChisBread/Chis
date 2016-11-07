#include "chis_move.h"
#include "chis_board.h"
#include <algorithm>
#include <cstdlib>
#include <time.h>
#include <iostream>
#pragma comment(lib,"psapi.lib") 
/////////////编译开关////////////
#define CHIS_DEBUG //debug 开关
//#define CHIS_DEFEND //选点策略
//#define CHIS_EXP
#define CHIS_ENABLE_MTB //使用选点缓存
#define CHIS_VCT
//#define CHIS_VCT_EXP
//#define CHIS_EXP
#define NEGA_WON (INT_MAX - 1)
#define NEGA_LOS (INT_MIN + 2)
#define HASH_GAMEOVER (U8)(0)
#define HASH_ALPHA (U8)(1)
#define HASH_PV (U8)(2)
#define HASH_BETA (U8)(3)
namespace chis {

	/////////////////////////////VCF/VCT/VC2相关//////////////////////////////
	//通通待完善
	//vcf剪枝策略
	bool vctf_search(_board &b, bool p) {
		//可加可不加的置换
		if(ptb.find(b.hash_value()) != ptb.end() && ptb[b.hash_value()].hash_flag == HASH_GAMEOVER) {
			if(p) {//算杀方
				if(ptb[b.hash_value()].value == NEGA_WON) {//上家输，算杀成功
#ifdef CHIS_DEBUG
					++hashfinded;
#endif
					return true;
				}
				else {
					return false;
				}
			}
			else {//防守方
				if(ptb[b.hash_value()].value == NEGA_LOS) {//上家赢，算杀成功
#ifdef CHIS_DEBUG
					++hashfinded;
#endif
					return true;
				}
				else {
					return false;
				}
			}
		}
		if(b.have_winner()) {
			if(p) {//算杀方被杀了.....
				return false;
			}
			return true;//算杀成功
		}
		//时间控制
		if(time() - search_time >= SEARCH_TIME || (time_left - (time() - search_time) <= 200)) {
			return false;
		}
#ifdef CHIS_VCT_EXP
		//延伸策略，在nps得到优化前无卵用
		if(p && vc_depth < VCT_DEPTH + VC2_DEPTH && VC_DEPTH < 15) {//在VCT范围内
			const _pattern &opp = b.move_count() % 2 ? b.black__pattern() : b.white__pattern();
			if(opp.four_b) {//如果对方冲四
				VCT_DEPTH += 2;
				VC_DEPTH += 2;
				int v = vctf_search(b, true);
				VCT_DEPTH -= 2;
				VC_DEPTH -= 2;
				return v;
			}
		}
#endif
		if((vc_depth >= VC_DEPTH)) {
			return false;
		}
		std::vector<_point_with_value> moves;
		//int ps[30][30] = {};
		if(vc_depth > VC2_DEPTH) {
			if(p) {
				if(vc_depth > VC2_DEPTH + VCT_DEPTH) {
					std::swap(moves,
						b.get_turn() == BLK ?
						b.get_pruned_moves_black_forvcf() : b.get_pruned_moves_white_forvcf());
				}
				else {
					std::swap(moves,
						b.get_turn() == BLK ?
						b.get_pruned_moves_black_forvct() : b.get_pruned_moves_white_forvct());
				}
			}
			else {
				std::swap(moves,
					b.get_turn() == BLK ?
					b.get_pruned_moves_black_fordefend() : b.get_pruned_moves_white_fordefend());
			}
		}
		else {
			std::swap(moves,
				b.get_turn() == BLK ?
				b.get_pruned_moves_black() : b.get_pruned_moves_white());
		}
		if(moves.empty()) {
			return false;
		}
		bool pn = !p;//算杀方从0起算，防守方从1起算
		for(auto&i : moves) {
			const Point &po = i.first;
			b.make_move({ po.x, po.y }, b.get_turn());
			++vc_depth;
			if(p) {//算杀方，OR
				pn = pn || vctf_search(b, !p);
				--vc_depth;
				b.unmove();
				if(pn) {
					break;//如果有算杀方有一处杀，返回算杀成功
				}
			}
			else {//防守方，AND
				pn = pn && vctf_search(b, !p);
				--vc_depth;
				b.unmove();
				if(!pn) {
					break;//如果防守方有一处活，返回算杀失败
				}
			}

		}
#ifdef CHIS_DEBUG
		++node_count;
#endif
		if(pn) {//算到了杀
			ptb[b.hash_value()] = { HASH_GAMEOVER, 99, p ? NEGA_WON : NEGA_LOS };//p?WON， !p?LOS
		}
		return pn;
	}
	//////////////////////////////待测算法/////////////////////////////////////
	//先手优势搜索
	//////////////////////////////搜索算法/////////////////////////////////////
	int max_min_search(_board &b, int alpha, int beta, U8 ply) {
		if(ptb.find(b.hash_value()) != ptb.end()) {
			if(ptb[b.hash_value()].depth == ply) {
#ifdef CHIS_DEBUG

				++hashfinded;
#endif		
				if(ptb[b.hash_value()].hash_flag == HASH_BETA) {
					if(ptb[b.hash_value()].value >= beta) {
						return ptb[b.hash_value()].value;
					}
				}
				else if(ptb[b.hash_value()].hash_flag == HASH_ALPHA) {
					if(ptb[b.hash_value()].value <= alpha) {
						return ptb[b.hash_value()].value;
					}
				}
				else if(ptb[b.hash_value()].hash_flag == HASH_PV) {
					if(ptb[b.hash_value()].value > alpha && ptb[b.hash_value()].value < beta) {
						return ptb[b.hash_value()].value;
					}
				}
			}
			else if(ptb[b.hash_value()].hash_flag == HASH_GAMEOVER) {
				return ptb[b.hash_value()].value;
			}
		}
		if(!(time() % 8)) {
			if(memcost() >= HASH_SIZE * (1024 * 1024)) {
				clear_hash();
			}
		}
		//终局剪枝
		if(b.have_winner()) {//上家已经赢了
			ptb[b.hash_value()] = { HASH_GAMEOVER, (U8)99, NEGA_LOS };
			return NEGA_LOS;
		}
		//深度
		if(ply <= 0) {
			int v = b.evaluation();
#ifdef CHIS_DEBUG
			++eval_cnt;
#endif
#ifdef CHIS_VCT

			if(v > -beta && v < -alpha) {//可能被选中的情况下
				if(allow_findvct && vctf_search(b, true)) {//算杀。（奇数层为opp）
#ifdef CHIS_DEBUG
					++vct_cnt;
#endif
					ptb[b.hash_value()] = { HASH_GAMEOVER, (U8)99, NEGA_WON };
					return NEGA_WON;//
				}
#ifdef CHIS_EXP
				if(allow_expand) {
					const _pattern &who = b.turn_color() == 1 ? b.white__pattern() : b.black__pattern();
					if(who.four_b || who.three_l) {
						//冲四/活三延伸，平衡局面延伸
						allow_expand = false;
						allow_findvct = false;
						int rv = max_min_search(b, alpha, beta, 4);
						allow_expand = true;
						allow_findvct = true;
						return rv;
					}
				}
#endif
			}
#endif
			return v;
		}
		std::vector<_point_with_value> moves;
#ifdef CHIS_ENABLE_MTB
		if(mtb[b.moves_size()].find(b.hash_value()) == mtb[b.moves_size()].end()) {
			std::swap(moves, (b.get_turn() == BLK ?
				b.get_pruned_moves_black() : b.get_pruned_moves_white()));
			mtb[b.moves_size()][b.hash_value()] = moves;
		}
		else {
			moves = mtb[b.moves_size()][b.hash_value()];
			if(pvs.find(b.hash_value()) != pvs.end()) {
				for(int i = 0; i < moves.size(); ++i) {
					if(moves[i].first == pvs[b.hash_value()]) {//如果是主要变例
						int j = i;
						while(j > 0) {
							std::swap(moves[j - 1], moves[j]);
							--j;
						}
						break;
					}
				}
			}
		}
#else
		std::swap(moves, (b.get_turn() == BLK ?
			b.get_pruned_moves_black() : b.get_pruned_moves_white()));
		if(pvs.find(b.hash_value()) != pvs.end()) {
			for(int i = 0; i < moves.size(); ++i) {
				if(moves[i].first == pvs[b.hash_value()]) {//如果是主要变例
					int j = i;
					while(j > 0) {
						std::swap(moves[j - 1], moves[j]);
						--j;
					}
					break;
				}
			}
		}
#endif
		int _alpha = -alpha;
		int _beta = -beta;
		int max_v = NEGA_LOS;//插入置换表的真实最值
		Point pv_point;

		for(auto &i : moves) {
			const Point &p = i.first;
			b.make_move({ p.x, p.y }, b.get_turn());
			int child_value;
			//pvs
			child_value = -max_min_search(b, _alpha - 1, _alpha, ply - 1);
			if(child_value >= alpha && child_value < beta) {
				child_value = -max_min_search(b, _beta, _alpha, ply - 1);
			}
			b.unmove();
			if(child_value > max_v) {
				max_v = child_value;
				pv_point = p;
				if(child_value > alpha) {
					_alpha = -(alpha = child_value);
					if(alpha >= beta) {//m持续变大而last_m取小，剪枝
#ifdef CHIS_DEBUG
						++ab_count;
#endif
						break;
					}
				}
			}
			if(time() - search_time >= SEARCH_TIME || (time_left - (time() - search_time) <= 200)) {//搜索时间到或者剩下的时间不足200ms
				stop_insert = true;
				return max_v;
			}

		}

#ifdef CHIS_DEBUG
		++node_count;
#endif
		if(!stop_insert) {//是否插入hash表
#ifdef CHIS_DEBUG
			++hashinsert;
#endif
			if(max_v == NEGA_LOS || max_v == NEGA_WON) {//杀局
				ptb[b.hash_value()] = { HASH_GAMEOVER, (U8)99, max_v };//局终不看深度
			}
			else if(max_v >= beta) {//beta节点
				ptb[b.hash_value()] = { HASH_BETA, (ply), max_v };
				//保存主要变例
				pvs[b.hash_value()] = pv_point;
#ifdef CHIS_DEBUG
				++pvs_cnt;
#endif
			}
			else if(max_v < alpha) {//alpha节点
				ptb[b.hash_value()] = { HASH_ALPHA, (ply), max_v };
				if(pvs.find(b.hash_value()) != pvs.end()) {//不是beta节点了，删除主要变例
					pvs.erase(b.hash_value());
				}
			}
			else {//PV节点
				ptb[b.hash_value()] = { HASH_PV, (ply), max_v };
			}

		}
		else if(max_v == NEGA_LOS || max_v == NEGA_WON) {//杀局
			ptb[b.hash_value()] = { HASH_GAMEOVER, 99, max_v };//局终不看深度
		}
		return max_v;
	}
	Point chis_move(_board &b) {
		search_time = time();//开始搜索的时间
		stop_insert = false;//启用置换表
		
		if(!b.moves_size()) {
			return Point(SIZE / 2 + 5, SIZE / 2 + 5);
		}
		for(int i = 0; i <= b.moves_size() + 3; ++i) {
			mtb[i].clear();
		}
		std::vector<_point_with_value>
			&&moves = b.get_turn() == BLK ?
			b.get_pruned_moves_black_root() : b.get_pruned_moves_white_root();
		//if(moves.size() == 0) {
		//	int i = 0;
		//	++i;
		//}
		int alpha;
		Point good_point;
		if(time_left <= 5000) {
			VCT_DEPTH = 4;
		}
		else if(time_left <= 1800) {
			//局时不够，降低搜索要求
			SEARCH_TIME = 125;
			MAX_DEPTH = 4;
			MAX_P = 8;
		}
		else if(time_left <= SEARCH_TIME) {
			SEARCH_TIME = time_left - 1795;
		}
		else {
			//还原设置

			CHIS_CONFIG.override_config();
			if(b.moves_size() <= 5) {
				MAX_DEPTH = 9;
			}
			else if(b.moves_size() <= 15) {
				MAX_DEPTH = 8;
			}
		}
		for(SEARCH_DEPTH = 2; (SEARCH_DEPTH <= MAX_DEPTH); ++SEARCH_DEPTH) {

			if(memcost() >= HASH_SIZE * (1024.0 * 1024)) {
				for(int i = 0; i < 400; ++i) {
					mtb[i].clear();
				}
				clear_hash();
			}
			alpha = NEGA_LOS;
			std::sort(moves.begin(), moves.end());
			////////////////////////////////////////////
			if(time_left <= 200) {//局时实在不够，静态估值
				return moves[0].first;
			}
			//裁剪必败点
			while(moves.size() > 1 && moves.back().value == NEGA_LOS) {
#ifdef CHIS_DEBUG
				++pruning_move_count;
#endif
				moves.pop_back();

			}
			////////////////////////////////////////////
			//裁剪最后一次搜索
			if(SEARCH_DEPTH == MAX_DEPTH) {
				while(moves.size() > MAX_P) {
					moves.pop_back();
				}
			}
			////////////////
			good_point = moves[0].first;
#ifdef CHIS_DEBUG
			good_move_cnt = 0;
#endif
			if(moves.size() <= 1) {
				break;
			}
#ifdef CHIS_DEBUG
			//如果有必胜点，剔除必胜点以外的点。
			if(moves[0].value == (NEGA_WON)) {
				while(moves.back().value != (NEGA_WON)) {
					moves.pop_back();
				}
			}
#endif

			for(size_t i = 0; i < moves.size() && time() - search_time < SEARCH_TIME; ++i) {
				const Point &p = moves[i].first;
				b.make_move({ p.x, p.y }, b.get_turn());
				int child_value;
				if(alpha == NEGA_LOS) {
					child_value = moves[i].value = -max_min_search(b, NEGA_LOS, -alpha, SEARCH_DEPTH);
				}
				else {
					child_value = moves[i].value = -max_min_search(b, -alpha - 1, -alpha, SEARCH_DEPTH);
					if(child_value >= alpha && child_value != NEGA_WON) {
						child_value = moves[i].value = -max_min_search(b, NEGA_LOS, -alpha, SEARCH_DEPTH);
					}
				}
				b.unmove();
				if(child_value > alpha) {
					alpha = child_value;
					good_point = p;
#ifdef CHIS_DEBUG
					good_move_cnt = i;
					if(alpha == NEGA_WON) {
						break;
					}
#endif
#ifndef CHIS_DEBUG
					if(alpha == NEGA_WON) {
						vct_cnt ? 0 : vct_cnt = (int)b.moves_size();
						return good_point;
					}
#endif
				}
			}
			if(time() - search_time >= SEARCH_TIME) {
				return good_point;
			}
#ifndef CHIS_DEBUG
			if(alpha == NEGA_WON) {
				break;
			}
#endif
			//if(SEARCH_DEPTH == MAX_DEPTH && time() - search_time < SEARCH_TIME && time() - search_time <= 5000) {
			//	//如果默认步时大于5000
			//	if(SEARCH_TIME > 5000 && CHIS_CONFIG.SEARCH_TIME > 5000) {
			//		++MAX_DEPTH;
			//		SEARCH_TIME = 5000;
			//	}
			//}
		}
#ifdef CHIS_DEBUG
		for(int i = 0; i < SIZE; ++i) {
			for(int j = 0; j < SIZE; ++j) {
				PS[i][j] = 0;
			}
		}
		for(auto &i : moves) {
			PS[i.first.x][i.first.y] = 2;
		}
#endif
		return good_point;
	}
}