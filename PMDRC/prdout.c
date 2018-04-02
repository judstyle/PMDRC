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

#include "prd.h"
#include "prdext.h"


/*--------------------------------------------------------------------*/
/* 出力処理 */
/*--------------------------------------------------------------------*/
/* 出力系 */
/* トラック出力処理 */
void trackout( int i_partno ){

	long l_curclock;
	T_BOC *tp_boccur;
	T_BON *tp_boncur;

	int i_pdeflen=0;
	int i_count1;
	int i_maxflen=0;
	int i_2ndflen=0;
	int i_maxfnum=0;
	int i_2ndfnum=0;
	int i_totalnotes=0;
	double d_maxrate;
	double d_2ndrate;
	int i_flenbig2H;
	int i_flenbig2L;

	/* B.O.Addresses (最初の一個は使わない) */
	tp_boccur = tp_curtra->caddr;
	tp_boccur++;
	tp_boncur = tp_curtra->naddr;
	tp_boncur++;

	l_curclock=0;

	/* 標準長の選出 */
	/* 標準長の設定方法↓ */
	/* ・標準長が指定されていない場合→使用しない */
	/* ・正で標準長が指定されている場合→偏りが大きい場合のみ変更する */
	/* ・負で標準長が指定されている場合→変更しないで指定通りの値を使用する */
	/* 自動算出する場合↓ */
	/* ・指定通りのものが最多の頻度で出現し２番目が(1)％以下→変更しない */
	/* ・ある要素が(2)％以上を占めている場合→1st */
	/* ・ある要素が(3-1)％以上を占めており、２番目が(3-2)％以下の場合→1st */
	/* ・２番目が(4)％以下の場合→1st */
	/* ・２番目が(5)％以上で１・２番目が８分以上・１６分だった場合→１６分 */
	/* ・最多頻度が(6)％以下の場合→ゼロ */
	/* ・上記の全てに合致しない場合→変更しない */
	if( gtp_gop->i_lendef == 0 ){
		i_pdeflen = 0;
	}else if( gtp_gop->i_autolendef == D_OFF ){
		i_pdeflen = gtp_gop->i_lendef;
	}else if( gtp_gop->i_autolendef == D_ON ){
		/* 自動判断 */
		/* 配列を検索し、最多と２番目を抽出する */
		/* スタート地点は１から←長さゼロは相手にしない */
//		for(i_count1=1;i_count1<(DN_MAX_NOTELENFREQ+1);i_count1++){
		for(i_count1=1;i_count1<(gi_zenlen/2);i_count1++){
			if( iss_notelenfreq[i_partno][i_count1] > i_maxfnum ){
				/* 最多更新 */
				i_2ndflen = i_maxflen;
				i_maxflen = i_count1;
				i_2ndfnum = i_maxfnum;
				i_maxfnum = iss_notelenfreq[i_partno][i_count1];
			}else if( iss_notelenfreq[i_partno][i_count1] > i_2ndflen ){
				/* ２番目更新 */
				i_2ndflen = i_count1;
				i_2ndfnum = iss_notelenfreq[i_partno][i_count1];
			}
			i_totalnotes+=iss_notelenfreq[i_partno][i_count1];
		}
		/* 頻度算出 */
		d_maxrate = (double)((double)iss_notelenfreq[i_partno][i_maxflen]/
			(double)i_totalnotes);
		d_2ndrate = (double)((double)iss_notelenfreq[i_partno][i_2ndflen]/
			(double)i_totalnotes);

		/* 条件分岐 */
		if( d_2ndrate <= DN_PCT_12P && gtp_gop->i_lendef == i_maxflen ){
			/* 変更ナシ */
			i_pdeflen = gtp_gop->i_lendef;
		}else if( d_maxrate >= DN_PCT_21P ){
			/* max */
			i_pdeflen = i_maxflen;
		}else if( d_maxrate >= DN_PCT_31P && d_2ndrate <= DN_PCT_32P ){
			/* max */
			i_pdeflen = i_maxflen;
		}else if( d_2ndrate <= DN_PCT_42P ){
			/* max */
			i_pdeflen = i_maxflen;
		}else if( d_2ndrate >= DN_PCT_52P ){
			/* ２番目が(5)％以上である場合、また分岐 */
			if( d_2ndrate > d_maxrate ){
				i_flenbig2H = i_2ndflen;
				i_flenbig2L = i_maxflen;
			}else{
				i_flenbig2L = i_2ndflen;
				i_flenbig2H = i_maxflen;
			}
			if( i_flenbig2L == (gi_zenlen/16) &&
				i_flenbig2H >= (gi_zenlen/8) ){
				/* １・２番目が１６分と８分（以上）の組み合わせ */
				i_pdeflen = gi_zenlen/16;
			}
		}else if( d_maxrate <= DN_PCT_61P ){
			i_pdeflen = 0;
		}else{
			/* 変更ナシ */
			i_pdeflen = gtp_gop->i_lendef;
		}
	}

	/* トラックヘッダ文字列出力 */
	if( gtp_gop->i_mmllevel < DN_OL_THROUGH ){
		/* レベル４未満のときのみ表示 */
		if( i_pdeflen == 0 ){
			/* 標準長にゼロが指定された場合は標準長を使用しない */
			/* そのため、l記号は表示しない (1.00c-) */
			sprintf(gcs_line,"%c o%d\t",
				tp_curtra->partname,
				tp_curtra->startoct);
			i_pdeflen = 0;
		}else{
			/* 正の場合は、自動判断を使用する */
			sprintf(gcs_line,"%c o%dl%d\t",
				tp_curtra->partname,
				tp_curtra->startoct,
				gi_zenlen/i_pdeflen);
		}
		hfpf( DH_BMML , gcs_line );
	}

	for(;;){
		/* V1.21 2008/7/14 start ループ終了直前の転調がループ直後に出るbugfix*/
		if( tp_boncur->note1 == (char)0x80 ){ break; }
		if( tp_boccur->toffset == l_curclock &&
			tp_boncur->address < tp_boccur->address ){
			/* clockでnotes/control同着 かつ addressでnotes先行 */
			outcharnote(tp_boncur,i_pdeflen);
			gc_lastoutcat = D_LC_NOTE;
			gc_lastoutchar = tp_boncur->note1;
			tp_boncur = tp_boncur->newarea;
			continue;
		}
		/* V1.21 2008/7/14 end */
		if( tp_boccur->toffset <= l_curclock ){
			if( tp_boccur->code == (char)0x80 ){ break; }
			/* 出力すべきcodeあり */
			outcharcode(tp_boccur);
			gc_lastoutcat = D_LC_CODE;
			gc_lastoutchar = tp_boccur->code;
			tp_boccur = tp_boccur->newarea;
			continue;
		}
		if( tp_boncur->note1 == (char)0x80 ){ break; }

		/* note出力 */
		if( tp_boncur->note1 != 0x00 ){
			outcharnote(tp_boncur,i_pdeflen);
			gc_lastoutcat = D_LC_NOTE;
			gc_lastoutchar = tp_boncur->note1;
		}
		tp_boncur = tp_boncur->newarea;
		l_curclock = tp_boncur->toffset;
	}

	/* トラックフッタ文字列出力 */
	if( gtp_gop->i_mmllevel < DN_OL_THROUGH ){
		sprintf(gcs_line,"\r\n");
		hfpf( DH_BMML , gcs_line );
	}
	return;
}
/*--------------------------------------------------------------------*/
/* 出力系 */
/* 制御系MML出力処理 */
void outcharcode( T_BOC *tp_boccur ){

	T_TCODE *tp_curcode;
	char cs_codes[256];
	char *cp_ret;

	/* control-code サーチ */
	tp_curcode = codefinderbystr( &(tp_boccur->code) );

	/* レベルチェック */
	if( tp_curcode->level < gtp_gop->i_mmllevel ){
		/* レベルに満たないので表示しない */
		return;
	}

	/* V1.21 2008/7/14 start 転調を音程に反映させる */
	if( gtp_gop->c_autotrans == D_ON &&
		(tp_boccur->code==(char)0xF5 || tp_boccur->code==(char)0xE7 ) ){
		/* c_autotransがONであり、絶対転調/相対転調の場合はスキップ */
		return;
	}
	/* V1.21 2008/7/14 end */

	/* MMLでカンマに相当するものは小節線区切りを禁止する */
	/* ex) @コマンドのループ部分は、データ上別コマンドになっているため */
	/*     前述される（はずの）@コマンドと接続しないと意味がない */
	if( *(tp_curcode->str) == (char)',' ){
		tp_boccur->barline[0] = 0x00;
	}

	if( tp_curcode == NULL ){
		/* 変 */
		return;}
	if( tp_curcode->format == 0 ||
		tp_curcode->format == 1 ||
		tp_curcode->format == 2 ){
		/* 無視 */
		return;
	}else if( tp_curcode->format == 3 ){
		/* ただ表示するだけ */
		sprintf(cs_codes,"%s%s",tp_boccur->barline,tp_curcode->str);
	}else if( tp_curcode->format == 4 ){
		/* データ(1bytes)をそのまま表示 */
		sprintf(cs_codes,"%s%s%d",tp_boccur->barline,tp_curcode->str,tp_boccur->value);
	}else if( tp_curcode->format == 5 ){
		/* データ(1bytes)をそのまま表示(符号なし) */
		sprintf(cs_codes,"%s%s%d",tp_boccur->barline,tp_curcode->str,(unsigned char)tp_boccur->value);
	}else if( tp_curcode->format == 7 ){
		/* データ(1bytes)を必ず符号をつけて表示 */
		sprintf(cs_codes,"%s%s%+d",tp_boccur->barline,tp_curcode->str,tp_boccur->value);
	}else if( tp_curcode->format == 8 ){
		/* データ(1bytes)をそのまま表示(符号あり) ゼロ時は数値省略 */
		sprintf(cs_codes,"%s%s%.d",tp_boccur->barline,tp_curcode->str,(unsigned char)tp_boccur->value);
	}else if( tp_curcode->format == 6 ){
		/* データを指定した出力フォーマットに加工して表示 */
		cp_ret = editprint( tp_curcode->outformat , tp_boccur->address );
		sprintf(cs_codes,"%s%s%s",tp_boccur->barline,tp_curcode->str,cp_ret);
	}

	if( gtp_gop->i_mmllevel <= tp_curcode->level ){
		/* 出力レベル以上のものだけを出す */
		sprintf( gcs_line , "%s" , cs_codes );
		hfpf( DH_BMML , gcs_line );
	}

	return;
}
/*--------------------------------------------------------------------*/
/* 出力系 */
/* cc特殊フォーマット出力処理 */
char *editprint( char *format , char *indata ){

	char *cp_curf;
static	char cs_outbuf[256];
	char cs_midbuf[4];
	char *cp_curo;
	int i_culc;
	int i_mid;
	char *cp_curd;
	char *cp_fmt;

	cp_curd = indata+1;
	cp_curf = format;
	memset(cs_outbuf,0x00,256);
	memset(cs_midbuf,0x00,4);
	cp_curo = cs_outbuf;

	for(cp_curf=format;*cp_curf!=0x00;cp_curf++){
		/* 解析(数値算出) */
		switch(*cp_curf){
			case '#':
			case '(':
				i_culc =  *cp_curd;
				break;
			case '$':
			case ')':
				i_culc =  *(cp_curd+1);
				i_culc =  i_culc * 256;
				i_culc += (int)(unsigned char)(*cp_curd);
				break;
			case '_':
				i_culc =  (int)(unsigned char)*cp_curd;
				break;
			case '~':
				i_culc =  (int)(unsigned char)*(cp_curd+1);
				i_culc =  i_culc * 256;
				i_culc += (int)(unsigned char)(*cp_curd);
				break;
			case ' ':
				i_culc = 0;
				break;
			case 'a':	case 'b':	case 'c':	case 'd':
			case 'e':	case 'f':	case 'g':	case 'h':
				/* 尻が有効 有効ビット数の算出 a=8/h=1 */
				i_mid = (int)(8- ( (*cp_curf) - 0x61 ));
				i_culc = powxy(2,i_mid) - 1;
				i_culc = i_culc & (int)(unsigned char)cp_curd;
				break;
			case 's':	case 't':	case 'u':	case 'v':
			case 'w':	case 'x':	case 'y':	case 'z':
				/* 頭が有効 無効ビット数の算出 z=0/s=7 */
				i_mid = (int)( 0x7A - (*cp_curf) );
				i_culc = powxy(2,i_mid) - 1;
				i_culc = (!i_culc) & (int)(unsigned char)cp_curd;
				break;
			default:
				i_culc = 0;
				break;
		}
		/* 解析(出力フォーマット) */
		switch(*cp_curf){
			case '(':
			case ')':
				cp_fmt = "%+d";
				break;
			case '#':
			case '$':
			case '_':
			case '~':
			case 'a':	case 'b':	case 'c':	case 'd':
			case 'e':	case 'f':	case 'g':	case 'h':
			case 's':	case 't':	case 'u':	case 'v':
			case 'w':	case 'x':	case 'y':	case 'z':
				cp_fmt = "%d";
				break;
			case ' ':
				cp_fmt = "";
				break;
			default:
				cs_midbuf[0] = *cp_curf;
				cs_midbuf[1] = 0;
				cp_fmt = cs_midbuf;
				break;
		}
		/* 解析(データアドレス進行) */
		switch(*cp_curf){
			case '(':
			case '#':
			case '_':
			case 'a':	case 'b':	case 'c':	case 'd':
			case 'e':	case 'f':	case 'g':	case 'h':
			case 's':	case 't':	case 'u':	case 'v':
			case 'w':	case 'x':	case 'y':	case 'z':
			case ' ':
				cp_curd++;
				break;
			case ')':
			case '$':
			case '~':
				cp_curd+=2;
				break;
			default:
				break;
		}
		sprintf(cp_curo,cp_fmt,i_culc);
		cp_curo+=strlen(cp_curo);
	}

	return(cs_outbuf);
}
/*--------------------------------------------------------------------*/
/* 出力系 */
/* ノートMML出力処理 */
void outcharnote( T_BON *tp_boncur , int i_pdeflen ){

	T_TCODE *tp_curcode;
	char cs_op[256];
	char cs_preoct[32];
	char cs_poroct[32];
	char c_octdiff;
	char *cp_curop;
	int i_tmp;
	int i_cnt1;
	unsigned char uc_curlen;
	int i_deflen;
	char c_connector='&';
	char c_addsymbol=DC_ADDSYMBOL;
	char cs_codes[256];

	cp_curop = cs_op;
	memset(cs_op,0x00,256);

	if( tp_boncur->note1 > 0 ){
		tp_curcode = codefinderbystr( &(tp_boncur->note1) );
		/* 発音部分 */
		/* octはポルタメントのときには特殊な算出 */
		if( tp_boncur->note2 != 0x00 ){
			/* ポルタメントの場合 */
			c_octdiff = 1+(0x70&tp_boncur->note1)/0x10 -
				tp_boncur->befarea->oct;
		}else{
			c_octdiff = tp_boncur->oct - tp_boncur->befarea->oct;
		}
		if( c_octdiff != 0){
			/* oct記述 */
			value2str_oct(c_octdiff,cs_preoct);
			sprintf(cp_curop,"%s",cs_preoct);
			cp_curop+=strlen(cp_curop);
		}
		if( tp_boncur->note2 != 0x00 ){
			/* ポルタメント : poroct ポルタメント中のoctを算出 */
			c_octdiff = tp_boncur->note2/0x10 - tp_boncur->note1/0x10;
			value2str_oct(c_octdiff,cs_poroct);
//			tp_boncur->oct += c_octdiff; /* 次の音符のためにoct操作 */
			sprintf(cp_curop,"{%s%s%s}",
				cps_note[tp_boncur->note1&0x0F],
				cs_poroct,
				cps_note[tp_boncur->note2&0x0F]);
			cp_curop+=strlen(cp_curop);
		}else{
			/* 音符 or 休符 */
			i_tmp = (int)((char)(tp_boncur->note1)&(char)0x0F);
			sprintf(cp_curop,"%s",cps_note[i_tmp]);
			if( i_tmp == 0x0F ){
				c_connector = 'r';
			}else{
				c_connector = '&';
			}
			cp_curop+=strlen(cp_curop);
		}
		/* 音長出力 */
/* 仕様変更： */
/* i_pdeflenにゼロがセットされた場合、従来の「デフォルト値を採用」から */
/* 「標準音長を使用しない」に変更 (1.00c) */
		if( i_pdeflen != 0 ){
			i_deflen = i_pdeflen;
		}else{
			/* とても大きい値にすれば標準音長としては成立しなくなるだろう */
			/* なおかつ素数ならさらに確率は下がる */
			i_deflen = DN_LENGTH_DEFLIMIT;
		}
		if( tp_boncur->length == i_deflen ){
			/* nop lコマンド通りの音長 */
		}else if( tp_boncur->length%3 == 0 &&
			tp_boncur->length*2/3 == i_deflen ){
			/* nop lコマンド分＋付点の音長 */
			sprintf(cp_curop,".");
		}else if( tp_boncur->length%7 == 0 &&
			tp_boncur->length*4/7 == i_deflen ){
			/* nop lコマンド分＋２付点の音長 */
			sprintf(cp_curop,"..");
		}else if( tp_boncur->length%15 == 0 &&
			tp_boncur->length*8/15 == i_deflen ){
			/* nop lコマンド分＋３付点の音長 */
			sprintf(cp_curop,"...");
		}else if( tp_boncur->length >= gtp_gop->i_lenmin ){
			/* MMLで音楽的表現で記述する音符長 */
			if( gi_zenlen < tp_boncur->length ){
				/* 全音符より長い */
				sprintf(cp_curop,"%%%d",tp_boncur->length);
			}else if( gi_zenlen == tp_boncur->length ){
				/* 全音符と同じ */
				sprintf(cp_curop,"1");
			}else if( gi_zenlen%tp_boncur->length == 0 ){
				/* 非付点音符 */
				sprintf(cp_curop,"%d",gi_zenlen/tp_boncur->length);
			}else if( tp_boncur->length % 9 == 0 &&
				/* 付点音符 */
				gi_zenlen%(tp_boncur->length*2/3) == 0 ){
				sprintf(cp_curop,"%d.",gi_zenlen/(tp_boncur->length*2/3));
			}else if( tp_boncur->length % 21 == 0 &&
				/* ２付点音符 */
				gi_zenlen%(tp_boncur->length*4/7) == 0 ){
				sprintf(cp_curop,"%d..",gi_zenlen/(tp_boncur->length*4/7));
			}else if( tp_boncur->length % 15 == 0 &&
				/* ３付点音符 */
				tp_boncur->length / 15 * 16 == gi_zenlen ){
				sprintf(cp_curop,"%d...",gi_zenlen/(tp_boncur->length*8/15));
			}else if( tp_boncur->length % 5 == 0 &&
				tp_boncur->length / 5 * 8 == gi_zenlen ){
				/* ２分＋８分音符 */
// 1.00bで直した
//				/* コンバイン禁止＝分割も禁止として判定 */
//				if( tp_boncur->nocombine != D_ON ){
//					if( i_deflen == tp_boncur->length / 5 ){
//						sprintf(cp_curop,"%c2",c_connector,tp_boncur->length);
//					}else{
						sprintf(cp_curop,"8%c2",c_addsymbol,tp_boncur->length);
//					}
//				}
			}else if( tp_boncur->length % 9 == 0 &&
				/* ２分＋１６分音符 */
				tp_boncur->length / 9 * 16 == gi_zenlen ){
// 1.00bで直した
//				/* コンバイン禁止＝分割も禁止として判定 */
//				if( tp_boncur->nocombine != D_ON ){
//					if( i_deflen == tp_boncur->length / 9 ){
//						sprintf(cp_curop,"%c2",c_connector,tp_boncur->length);
//					}else{
						sprintf(cp_curop,"16%c2",c_addsymbol,tp_boncur->length);
//					}
//				}
			}else if( tp_boncur->length % 5 == 0 &&
				/* ４分＋１６分音符 */
				tp_boncur->length / 5 * 16 == gi_zenlen ){
// 1.00bで直した
//				/* コンバイン禁止＝分割も禁止として判定 */
//				if( tp_boncur->nocombine != D_ON ){
//					if( i_deflen == tp_boncur->length / 5 ){
//						sprintf(cp_curop,"&4",tp_boncur->length);
//					}else{
						sprintf(cp_curop,"16%c4",c_addsymbol,tp_boncur->length);
//					}
//				}
			}else{
				/* 付点では表現しきれない音符長 */
				sprintf(cp_curop,"%%%d",tp_boncur->length);
			}
		}else if( tp_boncur->length % gtp_gop->i_len3min == 0 ){
			/* MMLで音楽的表現で記述する音符長（３連符） */
			if( gi_zenlen < tp_boncur->length ){
				/* 全音符より長い */
				sprintf(cp_curop,"%%%d",tp_boncur->length);
			}else if( gi_zenlen == tp_boncur->length ){
				/* 全音符と同じ */
				sprintf(cp_curop,"1");
			}else if( gi_zenlen%tp_boncur->length == 0 &&
				tp_boncur->length >= gtp_gop->i_len3min ){
				/* 非付点音符 */
				sprintf(cp_curop,"%d",gi_zenlen/tp_boncur->length);
			}else{
				/* （３連符では付点音符を使わない） */
				sprintf(cp_curop,"%%%d",tp_boncur->length);
			}
		}else{
			/* パーセント記号で記述する音符長 */
			sprintf(cp_curop,"%%%d",tp_boncur->length);
		}

		if( gtp_gop->i_mmllevel <= tp_curcode->level ){
			sprintf(gcs_line,"%s%s",tp_boncur->barline,cs_op);
			hfpf(DH_BMML,gcs_line);
			if( tp_boncur->note1 < 0x7F && tp_boncur->ties == (char)'&' ){
				/* '&'記号内蔵音符の場合(小節線上で切った場合など) */
				/* ただし、休符は除く */
				if( (char)(tp_boncur->note1&0x0F) != (char)0x0F ){
					sprintf(gcs_line,"%c",tp_boncur->ties);
					hfpf(DH_BMML,gcs_line);
				}
			}
		}

	}else{
		/* control-code サーチ */
		tp_curcode = codefinderbystr( &(tp_boncur->note1) );
		if( tp_curcode == NULL ){
			/* 変 */
			return;}
		if( tp_curcode->format == 0 ||
			tp_curcode->format == 1 ||
			tp_curcode->format == 2 ){
			/* 無視 */
			return;
		/* V1.21 2008/7/14 start 転調を音程に反映させる */
		}else if( gtp_gop->c_autotrans == D_ON &&
			(tp_curcode->code1==(char)0xF5||tp_curcode->code1==(char)0xE7 ) ){
			/* c_autotransがONであり、絶対転調/相対転調の場合はスキップ */
			return;
		/* V1.21 2008/7/14 end */
		}else if( tp_curcode->format == 3 ){
			/* ただ表示するだけ */
			sprintf(cs_codes,"%s%s",tp_boncur->barline,tp_curcode->str);
		}else if( tp_curcode->format == 4 ){
			/* データ(1bytes)をそのまま表示 */
			sprintf(cs_codes,"%s%s%d",tp_boncur->barline,tp_curcode->str,
				tp_boncur->note2);
		}else if( tp_curcode->format == 5 ){
			/* データ(1bytes)をそのまま表示(符号なし) */
			sprintf(cs_codes,"%s%s%d",tp_boncur->barline,tp_curcode->str,
				(unsigned char)tp_boncur->note2);
		}else if( tp_curcode->format == 7 ){
			/* データ(1bytes)を符号強制表示 */
			sprintf(cs_codes,"%s%s%+d",tp_boncur->barline,tp_curcode->str,
				tp_boncur->note2);
		}else if( tp_curcode->format == 8 ){
			/* データ(1bytes)を符号付ゼロ時符号なし表示 */
			sprintf(cs_codes,"%s%s%.d",tp_boncur->barline,tp_curcode->str,
				tp_boncur->note2);
		}

		if( gtp_gop->i_mmllevel <= tp_curcode->level ){
			/* 長さのないものは表示を省く場合がある */
			sprintf(gcs_line,"%s%s",tp_boncur->barline,cs_codes);
			hfpf(DH_BMML,gcs_line);
		}
	}

	return;
}
/*--------------------------------------------------------------------*/
/* 出力系 */
/* オクターブ文字列編集処理 +なら> -なら< だけ、oct記号を書き出す */
void value2str_oct( char c_in , char *cp_in ){

	int i_repeat;
	char c_outchar;

	if( c_in < 0 ){
		c_outchar = (char)'<';
		i_repeat = (int)(0-c_in);
	}else if( c_in > 0){
		c_outchar = (char)'>';
		i_repeat = (int)c_in;
	}else{
		*cp_in = 0x00;
		return;
	}

	cp_in[i_repeat] = 0x00;
	for(;i_repeat!=0;i_repeat--){
		cp_in[i_repeat-1] = c_outchar;
	}

	return;
}
/*--------------------------------------------------------------------*/
/* 出力系 */
/* 定義データを出力する */
void defineout( ){

	int i_count;


	if( gt_ginfo.cp_title != NULL &&
		((gtp_gop->i_definemode)&DA_DEFL_NORMAL) != 0 ){
		sprintf( gcs_line , DS_DEFINE_TITLE , gt_ginfo.cp_title );
		hfpf( DH_BDEFINE , gcs_line );
	}
	if( gt_ginfo.cp_composer != NULL &&
		((gtp_gop->i_definemode)&DA_DEFL_NORMAL) != 0 ){
		sprintf( gcs_line , DS_DEFINE_COMPOSER , gt_ginfo.cp_composer );
		hfpf( DH_BDEFINE , gcs_line );
	}
	if( gt_ginfo.cp_arrenger != NULL &&
		((gtp_gop->i_definemode)&DA_DEFL_NORMAL) != 0 ){
		sprintf( gcs_line , DS_DEFINE_ARRENGER , gt_ginfo.cp_arrenger );
		hfpf( DH_BDEFINE , gcs_line );
	}
	if( gt_ginfo.i_fPPZExtend == D_ON &&
		((gtp_gop->i_definemode)&DA_DEFL_PARTEXTEND) != 0 ){
		strcpy( gcs_line , DS_DEFINE_PPZEXTEND );
		hfpf( DH_BDEFINE , gcs_line );
	}
	if( gt_ginfo.i_fFM3Extend == D_ON &&
		((gtp_gop->i_definemode)&DA_DEFL_PARTEXTEND) != 0 ){
		strcpy( gcs_line , DS_DEFINE_FM3EXTEND );
		hfpf( DH_BDEFINE , gcs_line );
	}
	if( gt_ginfo.cp_pziname != NULL &&
		((gtp_gop->i_definemode)&DA_DEFL_NORMAL) != 0 ){
		sprintf( gcs_line , DS_DEFINE_PZINAME , gt_ginfo.cp_pziname );
		hfpf( DH_BDEFINE , gcs_line );
	}
	if( gt_ginfo.cp_ppsname != NULL &&
		((gtp_gop->i_definemode)&DA_DEFL_NORMAL) != 0 ){
		sprintf( gcs_line , DS_DEFINE_PPSNAME , gt_ginfo.cp_ppsname );
		hfpf( DH_BDEFINE , gcs_line );
	}
	if( gt_ginfo.cp_ppcname != NULL &&
		((gtp_gop->i_definemode)&DA_DEFL_NORMAL) != 0 ){
		sprintf( gcs_line , DS_DEFINE_PPCNAME , gt_ginfo.cp_ppcname );
		hfpf( DH_BDEFINE , gcs_line );
	}

	/* MEMO欄は複数あり得る */
	if( (gtp_gop->i_definemode)&DA_DEFL_NORMAL != 0 ){
		for( i_count=0 ; i_count<DN_MAX_MEMOPOINT ; i_count++ ){
			if( gt_ginfo.cps_memo[i_count] != NULL ){
				sprintf( gcs_line , DS_DEFINE_MEMO ,
					gt_ginfo.cps_memo[i_count] );
				hfpf( DH_BDEFINE , gcs_line );
			}else{
				break;
			}
		}
	}

	return;
}
/*--------------------------------------------------------------------*/
/* ダンプ出力処理 */
/*--------------------------------------------------------------------*/
/* 出力系 */
/* ボイスデータをダンプする */
void voiceout( ){

	char *cp_vcur;
	T_FMT t_fmtcur;
	int i_cnt;
	int is_op[4];
const int is_soptx[4]={3,1,2,0};
const int is_soppmd[4]={0,2,1,3};
	int i_fmnum;
	int i_fmalg;
	int i_fmfb;
	T_OFMT t_txvoice[4];
	T_IFMT t_voice[4];

	if( cp_voiceaddr == NULL ){
		/* ボイスアドレス見つからず→ボイスデータなしデータ */
		sprintf(gcs_line," [DLL] not built in voice-datas.\n");
		hfpf( DH_BMESSAGES , gcs_line );
		return;
	}

	switch( gtp_gop->i_voicemode ){
		case D_ON:
			memcpy(is_op,is_soppmd,sizeof(int)*4);
			break;
		case DA_TX81ZVOICE:
			memcpy(is_op,is_soptx,sizeof(int)*4);
			break;
		default:
			return;
	}


	cp_vcur = cp_voiceaddr;
	while((*cp_vcur!=0x00)||(*(cp_vcur+1)!=-1)){
		/* 先頭の音色番号にゼロが来たらvocedatas終了のポイント */
		/* V1.01a修正 0x00/0xFFの２バイトが来たら終了 */
		memcpy(&t_fmtcur,cp_vcur,sizeof(T_FMT));

		/* FM音色を整理 */
		i_fmnum = (int)(t_fmtcur.tnumber);
		i_fmalg = (int)(t_fmtcur.fbal & 0x07);
		i_fmfb  = (int)((t_fmtcur.fbal & 0x38)/0x08);

		/* 共通項目 */
		if( gtp_gop->i_voicemode == DA_TX81ZVOICE ){
			/* TX81Z型 */
			sprintf(gcs_line," @%03d\n",i_fmnum);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," Algorithm   ALG   %2d\n",i_fmalg+1);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," FeedBack    FB    %2d\n",i_fmfb);
			hfpf(DH_BVOICE,gcs_line);
		}else if( gtp_gop->i_voicemode == D_ON ){
			/* 従来PMD型 */
			sprintf(gcs_line,"; NM  AG  FB\n");
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line,"@%03d %3d %3d\n", i_fmnum , i_fmalg , i_fmfb );
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line,"; AR  DR  SR  RR  SL  TL  KS  ML  DT AMS\n");
			hfpf(DH_BVOICE,gcs_line);
		}

		/* ＦＭ音色を取り込んで内部フォーマットに変換 */
		for(i_cnt=0;i_cnt<4;i_cnt++){
			t_voice[i_cnt].i_ml =
				(int)(t_fmtcur.dtml[is_op[i_cnt]]&0x00F);
			t_voice[i_cnt].i_dt =
				(int)((t_fmtcur.dtml[is_op[i_cnt]]&0x0F0)/0x10);
			t_voice[i_cnt].i_ar =
				(int)(t_fmtcur.ksar[is_op[i_cnt]]&0x01F);
			t_voice[i_cnt].i_dr =
				(int)(t_fmtcur.amdr[is_op[i_cnt]]&0x01F);
			t_voice[i_cnt].i_sl =
				(int)((t_fmtcur.slrr[is_op[i_cnt]]&0x0F0)/0x10);
			t_voice[i_cnt].i_sr =
				(int)(t_fmtcur.sr[is_op[i_cnt]]);
			t_voice[i_cnt].i_rr =
				(int)(t_fmtcur.slrr[is_op[i_cnt]]&0x00F);
			t_voice[i_cnt].i_tl =
				(int)(t_fmtcur.tl[is_op[i_cnt]]);
			t_voice[i_cnt].i_ks =
				(int)((t_fmtcur.ksar[is_op[i_cnt]]&0x0C0)/0x40);
			t_voice[i_cnt].i_ams =
				(int)(( t_fmtcur.amdr[is_op[i_cnt]] & 0x0E0 ) / 0x20);
		}

		if( gtp_gop->i_voicemode == D_ON ){
			/* 従来PMD型 */
			for( i_cnt=0 ; i_cnt<4 ; i_cnt++ ){
				sprintf(gcs_line," %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
					t_voice[i_cnt].i_ar ,
					t_voice[i_cnt].i_dr ,
					t_voice[i_cnt].i_sr ,
					t_voice[i_cnt].i_rr ,
					t_voice[i_cnt].i_sl ,
					t_voice[i_cnt].i_tl ,
					t_voice[i_cnt].i_ks ,
					t_voice[i_cnt].i_ml ,
					t_voice[i_cnt].i_dt ,
					t_voice[i_cnt].i_ams );
				hfpf(DH_BVOICE,gcs_line);
			}
		}else if( gtp_gop->i_voicemode == DA_TX81ZVOICE ){
			/* TX81Z型 */
			for( i_cnt=0 ; i_cnt<4 ; i_cnt++ ){
				if( t_voice[i_cnt].i_ml == 0 ){
					/* Frequency : 0は0.5に変換 */
					t_txvoice[i_cnt].f_freq=(float)0.5;
				}else{
					t_txvoice[i_cnt].f_freq=
						(float)(t_voice[i_cnt].i_ml);
				}
				t_txvoice[i_cnt].i_det =
					t_voice[i_cnt].i_dt ;
				if( t_txvoice[i_cnt].i_det >= 4 ){
					/* Detune : 4～7は0～-3に変換 */
					t_txvoice[i_cnt].i_det =
						4 - t_txvoice[i_cnt].i_det;
				}
				t_txvoice[i_cnt].i_ar  = t_voice[i_cnt].i_ar;
				t_txvoice[i_cnt].i_d1r = t_voice[i_cnt].i_dr;
				t_txvoice[i_cnt].i_d1l = t_voice[i_cnt].i_sl;
				t_txvoice[i_cnt].i_d2r = t_voice[i_cnt].i_sr;
				t_txvoice[i_cnt].i_rr  = t_voice[i_cnt].i_rr;
				t_txvoice[i_cnt].i_out =
					(127-t_voice[i_cnt].i_tl) * 99 / 127;
				t_txvoice[i_cnt].i_rs  = t_voice[i_cnt].i_ks;
			}
			/* TX81Z型 */
			sprintf(gcs_line," Frequency   FREQ  %4.1f %4.1f %4.1f %4.1f\n",
				t_txvoice[0].f_freq,t_txvoice[1].f_freq,
				t_txvoice[2].f_freq,t_txvoice[3].f_freq);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," Detune      DET   %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_det,t_txvoice[1].i_det,
				t_txvoice[2].i_det,t_txvoice[3].i_det);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," AttackRate  AR    %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_ar,t_txvoice[1].i_ar,
				t_txvoice[2].i_ar,t_txvoice[3].i_ar);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," Decay1Rate  D1R   %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_d1r,t_txvoice[1].i_d1r,
				t_txvoice[2].i_d1r,t_txvoice[3].i_d1r);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," Decay1Level D1L   %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_d1l,t_txvoice[1].i_d1l,
				t_txvoice[2].i_d1l,t_txvoice[3].i_d1l);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," Decay2Rate  D2R   %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_d2r,t_txvoice[1].i_d2r,
				t_txvoice[2].i_d2r,t_txvoice[3].i_d2r);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," ReleaseRate RR    %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_rr,t_txvoice[1].i_rr,
				t_txvoice[2].i_rr,t_txvoice[3].i_rr);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," OutputLevel OUT   %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_out,t_txvoice[1].i_out,
				t_txvoice[2].i_out,t_txvoice[3].i_out);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," K.RateScale RS    %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_rs,t_txvoice[1].i_rs,
				t_txvoice[2].i_rs,t_txvoice[3].i_rs);
			hfpf(DH_BVOICE,gcs_line);
		}else{
			/* 特になし */
		}

		cp_vcur += ( sizeof(T_FMT)-1 );
		sprintf(gcs_line,"\n");
		hfpf(DH_BVOICE,gcs_line);
	}

	return;
}
/*--------------------------------------------------------------------*/
/* 出力系 */
/* １センテンスをダンプする */
void dump1op( char *cp_in ){

	T_TCODE *tp_cur;
static char c_partname=(char)' ';
static int i_numonline=0;
static int i_linenum=0;
	int i_curcodelen;

	if( cp_in == NULL ){
		/* トラック終端 */
		if( i_numonline >= 15 ){
			sprintf(gcs_line,"\n%03X| %80\n\n",i_linenum);
		}else if( i_numonline == 0 ){
			sprintf(gcs_line,"%03X| 80\n\n",i_linenum);
		}else{
			sprintf(gcs_line,"80\n\n");
		}
		hfpf( DH_BDUMP , gcs_line );
		return;
	}

	if( c_partname != tp_curtra->partname ){
		/* 初期化 */
		c_partname = tp_curtra->partname;
		i_numonline = 0;
		i_linenum = 0;
		sprintf(gcs_line,"==========================================================\n");
		hfpf( DH_BDUMP , gcs_line );
		sprintf(gcs_line," %c |  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10\n",c_partname);
		hfpf( DH_BDUMP , gcs_line );
		sprintf(gcs_line,"---+----------------------------------------------------\n");
		hfpf( DH_BDUMP , gcs_line );
	}

	tp_cur = codefinderbystr( cp_in );
	if( tp_cur->code2 == 0x00 ){
		/* １バイトcode */
		if( i_numonline == 15 ){
			sprintf(gcs_line,"%02X\n",0x000000FF&*cp_in);
			i_linenum++;
			i_numonline = 0;
		}else if( i_numonline == 0 ){
			sprintf(gcs_line,"%03X| %02X ",i_linenum,0x000000FF&*cp_in);
			i_numonline++;
		}else{
			sprintf(gcs_line,"%02X ",0x000000FF&*cp_in);
			i_numonline++;
		}
	}else{
		/* ２バイトcode */
		if( i_numonline == 15 ){
			sprintf(gcs_line,"%02X-%02X\n%03X|z    "
				,0x000000FF&*cp_in,0x000000FF&*(cp_in+1),i_linenum+1);
			i_numonline = 1;
			i_linenum++;
		}else if( i_numonline == 14 ){
			sprintf(gcs_line,"%02X-%02X\n",0x000000FF&*cp_in,
				0x000000FF&*(cp_in+1));
			i_numonline = 0;
			i_linenum++;
		}else if( i_numonline == 0 ){
			sprintf(gcs_line,"%03X| %02X-%02X ",i_linenum
				,0x000000FF&*cp_in,0x000000FF&*(cp_in+1));
			i_numonline = 2;
		}else{
			sprintf(gcs_line,"%02X-%02X ",0x000000FF&*cp_in,
				0x000000FF&*(cp_in+1));
			i_numonline+=2;
		}
	}
	hfpf( DH_BDUMP , gcs_line );

	return;
}
/*--------------------------------------------------------------------*/
/* 出力系 */
/* １センテンスをスーパーダンプする */
void dumps1op( char *cp_in ){

	T_TCODE *tp_cur;
	int i_curcode;
	int i_pnote;
	int i_ret;
	unsigned char uc_ret;
	char *cp_curstr;
	char cs_outbuf[256];
	char *cp_ret;
static long l_totalclock=0;
static char c_partname=' ';
	char c_octdiff;
	char cs_poroct[32];
	char cs_pormml[32];


	if( cp_in == NULL ){
		/* トラック終端 */
		l_totalclock = tracklen( D_OP_TLTOTAL , 0 );
		sprintf(gcs_line," 80    |   |%5d|code trackend\n",
			l_totalclock);
		hfpf( DH_BSDUMP , gcs_line );
		return;
	}

	if( c_partname != tp_curtra->partname ){
		/* 初期化 */
		c_partname = tp_curtra->partname;
		sprintf(gcs_line,"PART:%c ==========================================================\n",c_partname);
		hfpf( DH_BSDUMP , gcs_line );
		sprintf(gcs_line," codes |len| pos |     MML\n");
		hfpf( DH_BSDUMP , gcs_line );
	}

	tp_cur = codefinderbystr( cp_in );
	if( (unsigned char)0x7F >= (unsigned char)(tp_cur->code1) ){
		/* note */
		i_pnote = (int)0x0000000F&(int)(*cp_in);
		i_ret = tracklen( D_OP_TLLEVEL , 0 );
		if( i_ret == 0 ){
			l_totalclock = tracklen( D_OP_TLTOTAL , 0 );
			sprintf(cs_outbuf," %02X    |%3d|%5d|     %2s %3d\n",
				(char)*cp_in,
				(unsigned char)*(cp_in+1),
				l_totalclock,
				cps_note[i_pnote],
				(unsigned char)*(cp_in+1) );
		}else{
			sprintf(cs_outbuf," %02X    |%3d|     |     %2s %3d\n",
				(char)*cp_in,
				(unsigned char)*(cp_in+1),
				cps_note[i_pnote],
				(unsigned char)*(cp_in+1) );
		}
	}else if( (unsigned char)0xDA == (unsigned char)(tp_cur->code1) ){
		/* ポルタメント */

		c_octdiff = *(cp_in+1)/0x10 - *(cp_in+2)/0x10;
		value2str_oct(c_octdiff,cs_poroct);
		sprintf(cs_pormml,"{%s%s%s}",
			cps_note[*(cp_in+1)&0x0F],
			cs_poroct,
			cps_note[*(cp_in+2)&0x0F]);

		i_ret = tracklen( D_OP_TLLEVEL , 0 );
		i_curcode = (int)(*cp_in)&(int)0x000000FF;
		if( i_ret == 0 ){
			l_totalclock = tracklen( D_OP_TLTOTAL , 0 );
			sprintf(cs_outbuf," %02X    |%3d|%5d|%7s %3d\n",
				i_curcode,
				(unsigned char)*(cp_in+3),
				l_totalclock,
				cs_pormml,
				(unsigned char)*(cp_in+3) );
		}else{
			sprintf(cs_outbuf," %02X    |%3d|     |%7s %3d\n",
				i_curcode,
				(unsigned char)*(cp_in+3),
				cs_pormml,
				(unsigned char)*(cp_in+3) );
		}
	}else{
		/* code */
		i_curcode = (int)tp_cur->code1&(int)0x000000FF;
		cp_curstr = tp_cur->str;
		if( *cp_in == (char)0xF9 ){
			/* 始点 */
		}else if( *cp_in == (char)0xF8 ){
			/* 終点 */
			uc_ret = (unsigned char)(*(cp_in+1));
		}else if( *cp_in == (char)0xF6 ){
			/* L */
		}else if( *cp_in == (char)0xF7 ){
			/* : */
		}
		if( (char)*cp_in == (char)0xF8 || (char)*cp_in == (char)0xF9 ){
			i_ret = tracklen( D_OP_TLLEVEL , 0 );
			if( i_ret == 0 && (char)*cp_in == (char)0xF8 ){
				l_totalclock = tracklen( D_OP_TLTOTAL , 0 );
				sprintf(cs_outbuf," %02X    |   |%5d|code %s%d\n",
					i_curcode,
					l_totalclock,
					cp_curstr,
					(unsigned char)*(cp_in+1));
			}else if( i_ret == 1 && (char)*cp_in == (char)0xF9){
				l_totalclock = tracklen( D_OP_TLTOTAL , 0 );
				sprintf(cs_outbuf," %02X    |   |%5d|code %s\n",
					i_curcode,
					l_totalclock,
					cp_curstr);
			}else if( (char)*cp_in == (char)0xF8 ){
				sprintf(cs_outbuf," %02X    |   |     |code %s%d\n",
					i_curcode,
					cp_curstr,
					(unsigned char)*(cp_in+1));
			}else if( (char)*cp_in == (char)0xF9 ){
				sprintf(cs_outbuf," %02X    |   |     |code %s\n",
					i_curcode,
					cp_curstr);
			}
		}else{
			switch(tp_cur->format){
				case 0:
				case 1:
					sprintf(cs_outbuf," %02X    |   |     |code %s\n",
						i_curcode,
						cp_curstr);
					break;
				case 3:
					sprintf(cs_outbuf," %02X    |   |     |code %s\n",
						i_curcode,
						cp_curstr);
					break;
				case 4:
				case 7:
					sprintf(cs_outbuf," %02X    |   |     |code %s%d\n",
						i_curcode,
						cp_curstr,
						(char)*(cp_in+1));
					break;
				case 5:
					sprintf(cs_outbuf," %02X    |   |     |code %s%d\n",
						i_curcode,
						cp_curstr,
						(unsigned char)*(cp_in+1));
					break;
				case 8:
					sprintf(cs_outbuf," %02X    |   |     |code %s%.d\n",
						i_curcode,
						cp_curstr,
						(unsigned char)*(cp_in+1));
					break;
				case 6:
					cp_ret = editprint( tp_cur->outformat , cp_in );
					sprintf(cs_outbuf," %02X    |   |     |code %s%s\n",
						i_curcode,
						cp_curstr,
						cp_ret );
					break;
				default:
					sprintf(cs_outbuf," %02X    |   |     |\n",i_curcode);
					break;
			}
		}
	}

	sprintf(gcs_line,cs_outbuf);
	hfpf( DH_BSDUMP , gcs_line );

	return;
}
