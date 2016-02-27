#include <iostream>
#include <sstream>
#include <time.h>
#include <vector> 
#include "chis_gomoku.h"
using namespace std;
using namespace chis;
int chis::gomocup() {
	string command;
	Board b;
	vector<int> cl;
	vector<Point> ps;

	bool last_is_board = false;
	while(1) {
		if(last_is_board && cl.back() == 2) {
			command = "BEGIN";
		}
		else {
			cin >> command;
			for(auto &i : command) {
				if(i >= 'a' && i <= 'z') {
					i += 'A' - 'a';
				}
			}
		}
		if(last_is_board) {
			b.clear();
			for(size_t i = 0; i < ps.size(); ++i) {
				b.set(ps[i].x, ps[i].y, i % 2 ? -1 : 1);
			}
			ps.clear();
			cl.clear();
			last_is_board = false;
		}
		if(command == "START") {
			b.clear();
			int size;
			cin >> size;
			if(size > 30 || size <= 5) {
				std::cout << "ERROR" << endl;
			}
			else {
				chis::SIZE = size;
				b.clear();
				std::cout << "OK" << endl;
			}
		}
		if(command == "RESTART") {
			b.clear();
			std::cout << "OK" << endl;
			//command = "BEGIN";
		}
		else if(command == "TAKEBACK") {
			int x, y;
			char dot;
			cin >> x >> dot >> y;
			if(x < 0 || x >= chis::SIZE || y < 0 || y >= chis::SIZE) {
				std::cout << "ERROR" << endl;
			}
			else {
				b.set(x, y, 0);
				Point p;
#ifndef NEGAMAX
				p = chis_move(b);
#endif
#ifdef NEGAMAX
				p = chis_move(b);
#endif
				std::cout << p.x << "," << p.y << endl;
				b.set(p.x, p.y, b.move_count() % 2 ? -1 : 1);
			}
		}
		else if(command == "BEGIN") {
			Point p;
#ifndef NEGAMAX
			p = chis_move(b);
#endif
#ifdef NEGAMAX
			p = chis_move(b);
#endif
			b.set(p.x, p.y, b.move_count() % 2 ? -1 : 1);
			std::cout << p.x << "," << p.y << endl;
		}
		else if(command == "TURN") {
			int x, y;
			char dot;
			cin >> x >> dot >> y;
			if(x < 0 || x >= chis::SIZE || y < 0 || y >= chis::SIZE) {
				std::cout << "ERROR" << endl;
			}
			else {
				Point p;
				b.set(x, y, b.move_count() % 2 ? -1 : 1);
#ifndef NEGAMAX
				p = chis_move(b);
#endif
#ifdef NEGAMAX
				p = chis_move(b);
#endif
				std::cout << p.x << "," << p.y << endl;
				b.set(p.x, p.y, b.move_count() % 2 ? -1 : 1);
			}
		}

		else if(command == "BOARD") {
			cl.clear();
			ps.clear();
			b.clear();
			last_is_board = true;
			stringstream ss;
			cin >> command;
			while(command != "DONE") {
				ss.clear();
				ss << command;
				int x, y, c;
				char dot;
				ss >> x >> dot >> y >> dot >> c;
				if(x < 0 || x >= chis::SIZE || y < 0 || y >= chis::SIZE || (c != 1 && c != 2)) {
					std::cout << "ERROR" << endl;
				}
				else {
					ps.push_back(Point(x, y));
					cl.push_back(c);
				}
				cin >> command;
			}

		}
		else if(command == "INFO") {
			string key;
			cin >> key;
			if(key == "timeout_turn") {//Ö»½ÓÊÜÂýÆå
				int value;
				cin >> value;
				if(value >= 100) {
					SEARCH_TIME = value - 170;
				}
				else {
					SEARCH_TIME = 100;
				}
				CHIS_CONFIG.SEARCH_TIME = SEARCH_TIME;
			}
			else if(key == "timeout_match") {
				cin >> chis::timeout_match;
			}
			else if(key == "max_memory") {
				int value;
				cin >> value;
				//TODO
			}
			else if(key == "time_left") {
				cin >> time_left;
			}
			else if(key == "game_type") {
				int value;
				cin >> value;
				//TODO
			}
			else if(key == "rule") {
				int value;
				cin >> value;
				//TODO
			}
			else if(key == "folder") {
				string t;
				cin >> t;
			}
		}
		else if(command == "END") {
			b.clear();
		}

	}

}