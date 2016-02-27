#include "gomoku_board.h"
using namespace chis;

///////////////////////普通的函数////////////////////////////////
int Board::set(int i, int j, int c) {
	if(c /*&& !b[i][j]*/) {
		hash_key ^= zobrist[c == -1][(i*SIZE + j)];
		++step;
	}
	else if(!c) {
		--step;
		hash_key ^= zobrist[(b[i][j] == -1)][(i*SIZE + j)];
	}
	last_set = { i, j };
	last_set_color = c;
	update_patterns();
	return 0;
}
int Board::evaluate() {
	//
}
int Board::nega_value() {
#ifndef CHIS_TEST2
	return
		step % 2 ?
		default_para*(_patterns.first - _patterns.second) :
		default_para*(_patterns.second - _patterns.first);
#endif
#ifdef CHIS_TEST2
	return
		step % 2 ?
		chs_value3*(_patterns.first - _patterns.second) :
		chs_value3*(_patterns.second - _patterns.first);
#endif
}
int Board::try_set_nega(int i, int j, int try_c) {
	patterns_pair pair;
	int color = b[i][j];
	/////////己方落子后的棋型/////////
	b[i][j] = try_c;
	patterns_of_line_add(pair, begin1({ i, j }));
	patterns_of_line_add(pair, begin2({ i, j }));
	patterns_of_line_add(pair, begin3({ i, j }));
	patterns_of_line_add(pair, begin4({ i, j }));
	//////////////之前的棋型////////////
	b[i][j] = color;
	if(try_c == 1) {
		return chs_point*pair.first - ((chs_point*pair.second) >> 1);
	}
	else {
		return chs_point*pair.second - ((chs_point*pair.first) >> 1);
	}

}
int Board::try_set(int i, int j, int try_c) {
	Patterns &&who = get_update(i, j, try_c);// , &&opp = get_update(i, j, -try_c);
	return chs_point*who/* + ((chs_point*opp) >> 1)*/;

}
int Board::point_eval(int i, int j, int try_c) {
	Patterns &&who = get_update(i, j, try_c), &&opp = get_update(i, j, -try_c);
	return chs_point*who + ((chs_point*opp) >> 1);

}

