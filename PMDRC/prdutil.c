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

#include "prd.h"
#include "prdext.h"


/*--------------------------------------------------------------------*/
/* 内部処理系 */
/* EXパート発見時処理 パート定義を追加 */
int expset( char *cp_findout ){

	char *cp_cur;
	int i_offset;
	int i_cnt;
	int i_zcnt;
	T_TCODE *tp_tccur;

	int i_spoint;
	int i_epoint;
	char c_schar;

	cp_cur = cp_findout;
	cp_cur++;
	if( *cp_findout == (char)0xC6 ){
		/* FM3Extend */
		sprintf(gcs_line," [DLL] Findout FM3Extend. [Part:XYZ]\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
		i_spoint = DN_START_NUM_FM3EX;
		i_epoint = DN_START_NUM_PPZ;
		c_schar = (char)DC_START_CHAR_FM3EX;
		gt_ginfo.i_fFM3Extend = D_ON;
	}else if( *cp_findout == (char)0xB4 ){
		/* PPZExtend */
		sprintf(gcs_line," [DLL] Findout PPZExtend. [Part:abcdefgh]\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
		i_spoint = DN_START_NUM_PPZ;
		i_epoint = DN_MAX_TRACKNUM;
		c_schar = (char)DC_START_CHAR_PPZ;
		gt_ginfo.i_fPPZExtend = D_ON;
	}else{
		/* エラーです */
		sprintf(gcs_line,"E[DLL] Syntax error(ExPart code). [%02X]\r\n",
			*cp_findout );
		hfpf( DH_BMESSAGES , gcs_line );
		exit(-1);
	}

	for(i_cnt=i_spoint,i_zcnt=0;i_cnt<i_epoint;i_cnt++,i_zcnt++){
		ts_ts[i_cnt].use = D_ON;
		i_offset = (int)((unsigned char)(*cp_cur));
		i_offset += ( 256 * (int)((unsigned char)(*(cp_cur+1))) );
		if( i_offset == 0 ){
			/* Extendで増やされたパートが3(PPZなら8)未満の場合 */
			ts_ts[i_cnt].text = NULL;
			ts_ts[i_cnt].partname = (c_schar+(char)i_zcnt);
			ts_ts[i_cnt].use = D_OFF;
			sprintf(gcs_line," [DLL] Part-%c no-extend.\n",
				ts_ts[i_cnt].partname);
			hfpf( DH_BMESSAGES , gcs_line );
		}else{
			/* Extendでパートを増やす */
			ts_ts[i_cnt].text = gtp_gop->cp_indata;
			ts_ts[i_cnt].text += (1 + i_offset);
			ts_ts[i_cnt].partname = (c_schar+(char)i_zcnt);
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
		}
		cp_cur++;
		cp_cur++;
	}

	tp_tccur = codefinderbystr(cp_findout);
	if(tp_tccur == NULL){
		sprintf(gcs_line,"E[DLL] Internal error(ExPart/notExist).\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
		exit(-1);
	}
	return(tp_tccur->datasize);
}
/*--------------------------------------------------------------------*/
/* 引数のコードを記述したテーブルアドレスを返却 */
/* 主に、飛ばし用途に使用する（もちろん、解析にも使う） */
/* NULL:見つからないとき、終了コードのとき */
T_TCODE *codefinderbystr( char *cp_code ){

	T_TCODE *tp_tc;
	T_TCODE *tp_keep;
	char c_keep;

	/* notes or trackend */
	if( 0x00 <= *cp_code && *cp_code <= 0x7F ){
		return(&(ts_tc[0]));
	}else if( *cp_code == (char)0x80 ){
		return(NULL);
	}

	tp_keep = NULL;
	c_keep = 0x81;
	tp_tc = &(ts_tc[1]);
	for(;tp_tc->cont==D_NOTLAST;tp_tc++){
		if( *cp_code == tp_tc->code1 ){
			if( tp_tc->code2 == 0 ){
				/* findout */
				return(tp_tc);
			}else if( (char)(tp_tc->code2) == (char)(*(cp_code+1)) ){
				/* findout */
				return(tp_tc);
			}
		}
	}

	return(NULL);
}

/*--------------------------------------------------------------------*/
/* ユーティリティ */
/* BON個別部追加 末端に空きメモリを追加する */
/* 引数で指定された中間OBJテーブルを最後尾とみなし、追加確保・連結 */
/* 引数NULLの場合は、最初の１個であるのでアドレスを返す */
T_BON *newbon( T_BON *tp_in ){
	T_BON *tp_bon;
	T_BON *tp_cur;
	int i_cnt;
	tp_bon = apmalloc(sizeof(T_BON)*DN_GETBON);
	memset(tp_bon,0x00,sizeof(T_BON)*DN_GETBON);
	tp_cur = tp_bon;
	for(i_cnt=0;i_cnt<DN_GETBON;i_cnt++,tp_cur++){
		if( i_cnt >= (DN_GETBON-1) ){
			tp_cur->newarea = NULL;
			tp_cur->befarea = tp_cur-1;
			break;}
		tp_cur->newarea = tp_cur+1;
		if( i_cnt != 0 ){
			tp_cur->befarea = tp_cur-1;
		}else if( tp_in != NULL ){
			tp_cur->befarea = tp_in;
		}else{
			tp_cur->befarea = NULL;} }
	if(tp_in!=NULL){
		tp_in->newarea = tp_bon;
		return(tp_bon);
	}else{return(tp_bon);}
}
/*--------------------------------------------------------------------*/
/* ユーティリティ */
/* BON個別部削除 前後の個別部をショートカットし、当該個別部を削除する */
void delbon( T_BON *tp_in ){

	if( tp_in->newarea == NULL && tp_in->befarea == NULL ){
		/* 前後がない独立した個別部＝エラー */
		return;
	}

	if( tp_in->newarea != NULL ){
		/* 後ろの個別部がない */
		tp_in->befarea->newarea = NULL;
	}

	if( tp_in->befarea != NULL ){
		/* 手前の個別部がない */
		tp_in->newarea->befarea = NULL;
	}

	if( tp_in->befarea != NULL && tp_in->newarea != NULL ){
		/* 前後の個別部がある */
		tp_in->newarea->befarea = tp_in->befarea;
		tp_in->befarea->newarea = tp_in->newarea;
	}

	/* 当該個別部を捨てる */
	wastebon( tp_in );
	return;
}
/*--------------------------------------------------------------------*/
/* ユーティリティ */
/* BON個別部挿入 前後の個別部の間に、新個別部を追加する */
void insbon( T_BON *tp_bef , T_BON *tp_new ){

	T_BON *tp_cur;

	if( tp_bef == NULL || tp_bef == NULL ){
		/* 前後どちらかが欠落している＝エラー */
		return;
	}

	tp_cur = get1bon( NULL );

	tp_cur->newarea = tp_new;
	tp_cur->befarea = tp_bef;
	tp_cur->newarea->befarea = tp_cur;
	tp_cur->befarea->newarea = tp_cur;

	return;
}
/*--------------------------------------------------------------------*/
/* ユーティリティ */
/* 廃棄する個別部をストックする */
void wastebon( T_BON *tp_in ){

	if( gi_stockbons > 254 ){
		/* ストックしてある個別部が満杯 */
		return;
	}
	/* 廃棄個別部を拾って初期化 */
	gtps_stockbon[gi_stockbons] = tp_in;
	memset(tp_in,0x00,sizeof(T_BON));

	/* ストックカウンタを更新 */
	gi_stockbons++;

	return;
}
/*--------------------------------------------------------------------*/
/* ユーティリティ */
/* ストックしてある個別部を再利用する */
T_BON *reusebon( ){

	T_BON *tp_out;

	if( gi_stockbons <= 0 ){
		/* ストックしてある個別部がない */
		return(NULL);
	}

	/* ストック個別部の最後の一個を拾う */
	tp_out = gtps_stockbon[gi_stockbons-1];

	/* ストックカウンタを更新 */
	gi_stockbons--;

	return(tp_out);
}
/*--------------------------------------------------------------------*/
/* ユーティリティ */
/* １個だけ個別部を取得する 可能ならば以前廃棄したものを再利用する */
/* newbon()と違い、中身はまっさら又は保証できない状態なので注意すること */
T_BON *get1bon( T_BON *tp_in ){

static int i_lastbon=0;
static T_BON *tp_addbon;
	T_BON *tp_out;

	/* ストックを確認 */
	tp_out = reusebon();
	if( tp_out != NULL ){
		/* 廃棄物を回収できたのでこれを返す */
		memset(tp_out,0x00,sizeof(T_BON));
		if( tp_in != NULL ){
			tp_out->befarea = tp_in;
		}
		return(tp_out);
	}

	/* 廃棄物が存在しないので新しくメモリを割り当てる */
	if( i_lastbon <= 0 ){
		/* 新しいメモリもないので、apmallocで確保する */
		tp_addbon = newbon( NULL );
		i_lastbon = DN_GETBON;
	}
	tp_out = tp_addbon;
	tp_out += ( i_lastbon-1 );
	i_lastbon--;

	memset(tp_out,0x00,sizeof(T_BON));
	if( tp_in != NULL ){
		tp_out->befarea = tp_in;
	}
	return(tp_out);
}
/*--------------------------------------------------------------------*/
/* ユーティリティ */
/* BOC個別部追加 末端に空きメモリを追加する */
/* 引数で指定された中間OBJテーブルを最後尾とみなし、追加確保・連結 */
/* 引数NULLの場合は、最初の１個であるのでアドレスを返す */
T_BOC *newboc( T_BOC *tp_in ){
	T_BOC *tp_boc;
	T_BOC *tp_cur;
	int i_cnt;
	tp_boc = apmalloc(sizeof(T_BOC)*DN_GETBOC);
	memset((void *)tp_boc,0x00,sizeof(T_BOC)*DN_GETBOC);
	tp_cur = tp_boc;
	for(i_cnt=0;i_cnt<DN_GETBOC;i_cnt++,tp_cur++){
		if( i_cnt >= (DN_GETBOC-1) ){
			tp_cur->newarea = NULL;
			tp_cur->befarea = tp_cur-1;
			break;}
		tp_cur->newarea = tp_cur+1;
		if( i_cnt != 0 ){
			tp_cur->befarea = tp_cur-1;
		}else if( tp_in != NULL ){
			tp_cur->befarea = tp_in;
		}else{
			tp_cur->befarea = NULL;} }
	if(tp_in!=NULL){
		tp_in->newarea = tp_boc;
		return(NULL);
	}else{return(tp_boc);}
}
/*--------------------------------------------------------------------*/
/* ユーティリティ */
/* BOM個別部追加 末端に空きメモリを追加する */
/* 引数で指定された中間OBJテーブルを最後尾とみなし、追加確保・連結 */
/* 引数NULLの場合は、最初の１個であるのでアドレスを返す */
T_BOM *newbom( T_BOM *tp_in ){
	T_BOM *tp_bom;
	T_BOM *tp_cur;
	int i_cnt;
	tp_bom = apmalloc(sizeof(T_BOM)*DN_GETBOM);
	memset(tp_bom,0x00,sizeof(T_BOM)*DN_GETBOM);
	tp_cur = tp_bom;
	for(i_cnt=0;i_cnt<DN_GETBOM;i_cnt++,tp_cur++){
		if( i_cnt >= (DN_GETBOM-1) ){
			tp_cur->newarea = NULL;
			tp_cur->befarea = tp_cur-1;
			break;}
		tp_cur->newarea = tp_cur+1;
		if( i_cnt != 0 ){
			tp_cur->befarea = tp_cur-1;
		}else if( tp_in != NULL ){
			tp_cur->befarea = tp_in;
		}else{
			tp_cur->befarea = NULL;} }
	if(tp_in!=NULL){
		tp_in->newarea = tp_bom;
		return(NULL);
	}else{return(tp_bom);}
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
/* 管理関数 */
/* トラック長さ管理関数 */
/* 無限ループが存在する場合は返却値マイナス１ */
long tracklen( int i_op , long value ){

static long l_totallen;
static long ls_looplen[257];
static long ls_lcllen[257];
static long l_nestcount;
static long l_beforeL;
static int i_lc;
static int is_lcl[257];
static int i_init;
	long l_tmp;


	if( gi_tlinit == D_OFF ){
		gi_tlinit = D_ON;
		i_init = D_OFF;
		l_beforeL = 0;
		l_totallen = 0;
		l_nestcount = 0;
		for(i_lc=0;i_lc<256;i_lc++){ls_looplen[i_lc]=0;}
		for(i_lc=0;i_lc<256;i_lc++){ls_lcllen[i_lc]=0;}
		for(i_lc=0;i_lc<256;i_lc++){is_lcl[i_lc]=D_OFF;}
	}

	if( i_op != D_OP_TLINIT && l_nestcount == -1 ){
		return( -1 );
	}

	switch( i_op ){
		case D_OP_TLINIT:
			/* 初期化処理 */
			i_init = D_ON;
			l_beforeL = 0;
			l_totallen = 0;
			l_nestcount = 0;
			for(i_lc=0;i_lc<256;i_lc++){ls_looplen[i_lc]=0;}
			for(i_lc=0;i_lc<256;i_lc++){ls_lcllen[i_lc]=0;}
			for(i_lc=0;i_lc<256;i_lc++){is_lcl[i_lc]=D_OFF;}
			break;
		case D_OP_TLADD:
			/* 音長追加 value:音長 */
			ls_looplen[l_nestcount] += value;
			if( is_lcl[l_nestcount] == D_ON ){
				ls_lcllen[l_nestcount] += value;
			}
			break;
		case D_OP_TLLSTART:
			/* ループ開始 */
			l_nestcount++;
			break;
		case D_OP_TLLEND:
			/* ループ終了 value:ループ回数 */
			if( value == 0 ){
				/* 無限ループ */
				l_nestcount = -1;
				return(-1);
			}
			l_tmp = ls_looplen[l_nestcount] * value - ls_lcllen[l_nestcount];
			ls_looplen[l_nestcount-1] += l_tmp;
			if( is_lcl[l_nestcount-1] == D_ON ){
				ls_lcllen[l_nestcount-1] += l_tmp;
			}
			ls_looplen[l_nestcount] = 0;
			ls_lcllen[l_nestcount] = 0;
			is_lcl[l_nestcount] = D_OFF;
			l_nestcount--;
			break;
		case D_OP_TLFINDL:
			/* Lコマンド発見 */
			if( l_nestcount != 0 ){
				/* [ ] ループ中に L 発見 */
				return(-1);
			}
			l_beforeL = ls_looplen[0];
			break;
		case D_OP_TLFINDCL:
			/* :コマンド発見 */
			is_lcl[l_nestcount] = D_ON;
			break;
		case D_OP_TLTOTAL:
			/* 総合トータル表示 */
			if( l_nestcount == -1 ){
				return( -1 );
			}else{
				return(ls_looplen[0]);
			}
		case D_OP_TLTLEVEL:
			/* 現在のレベルになってからのトータルを表示 */
			return(ls_looplen[l_nestcount]);
		case D_OP_TLLEVEL:
			/* 現在のレベルを表示 */
			return(l_nestcount);
		case D_OP_TLBEFOREL:
			/* L以前長さ表示 */
			return(l_beforeL);
		default:
			break;
	}

	return(0);
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
				free( *vpp_mcur );
			}
			vpp_mcur++;
		}
		/* 最後に自分自身で使用しているメモリをリリース */
		free( vp_mtb );
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
		vp_tmp = malloc( (1+DN_SIZE_MTBUNIT*i_mtbunit) * (sizeof(void *)) );
		memcpy( vp_tmp , vp_mtb ,
			(1+DN_SIZE_MTBUNIT*(i_mtbunit-1)) * (sizeof(void *)) );
		free( vp_mtb );
		vp_mtb = vp_tmp;
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
/*  arg.1  -1:init  0-(DN_MAX_OUTBUFFER-1):handle */
/*  arg.2  出力文字列へのポインタ 返却値→ NULL:正常 cp_in:異常 */
/*         NULL時:当該ハンドルのバッファ先頭アドレスを返却 */
char *hfpf( int i_handle , char *cp_in ){

static int is_outbuffersize[DN_MAX_OUTBUFFER];
static int is_cursize[DN_MAX_OUTBUFFER];
static char *cps_cur[DN_MAX_OUTBUFFER];
static char *cps_buffer[DN_MAX_OUTBUFFER];
static int is_unit[DN_MAX_OUTBUFFER];
	char *cp_ret;
	int i_count;

	if( i_handle == -1 ){
		/* 全初期化 */
		for(i_count=0;i_count<DN_MAX_OUTBUFFER;i_count++){
			is_outbuffersize[i_count] = 0;
			is_cursize[i_count] = 0;
			cps_cur[i_count] =  NULL;
			cps_buffer[i_count] = NULL;
			is_unit[i_count] = 0;
		}
		sprintf(gcs_line," [DLL] Initialize output-buffers.\n");
		if( (gtp_gop->i_directmes&D_ON) != 0 ){
			/* 制御メッセージのダイレクト出力対応 */
			fprintf(stderr,gcs_line);
		}
		return( NULL );
	}
	/* ハンドル範囲外 */
	if( i_handle < 0 || i_handle >= DN_MAX_OUTBUFFER ){
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

	/* 非公開ファンクション */
	if( (gtp_gop->i_directmes&DA_DEBUG) != 0 && i_handle == DH_BSDUMP ){
		/* デバッグ時スーパーダンプのダイレクト出力対応 */
		fprintf(stdout,"%s",cp_in);
		return( NULL );
	}

	/* 長さチェック */
	if( cp_in != NULL && strlen(cp_in) >= DN_SIZE_LINEBUFFER ){
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
		free( cps_buffer[i_handle] );
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
