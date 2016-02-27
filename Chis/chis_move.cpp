#include "chis_move.h"
#include "gomoku_board.h"
#include <algorithm>
#include <cstdlib>
#include <time.h>
#pragma comment(lib,"psapi.lib") 
/////////////编译开关////////////
//#define CHIS_DEBUG //debug 开关
//#define CHIS_DEFEND //选点策略
//#define CHIS_TEST2//估值策略测试
//#define CHIS_EXP
#define CHIS_VCT
//#define CHIS_PARALLEL_KILL
//#define CHIS_VCT_EXP
#define NEGA_WON (INT_MAX - 1)
#define NEGA_LOS (INT_MIN + 2)
using namespace chis;

////////////////////////////////基本功能/////////////////////////////////
size_t chis::time() {
	SYSTEMTIME t;
	GetLocalTime(&t);
	return (t.wMinute * 60 + t.wSecond) * 1000 + t.wMilliseconds;
}

void chis::get_move(int(&ps)[30][30], Board &b) {
	for(int i = 0; i < SIZE; ++i) {
		for(int j = 0; j < SIZE; ++j) {
			if(b[i][j]) {//找候选点
				for(int k = 1, p = 2; k < 3; ++k, --p) {
					if(i + k < SIZE) {
						if(!b[i + k][j]) {
							ps[i + k][j] += p;
						}
						if(j + k < SIZE) {
							if(!b[i + k][j + k]) {
								ps[i + k][j + k] += p;
							}
						}
						if(j - k >= 0) {
							if(!b[i + k][j - k]) {
								ps[i + k][j - k] += p;
							}
						}
					}
					if(i - k >= 0) {
						if(!b[i - k][j]) {
							ps[i - k][j] += p;
						}
						if(j + k < SIZE) {
							if(!b[i - k][j + k]) {
								ps[i - k][j + k] += p;
							}
						}
						if(j - k >= 0) {
							if(!b[i - k][j - k]) {
								ps[i - k][j - k] += p;
							}
						}
					}
					if(j + k < SIZE) {
						if(!b[i][j + k]) {
							ps[i][j + k] += p;
						}
					}
					if(j - k >= 0) {
						if(!b[i][j - k]) {
							ps[i][j - k] += p;
						}
					}
				}
			}
		}
	}
}
void chis::get_move_in_root(int(&ps)[30][30], Board &b) {
	int cl = (b.move_count() % 2 ? -1 : 1);

	for(int i = 0; i < SIZE; ++i) {
		for(int j = 0; j < SIZE; ++j) {
			if(b[i][j]) {//找候选点
				bool isblocked[8] = {};
				for(int k = 1, p = ((cl == b[i][j]) ? 3 : 2); k <= 3 && p; ++k, --p) {
					if(i + k < SIZE) {
						if(!isblocked[0] && !b[i + k][j]) {
							ps[i + k][j] += p;
						}
						else {
							isblocked[0] = true;
						}
						if(j + k < SIZE) {
							if(!isblocked[1] && !b[i + k][j + k]) {
								ps[i + k][j + k] += p;
							}
							else {
								isblocked[1] = true;
							}
						}
						if(j - k >= 0) {
							if(!isblocked[2] && !b[i + k][j - k]) {
								ps[i + k][j - k] += p;
							}
							else {
								isblocked[2] = true;
							}
						}
					}
					if(i - k >= 0) {
						if(!isblocked[3] && !b[i - k][j]) {
							ps[i - k][j] += p;
						}
						else {
							isblocked[3] = true;
						}
						if(j + k < SIZE) {
							if(!isblocked[4] && !b[i - k][j + k]) {
								ps[i - k][j + k] += p;
							}
							else {
								isblocked[4] = true;
							}
						}
						if(j - k >= 0) {
							if(!isblocked[5] && !b[i - k][j - k]) {
								ps[i - k][j - k] += p;
							}
							else {
								isblocked[5] = true;
							}
						}
					}
					if(j + k < SIZE) {
						if(!isblocked[6] && !b[i][j + k]) {
							ps[i][j + k] += p;
						}
						else {
							isblocked[6] = true;
						}
					}
					if(j - k >= 0) {
						if(!isblocked[7] && !b[i][j - k]) {
							ps[i][j - k] += p;
						}
						else {
							isblocked[7] = true;
						}
					}
				}
			}
		}
	}
}
/////////////////////////////VCF/VCT/VC2相关//////////////////////////////
//通通待完善
//vcf剪枝策略
void vcf_prune(Board &b, std::vector<_point_with_value> &moves) {
	int cl = b.move_count() % 2;
	int mec = cl ? -1 : 1;
	int oppc = -mec;
	const Patterns &me = cl ? b.white_patterns() : b.black_patterns();
	const Patterns &opp = cl ? b.black_patterns() : b.white_patterns();
	std::vector<_point_with_value> moves_ped;
	if(!me.four_b && !me.four_l) {//己方没四
		if(opp.four_b) {//对方有四
		}
		else if(me.three_l) {//对方没有四，己方有活三,可胜
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//模拟己方落子，返回己方棋型
				if(pat.four_l) {//己方增加活四的点
					moves_ped.push_back(i);
				}
			}
		}
		else {//什么情况也不是,VCF剪枝
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//模拟己方落子
				//有冲四，同时保证能有下一步的进攻
				if(pat.four_b && pat.three_s) {
					moves_ped.push_back(i);
				}
			}
		}
	}
	else {//己方有四，可胜
		for(auto&i : moves) {
			Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//模拟己方落子，返回己方棋型
			if(pat.wined) {//己方胜利点
				moves_ped.push_back(i);
			}
		}
	}
	moves = moves_ped;
}
//vct剪枝策略
void vct_prune(Board &b, std::vector<_point_with_value> &moves) {
	int cl = b.move_count() % 2;
	int mec = cl ? -1 : 1;
	int oppc = -mec;
	const Patterns &me = cl ? b.white_patterns() : b.black_patterns();
	const Patterns &opp = cl ? b.black_patterns() : b.white_patterns();
	std::vector<_point_with_value> moves_ped;
	if(!me.four_b && !me.four_l) {//己方没四
		if(opp.four_b) {//对方有四
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//模拟己方落子,返回对方棋型
				Patterns &&pat2 = b.get_update(i.first.x, i.first.y, mec);//模拟己方落子
				if(!pat.four_b) {//对方减少冲四
					moves_ped.push_back(i);
				}
			}
		}
		else if(me.three_l) {//对方没有四，己方有活三,可胜
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//模拟己方落子，返回己方棋型
				if(pat.four_l) {//己方增加活四的点
					moves_ped.push_back(i);
				}
			}
		}
		else if(opp.three_l) {//对方有活三
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//模拟己方落子，返回对方棋型
				Patterns &&pat2 = b.get_update(i.first.x, i.first.y, mec);//模拟己方落子，返回己方棋型
				if(((pat.three_l < opp.three_l && pat2.three_l) || pat2.four_b)//对方减少活三,或者己方冲四
					&& (pat2.three_l + pat2.four_b + pat2.three_s + pat2.two_l) >= 2) {//己方有下一步进攻
					moves_ped.push_back(i);
				}
			}
		}
		else {//什么情况也不是,VCT剪枝
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//模拟己方落子
				//有冲四活三，同时保证能有下一步的进攻
				if((pat.three_l || pat.four_b) && (pat.three_l + pat.four_b + pat.three_s + pat.two_l) >= 2) {
					moves_ped.push_back(i);
				}
			}
		}
	}
	else {//己方有四，可胜
		for(auto&i : moves) {
			Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//模拟己方落子，返回己方棋型
			if(pat.wined) {//己方胜利点
				moves_ped.push_back(i);
			}
		}
	}
	moves = moves_ped;
}
//防守方的剪枝策略
void def_prune(Board &b, std::vector<_point_with_value> &moves) {
	int cl = b.move_count() % 2;
	int mec = cl ? -1 : 1;
	int oppc = -mec;
	const Patterns &me = cl ? b.white_patterns() : b.black_patterns();
	const Patterns &opp = cl ? b.black_patterns() : b.white_patterns();
	std::vector<_point_with_value> moves_ped;
	if(!me.four_b && !me.four_l) {//己方没四
		if(opp.four_b) {//对方有四
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//模拟己方落子,返回对方棋型
				if(!pat.four_b) {//对方减少冲四的点
					moves_ped.push_back(i);
				}
			}
		}
		else if(me.three_l) {//对方没有四，己方有活三,可胜

		}
		else if(opp.three_l) {//对方有活三
			//TODO XX__OO_O__XX 特判
			if(me.three_s) {//己方有眠三，可冲可挡
				for(auto&i : moves) {
					Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//模拟己方落子，返回对方棋型
					if(pat.three_l < opp.three_l) {//对方减少活三的点
						moves_ped.push_back(i);
					}
					else {
						Patterns &&pat1 = b.get_update(i.first.x, i.first.y, mec);//模拟己方落子，返回己方棋型
						if(pat1.four_b) {//己方增加冲四的点
							moves_ped.push_back(i);
						}
					}
				}
			}
			else {//己方没有眠三，可档
				for(auto&i : moves) {
					Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//模拟己方落子，返回对方棋型
					if(pat.three_l < opp.three_l) {//对方减少活三的点
						moves_ped.push_back(i);
					}
				}
			}
		}
	}
	moves = moves_ped;
}

