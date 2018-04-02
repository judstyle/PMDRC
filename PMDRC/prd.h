/**********************************************************************/
/* PMDRC - P.M.D. reversal compiler by JUD(T.Terata)                  */
/* Definitions                                                        */
/**********************************************************************/
#ifndef PRD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "../common/pmdrcif.h"

/*--------------------------------------------------------------------*/
/* 基本定数 */
#define D_LAST 0
#define D_NOTLAST 1

#ifndef JST_D_ONOFF
#define D_ON 1
#define D_OFF 0
#endif
#define JST_D_ONOFF

#define DS_DEFINE_PPZEXTEND "#PPZExtend  abcdefgh\n"
#define DS_DEFINE_FM3EXTEND "#FM3Extend  XYZ\n"

#define DS_DEFINE_PZINAME   "#PPZFile    %s\n"
#define DS_DEFINE_PPSNAME   "#PPSFile    %s\n"
#define DS_DEFINE_PPCNAME   "#PCMFile    %s\n"
#define DS_DEFINE_TITLE     "#Title      %s\n"
#define DS_DEFINE_COMPOSER  "#Composer   %s\n"
#define DS_DEFINE_ARRENGER  "#Arranger   %s\n"
#define DS_DEFINE_MEMO      "#Memo       %s\n"

#define DS_OK "OK"
#define DS_NG "NG"

#define DC_ADDSYMBOL '&'
/*--------------------------------------------------------------------*/
/* 内部処理定数 */

#define DN_SIZE_LINEBUFFER 1024
#define DN_MAX_TRACKNUM 21
//#define DN_MAX_OBJSIZE 32768
#define DN_MAX_CONTROLCODE 256
#define DN_MAX_OUTUNITSIZE 1024 /* 出力バッファ増設単位 */
/* 内部テーブルメモリ増設単位 */
#define DN_GETBON 20
#define DN_GETBOC 16
#define DN_GETBOM 16
#define DN_SIZE_MTBUNIT 64
#define DN_MAX_NOTELENFREQ 192

#define DN_LENGTH_DEFLIMIT 98765

/* 標準長算出定数 */
#define DN_PCT_12P 0.05
#define DN_PCT_21P 0.9
#define DN_PCT_31P 0.7
#define DN_PCT_32P 0.2
#define DN_PCT_42P 0.3
#define DN_PCT_52P 0.25
#define DN_PCT_61P 0.1

/* バッファハンドラ */
#define DH_BMESSAGES 0
#define DH_BMML 1
#define DH_BDUMP 2
#define DH_BSDUMP 3
#define DH_BVOICE 4
#define DH_BDEFINE 5
#define DH_BLAST 6

/* ＰＭＤフォーマット定数 */
#define DC_START_CHAR_FMSSG 'A'
#define DC_START_CHAR_ADPCM 'J'
#define DC_START_CHAR_FM3EX 'X'
#define DC_START_CHAR_PPZ 'a'
#define DN_START_NUM_FMSSG 0
#define DN_START_NUM_ADPCM 9
#define DN_START_NUM_FM3EX 10
#define DN_START_NUM_PPZ 13
/* ＰＭＤデータ内のＦＭ音色データサイズ */
#define DN_FMTONE_SIZE 26
/* #MEMO の最大個数 */
#define DN_MAX_MEMOPOINT 128

/* 内部処理用コマンド定数 */
#define D_OP_TLINIT 1
#define D_OP_TLADD 2
#define D_OP_TLLSTART 3
#define D_OP_TLLEND 4
#define D_OP_TLTOTAL 5
#define D_OP_TLBEFOREL 6
#define D_OP_TLFINDL 7
#define D_OP_TLFINDCL 8
#define D_OP_TLTLEVEL 9
#define D_OP_TLLEVEL 10

/* テーブル区別用 */
#define D_LC_NOTE 1
#define D_LC_CODE 2

/* 小節線処理モード内部定数 */
#define D_BAR_UNDEFINE -1
	/* 未分別 */
#define D_BAR_FULLAUTO1 1
	/* L以降が全パート等しく、zenlenの倍数 */
#define D_BAR_FULLAUTO2 2
	/* L以降が全パート等しくなく、全パートzenlenの倍数 */
#define D_BAR_FULLAUTO3 3
	/* L以降が全パート等しくなく、最長パートが全パート長の最小公倍数 */
#define D_BAR_HALFAUTO5 4
	/* 半自動 Lが存在しないが、全パートがzenlenの倍数 */
#define D_BAR_TOPSTART 5
	/* 起点をデータ先頭に固定 /B2 が指定された場合 */

