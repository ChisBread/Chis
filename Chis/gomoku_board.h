#ifndef _CHIS_BOARD
#define _CHIS_BOARD
#include "chis_config.h"
namespace chis 
{


	struct Point {
		//点类型
		char x, y;
		Point() {}
		Point(char a, char b) :x(a), y(b) {}
		bool operator==(const Point&b)const {
			return x == b.x && y == b.y;
		}
		bool operator<(const Point &p) const {
			return ((x << 6) ^ y) < ((p.x << 6) ^ p.y);
		}
	};
	struct _point_with_value {
		//点与启发估值
		Point first;
		int value;
		_point_with_value(Point _p, int _v) :first(_p), value(_v) {}

		bool operator<(const _point_with_value &a)const {
			return value > a.value;
		}
		bool operator==(const _point_with_value &b)const {
			return first.x == b.first.x && first.y == b.first.y;
		}
	};
	struct _pwv_cmp {
		bool operator()(const _point_with_value &a, const _point_with_value &b) const {
			return a.first.operator<(b.first);
		}
	};
	struct _depth_with_value_ {
		//搜索深度与节点值
		//flag
		//0 杀局
		//1 alpha节点
		//2 pv节点
		//3 beta节点
		char hash_flag;
		char depth;
		int value;
	};
	struct Patterns {
		//位置不能变！（用于直接寻址）
		char one_s = 0;//0 眠一
		char one_l = 0;//1 活一
		char two_s = 0;//2 眠二
		char two_l = 0;//3 活二
		char three_s = 0;//4 眠三
		char three_l = 0;//5 活三
		char four_b = 0;//6 眠四
		char four_l = 0;//7 活四
		char wined = 0;//8 胜利（啊咧，好像拼错了
		Patterns& operator+=(const Patterns &a) {
			wined += a.wined;
			four_l += a.four_l;
			four_b += a.four_b;
			three_l += a.three_l;
			three_s += a.three_s;
			two_l += a.two_l;
			two_s += a.two_s;
			one_l += a.one_l;
			one_s += a.one_s;
			return *this;
		}
		Patterns& operator-=(const Patterns &a) {
			wined -= a.wined;
			four_l -= a.four_l;
			four_b -= a.four_b;
			three_l -= a.three_l;
			three_s -= a.three_s;
			two_l -= a.two_l;
			two_s -= a.two_s;
			one_l -= a.one_l;
			one_s -= a.one_s;
			return *this;
		}
		Patterns operator-(const Patterns &a)const {
			Patterns temp = *this;
			return (temp -= a);
		}
		void reset() {
			one_s = 0;//0 眠一
			one_l = 0;//1 活一
			two_s = 0;//2 眠二
			two_l = 0;//3 活二
			three_s = 0;//4 眠三
			three_l = 0;//5 活三
			four_b = 0;//6 眠四
			four_l = 0;//7 活四
			wined = 0;//8 胜利（啊咧，好像拼错了
		}
	};
	struct patterns_pair {
		Patterns first;
		Patterns second;
		void operator+=(const patterns_pair &r) {
			first += r.first;
			second += r.second;
		}
		void operator-=(const patterns_pair &r) {
			first -= r.first;
			second -= r.second;
		}
		void reset() {
			first.reset();
			second.reset();
		}
	};
	struct Parameters {
		int one_s = 3;
		int one_l = 15;
		int two_s = 45;
		int two_l = 180;
		int three_s = 280;
		int three_l = 485;
		int four_b = 580;
		int four_l = 850;
		int wined = 10000000;
		/*
		int one_s = 3;
		int one_l = 15;
		int two_s = 45;
		int two_l = 180;
		int three_s = 280;
		int three_l = 485;
		int four_b = 580;
		int four_l = 850;
		int wined = 10000000;*/
		Parameters() {}
		Parameters(int os, int ol, int ts, int tl, int ths, int thl, int fb, int fl, int w)
			:one_s(os), one_l(ol), two_s(ts), two_l(tl), three_s(ths), three_l(thl), four_b(fb), four_l(fl), wined(w) {
		}
		int operator*(const Patterns &cnt) const {
			return
				wined * cnt.wined +
				four_l * cnt.four_l +
				four_b * cnt.four_b +
				three_l * cnt.three_l +
				three_s * cnt.three_s +
				two_l *cnt.two_l +
				two_s * cnt.two_s +
				one_l * cnt.one_l +
				one_s * cnt.one_s;
		}
	};
	struct chis_config {
		int VCF_DEPTH;
		int VCT_DEPTH;
		int VC2_DEPTH;
		int MAX_DEPTH;//设置搜索深度
		int MAX_P;//设置搜索最后一层候选着法的个数
		int SEARCH_TIME;
		chis_config(int vf, int vt, int v2, int md, int mp, int st)
			:VCF_DEPTH(vf), VCT_DEPTH(vt), VC2_DEPTH(v2), MAX_DEPTH(md), MAX_P(mp), SEARCH_TIME(st) {
		}
		void override_config() {
			chis::VCF_DEPTH = VCF_DEPTH;
			chis::VCT_DEPTH = VCT_DEPTH;
			chis::VC2_DEPTH = VC2_DEPTH;
			chis::MAX_DEPTH = MAX_DEPTH;//设置搜索深度
			chis::MAX_P = MAX_P;//设置搜索最后一层候选着法的个数
			chis::VC_DEPTH = VCT_DEPTH + VCF_DEPTH + VC2_DEPTH;
			chis::SEARCH_TIME = SEARCH_TIME;
		}
	};
	struct int_pair {
		int first;
		int second;
	};
	////////////////////////////////
	class Board {
		///////////////横竖撇捺，4种迭代器///////////////
		/*
		共有运算符：
		‘++’：向前一个点
		‘--’：后退一个点
		‘!=’：判断是否相等
		‘=’ ：赋值
		‘*’ ：解引用，返回迭代器指向的点的值
		函数：
		in_board():判断迭代器是否在棋盘内，如果是，返回true
		itor_n(int (&_b)[30][30], int _i, int _j) :构造函数，需要int[30][30]形式的棋盘, 还有一个点的坐标。
		*/
		///////////////////////////////////////////////
		class itor_1 {//横向
		public:
			itor_1(int(&_b)[30][30], int _i, int _j) : b(_b), i(_i), j(_j) {}
			bool in_board() {
				return j >= 0 && j < SIZE;
			}
			itor_1& operator++() {
				++j;
				return *this;
			}
			itor_1& operator--() {
				--j;
				return *this;
			}
			int operator*() {
				return b[i][j];
			}
			int& operator[](int) {
				return b[i][j];
			}
			bool operator!=(itor_1 &a) {
				return j != a.j;//变化的只有j
			}
			void operator=(itor_1 &a) {
				i = a.i;
				j = a.j;
			}
			int i, j;
			int(&b)[30][30];
		};
		class itor_2 {//竖向
		public:
			itor_2(int(&_b)[30][30], int _i, int _j) : b(_b), i(_i), j(_j) {}
			bool in_board() {
				return i >= 0 && i < SIZE;
			}
			itor_2& operator++() {
				++i;
				return *this;
			}
			itor_2& operator--() {
				--i;
				return *this;
			}
			int operator*() {
				return b[i][j];
			}
			int& operator[](int) {
				return b[i][j];
			}
			bool operator!=(itor_2 &a) {
				return i != a.i;
			}
			void operator=(itor_2 &a) {
				i = a.i;
				j = a.j;
			}
			int i, j;
			int(&b)[30][30];
		};
		class itor_3 {//撇向
		public:
			itor_3(int(&_b)[30][30], int _i, int _j) : b(_b), i(_i), j(_j) {}
			bool in_board() {
				return i >= 0 && i < SIZE && j >= 0 && j < SIZE;
			}
			itor_3& operator++() {
				++i, ++j;
				return *this;
			}
			itor_3& operator--() {
				--i, --j;
				return *this;
			}
			int operator*() {
				return b[i][j];
			}
			int& operator[](int) {
				return b[i][j];
			}
			bool operator!=(itor_3 &a) {
				return i != a.i || j != a.j;
			}
			void operator=(itor_3 &a) {
				i = a.i;
				j = a.j;
			}
			int i, j;
			int(&b)[30][30];
		};
		class itor_4 {//捺向
		public:
			itor_4(int(&_b)[30][30], int _i, int _j) : b(_b), i(_i), j(_j) {}
			bool in_board() {
				return i >= 0 && i < SIZE && j >= 0 && j < SIZE;
			}
			itor_4& operator++() {
				++i, --j;
				return *this;
			}
			itor_4& operator--() {
				--i, ++j;
				return *this;
			}
			int operator*() {
				return b[i][j];
			}
			int& operator[](int) {
				return b[i][j];
			}
			bool operator!=(itor_4 &a) {
				return i != a.i || j != a.j;
			}
			void operator=(itor_4 &a) {
				i = a.i;
				j = a.j;
			}
			int i, j;
			int(&b)[30][30];
		};
	public:
		//方便读取值
		const int* operator[](int i) const {
			return b[i];
		}
		//落子函数，原则上不允许用棋子覆盖另一个棋子，但为了性能不做无用的检查
		//维护hask_key
		//维护step(move_count)
		//维护_patterns
		//...
		void move(char i, char j);
		void unmove();
		int value() {
			return default_para*(_patterns.first - _patterns.second);
		}
		int nega_value();
		int evaluate();
		//假设落子c于i，j。得到的（对点）估值。不影响棋盘本身。
		int try_set(char i, char j, int c);
		int try_set_nega(char i, char j, int c);
		int point_eval(char i, char j, int try_c);
		//得到当前棋盘的棋型统计表
		patterns_pair& get_patterns() {
			return _patterns;
		}
		//得到i，j 点的四线（横竖撇捺）棋型统计表。
		Patterns get_update(char i, char j, int try_c, int ret_color = 0);
		//从棋型表判断，是否已有定局。
		int have_winner();
		//步数统计
		int move_count() {
			return step;
		}
		int turn_color() {
			return turn;
		}
		//单独得到黑子棋型
		const Patterns& black_patterns() {
			return _patterns.first;
		}
		//单独得到白子棋型
		const Patterns& white_patterns() {
			return _patterns.second;
		}
		__int64 hash_value() const {
			//返回棋盘的64位hash值
			return hash_key;
		}
		//reset棋盘,还原设置
		void clear() {
			for(int i = 0; i < SIZE; ++i) {
				for(int j = 0; j < SIZE; ++j) {
					b[i][j] = 0;
					ps[i][j] = 0;
				}
			}
			hash_key = 0;
			step = 0;
			_patterns.reset();
			moves.clear();
			pattern_change.clear();
			turn = 1;
			CHIS_CONFIG.override_config();
		}
		bool operator==(const Board &b2) const {
			//判断棋盘是否相等
			for(int i = 0; i < SIZE; ++i) {
				for(int j = 0; j < SIZE; ++j) {
					if(b[i][j] != b2[i][j]) {
						return false;
					}
				}
			}
			return true;
		}
		bool operator<(const Board &b2) const {
			//判断棋盘顺序
			return hash_key < b2.hash_key;
		}
		int ps[30][30] = {};//点权值数组
	private:
		int b[30][30] = {};//board 棋盘
		__int64 hash_key = 0;//棋盘的64位hash值
		patterns_pair _patterns;//动态的棋型统计表
		int step = 0;//即move_count
		int turn = 1;
		//TODO：历史落子表
		Point last_set;//最近一次落子的位置
		int last_set_color;//最近一次落子的颜色
		std::vector<Point> moves;
		std::vector<patterns_pair> pattern_change;
		void moves_update();
		//更新统计表。仅在set之后调用。
		inline void update_patterns();
		template<typename itor>
		void patterns_add(Patterns *ret, itor &begin, int c);
		template<typename itor>
		void patterns_dec(Patterns *ret, itor &begin, int c);
		//基于点产生起始迭代器 (由于性lan能de问xie题daima，不提供尾后迭代器)
		//由于遍历点p所在的四条直线（横竖撇捺）
		itor_1 begin1(Point p) {
			return itor_1(b, p.x, 0);
		}
		itor_2 begin2(Point p) {
			return itor_2(b, 0, p.y);
		}
		itor_3 begin3(Point p) {
			return p.x <= p.y ?
				itor_3(b, 0, p.y - p.x) :
				itor_3(b, p.x - p.y, 0);
		}
		itor_4 begin4(Point p) {
			return p.x + p.y >= SIZE ?
				itor_4(b, p.x + p.y - (SIZE - 1), SIZE - 1) :
				itor_4(b, 0, p.x + p.y);
		}
	};
	////////////估值信息////////////
	
}
#endif