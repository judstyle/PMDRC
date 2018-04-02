#ifndef F2PCVIF_H
/**********************************************************************/
/* FMP -> P.M.D. data converter interface for F2PCV.DLL               */
/**********************************************************************/

#include "comrcif.h"

#ifndef JST_D_ONOFF
#define D_ON 1
#define D_OFF 0
#endif
#define JST_D_ONOFF

/*--------------------------------------------------------------------*/
/* 設定共通定数 */

#define D_TYPE_UK 0
#define D_TYPE_OPI 2
#define D_TYPE_OVI 3
#define D_TYPE_OZI 4

#define D_TYPE_06PART 0x20
#define D_TYPE_11PART 0x40

#define DS_F2PFILENAME "f2pcv.dll"
#define DS_F2PFUNCNAME "f2pcv_dllmain"
#define DS_F2PFMFUNCNAME "f2pcv_dllfreem"
/*--------------------------------------------------------------------*/
/* インターフェース構造体 V1.10 */
#define DN_MAX_F2POUTBUFFER 4 /* 出力バッファ数 */
typedef	struct	_t_f2pif{

	int i_iv;			/* 入出力 インターフェースバージョン情報 */
	int i_directmes;	/* 入力 制御メッセージのリアルタイム出力 0:OFF 1:ON */
	int i_datatype;		/* 入出力 データファイルのタイプ(OPI/OVI/OZI) */
	int i_result;		/* 出力 結果 0:OK +:Warning -:Error */

	char *cp_indata;	/* 入力 データアドレス */
	char *cp_debugdata;	/* 入力 デバッグデータ */
	char *cp_outdata;	/* 出力 データアドレス */
	int i_datalen;		/* 出力 データサイズ */

	int i_voiceconv;	/* 入力 音色コンバート有無 0:OFF 1:ON */
	int i_nouse31;
	int i_nouse32;
	int i_nouse33;

	/* 出力バッファ 0:Messages */
	char *cps_out[DN_MAX_F2POUTBUFFER];
}T_F2PIF;

/* 逆コンパイルメイン処理関数 */
extern T_F2PIF __declspec(dllexport) *__stdcall f2pcv_dllmain( T_F2PIF * );
/* 出力領域解放処理関数 */
extern T_F2PIF __declspec(dllexport) *__stdcall f2pcv_dllfreem( T_F2PIF * );

#endif
#define F2PCVIF_H
