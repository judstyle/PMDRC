/**********************************************************************/
/* F2PCV - FMP -> P.M.D. data converter by JUD(T.Terata)              */
/* Definitions                                                        */
/**********************************************************************/
#ifndef FPD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "../common/f2pcvif.h"

#define DN_PART_PHELEM 12
#define DN_PART_PDELEM 11
#define DN_PART_FELEM 11
#define DN_MAX_CONTROLCODE 512
#define DH_BMESSAGES 0
#define DN_SIZE_LINEBUFFER 1024
#define DN_SIZE_STACKUNIT 64
#define DN_SIZE_MTBUNIT 64
#define DN_SIZE_PMDFOOTREAL 25
#define DN_SIZE_PMDHEADREAL 27
#define DN_SIZE_PMDHEADBUF 28
#define DN_MAX_OUTUNITSIZE 1024
#define DN_SIZE_VOICEPMD 26
#define DN_SIZE_VOICEFMP 25
#define DN_VOFFSET_OPI 28
#define DN_VOFFSET_OVI 50

#define D_VERSION_OUTPMDDATA 0x47

#define DN_SIZE_OUTBUFUNIT 512

#define D_PARTTYPE_FM 0
#define D_PARTTYPE_SSG 1
#define D_PARTTYPE_PCM 2
#define D_PARTTYPE_UK -1

#define D_OBJ_TIE ((char)(0x66))
#define D_OBJ_SLUR ((char)(0x7A))

#define DS_DEBUGFILENAMESTR "F2PCVTEST"

/*--------------------------------------------------------------------*/
/* 内部構造体(PMD) */
typedef	struct	_t_ppartstate{
	int use;		/* D_ON/D_OFF */
	int partlength;	/* 当該パートの現在の長さ(終端含む) */
	int bufsize;	/* 現在のバッファサイズ */
	short offset;	/* ＰＭＤデータに収まるときのオフセット値 */
	short nouse;
	char *buffer;	/* 当該パートのバッファアドレス */
	char *bufcur;	/* 次のバッファ書き込みアドレス */
}T_PPARTSTATE;

/* 内部構造体(FMP) */
typedef	struct	_t_fpartstate{
	int use;
	int parttype;	/* パートタイプ */
	int ppartnum;	/* 変換後のパート番号 */
	char *pdata;	/* パートデータ先頭アドレス */
	char *lladdr;	/* 'L' が存在する（べき）アドレス */
	char **lsaddr;	/* '['ループ開始記号が存在する（べき）アドレス群 */
	char **lscur;	/* アドレス群で次の値 */
	char *nextlp;	/* 次回'L'または'['が登場するアドレス */
}T_FPARTSTATE;

/* control codes */
typedef	struct	_t_tcode{
	int cont;
	int f_opelen[3];	/* 変換前のオペレーション長 */
	int f_datalen[3];	/* 変換前のデータ長 */
	int p_opelen;		/* 変換後のオペレーション長 */
	int p_datalen;		/* 変換後のデータ長 */
	char fcode[7];		/* 変換前のデータ ７バイト(7c,01系のため増量) */
	char pcode1;		/* 変換後のデータ １バイト目 */
	int convtype;		/* 0:無視 1:音符置換 2:データ付き単純置換 */
						/* 3:データなし単純置換 4:個別処理 */
}T_TCODE;








#endif
#define FPD_H
