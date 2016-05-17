#include "chis_move.h"
#include "gomoku_board.h"
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
//#define CHIS_PROFILE
//#define CHIS_PARALLEL_KILL
//#define CHIS_VCT_EXP
//#define CHIS_EXP
#define NEGA_WON (INT_MAX - 1)
#define NEGA_LOS (INT_MIN + 2)
#define HASH_GAMEOVER (char)(0)
#define HASH_ALPHA (char)(1)
#define HASH_PV (char)(2)
#define HASH_BETA (char)(3)
using namespace chis;

////////////////////////////////��������/////////////////////////////////
size_t chis::time() {
	SYSTEMTIME t;
	GetLocalTime(&t);
	return (t.wMinute * 60 + t.wSecond) * 1000 + t.wMilliseconds;
}
void chis::get_move_in_root(int(&ps)[30][30], Board &b) {
	int cl = (b.move_count() % 2 ? -1 : 1);

	for(int i = 0; i < SIZE; ++i) {
		for(int j = 0; j < SIZE; ++j) {
			if(b[i][j]) {//�Һ�ѡ��
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
/////////////////////////////VCF/VCT/VC2���//////////////////////////////
//ͨͨ������
//vcf��֦����
void vcf_prune(Board &b, std::vector<_point_with_value> &moves) {
	int cl = b.move_count() % 2;
	int mec = cl ? -1 : 1;
	int oppc = -mec;
	const Patterns &me = cl ? b.white_patterns() : b.black_patterns();
	const Patterns &opp = cl ? b.black_patterns() : b.white_patterns();
	std::vector<_point_with_value> moves_ped;
	if(!me.four_b && !me.four_l) {//����û��
		if(opp.four_b) {//�Է�����
		}
		else if(me.three_l) {//�Է�û���ģ������л���,��ʤ
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//ģ�⼺�����ӣ����ؼ�������
				if(pat.four_l) {//�������ӻ��ĵĵ�
					moves_ped.push_back(i);
				}
			}
		}
		else {//ʲô���Ҳ����,VCF��֦
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//ģ�⼺������
				//�г��ģ�ͬʱ��֤������һ���Ľ���
				if(pat.four_b && pat.three_s) {
					moves_ped.push_back(i);
				}
			}
		}
	}
	else {//�������ģ���ʤ
		for(auto&i : moves) {
			Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//ģ�⼺�����ӣ����ؼ�������
			if(pat.wined) {//����ʤ����
				moves_ped.push_back(i);
			}
		}
	}
	moves = moves_ped;
}
//vct��֦����
void vct_prune(Board &b, std::vector<_point_with_value> &moves) {
	int cl = b.move_count() % 2;
	int mec = cl ? -1 : 1;
	int oppc = -mec;
	const Patterns &me = cl ? b.white_patterns() : b.black_patterns();
	const Patterns &opp = cl ? b.black_patterns() : b.white_patterns();
	std::vector<_point_with_value> moves_ped;
	if(!me.four_b && !me.four_l) {//����û��
		if(opp.four_b) {//�Է�����
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//ģ�⼺������,���ضԷ�����
				Patterns &&pat2 = b.get_update(i.first.x, i.first.y, mec);//ģ�⼺������
				if(!pat.four_b) {//�Է����ٳ���
					moves_ped.push_back(i);
				}
			}
		}
		else if(me.three_l) {//�Է�û���ģ������л���,��ʤ
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//ģ�⼺�����ӣ����ؼ�������
				if(pat.four_l) {//�������ӻ��ĵĵ�
					moves_ped.push_back(i);
				}
			}
		}
		else if(opp.three_l) {//�Է��л���
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//ģ�⼺�����ӣ����ضԷ�����
				Patterns &&pat2 = b.get_update(i.first.x, i.first.y, mec);//ģ�⼺�����ӣ����ؼ�������
				if(((pat.three_l < opp.three_l && pat2.three_l) || pat2.four_b)//�Է����ٻ���,���߼�������
					&& (pat2.three_l + pat2.four_b + pat2.three_s + pat2.two_l) >= 2) {//��������һ������
					moves_ped.push_back(i);
				}
			}
		}
		else {//ʲô���Ҳ����,VCT��֦
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//ģ�⼺������
				//�г��Ļ�����ͬʱ��֤������һ���Ľ���
				if((pat.three_l || pat.four_b) && (pat.three_l + pat.four_b + pat.three_s + pat.two_l) >= 2) {
					moves_ped.push_back(i);
				}
			}
		}
	}
	else {//�������ģ���ʤ
		for(auto&i : moves) {
			Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//ģ�⼺�����ӣ����ؼ�������
			if(pat.wined) {//����ʤ����
				moves_ped.push_back(i);
			}
		}
	}
	moves = moves_ped;
}
//���ط��ļ�֦����
void def_prune(Board &b, std::vector<_point_with_value> &moves) {
	int cl = b.move_count() % 2;
	int mec = cl ? -1 : 1;
	int oppc = -mec;
	const Patterns &me = cl ? b.white_patterns() : b.black_patterns();
	const Patterns &opp = cl ? b.black_patterns() : b.white_patterns();
	std::vector<_point_with_value> moves_ped;
	if(!me.four_b && !me.four_l) {//����û��
		if(opp.four_b) {//�Է�����
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//ģ�⼺������,���ضԷ�����
				if(!pat.four_b) {//�Է����ٳ��ĵĵ�
					moves_ped.push_back(i);
				}
			}
		}
		else if(me.three_l) {//�Է�û���ģ������л���,��ʤ

		}
		else if(opp.three_l) {//�Է��л���
			//TODO XX__OO_O__XX ����
			if(me.three_s) {//�������������ɳ�ɵ�
				for(auto&i : moves) {
					Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//ģ�⼺�����ӣ����ضԷ�����
					if(pat.three_l < opp.three_l) {//�Է����ٻ����ĵ�
						moves_ped.push_back(i);
					}
					else {
						Patterns &&pat1 = b.get_update(i.first.x, i.first.y, mec);//ģ�⼺�����ӣ����ؼ�������
						if(pat1.four_b) {//�������ӳ��ĵĵ�
							moves_ped.push_back(i);
						}
					}
				}
			}
			else {//����û���������ɵ�
				for(auto&i : moves) {
					Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//ģ�⼺�����ӣ����ضԷ�����
					if(pat.three_l < opp.three_l) {//�Է����ٻ����ĵ�
						moves_ped.push_back(i);
					}
				}
			}
		}
	}
	moves = moves_ped;
}

