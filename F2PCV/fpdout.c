/**********************************************************************/
/* PMDRC - P.M.D. reversal compiler by JUD(T.Terata)                  */
/* Output functions                                                   */
/**********************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "fpd.h"
#include "fpdext.h"



/*--------------------------------------------------------------------*/
/* 解析 ループ終点出力 */
int outsloopend( char *cp_in , int i_part ){

/* 変換前(FMP) */
/* オペレート、アドレス（２）、レジスタ、回数 */
/* 変換後(PMD) */
/* オペレート、回数、回数、アドレス（２） */

	outpmd( i_part , (char)0xF8 );
	outpmd( i_part , (char)(*(cp_in+4)) );
	outpmd( i_part , (char)(*(cp_in+4)) );
	outpmd( i_part , (char)0x00 );
	outpmd( i_part , (char)0x00 );

	return(0);
}
/*--------------------------------------------------------------------*/
/* 解析 ポルタメント出力 */
int outsporta( char *cp_in , int i_part , int i_deflen ){

	char *cp_cur;
	int i_tmplen;
	T_TCODE *tp_tcode;

/* 変換前(FMP) */
/* オペレート、音階２、Ａウェイト(-2)、速度、変化量、いつ来るか判らん音符１ */
/* 変換後(PMD) */
/* 音階１、Ａ音長、＆、オペレート、音階１、音階２、音長 */

	/* 変換前のウェイトが変換前の音符１より長い場合は省略する処理 */
	/*  */

	/* 最初の音符 */
	cp_cur = cp_in;
	cp_cur += 5;
	/* searchbycodeのパートタイプは、気にしない */
	/* その理由は、音階だからパートタイプに左右されないということ */
	tp_tcode = searchbycode( cp_cur , D_PARTTYPE_FM );
	outpmd( i_part , tp_tcode->pcode1 );
	if( tp_tcode->f_datalen[D_PARTTYPE_FM] == 0 ){
		/* 音長省略音符 */
		i_tmplen = i_deflen;
	}else{
		/* 通常の音符 */
		cp_cur = cp_in;
		cp_cur++;
		i_tmplen = (int)((unsigned char)(*cp_cur));
	}
	outpmd( i_part , (char)i_tmplen ); /* ウェイト長 */
	outpmd( i_part , 0xFB ); /* ＆ */

	/* ポルタメント本体 */
	outpmd( i_part , 0xDA ); /* ポルタメント */
	outpmd( i_part , tp_tcode->pcode1 ); /* 第一音階 */
	cp_cur = cp_in;
	cp_cur++;
	tp_tcode = searchbycode( cp_cur , D_PARTTYPE_FM );
	outpmd( i_part , tp_tcode->pcode1 ); /* 第二音階 */
	cp_cur = cp_in;
	cp_cur++;
	i_tmplen = (int)((unsigned char)(*cp_cur)) - i_tmplen;
	outpmd( i_part , (char)i_tmplen ); /* ポルタメント長 */

	return( 0 );
}
/*--------------------------------------------------------------------*/
/* 解析 ＰＭＤフッタ作成 24bytes */
/* 返却値：各パートのデータサイズ合計 */
int makepmdfoot( char *cp_out , int i_totalpartsize , int i_FMparam ){

#define DN_SIZE_PMDFOOTR0 4
#define DN_SIZE_PMDFOOTR1 7
#define DN_SIZE_PMDFOOTR2 14
typedef	struct	_t_pmdfoot0{
	short olistoffset;
	char c_version;
	char c_s1;
}T_PMDFOOT0;
typedef	struct	_t_pmdfoot1{
	char c_ft1; /* voice */
	char c_ft2;
	char c_pps;
	char c_ppc;
	char c_title;
	char c_comp;
	char c_arr;
}T_PMDFOOT1;
typedef	struct	_t_pmdfoot2{
	char cs_offset_pps[2];
	char cs_offset_ppc[2];
	char cs_offset_title[2];
	char cs_offset_comp[2];
	char cs_offset_arr[2];
	char cs_offset_memo[2];
	char c_t1;
	char c_t2;
}T_PMDFOOT2;

	int i_startoffset;
	int i_addrlistoffset;
	int i_addrlist1st;
	T_PMDFOOT0 t_lpfoot0;
	T_PMDFOOT1 t_lpfoot1;
	T_PMDFOOT2 t_lpfoot2;
	int i_count;
	unsigned short us_ocur;
	int i_memolen;
	char *cp_ocur;

	/* 書き出しアドレスを算出する */
	/* ヘッダサイズ + 本文サイズ */
	i_startoffset = DN_SIZE_PMDHEADREAL + i_totalpartsize;
	i_addrlistoffset = i_startoffset + 11 - 1;
	i_addrlist1st = i_startoffset + 6 - 1;

	memset( &t_lpfoot0 , 0x00 , sizeof(T_PMDFOOT1) );
	memset( &t_lpfoot1 , 0x00 , sizeof(T_PMDFOOT1) );
	memset( &t_lpfoot2 , 0x00 , sizeof(T_PMDFOOT2) );

	/* アドレス群オフセット */
	t_lpfoot0.olistoffset =
		(short)i_addrlistoffset + ((short)DN_SIZE_VOICEPMD*i_FMparam);
	if( gcp_comment != NULL ){
		i_memolen = strlen(gcp_comment)+1;
		t_lpfoot0.olistoffset += (short)(i_memolen);
	}

	/* MC.EXE バージョン */
	t_lpfoot0.c_version = D_VERSION_OUTPMDDATA;
	/* 0xFE */
	t_lpfoot0.c_s1 = 0xFE;

	/* ＦＭ音色領域（ターミネータのみ） */
	t_lpfoot1.c_ft1 = 0x00;
	t_lpfoot1.c_ft2 = 0xFF;

	us_ocur = i_addrlist1st;
	us_ocur += ((short)DN_SIZE_VOICEPMD*i_FMparam); /* V1.01c ＦＭ音色追加分 */
	t_lpfoot2.cs_offset_pps[0] = us_ocur%256;
	t_lpfoot2.cs_offset_pps[1] = us_ocur/256;
	us_ocur++;
	t_lpfoot2.cs_offset_ppc[0] = us_ocur%256;
	t_lpfoot2.cs_offset_ppc[1] = us_ocur/256;
	us_ocur++;
	t_lpfoot2.cs_offset_title[0] = us_ocur%256;
	t_lpfoot2.cs_offset_title[1] = us_ocur/256;
	us_ocur++;
	t_lpfoot2.cs_offset_comp[0] = us_ocur%256;
	t_lpfoot2.cs_offset_comp[1] = us_ocur/256;
	us_ocur++;
	t_lpfoot2.cs_offset_arr[0] = us_ocur%256;
	t_lpfoot2.cs_offset_arr[1] = us_ocur/256;
	if( gcp_comment != NULL ){
		us_ocur++;
		t_lpfoot2.cs_offset_memo[0] = us_ocur%256;
		t_lpfoot2.cs_offset_memo[1] = us_ocur/256;
	}

	cp_ocur = cp_out;
	memcpy( cp_ocur , (char *)(&t_lpfoot0) , DN_SIZE_PMDFOOTR0 );
	cp_ocur += DN_SIZE_PMDFOOTR0;

	/* ＦＭ音色データを割り込ます */
	if( i_FMparam > 0 ){
		memcpy( cp_ocur , gcp_FMparam , DN_SIZE_VOICEPMD*i_FMparam );
		cp_ocur += DN_SIZE_VOICEPMD*i_FMparam;
	}

	memcpy( cp_ocur , (char *)(&t_lpfoot1) , DN_SIZE_PMDFOOTR1 );
	cp_ocur += DN_SIZE_PMDFOOTR1;
	if( gcp_comment != NULL ){
		memcpy( cp_ocur , gcp_comment , i_memolen );
		cp_ocur += i_memolen;
	}
	memcpy( cp_ocur , (char *)(&t_lpfoot2) , DN_SIZE_PMDFOOTR2 );

	return( 0 );
}
/*--------------------------------------------------------------------*/
/* 解析 ＰＭＤヘッダ作成 25bytes(DN_SIZE_PMDHEADREAL) */
/* 返却値：各パートのデータサイズ合計 */
int makepmdhead( char *cp_out ){

/* 内部構造体(FMP) */
typedef	struct	_t_pmdhead{
	short loffset[DN_PART_PHELEM+1];
}T_PMDHEAD;

	char *cp_cur;
	T_PMDHEAD t_phead;
	short s_sumsize;
	int i_count;

	/* オフセット求め */
	s_sumsize=0x1A;
	for(i_count=0;i_count<DN_PART_PHELEM;i_count++){
		ts_tsp[i_count].offset = s_sumsize;
		s_sumsize += ts_tsp[i_count].partlength;
		t_phead.loffset[i_count] = ts_tsp[i_count].offset;
	}
	t_phead.loffset[DN_PART_PHELEM] = s_sumsize+4;

	/* ヘッダ転載（先頭が１バイト0x00として存在するためこうする） */
	if( cp_out != NULL ){
		/* 出力領域がＮＵＬＬの場合はサイズを測定するだけ */
		cp_cur = cp_out;
		*cp_cur = 0x00;
		cp_cur++;
		memcpy( cp_cur , &t_phead , (DN_PART_PHELEM+1)*sizeof(short) );
	}

	return( (int)(s_sumsize-0x1A) );
}
/*--------------------------------------------------------------------*/
/* 解析 ＰＭＤバッファに書き込む */
/* 引数１：ＰＭＤパート番号 */
/* 引数２：書き込むデータ */
int outpmd( int i_part , char c_in ){

	char *cp_tmp;

	if( ts_tsp[i_part].bufsize == 0 ){
		/* 初書き込み */
		ts_tsp[i_part].buffer = malloc( DN_SIZE_OUTBUFUNIT );
		ts_tsp[i_part].bufcur = ts_tsp[i_part].buffer;
		ts_tsp[i_part].bufsize = DN_SIZE_OUTBUFUNIT;
	}else if( ts_tsp[i_part].bufsize <= ts_tsp[i_part].partlength ){
		/* バッファサイズ不足 */

		ts_tsp[i_part].buffer = realloc( ts_tsp[i_part].buffer ,
			DN_SIZE_OUTBUFUNIT + ts_tsp[i_part].bufsize );
		ts_tsp[i_part].bufcur =
			ts_tsp[i_part].buffer + ts_tsp[i_part].partlength;
		ts_tsp[i_part].bufsize += DN_SIZE_OUTBUFUNIT;
	}

	/* 書き込み */
	*(ts_tsp[i_part].bufcur) = c_in;
	ts_tsp[i_part].bufcur++;
	ts_tsp[i_part].partlength++;

	return( 0 );
}
