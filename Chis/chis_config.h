#ifndef CHIS_CFG
#define CHIS_CFG
#include "chis_hash.h"
#include <unordered_map>
#include <map>
#include <set>
#include <windows.h>
#include <psapi.h>

namespace chis
{
	struct Parameters;
	struct _depth_with_value_;
	struct _point_with_value;
	struct Point;
	struct chis_config;
	class Board;
	extern std::unordered_map<__int64, _depth_with_value_> ptb;
	extern std::unordered_map<__int64, std::vector<_point_with_value>> mtb[400];
	extern std::unordered_map<__int64, Point> pvs;
	///////////////////////////////
	//������
	extern int SIZE;//���̴�С��С�ڵ���30��
	extern int SEARCH_TIME;//����ʱ������
	extern const int HASH_SIZE;//Hash��Ĵ�С����(MB)
	extern int RULE;//�������ࣨTODO��
	extern time_t timeout_match;//��ʱ
	extern time_t time_left;//��ʱ
	extern int MAX_DEPTH;//����������
	extern int MAX_P;//���Ƹ��ڵ�ѡ����
	extern int VCF_DEPTH;//VCF�������
	extern int VCT_DEPTH;//VCT�������
	extern int VC2_DEPTH;//VC2�������
	extern int VC_DEPTH;//��������ȣ�ż���������壩
	extern chis_config CHIS_CONFIG;//Ĭ������
	//��̬��
	//����Ҫ�²�����������
	extern time_t search_time;//������ʼʱ��
	extern int SEARCH_DEPTH;//��ǰҶ�ڵ��������
	extern int vc_depth;//vc
	extern bool stop_insert;//ֹͣ����HASH���������������ʱ��
	////////////DEBUG��Ϣ////////
	extern int PS[30][30];//��һ��Ĵ��Ժ�ѡ��ֲ�
	extern int hashfinded;//�鵽�û����ضϣ�����
	extern int hashinsert;//�û���Ĳ������
	extern int ab_count;//AB��֦ͳ��
	extern int real_cnt;//����ͳ�Ƶ�Ч�ʣ�ָ��֮һ��
	extern int pruning_move_count;//�ذ��ŷ�ɾ������
	extern int good_move_cnt;//������ʱѡ���˵ڼ�����
	extern int prune_cnt;//����ȫ����֦ͳ��
	extern HANDLE handle;//��ǰ���̵�α���
	extern PROCESS_MEMORY_COUNTERS pmc;//�ڴ���Ϣ
	extern size_t node_count;//�����Ľڵ���
	extern int eval_cnt;//��ֵ����

	extern int vct_cnt;//�ҵ�vct�Ĵ���
	extern int pvs_cnt;//pv����Ҫ�������������
	extern int exp_cnt;//��������Ĵ���
	extern int max_scdepth;//����������
	//Profile
	extern unsigned __int64 pf_begin;
	extern unsigned __int64 move_creater_pf;
	extern unsigned __int64 vct_finder_pf;
	extern unsigned __int64 hash_finder_pf;
	extern unsigned __int64 hash_inserter_pf;
	extern unsigned __int64 gm_over_pf;
	extern unsigned __int64 prunning_pf;
	extern unsigned __int64 move_order_pf;
	////////////////////////////////////////////
	extern const Parameters default_para;//Ĭ�Ϲ�ֵ
	extern const Parameters chs_point;//����������ֵ
	extern const Parameters chs_value;//�����ֵ
	extern const Parameters chs_value2;//�����ֵ
	extern const Parameters chs_value3;//�����ֵ
	void clear_hash();
}
#endif