#ifndef CHIS_BOARD_HASH
#define CHIS_BOARD_HASH
namespace chis {
	bool check_zobrist();
	extern __int64 zobrist[2][30 * 30];
}
#endif