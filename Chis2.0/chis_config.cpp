#include "chis_config.h"
#include "chis_board.h"

namespace chis {
	///////////////��ֵ/////////////////
	const Parameters default_para;
	const Parameters chs_point(1, 4, 4, 16, 16, 64, 64, 256, 10000);//ѡ���ֵ
	const Parameters chs_value(5, 45, 56, 280, 320, 360, 400, 520, 10000);//�����ֵ
	const Parameters chs_value2(5, 45, 56, 280, 320, 460, 600, 1024, 10000);//�����ֵ
	const Parameters chs_value3(15, 65, 70, 280, 310, 450, 650, 1024, 10000);//�����ֵ
	/////////////�ؼ�����////////////
	std::unordered_map<__int64, _depth_with_value_> ptb;//�û���
	std::unordered_map<__int64, std::vector<_point_with_value>> mtb[400];//������������ʱ���ã�
	std::unordered_map<__int64, Point> pvs;//��ʷ��
	/////////////�������/////////////
	//������
	int SIZE = 15;//���̴�С
	int RULE = 0;//�������ͣ�ֻ֧��freestyle��
	time_t timeout_match = 180000;//������ʱ������
	time_t time_left = 180000;//ʣ�µ�ʱ��
	time_t SEARCH_TIME = 29990;//һ������ʱ������
	int MAX_DEPTH = 9;
	int MAX_P = 24;
	int VCF_DEPTH = 2;
	int VCT_DEPTH = 10;
	int VC2_DEPTH = 0;
	int VC_DEPTH = VCT_DEPTH + VC2_DEPTH + VCF_DEPTH;
	chis_config CHIS_CONFIG(VCF_DEPTH, VCT_DEPTH, VC2_DEPTH, MAX_DEPTH, MAX_P, SEARCH_TIME);//Ĭ������
	const int HASH_SIZE = 270;//hash���С��MB)

	//��̬�� 
	time_t search_time;//
	int SEARCH_DEPTH;
	int vc_depth = 1;
	bool stop_insert = false;
	int allow_expand = true;
	int allow_findvct = true;
	/////////////debug���///////////
	int PS[30][30] = {};//ѡ��
	int hashfinded = 0;
	int hashinsert = 0;
	int pruning_move_count = 0;
	int real_cnt = 0;
	int good_move_cnt;
	int prune_cnt = 0;
	int ab_count = 0;
	int vct_cnt = 0;
	int exp_cnt = 0;
	int eval_cnt = 0;
	int pvs_cnt = 0;
	int max_scdepth = 0;
	size_t node_count = 0;
	//Profile
	unsigned __int64 pf_begin = 0;
	unsigned __int64 move_creater_pf = 0;
	unsigned __int64 vct_finder_pf = 0;
	unsigned __int64 hash_finder_pf = 0;
	unsigned __int64 hash_inserter_pf = 0;
	unsigned __int64 gm_over_pf = 0;
	unsigned __int64 prunning_pf = 0;
	unsigned __int64 move_order_pf = 0;
	
	//���hash��
	void clear_hash() {
#ifdef CHIS_ENABLE_MTB
		for(int i = 0; i < 400; ++i) {
			mtb[i].clear();
		}
#endif
		ptb.clear();
		pvs.clear();
	}
#ifdef WIN32
	//Windows
	HANDLE handle = GetCurrentProcess();//���
	PROCESS_MEMORY_COUNTERS pmc;
	time_t time() {
		SYSTEMTIME t;
		GetLocalTime(&t);

		return (t.wMinute * 60 + t.wSecond) * 1000 + t.wMilliseconds;
	}
	size_t memcost() {
		
		GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));//��ȡ�ڴ�ռ����Ϣ
		return pmc.WorkingSetSize;
	}
}
#else
	time_t time(){
		struct timeval tv;
		gettimeofday(&tv,NULL);    
		return tv.tv_sec * 1000 + tv.tv_usec / 1000;
	}
	size_t memcost() {
		size_t size = (size_of(__int64) + sizeof(_depth_with_value_)) * ptb.max_size();
		size += (size_of(__int64) + sizeof(Point)) * pvs.max_size();
#ifdef CHIS_ENABLE_MTB
		for(int i = 0; i < 400; ++i) {
			size += 
				(size_of(__int64) 
				+ sizeof(std::vector<_point_with_value>) + 50 * size_of(_point_with_value))
				* mtb[i].max_size();
		}
#endif
		return size / 1024;
	}
#endif