#ifndef CHIS_CFG
#define CHIS_CFG
#include "chis_hash.h"
#include <unordered_map>
#include <algorithm>
#include <map>
#include <set>
#ifdef WIN32
	#include <windows.h>
	#include <psapi.h>
#else
	#include <sys/time.h>
	#include <sys/sysinfo.h>
#endif
#include <cstdlib>
#include <time.h>
#define CHIS_ENABLE_MTB //使用选点缓存
namespace chis
{
	using U64 = unsigned __int64;
	using U32 = unsigned __int32;
	using U16 = unsigned __int16;
	using U8 = unsigned char;
	struct Parameters;
	struct _depth_with_value_;
	struct _point_with_value;
	struct Point;
	struct chis_config;
	class _board;
	extern std::unordered_map<__int64, _depth_with_value_> ptb;
	extern std::unordered_map<__int64, std::vector<_point_with_value>> mtb[400];
	extern std::unordered_map<__int64, Point> pvs;
	///////////////////////////////
	//设置项
	extern int SIZE;//棋盘大小（小于等于30）
	extern time_t SEARCH_TIME;//搜索时间限制
	extern const int HASH_SIZE;//Hash表的大小限制(MB)
	extern int RULE;//规则种类（TODO）
	extern time_t timeout_match;//局时
	extern time_t time_left;//步时
	extern int MAX_DEPTH;//最大搜索深度
	extern int MAX_P;//限制根节点选点数
	extern int VCF_DEPTH;//VCF搜索深度
	extern int VCT_DEPTH;//VCT搜索深度
	extern int VC2_DEPTH;//VC2搜索深度
	extern int VC_DEPTH;//总搜索深度（偶数才有意义）
	extern chis_config CHIS_CONFIG;//默认设置
	//动态项
	//（不要吐槽命名……）
	extern time_t search_time;//搜索开始时间
	extern int SEARCH_DEPTH;//当前叶节点搜索深度
	extern int vc_depth;//vc
	extern bool stop_insert;//停止插入HASH表（搜索结果不可信时）
	extern int allow_expand;
	extern int allow_findvct;
	////////////DEBUG信息////////
	extern int PS[30][30];//第一层的粗略候选点分布
	extern int hashfinded;//查到置换表（截断）次数
	extern int hashinsert;//置换表的插入次数
	extern int ab_count;//AB剪枝统计
	extern int real_cnt;//棋型统计的效率（指标之一）
	extern int pruning_move_count;//必败着法删除个数
	extern int good_move_cnt;//在搜索时选择了第几个点
	extern int prune_cnt;//（安全）剪枝统计
#ifdef WIN32
	//Windows
	extern HANDLE handle;//当前进程的伪句柄
	extern PROCESS_MEMORY_COUNTERS pmc;//内存信息
	//
#endif
	extern size_t node_count;//搜索的节点数
	extern int eval_cnt;//估值次数

	extern int vct_cnt;//找到vct的次数
	extern int pvs_cnt;//pv（主要变例）插入次数
	extern int exp_cnt;//搜索延伸的次数
	extern int max_scdepth;//最大搜索深度
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
	extern const Parameters default_para;//默认估值
	extern const Parameters chs_point;//择序启发估值
	extern const Parameters chs_value;//待测估值
	extern const Parameters chs_value2;//待测估值
	extern const Parameters chs_value3;//待测估值
	void clear_hash();
	time_t time();
	size_t memcost();
}
#endif