int Board::have_winner() {
	if(_patterns.first.wined >= 1) {
		return 1;
	}
	if(_patterns.second.wined >= 1) {
		return -1;
	}
	//必败判定
	//定义必败：
	//前提：己方未落子，且无成五。己方无四
	//0.对方已成活四
	//1.对方有双冲四
	//2.对方有双活三或者一个活三，一个冲四，己方无三
	if(step % 2) {
		if(!_patterns.second.four_b && !_patterns.second.four_l) {
			if(_patterns.first.four_l || _patterns.first.four_b >= 2 ||
				(!_patterns.second.three_l && !_patterns.second.three_s && _patterns.first.four_b + _patterns.first.three_l >= 2)) {
				return 1;
			}
		}
	}
	else {
		if(!_patterns.first.four_b && !_patterns.first.four_l) {
			if(_patterns.second.four_l || _patterns.second.four_b >= 2 ||
				(!_patterns.first.three_l && !_patterns.first.three_s && _patterns.second.four_b + _patterns.second.three_l >= 2)) {
				return -1;
			}
		}
	}
	return 0;
}
///////////////////////棋型扫描函数////////////////////////////////
void Board::update_patterns() {
	/////////没下前的棋型/////////////
	patterns_of_line_dec(_patterns, begin1(last_set));
	patterns_of_line_dec(_patterns, begin2(last_set));
	patterns_of_line_dec(_patterns, begin3(last_set));
	patterns_of_line_dec(_patterns, begin4(last_set));
	/////////下过后的棋型/////////////
	b[last_set.x][last_set.y] = last_set_color;
	patterns_of_line_add(_patterns, begin1(last_set));
	patterns_of_line_add(_patterns, begin2(last_set));
	patterns_of_line_add(_patterns, begin3(last_set));
	patterns_of_line_add(_patterns, begin4(last_set));

}
template<typename itor>
void Board::patterns_of_line_add(patterns_pair &count, itor &begin) {
	itor backup = begin;
	Patterns *ret = &(count.first);
	int c = 1;
COLORCHANGE:
	for(; begin.in_board(); ++begin) {
		if(*begin == c) {
			char real = 0;
			int spcnt = 0;
			while(begin.in_board() && *begin != -c && spcnt < 5) {
				if(*begin == c) {
					spcnt = 0;
					unsigned long key = 1;
					itor l = begin, r = begin;
					//两个方案
					//1.限制总长度，2.各边长度限制
					for(int cnt = 0, len = 0; l.in_board() && *l != -c && cnt < 5 && len < 6; ++len, --l, (*l == 0 ? ++cnt : cnt = 0));
					for(int cnt = 0, len = 0; r.in_board() && *r != -c && cnt < 5 && len < 6; ++len, ++r, (*r == 0 ? ++cnt : cnt = 0));
					++l;
					for(; l != r; ++l) {
						key = key << 1;
						if(*l == c) {
							++key;
						}
					}
					if(real < pattern_map[key]) {
						real = pattern_map[key];
#ifdef CHIS_DEBUG
						++real_cnt;
#endif
					}
					begin = r;
				}
				else {
					++begin;
					if(*begin == 0) {
						++spcnt;
					}
					else {
						spcnt = 0;
					}
				}
			}
			if(real >= 0) {
				++(((char*)ret)[real]);
			}
		}
	}
	if(c == 1) {
		c = -1;
		ret = &(count.second);
		begin = backup;
		goto COLORCHANGE;
	}

}
template<typename itor>
void Board::patterns_of_line_dec(patterns_pair &count, itor &begin) {
	itor backup = begin;
	Patterns *ret = &(count.first);
	int c = 1;
COLORCHANGE1:
	for(; begin.in_board(); ++begin) {
		if(*begin == c) {
			char real = 0;
			int spcnt = 0;
			while(begin.in_board() && *begin != -c && spcnt < 5) {
				if(*begin == c) {
					spcnt = 0;
					unsigned long key = 1;
					itor l = begin, r = begin;
					//两个方案
					//1.限制总长度，2.各边长度限制
					for(int cnt = 0, len = 0; l.in_board() && *l != -c && cnt < 5 && len < 6; ++len, --l, (*l == 0 ? ++cnt : cnt = 0));
					for(int cnt = 0, len = 0; r.in_board() && *r != -c && cnt < 5 && len < 6; ++len, ++r, (*r == 0 ? ++cnt : cnt = 0));
					++l;
					for(; l != r; ++l) {
						key = key << 1;
						if(*l == c) {
							++key;
						}
					}
					if(real < pattern_map[key]) {
						real = pattern_map[key];
#ifdef CHIS_DEBUG
						++real_cnt;
#endif
					}
					begin = r;
				}
				else {
					++begin;
					if(*begin == 0) {
						++spcnt;
					}
					else {
						spcnt = 0;
					}
				}
			}
			if(real >= 0) {
				--(((char*)ret)[real]);
			}
		}
	}
	if(c == 1) {
		c = -1;
		ret = &(count.second);
		begin = backup;
		goto COLORCHANGE1;
	}
}
template<typename itor>
void pattern_of_line_add(Patterns *ret, itor &begin, int c) {
	for(; begin.in_board(); ++begin) {
		if(*begin == c) {
			char real = 0;
			int spcnt = 0;
			while(begin.in_board() && *begin != -c && spcnt < 5) {
				if(*begin == c) {
					spcnt = 0;
					unsigned long key = 1;
					itor l = begin, r = begin;
					//两个方案
					//1.限制总长度，2.各边长度限制
					for(int cnt = 0, len = 0; l.in_board() && *l != -c && cnt < 5 && len < 6; ++len, --l, (*l == 0 ? ++cnt : cnt = 0));
					for(int cnt = 0, len = 0; r.in_board() && *r != -c && cnt < 5 && len < 6; ++len, ++r, (*r == 0 ? ++cnt : cnt = 0));
					++l;
					for(; l != r; ++l) {
						key = key << 1;
						if(*l == c) {
							++key;
						}
					}
					if(real < pattern_map[key]) {
						real = pattern_map[key];
#ifdef CHIS_DEBUG
						++real_cnt;
#endif
					}
					begin = r;
				}
				else {
					++begin;
					if(*begin == 0) {
						++spcnt;
					}
					else {
						spcnt = 0;
					}
				}
			}
			if(real >= 0) {
				++(((char*)ret)[real]);
			}
		}
	}

}
template<typename itor>
void pattern_of_line_dec(Patterns *ret, itor &begin, int c) {
	for(; begin.in_board(); ++begin) {
		if(*begin == c) {
			char real = 0;
			int spcnt = 0;
			while(begin.in_board() && *begin != -c && spcnt < 5) {
				if(*begin == c) {
					spcnt = 0;
					unsigned long key = 1;
					itor l = begin, r = begin;
					//两个方案
					//1.限制总长度，2.各边长度限制
					for(int cnt = 0, len = 0; l.in_board() && *l != -c && cnt < 5 && len < 6; ++len, --l, (*l == 0 ? ++cnt : cnt = 0));
					for(int cnt = 0, len = 0; r.in_board() && *r != -c && cnt < 5 && len < 6; ++len, ++r, (*r == 0 ? ++cnt : cnt = 0));
					++l;
					for(; l != r; ++l) {
						key = key << 1;
						if(*l == c) {
							++key;
						}
					}
					if(real < pattern_map[key]) {
						real = pattern_map[key];
#ifdef CHIS_DEBUG
						++real_cnt;
#endif
					}
					begin = r;
				}
				else {
					++begin;
					if(*begin == 0) {
						++spcnt;
					}
					else {
						spcnt = 0;
					}
				}
			}
			if(real >= 0) {
				--(((char*)ret)[real]);
			}
		}
	}

}