bool vctf_search(Board &b, bool p) {
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
	//��ν��������ԣ���nps�õ��Ż�ǰ������
	if(p && vct_depth < VCT_DEPTH + VC2_DEPTH && VC_DEPTH < 15) {//��VCT��Χ��
		const Patterns &opp = b.move_count() % 2 ? b.black_patterns() : b.white_patterns();
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
	int color = b.turn_color();//0���������1��������С
	for(int i = 0; i < chis::SIZE; ++i) {
		for(int j = 0; j < chis::SIZE; ++j) {
			if(b.ps[i][j] >= 2 && !b[i][j]) {
				moves.push_back(_point_with_value(Point(i, j), 0));
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
	for(auto &i : moves) {
		i.value = b.try_set_nega(i.first.x, i.first.y, color);
	}
	std::sort(moves.begin(), moves.end());
	bool pn = !p;//��ɱ����0���㣬���ط���1����
	for(auto&i : moves) {
		const Point &po = i.first;
		b.move(po.x, po.y);
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
int chis::max_min_search(Board &b, int alpha, int beta, char ply) {
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
	if(!(time() % 50)) {
		GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));//��ȡ�ڴ�ռ����Ϣ
		if(pmc.WorkingSetSize >= HASH_SIZE * (1024 * 1024)) {
			for(int i = 0; i < 400; ++i) {
				mtb[i].clear();
			}
		}
	}
	//�վּ�֦
	if(b.have_winner()) {//�ϼ��Ѿ�Ӯ��
		ptb[b.hash_value()] = { HASH_GAMEOVER, (char)99, NEGA_LOS };
		return NEGA_LOS;
	}
	//���
	if(ply <= 0) {
		int v = b.nega_value();
#ifdef CHIS_DEBUG
		++eval_cnt;
#endif
#ifdef CHIS_VCT

		if(v > -beta && v < -alpha) {//���ܱ�ѡ�е������
			if(allow_findvct && vctf_search(b, true)) {//��ɱ����������Ϊopp��
#ifdef CHIS_DEBUG
				++vct_cnt;
#endif
				ptb[b.hash_value()] = { HASH_GAMEOVER, (char)99, NEGA_WON };
				return NEGA_WON;//
			}
#ifdef CHIS_EXP
			if(allow_expand) {
				const Patterns &who = b.turn_color() == 1 ? b.white_patterns() : b.black_patterns();
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
	int color = b.turn_color();//0���������1��������С
	if(mtb[b.move_count()].find(b.hash_value()) == mtb[b.move_count()].end()) {
		for(char i = 0; i < SIZE; ++i) {
			for(char j = 0; j < SIZE; ++j) {
				if(b.ps[i][j] >= 2 && !b[i][j]) {
					moves.push_back(_point_with_value({ i, j }, 0));
				}
			}
		}
		safe_prune(b, moves);
		for(auto &i : moves) {
			i.value = b.point_eval(i.first.x, i.first.y, color);
		}
		std::sort(moves.begin(), moves.end());//����
		mtb[b.move_count()][b.hash_value()] = moves;
	}
	else {
		moves = mtb[b.move_count()][b.hash_value()];
		if(pvs.find(b.hash_value()) != pvs.end()) {
			for(auto &i : moves) {
				if(i.first == pvs[b.hash_value()]) {//�������Ҫ��������һ������ֵ
					i.value = 10000;
					break;
				}
			}
			std::sort(moves.begin(), moves.end());
		}
	}
	int _alpha = -alpha;
	int _beta = -beta;
	int max_v = NEGA_LOS;//�����û������ʵ��ֵ
	Point pv_point;

	for(auto &i : moves) {
		const Point &p = i.first;
		b.move(p.x, p.y);
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
			ptb[b.hash_value()] = { HASH_GAMEOVER, (char)99, max_v };//���ղ������
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
Point chis::chis_move(Board &b) {
	search_time = time();//��ʼ������ʱ��
	stop_insert = false;//�����û���
	handle = GetCurrentProcess();
	bool cl = (b.move_count() % 2);//0���������1��������С
	if(!b.move_count()) {
		return Point(SIZE / 2, SIZE / 2);
	}
	for(int i = 0; i <= b.move_count() + 3; ++i) {
		mtb[i].clear();
	}
	std::vector<_point_with_value> moves;
	int ps[30][30] = {};
	get_move_in_root(ps, b);
	for(char i = 0; i < SIZE; ++i) {
		for(char j = 0; j < SIZE; ++j) {
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
		if(b.move_count() <= 5) {
			MAX_DEPTH = 9;
		}
		else if(b.move_count() <= 15) {
			MAX_DEPTH = 8;
		}
	}
	
	for(SEARCH_DEPTH = 2; (SEARCH_DEPTH <= MAX_DEPTH); ++SEARCH_DEPTH) {
		
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
			b.move(p.x, p.y);
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
					vct_cnt ? 0 : vct_cnt = b.move_count();
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
///////////////////////////////�ɿ��ļ�֦�㷨//////////////////////////////
//��ȫ��֦����Ӱ����׼ȷ�Ե�����£��ӿ������ٶȣ����Ż���
void chis::safe_prune(Board &b, std::vector<_point_with_value> &moves) {
	int cl = b.move_count() % 2;
	int mec = cl ? -1 : 1;
	int oppc = -mec;
	const Patterns &me = cl ? b.white_patterns() : b.black_patterns();
	const Patterns &opp = cl ? b.black_patterns() : b.white_patterns();
	std::vector<_point_with_value> moves_ped;
	if(!me.four_b && !me.four_l) {//����û��
		if(opp.four_b) {//�Է�����
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//ģ�⼺������,���ضԷ�����
				if(!pat.four_b) {//�Է����ٳ��ĵĵ�
					moves_ped.push_back(i);
				}
			}
		}
		else if(me.three_l) {//�Է�û���ģ������л���,��ʤ
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//ģ�⼺�����ӣ����ؼ�������
				if(pat.four_l) {//�������ӻ��ĵĵ�
					moves_ped.push_back(i);
				}
			}
		}
		else if(opp.three_l) {//�Է��л���
			if(me.three_s) {//�������������ɳ�ɵ�
				for(auto&i : moves) {
					Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//ģ�⼺�����ӣ����ضԷ�����
					if(pat.three_l < opp.three_l) {//�Է����ٻ����ĵ�
						moves_ped.push_back(i);
					}
					else {
						Patterns &&pat1 = b.get_update(i.first.x, i.first.y, mec);//ģ�⼺�����ӣ����ؼ�������
						if(pat1.four_b) {//�������ӳ��ĵĵ�
							moves_ped.push_back(i);
						}
					}
				}
			}
			else {//����û���������ɵ�
				for(auto&i : moves) {
					Patterns &&pat = b.get_update(i.first.x, i.first.y, mec, oppc);//ģ�⼺�����ӣ����ضԷ�����
					if(pat.three_l < opp.three_l) {//�Է����ٻ����ĵ�
						moves_ped.push_back(i);
					}
				}
			}
		}
		else if((!me.two_l && !me.three_s) && (!opp.two_l && !opp.three_s)) {//˫��û�л��壬����
			for(auto&i : moves) {
				Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//ģ�⼺�����ӣ����ؼ�������
				if(pat.two_l > me.two_l || pat.three_s > me.three_s) {//
					moves_ped.push_back(i);
				}
			}
		}
		
	}
	else {//�������ģ���ʤ
		for(auto&i : moves) {
			Patterns &&pat = b.get_update(i.first.x, i.first.y, mec);//ģ�⼺�����ӣ����ؼ�������
			if(pat.wined) {//����ʤ����
				moves_ped.push_back(i);
			}
		}
	}
	if(!moves_ped.empty()) {
		//moves = moves_ped;
		swap(moves, moves_ped);
	}
}