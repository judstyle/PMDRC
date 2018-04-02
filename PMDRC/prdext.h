/**********************************************************************/
/* PMDRC - P.M.D. reversal compiler by JUD(T.Terata)                  */
/* external symbols                                                   */
/**********************************************************************/
#ifndef JPREXT_H

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

/* DLL化改造部分 */
extern T_OPIF *gtp_gop;
/* 構造体に吸収しないポリシー：型が違う、頻繁に使うものは残す */
extern char gcs_line[DN_SIZE_LINEBUFFER]; /* hfpf()用バッファ */

/* デバッグ用 */
extern void *vp_debug;
extern void *vp_debug2;

/* 092f3追加部分 2004/3/10～ */
extern char gc_lastoutcat;
extern char gc_lastoutchar;

/* 埋め込み */
extern char JSign[];
extern char systemvarsion[];
extern char systemdate[];

/* 関数プロトタイプ */
/*extern int main(argc,argv);*/
extern void trackout( int );
extern void outcharcode( T_BOC * );
extern void outcharnote( T_BON * , int );
extern void value2str_oct( char , char * );
extern void editoct( );
extern void readheader( char * );
extern void readhead2( char * );
extern void part2master( );
extern int delsilenttrack( );
extern int combinenotes( int );
extern int sortmaster( );
extern void dividebybar( int );
extern void anltrack( );
extern int expset( char *cp_findout );
extern int findnote( char *cp_cur,T_BON *tp_boncur,long l_clock );
extern int findcontrol( char *cp_cur , T_BOC *tp_boccur , long l_clock );
extern T_TCODE *codefinderbystr( char *cp_code );
extern T_BON *newbon( T_BON *tp_in );
extern T_BOC *newboc( T_BOC *tp_in );
extern T_BOM *newbom( T_BOM *tp_in );
extern int powxy(int x,int y);
extern char *anarg( int argc , char **argv );
extern void setargvalue( char *cp_in );
extern long tracklen( int , long );
extern int analclocklist( int );
extern void wastebon( T_BON * );
extern T_BON *reusebon( );
extern T_BON *get1bon( );
extern void dump1op( char * );
extern void dumps1op( char * );
extern void startdialog( );
extern void voiceout( );
extern void defineout( );
extern char *editprint( char * , char * );
extern char *hfpf( int , char * );
extern void *apmalloc( int i_size );
extern T_OPIF *anlmain( T_OPIF * );
extern void delbon( T_BON * );
extern void insbon( T_BON * , T_BON * );

/* 各種設定 */
extern int gi_zenlen;
/* 現在処理中のトラックに関する情報 */
extern T_TRACKTABLE *tp_curtra;
extern T_BOM *tp_BOMtop;
extern int gi_bomnum;
extern int gi_llfindout;
extern int gi_tlinit;
extern char *cp_voiceaddr;
/* 内部処理に関する情報 */
extern int gi_stockbons;
extern T_BON *gtps_stockbon[];
extern T_GINFO gt_ginfo;
/* コード→ＭＭＬ 対応テーブル */
extern T_TCODE	ts_tc[DN_MAX_CONTROLCODE];
/* コードの音程を表す４ビット→ＭＭＬ変換テーブル */
extern char *cps_note[16];
extern int iss_notelenfreq[DN_MAX_TRACKNUM][DN_MAX_NOTELENFREQ+1];

extern T_TRACKTABLE ts_ts[DN_MAX_TRACKNUM];

#endif
#define JPREXT_H