///////////////////////棋型获得函数////////////////////////////////
patterns_pair Board::get_patterns(int i, int j) {
	patterns_pair pair;
	patterns_of_line_add(pair, begin1({ i, j }));
	patterns_of_line_add(pair, begin2({ i, j }));
	patterns_of_line_add(pair, begin3({ i, j }));
	patterns_of_line_add(pair, begin4({ i, j }));
	return pair;
}
patterns_pair Board::get_patterns(int i, int j, int try_c) {
	int c = b[i][j];
	patterns_pair try_count = _patterns;
	b[i][j] = try_c;
	patterns_of_line_add(try_count, begin1({ i, j }));
	patterns_of_line_add(try_count, begin2({ i, j }));
	patterns_of_line_add(try_count, begin3({ i, j }));
	patterns_of_line_add(try_count, begin4({ i, j }));
	b[i][j] = c;
	return try_count;
}
Patterns Board::get_update(int i, int j, int try_c, int ret_color) {//返回落子后的指定颜色(全盘)棋型
	int c = b[i][j];
	if(!ret_color) {
		ret_color = try_c;
	}
	Patterns try_count = ret_color == 1 ? _patterns.first : _patterns.second;
	/////////减去没下前的棋型/////////////
	pattern_of_line_dec(&try_count, begin1({ i, j }), ret_color);
	pattern_of_line_dec(&try_count, begin2({ i, j }), ret_color);
	pattern_of_line_dec(&try_count, begin3({ i, j }), ret_color);
	pattern_of_line_dec(&try_count, begin4({ i, j }), ret_color);
	b[i][j] = try_c;
	pattern_of_line_add(&try_count, begin1({ i, j }), ret_color);
	pattern_of_line_add(&try_count, begin2({ i, j }), ret_color);
	pattern_of_line_add(&try_count, begin3({ i, j }), ret_color);
	pattern_of_line_add(&try_count, begin4({ i, j }), ret_color);
	b[i][j] = c;

	return try_count;
}