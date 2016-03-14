#include "chis_gomoku.h"
#include <iostream>
#include <time.h>
#include <vector> 
using namespace chis;
using namespace std;
int quick_test();//���ٶ���
int main() {
	//std::cout << chis::check_zobrist() << std::endl;//���zobrist hashing�����Ƿ��ظ�
	quick_test();
	gomocup();
}
////////////////////////////�������///////////////////////////////////////
int quick_test() {
	//TODO:ƽ�⿪��
	int maxcnt = 0;
	int maxc[100] = {};
	while(1) {
		Board b;
		int c = 1;
		while(1) {
			Point p;
			time_t t = time();
			p = chis_move(b);
			b.move(p.x, p.y);
			chis::clear_hash();
			t = time() - t;

			system("CLS");
			std::cout << (c == 1 ? "��" : "��") << std::endl;//
			std::cout << "  0 1 2 3 4 5 6 7 8 9 1011121314" << std::endl;
			for(int i = 0; i < chis::SIZE; ++i) {
				i >= 10 ? (std::cout << i) : (std::cout << " " << i);
				for(int j = 0; j < chis::SIZE; ++j) {
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
			std::cout << "STEP: " << b.move_count() << " TIME: " << t << "ms" << " TIMELEFT: " << (time_left -= t) << "s" << std::endl;
			std::cout << "SPS: " << (timeout_match-time_left) *1.0 / b.move_count() << std::endl;
			std::cout << "NPS: " << node_count / ((timeout_match - time_left)/1000.0) << std::endl;
			std::cout << "VCT: " << vct_cnt << "  PVS: " << pvs_cnt << std::endl;
			std::cout << "EXP: " << exp_cnt << " MAX_DEPTH��" << max_scdepth << std::endl;
			std::cout << "EVAL(ps): " << eval_cnt / ((timeout_match - time_left) / 1000.0) << std::endl;
			std::cout << "Value:" << b.value() << std::endl;
			std::cout << "Prune count: " << chis::prune_cnt << std::endl;
			std::cout << "Patterns:" << std::endl;
			std::cout
				<< "ʤ��" << (int)b.black_patterns().wined << " "
				<< "����" << (int)b.black_patterns().four_l << " "
				<< "����" << (int)b.black_patterns().four_b << " "
				<< "����" << (int)b.black_patterns().three_l << " "
				<< "����" << (int)b.black_patterns().three_s << " "
				<< "���" << (int)b.black_patterns().two_l << " "
				<< "�߶�" << (int)b.black_patterns().two_s << " "
				<< "��һ" << (int)b.black_patterns().one_l << " "
				<< "��һ" << (int)b.black_patterns().one_s << std::endl;
			std::cout
				<< "ʤ��" << (int)b.white_patterns().wined << " "
				<< "����" << (int)b.white_patterns().four_l << " "
				<< "����" << (int)b.white_patterns().four_b << " "
				<< "����" << (int)b.white_patterns().three_l << " "
				<< "����" << (int)b.white_patterns().three_s << " "
				<< "���" << (int)b.white_patterns().two_l << " "
				<< "�߶�" << (int)b.white_patterns().two_s << " "
				<< "��һ" << (int)b.white_patterns().one_l << " "
				<< "��һ" << (int)b.white_patterns().one_s << std::endl;
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
			if(b.white_patterns().wined || b.black_patterns().wined || b.move_count() == chis::SIZE * chis::SIZE) {
				return b.have_winner();
			}
		}
	}
	return 0;
}
