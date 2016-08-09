#include "chis_gomoku.h"
#include <iostream>
#include <time.h>
#include <vector> 
#include <fstream>
using namespace chis;
using namespace std;
int quick_test2();//���ٶ���
int quick_game();//���ٶ���
void create_rev() {
	//1ABCDE->1EDCBA
	//���λ��1�����䣬ʣ�µķ�ת
	ofstream fout("ptrev.txt");
	for(unsigned i = 0; i < (1 << 7); ++i) {
		unsigned k = i;
		unsigned rev = 1;
		while(k > 1) {//
			rev = rev << 1;
			if(k % 2) {
				++rev;
			}
			k = k >> 1;
		}
		fout << (rev>>1) << ", ";
		if(!(i%20)) {
			fout << endl;
		}
	}
}
int main() {
	//std::cout << chis::check_zobrist() << std::endl;//���zobrist hashing�����Ƿ��ظ�
	//quick_test2();
	//quick_game();
	gomocup();
	
}
////////////////////////////�������///////////////////////////////////////
int quick_test2() {
	//TODO:ƽ�⿪��
	_board b(20);
	for(int i = 0; i < 15; ++i) {
		int x, y, c;
		cin >> x >> y >> c;
		x += 5;
		y += 5;
		b.make_move({(U8)x, (U8)y}, c);
	}
	b.clear();
	return 0;
}
int quick_game() {
	//TODO:ƽ�⿪��
	int maxcnt = 0;
	int maxc[100] = {};
	while(1) {
		_board b(chis::SIZE);
		int c = 1;
		//b.make_move({ 12, 7 }, b.get_turn());
		while(1) {
			Point p;
			time_t t = time();
			p = chis_move(b);
			b.make_move({ p.x, p.y }, b.get_turn());
			chis::clear_hash();
			t = time() - t;

			system("CLS");
			std::cout << (c == 1 ? "��" : "��") << std::endl;//
			std::cout << "  0 1 2 3 4 5 6 7 8 9 1011121314" << std::endl;
			for(int i = 5; i < chis::SIZE + 5; ++i) {
				i >= 10 ? (std::cout << i) : (std::cout << " " << i);
				for(int j = 5; j < chis::SIZE + 5; ++j) {
					if(i == p.x && j == p.y) {
						std::cout << (c == 1 ? "��" : "��");
					}
					else {
						std::cout << (b[i][j] == 1 ? "��" : (b[i][j] ? "��" : (PS[i][j] > 1 ? (c == 1 ? "��" : "��") : "��")));
					}
					PS[i][j] = 0;
				}
				std::cout << std::endl;
			}

			//std::cout << b.step_path() << std::endl;
			std::cout << "HASH_KEY: " << b.hash_value() << std::endl;
			std::cout << "STEP: " << b.moves_size() << " TIME: " << t << "ms" << " TIMELEFT: " << (time_left -= t) << "s" << std::endl;
			std::cout << "SPS: " << (timeout_match - time_left) *1.0 / b.moves_size() << std::endl;
			std::cout << "NPS: " << node_count / ((timeout_match - time_left) / 1000.0) << std::endl;
			std::cout << "VCT: " << vct_cnt << "  PVS: " << pvs_cnt << std::endl;
			std::cout << "EXP: " << exp_cnt << " MAX_DEPTH��" << max_scdepth << std::endl;
			std::cout << "EVAL(ps): " << eval_cnt / ((timeout_match - time_left) / 1000.0) << std::endl;
			std::cout << "Value:" << b.evaluation() << std::endl;
			std::cout << "Prune count: " << chis::prune_cnt << std::endl;
			std::cout << "Patterns:" << std::endl;
			std::cout
				<< "ʤ��" << (int)b.get_patterns().first.won << " "
				<< "����" << (int)b.get_patterns().first.four_l << " "
				<< "����" << (int)b.get_patterns().first.four_b << " "
				<< "����" << (int)b.get_patterns().first.three_l << " "
				<< "����" << (int)b.get_patterns().first.three_s << " "
				<< "���" << (int)b.get_patterns().first.two_l << " "
				<< "�߶�" << (int)b.get_patterns().first.two_s << " "
				<< "��һ" << (int)b.get_patterns().first.one_l << " "
				<< "��һ" << (int)b.get_patterns().first.one_s << std::endl;
			std::cout
				<< "ʤ��" << (int)b.get_patterns().second.won << " "
				<< "����" << (int)b.get_patterns().second.four_l << " "
				<< "����" << (int)b.get_patterns().second.four_b << " "
				<< "����" << (int)b.get_patterns().second.three_l << " "
				<< "����" << (int)b.get_patterns().second.three_s << " "
				<< "���" << (int)b.get_patterns().second.two_l << " "
				<< "�߶�" << (int)b.get_patterns().second.two_s << " "
				<< "��һ" << (int)b.get_patterns().second.one_l << " "
				<< "��һ" << (int)b.get_patterns().second.one_s << std::endl;
			//system("PAUSE");
			std::cout << "Hash��ͳ��:" << std::endl;
			GetProcessMemoryInfo(chis::handle, &chis::pmc, sizeof(chis::pmc));//��ȡ�ڴ�ռ����Ϣ
			//((ptb.size()*(sizeof(chis::_depth_with_value_) + sizeof(__int64))))*0.8 / (1024.0 * 256)
			std::cout << "�û��� �� �ضϴ���: " << hashfinded << " �������: " << hashinsert
				<< " ��ǰ��С(�������ô��): "
				<< (pmc.WorkingSetSize) / (1024.0 * 1024) << std::endl;
			std::cout << "PVS��:" << pvs.size() << std::endl;
			std::cout << "Alpha-Beta��֦: " << ab_count / (((timeout_match - time_left) / 1000.0)) << endl;
			std::cout << "ɾ���˼����ذܵ�: " << pruning_move_count << " ͳ������ʱ�Ʒ��˼��ν��: " << real_cnt << std::endl;
			++maxc[good_move_cnt];
			std::cout << "ѡ���˵ڼ�����ѡ�㣺" << good_move_cnt
				<< " ���ѡ�˵ڼ���/�Σ�" << ((good_move_cnt > maxcnt) ? (maxcnt = good_move_cnt) : maxcnt) << "/" << maxc[maxcnt] << std::endl;
			for(int i = 0; i < 10; ++i) {
				std::cout << i << ": " << maxc[i] << " ";
			}
			std::cout << std::endl;
			std::cout << "Profile:" << std::endl;
			std::cout << "hash_finder: " << hash_finder_pf << " hash_insert: " << hash_inserter_pf << std::endl;
			std::cout << "move_creator: " << move_creater_pf << " move_order: " << move_order_pf << std::endl;
			std::cout << "prunning: " << prunning_pf << " game_over: " << gm_over_pf << std::endl;
			std::cout << "vct_finder: " << vct_finder_pf << std::endl;
			if(vct_cnt) {
				//system("PAUSE");
			}
			c = -c;
			if(b.get_patterns().second.won || b.get_patterns().first.won || b.moves_size() == chis::SIZE * chis::SIZE) {
				return b.have_winner();
			}
		}
	}
	return 0;
}