bool vctf_search(Board &b, bool p) {
	//可加可不加的置换
	if(ptb.find(b.hash_value()) != ptb.end()) {
		if(p) {//算杀方
			if(ptb[b.hash_value()].value == NEGA_LOS) {//上家输，算杀成功
#ifdef CHIS_DEBUG
				++hashfinded;
#endif
				return true;
			}
		}
		else {//防守方
			if(ptb[b.hash_value()].value == NEGA_WON) {//上家赢，算杀成功
#ifdef CHIS_DEBUG
				++hashfinded;
#endif
				return true;
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
	//所谓的延伸策略，在nps得到优化前无卵用
	if(p && vct_depth < VCT_DEPTH + VC2_DEPTH && VC_DEPTH < 15) {//在VCT范围内
		const Patterns &opp = b.move_count() % 2 ? b.black_patterns() : b.white_patterns();
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
	int ps[30][30] = {};
	int color = (b.move_count() % 2 ? -1 : 1);//0代表返回最大，1代表返回最小
	chis::get_move(ps, b);
	for(int i = 0; i < chis::SIZE; ++i) {
		for(int j = 0; j < chis::SIZE; ++j) {
			if(ps[i][j] >= 2) {
				moves.push_back(_point_with_value(Point(i, j), b.try_set_nega(i, j, color)));
			}
		}
	}
	if(vc_depth > VC2_DEPTH) {
		if(p) {
			if(vc_depth > VC2_DEPTH + VCT_DEPTH) {
				vcf_prune(b, moves);
			}
			else {
				vct_prune(b, moves);
			}
		}
		else {
			def_prune(b, moves);
		}
	}
	else {
		safe_prune(b, moves);
	}
	if(moves.empty()) {
		return false;
	}
	std::sort(moves.begin(), moves.end());
	bool pn = !p;//算杀方从0起算，防守方从1起算
	for(auto&i : moves) {
		const Point &po = i.first;
		b.set(po.x, po.y, color);
		++vc_depth;
		if(p) {//算杀方，OR
			pn = pn || vctf_search(b, !p);
			--vc_depth;
			b.set(po.x, po.y, 0);
			if(pn) {
				break;//如果有算杀方有一处杀，返回算杀成功
			}
		}
		else {//防守方，AND
			pn = pn && vctf_search(b, !p);
			--vc_depth;
			b.set(po.x, po.y, 0);
			if(!pn) {
				break;//如果防守方有一处活，返回算杀失败
			}
		}

	}
#ifdef CHIS_DEBUG
	++node_count;
#endif
	if(pn) {//算到了杀
		ptb[b.hash_value()] = { 9, p ? NEGA_LOS : NEGA_WON };//p?上家LOS， !p?上家WON
	}
	return pn;
}
//////////////////////////////待测算法/////////////////////////////////////

//////////////////////////////搜索算法/////////////////////////////////////
int chis::max_min_search(Board &b, int alpha, int beta) {
	if(ptb.find(b.hash_value()) != ptb.end()) {
		if(ptb[b.hash_value()].depth >= (SEARCH_DEPTH - depth)) {

#ifdef CHIS_DEBUG
			++hashfinded;
#endif
			return ptb[b.hash_value()].value;
		}
	}
	//终局剪枝
	if(b.have_winner()) {
		return NEGA_WON;
	}
	//深度
	if(depth > SEARCH_DEPTH) {
#ifdef CHIS_EXP
		if((b.move_count() % 2) ?
			(b.black_patterns().four_b)
			: (b.white_patterns().four_b)) {
			++SEARCH_DEPTH;
#ifdef CHIS_DEBUG
			if(SEARCH_DEPTH > max_scdepth) {
				max_scdepth = SEARCH_DEPTH;
			}
#endif
			int v = max_min_search(b, alpha, beta);//棋子带来的影响
			--SEARCH_DEPTH;
			return v;
		}
#endif
		int v = b.nega_value();
#ifdef CHIS_DEBUG
		++eval_cnt;
#endif
#ifdef CHIS_VCT
		if(v > -beta && v < -alpha) {//可能被选中的情况下
			if(vctf_search(b, true)) {//算杀。（奇数层为opp）
#ifdef CHIS_DEBUG
				++vct_cnt;
#endif
				return NEGA_LOS;
			}
		}
#endif
		return v;
	}
	std::vector<_point_with_value> moves;

	int color = (b.move_count() % 2 ? -1 : 1);//0代表返回最大，1代表返回最小
	int ps[30][30] = {};
	get_move(ps, b);
	for(int i = 0; i < SIZE; ++i) {
		for(int j = 0; j < SIZE; ++j) {
			if(ps[i][j] >= 2) {
				moves.push_back(_point_with_value(Point(i, j), b.point_eval(i, j, color)));
			}
		}
	}
	safe_prune(b, moves);
	if(pvs.find(b.hash_value()) != pvs.end()) {
		for(auto &i : moves) {
			if(i.first == pvs[b.hash_value()]) {//如果是主要变例，给一个最大估值
				i.value = 10000;
				break;
			}
		}
	}
	std::sort(moves.begin(), moves.end());//降序

	int _alpha = -alpha;
	int _beta = -beta;
	int max_v = NEGA_LOS;//插入置换表的真实最值
	Point pv_point;

	for(auto &i : moves) {
		const Point &p = i.first;
		b.set(p.x, p.y, color);
		++depth;
		int child_value;
		//pvs
		child_value = max_min_search(b, _alpha - 1, _alpha);
		if(child_value >= alpha && child_value < beta) {
			child_value = max_min_search(b, _beta, _alpha);
		}
		--depth;
		b.set(p.x, p.y, 0);
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
			return -max_v;
		}
	}

#ifdef CHIS_DEBUG
	++node_count;
#endif
	if(!stop_insert) {//是否插入hash表
#ifdef CHIS_DEBUG
		++hashinsert;
#endif
		if(max_v == NEGA_LOS || max_v == NEGA_WON) {
			ptb[b.hash_value()] = { 9, -max_v };//局终不看深度
		}
		else if(max_v >= alpha && max_v < beta) {//PV节点
#ifdef CHIS_PARALLEL_KILL
			if(vctf_search(b, true)) {
				ptb[b.hash_value()] = { 9, NEGA_LOS };
#ifdef CHIS_DEBUG
				++vct_cnt;
#endif
				return NEGA_LOS;
			}

#endif
			ptb[b.hash_value()] = { (SEARCH_DEPTH - depth), -max_v };
		}
		else if(max_v >= beta) {//beta节点
			pvs[b.hash_value()] = pv_point;
#ifdef CHIS_DEBUG
			++pvs_cnt;
#endif
		}
		else {
			if(pvs.find(b.hash_value()) != pvs.end()) {//不是beta节点了，删除主要变例
				pvs.erase(b.hash_value());
			}
		}
	}
	return -max_v;
}
Point chis::chis_move(Board &b) {
	search_time = time();//开始搜索的时间
	stop_insert = false;//启用置换表
	handle = GetCurrentProcess();
	bool cl = (b.move_count() % 2);//0代表返回最大，1代表返回最小
	if(!b.move_count()) {
		return Point(SIZE / 2, SIZE / 2);
	}
	std::vector<_point_with_value> moves;
	std::vector<_point_with_value> good_points;
	int ps[30][30] = {};
	get_move_in_root(ps, b);
	for(int i = 0; i < SIZE; ++i) {
		for(int j = 0; j < SIZE; ++j) {
			if(ps[i][j] >= 2) {
				moves.push_back(_point_with_value(Point(i, j), b.point_eval(i, j, cl ? -1 : 1)));
			}
		}
	}
	safe_prune(b, moves);
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
	else if(time_left >= SEARCH_TIME) {
		//还原设置
		CHIS_CONFIG.override_config();
	}
	else if(time_left <= SEARCH_TIME) {
		SEARCH_TIME = time_left - 1795;
	}
	for(SEARCH_DEPTH = 2; (SEARCH_DEPTH <= MAX_DEPTH); ++SEARCH_DEPTH) {
		
		GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));//获取内存占用信息
		if(pmc.WorkingSetSize / (1024.0 * 1024) >= HASH_SIZE) {
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
			b.set(p.x, p.y, cl ? -1 : 1);
			int child_value;// = moves[i].value = max_min_search(b, NEGA_LOS, -alpha);
			if(alpha == NEGA_LOS) {
				child_value = moves[i].value = max_min_search(b, NEGA_LOS, -alpha);
			}
			else {
				child_value = moves[i].value = max_min_search(b, -alpha - 1, -alpha);
				if(child_value >= alpha && child_value != NEGA_WON) {
					child_value = moves[i].value = max_min_search(b, NEGA_LOS, -alpha);
				}
			}
			b.set(p.x, p.y, 0);
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
					vct_cnt ? 0 : vct_cnt = b.move_count();
					return good_point;
				}
#endif
			}
		}
#ifndef CHIS_DEBUG
		if(alpha == NEGA_WON) {
			break;
		}
#endif
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
///////////////////////////////可靠的剪枝算法//////////////////////////////
//安全剪枝：不影响结果准确性的情况下，加快搜索速度（待优化）
void chis::safe_prune(Board &b, std::vector<_point_with_value> &moves) {
	int cl = b.move_count() % 2;
	int mec = cl ? -1 : 1;
	int oppc = -mec;
	const Patterns &me = cl ? b.white_patterns() : b.black_patterns();
	const Patterns &opp = cl ? b.black_patterns() : b.white_patterns();
	std::vector<_point_with_value> moves_ped;
	if(!me.four_b && !me.four_l) {//己方没四
		if(opp.four_b) {//对方有四
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//模拟己方落子,返回对方棋型
				if(!pat.four_b) {//对方减少冲四的点
					moves_ped.push_back(i);
				}
			}
		}
		else if(me.three_l) {//对方没有四，己方有活三,可胜
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//模拟己方落子，返回己方棋型
				if(pat.four_l) {//己方增加活四的点
					moves_ped.push_back(i);
				}
			}
		}
		else if(opp.three_l) {//对方有活三
			//TODO XX__OO_O__XX 特判
			if(me.three_s) {//己方有眠三，可冲可挡
				for(auto&i : moves) {
					Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//模拟己方落子，返回对方棋型
					if(pat.three_l < opp.three_l) {//对方减少活三的点
						moves_ped.push_back(i);
					}
					else {
						Patterns &&pat1 = b.get_update(i.first.x, i.first.y, mec);//模拟己方落子，返回己方棋型
						if(pat1.four_b) {//己方增加冲四的点
							moves_ped.push_back(i);
						}
					}
				}
			}
			else {//己方没有眠三，可档
				for(auto&i : moves) {
					Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//模拟己方落子，返回对方棋型
					if(pat.three_l < opp.three_l) {//对方减少活三的点
						moves_ped.push_back(i);
					}
				}
			}
		}
		else if((!me.two_l && !me.three_s) && (!opp.two_l && !opp.three_s)) {//双方没有活棋，做棋
			bool hasbest = false;
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//模拟己方落子，返回己方棋型
				if(pat.two_l > me.two_l || pat.three_s > me.three_s) {//
					moves_ped.push_back(i);
					moves.back().value += (pat.two_l + pat.three_s)*default_para.two_l;
				}
			}
		}
	}
	else {//己方有四，可胜
		for(auto&i : moves) {
			Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//模拟己方落子，返回己方棋型
			if(pat.wined) {//己方胜利点
				moves_ped.push_back(i);
			}
		}
	}
	if(!moves_ped.empty()) {
		moves = moves_ped;
	}
}