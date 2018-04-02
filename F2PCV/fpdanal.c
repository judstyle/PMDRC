/**********************************************************************/
/* PMDRC - P.M.D. reversal compiler by JUD(T.Terata)                  */
/* Analyzes functions                                                 */
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
/* 解析メイン */
int anlmain( T_F2PIF *tp_in ){

	int i_ret;
	char *cp_ret;
	int i_count;
	int i_partelem;
	char *cp_pbuf;
	char *cp_pcur;
	int i_pbufsize;
	int i_totalpartsize;
	int i_FMparam;

	sprintf(gcs_line," [F2P] Start analysis.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* 走査１ ヘッダの参照、各パートの始点、ループアドレスの取り込み */
	i_ret = readheader( tp_in->cp_indata );
	if( i_ret < 0 ){
		return( i_ret );
	}
	sprintf(gcs_line," [F2P] Complete read header.\n");
	hfpf( DH_BMESSAGES , gcs_line );
	i_partelem = i_ret;

	/* 走査１．５ ＦＭ音色データサーチ (V1.01c) */
	i_FMparam = 0;
	i_ret = getFMparam( tp_in->cp_indata );
	if( i_ret > 0 ){
		/* ＦＭパラメータ取得成功 */
		i_FMparam = i_ret;
		sprintf(gcs_line," [F2P] Complete read FM-parameters [%d].\n",i_ret);
		hfpf( DH_BMESSAGES , gcs_line );
	}else if( i_ret < 0 ){
		sprintf(gcs_line," [F2P] Failed read FM-parameter.\n");
		hfpf( DH_BMESSAGES , gcs_line );
	}

	/* 走査２ 各パートのループ終点を検索し、対応する始点アドレスを列挙 */
	for(i_count=0;i_count<i_partelem;i_count++){
		if( ts_tsf[i_count].use == D_ON ){
			i_ret = searchloop( &(ts_tsf[i_count]) );
			if( i_ret < 0 ){
				return( i_ret );
			}
		}
	}
	sprintf(gcs_line," [F2P] Complete pickup looppoints.\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* 走査３ 各パートのデータをコンバートしながらバッファに書き出し */
	for(i_count=0;i_count<i_partelem;i_count++){
		if( ts_tsf[i_count].use == D_ON ){
			i_ret = convertpart( &(ts_tsf[i_count]) );
			if( i_ret < 0 ){
				return( i_ret );
			}
		}
	}
	sprintf(gcs_line," [F2P] Complete convert trackdata.\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* 走査４ 未使用ＰＭＤトラックに終端を仕込んで回る */
	for(i_count=0;i_count<DN_PART_PDELEM;i_count++){
		if( ts_tsp[i_count].use == D_OFF ){
			ts_tsp[i_count].buffer = (char *)&DS_PMD_ONLYTERMINATION;
//			ts_tsp[i_count].use = D_ON;
			ts_tsp[i_count].partlength = 1;
			ts_tsp[i_count].bufsize = 1;
		}
	}
	sprintf(gcs_line," [F2P] Odd trackdata.\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* ＰＭＤヘッダ作り データサイズ測定 */
	i_totalpartsize = makepmdhead( NULL );
	if( i_totalpartsize < 0 ){
		sprintf(gcs_line,"E[F2P] Error in makepmdhead(NULL).\n");
		hfpf( DH_BMESSAGES , gcs_line );
		return( i_totalpartsize );
	}

	/* バッファ確保 */
	/* makepmdhead返却値=パート合計 + ヘッダサイズ + フッタサイズ */
	/* 1.01cで追加 さらに音色データ１個につき26bytes */
	i_pbufsize = i_totalpartsize;
	i_pbufsize += DN_SIZE_PMDHEADREAL;
	i_pbufsize += DN_SIZE_PMDFOOTREAL;
	i_pbufsize += DN_SIZE_VOICEPMD * i_FMparam;
	if( gcp_comment != NULL ){
		i_pbufsize += (strlen(gcp_comment)+1);
	}
	cp_pbuf = malloc( i_pbufsize );
	cp_pcur = cp_pbuf;
	/* ＩＦにアドレス記録とバッファサイズを忘れずに */
	tp_in->i_datalen = i_pbufsize;
	tp_in->cp_outdata = cp_pbuf;

	/* ヘッダ作成 */
	makepmdhead( cp_pcur );
	cp_pcur += DN_SIZE_PMDHEADREAL;
	sprintf(gcs_line," [F2P] Complete make pmdhead.\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* ＰＭＤデータ領域に書き込み */
	for(i_count=0;i_count<DN_PART_PDELEM;i_count++){
		memcpy( cp_pcur,ts_tsp[i_count].buffer,ts_tsp[i_count].partlength );
		cp_pcur += ts_tsp[i_count].partlength;
		if( ts_tsp[i_count].use == D_ON ){
			apfree( ts_tsp[i_count].buffer );
		}
		sprintf(gcs_line," [F2P] Part%2d : %6d Bytes.\n",
			i_count , ts_tsp[i_count].partlength );
		hfpf( DH_BMESSAGES , gcs_line );
	}

	/* フッタ作成 V1.01cで変更 */
	makepmdfoot( cp_pcur , i_totalpartsize , i_FMparam );
	sprintf(gcs_line," [F2P] Complete make pmdfoot.\n");
	hfpf( DH_BMESSAGES , gcs_line );

	return( 0 );
}
/*--------------------------------------------------------------------*/
/* 解析 ＦＭＰのデータアドレスから、該当する定義構造体アドレスを返す */
T_TCODE *searchbycode( char *cp_in , int i_in ){

	T_TCODE *tp_ret;
	int i_count;
	int i_c1;
	int i_c2;
	int i_c3;
	int i_isdiff;
	char css_PNAME[4][4]={" FM","SSG","PCM",""};

	for(i_count=0;i_count<DN_MAX_CONTROLCODE;i_count++){
		if( ts_tc[i_count].cont == D_OFF ){
			/* リストの終端まで来たが該当ナシ */
			break;
		}
		if( ts_tc[i_count].f_opelen[i_in] == 0 ){
			/* このパート型では未使用のデータ */
			continue;
		}
		i_isdiff = D_OFF;
		for(i_c1=0;i_c1<ts_tc[i_count].f_opelen[i_in];i_c1++){
			if( (char)(ts_tc[i_count].fcode[i_c1]) !=
				(char)(*(cp_in+i_c1)) ){
				/* 違った */
				i_isdiff = D_ON;
				break;
			}
		}
		if( i_isdiff != D_ON ){
			/* ここで当たりの処理 */
			return( &(ts_tc[i_count]) );
		}
	}

	/* この時点でハズレなので、問題部分を表示する */
	sprintf(gcs_line,
		"E[F2P] ERROR in searchbycode(). [%s:%02X/%02X/%02X/%02X]\n",
		css_PNAME[i_in],
		(unsigned char)(*(cp_in)),
		(unsigned char)(*(cp_in+1)),
		(unsigned char)(*(cp_in+2)),
		(unsigned char)(*(cp_in+3)) );
	hfpf( DH_BMESSAGES , gcs_line );

	return( NULL );
}
/*--------------------------------------------------------------------*/
/* 解析 パート単位でコンバート */
int convertpart( T_FPARTSTATE *tp_in ){

	char *cp_fcur;
	char *cp_pcur;
	int i_ret;

	char *cp_ret;
	T_TCODE *tp_tcode;

	T_TCODE *tp_pcode;
	char c_tieorslur;

	int i_tieflag=D_OFF;
	int i_slurflag=D_OFF;
	int i_stdlen=8;
	int i_count;

	unsigned char uc_tmp;

	/* ＰＭＤバッファの初期化 */
	for(i_count=0;i_count>DN_PART_PDELEM;i_count++){
		ts_tsp[i_count].bufsize = 0;
		ts_tsp[i_count].partlength = 0;
		ts_tsp[i_count].buffer = NULL;
		ts_tsp[i_count].bufcur = NULL;
	}

	/* コンバート先の設定 */
	ts_tsp[tp_in->ppartnum].partlength = 0;
	ts_tsp[tp_in->ppartnum].use = D_ON;

	for(cp_fcur=tp_in->pdata;;){
//fprintf(stderr,"%1X:STEP:%02X\n",tp_in->ppartnum,*cp_fcur);
		/* 終了条件のチェック→終了処理 */
		if( *cp_fcur == 0x74 ){
			outpmd( tp_in->ppartnum , (char)0x80 );
			break;
		}

		tp_tcode = searchbycode( cp_fcur , tp_in->parttype );
		if( tp_tcode == NULL ){
			/* UNKNOWN-CODE */
			sprintf(gcs_line,"E[F2P] Search error at searchbycode().\n");
			hfpf( DH_BMESSAGES , gcs_line );
			return( -1 );
		}

		/* ループ記号のヒットチェック */
		if( cp_fcur == tp_in->nextlp ){
			/* L または [ ループ制御文字の出力 */
			if( tp_in->nextlp == tp_in->lladdr ){
				/* L の部分 */
				outpmd( tp_in->ppartnum , (char)0xF6 );
				/* 'L'は１回使ったら終わり */
				tp_in->lladdr = NULL;
			}else{
				/* [ の部分（とデータ部のハリボテ2byte） */
				outpmd( tp_in->ppartnum , (char)0xF9 );
				outpmd( tp_in->ppartnum , (char)0x00 );
				outpmd( tp_in->ppartnum , (char)0x00 );
				tp_in->lscur++;
			}
			/* 次回ループ記号位置チェック（'L'が'['より優先だが） */
			if( tp_in->lladdr != NULL && *(tp_in->lscur) > tp_in->lladdr ){
				/* 先に来るのが'L'、かつ'L'未設置 */
				tp_in->nextlp = tp_in->lladdr;
			}else if( tp_in->lscur != NULL ){
				/* 先に来るのが'['、または'L'設置済み */
				tp_in->nextlp = *(tp_in->lscur);
			}else{
				/* '[' が存在しない場合 */
				tp_in->nextlp = NULL;
			}
			/* ここでは実際にＦＭＰ側のデータを読んでいるわけではないので */
			/* ポインタを進めずにもう一度音符を出力 */
			continue;
		}

		/* データの１ステップ解析 */
		switch( tp_tcode->convtype ){
			case 0:
				/* 無視 */
				break;
			case 1:
				/* 音符型置換 音長なければ標準値を使用 */
				if( tp_tcode->f_datalen[tp_in->parttype] == 0 ){
					/* 音長のない音符 */
					uc_tmp = i_stdlen;
				}else{
					/* 通常ＰＭＤ型の音符 */
					uc_tmp = *(cp_fcur+1);
				}
				outpmd( tp_in->ppartnum , tp_tcode->pcode1 );
				outpmd( tp_in->ppartnum , (char)uc_tmp );

				/* タイ・スラー */
				if( i_tieflag == D_ON ){
					/* ここで& */
					c_tieorslur = D_OBJ_TIE;
					i_tieflag = D_OFF;
					tp_pcode = searchbycode( &c_tieorslur , tp_in->parttype );
					if( tp_pcode == NULL ){
						/* コード該当ナシ */
						return( -1 );
					}
					outpmd( tp_in->ppartnum , tp_pcode->pcode1 );
				}else if( i_slurflag == D_ON ){
					/* ここで&& */
					c_tieorslur = D_OBJ_SLUR;
					i_slurflag = D_OFF;
					tp_pcode = searchbycode( &c_tieorslur , tp_in->parttype );
					if( tp_pcode == NULL ){
						/* コード該当ナシ */
						return( -1 );
					}
					outpmd( tp_in->ppartnum , tp_pcode->pcode1 );
				}
				break;
			case 2:
				/* データ付き単純置換（サイズは自動追随） */
				outpmd( tp_in->ppartnum , tp_tcode->pcode1 );
				for(i_count=0;i_count<tp_tcode->f_datalen[tp_in->parttype];
					i_count++){
					outpmd( tp_in->ppartnum , *(cp_fcur+1+i_count) );
				}
				break;
			case 3:
				/* データなし単純置換（変換後のサイズはナシ） */
				outpmd( tp_in->ppartnum , tp_tcode->pcode1 );
				break;
			case 4:
				/* タイ、フラグ立てるだけ */
				i_tieflag=D_ON;
				break;
			case 5:
				/* スラー、フラグ立てるだけ */
				i_slurflag=D_ON;
				break;
			case 6:
				/* 標準音長、内部的に更新するだけ */
				i_stdlen = (int)((unsigned char)(*(cp_fcur+1)));
				break;
			case 7:
				/* データ部はハリボテとして扱うオペレート */
				/* オペレートだけ変換し、 */
				/* データ部は変換後データ長さに従いゼロパディング */
				outpmd( tp_in->ppartnum , tp_tcode->pcode1 );
				for(i_count=0;i_count<tp_tcode->f_datalen[tp_in->parttype];
					i_count++){
					outpmd( tp_in->ppartnum , 0 );
				}
				break;
			case 8:
				/* ループ終点 */
				outsloopend( cp_fcur , tp_in->ppartnum );
				break;
			case 9:
				/* ポルタメント */
				outsporta( cp_fcur , tp_in->ppartnum , i_stdlen );
				break;
			default:
				/* バグ */
				break;
		}

		/* 長さの取得・読み取り側のポインタスキップ */
		i_ret = getflen( cp_fcur , tp_in->parttype );
		if( i_ret < 0 ){
			/* コード該当ナシ */
			return( -1 );
		}
		cp_fcur += i_ret;
	}

	/* 終了処理は、ない */
	return( 0 );
}
/*--------------------------------------------------------------------*/
/* 解析 パートごとに異なる該当するオペレートの長さを返す */
/* 引数１：入力データ */
/* 引数２：パート型(FM/SSG/PCM) */
int getflen( char *cp_in , int i_type ){

	T_TCODE *tp_tcode;
	int i_totallen;

	tp_tcode = searchbycode( cp_in , i_type );
	if( tp_tcode == NULL ){
		sprintf(gcs_line,"E[F2P] Search error in getflen().\n");
		hfpf( DH_BMESSAGES , gcs_line );
		return( -1 );
	}

	i_totallen = tp_tcode->f_opelen[i_type] + tp_tcode->f_datalen[i_type];

	return( i_totallen );
}
/*--------------------------------------------------------------------*/
/* 解析 ループ始点の検索 */
int searchloop( T_FPARTSTATE *tp_in ){

	char *cp_cur;
	int i_curopesize;
	int i_rewindsize;
	char *cp_startpoint;

	if( tp_in->use != D_ON ){
		/* 未使用パートならなんにもしない */
		return( 0 );
	}

	/* 終端までループ */
	for(cp_cur=tp_in->pdata;(*cp_cur)!=(char)0x74;){
		if( *cp_cur != (char)0x64 ){
			/* ループ終点でない */
			i_curopesize = getflen( cp_cur , tp_in->parttype );
			if( i_curopesize < 0 ){
				/* コード該当ナシ */
				return( -1 );
			}
			cp_cur += i_curopesize;
			continue;
		}
		/* ループ発見 */
		i_rewindsize = (int)((unsigned char)(*(cp_cur+2)));
		i_rewindsize += ( 256 * (int)((unsigned char)(*(cp_cur+3))) );
		cp_startpoint = cp_cur - (i_rewindsize-4);
		stackaddr( (void *)cp_startpoint );
		i_curopesize = getflen( cp_cur , tp_in->parttype );
		if( i_curopesize < 0 ){
			/* コード該当ナシ */
			return( -1 );
		}
		cp_cur += i_curopesize;
	}

	/* スタックの整列と取得 */
	tp_in->lsaddr = (char **)stackaddr( NULL );
	tp_in->lscur = tp_in->lsaddr;

	/* 'L' または '[' 割り込みアドレス、最初の１個をセット */
	if( tp_in->lsaddr == NULL && tp_in->lladdr == NULL ){
		tp_in->nextlp = NULL;
	}else if( tp_in->lsaddr != NULL && tp_in->lladdr != NULL ){
		if( *(tp_in->lsaddr) < tp_in->lladdr ){
			tp_in->nextlp = *(tp_in->lsaddr);
		}else{
			tp_in->nextlp = tp_in->lladdr;
		}
	}else if( tp_in->lsaddr!= NULL ){
		tp_in->nextlp = *(tp_in->lsaddr);
	}else{
		tp_in->nextlp = tp_in->lladdr;
	}

	return( 0 );
}
/*--------------------------------------------------------------------*/
/* 解析 データ型の解析 */
/* ＦＭＰデータファイルのパート数を判断する */
/* 戻り値  0x20:６パート  0x40:１１パート */
int analdatatype( char *cp_indata ){

	short *sp_offset;
	char *cp_cur;

	/* コンパイラ名まで */
	sp_offset = (short *)cp_indata;
	cp_cur = cp_indata;
	cp_cur += (int)(*sp_offset);
	/* コンパイラバージョンまで移動 */
	cp_cur += 3;

	/* 1.01b コメント部の先頭アドレスを取得 */
	gcp_comment = cp_cur;
	gcp_comment++;
	if( *gcp_comment == 0x00 ){
		gcp_comment = NULL;
	}

	return( (int)(*cp_cur)&0xF0 );
}
/*--------------------------------------------------------------------*/
/* 解析 ヘッダの読み込み */
int readheader( char *cp_in ){

	/* 各データタイプにおけるパート数 */
	int is_partelem[]={6,11,-1,0};
	/* 各データタイプにおけるパートタイプ */
	int iss_parttype[4][12]={
		{ 0, 0, 0, 1, 1, 1,-1,-1,-1,-1,-1,-1},
		{ 0, 0, 0, 0, 0, 0, 1, 1, 1,-1, 2,-1},
		{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
	};
	/* 各データタイプにおけるパート毎の変換後パート番号 */
	int iss_ppartnum[4][12]={
		{0,1,2,6,7,8,0,0,0,0,0,0},
		{0,1,2,3,4,5,6,7,8,0,9,0},
		{0,0,0,0,0,0,0,0,0,0,0,0}
	};
	int i_count;
	int i_offset;
	int i_loopoffset;
	int i_loop2offset;
	char *cp_cur;
	int i_ret;
	int i_datatype;

	i_ret = analdatatype( cp_in );
	/* データタイプの振り落とし */
	switch( i_ret ){
		case 0x20:
			/* ６パート */
			i_datatype = 0;
			break;
		case 0x40:
			/* １１パート */
			i_datatype = 1;
			break;
		default:
			sprintf(gcs_line," [F2P] BAD FMP Format.\n");
			return( DR_E_NSDTVERSION );
			hfpf( DH_BMESSAGES , gcs_line );
	}
	sprintf(gcs_line," [F2P] FMP Format is 0x%02X.\n",i_ret);
	hfpf( DH_BMESSAGES , gcs_line );

	/* 読み込みデータの先頭２バイトから */
	cp_cur = (char *)(cp_in);
	cp_cur += 2;
	i_loop2offset = is_partelem[i_datatype] * 2;
	for(i_count=0;i_count<is_partelem[i_datatype];i_count++,cp_cur+=2){
		if( iss_parttype[i_datatype][i_count] == -1 ){
			/* 当該パートは使用しないタイプ */
			ts_tsf[i_count].use = D_OFF;
			continue;
		}
		/* パートタイプのセット */
		ts_tsf[i_count].parttype = iss_parttype[i_datatype][i_count];
		ts_tsf[i_count].ppartnum = iss_ppartnum[i_datatype][i_count];
		/* 開始アドレス・ループアドレスの算出 */
		i_offset = (int)((unsigned char)(*cp_cur));
		i_offset += ( 256 * (int)((unsigned char)(*(cp_cur+1))) );
		i_loopoffset = (int)((unsigned char)(*(cp_cur+i_loop2offset)));
		i_loopoffset += ( 256 *
			(int)((unsigned char)(*(cp_cur+1+i_loop2offset))) );
		if( i_offset == 0 || i_offset == 0x0FFFF ){
			/* そんなことはないと思うが当該パート未使用 */
			ts_tsf[i_count].use = D_OFF;
			continue;
		}
		ts_tsf[i_count].use = D_ON;
		ts_tsf[i_count].pdata = cp_in + i_offset;
		if( (char)(*(ts_tsf[i_count].pdata)) == (char)0x74 ){
			/* 当該パートはデータなし */
			ts_tsf[i_count].use = D_OFF;
			sprintf(gcs_line," [F2P] Silent part:[%d]\n",i_count);
			hfpf( DH_BMESSAGES , gcs_line );
			continue;
		}
		if( i_loopoffset != 0x0FFFF ){
			/* 当該パート無限ループあり */
			ts_tsf[i_count].lladdr = cp_in + i_loopoffset;
		}
	}

	return( is_partelem[i_datatype] );
}
/*--------------------------------------------------------------------*/
/* 解析 ヘッダの読み込み(ＦＭ音色) */
/* 1.01d 0x40系の11パートデータで全然ダメな恥ずかしいバグを修正 */
int getFMparam( char *cp_in ){

typedef	struct	_t_voiceconv{
	unsigned char uc_number;
	char cs_data[DN_SIZE_VOICEFMP];
}T_VOICECONV;

typedef	struct	_t_dataheader{
	unsigned short us_nouse1;
	unsigned short us_partAaddr;
	unsigned short uss_nouse2[11];
	unsigned short us_tempo;
	char c_voicestart;
}T_DATAHEADER;

typedef	struct	_t_data40header{
	unsigned short us_nouse1;
	unsigned short us_partAaddr;
	unsigned short uss_nouse2[21];
	unsigned short us_tempo;
	unsigned short us_voicetermaddr;
	char c_voicestart;
}T_DATA40HEADER;

	T_DATAHEADER *tp_header;
	T_DATA40HEADER *tp_40header;
	T_VOICECONV *tp_pvoice;
	int i_count;
	unsigned short us_vsize;
	int i_voicenum;
	char *cp_rcur;
	char *cp_wcur;
	int i_datatype;
	int i_voffset;

	unsigned short us_startAaddr;
	unsigned short us_startVaddr;
	unsigned short us_endVaddr;

	gcp_FMparam = gcs_wbuf;
	cp_wcur = gcs_wbuf;
	for( i_count=0 ; i_count<256 ; i_count++){
		tp_pvoice = (T_VOICECONV *)cp_wcur;
		tp_pvoice->uc_number = (unsigned char)i_count;
		cp_wcur += DN_SIZE_VOICEPMD;
	}

	/* パート数判定 */
	i_datatype = analdatatype( cp_in );
fprintf(stderr,"TYPE:%X\n",i_datatype);
	if( i_datatype == 0x20 ){
		/* ６パート型 */
		i_voffset = DN_VOFFSET_OPI;
	}else if( i_datatype == 0x40 ){
		/* １１パート型 */
		i_voffset = DN_VOFFSET_OVI;
	}else{
		/* 1.01d修正 パート数の確定が出来ない場合放棄 */
		return(-1);
	}

fprintf(stderr,"TYPE:%X/%d\n",i_datatype,i_voffset);

	/* ＦＭ音色あるか判定 */
	tp_header = (T_DATAHEADER *)cp_in;
	tp_40header = (T_DATA40HEADER *)cp_in;
	if( i_datatype == 0x20 ){
		/* ６パート型 */
		us_startVaddr = DN_VOFFSET_OPI;
	}else if( i_datatype == 0x40 ){
		/* １１パート型 */
		us_startVaddr = DN_VOFFSET_OVI;
	}
fprintf(stderr,"ADDR:%X/%X\n",us_startVaddr,tp_header->us_partAaddr);
	if( us_startVaddr == tp_header->us_partAaddr ){
		/* ＦＭ音色なし */
		return(0);
	}

	/* ＦＭ音色数取得 */
	/* ＦＭ音色領域始点・終端取得 */
	if( i_datatype == 0x20 ){
		/* ６パート型 */
		us_endVaddr = tp_header->us_partAaddr;
	}else if( i_datatype == 0x40 ){
		/* １１パート型 */
		us_endVaddr = tp_40header->us_voicetermaddr;
	}
fprintf(stderr,"SIZE:%d/%d/%d\n",us_endVaddr,us_startVaddr,DN_SIZE_VOICEFMP);
	if( (us_endVaddr-us_startVaddr)%DN_SIZE_VOICEFMP != 0 ){
		/* ＦＭ音色領域サイズがＦＭレジスタサイズと一致せず */
		return(-2);
	}
	i_voicenum = ((int)(us_endVaddr-us_startVaddr))/DN_SIZE_VOICEFMP;
fprintf(stderr,"VOCES:%d\n",i_voicenum);

	cp_rcur = cp_in + ((int)(us_startVaddr));

	/* ＦＭ音色コンバート */
	cp_wcur = gcs_wbuf;
	for( i_count=0 ; i_count<i_voicenum ; i_count++){
		tp_pvoice = (T_VOICECONV *)cp_wcur;
		memcpy( tp_pvoice->cs_data , cp_rcur , DN_SIZE_VOICEFMP );
		tp_pvoice->uc_number = (unsigned char)i_count;
		cp_rcur += DN_SIZE_VOICEFMP;
		cp_wcur += DN_SIZE_VOICEPMD;
	}

	return( i_voicenum );
}
