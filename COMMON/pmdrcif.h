#ifndef PMDRCIF_H
/**********************************************************************/
/* P.M.D. reversal compiler interface for PMDRC.DLL at 2004/ 5/31     */
/* IFVersion : 1.01                                                   */
/**********************************************************************/

#include "comrcif.h"

#ifndef JST_D_ONOFF
#define D_ON 1
#define D_OFF 0
#endif
#define JST_D_ONOFF

/*--------------------------------------------------------------------*/
/* 設定共通定数 */
#define DA_ON 1
#define DA_OFF 0
#define DA_AUTO 1
#define DA_FORCED 2
#define DA_TX81ZVOICE 2
#define DA_DEBUG 128
#define DA_ALL 255

/* define出力レベル定数 */
#define DA_DEFL_NORMAL 1
#define DA_DEFL_PARTEXTEND 2

/* 出力レベル定数 */
#define DN_OL_THROUGH 5
#define DN_OL_FRAME 4
#define DN_OL_NORMAL 3
#define DN_OL_DETAIL 2

/* ダンプ出力レベル定数 */
#define DN_DL_BOTHDUMP 3
#define DN_DL_SUPERDUMP 2
#define DN_DL_FRAMEDUMP 1
#define DN_DL_NODUMP 0

/* 小節線処理自動レベル定数 */
#define DN_BARSTART_AUTO 0
#define DN_BARSTART_MARKL 1
#define DN_BARSTART_TOP 2

/* オペレーション用定数(予約：未実装) */
#define DO_APTYPE_UK 0		/* 固有動作なし */

#define DS_DLLFILENAME "pmdrc.dll"
#define DS_DLLFUNCNAME "pmdrc_dllmain"
#define DS_DLLFMFUNCNAME "pmdrc_dllfreem"

/*--------------------------------------------------------------------*/
/* インターフェース構造体 V1.00 */
#define DN_MAX_OUTBUFFER 8 /* 出力バッファ数 */
typedef	struct	_t_opif{
						/*   1.00 -> 1000 / 1.23b -> 1232 */
	int i_iv;			/* 入力 インターフェースバージョン情報 */
	int i_av;			/* 入力 アプリケーションバージョン情報 */
	int i_at;			/* 入力 アプリケーションタイプ */
	int i_lv;			/* 出力 ライブラリバージョン情報 */

						/*   2:Detail,3:Normal,4:Frame,5:Through */
	int i_mmllevel;		/* 入力 MML出力レベル */
	int i_dumpmode;		/* 入力 ダンプ出力レベル 0:no 1:Dump,2:SDump,3:Both */
	int i_voicemode;	/* 入力 ＦＭ音色出力レベル 0:OFF,1:ON */
	int i_definemode;	/* 入力 define出力レベル 0:OFF,1:ON  */

	int i_zenlen;		/* 入力 zenlen値 clock表現 */
	int i_lendef;		/* 入力 標準音長 clock表現 */
	int i_lenmin;		/* 入力 最低音長 clock表現 */
	int i_len3min;		/* 入力 最低音長 clock表現（３連） */

	int i_barlevel;		/* 入力 小節線処理レベル 0:OFF,1:ON,2:Forced */
	int i_autothresh;	/* 入力 小節線処理自動レベル 0:AUTO 1:[L] 2:TOP */
	int i_autodelay;	/* 入力 自動ディレイ 0:OFF,1:ON（未実装） */
	int i_mandelay;		/* 入力 手動ディレイ clock表現（未実装） */

	int i_barlen;		/* 入力 手動小節長 clock表現（未実装） */
	int i_manoffset;	/* 入力 手動オフセット clock表現 */
	int i_datasize;		/* 入力 データサイズ */
	char *cp_indata;	/* 入力 データアドレス */

	int i_result;		/* 出力 結果 0:OK +:Warning -:Error */
	int i_directmes;	/* 入力 制御メッセージのリアルタイム出力 0:OFF 1:ON */
	int i_autolendef;	/* 入力 標準音長の自動化 0:OFF 1:ON */
	/* V1.21 2008/7/14 start */
	char c_autotrans;	/* 入力 転調をMMLに反映 */
	char c_nouse61;
	char c_nouse62;
	char c_nouse63;
	/* V1.21 2008/7/14 end */

	/* 出力バッファ 1:MML 2:Dump 3:SDump 4:Voice 5:Define 0:Messages */
	char *cps_out[DN_MAX_OUTBUFFER];
}T_OPIF;

/* 逆コンパイルメイン処理関数 */
extern T_OPIF __declspec(dllexport) *__stdcall pmdrc_dllmain( T_OPIF * );
/* 出力領域解放処理関数 */
extern T_OPIF __declspec(dllexport) *__stdcall pmdrc_dllfreem( T_OPIF * );

#endif
#define PMDRCIF_H
