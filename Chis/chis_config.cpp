#include "chis_config.h"
#include "gomoku_board.h"


using namespace chis;
///////////////��ֵ/////////////////
const Parameters chis::default_para;
const Parameters chis::chs_point(1, 4, 4, 16, 16, 64, 64, 256, 10000);//ѡ���ֵ
const Parameters chis::chs_value(5, 45, 56, 280, 320, 360, 400, 520, 10000);//�����ֵ
const Parameters chis::chs_value2(5, 45, 56, 280, 320, 460, 600, 1024, 10000);//�����ֵ
const Parameters chis::chs_value3(15, 65, 70, 280, 310, 450, 650, 1024, 10000);//�����ֵ
/////////////�ؼ�����////////////
std::unordered_map<__int64, _depth_with_value_> chis::ptb;//�û���
std::unordered_map<__int64, std::vector<_point_with_value>> chis::mtb[400];//������������ʱ���ã�
std::unordered_map<__int64, Point> chis::pvs;//��ʷ��
/////////////�������/////////////
//������
int chis::SIZE = 15;//���̴�С
int chis::RULE = 0;//�������ͣ�ֻ֧��freestyle��
time_t chis::timeout_match = 180000;//������ʱ������
time_t chis::time_left = 180000;//ʣ�µ�ʱ��
int chis::SEARCH_TIME = 29990;//һ������ʱ������
int chis::MAX_DEPTH = 7;
int chis::MAX_P = 24;
int chis::VCF_DEPTH = 2;
int chis::VCT_DEPTH = 10;
int chis::VC2_DEPTH = 0;
int chis::VC_DEPTH = VCT_DEPTH + VC2_DEPTH + VCF_DEPTH;
chis_config chis::CHIS_CONFIG(VCF_DEPTH, VCT_DEPTH, VC2_DEPTH, MAX_DEPTH, MAX_P, SEARCH_TIME);//Ĭ������
const int chis::HASH_SIZE = 325;//hash���С��MB)

//��̬�� 
time_t chis::search_time;//
int chis::SEARCH_DEPTH;
int chis::vc_depth = 1;
bool chis::stop_insert = false;
/////////////debug���///////////
int chis::PS[30][30] = {};//ѡ��
int chis::hashfinded = 0;
int chis::hashinsert = 0;
int chis::pruning_move_count = 0;
int chis::real_cnt = 0;
int chis::good_move_cnt;
int chis::prune_cnt = 0;
int chis::ab_count = 0;
int chis::vct_cnt = 0;
int chis::exp_cnt = 0;
int chis::eval_cnt = 0;
int chis::pvs_cnt = 0;
int chis::max_scdepth = 0;
size_t chis::node_count = 0;
//Profile
unsigned __int64 chis::pf_begin = 0;
unsigned __int64 chis::move_creater_pf = 0;
unsigned __int64 chis::vct_finder_pf = 0;
unsigned __int64 chis::hash_finder_pf = 0;
unsigned __int64 chis::hash_inserter_pf = 0;
unsigned __int64 chis::gm_over_pf = 0;
unsigned __int64 chis::prunning_pf = 0;
unsigned __int64 chis::move_order_pf = 0;
HANDLE chis::handle;//���
PROCESS_MEMORY_COUNTERS chis::pmc;
//���hash��
void chis::clear_hash() {
	chis::ptb.clear();
	chis::pvs.clear();
}