/* L以降はzenlenで、L以前はL直前から逆方向にzenlen単位で */
#define D_BAR_HALFAUTO1 101
	/* 半自動 L以降が全パート等しく、zenlenの非倍数 */
	/* 指定があればbarlenで分割 */
#define D_BAR_HALFAUTO2 102
	/* 半自動 Lが存在しないが、全パート等長 */
	/* 重み判断による自動処理 */
#define D_BAR_MANUAL 999
	/* どの法則にも当てはまらない場合、小節分割を行わない */
#define D_BAR_FORCEDMANUAL 998
	/* 強制的に禁止。小節分割を行わない */
#define D_BAR_FORCEDAUTO 997
	/* 強制的に実施。小節分割を必ず行う */

/* notes , portaments , r 28bytes V1.21 */
typedef	struct	_t_bon{
	long toffset;	/* 始点からの総長さ */
	short length;	/* オブジェクト長さ */
	char note1;		/* note/ポルタメント始点/control-code */
	char note2;		/* ポルタメント終点/control-value */
	char trans;		/* 累積トランスポーズ */
	char mtrans;	/* マスタートランスポーズ */
	char oct;		/* 現在のoct */
	char ties;		/* タイ・スラー (&/&&) */
	char barline[2];/* 小節線 */
	char nocombine;	/* 後続とのコンバイン禁止 */
	char nouse6;
	char *address;	/* コードのアドレスを保管→出力前に参照する V1.21 */
	struct _t_bon *newarea;	/* 次の領域の始点addr NULL:Empty */
	struct _t_bon *befarea;	/* 前の領域の始点addr NULL:StartPoint */
}T_BON;
/* 格納し得るコード */
/* 音符・休符、ポルタメント、タイ・スラー、空白、転調系 */

/* controlles 24bytes */
typedef	struct	_t_boc{
	long toffset;	/* 始点からの総長さ */
	char code;		/* code */
	char value;		/* 小節長さ(Z) ループ回数(]) */
	char nouse1;
	char disable;	/* D_ON:取り潰し済み */
	char barline[2];/* 小節線 */
	char nocombine;	/* 後続とのコンバイン禁止 */
	char nouse6;
	char *address;	/* コードのアドレスを保管→出力前に参照する */
	struct _t_boc *newarea;	/* 次の領域の始点addr NULL:Empty */
	struct _t_boc *befarea;	/* 前の領域の始点addr NULL:StartPoint */
}T_BOC;

/* mastertrack 16bytes */
typedef	struct	_t_bom{
	long toffset;	/* 始点からの総長さ */
	char code;
	unsigned char barlen;	/* 小節長さ */
	char nouse1;
	char last;
	struct _t_bom *newarea;	/* 次の領域の始点addr NULL:Empty */
	struct _t_bom *befarea;	/* 前の領域の始点addr NULL:StartPoint */
}T_BOM;

/* track status 32bytes x 21 */
typedef	struct	_t_tstatus{
	char use;		/* 使用する場合:D_ON 存在しない及び長さゼロ:D_OFF */
	char partname;	/* 名前は固定 A-F G-I J X-Z a-h */
	char offset;	/* パート先頭のr%1等に対応するオフセット */
	char startoct;	/* 最初にoコマンドで指定する値 */
	int partlen;	/* パートの長さ 0x80の登場するl_clock */
	T_BON *naddr;	/* notes領域のメモリ上のアドレス */
	T_BOC *caddr;	/* controlles領域のメモリ上のアドレス */
	char *text;		/* 読み込みファイルのパート先頭アドレス */
	int tracktotal;	/* トラック全体の長さ（無限ループ[]0の時は-1）*/
	int tracklenbl;	/* トラック内でLより前の長さ（Lがない場合-1）*/
	int tracklenal;	/* トラック内でLより後の長さ（Lがない場合-1）*/
}T_TRACKTABLE;

/* FM-Tone Bank 27(26)bytes */
typedef	struct	_t_fmt{
	unsigned char tnumber;	/* ToneNumber */
	unsigned char dtml[4];	/* 1/3-DT/4-ML */
	unsigned char tl[4];	/* 1/7-TL */
	unsigned char ksar[4];	/* 1/2-KS/5-AR */
	unsigned char amdr[4];	/* 2/1-AMS/5-DR */
	unsigned char sr[4];	/* 3/5-SR */
	unsigned char slrr[4];	/* 4-SL/4-RR */
	unsigned char fbal;		/* 2/3-FeedBack/3-Algorhythm */
	unsigned char nextnum;	/* ToneNumber */
}T_FMT;

