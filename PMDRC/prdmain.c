char scs_dlldate[]    ="2004/11/22";
char scs_DLLVersion[] ="1.21a";
int  si_DLLVersion    = 1211;
char scs_IFVersion[]  ="1.21";
int  si_IFVersion     = 1210;
int  si_upperAPV      = 1999;
int  si_lowerAPV      = 1000;

/**********************************************************************/
/* PMDRC - P.M.D. reversal compiler by JUD(T.Terata)                  */
/* Main function & Frame functions                                    */
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
#include <windows.h>

#include "prd.h"
#include "prdext.h"

/*--------------------------------------------------------------------*/
/* EXECUTE-MAIN */
T_OPIF __declspec(dllexport) *__stdcall pmdrc_dllmain( T_OPIF *tp_indata ){

	T_OPIF *t1p_ret;
	int i_count;
	int i_ret;
	char cs_line[DN_SIZE_LINEBUFFER];

	/* 情報 */
	gtp_gop = tp_indata;

	/* バッファの初期化 */
	hfpf( -1 , NULL );

	/* ＤＬＬ情報埋め込み */
	tp_indata->i_lv = si_IFVersion;

	/* 低レベルな入力チェックとデフォルト値セット */
	i_ret = dllifinit( tp_indata );
	if( i_ret != 0 ){
		tp_indata->i_result = i_ret;
		return( NULL );
	}

	/* バージョンチェック */
	if( tp_indata->i_iv != si_IFVersion ){
		/* IFバージョン違い */
		tp_indata->i_result = DR_E_NEIFVERSION;
		return( NULL );
	}
	if( tp_indata->i_av > si_upperAPV || tp_indata->i_av < si_lowerAPV ){
		/* APバージョン 高すぎ or 低すぎ */
		tp_indata->i_result = DR_E_NSAPVERSION;
		return( NULL );
	}

	/* ---- 解析処理 ---- */
	t1p_ret = anlmain( tp_indata );

	/* 上位へ返すバッファアドレスをセット */
	if( t1p_ret != NULL ){
		for(i_count=0;i_count<DH_BLAST;i_count++){
			tp_indata->cps_out[i_count] = hfpf( i_count , NULL );
		}
	}

	for(i_count=0;i_count<DN_MAX_OUTBUFFER;i_count++){
		if( tp_indata->cps_out[i_count] != NULL ){
			/* 出力文字列あり */
			sprintf(gcs_line," [DLL] OUTPUT[%d] : %6d Bytes\n",
				i_count,strlen(tp_indata->cps_out[i_count]));
		}else{
			/* 出力文字列なし */
			sprintf(gcs_line," [DLL] OUTPUT[%d] :        nothing\n",
				i_count);
		}
		hfpf( DH_BMESSAGES , gcs_line );
	}

	sprintf(gcs_line," [DLL] End analysys.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* メモリ一括解放 */
	apmalloc( -1 );

	return( gtp_gop );
}
/*--------------------------------------------------------------------*/
/* EXECUTE-MAIN */
int dllifinit( T_OPIF *tp_in ){

	if( tp_in->i_mmllevel > 5 && tp_in->i_mmllevel < 2 ){return(DR_E_NGIF);}
	if( tp_in->cp_indata == NULL ){return(DR_E_BADDATA);}

	if( tp_in->i_zenlen == 0 ){
		tp_in->i_zenlen = 96;
	}

/* 仕様変更： */
/* i_lendefにゼロがセットされた場合、従来の「デフォルト値を採用」から */
/* 「標準音長を使用しない」に変更 (1.00c) */
//	if( tp_in->i_lendef == 0 ){
//		tp_in->i_lendef = tp_in->i_zenlen / 16;
//	}

	if( tp_in->i_lenmin == 0 ){
		tp_in->i_lenmin = tp_in->i_zenlen / 32;
	}

	if( tp_in->i_len3min == 0 ){
		tp_in->i_len3min = tp_in->i_zenlen / 24;
	}

	return( 0 );
}
/*--------------------------------------------------------------------*/
/* EXECUTE-MAIN */
T_OPIF __declspec(dllexport) *__stdcall pmdrc_dllfreem( T_OPIF *tp_indata ){

	int i_cnt;

	for(i_cnt=0;i_cnt<DN_MAX_OUTBUFFER;i_cnt++){
		/* NULLの場合はそのテキストは存在しないのでメモリも確保していない */
		if( tp_indata->cps_out[i_cnt] == NULL ){ continue; }
		free( tp_indata->cps_out[i_cnt] );
		tp_indata->cps_out[i_cnt] = NULL;
	}

	return( tp_indata );
}
