#include "sillychis.h"
#include <iostream>
#include <string>
#include <strstream>
#include <ctime>
#include <thread>
using namespace std;
using namespace chis;
void quick_game();
int gomocup();
int main() {
	gomocup();
}
void quick_game() {
	silly_chis<HASH_MB(128)> sc(15);
	while(!(sc.bod.get_patterns().first.won || sc.bod.get_patterns().second.won)) {
		auto t = time(NULL);
		sc.bod.make_move(sc.chis_move(), sc.bod.get_turn());
		t = time(NULL) - t;
		system("CLS");
		cout << "  0 1 2 3 4 5 6 7 8 9 1011121314" << endl;
		for(int i = 0; i < 15; ++i) {
			cout << (i<10 ? " " : "") << i;
			for(int j = 0; j < 15; ++j) {
				cout << (sc.bod[i + 5][j + 5] ? (sc.bod[i + 5][j + 5] == BLK ? "¡ñ" : "¡ð") : ("¡ö"));
			}
			cout << endl;
		}
		cout << t << "s" << endl;
	}
	system("PAUSE");
}
int gomocup() {
	string command;
	silly_chis<HASH_MB(128)> sc(20);
	thread move([&]() {
		return sc.chis_move();
	});

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
			sc.bod.clear();
			for(size_t i = 0; i < ps.size(); ++i) {
				sc.bod.make_move({ ps[i].x, ps[i].y }, sc.bod.get_turn());
			}
			ps.clear();
			cl.clear();
			last_is_board = false;
		}
		if(command == "START") {
			sc.bod.clear();
			int size;
			cin >> size;
			if(size > 30 || size <= 5) {
				std::cout << "ERROR" << endl;
			}
			else {
				std::cout << "OK" << endl;
			}
		}
		if(command == "RESTART") {
			sc.bod.clear();
			std::cout << "OK" << endl;
			//command = "BEGIN";
		}
		else if(command == "TAKEBACK") {
			int x, y;
			char dot;
			cin >> x >> dot >> y;
			if(x < 0 || x >= 20 || y < 0 || y >= 20) {
				std::cout << "ERROR" << endl;
			}
			else {
				sc.bod.unmove();
				Point p;
				p = sc.chis_move();
				std::cout << (int)p.x - 5 << "," << (int)p.y - 5 << endl;
				sc.bod.make_move({ p.x, p.y }, sc.bod.get_turn());
			}
		}
		else if(command == "BEGIN") {
			Point p;
			p = sc.chis_move();
			sc.bod.make_move({ p.x, p.y }, sc.bod.get_turn());
			std::cout << (int)p.x - 5 << "," << (int)p.y - 5 << endl;
		}
		else if(command == "TURN") {
			int x, y;
			char dot;
			cin >> x >> dot >> y;
			if(x < 0 || x >= 20 || y < 0 || y >= 20) {
				std::cout << "ERROR" << endl;
			}
			else {
				Point p;
				sc.bod.make_move({ (U8)(x + 5), (U8)(y + 5) }, sc.bod.get_turn());
				p = sc.chis_move();
				std::cout << (int)p.x - 5 << "," << (int)p.y - 5 << endl;
				sc.bod.make_move({ p.x, p.y }, sc.bod.get_turn());
			}
		}

		else if(command == "BOARD") {
			cl.clear();
			ps.clear();
			sc.bod.clear();
			last_is_board = true;
			strstream ss;
			cin >> command;
			while(command != "DONE") {
				ss.clear();
				ss << command;
				int x, y, c;
				char dot;
				ss >> x >> dot >> y >> dot >> c;
				if(x < 0 || x >= 20 || y < 0 || y >= 20 || (c != 1 && c != 2)) {
					std::cout << "ERROR" << endl;
				}
				else {
					ps.push_back(Point(x + 5, y + 5));
					cl.push_back(c);
				}
				cin >> command;
			}

		}
		else if(command == "INFO") {
			string key;
			int value;
			cin >> key;
			if(key == "timeout_turn") {//Ö»½ÓÊÜÂýÆå
				
				cin >> value;
			}
			else if(key == "timeout_match") {
				cin >> value;
			}
			else if(key == "max_memory") {
				cin >> value;
				//TODO
			}
			else if(key == "time_left") {
				cin >> value;
			}
			else if(key == "game_type") {
				cin >> value;
				//TODO
			}
			else if(key == "rule") {
				cin >> value;
				//TODO
			}
			else if(key == "folder") {
				string t;
				cin >> t;
			}
		}
		else if(command == "END") {
			sc.bod.clear();
		}

	}

}