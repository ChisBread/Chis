#include "chis_board.h"
#include <iostream>
#include <cstdlib>
#define PAT_PATCH //棋型bug补丁
//#define SPACE_SCORE //空间分
//#define HUM_PRUNE //有害剪枝
#define PATVAL_B(x) (chs_point * x.first)
#define PATVAL_W(x) (chs_point * x.second)
#define PVAL_B(x) (chs_point * x.first + (chs_point * x.second) / 2)
#define PVAL_W(x) (chs_point * x.second + (chs_point * x.first) / 2)
#define SSF1 (4)
#define SSF2 (SSF1 << 1)
namespace chis {

	patterns_map::patterns_map() {
		U8 board[11] = {};
		for(U32 i = 0; i < (1 << 22); ++i) {
			U32 temp = i;
			for(int j = 10; j >= 0; --j) {
				board[j] = temp % 4;
				temp /= 4;
			}
			U16 key = 1;
			U8 ans = 0;
			U8 *l = board + 5, *r = board + 5;
			//1.各边长度限制
			for(int len = 0, spc = 0; l >= board && len < 6 && *l != WHI && *l != SID && spc < 4; ++len, --l) {
				key = key << 1;
				if(*l == BLK) {
					++key;
				}
				if(*l == EMP) {
					++spc;
				}
				else {
					spc = 0;
				}
			}
			key = pattern_rev[key];
			for(int len = 0, spc = 0; r < board + 11 && len < 6 && *r != WHI && *r != SID && spc < 4; ++len, ++r) {
				key = key << 1;
				if(*r == BLK) {
					++key;
				}
				if(*r == EMP) {
					++spc;
				}
				else {
					spc = 0;
				}
			}
			ans += pattern_map[key];
			key = 1;
			r = l = board + 5;
			for(int len = 0, spc = 0; l >= board && len < 6 && *l != BLK && *l != SID&& spc < 4; ++len, --l) {
				key = key << 1;
				if(*l == WHI) {
					++key;
				}
				if(*l == EMP) {
					++spc;
				}
				else {
					spc = 0;
				}
			}
			key = pattern_rev[key];
			for(int len = 0, spc = 0; r < board + 11 && *r != BLK && *r != SID && len < 6 && spc < 4; ++len, ++r) {
				key = key << 1;
				if(*r == WHI) {
					++key;
				}
				if(*r == EMP) {
					++spc;
				}
				else {
					spc = 0;
				}
			}
			ans += pattern_map[key] << 4;
			_map[i] = ans;
		}
	}
	zobrist_key::zobrist_key() {
		for(int i = 0; i < 64; ++i) {
			for(int j = 0; j < 64; ++j) {
				_rand[i][j][0] = rand64();
				_rand[i][j][1] = rand64();
				_rand[i][j][2] = rand64();
				_rand[i][j][3] = rand64();
			}
		}
	}
	_board::_board(size_t s)
		:board_size(s), pcode(), pat(), end_index(s + begin_index), _size(s + 10){
		pat_moves.push_back(pat);
		ss_moves.push_back(ss);
		for(U64 i = 0; i < _size; ++i) {
			for(U64 j = 0; j < _size; ++j) {
				//init board
				if(i < begin_index || i >= end_index
					|| j < begin_index || j >= end_index) {
					board[i][j] = SID;
				}
				else {
					board[i][j] = EMP;

				}
				//init pcode
				board_itor *itor = new board_itor(*this, i, j);
				((board_itor_1*)itor)->xor_with_pcode();
				((board_itor_2*)itor)->xor_with_pcode();
				((board_itor_3*)itor)->xor_with_pcode();
				((board_itor_4*)itor)->xor_with_pcode();
				delete itor;
				//init cand
				cand[i][j].p = { (U8)i, (U8)j };
				cand[i][j].ppc = get_patcode(i, j);
				weight[i][j] = 0;
				synced[i][j] = false;
			}
		}

	}
	void _board::make_move(Point p, U64 c) {
		turn ^= SID;
		moves.push_back({ p, c });
		hash_key ^= zob[p.x][p.y][c];
		board_itor *itor = new board_itor(*this, p.x, p.y);
		//hold pat
		//before
		/*if(moves.size() == 36) {
			int i = 0;
			++i;
		}*/
		U8 pat_index[4];
		U8 pat_index_2[12];
		pat_index[0] = pmap[((board_itor_1*)(itor))->get_pcode()];
		pat_index[1] = pmap[((board_itor_2*)(itor))->get_pcode()];
		pat_index[2] = pmap[((board_itor_3*)(itor))->get_pcode()];
		pat_index[3] = pmap[((board_itor_4*)(itor))->get_pcode()];

		for(int i = 0; i < 4; ++i) {
			--((char*)(&pat.first))[pat_index[i] & 15];
			--((char*)(&pat.second))[(pat_index[i] >> 4)];
		}
		//hold pcode
		board[p.x][p.y] = c;
		((board_itor_1*)itor)->xor_with_pcode();
		((board_itor_2*)itor)->xor_with_pcode();
		((board_itor_3*)itor)->xor_with_pcode();
		((board_itor_4*)itor)->xor_with_pcode();
		
		//point weight
		weight[p.x - 1][p.y] += 2;
		weight[p.x - 2][p.y] += 1;
		weight[p.x + 1][p.y] += 2;
		weight[p.x + 2][p.y] += 1;

		weight[p.x][p.y - 1] += 2;
		weight[p.x][p.y - 2] += 1;
		weight[p.x][p.y + 1] += 2;
		weight[p.x][p.y + 2] += 1;

		weight[p.x - 1][p.y - 1] += 2;
		weight[p.x - 2][p.y - 2] += 1;
		weight[p.x + 1][p.y + 1] += 2;
		weight[p.x + 2][p.y + 2] += 1;

		weight[p.x - 1][p.y + 1] += 2;
		weight[p.x - 2][p.y + 2] += 1;
		weight[p.x + 1][p.y - 1] += 2;
		weight[p.x + 2][p.y - 2] += 1;
		for(int i = 1; i <= 5; ++i) {
			synced[p.x][p.y + i] = false;
			synced[p.x + i][p.y] = false;
			synced[p.x + i][p.y + i] = false;
			synced[p.x + i][p.y - i] = false;

			synced[p.x][p.y - i] = false;
			synced[p.x - i][p.y] = false;
			synced[p.x - i][p.y - i] = false;
			synced[p.x - i][p.y + i] = false;
		}
		//after
		pat_index_2[0] = pmap[((board_itor_1*)(itor))->get_pcode()];
		pat_index_2[1] = pmap[((board_itor_2*)(itor))->get_pcode()];
		pat_index_2[2] = pmap[((board_itor_3*)(itor))->get_pcode()];
		pat_index_2[3] = pmap[((board_itor_4*)(itor))->get_pcode()];

		pat_index_2[4] = pmap[board_itor_1(*this, p.x, p.y - 1).get_pcode()];
		pat_index_2[5] = pmap[board_itor_1(*this, p.x, p.y + 1).get_pcode()];
		pat_index_2[6] = pmap[board_itor_2(*this, p.x - 1, p.y).get_pcode()];
		pat_index_2[7] = pmap[board_itor_2(*this, p.x + 1, p.y).get_pcode()];
		pat_index_2[8] = pmap[board_itor_3(*this, p.x + 1, p.y + 1).get_pcode()];
		pat_index_2[9] = pmap[board_itor_3(*this, p.x - 1, p.y - 1).get_pcode()];
		pat_index_2[10] = pmap[board_itor_4(*this, p.x + 1, p.y - 1).get_pcode()];
		pat_index_2[11] = pmap[board_itor_4(*this, p.x - 1, p.y + 1).get_pcode()];
		
		if(c == BLK) {
			for(int i = 0; i < 4; ++i) {
				++((char*)(&pat.first))[pat_index_2[i] & 15];
			}
			for(int i = 4; i < 12; ++i) {
				//特判:对手落子前是眠三，落子后是活三, 那么是误判
				if(((pat_index_2[i] >> 4)) == 5 && ((pat_index[(i - 4) / 2] >> 4)) == 4) {
					++((char*)(&pat.second))[4];
					continue;
				}
				++((char*)(&pat.second))[(pat_index_2[i] >> 4)];
			}
		}
		else if(c == WHI) {
			for(int i = 0; i < 4; ++i) {
				++((char*)(&pat.second))[(pat_index_2[i] >> 4)];
			}
			for(int i = 4; i < 12; ++i) {
				//特判:对手落子前是眠三，落子后是活三, 那么是误判
				if((pat_index_2[i] & 15) == 5 && (pat_index[(i - 4) / 2] & 15) == 4) {
					++((char*)(&pat.first))[4];
					continue;
				}
				++((char*)(&pat.first))[pat_index_2[i] & 15];
			}
		}
		pat_moves.push_back(pat);
		//space score
#ifdef SPACE_SCORE
		int ssc;
		auto  holdss2 = [&](int x, int y) {
			ssc = (board[x][y] + (SSF2-1)) & SSF2;//111 + 01(10/11) = 1XXX, 1XXX & 1000= 1000(8)
			ss[board[x][y]] -= ssc;
			ss[c] -= ssc;
		};
		auto  holdss1 = [&](int x, int y) {
			ssc = (board[x][y] + (SSF1-1)) & SSF1;//11 + 01(10/11) = 1XX, 1XX & 100= 100(4) 
			ss[board[x][y]] -= ssc;
			ss[c] -= ssc;
		};
		holdss1(p.x, p.y - 1);
		holdss1(p.x, p.y + 1);
		holdss1(p.x - 1, p.y);
		holdss1(p.x + 1, p.y);
		holdss1(p.x - 1, p.y - 1);
		holdss1(p.x + 1, p.y + 1);
		holdss1(p.x - 1, p.y + 1);
		holdss1(p.x + 1, p.y - 1);
		holdss2(p.x, p.y - 2);
		holdss2(p.x, p.y + 2);
		holdss2(p.x - 2, p.y);
		holdss2(p.x + 2, p.y);
		holdss2(p.x - 2, p.y - 2);
		holdss2(p.x + 2, p.y + 2);
		holdss2(p.x - 2, p.y + 2);
		holdss2(p.x + 2, p.y - 2);
		ss_moves.push_back(ss);
#endif
		delete itor;
	}
	void _board::unmove() {
		auto p = moves.back().first;
		auto c = moves.back().second;
		turn = c;
		moves.pop_back();
		hash_key ^= zob[p.x][p.y][c];
		//TODO: hold pcode
		board_itor *itor = new board_itor(*this, p.x, p.y);
		((board_itor_1*)itor)->xor_with_pcode();
		((board_itor_2*)itor)->xor_with_pcode();
		((board_itor_3*)itor)->xor_with_pcode();
		((board_itor_4*)itor)->xor_with_pcode();
		delete itor;
		board[p.x][p.y] = EMP;
		pat_moves.pop_back();
		pat = pat_moves.back();
#ifdef SPACE_SCORE
		ss_moves.pop_back();
		ss = ss_moves.back();
#endif
		//point weight
		weight[p.x - 1][p.y] -= 2;
		weight[p.x - 2][p.y] -= 1;
		weight[p.x + 1][p.y] -= 2;
		weight[p.x + 2][p.y] -= 1;

		weight[p.x][p.y - 1] -= 2;
		weight[p.x][p.y - 2] -= 1;
		weight[p.x][p.y + 1] -= 2;
		weight[p.x][p.y + 2] -= 1;

		weight[p.x - 1][p.y - 1] -= 2;
		weight[p.x - 2][p.y - 2] -= 1;
		weight[p.x + 1][p.y + 1] -= 2;
		weight[p.x + 2][p.y + 2] -= 1;

		weight[p.x - 1][p.y + 1] -= 2;
		weight[p.x - 2][p.y + 2] -= 1;
		weight[p.x + 1][p.y - 1] -= 2;
		weight[p.x + 2][p.y - 2] -= 1;

		for(int i = 1; i <= 5; ++i) {
			synced[p.x][p.y + i] = false;
			synced[p.x + i][p.y] = false;
			synced[p.x + i][p.y + i] = false;
			synced[p.x + i][p.y - i] = false;

			synced[p.x][p.y - i] = false;
			synced[p.x - i][p.y] = false;
			synced[p.x - i][p.y - i] = false;
			synced[p.x - i][p.y + i] = false;
		}
	}
	_point_pattern_code _board::get_patcode(U64 x, U64 y) {
		board_itor *itor = new board_itor(*this, x, y);
		_point_pattern_code pcode;
		pcode[0] = ((board_itor_1*)itor)->get_pcode();
		pcode[1] = ((board_itor_2*)itor)->get_pcode();
		pcode[2] = ((board_itor_3*)itor)->get_pcode();
		pcode[3] = ((board_itor_4*)itor)->get_pcode();
		delete itor;
		return pcode;
	}
	void _board::sync_candidate_root() {
		candidate_moves.clear();
		int weight[64][64] = { 0 };
		for(size_t i = begin_index; i < end_index; ++i) {
			for(size_t j = begin_index; j < end_index; ++j) {
				if(board[i][j]) {
					for(int k = 1, p = (board[i][j] == turn ? 3 : 2); p > 0; --p, ++k) {
						weight[i - k][j] += p;
						weight[i + k][j] += p;

						weight[i][j - k] += p;
						weight[i][j + k] += p;

						weight[i - k][j - k] += p;
						weight[i + k][j + k] += p;

						weight[i - k][j + k] += p;
						weight[i + k][j - k] += p;
					}
				}
			}
		}
		for(size_t i = begin_index; i < end_index; ++i) {
			for(size_t j = begin_index; j < end_index; ++j) {
				if(weight[i][j] >= 2 && !board[i][j]) {
					synced[i][j] = true;
					cand[i][j].ppc = get_patcode(i, j);
					cand[i][j].pat.reset();
					for(int k = 0; k < 4; ++k) {
						//first 落黑子以后的黑棋型
						++((char*)(&cand[i][j].pat.first))[pmap[cand[i][j].ppc[k] ^ (BLK << 10)] & 15];
						//second 落白子以后的白棋型
						++((char*)(&cand[i][j].pat.second))[(pmap[cand[i][j].ppc[k] ^ (WHI << 10)] >> 4)];
					}
					candidate_moves.push_back(cand[i][j]);
				}
			}
		}
	}
	void _board::sync_candidate() {
		candidate_moves.clear();
		for(size_t i = begin_index; i < end_index; ++i) {
			for(size_t j = begin_index; j < end_index; ++j) {
				if(weight[i][j] >= 2 && !board[i][j]) {
					if(synced[i][j] == false) {
						synced[i][j] = true;
						cand[i][j].ppc = get_patcode(i, j);
						cand[i][j].pat.reset();
						for(int k = 0; k < 4; ++k) {
							//first 落黑子以后的黑棋型
							++((char*)(&cand[i][j].pat.first))[pmap[cand[i][j].ppc[k] ^ (BLK << 10)] & 15];
							//second 落白子以后的白棋型
							++((char*)(&cand[i][j].pat.second))[(pmap[cand[i][j].ppc[k] ^ (WHI << 10)] >> 4)];
						}
					}
					candidate_moves.push_back(cand[i][j]);
				}
			}
		}
	}
	int _board::have_winner() {
		if(pat.first.won >= 1) {
			return BLK;
		}
		if(pat.second.won >= 1) {
			return WHI;
		}
		//必败判定
		//定义必败：
		//前提：己方未落子，且无成五。己方无四
		//0.对方已成活四
		//1.对方有双冲四
		//2.对方有双活三或者一个活三，一个冲四，己方无三
		if(moves.size() % 2) {
			if(!pat.second.four_b && !pat.second.four_l) {
				if(pat.first.four_l || pat.first.four_b >= 2 ||
					(!pat.second.three_l && !pat.second.three_s && pat.first.four_b + pat.first.three_l >= 2)) {
					return 1;
				}
			}
		}
		else {
			if(!pat.first.four_b && !pat.first.four_l) {
				if(pat.second.four_l || pat.second.four_b >= 2 ||
					(!pat.first.three_l && !pat.first.three_s && pat.second.four_b + pat.second.three_l >= 2)) {
					return -1;
				}
			}
		}
		return 0;
	}
	int _board::evaluation() {
#ifdef SPACE_SCORE
		return
			//最后落子的是黑色
			moves.back().second == BLK ?
			default_para*(pat.second - pat.first) + (ss[WHI]-ss[BLK]):
			default_para*(pat.first - pat.second) + (ss[BLK] - ss[WHI]);
#else
		return
			//最后落子的是黑色
			moves.back().second == BLK ?
			default_para*(pat.second - pat.first):
			default_para*(pat.first - pat.second);
#endif
	}
	std::vector<_point_with_value> _board::get_pruned_moves_black_root() {
		sync_candidate_root();
		std::vector<_point_with_value> pruned_moves;
		//己方有四
BEGIN:
		if(pat.first.four_l || pat.first.four_b) {
			//从己方候选点中找己方成五
			for(auto &i : candidate_moves) {
				if(i.pat.first.won) {
					pruned_moves.push_back({ i.p, PVAL_B(i.pat) });
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.first.four_b = pat.first.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方冲四（如果成四就已经死了）
		else if(pat.second.four_b) {
			//从对方候选点中找对方成五
			for(auto &i : candidate_moves) {
				if(i.pat.second.won) {
					pruned_moves.push_back({ i.p, PVAL_B(i.pat) });
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.second.four_b = pat.second.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//己方活三
		else if(pat.first.three_l) {
			//从己方候选点中找己方成活四
			for(auto &i : candidate_moves) {
				if(i.pat.first.four_l) {
					pruned_moves.push_back({ i.p, PVAL_B(i.pat) });
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成四点
			if(pruned_moves.empty()) {
				pat.first.three_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方活三
		else if(pat.second.three_l) {
			//从对方候选点中找对方成四 或者己方成冲四
#ifdef PAT_PATCH
			char exist4 = 0;
#endif
			for(auto &i : candidate_moves) {
				if(i.pat.second.four_l || i.pat.first.four_b) {
					pruned_moves.push_back({ i.p, PVAL_B(i.pat) });
#ifdef PAT_PATCH
					exist4 += i.pat.second.four_l;
#endif
					continue;
				}
				for(int k = 0; k < 4; ++k) {
					//对方落子前成活三
					if((pmap[i.ppc[k]] >> 4) == 5) {
						pruned_moves.push_back({ i.p, PVAL_B(i.pat) });
						break;
					}
				}
			}
#ifdef PAT_PATCH
			if(!exist4) {
				pat.second.three_l = 0;
				pat_moves.back() = pat;
				pruned_moves.clear();
				goto BEGIN;
			}
#endif
		}
		//双方没活棋
#ifdef HUM_PRUNE
		else if(!pat.first.two_l && !pat.second.two_l && !pat.first.three_s && !pat.second.three_s) {
			//从己方候选点中找能成二或眠三的
			for(auto &i : candidate_moves) {
				if(i.pat.first.two_l || i.pat.first.three_s) {
					pruned_moves.push_back({ i.p, PVAL_B(i.pat) });
				}
			}
		}
#endif
		//除此之外
		if(pruned_moves.empty()) {
			//复制己方候选点
			for(auto &i : candidate_moves) {
				pruned_moves.push_back({ i.p, PVAL_B(i.pat) });
			}
		}
		std::sort(pruned_moves.begin(), pruned_moves.end());
		return std::move(pruned_moves);
	}
	std::vector<_point_with_value> _board::get_pruned_moves_white_root() {
		sync_candidate_root();
		std::vector<_point_with_value> pruned_moves;
		//己方有四
BEGIN:
		if(pat.second.four_l || pat.second.four_b) {
			//从己方候选点中找己方成五
			for(auto &i : candidate_moves) {
				if(i.pat.second.won) {
					pruned_moves.push_back({ i.p, PVAL_W(i.pat) });
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.second.four_b = pat.second.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方冲四（如果成四就已经死了）
		else if(pat.first.four_b) {
			//从对方候选点中找对方成五
			for(auto &i : candidate_moves) {
				if(i.pat.first.won) {
					pruned_moves.push_back({ i.p, PVAL_W(i.pat) });
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.first.four_b = pat.first.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//己方活三
		else if(pat.second.three_l) {
			//从己方候选点中找己方成活四
			for(auto &i : candidate_moves) {
				if(i.pat.second.four_l) {
					pruned_moves.push_back({ i.p, PVAL_W(i.pat) });
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成四点
			if(pruned_moves.empty()) {
				pat.second.three_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方活三
		else if(pat.first.three_l) {
			//从对方候选点中找对方成四 或者己方成冲四
#ifdef PAT_PATCH
			//为空说明没有成五点
			char exist4 = 0;
#endif
			for(auto &i : candidate_moves) {
				if(i.pat.first.four_l || i.pat.second.four_b) {
					pruned_moves.push_back({ i.p, PVAL_W(i.pat) });
#ifdef PAT_PATCH
					exist4 += i.pat.first.four_l;
#endif
					continue;
				}
				for(int k = 0; k < 4; ++k) {
					//对方落子前成活三
					if((pmap[i.ppc[k]] & 15) == 5) {
						pruned_moves.push_back({ i.p, PVAL_W(i.pat) });
						break;
					}
				}
			}
#ifdef PAT_PATCH
			if(!exist4) {
				pat.first.three_l = 0;
				pat_moves.back() = pat;
				pruned_moves.clear();
				goto BEGIN;
			}
#endif
		}
		//双方没活棋
#ifdef HUM_PRUNE
		else if(!pat.second.two_l && !pat.first.two_l && !pat.second.three_s && !pat.first.three_s) {
			//从己方候选点中找能成二或眠三的
			for(auto &i : candidate_moves) {
				if(i.pat.second.two_l || i.pat.second.three_s) {
					pruned_moves.push_back({ i.p, PVAL_W(i.pat) });
				}
			}
		}
#endif
		//除此之外
		if(pruned_moves.empty()) {
			//复制己方候选点
			for(auto &i : candidate_moves) {
				pruned_moves.push_back({ i.p, PVAL_W(i.pat) });
			}
		}
		std::sort(pruned_moves.begin(), pruned_moves.end());
		return std::move(pruned_moves);
	}

	std::vector<_point_with_value> _board::get_pruned_moves_black() {
		sync_candidate();
		std::vector<_point_with_value> pruned_moves;
		//己方有四
BEGIN:
		if(pat.first.four_l || pat.first.four_b) {
			//从己方候选点中找己方成五
			for(auto &i : candidate_moves) {
				if(i.pat.first.won) {
					pruned_moves.push_back({i.p, PVAL_B(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.first.four_b = pat.first.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方冲四（如果成四就已经死了）
		else if(pat.second.four_b) {
			//从对方候选点中找对方成五
			for(auto &i : candidate_moves) {
				if(i.pat.second.won) {
					pruned_moves.push_back({i.p, PVAL_B(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.second.four_b = pat.second.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//己方活三
		else if(pat.first.three_l) {
			//从己方候选点中找己方成活四
			for(auto &i : candidate_moves) {
				if(i.pat.first.four_l) {
					pruned_moves.push_back({i.p, PVAL_B(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成四点
			if(pruned_moves.empty()) {
				pat.first.three_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方活三
		else if(pat.second.three_l) {
			//从对方候选点中找对方成四 或者己方成冲四
#ifdef PAT_PATCH
			char exist4 = 0;
#endif
			for(auto &i : candidate_moves) {
				if(i.pat.second.four_l || i.pat.first.four_b) {
					pruned_moves.push_back({ i.p, PVAL_B(i.pat) });
#ifdef PAT_PATCH
					exist4 += i.pat.second.four_l;
#endif
					continue;
				}
				for(int k = 0; k < 4; ++k) {
					//对方落子前成活三
					if(((pmap[i.ppc[k]] >> 4)) == 5) {
						pruned_moves.push_back({ i.p, PVAL_B(i.pat) });
						break;
					}
				}
			}
#ifdef PAT_PATCH
			if(!exist4) {
				pat.second.three_l = 0;
				pat_moves.back() = pat;
				pruned_moves.clear();
				goto BEGIN;
			}
#endif
		}
		//双方没活棋
#ifdef HUM_PRUNE
		else if(!pat.first.two_l && !pat.second.two_l && !pat.first.three_s && !pat.second.three_s) {
			//从己方候选点中找能成二或眠三的
			for(auto &i : candidate_moves) {
				if(i.pat.first.two_l || i.pat.first.three_s) {
					pruned_moves.push_back({i.p, PVAL_B(i.pat)});
				}
			}
		}
#endif
		//除此之外
		if(pruned_moves.empty()) {
			//复制己方候选点
			for(auto &i : candidate_moves) {
				pruned_moves.push_back({i.p, PVAL_B(i.pat)});
			}
		}
		std::sort(pruned_moves.begin(), pruned_moves.end());
		return std::move(pruned_moves);
	}
	std::vector<_point_with_value> _board::get_pruned_moves_white() {
		sync_candidate();
		std::vector<_point_with_value> pruned_moves;
BEGIN:
		//己方有四
		if(pat.second.four_l || pat.second.four_b) {
			//从己方候选点中找己方成五
			for(auto &i : candidate_moves) {
				if(i.pat.second.won) {
					pruned_moves.push_back({i.p, PVAL_W(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.second.four_b = pat.second.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方冲四（如果成四就已经死了）
		else if(pat.first.four_b) {
			//从对方候选点中找对方成五
			for(auto &i : candidate_moves) {
				if(i.pat.first.won) {
					pruned_moves.push_back({i.p, PVAL_W(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.first.four_b = pat.first.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//己方活三
		else if(pat.second.three_l) {
			//从己方候选点中找己方成活四
			for(auto &i : candidate_moves) {
				if(i.pat.second.four_l) {
					pruned_moves.push_back({i.p, PVAL_W(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成四点
			if(pruned_moves.empty()) {
				pat.second.three_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方活三
		else if(pat.first.three_l) {
			//从对方候选点中找对方成四 或者己方成冲四
#ifdef PAT_PATCH
			//为空说明没有成五点
			char exist4 = 0;
#endif
			for(auto &i : candidate_moves) {
				if(i.pat.first.four_l || i.pat.second.four_b) {
					pruned_moves.push_back({ i.p, PVAL_W(i.pat) });
#ifdef PAT_PATCH
					exist4 += i.pat.first.four_l;
#endif
					continue;
				}
				for(int k = 0; k < 4; ++k) {
					//对方落子前成活三
					if((pmap[i.ppc[k]] & 15) == 5) {
						pruned_moves.push_back({ i.p, PVAL_W(i.pat) });
						break;
					}
				}
			}
#ifdef PAT_PATCH
			if(!exist4) {
				pat.first.three_l = 0;
				pat_moves.back() = pat;
				pruned_moves.clear();
				goto BEGIN;
			}
#endif
		}
		//双方没活棋
#ifdef HUM_PRUNE
		else if(!pat.second.two_l && !pat.first.two_l && !pat.second.three_s && !pat.first.three_s) {
			//从己方候选点中找能成二或眠三的
			for(auto &i : candidate_moves) {
				if(i.pat.second.two_l || i.pat.second.three_s) {
					pruned_moves.push_back({i.p, PVAL_W(i.pat)});
				}
			}
		}
#endif
		//除此之外
		if(pruned_moves.empty()) {
			//复制己方候选点
			for(auto &i : candidate_moves) {
				pruned_moves.push_back({i.p, PVAL_W(i.pat)});
			}
		}
		std::sort(pruned_moves.begin(), pruned_moves.end());
		return std::move(pruned_moves);
	}

	std::vector<_point_with_value> _board::get_pruned_moves_black_forvct() {
		sync_candidate();
		std::vector<_point_with_value> pruned_moves;
		//己方有四
BEGIN:
		if(pat.first.four_l || pat.first.four_b) {
			//从己方候选点中找己方成五
			for(auto &i : candidate_moves) {
				if(i.pat.first.won) {
					pruned_moves.push_back({i.p, PATVAL_B(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.first.four_b = pat.first.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
			
		}
		//对方冲四（如果成四就已经死了）
		else if(pat.second.four_b) {
			//从对方候选点中找对方成五
			for(auto &i : candidate_moves) {
				if(i.pat.second.won) {
					pruned_moves.push_back({i.p, PATVAL_B(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.second.four_b = pat.second.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//己方活三
		else if(pat.first.three_l) {
			//从己方候选点中找己方成活四
			for(auto &i : candidate_moves) {
				if(i.pat.first.four_l) {
					pruned_moves.push_back({i.p, PATVAL_B(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成四点
			if(pruned_moves.empty()) {
				pat.first.three_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//
		else {
			//从己方候选点中找冲四活三
			for(auto &i : candidate_moves) {
				if(i.pat.first.four_b || i.pat.first.three_l 
					&& (pat.first.two_l || pat.first.three_s || i.pat.first.two_l || i.pat.first.three_s)) {
					pruned_moves.push_back({i.p, PATVAL_B(i.pat)});
				}
			}
		}
		std::sort(pruned_moves.begin(), pruned_moves.end());
		return std::move(pruned_moves);
	}
	std::vector<_point_with_value> _board::get_pruned_moves_white_forvct() {
		sync_candidate();
		std::vector<_point_with_value> pruned_moves;
BEGIN:
		//己方有四
		if(pat.second.four_l || pat.second.four_b) {
			//从己方候选点中找己方成五
			for(auto &i : candidate_moves) {
				if(i.pat.second.won) {
					pruned_moves.push_back({i.p, PATVAL_W(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.second.four_b = pat.second.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方冲四（如果成四就已经死了）
		else if(pat.first.four_b) {
			//从对方候选点中找对方成五
			for(auto &i : candidate_moves) {
				if(i.pat.first.won) {
					pruned_moves.push_back({i.p, PATVAL_W(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.first.four_b = pat.first.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//己方活三
		else if(pat.second.three_l) {
			//从己方候选点中找己方成活四
			for(auto &i : candidate_moves) {
				if(i.pat.second.four_l) {
					pruned_moves.push_back({i.p, PATVAL_W(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成四点
			if(pruned_moves.empty()) {
				pat.second.three_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		else {
			//从己方候选点中找冲四活三
			//同时保证有剩余的走法
			for(auto &i : candidate_moves) {
				if(i.pat.second.four_b || i.pat.second.three_l 
					&& (pat.second.two_l || pat.second.three_s || i.pat.second.two_l || i.pat.second.three_s)) {
					pruned_moves.push_back({i.p, PATVAL_W(i.pat)});
				}
			}
		}
		std::sort(pruned_moves.begin(), pruned_moves.end());
		return std::move(pruned_moves);
	}
	std::vector<_point_with_value> _board::get_pruned_moves_black_forvcf() {
		sync_candidate();
		std::vector<_point_with_value> pruned_moves;
BEGIN:		
		//己方有四
		if(pat.first.four_l || pat.first.four_b) {
			//从己方候选点中找己方成五
			for(auto &i : candidate_moves) {
				if(i.pat.first.won) {
					pruned_moves.push_back({i.p, PATVAL_B(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.first.four_b = pat.first.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方冲四（如果成四就已经死了）
		else if(pat.second.four_b) {
			//从对方候选点中找对方成五
			for(auto &i : candidate_moves) {
				if(i.pat.second.won) {
					pruned_moves.push_back({i.p, PATVAL_B(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.second.four_b = pat.second.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//己方活三
		else if(pat.first.three_l) {
			//从己方候选点中找己方成活四
			for(auto &i : candidate_moves) {
				if(i.pat.first.four_l) {
					pruned_moves.push_back({i.p, PATVAL_B(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成四点
			if(pruned_moves.empty()) {
				pat.first.three_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//
		else {
			//从己方候选点中找冲四
			for(auto &i : candidate_moves) {
				if(i.pat.first.four_b && ( pat.first.three_s || i.pat.first.three_s)) {
					pruned_moves.push_back({i.p, PATVAL_B(i.pat)});
				}
			}
		}
		std::sort(pruned_moves.begin(), pruned_moves.end());
		return std::move(pruned_moves);
	}
	std::vector<_point_with_value> _board::get_pruned_moves_white_forvcf() {
		sync_candidate();
		std::vector<_point_with_value> pruned_moves;
BEGIN:
		//己方有四
		if(pat.second.four_l || pat.second.four_b) {
			//从己方候选点中找己方成五
			for(auto &i : candidate_moves) {
				if(i.pat.second.won) {
					pruned_moves.push_back({i.p, PATVAL_W(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.second.four_b = pat.second.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方冲四（如果成四就已经死了）
		else if(pat.first.four_b) {
			//从对方候选点中找对方成五
			for(auto &i : candidate_moves) {
				if(i.pat.first.won) {
					pruned_moves.push_back({i.p, PATVAL_W(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.first.four_b = pat.first.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//己方活三
		else if(pat.second.three_l) {
			//从己方候选点中找己方成活四
			for(auto &i : candidate_moves) {
				if(i.pat.second.four_l) {
					pruned_moves.push_back({i.p, PATVAL_W(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成四点
			if(pruned_moves.empty()) {
				pat.second.three_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		else {
			//从己方候选点中找冲四&眠三
			for(auto &i : candidate_moves) {
				if(i.pat.second.four_b && ( pat.second.three_s || i.pat.second.three_s)) {
					pruned_moves.push_back({i.p, PATVAL_W(i.pat)});
				}
			}
		}
		std::sort(pruned_moves.begin(), pruned_moves.end());
		return std::move(pruned_moves);
	}

	std::vector<_point_with_value> _board::get_pruned_moves_black_fordefend() {
		sync_candidate();
		std::vector<_point_with_value> pruned_moves;
BEGIN:
		//己方有四
		if(pat.first.four_l || pat.first.four_b) {
			//从己方候选点中找己方成五
			for(auto &i : candidate_moves) {
				if(i.pat.first.won) {
					pruned_moves.push_back({i.p, PATVAL_W(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.first.four_b = pat.first.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方冲四（如果成四就已经死了）
		else if(pat.second.four_b) {
			//从对方候选点中找对方成五
			for(auto &i : candidate_moves) {
				if(i.pat.second.won) {
					pruned_moves.push_back({i.p, PATVAL_W(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.second.four_b = pat.second.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//己方活三
		else if(pat.first.three_l) {
			//从己方候选点中找己方成活四
			for(auto &i : candidate_moves) {
				if(i.pat.first.four_l) {
					pruned_moves.push_back({i.p, PATVAL_W(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成四点
			if(pruned_moves.empty()) {
				pat.first.three_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方活三
		else if(pat.second.three_l) {
			//从对方候选点中找对方成四 或者己方成冲四
#ifdef PAT_PATCH
			char exist4 = 0;
#endif
			for(auto &i : candidate_moves) {
				if(i.pat.second.four_l || i.pat.first.four_b) {
					pruned_moves.push_back({ i.p, PATVAL_W(i.pat) });
#ifdef PAT_PATCH
					exist4 += i.pat.second.four_l;
#endif
					continue;
				}
				for(int k = 0; k < 4; ++k) {
					//对方落子前成活三
					if(((pmap[i.ppc[k]] >> 4)) == 5) {
						pruned_moves.push_back({ i.p, PATVAL_W(i.pat) });
						break;
					}
				}
			}
#ifdef PAT_PATCH
			if(!exist4) {
				pat.second.three_l = 0;
				pat_moves.back() = pat;
				pruned_moves.clear();
				//goto BEGIN;
			}
#endif
		}
		std::sort(pruned_moves.begin(), pruned_moves.end());
		return std::move(pruned_moves);
	}
	std::vector<_point_with_value> _board::get_pruned_moves_white_fordefend() {
		sync_candidate();
		std::vector<_point_with_value> pruned_moves;
BEGIN:
		//己方有四
		if(pat.second.four_l || pat.second.four_b) {
			//从己方候选点中找己方成五
			for(auto &i : candidate_moves) {
				if(i.pat.second.won) {
					pruned_moves.push_back({i.p, PATVAL_B(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.second.four_b = pat.second.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方冲四（如果成四就已经死了）
		else if(pat.first.four_b) {
			//从对方候选点中找对方成五
			for(auto &i : candidate_moves) {
				if(i.pat.first.won) {
					pruned_moves.push_back({i.p, PATVAL_B(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成五点
			if(pruned_moves.empty()) {
				pat.first.four_b = pat.first.four_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//己方活三
		else if(pat.second.three_l) {
			//从己方候选点中找己方成活四
			for(auto &i : candidate_moves) {
				if(i.pat.second.four_l) {
					pruned_moves.push_back({i.p, PATVAL_B(i.pat)});
					break;
				}
			}
#ifdef PAT_PATCH
			//为空说明没有成四点
			if(pruned_moves.empty()) {
				pat.second.three_l = 0;
				pat_moves.back() = pat;
				goto BEGIN;
			}
#endif
		}
		//对方活三
		else if(pat.first.three_l) {
			//从对方候选点中找对方成四 或者己方成冲四
#ifdef PAT_PATCH
			//为空说明没有成五点
			char exist4 = 0;
#endif
			for(auto &i : candidate_moves) {
				if(i.pat.first.four_l || i.pat.second.four_b) {
					pruned_moves.push_back({ i.p, PATVAL_B(i.pat) });
#ifdef PAT_PATCH
					exist4 += i.pat.first.four_l;
#endif
					continue;
				}
				for(int k = 0; k < 4; ++k) {
					//对方落子前成活三
					if((pmap[i.ppc[k]] & 15) == 5) {
						pruned_moves.push_back({ i.p, PATVAL_B(i.pat) });
						break;
					}
				}
			}
#ifdef PAT_PATCH
			if(!exist4) {
				pat.first.three_l = 0;
				pat_moves.back() = pat;
				pruned_moves.clear();
				//goto BEGIN;
			}
#endif
		}
		std::sort(pruned_moves.begin(), pruned_moves.end());
		return std::move(pruned_moves);
	}

	zobrist_key _board::zob;
	const patterns_map _board::pmap;
}