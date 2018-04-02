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

#include "prd.h"
#include "prdext.h"




/*--------------------------------------------------------------------*/
/* 解析メイン */
T_OPIF *anlmain( T_OPIF *tp_indata ){

	int i_ret;
	int i_cnt;
	int i_cnt2;
	long l_beforeLcur;
	T_BON *tp_boncur;
	char *cp_infile;
	int i_maxloop=0;
	int i_maxtotal=0;
	char *cp_checkresult_l;
	char *cp_checkresult_t;
	int i_L_incoordination = D_OFF;

	/* 初期化 */
	memset(&gt_ginfo,0x00,sizeof(T_GINFO));
	gt_ginfo.l_mctotal=-1;
	gt_ginfo.l_mcloop=-1;
	gt_ginfo.i_nmemo=-1;
	for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
		for(i_cnt2=0;i_cnt2<(DN_MAX_NOTELENFREQ+1);i_cnt2++){
			iss_notelenfreq[DN_MAX_TRACKNUM][DN_MAX_NOTELENFREQ+1] = 0;
		}
	}
	/* トラック初期化 */
	for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
		ts_ts[i_cnt].use = D_OFF;
	}
	/* メモリ再利用管理領域初期化 */
	for(i_cnt=0;i_cnt<256;i_cnt++){
		gtps_stockbon[i_cnt] = NULL;
	}

	/* グローバル変数へ取り込み */
	gi_zenlen = tp_indata->i_zenlen;

	/* ヘッダ読み取り１ */
	readheader( tp_indata->cp_indata );
	sprintf(gcs_line," [DLL] Succeed read header 1st.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* ヘッダ読み取り２ */
	if( (tp_indata->i_definemode)&DA_DEFL_NORMAL != 0 ){
		/* i_definemodeがDA_DEFINEONLYPARTEXTENDである場合を考慮 */
		readhead2( tp_indata->cp_indata );
		sprintf(gcs_line," [DLL] Succeed read header 2nd.\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
	}

	/* トラック走査０ トラック読み取り */
	for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
		if( ts_ts[i_cnt].use != D_ON ){ continue; }
		tp_curtra = &(ts_ts[i_cnt]);
		anltrack( );
	}
	sprintf(gcs_line," [DLL] Succeed read MML on tracks.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* トラック走査１・２・３（全パートコントロール・音符トラック） */
	for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
		if( ts_ts[i_cnt].use != D_ON ){ continue; }
		tp_curtra = &(ts_ts[i_cnt]);
		/* Zコマンドをマスタートラックへ移動 */
		part2master( );

		/* notes-tableの終端(note1=0x80)まで検索し、 */
		/* それまでに、発音がない場合は無効化する */
		i_ret = delsilenttrack( );
		if( i_ret == D_OFF ){ continue; }

		/* 連続している休符は繋げる タイで繋げられている同音符は繋げる */
		combinenotes( i_cnt );
	}
	sprintf(gcs_line," [DLL] Succeed make master-track, Succeed combine notes.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );


	/* トラック走査４（全パート音符トラック） oct/転調の履歴辻褄あわせ */
	for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
		if( ts_ts[i_cnt].use != D_ON ){ continue; }
		tp_curtra = &(ts_ts[i_cnt]);
		editoct( );
	}
	sprintf(gcs_line," [DLL] Succeed edit octerves.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* トラック走査５ 寄せ集めただけのマスタートラックをソート */
	sortmaster( );
	sprintf(gcs_line," [DLL] Succeed sorting mastertrack.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* トラック走査６ 小節線処理 */
	if( gtp_gop->i_barlevel != D_OFF ){
		i_ret = barcheck( );
		i_ret = analclocklist( i_ret );
//fprintf(stderr,"BARTYPE:%d\n",i_ret);
		sprintf(gcs_line," [DLL] Succeed analysis length of part.\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
		if( i_ret != D_BAR_MANUAL ){
			for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
				if( ts_ts[i_cnt].use != D_ON ){ continue; }
				tp_curtra = &(ts_ts[i_cnt]);
				dividebybar( i_ret );
			}
		}
		sprintf(gcs_line," [DLL] Succeed insert blank on the disregard-bar.\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
	}

	/* シーケンシャルに並べてテキスト出力 */
	for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
		if( ts_ts[i_cnt].use != D_ON ){ continue; }
		tp_curtra = &(ts_ts[i_cnt]);
		trackout( i_cnt );
	}
	sprintf(gcs_line," [DLL] Succeed analysis MML.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );
	/* サービス情報 */

	/* フッタ代わりのＦＭ音色定義 */
	if( gtp_gop->i_mmllevel < 5 &&
		gtp_gop->i_voicemode != D_OFF ){
		voiceout();
	}

	/* DEFINE表示 */
	if( gtp_gop->i_definemode != D_OFF ){
		defineout();
	}


	/* サービス情報 */
	sprintf(gcs_line," [DLL] Pa| front | Loop' | Total | data\n");
	hfpf( DH_BMESSAGES , gcs_line );
	sprintf(gcs_line," [DLL] rt| at 'L'| clock | clock |offset\n");
	hfpf( DH_BMESSAGES , gcs_line );
	sprintf(gcs_line," [DLL] --+-------+-------+-------+------\n");
	hfpf( DH_BMESSAGES , gcs_line );
	for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
		if( ts_ts[i_cnt].use != D_ON ){ continue; }
		tp_curtra = &(ts_ts[i_cnt]);
		sprintf(gcs_line," [DLL] %c | %5d | %5d | %5d | %04X\r\n",
			tp_curtra->partname,
			tp_curtra->tracklenbl,
			tp_curtra->tracklenal,
			tp_curtra->tracktotal,
			tp_curtra->text-gtp_gop->cp_indata);
		hfpf( DH_BMESSAGES , gcs_line );
		if( tp_curtra->tracklenal > i_maxloop ){
			i_maxloop = tp_curtra->tracklenal;
		}
		if( tp_curtra->tracktotal > i_maxtotal ){
			i_maxtotal = tp_curtra->tracktotal;
		}
	}

	/* MC.EXEの記録したクロック長の表示（V4.8以降のみ） */
	if( gt_ginfo.c_varsion >= 0x48 ){
		sprintf(gcs_line," [DLL] --+-------+-------+-------+------\n");
		hfpf( DH_BMESSAGES , gcs_line );
		sprintf(gcs_line," [DLL]   |       | %5d | %5d |MC.EXE\n",
			gt_ginfo.l_mcloop , gt_ginfo.l_mctotal );
		hfpf( DH_BMESSAGES , gcs_line );
		if( i_maxloop == gt_ginfo.l_mcloop ){
			cp_checkresult_l = DS_OK;
		}else{
			cp_checkresult_l = DS_NG;
			i_L_incoordination = D_ON;
		}
		if( i_maxtotal == gt_ginfo.l_mctotal ){
			cp_checkresult_t = DS_OK;
		}else{
			cp_checkresult_t = DS_NG;
			i_L_incoordination = D_ON;
		}
		/* 長さチェックの結果がＮＧだった場合 */
		if( i_L_incoordination == D_ON ){
			tp_indata->i_result = DR_W_BADMML;
		}
		sprintf(gcs_line," [DLL]   |       |    %s |    %s | Check\n",
			cp_checkresult_l , cp_checkresult_t );
		hfpf( DH_BMESSAGES , gcs_line );
	}

	sprintf(gcs_line," [DLL] --+-------+-------+-------+------\n");
	hfpf( DH_BMESSAGES , gcs_line );

	return( tp_indata );
}
/*--------------------------------------------------------------------*/
/* 解析系 */
/* ヘッダ読み込み初期化処理２（#Title等） */
void readhead2( char *cp_infile ){

	char *cp_cur1;
	short *sp_cur1;
	long *lp_cur1;
	int i_count;
	int i_offset;

	if( gt_ginfo.cp_voice == NULL ){
		/* ボイスデータが存在しない = バージョン記述箇所が特定できず */
		return;
	}

	cp_cur1 = gt_ginfo.cp_voice;
	cp_cur1--;
	if( (unsigned char)(*cp_cur1) != (unsigned char)(0xFE) ){ return; }

	/* バージョン情報 */
	cp_cur1--;
	gt_ginfo.c_varsion = *cp_cur1;
	sprintf(gcs_line," [DLL] MC.EXE Version is [%d.%d].\n",
		gt_ginfo.c_varsion/16 , gt_ginfo.c_varsion%16 );
	hfpf( DH_BMESSAGES , gcs_line );

	/* テキスト格納アドレス群アドレス */
	cp_cur1--;
	i_offset = ((int)(unsigned char)*(cp_cur1))*256;
	cp_cur1--;
	i_offset += ((int)(unsigned char)*(cp_cur1));
	if( i_offset == 0 ){
		/* アドレス群オフセットがゼロ→情報取得できず */
		return;
	}else{
		gt_ginfo.cp_dtaddr = cp_infile;
		gt_ginfo.cp_dtaddr += i_offset;
		gt_ginfo.cp_dtaddr++;
	}

	/* アドレス群オフセット基準の取得 */
	sp_cur1 = (short *)(gt_ginfo.cp_dtaddr);
	/* ループ長情報の取得（V4.8以降のみ） */
	if( gt_ginfo.c_varsion >= 0x48 ){
		cp_cur1 -= 8;
		lp_cur1 = (long *)(cp_cur1);
		gt_ginfo.l_mctotal = *lp_cur1;
		lp_cur1++;
		gt_ginfo.l_mcloop = *lp_cur1;
	}

	/* 各テキスト領域を取得する */
	/* 但し、テキスト長がゼロの場合、ポインタにはNULLを入れておく */
	if( gt_ginfo.c_varsion >= 0x48 ){
		/* PPZ対応後 = "PZINAMEが存在する" */
		gt_ginfo.cp_pziname = cp_infile + (int)(*sp_cur1) + 1;
		if( *(gt_ginfo.cp_pziname) == 0x00 ){ gt_ginfo.cp_pziname = NULL; }
		sp_cur1++;
	}
	gt_ginfo.cp_ppsname = cp_infile + (int)(*sp_cur1) + 1;
	if( *(gt_ginfo.cp_ppsname) == 0x00 ){ gt_ginfo.cp_ppsname = NULL; }
	sp_cur1++;
	gt_ginfo.cp_ppcname = cp_infile + (int)(*sp_cur1) + 1;
	if( *(gt_ginfo.cp_ppcname) == 0x00 ){ gt_ginfo.cp_ppcname = NULL; }
	sp_cur1++;
	gt_ginfo.cp_title = cp_infile + (int)(*sp_cur1) + 1;
	if( *(gt_ginfo.cp_title) == 0x00 ){ gt_ginfo.cp_title = NULL; }
	sp_cur1++;
	gt_ginfo.cp_composer = cp_infile + (int)(*sp_cur1) + 1;
	if( *(gt_ginfo.cp_composer) == 0x00 ){ gt_ginfo.cp_composer = NULL; }
	sp_cur1++;
	gt_ginfo.cp_arrenger = cp_infile + (int)(*sp_cur1) + 1;
	if( *(gt_ginfo.cp_arrenger) == 0x00 ){ gt_ginfo.cp_arrenger = NULL; }
	sp_cur1++;
	for( i_count=0 ; *sp_cur1!=0 ; sp_cur1++ , i_count++ ){
		gt_ginfo.cps_memo[i_count] = cp_infile + (int)(*sp_cur1) + 1;
		if( gt_ginfo.i_nmemo < 0 ){ gt_ginfo.i_nmemo=0; }
		gt_ginfo.i_nmemo++;
	}

	return;
}
/*--------------------------------------------------------------------*/
/* 解析系 */
/* ヘッダ読み込み初期化処理１ */
void readheader( char *cp_infile ){

	char *cp_cur;
	char *cp_tmp;
	int i_offset;
	int i_cnt;

	cp_cur = cp_infile;
	cp_cur++;
	/* part A-J */
	for(i_cnt=DN_START_NUM_FMSSG;i_cnt<=DN_START_NUM_ADPCM;i_cnt++){
		ts_ts[i_cnt].use = D_ON;
		i_offset = (int)((unsigned char)(*cp_cur));
		i_offset += ( 256 * (int)((unsigned char)(*(cp_cur+1))) );
		ts_ts[i_cnt].text = cp_infile;
		ts_ts[i_cnt].text += (1 + i_offset);
		ts_ts[i_cnt].partname = ((char)DC_START_CHAR_FMSSG+(char)i_cnt);
		if( i_cnt == 0){
			if( *cp_cur != 0x18 ){
				/* with on voice-datas */
				/* V1.01b ボイスデータがない場合がある(古いmc.exe等) */
				if( *(cp_infile+1+0x18) == *(cp_infile+1+0x16) &&
					*(cp_infile+1+0x18+1) == *(cp_infile+1+0x16+1) ){
					/* ボイスデータアドレスがＲパートアドレスと同じ場合 */
					/* ボイスデータは存在しない(Ver4.5以前?) */
				}else{
					cp_tmp = cp_infile + 1 + 0x18;
					i_offset = (int)((unsigned char)(*cp_tmp));
					i_offset += ( 256 * (int)((unsigned char)(*(cp_tmp+1))) );
					cp_voiceaddr = cp_infile + 1 + i_offset;
					gt_ginfo.cp_voice = cp_voiceaddr;
					sprintf(gcs_line," [DLL] Findout voice-datas.\n");
					hfpf( DH_BMESSAGES , gcs_line );
				}
			}
		}
		if( *(ts_ts[i_cnt].text) == (char)0x80 ){
			/* 長さゼロのパート */
			sprintf(gcs_line," [DLL] Part-%c no-datas.\n",
				ts_ts[i_cnt].partname);
			hfpf( DH_BMESSAGES , gcs_line );
			ts_ts[i_cnt].use = D_OFF;

		}else{
			/* 初期メモリを与える */
			ts_ts[i_cnt].naddr = newbon(NULL);
			ts_ts[i_cnt].caddr = newboc(NULL);
		}

		cp_cur++;
		cp_cur++;
	}

	tp_BOMtop = newbom(NULL);
	gi_bomnum = 0;

	return;
}
/*--------------------------------------------------------------------*/
/* 解析系 */
/* トラック解析（シーケンシャル） */
void anltrack( ){

	char *cp_cur;
	int i_steps;
	int i_ret;
	long l_ret;
	unsigned char uc_ret;
	T_BON *tp_boncur;
	T_BOC *tp_boccur;
	long l_clock;
	T_TCODE *tp_cur;
	int i_lasttype=-1;
	long l_nextaddclock;

	/* B.O.Addresses (最初の一個は使わない) */
	tp_boncur = tp_curtra->naddr;
	tp_boncur++;
	tp_boccur = tp_curtra->caddr;
	tp_boccur++;
	l_clock = 0;


	tracklen( D_OP_TLINIT , 0 );
	for(cp_cur=tp_curtra->text;;){
		l_nextaddclock = 0;
		if( *cp_cur == (char)0x80 ){
			/* トラック終了処理 */
			if( (gtp_gop->i_dumpmode & DN_DL_SUPERDUMP) != 0 ){
				/* スーパーダンプ終了処理 */
				dumps1op( NULL );
			}
			if( (gtp_gop->i_dumpmode & DN_DL_FRAMEDUMP) != 0 ){
				/* ダンプ終了処理 */
				dump1op( NULL );
			}
			tp_boncur->toffset = l_clock;
			tp_boncur->note1 = *cp_cur;
			tp_boccur->toffset = l_clock;
			tp_boccur->code = *cp_cur;
			tp_curtra->partlen = (int)tracklen( D_OP_TLTOTAL , 0 );
			tp_curtra->tracklenbl = (int)tracklen( D_OP_TLBEFOREL , 0 );
			tp_curtra->tracktotal = tp_curtra->partlen;
			if( tp_curtra->tracklenbl >= 0 ){
				tp_curtra->tracklenal =
					tp_curtra->tracktotal - tp_curtra->tracklenbl;
			}else{
				tp_curtra->tracklenal = -1;
			}
			break;
		}
		/* コードサーチ */
		tp_cur = codefinderbystr( cp_cur );
		if( tp_cur == NULL ){
			sprintf(gcs_line,"E[DLL] Syntax error(unknown code). [%c:%02X]\r\n",
				tp_curtra->partname,*cp_cur);
			hfpf( DH_BMESSAGES , gcs_line );
			exit(-1);
		}
		/* データ長の確定 */
		i_steps = tp_cur->datasize + 1;

		/* 全体の記録 */
		switch( (int)((unsigned char)(*cp_cur)) ){
			case 0xF6:
				/* 'L' */
				gt_ginfo.i_Lcount++;
				break;
			case 0xD2:
				/* 'F' */
				gt_ginfo.i_Fcount++;
				break;
			default:
				break;
		}

		/* ここに来た場合は問答無用で取得する */
		if( tp_cur->table == 1 ){
			/* コードサーチ table1(=note)格納部分 */
			/* notes or control code (音階を持つ：ポルタメント／転調) */
			i_lasttype = 1;
			findnote( cp_cur , tp_boncur , l_clock );
			if( tp_cur->steps == D_ON ){
				/* 長さを持つ場合のみ */
				l_clock = tp_boncur->toffset + tp_boncur->length;
				/* 長さの計上 */
				l_nextaddclock = (long)tp_boncur->length;
			}
			tp_boncur = tp_boncur->newarea;
		}else if( tp_cur->table == 0 ){
			/* コードサーチ table1(=cc)格納部分 */
			/* control code (音階・長さを持たない) */
			i_ret = findcontrol( cp_cur , tp_boccur , l_clock );
			if( i_ret == 0 ){
				tp_boccur = tp_boccur->newarea;

				/* 後続とのコンバインを禁止する */
				if( i_lasttype == 0 ){
					tp_boccur->befarea->nocombine = D_ON;
				}else if( i_lasttype == 1 ){
					tp_boncur->befarea->nocombine = D_ON;
				}
				i_lasttype = 0;

				/* [:] ループの場合はその区間を乗算する処理 */
				if( *cp_cur == (char)0xF9 ){
					/* 始点 */
					tracklen( D_OP_TLLSTART , 0 );
				}else if( *cp_cur == (char)0xF8 ){
					/* 終点 */
					uc_ret = (unsigned char)(*(cp_cur+1));
					tracklen( D_OP_TLLEND , (long)(uc_ret) );
				}else if( *cp_cur == (char)0xF6 ){
					/* L */
					tracklen( D_OP_TLFINDL , 0 );
				}else if( *cp_cur == (char)0xF7 ){
					/* : */
					tracklen( D_OP_TLFINDCL , 0 );
				}
			}else if( i_ret > 0 ){
				/* 無視するコードだった場合 */
				/* なにもしない */
			}else if( i_ret < 0 ){
				/* こんなところに音符を発見してしまった場合→ロジック変 */
				sprintf(gcs_line,"E[DLL] Bad sequence.\r\n");
				hfpf( DH_BMESSAGES , gcs_line );
				exit(-1);
			}
		}else{
			/* error */
			sprintf(gcs_line,"E[DLL] Syntax error(unknown code).\r\n");
			hfpf( DH_BMESSAGES , gcs_line );
			exit(-1);
		}

		/* ダンプ・スーパーダンプ */
		if( (gtp_gop->i_dumpmode & DN_DL_SUPERDUMP) != 0 ){
			dumps1op( cp_cur );
		}
		if( (gtp_gop->i_dumpmode & DN_DL_FRAMEDUMP) != 0 ){
			dump1op( cp_cur );
		}

		/* 経過クロックの加算はここで */
		tracklen( D_OP_TLADD , l_nextaddclock );

		cp_cur += i_steps;
	}
	return;
}
/*--------------------------------------------------------------------*/
/* 解析系 */
/* トラック走査(control-trackからmaster-trackへ移動) */
void part2master( ){

	int i_steps;
	int i_ret;
	T_BOM *tp_bomcur;
	T_BOC *tp_boccur;
	long l_clock;
	T_TCODE *tp_cur;

	/* B.O.Addresses (最初の一個は使わない) */
	tp_bomcur = tp_BOMtop;
	tp_bomcur++;
	tp_boccur = tp_curtra->caddr;
	tp_boccur++;

	for(;tp_boccur->code != (char)0x80;){
		if( (unsigned char)(tp_boccur->code) == (unsigned char)0xDF ){
			/* Z 発見 */
			tp_bomcur->toffset = tp_boccur->toffset;
			tp_bomcur->barlen = tp_boccur->value;
			tp_bomcur->code = tp_boccur->code;
			tp_bomcur = tp_bomcur->newarea;
			gi_bomnum++;
			/* control-table の Z を消す */
			tp_boccur->newarea->befarea = tp_boccur->befarea;
			tp_boccur->befarea->newarea = tp_boccur->newarea;
		}
		tp_boccur = tp_boccur->newarea;
		/* 終端に到達している場合は次のエリアを確保 */
		if( tp_bomcur->newarea == NULL ){ newbom( tp_bomcur ); }
	}

	return;
}
/*--------------------------------------------------------------------*/
/* 解析系 */
/* トラック走査(silent-trackをdelete) */
/* 最初の音長を持つオブジェクトが休符だった場合、ついでにoctをあわせる*/
int delsilenttrack( ){

	T_BON *tp_boncur;
	T_BON *tp_bon1r;
	int i_firstflag;

	i_firstflag = D_OFF;
	/* B.O.Addresses (最初の一個は使わない) */
	tp_boncur = tp_curtra->naddr;
	tp_boncur++;

	for(;tp_boncur->note1!=(char)0x80;){
		if( ((char)(tp_boncur->note1)&0x80) == 0 &&
			((char)(tp_boncur->note1)&0x0F) != 0x0F ){
			if( i_firstflag == D_ON ){
				tp_bon1r->oct = tp_boncur->oct;
				tp_curtra->startoct = tp_boncur->oct;
				(tp_curtra->naddr)->oct = tp_boncur->oct;
				return( D_ON );
			}else{
				return( D_ON );
			}
		}else if( i_firstflag != D_ON &&
			((char)(tp_boncur->note1)&0x0F) == 0x0F ){
			i_firstflag = D_ON;
			tp_bon1r = tp_boncur;
		}
		tp_boncur = tp_boncur->newarea;
	}

	sprintf(gcs_line," [DLL] Silent part:%c.\r\n",tp_curtra->partname);
	hfpf( DH_BMESSAGES , gcs_line );
	tp_curtra->use = D_OFF;
	return(D_OFF);
}
/*--------------------------------------------------------------------*/
/* 解析系 */
/* トラック走査(タイで結ばれた同一ノート音符・休符同士を一度繋げる) */
/* また、最初の音符を見つけた時点で、先頭まで遡ってoctを書き戻す */
int combinenotes( int i_partno ){

	T_BON *tp_boncur;
	T_BON *tp_bonrev;
	int i_firstnote;

	/* B.O.Addresses (最初の一個は使わない) */
	tp_boncur = tp_curtra->naddr;
	tp_boncur++;

	i_firstnote=D_OFF;

	for(;tp_boncur->note1!=(char)0x80;){
		if( tp_boncur->note1 > 0 &&
			(char)(tp_boncur->note1&0x0F) != (char)0x0F &&
			i_firstnote == D_OFF ){
			/* パート内で最初の音符だった場合 */
			/* 最初の音符でo値を決定 */
			tp_curtra->startoct = 1+(0x70&tp_boncur->note1)/0x10;
			/* それ以前のbonにおけるo値も一致させる必要がある */
			for(tp_bonrev=tp_boncur->befarea
				;tp_bonrev!=NULL
				;tp_bonrev=tp_bonrev->befarea){
				/* boncurを遡りながらoctを逆方向に延長 */
				tp_bonrev->oct = tp_curtra->startoct;
			}
			i_firstnote = D_ON;
		}
		if( tp_boncur->nocombine == D_ON ){
			/* コンバイン禁止 */
			/* 音符長の集計（標準長の算出に使用） v1.01 */
			if( tp_boncur->length <= DN_MAX_NOTELENFREQ ){
				iss_notelenfreq[i_partno][(int)tp_boncur->length]++;
			}
			tp_boncur = tp_boncur->newarea;
			continue;
		}
		if( tp_boncur->note1 > 0 ){
			if( (char)(tp_boncur->note1&0x0F) == (char)0x0F ){
				/* 休符の場合 */
				if( tp_boncur->newarea->note1 > 0 &&
					(char)(tp_boncur->newarea->note1) == (char)0x0F ){
					/* 続いて休符 */
					tp_boncur->length =
						tp_boncur->length + tp_boncur->newarea->length;
					delbon( tp_boncur->newarea );
					/* 次の次と繋がる可能性があるのでポインタ進めずもう一度 */
					continue;
				}
			}else{
				/* 音符の場合 */
				if( (char)(tp_boncur->newarea->note1) == (char)0xFB &&
					( (char)(tp_boncur->newarea->newarea->note1) ==
						(char)(tp_boncur->note1) ||
					(char)(tp_boncur->newarea->newarea->note1) ==
						(char)0x0F ) ){
					/* タイで繋がれて同じ音が出ているか休符に繋がる場合 */
					tp_boncur->length = tp_boncur->length +
						tp_boncur->newarea->newarea->length;
					delbon( tp_boncur->newarea );
					delbon( tp_boncur->newarea );
					/* 次の次と繋がる可能性があるのでポインタ進めずもう一度 */
					continue;
				}
			}
		}

		/* 音符長の集計（標準長の算出に使用） v1.01 */
		if( tp_boncur->length <= DN_MAX_NOTELENFREQ ){
			iss_notelenfreq[i_partno][(int)tp_boncur->length]++;
		}

		tp_boncur = tp_boncur->newarea;
	}
	return(0);
}
/*--------------------------------------------------------------------*/
/* 解析系 */
/* oct辻褄合わせ */
void editoct(){

	T_BON *tp_boncur;
	int i_firstnote=D_OFF;
	int i_ctranso;
	int i_ctransn;

	/* B.O.Addresses (最初の一個は使わない) */
	tp_boncur = tp_curtra->naddr;
	tp_boncur++;

	for(;tp_boncur->note1!=(char)0x80;){
		if( (char)(tp_boncur->note1&0x0F) == (char)0x0F ||
			tp_boncur->note1 < 0 ){
			/* 休符またはコントロールコードの場合 */
			tp_boncur->oct = tp_boncur->befarea->oct;
		}
		tp_boncur = tp_boncur->newarea;
	}

	/* V1.21 2008/7/14 start 転調を音程に反映させる */
	if( gtp_gop->c_autotrans != D_ON ){ return; }

	tp_boncur = tp_curtra->naddr;
	tp_boncur++;
	for(;tp_boncur->note1!=(char)0x80;){
		if( (char)(tp_boncur->note1) > 0 &&
			(char)(tp_boncur->note1&0x0F) != (char)0x0F  ){
			/* 休符以外である場合 */
			i_ctranso = tp_boncur->trans / 12;
			i_ctransn = tp_boncur->trans % 12;
			tp_boncur->oct += i_ctranso;
			if( (tp_boncur->note1&0x0F)+i_ctransn > 0x0b ){
				/* オクターブupなので+4 */
				tp_boncur->note1 += (i_ctransn+4);
				tp_boncur->oct++;
			}else if( (tp_boncur->note1&0x0F)+i_ctransn < 0x00 ){
				/* オクターブdownなので-4 */
				tp_boncur->note1 += (i_ctransn-4);
				tp_boncur->oct--;
			}else{
				tp_boncur->note1 += i_ctransn;
			}
			if( tp_boncur->note2 != 0x00 ){
				if( (tp_boncur->note2&0x0F)+i_ctransn > 0x0b ){
					/* オクターブupなので+4 */
					tp_boncur->note2 += (i_ctransn+4);
				}else if( (tp_boncur->note2&0x0F)+i_ctransn < 0x01 ){
					/* オクターブdownなので-4 */
					tp_boncur->note2 += (i_ctransn-4);
				}else{
					tp_boncur->note2 += i_ctransn;
				}
			}
		}
		tp_boncur = tp_boncur->newarea;
	}
	/* V1.21 2008/7/14 end */

	return;
}
/*--------------------------------------------------------------------*/
/* 解析系 */
/* トラック走査(master-trackをsort ついでに重複しているZを整理) */
int sortmaster( ){

	char cp_cur;
	int i_cnt;
	int i_steps;
	int i_ret;
	T_BOM *tp_bomcur;
	T_BOM *tp_lastZbomcur;
	long l_lastZclock=0;
	char c_barlencur=96;
	int i_firstZ=D_OFF;
	int i_Znotonline=D_OFF;
	long l_befsectionlen;

	/* シーケンス順に整列 */
	for(i_steps=gi_bomnum-1;i_steps>0;i_steps--){
		tp_bomcur = tp_BOMtop;
		tp_bomcur = tp_bomcur->newarea;
		for(i_cnt=0;i_cnt<i_steps;i_cnt++){
			if( tp_bomcur->toffset > tp_bomcur->newarea->toffset ){
				tp_bomcur->newarea->newarea->befarea = tp_bomcur;
				tp_bomcur->befarea->newarea = tp_bomcur->newarea;
				tp_bomcur->newarea->befarea = tp_bomcur->befarea;
				tp_bomcur->befarea = tp_bomcur->newarea;
				tp_bomcur->newarea = tp_bomcur->befarea->newarea;
				tp_bomcur->befarea->newarea = tp_bomcur;
				tp_bomcur = tp_bomcur->newarea;
			}else{
				tp_bomcur = tp_bomcur->newarea;
			}
		}
	}

	/* コントロールトラックの終点を明示 */
	tp_bomcur = tp_BOMtop;
	tp_bomcur++;
	tp_bomcur+=gi_bomnum;
	tp_bomcur->last = D_ON;

	/* 連続する、同値あるいは同時のコントロールコマンドを整理 */
	tp_bomcur = tp_BOMtop;
	tp_bomcur++;
	for(;tp_bomcur->last!=D_ON;){
		if( (unsigned char)(tp_bomcur->code) == (unsigned char)0xDF ){
			/* Zの場合 */
			if( i_firstZ == D_OFF ){
				/* 最初の一個 */
				i_firstZ = D_ON;
			}else if( tp_bomcur->barlen == (unsigned char)c_barlencur ){
				/* 現在適用中のものと同一長のbarlen→単純に消去 */
				tp_bomcur->newarea->befarea = tp_bomcur->befarea;
				tp_bomcur->befarea->newarea = tp_bomcur->newarea;
				tp_bomcur = tp_bomcur->newarea;
				continue;
			}else if( l_lastZclock != 0 &&
					tp_bomcur->toffset == l_lastZclock ){
				/* 前回指定時と同一グリッド上のbarlen→古いほうを消す */
				/* 古いほうを消す根拠： */
				/*   →同時指定の場合、後ろのトラックが優先される規則 */
				tp_lastZbomcur->befarea->newarea = tp_lastZbomcur->newarea;
				tp_lastZbomcur->newarea->befarea = tp_lastZbomcur->befarea;
				gt_ginfo.i_Zcount--;
			}
			/* Ｚコマンドが小節線上に存在するかどうかのチェック */
			l_befsectionlen = tp_bomcur->toffset - l_lastZclock;
			if( l_befsectionlen % (long)((unsigned char)(c_barlencur)) != 0){
				/* 現在のＺコマンドが小節線上でないところに存在する */
				i_Znotonline = D_ON;
			}
			/* 今回の記録 */
			tp_lastZbomcur = tp_bomcur;
			l_lastZclock = tp_bomcur->toffset;
			c_barlencur = tp_bomcur->barlen;
			gt_ginfo.i_Zcount++;
		}
		tp_bomcur = tp_bomcur->newarea;
	}

	gt_ginfo.i_Znotonline = i_Znotonline;
	return( i_Znotonline );
}
/*--------------------------------------------------------------------*/
/* 解析系 */
/* Ｚを考慮した小節判定 -1:no 0:Z式 1-:指定数値 */
int barcheck( ){

	T_BOM *tp_bomcur;

	if( gtp_gop->i_barlevel == D_OFF ){
		/* 小節区切りしない */
		return( -1 );
	}else if( gtp_gop->i_barlevel == DA_FORCED ){
		/* 強制的に実施 */
		if( gtp_gop->i_zenlen > 0 ){
			return( gtp_gop->i_zenlen );
		}else{
			return( -1 );
		}
	}

	if( gt_ginfo.i_Zcount <= 0 ){
		/* 小節長指定がない */
		if( gt_ginfo.i_ILLcount > 0 ){
			/* 局所無限ループがある→測定不能 */
			return( -1 );
		}
		if( gt_ginfo.i_Fcount > 0 || gt_ginfo.i_Lcount <= 0 ){
			/* フェードアウトがある or ループがない →ループしない曲 */
			return( gtp_gop->i_zenlen );
		}else{
			/* ループする曲 */
			return( gtp_gop->i_zenlen );
		}
	}else{
		if( gt_ginfo.i_Znotonline == D_OFF ){
			/* 小節長指定が全て小節線上に存在 */
			tp_bomcur = tp_BOMtop;
			tp_bomcur++;
			for(;tp_bomcur->last!=D_ON;tp_bomcur=tp_bomcur->newarea){
				
			}
			return( 0 );
		}else{
			/* 小節線上でないところに小節長が存在 */
			return( -1 );
		}
	}

	return( -1 );
}
/*--------------------------------------------------------------------*/
/* 解析系 */
/* クロック・小節判定 */
int analclocklist( int i_bartype ){

	int i_cnt1;
	int i_cnt2;
	long ls_plen[DN_MAX_TRACKNUM];
	int i_plennum=0;
	long ls_blen[DN_MAX_TRACKNUM];
	int i_blennum=0;
	int i_flag1;
	int i_flag2;
	long l_maxlen;

	long l_maxvalue;
	long l_tmp;
	int i_loopcount;

	/* ループ後長さ */
	for(i_cnt1=0;i_cnt1<DN_MAX_TRACKNUM;i_cnt1++){ ls_plen[i_cnt1] = 0; }
	for(i_cnt1=0;i_cnt1<DN_MAX_TRACKNUM;i_cnt1++){
		if( ts_ts[i_cnt1].use != D_ON ){ continue; }
		for(i_cnt2=0;i_cnt2<DN_MAX_TRACKNUM;i_cnt2++){
			if( ls_plen[i_cnt2] == ts_ts[i_cnt1].tracklenal ){
				/* 過去に記録したpartlengthと等しい場合は */
				break;
			}
			/* 初めて出現するpartlengthは記録する */
			if( ls_plen[i_cnt2] == 0){
				ls_plen[i_cnt2] = ts_ts[i_cnt1].tracklenal;
				break;
			}
		}
	}
	/* ループ前長さ */
	for(i_cnt1=0;i_cnt1<DN_MAX_TRACKNUM;i_cnt1++){ ls_blen[i_cnt1] = 0; }
	for(i_cnt1=0;i_cnt1<DN_MAX_TRACKNUM;i_cnt1++){
		if( ts_ts[i_cnt1].use != D_ON ){ continue; }
		for(i_cnt2=0;i_cnt2<DN_MAX_TRACKNUM;i_cnt2++){
			if( ls_blen[i_cnt2] == ts_ts[i_cnt1].tracklenbl ){
				/* 過去に記録したpartlengthと等しい場合は */
				break;
			}
			/* 初めて出現するpartlengthは記録する */
			if( ls_blen[i_cnt2] == 0){
				ls_blen[i_cnt2] = ts_ts[i_cnt1].tracklenbl;
				break;
			}
		}
	}

	/* ループ後長さリストはソートする -1 → 大 →小 */
	for(i_loopcount=0;i_loopcount<DN_MAX_TRACKNUM;i_loopcount++){
		for(i_cnt1=0;ls_plen[i_cnt1]!=0&&ls_plen[i_cnt1+1]!=0;i_cnt1++){
			if( ls_plen[i_cnt1+1] == -1 ||
				ls_plen[i_cnt1] > ls_plen[i_cnt1+1] ){
				l_tmp = ls_plen[i_cnt1];
				ls_plen[i_cnt1] = ls_plen[i_cnt1+1];
				ls_plen[i_cnt1+1] = l_tmp;
			}
		}
	}

	/* 小節線処理の起点の指定がある場合 */
	switch( gtp_gop->i_autothresh ){
		case 2:
			/* データ先頭に固定 */
			return(D_BAR_TOPSTART);
		default:
			break;
	}

	/* ループしており、全パート同一でzenlenの倍数 */
	if( ls_plen[1] == 0 && ls_plen[0] % gi_zenlen == 0 ){
		return(D_BAR_FULLAUTO1);
	}

	/* L が存在していないパート以外は長さが同一 */
	if( ls_plen[2]==0&&ls_plen[0]==-1 ){
		return(D_BAR_FULLAUTO1);
	}

	/* L が存在しているパートは長さが全てzenlenの倍数、 */
	/* かつ最大のものが他全てのパートの最小公倍数 */
	i_flag1 = D_OFF;
	i_flag2 = D_ON;
	l_maxlen = 0;
	for(i_cnt1=0;ls_plen[i_cnt1]!=0;i_cnt1++){
		if( ls_plen[i_cnt1] % gi_zenlen != 0 ){ i_flag1=D_ON; }
		if( ls_plen[i_cnt1] > l_maxlen ){ l_maxlen = ls_plen[i_cnt1]; }
	}
	if( i_flag1 == D_OFF && l_maxlen > 0 ){
		/* zenlenの倍数という点はクリア */
		i_flag2 = D_OFF;
		for(i_cnt1=0;ls_plen[i_cnt1]!=0;i_cnt1++){
			if( ls_plen[i_cnt1] == -1 ){ continue; }
			if( l_maxlen % ls_plen[i_cnt1] != 0 ){
				i_flag2 = D_ON;
			}
		}
	}
	if( i_flag1 == D_OFF && i_flag2 == D_OFF ){
		/* 両方ともクリアしているので、FULLAUTOに準じる */
		return(D_BAR_FULLAUTO1);
	}



	/* 変 */
	return(D_BAR_MANUAL);
}
/*--------------------------------------------------------------------*/
/* 解析系 */
/* 小節線を跨いでいる音符を分割し、小節線（空白で表示）を挿入 */
/* １回の処理で１パートのみ実施 */
void dividebybar( int i_type ){

	T_BOC *tp_boccur;
	T_BON *tp_boncur;
	T_BON *tp_bonadd;
	long l_curcnt=0;
	long l_befcnt=0;
	long l_nxtcnt=0;
	long l_toffset;
	long l_coffset;
	long l_curpoint=0;
	long l_curbar=0;
	int i_ret;
	long l_NGlevel=-1;
	int i_updateclock=D_ON;

	/* B.O.Addresses (最初の一個は使わない) */
	tp_boccur = tp_curtra->caddr;
	tp_boccur++;
	tp_boncur = tp_curtra->naddr;
	tp_boncur++;

	/* 実装していない解析ルーチンTYPEは跳ね返す */
	switch(i_type){
		case D_BAR_TOPSTART:
			break;
		case D_BAR_FULLAUTO1:
			break;
		case D_BAR_FORCEDAUTO:
			break;
		default:
			return;
	}

	tracklen(D_OP_TLINIT,0);
	/* D_BAR_FULLAUTO1 */
	/* Lがない場合は効果音パートとして無視 */
	if( tp_curtra->tracklenbl < 0 ){ return; }
	/* オフセット算出 */
	if( i_type == D_BAR_TOPSTART ){
		l_toffset = gtp_gop->i_manoffset;
	}else{
		l_toffset = ( tp_curtra->tracklenbl + gtp_gop->i_manoffset ) %
			gi_zenlen;
	}
	/* note/ccのテーブルを総浚い */
	while(tp_boncur->newarea!=NULL||tp_boccur->newarea!=NULL){
		/* notesトラック終了で小節線挿入処理自体を終了 */
		if( (char)tp_boncur->note1 == (char)0x80 ){
			break;
		}
		/* 実質pointerをチェック */

		/*------------------------------------------------------------*/
		/* 新ルーチン */
		/* 実質pointerはcc/notesを追い越してはいけない前提 */
		/* 初期値の記録レベルは-1 */
		/* TLLEVEL TLTLEVELを新設する */

		/* "["でレベルが上がったら、TLLSTARTのあとでclockを見る */
		/* clockが小節線からズレていたら、TLLEVEL戻り値のレベルを記録する */
		/* (TLLEVELは、現在のレベルを返す) */

		/* "]"でレベルが下がったら、TLENDのあとTLLEVELを見て */
		/* 戻り値が記録しているレベルより低くなったら、 */
		/* そのレベルを消去(-1)する そうでなければ放置 */

		/* 小節線判定に使うのはTLTLEVELを使う */
		/* (TLTLEVELは、現在のレベルになってからのclockを返す) */
		/* 但し、TLLEVELがゼロのときだけはTLTOTALを使う */
		/* 小節線判定時、-1なら無条件でTLTOTALを信じる */
		/* そうでなければ、狂っているので信用しない */
		/*------------------------------------------------------------*/

		/* ccとnotesどちらが先にくるか判定(これはtoffsetの比較でできる) */
		/* ちなみに、ccがnotesより先に来ることはありえないので */
		/* パターンは「notesが先着」か「同着」しかない */
		if( tp_boncur->toffset == tp_boccur->toffset ){
			/* 同着であった場合 */
			if( (char)tp_boccur->code == (char)0xF9 ){
				/* 始点 */
				/* レベルが上がったのでズレがないかチェックする */
				i_ret = tracklen( D_OP_TLLEVEL , 0 );
				if( i_ret == 0 ){
					/* レベルゼロの場合はオフセットを考慮かつTOTALを見る */
					i_ret = tracklen( D_OP_TLTOTAL , 0 );
					if( i_ret % gi_zenlen != l_toffset ){
						/* ズレている */
						l_NGlevel = 1;
					}
				}else{
					/* レベル１以上の場合はオフセットを無視しTLTLEVELを見る */
					i_ret = tracklen( D_OP_TLTLEVEL , 0 );
					if( i_ret % gi_zenlen != 0 ){
						/* ズレている */
						l_NGlevel = tracklen( D_OP_TLLEVEL , 0 ) + 1;
					}
				}
				/* レベルを上げる */
				tracklen( D_OP_TLLSTART , 0 );
			}else if( (char)tp_boccur->code == (char)0xF8 ){
				/* 終点 */
				/* レベルが下がったのでズレがないところまで戻っているか */
				i_ret = tracklen( D_OP_TLLEVEL , 0 );
				if( l_NGlevel == i_ret ){
					/* ズレていないレベルまで戻ってきた */
					l_NGlevel = -1;
				}
				tracklen( D_OP_TLLEND , (char)tp_boccur->value );
			}else if( (char)tp_boccur->code == (char)0xF7 ){
				/* : */
				tracklen( D_OP_TLFINDCL , 0 );
			}else{
				/* レベルが変わらない */
			}

			/* clock信頼性チェック */
			if( l_NGlevel != -1 ){
				/* clock信用できない状態 */
				/* 次のcc項目まで進む */
				tp_boccur = tp_boccur->newarea;
				continue;
			}

			/* まだこの時点で同着なので、チェックしてOKだったら小節線処理 */
			if( 0 == tracklen( D_OP_TLLEVEL , 0 ) ){
				/* レベルゼロ */
				i_ret = tracklen( D_OP_TLTOTAL , 0 );
				if( i_ret % gi_zenlen == l_toffset ){
					/* 今まさに小節線上にいるので小節線処理 */
					tp_boccur->barline[0] = 0x20;
					tp_boccur->barline[1] = 0x0;
					/* 同clockにならんでいるCCは全てパスするため、 */
					/* 「小節線から動いたらD_ONになるフラグをD_OFFにする */
					/* ただし、ループ終了記号の直後は小節線を許可する */
					if( tp_boccur->code != (char)0xF8 ){
						i_updateclock = D_OFF;
					}
				}
			}else{
				/* レベル１以上 */
				i_ret = tracklen( D_OP_TLTLEVEL , 0 );
				if( i_ret % gi_zenlen == 0 ){
					/* 今まさに小節線上にいるので小節線処理 */
					tp_boccur->barline[0] = 0x20;
					tp_boccur->barline[1] = 0x0;
					/* 同clockにならんでいるCCは全てパスするため、 */
					/* 「小節線から動いたらD_ONになるフラグをD_OFFにする */
					/* ただし、ループ終了記号の直後は小節線を許可する */
					if( tp_boccur->code != (char)0xF8 ){
						i_updateclock = D_OFF;
					}
				}
			}
			/* ] 記号の手前には小節線は置かない */
			if( tp_boccur->code == (char)0xF8 ){
				tp_boccur->barline[0] = 0;
			}
			/* 次のcc項目まで進む */
			tp_boccur = tp_boccur->newarea;
			continue;
		}else{
			/* ここに来る時点で、ccよりもnotesが先着している */

			/* clock信頼性チェック */
			if( l_NGlevel != -1 ){
				/* clock信用できない状態 */
				/* notes長さの処理(TLADD)だけやって次へ */
				tracklen( D_OP_TLADD , (long)tp_boncur->length );
				/* 次のnotes項目まで進む */
				tp_boncur = tp_boncur->newarea;
				/* clockが進んだフラグをD_ON */
				i_updateclock = D_ON;
				continue;
			}

			/* 小節線の本処理 */
			if( 0 == tracklen( D_OP_TLLEVEL , 0 ) ){
				/* レベルゼロの場合 */
				i_ret = tracklen( D_OP_TLTOTAL , 0 );
				if( i_ret % gi_zenlen == l_toffset ){
					/* 今まさに小節線上にいるので小節線処理 */
					if( i_updateclock != D_OFF ){
						/* 前回小節線を引いてからclockが動いている場合のみ */
						tp_boncur->barline[0] = 0x20;
						tp_boncur->barline[1] = 0x0;
					}
				}
			}else{
				/* レベル１以上の場合 */
				i_ret = tracklen( D_OP_TLTLEVEL , 0 );
				if( i_ret % gi_zenlen == 0 ){
					/* 今まさに小節線上にいるので小節線処理 */
					if( i_updateclock != D_OFF ){
						/* 前回小節線を引いてからclockが動いている場合のみ */
						tp_boncur->barline[0] = 0x20;
						tp_boncur->barline[1] = 0x0;
					}
				}
			}

			/* 小節線上を跨いで音符が存在している場合の処理 */
			if( 0 == tracklen( D_OP_TLLEVEL , 0 ) ){
				/* レベルゼロの場合 */
				i_ret = tracklen( D_OP_TLTOTAL , 0 );
				i_ret = (i_ret-l_toffset) % gi_zenlen;
			}else{
				/* レベル１以上の場合 */
				i_ret = tracklen(D_OP_TLTLEVEL,0) % gi_zenlen;
			}
			/* l_coffset 次の小節線までの距離 */
			l_coffset = gi_zenlen - i_ret;
			/* 小節線までの距離 l_coffset よりも現在の音の音長が長い場合 */
			if( l_coffset < tp_boncur->length && tp_boncur->note2 == 0x00 ){
				/* 分割できないものは分割しない(portaments) */
				/* 個別部割り込ませ */
				insbon( tp_boncur , tp_boncur->newarea );
				/* 引き継げるパラメータは全て引き継いで、それ以外は修正 */
				tp_boncur->newarea->toffset = tp_boncur->toffset + l_coffset;
				tp_boncur->newarea->length =
					tp_boncur->length - (short)l_coffset;
				tp_boncur->newarea->note1 = tp_boncur->note1;
				tp_boncur->newarea->trans = tp_boncur->trans;
				tp_boncur->newarea->mtrans = tp_boncur->mtrans;
				tp_boncur->newarea->note1 = tp_boncur->note1;
				tp_boncur->newarea->oct = tp_boncur->oct;
				tp_boncur->length = (short)l_coffset;
				tp_boncur->ties = (char)'&';
			}

			/* notes長さの処理(TLADD) */
			tracklen( D_OP_TLADD , (long)tp_boncur->length );
			/* 次のnotes項目まで進む */
			tp_boncur = tp_boncur->newarea;
			/* clockが進んだフラグをD_ON */
			i_updateclock = D_ON;
			continue;
		}
	}

	return;
}
/*--------------------------------------------------------------------*/
/* 解析系部品 */
/* 音長ありcode */
int findnote( char *cp_cur,T_BON *tp_boncur,long l_clock ){

	T_TCODE *tp_cur;

	tp_cur = codefinderbystr( cp_cur );
	/* B.O.Notesに書き込み */
	tp_boncur->toffset = l_clock;
	tp_boncur->trans = tp_boncur->befarea->trans;
	tp_boncur->mtrans = tp_boncur->befarea->trans;
	tp_boncur->oct = tp_boncur->befarea->oct;
	/* V1.21 2008/7/14 アドレスは常に保存 */
	/* 重複したcontrolとnotes-controlの前後関係を保証するため */
	tp_boncur->address = cp_cur;
	switch( (char)(tp_cur->code1) ){
		case (char)0x7F:
			/* 通常のnotes */
			tp_boncur->length = (unsigned char)*(cp_cur+1);
			tp_boncur->note1 = *cp_cur;
			tp_boncur->note2 = 0;
			if( (char)(tp_boncur->note1&0x0F) == (char)0x0F ){
				/* 休符の場合は、その前からoctを引き継ぐ */
				tp_boncur->oct = tp_boncur->befarea->oct;
			}else{
				tp_boncur->oct = 1+((*cp_cur)&0x70)/16;
			}
			tp_boncur->ties = D_OFF;
			break;
		case (char)0xDA:
			/* ポルタメント */
			tp_boncur->length = (unsigned char)*(cp_cur+3);
			tp_boncur->note1 = *(cp_cur+1);
			tp_boncur->note2 = *(cp_cur+2);
			/* octにはポルタメント終了後の値を入れる */
			/* ポルタメントがoct境界を跨ぐような場合の問題は */
			/* ポルタメントMMLを表示する際に修正する */
			tp_boncur->oct = 1+((*cp_cur+2)&0x70)/16;
			tp_boncur->ties = D_OFF;
			tp_boncur->nocombine = D_ON;
			break;
		case (char)0xC1:
			/* && スラー */
			tp_boncur->note1 = *cp_cur;
			tp_boncur->ties = *cp_cur;
			tp_boncur->oct = tp_boncur->befarea->oct;
			break;
		case (char)0xFB:
			/* & タイ・スラー */
			tp_boncur->note1 = *cp_cur;
			tp_boncur->ties = *cp_cur;
			tp_boncur->oct = tp_boncur->befarea->oct;
			break;
		case (char)0xB2:
			/* _M マスター転調 */
			tp_boncur->note1 = *cp_cur;
			tp_boncur->note2 = *(cp_cur+1);
			tp_boncur->trans = 0;
			tp_boncur->mtrans = *(cp_cur+1);
			break;
		/* V1.21 2008/7/14 start 相対/絶対転調のbugをfix */
		case (char)0xE7:
			/* __ 相対転調 */
			tp_boncur->note1 = *cp_cur;
			tp_boncur->note2 = *(cp_cur+1);
			tp_boncur->trans = tp_boncur->befarea->trans + *(cp_cur+1);
			break;
		case (char)0xF5:
			/* _ 絶対転調 */
			tp_boncur->note1 = *cp_cur;
			tp_boncur->note2 = *(cp_cur+1);
			tp_boncur->trans = *(cp_cur+1);
			break;
		/* V1.21 2008/7/14 end */
		default:
			break;
	}

	/* 終端に到達している場合は次のエリアを確保 */
	if( tp_boncur->newarea == NULL ){
		newbon( tp_boncur );
	}

	return(tp_cur->datasize+1);
}
/*--------------------------------------------------------------------*/
/* 解析系部品 */
/* 戻り値 0:OK 1:無視するcode -1:ERROR */
int findcontrol( char *cp_cur , T_BOC *tp_boccur , long l_clock ){

	int mmllength;
	T_TCODE *tp_cur;

	tp_cur = codefinderbystr( cp_cur );
	if( tp_cur == NULL ){
		/* ロジックやばい */
		sprintf(gcs_line,"E[DLL] Internal error(unknown code 2).\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
		exit(-1);
	}
	if( tp_cur->format == 0 ){
		/* 無視するコード */
		return(1);
	}
	if( tp_cur->format == 2 ){
		/* 音符。ありえない */
		sprintf(gcs_line,"E[DLL] Internal error(unknown code 3).\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
		exit(-1);
	}

	tp_boccur->toffset = l_clock;
	tp_boccur->code = *cp_cur;
	tp_boccur->disable = D_OFF;
	/* L を見つけた場合はフラグを立てる */
	if( 0xF6 == *cp_cur || gi_llfindout == D_OFF ){
		gi_llfindout = D_ON;
	}

	/* V1.21 2008/7/14 アドレスは常に保存 */
	/* 重複したcontrolとnotes-controlの前後関係を保証するため */
	tp_boccur->address = cp_cur;

	switch(tp_cur->format){
		case 1:
			/* 内部処理関数 #PPZ/FM3Extend */
			expset(cp_cur);
			break;
		case 3:
			/* ただ出すだけのもの L : [ */
			tp_boccur->value = 0;
			break;
		case 4:
		case 5:
		case 7:
		case 8:
			/* 途中の編集に数値が必要なものはパラメータを格納 (Z/])は必須 */
			tp_boccur->value = *(cp_cur+1);
		case 6:
			/* 出力様式を別途定めているもの→アドレス保存(のみ-V1.21) */
			break;
		default:
			/* 無視するコード、あるいは音符。ありえない */
			sprintf(gcs_line,"E[DLL] Internal error(unknown code 4).\r\n");
			hfpf( DH_BMESSAGES , gcs_line );
			exit(-1);
	}

	/* 終端に到達している場合は次のエリアを確保 */
	if( tp_boccur->newarea == NULL ){
		newboc( tp_boccur );
	}

	return( 0 );
}
