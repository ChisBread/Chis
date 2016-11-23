#ifndef _CHIS_BOARD
#define _CHIS_BOARD
#include "chis_types.h"
namespace chis {

	struct Point {
		//点类型
		U8 x, y;
		Point() {}
		Point(U8 a, U8 b) :x(a), y(b) {}
		Point(int a, int b) :x((U8)a), y((U8)b) {}
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
	struct _depth_with_value_ {
		//搜索深度与节点值
		//flag
		//0 杀局
		//1 alpha节点
		//2 pv节点
		//3 beta节点
		U8 hash_flag;
		U8 depth;
		int value;
	};
	
	struct _pattern {
		//位置不能变！（用于直接寻址）
		char one_s = 0;//0 眠一
		char one_l = 0;//1 活一
		char two_s = 0;//2 眠二
		char two_l = 0;//3 活二
		char three_s = 0;//4 眠三
		char three_l = 0;//5 活三
		char four_b = 0;//6 眠四
		char four_l = 0;//7 活四
		char won = 0;//8 胜利（啊咧，好像拼错了
		char nothing;
		_pattern& operator+=(const _pattern &a) {
			won += a.won;
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
		_pattern& operator-=(const _pattern &a) {
			won -= a.won;
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
		_pattern operator-(const _pattern &a)const {
			_pattern temp = *this;
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
			won = 0;//8 胜利（啊咧，好像拼错了
		}
	};
	struct Patterns {
		_pattern first;
		_pattern second;
		void operator+=(const Patterns &r) {
			first += r.first;
			second += r.second;
		}
		void operator-=(const Patterns &r) {
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
		int won = 10000000;
		Parameters() {}
		Parameters(int os, int ol, int ts, int tl, 
			int ths, int thl, int fb, int fl, int w)

			:one_s(os), one_l(ol), two_s(ts), two_l(tl), three_s(ths), 
			three_l(thl), four_b(fb), four_l(fl), won(w) {
		}
		int operator*(const _pattern &cnt) const {
			return
				won * cnt.won +
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
	
	struct _point_pattern_code {
		U64& operator[](size_t i) {
			return c[i];
		}
		void xor_with(U64 s) {
			for(int i = 0; i < 4; ++i) {
				c[i] ^= s << 10;
			}
		}
		U64 c[4];
	};
	struct space_score {
		int ss[4] = {};
		int& operator[](size_t i) {
			return ss[i];
		}
	};
	////////////////////////////////
	
	class patterns_map {
	public:
		patterns_map();
		U8 operator[](size_t i) const {
			return _map[i];
		}
	private:
		//高四位白，低四位黑
		U8 _map[1 << 22];
		
	};
	class zobrist_key {
		using Array64_4 = const U64(&)[64][4];
	public:
		zobrist_key();
		Array64_4 operator[](size_t i) const {
			return _rand[i];
		}
		U64 _rand[64][64][4];
		U64 rand64() {
			return 
				rand() ^ ((__int64)rand() << 15) 
				^ ((__int64)rand() << 30) 
				^ ((__int64)rand() << 45) 
				^ ((__int64)rand() << 60);
		}
	};
	////////////////////////////////
	/**
	* _board 类型
	* 维护棋盘相关的信息 包括
	* hash_key、当前移动、棋局、二进制形式的棋局编码
	* 提供四个方向的迭代器，便于从各个方向扫描棋盘
	**/
	class _board {
	public:
		class board_itor;
		class board_itor_1;//横
		class board_itor_2;//竖
		class board_itor_3;//撇
		class board_itor_4;//捺
		struct _candidate {
			Point p;//不变
			Patterns pat;//只在点被选中时维护
			_point_pattern_code ppc;//一直维护
		};
		//typedef const  U8(&Array64)[64];
		using Array64 = const U64(&)[64];
	public:
		
		_board(size_t s);
		Array64 operator[](size_t i) const {
			return board[i];
		}
		U64 hash_value() const {
			//返回棋盘的64位hash值
			return hash_key;
		}
		_point_pattern_code get_patcode(U64 x, U64 y);
		void xor_with_pcode(U64 x, U64 y);
		const Patterns& get_patterns() const {
			return pat;
		}
		void sync_candidate_root();
		void sync_candidate();
		int have_winner();
		int evaluation();
		U64 get_turn() const {
			return turn;
		}
		const std::vector<std::pair<Point, U64>>& get_moves() const {
			return moves;
		}
		size_t moves_size() const {
			return moves.size();
		}
		std::vector<_point_with_value> get_pruned_moves_black_root();
		std::vector<_point_with_value> get_pruned_moves_white_root();
		//如果无法剪枝，返回完整的有序候选着法
		std::vector<_point_with_value> get_pruned_moves_black();
		std::vector<_point_with_value> get_pruned_moves_white();
		//如果没有符合条件的，返回空
		std::vector<_point_with_value> get_pruned_moves_black_forvct();
		std::vector<_point_with_value> get_pruned_moves_white_forvct();
		std::vector<_point_with_value> get_pruned_moves_black_forvcf();
		std::vector<_point_with_value> get_pruned_moves_white_forvcf();
		std::vector<_point_with_value> get_pruned_moves_black_fordefend();
		std::vector<_point_with_value> get_pruned_moves_white_fordefend();
		void make_move(Point p, U64 c);
		void unmove();
		void clear() {
			while(!moves.empty()) {
				unmove();
			}
			turn = BLK;
		}
	private:
		static const zobrist_key zob;
		static const patterns_map pmap;

		const U64 begin_index = 5;//5格边界
		const U64 board_size;
		const U64 _size;
		const U64 end_index;

		U64 turn = BLK;
		
		space_score ss;
		U64 board[64][64];
		_candidate cand[64][64];
		int weight[64][64] = {};
		bool synced[64][64] = {};
		// 四个方向上，整条直线的棋型编码
		U64 pcode[4][64];
		Patterns pat;
		U64 hash_key = 0;
		std::vector<_candidate> candidate_moves;
		std::vector<std::pair<Point, U64>> moves;
		std::vector<Patterns> pat_moves;
		std::vector<space_score> ss_moves;
	};

	class _board::board_itor {
	public:
		board_itor(_board &_b, U64 _i, U64 _j) : b(_b), i(_i), j(_j) {}
		bool in_board() const {
			return i >= b.begin_index && i < b.end_index
				&& j >= b.begin_index && j < b.end_index;
		}
		U64 operator*() const {
			return b.board[i][j];
		}
		void operator=(const board_itor &b) {
			i = b.i;
			j = b.j;
		}
		bool operator==(const board_itor &b) const {
			return i == b.i && j == b.j;
		}
		bool operator!=(const board_itor &b) const {
			return i != b.i || j != b.j;
		}
		U64 i, j;
		_board &b;
	};

	class _board::board_itor_1 :public board_itor{//横向
	public:
		board_itor_1(_board &_b, U64 _i, U64 _j) : board_itor(_b, _i, _j) {}
		board_itor_1& operator++() {
			++j;
			return *this;
		}
		board_itor_1& operator--() {
			--j;
			return *this;
		}
		void reset() {
			j = 0;
		}
		board_itor_1 end() {
			return board_itor_1(b, i, b.end_index + 5);
		}
		void xor_with_pcode() {
			//
			b.pcode[0][i] ^= (b[i][j] << (j << 1));
		}
		//取得22位的棋型编码
		U64 get_pcode() {
			return (b.pcode[0][i] >> ((j << 1) - 10)) & (U64)((1 << 22) - 1);
		}
		static U64 get_pcode(_board &b, U64 i, U64 j) {
			return (b.pcode[0][i] >> ((j << 1) - 10)) & (U64)((1 << 22) - 1);
		}
		static void xor_with_pcode(_board &b, U64 i, U64 j) {
			b.pcode[0][i] ^= (b[i][j] << (j << 1));
		}
	};
	class _board::board_itor_2 :public board_itor{//竖向

	public:
		board_itor_2(_board &_b, U64 _i, U64 _j) : board_itor(_b, _i, _j) {}
		board_itor_2& operator++() {
			++i;
			return *this;
		}
		board_itor_2& operator--() {
			--i;
			return *this;
		}
		void reset() {
			i = 0;
		}
		board_itor_2 end() {
			return board_itor_2(b, b.end_index + 5, j);
		}
		void xor_with_pcode() {
			//
			b.pcode[1][j] ^= (b[i][j] << (i << 1));
		}
		//取得22位的棋型编码
		U64 get_pcode() {
			return (b.pcode[1][j] >> ((i << 1) - 10)) & (U64)((1 << 22) - 1);
		}
		static U64 get_pcode(_board &b, U64 i, U64 j) {
			return (b.pcode[1][j] >> ((i << 1) - 10)) & (U64)((1 << 22) - 1);
		}
		static void xor_with_pcode(_board &b, U64 i, U64 j) {
			//
			b.pcode[1][j] ^= (b[i][j] << (i << 1));
		}
	};
	class _board::board_itor_3 :public board_itor{//撇向
	public:
		board_itor_3(_board &_b, U64 _i, U64 _j) : board_itor(_b, _i, _j) {}
		board_itor_3& operator++() {
			++i, ++j;
			return *this;
		}
		board_itor_3& operator--() {
			--i, --j;
			return *this;
		}
		void reset() {
			i > j ? (j = 0, i -= j) : (i = 0, j -= i);
		}
		board_itor_3 end() {
			auto b_end = b.end_index + 5;
			return
				i > j ?
				board_itor_3(b, b_end, b_end + (j - i)) :
				board_itor_3(b, b_end + (i - j), b_end);
		}
		void xor_with_pcode() {
			//第几撇
			size_t index = i >= j ? i - j : j - i + b.end_index + 4;//左下0开始，右上size开始
			//第几个元素
			U64 biti = i > j ? j : i;
			b.pcode[2][index] ^= (b[i][j] << (biti << 1));
		}
		//取得22位的棋型编码
		U64 get_pcode() {
			//第几撇
			size_t index = i >= j ? i - j : j - i + b.end_index + 4;//左下0开始，右上size开始
			//第几个元素
			U64 biti = i > j ? j : i;
			return (b.pcode[2][index] >> ((biti << 1) - 10)) & (U64)((1 << 22) - 1);
		}
		static U64 get_pcode(_board &b, U64 i, U64 j) {
			//第几撇
			size_t index = i >= j ? i - j : j - i + b.end_index + 4;//左下0开始，右上size开始
			//第几个元素
			U64 biti = i > j ? j : i;
			return (b.pcode[2][index] >> ((biti << 1) - 10)) & (U64)((1 << 22) - 1);
		}
		static void xor_with_pcode(_board &b, U64 i, U64 j) {
			//第几撇
			size_t index = i >= j ? i - j : j - i + b.end_index + 4;//左下0开始，右上size开始
			//第几个元素
			U64 biti = i > j ? j : i;
			b.pcode[2][index] ^= (b[i][j] << (biti << 1));
		}
	};
	class _board::board_itor_4 :public board_itor{//捺向
	public:
		board_itor_4(_board &_b, U64 _i, U64 _j) : board_itor(_b, _i, _j) {}
		board_itor_4& operator++() {
			++i, --j;
			return *this;
		}
		board_itor_4& operator--() {
			--i, ++j;
			return *this;
		}
		void reset() {
			auto b_back = b.end_index + 4;
			i + j > b_back ? (i = i + j - b_back, j = b_back) : (i = 0, j = i + j);
		}
		board_itor_4 end() {
			auto b_back = b.end_index + 4;
			return
				i + j > b_back ?
				board_itor_4(b, b_back + 1, i + j - (b_back + 1)) :
				board_itor_4(b, i + j + 1, -1);
		}
		void xor_with_pcode() {
			auto b_back = b.end_index + 4;
			//第几捺
			size_t index = i + j;//
			//第几个元素
			U64 biti = i + j > b_back ? b_back - j : i;
			b.pcode[3][index] ^= (b[i][j] << (biti << 1));
		}
		//取得22位的棋型编码
		U64 get_pcode() {
			auto b_back = b.end_index + 4;
			//第几捺
			size_t index = i + j;//
			//第几个元素
			U64 biti = i + j > b_back ? b_back - j : i;
			return (b.pcode[3][index] >> ((biti << 1) - 10)) & (U64)((1 << 22) - 1);
		}
		static U64 get_pcode(_board &b, U64 i, U64 j) {
			auto b_back = b.end_index + 4;
			//第几捺
			size_t index = i + j;//
			//第几个元素
			U64 biti = i + j > b_back ? b_back - j : i;
			return (b.pcode[3][index] >> ((biti << 1) - 10)) & (U64)((1 << 22) - 1);
		}
		static void xor_with_pcode(_board &b, U64 i, U64 j) {
			auto b_back = b.end_index + 4;
			//第几捺
			size_t index = i + j;//
			//第几个元素
			U64 biti = i + j > b_back ? b_back - j : i;
			b.pcode[3][index] ^= (b[i][j] << (biti << 1));
		}
	};
}
#endif