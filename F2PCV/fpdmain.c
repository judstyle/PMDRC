char scs_IFVersion[]  ="1.10";
int  si_IFVersion     = 1100;

/**********************************************************************/
/* F2PCV - FMP -> P.M.D. data converter by JUD(T.Terata)              */
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

#include "fpd.h"
#include "fpdext.h"

/*--------------------------------------------------------------------*/
/* EXECUTE-MAIN */
T_F2PIF __declspec(dllexport) *__stdcall f2pcv_dllmain( T_F2PIF *tp_indata ){

	int i_ret;
	int i_count;
	char *cp_cur;
	FILE *fp_io;

	gtp_gop = tp_indata;

	/* バッファの初期化 */
	hfpf( -1 , NULL );

	sprintf(gcs_line," [F2P] Start F2PCV.DLL.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* ＤＬＬ情報埋め込み */
	if( tp_indata->i_iv != si_IFVersion){
		sprintf(gcs_line," [F2P] Unsupported version AP.\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
		tp_indata->i_iv = si_IFVersion;
		tp_indata->i_result = DR_E_NEIFVERSION;
		return( NULL );
	}

	/* ファイルタイプの確認 */
	switch( tp_indata->i_datatype ){
		case D_TYPE_OPI:
		case D_TYPE_OVI:
		case D_TYPE_OZI:
			break;
		case D_TYPE_UK:
		default:
			sprintf(gcs_line," [F2P] UNKNOWN data type.\r\n");
			hfpf( DH_BMESSAGES , gcs_line );
			tp_indata->i_result = DR_E_NSDTVERSION;
			return(NULL);
	}

	/* アドレスの解決 */
	gtp_gop = tp_indata;

	/* 処理メインの呼び出し */
	i_ret = anlmain( tp_indata );
	if( i_ret != 0 ){
		tp_indata->i_result = i_ret;
		return( NULL );
	}

	sprintf(gcs_line," [F2P] OutputPMD : %6d Bytes.\n",tp_indata->i_datalen);
	hfpf( DH_BMESSAGES , gcs_line );


	/* ここで、tp_indata->cp_debugdata をファイル名にして出力 */
	if( tp_indata->cp_debugdata != NULL ){
		fp_io = fopen( tp_indata->cp_debugdata , "wb" );
		i_ret = fwrite( tp_indata->cp_outdata , 1 ,
			tp_indata->i_datalen , fp_io );
		fclose( fp_io );
		sprintf(gcs_line," [F2P] DEBUG-Output filename : %s\n",
			tp_indata->cp_debugdata);
		hfpf( DH_BMESSAGES , gcs_line );
	}

	sprintf(gcs_line," [F2P] End analysys.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* メモリ一括解放 */
	apmalloc( -1 );

	return( gtp_gop );
}
/*--------------------------------------------------------------------*/
/* EXECUTE-MAIN */
T_F2PIF __declspec(dllexport) *__stdcall f2pcv_dllfreem( T_F2PIF *tp_indata ){

	int i_cnt;

	for(i_cnt=0;i_cnt<DN_MAX_F2POUTBUFFER;i_cnt++){
		/* NULLの場合はそのテキストは存在しないのでメモリも確保していない */
		if( tp_indata->cps_out[i_cnt] == NULL ){ continue; }
		apfree( tp_indata->cps_out[i_cnt] );
		tp_indata->cps_out[i_cnt] = NULL;
	}

	return( tp_indata );
}