/* FM-Tone Format(INTERNAL) */
typedef	struct	_t_ifmt{
	int i_ml;	/* ML */
	int i_dt;		/* DT */
	int i_ar;		/* AR */
	int i_dr;		/* DR */
	int i_sl;		/* SL */
	int i_sr;		/* SR */
	int i_rr;		/* RR */
	int i_tl;		/* TL */
	int i_ks;		/* KS */
	int i_ams;		/* AMS */
}T_IFMT;
/* FM-Tone Format(OUTPUT) */
typedef	struct	_t_ofmt{
	float f_freq;	/* ML */
	int i_det;		/* DT */
	int i_ar;		/* AR */
	int i_d1r;		/* DR */
	int i_d1l;		/* SL */
	int i_d2r;		/* SR */
	int i_rr;		/* RR */
	int i_out;		/* TL */
	int i_rs;		/* KS */
}T_OFMT;

/* control codes */
typedef	struct	_t_tcode{
	int cont;
	char code1;		/* １バイト目 */
	char code2;		/* 0x00以外のとき ２バイト目 */
	char dmusk;		/* ２バイト目を判断する逆マスク(0x80の時bit7だけを判断) */
	char nouse;		/* nouse */
	int datasize;	/* 後ろに続くデータサイズ */
	int format;		/* フォーマット */
	int level;
		/*-新フォーマット---------------------*/
		/* 処理：                             */
		/*  0:無視 */
		/*  1:expset関数をコールし、内部処理を実施(FM3Ex/PPZEx) */
		/*  2:音符 */
		/*  3:当該文字列を表示するだけ */
		/*  4:データ(1byte)を付属して表示(符号あり -128/127) */
		/*  5:データ(1byte)を付属して表示(符号なし 0/255) */
		/*  6:2bytes以上のデータを付加して表示(様式は別途) */
		/*  7:データ(1byte)を付属して表示(符号強制 -128/127) */
		/*  8:データ(1byte)を付属して表示(符号あり -128/127) ゼロ時は無出力 */
		/* レベル：                           */
		/*  0:常に非表示、中間オブジェクトにも変換しない */
		/*  1:取得を行わない */
		/*  2:グローバル情報に変換 */
		/*  3:全出力指定時のみ取得 */
		/*  4:通常表示 */
		/*  5:簡略表示時でも表示 */
		/*------------------------------------*/
	int table;		/* 書き込むtable 1:note 0:control */
	int steps;		/* D_ON/1:音長を有する場合 D_OFF/0:音長ゼロ */
	char *str;		/* MML文字列 */
	char *outformat;/* MML出力時フォーマット文字列 format=6の時のみ有効 */
					/* '#':１バイト符号あり '$':２バイト符号あり */
					/* '(':１バイト強制符号 ')':２バイト強制符号 */
					/* '_':１バイト符号なし '~':２バイト符号なし */
					/* 'a':８ビット符号なし 'b':７ビット符号なし */
					/* 'z':逆方向８ビット符号なし 'y':逆方向７ビット符号なし */
					/* ' ':１バイト表示しない */
					/* それ以外:そのまま表示 */
					/* ex) "_#$#" → 1(unsigned)/1/2/1 */
}T_TCODE;

/* システム変数 */
/* 初期化直後（未定義時）は0x00パディングする。個別に初期化するものもある */
typedef	struct	_t_ginfo{
	char c_varsion;		/* mc.exeバージョン情報 */
	char nouse1;		
	char nouse2;		
	char nouse3;		
	long l_mctotal;		/* mc.exeによるtotalclock 初期化時は-1 */
	long l_mcloop;		/* mc.exeによるloopclock 初期化時は-1 */
	char *cp_voice;		/* FM音色定義先頭アドレス */
	char *cp_dtaddr;	/* 各種テキスト定義アドレス群のアドレス */
	char *cp_pziname;	/* PPZFileのアドレス */
	char *cp_ppsname;	/* PPSFileのアドレス */
	char *cp_ppcname;	/* PPCFileのアドレス */
	char *cp_title;		/* Titleのアドレス */
	char *cp_composer;	/* Composerのアドレス */
	char *cp_arrenger;	/* Arrengerのアドレス */
	char *cps_memo[DN_MAX_MEMOPOINT];
						/* memoのアドレス配列 */
	int i_nmemo;		/* memo個数 初期化時は-1 */
	int i_fPPZExtend;	/*-PPZExtend 0:OFF 1:ON */
	int i_fFM3Extend;	/*-FM3Extend 0:OFF 1:ON */
	int i_Zcount;		/*-Zコマンドの登場回数 */
	int i_Znotonline;	/*-Zコマンドが小節線上以外に存在する 0:no 1;yes */
	int i_Fcount;		/*-Fコマンド登場回数 */
	int i_Lcount;		/*-Lコマンド登場回数 */
	int i_ILLcount;		/*-無限ループ局所ループ"]0"コマンド登場回数 */
}T_GINFO;




#endif
#define PRD_H
