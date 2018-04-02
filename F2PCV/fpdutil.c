/**********************************************************************/
/* PMDRC - P.M.D. reversal compiler by JUD(T.Terata)                  */
/* Utility functions                                                  */
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
/* 解析 アドレスのスタック */
/* arg. NULL:集計整列 other:スタック */
char *stackaddr( char *cp_getm ){

static char *cp_mtb=NULL;
static char **cpp_mcur=NULL;
static int i_mtbcount=0;
static int i_mtbsize=0;
static int i_mtbunit=0;
	char *cp_tmp;
	char **cpp_tmp;
	char *cp_ret;
	int i_count;
	int i_doublecount;
	char **cpp_sorter;
	char **cpp_nsorter;
	char **cpp_scur=NULL;
long *lp_tmp;

	/* 集計整列の場合 */
	if( cp_getm == NULL ){
		/* 実施・・・・って整列ってどーやんだ？ */
		/* とりあえず、ぐるぐる回すしかないか・・・・・ */
		if( i_mtbcount == 0 ){
			/* １個もない場合 */
			return( NULL );
		}
		if( i_mtbcount > 1 ){
			/* ２個以上あった場合は整列 */
			for(i_doublecount=0;i_doublecount<(i_mtbcount-1);i_doublecount++){
				cpp_scur = (char **)cp_mtb;
				for(i_count=0;i_count<(i_mtbcount-1);i_count++){
					cpp_sorter = cpp_scur;
					cpp_scur++;
					cpp_nsorter = cpp_scur;
					if( *cpp_sorter > *cpp_nsorter ){
						/* 順番を入れ替える */
						cp_tmp = *cpp_nsorter;
						*cpp_nsorter = *cpp_sorter;
						*cpp_sorter = cp_tmp;
					}
				}
			}
		}

		/* それと、新しくapmallocで取り直さないとマズイ */
		cp_ret = apmalloc( sizeof(char *) * (i_mtbcount+1) );
		memset( cp_ret , 0x00 , sizeof(char *) * (i_mtbcount) );
		memcpy( cp_ret , cp_mtb , sizeof(char *) * (i_mtbcount) );
//		apfree( cp_mtb );

		/* static変数の初期化 */
		cp_mtb=NULL;
		cpp_mcur=NULL;
		i_mtbcount=0;
		i_mtbsize=0;
		i_mtbunit=0;

		/* 返すのは(char *)で */
		return( cp_ret );
	}

	/* メモリの割り当てが必要な場合→初回 */
	if( cp_mtb == NULL && i_mtbcount == 0 ){
		/* 初期割り当て */
		cp_mtb = apmalloc( (1+DN_SIZE_STACKUNIT) * (sizeof(char *)) );
		cpp_mcur = (char **)(cp_mtb);
		i_mtbcount = 0;
		i_mtbunit = 1;
		i_mtbsize = DN_SIZE_STACKUNIT * sizeof(void *);
	}else if( i_mtbcount > 0 && i_mtbcount % DN_SIZE_STACKUNIT == 0 ){
		/* 再割り当て */
		i_mtbunit++;
		cp_tmp = apmalloc( (1+DN_SIZE_STACKUNIT*i_mtbunit) * (sizeof(char *)) );
		memcpy( cp_tmp , cp_mtb ,
			(1+DN_SIZE_STACKUNIT*(i_mtbunit-1)) * (sizeof(char *)) );
//		apfree( cp_mtb );
		cp_mtb = cp_tmp;
		cpp_mcur = (char **)(cp_mtb);
		cpp_mcur += i_mtbcount;
		i_mtbsize += DN_SIZE_STACKUNIT * sizeof(char *);
	}

	/* 取得したメモリアドレスをスタック */
	*cpp_mcur = cp_getm;
	i_mtbcount++;
	cpp_mcur++;

	return( NULL );
}
/*--------------------------------------------------------------------*/
/* ユーティリティ */
/* べき乗 */
int powxy(int x,int y){
	int i_loop;
	int	i_ans;

	i_ans = 1;
	for(i_loop=0;i_loop<y;i_loop++){
		i_ans = i_ans * x;
	}
	return( i_ans );
}
/*--------------------------------------------------------------------*/
/* メモリ一括解放補助関数 */
/*  arg.  -1:Rerease  0-:size */
void *apmalloc( int i_size ){

static void *vp_mtb=NULL;
static void **vpp_mcur=NULL;
static int i_mtbcount=0;
static int i_mtbsize=0;
static int i_mtbunit=0;
	void *vp_tmp;
static void *vp_getm;
	int i_count;

	/* メモリ解放の場合 */
	if( i_size == -1 ){
		if( i_mtbcount == 0 ){
			/* 動作実績がないので何もしない */
			return( NULL );
		}
		/* 実施 */
		vpp_mcur = vp_mtb;
		for(i_count=0;i_count<i_mtbcount;i_count++){
			if( *vpp_mcur != NULL ){
				apfree( *vpp_mcur );
			}
			vpp_mcur++;
		}
		/* 最後に自分自身で使用しているメモリをリリース */
		apfree( vp_mtb );
		return( NULL );
	}

	/* メモリ取得本筋 */
	vp_getm = malloc( i_size );
	if( vp_getm == NULL ){
		return( NULL );
	}

	/* メモリの割り当てが必要な場合→初回 */
	if( vp_mtb == NULL && i_mtbcount == 0 ){
		/* 初期割り当て */
		vp_mtb = malloc( (1+DN_SIZE_MTBUNIT) * (sizeof(void *)) );
		vpp_mcur = (void *)(vp_mtb);
		i_mtbcount = 0;
		i_mtbunit = 1;
		i_mtbsize = DN_SIZE_MTBUNIT * sizeof(void *);
	}else if( i_mtbcount > 0 && i_mtbcount % DN_SIZE_MTBUNIT == 0 ){
		/* 再割り当て */
		i_mtbunit++;

//		vp_tmp = malloc( (1+DN_SIZE_MTBUNIT*i_mtbunit) * (sizeof(void *)) );
//		memcpy( vp_tmp , vp_mtb ,
//			(1+DN_SIZE_MTBUNIT*(i_mtbunit-1)) * (sizeof(void *)) );
//		apfree( vp_mtb );
//		vp_mtb = vp_tmp;
		vp_mtb = realloc( vp_mtb ,
			(1+DN_SIZE_MTBUNIT*i_mtbunit) * (sizeof(void *)) );
//
		vpp_mcur = (void *)(vp_mtb);
		vpp_mcur += i_mtbcount;
		i_mtbsize += DN_SIZE_MTBUNIT * sizeof(void *);
	}

	/* 取得したメモリアドレスをスタック */
	*vpp_mcur = vp_getm;
	i_mtbcount++;
	vpp_mcur++;

	return( vp_getm );
}
/*--------------------------------------------------------------------*/
/* 文字列出力補助関数 */
/*  arg.1  -1:init  0-(DN_MAX_F2POUTBUFFER-1):handle */
/*  arg.2  出力文字列へのポインタ 返却値→ NULL:正常 cp_in:異常 */
/*         NULL時:当該ハンドルのバッファ先頭アドレスを返却 */
int apfree( void *vp_in ){

	int i_ret;

	if( vp_in != NULL ){
		free( vp_in );
	}

	return( 0 );
}
/*--------------------------------------------------------------------*/
/* 文字列出力補助関数 */
/*  arg.1  -1:init  0-(DN_MAX_F2POUTBUFFER-1):handle */
/*  arg.2  出力文字列へのポインタ 返却値→ NULL:正常 cp_in:異常 */
/*         NULL時:当該ハンドルのバッファ先頭アドレスを返却 */
char *hfpf( int i_handle , char *cp_in ){

static int is_outbuffersize[DN_MAX_F2POUTBUFFER];
static int is_cursize[DN_MAX_F2POUTBUFFER];
static char *cps_cur[DN_MAX_F2POUTBUFFER];
static char *cps_buffer[DN_MAX_F2POUTBUFFER];
static int is_unit[DN_MAX_F2POUTBUFFER];
	char *cp_ret;
	int i_count;

	if( i_handle == -1 ){
		/* 全初期化 */
		for(i_count=0;i_count<DN_MAX_F2POUTBUFFER;i_count++){
			is_outbuffersize[i_count] = 0;
			is_cursize[i_count] = 0;
			cps_cur[i_count] =  NULL;
			cps_buffer[i_count] = NULL;
			is_unit[i_count] = 0;
		}
		sprintf(gcs_line," [F2P] Initialize output-buffers.\n");
		if( (gtp_gop->i_directmes&D_ON) != 0 ){
			/* 制御メッセージのダイレクト出力対応 */
			fprintf(stderr,gcs_line);
		}
		return( NULL );
	}
	/* ハンドル範囲外 */
	if( i_handle < 0 || i_handle >= DN_MAX_F2POUTBUFFER ){
		return( cp_in );
	}

	if( cp_in == NULL ){
		/* 各ハンドル対応のバッファを返す場合 */
		return( cps_buffer[i_handle] );
	}

	if( (gtp_gop->i_directmes&D_ON) != 0 && i_handle == DH_BMESSAGES ){
		/* 制御メッセージのダイレクト出力対応 */
		fprintf(stderr,"%s",cp_in);
	}

	/* 長さチェック */
	if( cp_in != NULL && (strlen(cp_in) >= DN_SIZE_LINEBUFFER) ){
		return( cp_in );
	}

	/* メモリ割り当て */
	if( is_outbuffersize[i_handle] == 0 ){
		/* メモリ確保(初回) */
		cps_buffer[i_handle] = malloc( DN_MAX_OUTUNITSIZE );
		cps_cur[i_handle] = cps_buffer[i_handle];
		is_unit[i_handle] = 1;
		is_outbuffersize[i_handle] = DN_MAX_OUTUNITSIZE;
	}else if( is_cursize[i_handle]+strlen(cp_in)+1 >
				is_outbuffersize[i_handle] ){
		/* メモリ不足→追加割り当て */
		is_unit[i_handle]++;
		cp_ret = malloc( DN_MAX_OUTUNITSIZE * is_unit[i_handle] );
		memcpy( cp_ret , cps_buffer[i_handle] ,
			DN_MAX_OUTUNITSIZE * (is_unit[i_handle]-1) );
		apfree( cps_buffer[i_handle] );
		cps_buffer[i_handle] = cp_ret;
		/* カレントポインタ再配置 */
		cps_cur[i_handle] = cps_buffer[i_handle] + is_cursize[i_handle];
		is_outbuffersize[i_handle] = DN_MAX_OUTUNITSIZE * is_unit[i_handle];
	}
	/* 出力 */
	strcpy( cps_cur[i_handle] , cp_in );
	/* カレントサイズ加算・カレントポインタ移動・メモリ掃除 */
	cps_cur[i_handle] += strlen( cp_in );
	is_cursize[i_handle] += strlen( cp_in );
	if( cp_in != NULL ){
		memset( cp_in , 0x00 , DN_SIZE_LINEBUFFER );
	}

	return( NULL );
}
