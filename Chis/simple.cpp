#include "chis_gomoku.h"
#include <iostream>
#include <time.h>
#include <vector> 
using namespace chis;
using namespace std;
int quick_test();//快速对弈
int main() {
	//std::cout << chis::check_zobrist() << std::endl;//检查zobrist hashing数组是否重复
	quick_test();
	gomocup();
}
////////////////////////////测试相关///////////////////////////////////////
int quick_test() {
	//TODO:平衡开局
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
			std::cout << (c == 1 ? "●" : "○") << std::endl;//
			std::cout << "  0 1 2 3 4 5 6 7 8 9 1011121314" << std::endl;
			for(int i = 0; i < chis::SIZE; ++i) {
				i >= 10 ? (std::cout << i) : (std::cout << " " << i);
				for(int j = 0; j < chis::SIZE; ++j) {
					if(i == p.x && j == p.y) {
						std::cout << (c == 1 ? "◆" : "◇");
					}
					else {
						std::cout << (b[i][j] == 1 ? "●" : (b[i][j] ? "○" : (PS[i][j] > 1 ? (c == 1 ? "★" : "☆") : "■")));
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
			std::cout << "EXP: " << exp_cnt << " MAX_DEPTH：" << max_scdepth << std::endl;
			std::cout << "EVAL(ps): " << eval_cnt / ((timeout_match - time_left) / 1000.0) << std::endl;
			std::cout << "Value:" << b.value() << std::endl;
			std::cout << "Prune count: " << chis::prune_cnt << std::endl;
			std::cout << "Patterns:" << std::endl;
			std::cout
				<< "胜利" << (int)b.black_patterns().wined << " "
				<< "活四" << (int)b.black_patterns().four_l << " "
				<< "冲四" << (int)b.black_patterns().four_b << " "
				<< "活三" << (int)b.black_patterns().three_l << " "
				<< "眠三" << (int)b.black_patterns().three_s << " "
				<< "活二" << (int)b.black_patterns().two_l << " "
				<< "眠二" << (int)b.black_patterns().two_s << " "
				<< "活一" << (int)b.black_patterns().one_l << " "
				<< "眠一" << (int)b.black_patterns().one_s << std::endl;
			std::cout
				<< "胜利" << (int)b.white_patterns().wined << " "
				<< "活四" << (int)b.white_patterns().four_l << " "
				<< "冲四" << (int)b.white_patterns().four_b << " "
				<< "活三" << (int)b.white_patterns().three_l << " "
				<< "眠三" << (int)b.white_patterns().three_s << " "
				<< "活二" << (int)b.white_patterns().two_l << " "
				<< "眠二" << (int)b.white_patterns().two_s << " "
				<< "活一" << (int)b.white_patterns().one_l << " "
				<< "眠一" << (int)b.white_patterns().one_s << std::endl;
			//system("PAUSE");
			std::cout << "Hash表统计:" << std::endl;
			GetProcessMemoryInfo(chis::handle, &chis::pmc, sizeof(chis::pmc));//获取内存占用信息
			//((ptb.size()*(sizeof(chis::_depth_with_value_) + sizeof(__int64))))*0.8 / (1024.0 * 256)
			std::cout << "置换表 ： 截断次数: " << hashfinded << " 插入次数: " << hashinsert
				<< " 当前大小(大概是这么大): "
				<< (pmc.WorkingSetSize) / (1024.0 * 1024) << std::endl;
			std::cout << "PVS表:" << pvs.size() << std::endl;
			std::cout << "Alpha-Beta剪枝: " << ab_count / (((timeout_match - time_left) / 1000.0)) << endl;
			std::cout << "删除了几个必败点: " << pruning_move_count << " 统计棋型时推翻了几次结果: " << real_cnt << std::endl;
			++maxc[good_move_cnt];
			std::cout << "选择了第几个候选点：" << good_move_cnt 
				<< " 最多选了第几个/次：" << ((good_move_cnt > maxcnt) ? (maxcnt = good_move_cnt) : maxcnt) << "/" << maxc[maxcnt] << std::endl;
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
