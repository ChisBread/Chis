#include "chis_move.h"
#include "chis_board.h"
#include <algorithm>
#include <cstdlib>
#include <time.h>
#include <iostream>
#pragma comment(lib,"psapi.lib") 
/////////////���뿪��////////////
//#define CHIS_DEBUG //debug ����
//#define CHIS_DEFEND //ѡ�����
//#define CHIS_EXP
#define CHIS_VCT
//#define CHIS_VCT_EXP
//#define CHIS_EXP
#define NEGA_WON (INT_MAX - 1)
#define NEGA_LOS (INT_MIN + 2)
#define HASH_GAMEOVER (U8)(0)
#define HASH_ALPHA (U8)(1)
#define HASH_PV (U8)(2)
#define HASH_BETA (U8)(3)
using namespace chis;

////////////////////////////////��������/////////////////////////////////
time_t chis::time() {
	SYSTEMTIME t;
	GetLocalTime(&t);
	return (t.wMinute * 60 + t.wSecond) * 1000 + t.wMilliseconds;
}
/////////////////////////////VCF/VCT/VC2���//////////////////////////////
//ͨͨ������
//vcf��֦����
bool vctf_search(_board &b, bool p) {
	//�ɼӿɲ��ӵ��û�
	if(ptb.find(b.hash_value()) != ptb.end() && ptb[b.hash_value()].hash_flag == HASH_GAMEOVER) {
		if(p) {//��ɱ��
			if(ptb[b.hash_value()].value == NEGA_WON) {//�ϼ��䣬��ɱ�ɹ�
#ifdef CHIS_DEBUG
				++hashfinded;
#endif
				return true;
			}
			else {
				return false;
			}
		}
		else {//���ط�
			if(ptb[b.hash_value()].value == NEGA_LOS) {//�ϼ�Ӯ����ɱ�ɹ�
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
		if(p) {//��ɱ����ɱ��.....
			return false;
		}
		return true;//��ɱ�ɹ�
	}
	//ʱ�����
	if(time() - search_time >= SEARCH_TIME || (time_left - (time() - search_time) <= 200)) {
		return false;
	}
#ifdef CHIS_VCT_EXP
	//������ԣ���nps�õ��Ż�ǰ������
	if(p && vc_depth < VCT_DEPTH + VC2_DEPTH && VC_DEPTH < 15) {//��VCT��Χ��
		const _pattern &opp = b.move_count() % 2 ? b.black__pattern() : b.white__pattern();
		if(opp.four_b) {//����Է�����
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
	bool pn = !p;//��ɱ����0���㣬���ط���1����
	for(auto&i : moves) {
		const Point &po = i.first;
		b.make_move({ po.x, po.y }, b.get_turn());
		++vc_depth;
		if(p) {//��ɱ����OR
			pn = pn || vctf_search(b, !p);
			--vc_depth;
			b.unmove();
			if(pn) {
				break;//�������ɱ����һ��ɱ��������ɱ�ɹ�
			}
		}
		else {//���ط���AND
			pn = pn && vctf_search(b, !p);
			--vc_depth;
			b.unmove();
			if(!pn) {
				break;//������ط���һ���������ɱʧ��
			}
		}

	}
#ifdef CHIS_DEBUG
	++node_count;
#endif
	if(pn) {//�㵽��ɱ
		ptb[b.hash_value()] = { HASH_GAMEOVER ,99, p ? NEGA_WON : NEGA_LOS };//p?WON�� !p?LOS
	}
	return pn;
}
//////////////////////////////�����㷨/////////////////////////////////////
//������������
//////////////////////////////�����㷨/////////////////////////////////////
int chis::max_min_search(_board &b, int alpha, int beta, U8 ply) {
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
		GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));//��ȡ�ڴ�ռ����Ϣ
		if(pmc.WorkingSetSize >= HASH_SIZE * (1024 * 1024)) {
			for(int i = 0; i < 400; ++i) {
				mtb[i].clear();
			}
			pvs.clear();
			ptb.clear();
		}
	}
	//�վּ�֦
	if(b.have_winner()) {//�ϼ��Ѿ�Ӯ��
		ptb[b.hash_value()] = { HASH_GAMEOVER, (U8)99, NEGA_LOS };
		return NEGA_LOS;
	}
	//���
	if(ply <= 0) {
		int v = b.evaluation();
#ifdef CHIS_DEBUG
		++eval_cnt;
#endif
#ifdef CHIS_VCT

		if(v > -beta && v < -alpha) {//���ܱ�ѡ�е������
			if(allow_findvct && vctf_search(b, true)) {//��ɱ����������Ϊopp��
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
					//����/�������죬ƽ���������
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
	if(mtb[b.moves_size()].find(b.hash_value()) == mtb[b.moves_size()].end()) {
		std::swap(moves, (b.get_turn() == BLK ?
			b.get_pruned_moves_black() : b.get_pruned_moves_white()));
		mtb[b.moves_size()][b.hash_value()] = moves;
	}
	else {
		moves = mtb[b.moves_size()][b.hash_value()];
		if(pvs.find(b.hash_value()) != pvs.end()) {
			for(int i = 0; i < moves.size(); ++i) {
				if(moves[i].first == pvs[b.hash_value()]) {//�������Ҫ����
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
	//Ѱ��bug case
	/*if(moves.empty()) {
		for(auto &i : b.get_moves()) {
			std::cout << (int)(i.first.x-5) << ", " << (int)(i.first.y-5) << ", 0" << std::endl;
		}
		system("PAUSE");
	}*/
	int _alpha = -alpha;
	int _beta = -beta;
	int max_v = NEGA_LOS;//�����û������ʵ��ֵ
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
				if(alpha >= beta) {//m��������last_mȡС����֦
#ifdef CHIS_DEBUG
					++ab_count;
#endif
					break;
				}
			}
		}
		if(time() - search_time >= SEARCH_TIME || (time_left - (time() - search_time) <= 200)) {//����ʱ�䵽����ʣ�µ�ʱ�䲻��200ms
			stop_insert = true;
			return max_v;
		}
		
	}

#ifdef CHIS_DEBUG
	++node_count;
#endif
	if(!stop_insert) {//�Ƿ����hash��
#ifdef CHIS_DEBUG
		++hashinsert;
#endif
		if(max_v == NEGA_LOS || max_v == NEGA_WON) {//ɱ��
			ptb[b.hash_value()] = { HASH_GAMEOVER, (U8)99, max_v };//���ղ������
		}
		else if(max_v >= beta) {//beta�ڵ�
			ptb[b.hash_value()] = { HASH_BETA, (ply), max_v };
			//������Ҫ����
			pvs[b.hash_value()] = pv_point;
#ifdef CHIS_DEBUG
			++pvs_cnt;
#endif
		}
		else if(max_v < alpha) {//alpha�ڵ�
			ptb[b.hash_value()] = { HASH_ALPHA, (ply), max_v };
			if(pvs.find(b.hash_value()) != pvs.end()) {//����beta�ڵ��ˣ�ɾ����Ҫ����
				pvs.erase(b.hash_value());
			}
		}
		else {//PV�ڵ�
			ptb[b.hash_value()] = {HASH_PV, (ply), max_v };
		}
		
	}
	else if(max_v == NEGA_LOS || max_v == NEGA_WON) {//ɱ��
		ptb[b.hash_value()] = { HASH_GAMEOVER, 99, max_v };//���ղ������
	}
	return max_v;
}
Point chis::chis_move(_board &b) {
	search_time = time();//��ʼ������ʱ��
	stop_insert = false;//�����û���
	handle = GetCurrentProcess();
	
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
		//��ʱ��������������Ҫ��
		SEARCH_TIME = 125;
		MAX_DEPTH = 4;
		MAX_P = 8;
	}
	else if(time_left <= SEARCH_TIME) {
		SEARCH_TIME = time_left - 1795;
	}
	else {
		//��ԭ����
		
		CHIS_CONFIG.override_config();
		if(b.moves_size() <= 5) {
			MAX_DEPTH = 9;
		}
		else if(b.moves_size() <= 15) {
			MAX_DEPTH = 8;
		}
	}
	for(SEARCH_DEPTH = 2; (SEARCH_DEPTH <= MAX_DEPTH) ; ++SEARCH_DEPTH) {
		
		GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));//��ȡ�ڴ�ռ����Ϣ
		if(pmc.WorkingSetSize >= HASH_SIZE * (1024.0 * 1024)) {
			for(int i = 0; i < 400; ++i) {
				mtb[i].clear();
			}
			clear_hash();
		}
		alpha = NEGA_LOS;
		std::sort(moves.begin(), moves.end());
		////////////////////////////////////////////
		if(time_left <= 200) {//��ʱʵ�ڲ�������̬��ֵ
			return moves[0].first;
		}
		//�ü��ذܵ�
		while(moves.size() > 1 && moves.back().value == NEGA_LOS) {
#ifdef CHIS_DEBUG
			++pruning_move_count;
#endif
			moves.pop_back();

		}
		////////////////////////////////////////////
		//�ü����һ������
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
		//����б�ʤ�㣬�޳���ʤ������ĵ㡣
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
		//	//���Ĭ�ϲ�ʱ����5000
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