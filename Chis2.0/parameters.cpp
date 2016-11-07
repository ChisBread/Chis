#include "chis_board.h"
#include "chis_move.h"
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <ctime>
namespace chis {
	struct chis_parameters {
		int score = 0;
		Parameters par;
		bool operator<(const chis_parameters &cp)const {
			return score > cp.score;
		}
	};
	std::vector<std::pair<std::vector<Point>, int>> good_games;
	void read_good_games() {
		std::ifstream fin("to_bread.txt");
		int games;
		fin >> games;
		for(int i = 0; i < 100; ++i) {
			int board_size, moves;
			fin >> board_size >> moves;
			std::vector<Point> game;
			int avalmoves = 0;
			for(int j = 0; j < moves; ++j) {
				int movetime, x, y;
				Point move;
				fin >> x >> y >> movetime;
				move = { x + 5, y + 5};
				if(!avalmoves && movetime) {
					avalmoves = j;
				}
				game.push_back(move);
			}
			good_games.push_back({ game , avalmoves});
		}
	}
	int parameters_score(chis_parameters &cp, int testbegin, int testend) {
		default_para = cp.par;
		CHIS_CONFIG.MAX_DEPTH = 5;
		SEARCH_TIME = 2000;
		for(auto &game : good_games) {
			if(testbegin) {
				--testbegin; 
				continue;
			}
			if(!testend) break;
			--testend;
			int i = 0;
			int avl = game.second;
			_board b(20);
			for(auto m : game.first) {
				//开局下完了
				if(i >= avl) {
					Point p;
					p = chis_move(b);
					if(m == p) {
						++cp.score;
						std::cout << testend << " : moves" << i << ": " << cp.score << std::endl;
					}
				}
				b.make_move(m, b.get_turn());
				++i;
			}
		}
		return cp.score;
	}
	void adjustment() {
		srand(std::time(NULL));
		read_good_games();
		auto randin = [](int r) {
			return rand() % r;
		};
		std::vector<chis_parameters> cps;
		chis_parameters def;
		def.par = default_para;
		parameters_score(def, 0, 30);
		std::cout << "def score : " << def.score << std::endl;
		cps.push_back(def);
		for(int i = 0; i < 10; ++i) {
			chis_parameters cp;
			for(int j = 0; j < 8; ++j) {
				int temp = ((int*)(&default_para))[j];
				((int*)(&cp.par))[j] = temp + (randin(2) ? -randin(temp / 3) : randin(temp / 3));
			}
			parameters_score(cp, 0, 30);
			std::cout << i << " : " << cp.score << std::endl;
			cps.push_back(cp);
		}
		std::sort(cps.begin(), cps.end());
		std::ofstream fout("cp1.txt");
		fout << cps.size() << std::endl;
		for(auto &cp:cps) {
			fout << cp.score << " ";
			for(int j = 0; j < 8; ++j) {
				fout << ((int*)(&cp.par))[j] << " ";
			}
			fout << std::endl;
		}
	}
}