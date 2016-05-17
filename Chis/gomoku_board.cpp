#include "gomoku_board.h"
using namespace chis;

#define WIDTH 3
///////////////////////��ͨ�ĺ���////////////////////////////////
void Board::move(char i, char j) {
	//set(i, j, turn);//����
	++step;
	hash_key ^= zobrist[turn == -1][(i*SIZE + j)];
	//������Ϣ��
	last_set = { i, j };
	last_set_color = turn;
	update_patterns();//����_patterns,˳������
	moves_update();

	moves.push_back({ i, j });
	pattern_change.push_back(_patterns);
	turn = -turn;//��ת
	
}
void Board::unmove() {
	//set(moves.back().x, moves.back().y, 0);
	--step;
	hash_key ^= zobrist[(b[moves.back().x][moves.back().y] == -1)][(moves.back().x*SIZE + moves.back().y)];
	//������Ϣ��
	last_set = { moves.back().x, moves.back().y };
	last_set_color = 0;
	b[moves.back().x][moves.back().y] = 0;
	moves_update();

	moves.pop_back();
	pattern_change.pop_back();
	_patterns = pattern_change.back();
	turn = -turn;
	
}
int Board::evaluate() {
	//
	return 0;
}
int Board::nega_value() {
	return
		turn == -1 ?
		default_para*(_patterns.second - _patterns.first) :
		default_para*(_patterns.first - _patterns.second);
}
int Board::try_set_nega(char i, char j, int try_c) {
	patterns_pair pair;
	int color = b[i][j];
	/////////�������Ӻ������/////////
	b[i][j] = try_c;
	patterns_add(&(pair.first), itor_1(b, i, j), 1);
	patterns_add(&(pair.first), itor_2(b, i, j), 1);
	patterns_add(&(pair.first), itor_3(b, i, j), 1);
	patterns_add(&(pair.first), itor_4(b, i, j), 1);
	patterns_add(&(pair.second), itor_1(b, i, j), -1);
	patterns_add(&(pair.second), itor_2(b, i, j), -1);
	patterns_add(&(pair.second), itor_3(b, i, j), -1);
	patterns_add(&(pair.second), itor_4(b, i, j), -1);
	//////////////֮ǰ������////////////
	b[i][j] = color;
	if(try_c == 1) {
		return chs_point*pair.first - ((chs_point*pair.second) >> 1);
	}
	else {
		return chs_point*pair.second - ((chs_point*pair.first) >> 1);
	}

}
int Board::try_set(char i, char j, int try_c) {
	Patterns &&who = get_update(i, j, try_c);// , &&opp = get_update(i, j, -try_c);
	return chs_point*who/* + ((chs_point*opp) >> 1)*/;

}
int Board::point_eval(char i, char j, int try_c) {
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
	//�ذ��ж�
	//����ذܣ�
	//ǰ�᣺����δ���ӣ����޳��塣��������
	//0.�Է��ѳɻ���
	//1.�Է���˫����
	//2.�Է���˫��������һ��������һ�����ģ���������
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
void Board::moves_update() {
	//�������һ�����ӣ�����Ȩֵ����ŷ���
	//todo �������ͱ�ĸ���
	int n;
	if(last_set_color) {
		n = WIDTH;
		for(itor_1 bg(ps, last_set.x, last_set.y); n && bg.in_board(); ++bg, --n) {
			bg[0] += n;
		}
		n = WIDTH;
		for(itor_1 bg(ps, last_set.x, last_set.y); n && bg.in_board(); --bg, --n) {
			bg[0] += n;
		}
		n = WIDTH;
		for(itor_2 bg(ps, last_set.x, last_set.y); n && bg.in_board(); ++bg, --n) {
			bg[0] += n;
		}
		n = WIDTH;
		for(itor_2 bg(ps, last_set.x, last_set.y); n && bg.in_board(); --bg, --n) {
			bg[0] += n;
		}
		n = WIDTH;
		for(itor_3 bg(ps, last_set.x, last_set.y); n && bg.in_board(); ++bg, --n) {
			bg[0] += n;
		}
		n = WIDTH;
		for(itor_3 bg(ps, last_set.x, last_set.y); n && bg.in_board(); --bg, --n) {
			bg[0] += n;
		}
		n = WIDTH;
		for(itor_4 bg(ps, last_set.x, last_set.y); n && bg.in_board(); ++bg, --n) {
			bg[0] += n;
		}
		n = WIDTH;
		for(itor_4 bg(ps, last_set.x, last_set.y); n && bg.in_board(); --bg, --n) {
			bg[0] += n;
		}
	}
	else {
		n = WIDTH;
		for(itor_1 bg(ps, last_set.x, last_set.y); n && bg.in_board(); ++bg, --n) {
			bg[0] -= n;
		}
		n = WIDTH;
		for(itor_1 bg(ps, last_set.x, last_set.y); n && bg.in_board(); --bg, --n) {
			bg[0] -= n;
		}
		n = WIDTH;
		for(itor_2 bg(ps, last_set.x, last_set.y); n && bg.in_board(); ++bg, --n) {
			bg[0] -= n;
		}
		n = WIDTH;
		for(itor_2 bg(ps, last_set.x, last_set.y); n && bg.in_board(); --bg, --n) {
			bg[0] -= n;
		}
		n = WIDTH;
		for(itor_3 bg(ps, last_set.x, last_set.y); n && bg.in_board(); ++bg, --n) {
			bg[0] -= n;
		}
		n = WIDTH;
		for(itor_3 bg(ps, last_set.x, last_set.y); n && bg.in_board(); --bg, --n) {
			bg[0] -= n;
		}
		n = WIDTH;
		for(itor_4 bg(ps, last_set.x, last_set.y); n && bg.in_board(); ++bg, --n) {
			bg[0] -= n;
		}
		n = WIDTH;
		for(itor_4 bg(ps, last_set.x, last_set.y); n && bg.in_board(); --bg, --n) {
			bg[0] -= n;
		}
	}
}

///////////////////////����ɨ�躯��////////////////////////////////
template<typename itor>
unsigned long get_patterns_key(itor &begin, int c) {//�õ���beginΪ���ĵ�����key
	unsigned long key = 1;
	itor l = begin, r = begin;
	//��������
	//1.���߳�������
	for(int cnt = 0, len = 0; l.in_board() && *l != -c && cnt < 5 && len < 6; ++len, --l, (*l == 0 ? ++cnt : cnt = 0));
	for(int cnt = 0, len = 0; r.in_board() && *r != -c && cnt < 5 && len < 6; ++len, ++r, (*r == 0 ? ++cnt : cnt = 0));
	++l;
	for(; l != r; ++l) {
		key = key << 1;
		if(*l == c) {
			++key;
		}
	}
	return pattern_map[key];
}
//TODO:�Ż���
template<typename itor>
void Board::patterns_add(Patterns *ret, itor &begin, int c) {//��beginΪ���ģ�ɨ��c�ӵ����ͣ��ӵ�ret�ϡ�
	if(*begin == -c) {//������ĵ���ɫ��Ҫɨ�����ɫ��ͬ
		itor l = begin, r = begin;
		unsigned long key = 1;
		--l;
		++r;
		for(int len = 0; l.in_board() && *l != -c && len < 6; ++len, --l) {
			key = key << 1;
			if(*l == c) {
				++key;
			}
		}
		if(pattern_map[key] >= 0) {
			++(((char*)ret)[pattern_map[key]]);
		}
		key = 1;
		for(int len = 0; r.in_board() && *r != -c && len < 6; ++len, ++r) {
			key = key << 1;
			if(*r == c) {
				++key;
			}
		}
		if(pattern_map[key] >= 0) {
			++(((char*)ret)[pattern_map[key]]);
		}
	}
	else {
		unsigned long key = 1;
		itor l = begin, r = begin;
		//1.���߳�������
		for(int len = 0; l.in_board() && *l != -c && len < 6; ++len, --l) {
			key = key << 1;
			if(*l == c) {
				++key;
			}
		}
		key = pattern_rev[key];
		/*for(int len = 0; l.in_board() && *l != -c && len < 6; ++len, --l);
		++l;
		for(; l != r; ++l) {
			key = key << 1;
			if(*l == c) {
				++key;
			}
		}*/
		for(int len = 0; r.in_board() && *r != -c && len < 6; ++len, ++r) {
			key = key << 1;
			if(*r == c) {
				++key;
			}
		}
		if(pattern_map[key] >= 0) {
			++(((char*)ret)[pattern_map[key]]);
		}
	}
}
template<typename itor>
void Board::patterns_dec(Patterns *ret, itor &begin, int c) {
	if(*begin == -c) {//������ĵ���ɫ��Ҫɨ�����ɫ��ͬ
		itor l = begin, r = begin;
		unsigned long key = 1;
		--l;
		++r;
		for(int len = 0; l.in_board() && *l != -c && len < 6; ++len, --l) {
			key = key << 1;
			if(*l == c) {
				++key;
			}
		}
		if(pattern_map[key] >= 0) {
			--(((char*)ret)[pattern_map[key]]);
		}
		key = 1;
		for(int len = 0; r.in_board() && *r != -c && len < 6; ++len, ++r) {
			key = key << 1;
			if(*r == c) {
				++key;
			}
		}
		if(pattern_map[key] >= 0) {
			--(((char*)ret)[pattern_map[key]]);
		}
	}
	else {
		unsigned long key = 1;
		itor l = begin, r = begin;
		//1.���߳�������
		for(int len = 0; l.in_board() && *l != -c && len < 6; ++len, --l) {
			key = key << 1;
			if(*l == c) {
				++key;
			}
		}
		key = pattern_rev[key];
		for(int len = 0; r.in_board() && *r != -c && len < 6; ++len, ++r) {
			key = key << 1;
			if(*r == c) {
				++key;
			}
		}
		if(pattern_map[key] >= 0) {
			--(((char*)ret)[pattern_map[key]]);
		}
	}
}
template<typename itor>
void patterns_add_old(Patterns *ret, itor &begin, int c) {//��beginΪ���ģ�ɨ��c�ӵ����ͣ��ӵ�ret�ϡ�
	if(*begin == -c) {//������ĵ���ɫ��Ҫɨ�����ɫ��ͬ
		itor l = begin, r = begin;
		--l;
		++r;
		for(int len = 0; l.in_board() && *l != -c && len < 6; ++len, --l);
		for(int len = 0; r.in_board() && *r != -c && len < 6; ++len, ++r);
		++l;
		unsigned long key = 1;
		for(; l != begin; ++l) {
			key = key << 1;
			if(*l == c) {
				++key;
			}
		}
		if(pattern_map[key] >= 0) {
			++(((char*)ret)[pattern_map[key]]);
		}
		++l;//begin+1
		key = 1;
		for(; l != r; ++l) {
			key = key << 1;
			if(*l == c) {
				++key;
			}
		}
		if(pattern_map[key] >= 0) {
			++(((char*)ret)[pattern_map[key]]);
		}
	}
	else {
		unsigned long key = 1;
		itor l = begin, r = begin;
		//��������
		//1.���߳�������
		for(int len = 0; l.in_board() && *l != -c && len < 6; ++len, --l);
		for(int len = 0; r.in_board() && *r != -c && len < 6; ++len, ++r);
		++l;
		for(; l != r; ++l) {
			key = key << 1;
			if(*l == c) {
				++key;
			}
		}
		if(pattern_map[key] >= 0) {
			++(((char*)ret)[pattern_map[key]]);
		}
	}
}
template<typename itor>
void patterns_dec_old(Patterns *ret, itor &begin, int c) {
	if(*begin == -c) {//������ĵ���ɫ��Ҫɨ�����ɫ��ͬ
		itor l = begin, r = begin;
		--l;
		++r;
		for(int len = 0; l.in_board() && *l != -c && len < 6; ++len, --l);
		for(int len = 0; r.in_board() && *r != -c && len < 6; ++len, ++r);
		++l;
		unsigned long key = 1;
		for(; l != begin; ++l) {
			key = key << 1;
			if(*l == c) {
				++key;
			}
		}
		if(pattern_map[key] >= 0) {
			--(((char*)ret)[pattern_map[key]]);
		}
		++l;//begin+1
		key = 1;
		for(; l != r; ++l) {
			key = key << 1;
			if(*l == c) {
				++key;
			}
		}
		if(pattern_map[key] >= 0) {
			--(((char*)ret)[pattern_map[key]]);
		}
	}
	else {
		unsigned long key = 1;
		itor l = begin, r = begin;
		//��������
		//1.���߳�������
		for(int len = 0; l.in_board() && *l != -c && len < 6; ++len, --l);
		for(int len = 0; r.in_board() && *r != -c && len < 6; ++len, ++r);
		++l;
		for(; l != r; ++l) {
			key = key << 1;
			if(*l == c) {
				++key;
			}
		}
		if(pattern_map[key] >= 0) {
			--(((char*)ret)[pattern_map[key]]);
		}
	}
}
void Board::update_patterns() {
	patterns_dec(&(_patterns.first), itor_1(b, last_set.x, last_set.y), 1);
	patterns_dec(&(_patterns.first), itor_2(b, last_set.x, last_set.y), 1);
	patterns_dec(&(_patterns.first), itor_3(b, last_set.x, last_set.y), 1);
	patterns_dec(&(_patterns.first), itor_4(b, last_set.x, last_set.y), 1);
	patterns_dec(&(_patterns.second), itor_1(b, last_set.x, last_set.y), -1);
	patterns_dec(&(_patterns.second), itor_2(b, last_set.x, last_set.y), -1);
	patterns_dec(&(_patterns.second), itor_3(b, last_set.x, last_set.y), -1);
	patterns_dec(&(_patterns.second), itor_4(b, last_set.x, last_set.y), -1);
	b[last_set.x][last_set.y] = last_set_color;
	patterns_add(&(_patterns.first), itor_1(b, last_set.x, last_set.y), 1);
	patterns_add(&(_patterns.first), itor_2(b, last_set.x, last_set.y), 1);
	patterns_add(&(_patterns.first), itor_3(b, last_set.x, last_set.y), 1);
	patterns_add(&(_patterns.first), itor_4(b, last_set.x, last_set.y), 1);
	patterns_add(&(_patterns.second), itor_1(b, last_set.x, last_set.y), -1);
	patterns_add(&(_patterns.second), itor_2(b, last_set.x, last_set.y), -1);
	patterns_add(&(_patterns.second), itor_3(b, last_set.x, last_set.y), -1);
	patterns_add(&(_patterns.second), itor_4(b, last_set.x, last_set.y), -1);
}
///////////////////////���ͻ�ú���////////////////////////////////
Patterns Board::get_update(char i, char j, int try_c, int ret_color) {
	int c = b[i][j];
	if(!ret_color) {
		ret_color = try_c;
	}
	Patterns try_count = ret_color == 1 ? _patterns.first : _patterns.second;
	/////////��ȥû��ǰ������/////////////
	patterns_dec(&(try_count), itor_1(b, i, j), ret_color);
	patterns_dec(&(try_count), itor_2(b, i, j), ret_color);
	patterns_dec(&(try_count), itor_3(b, i, j), ret_color);
	patterns_dec(&(try_count), itor_4(b, i, j), ret_color);
	b[i][j] = try_c;
	patterns_add(&(try_count), itor_1(b, i, j), ret_color);
	patterns_add(&(try_count), itor_2(b, i, j), ret_color);
	patterns_add(&(try_count), itor_3(b, i, j), ret_color);
	patterns_add(&(try_count), itor_4(b, i, j), ret_color);
	b[i][j] = c;
	return try_count;
}
