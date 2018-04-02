char scs_APVersion[]  ="1.21a";
int  si_APVersion     = 1211;
char scs_IFVersion[]  ="1.21";
int  si_IFVersion     = 1210;
char scs_f2pIFVersion[]  ="1.00";
int  si_f2pIFVersion     = 1000;

char JSign[]="@(#)JPR V.1.21a by JUD(T.Terata) at 2008/ 7/14";
char systemvarsion[]="1.21c"; /* 埋め込みバージョン情報 */
char systemdate[]="2008/ 7/14"; /* 埋め込み日付 */

/**********************************************************************/
/* PMDRCC - P.M.D. reversal compiler at CUI by JUD(T.Terata)          */
/**********************************************************************/
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>

#include "../common/pmdrcif.h"
#include "../common/f2pcvif.h"

char *anarg( int , char ** );
void setargvalue( char * );
void startdialog( );
void outmml( T_OPIF * );
void initinit( );
char *f2pconvert( char * , int );
char *v2pconvert( char * , int );
int readdatafile( char * , char * );

T_OPIF t_opif;
T_F2PIF t_f2pif;
int gi_dodialog=DA_OFF;
char *gcp_outfilename=NULL;

/* 最大PMDデータサイズは16kBytesくらいだが */
#define DN_MAX_OBJSIZE 65536

#define D_FTYPE_ERR -1
#define D_FTYPE_UK 0
#define D_FTYPE_PMD 1
#define D_FTYPE_OPI 2
#define D_FTYPE_OVI 3
#define D_FTYPE_OZI 4
#define D_FTYPE_PMF 5

#define DN_MAX_EXTNAME 8

/*--------------------------------------------------------------------*/
/* EXECUTE-MAIN */
int main(argc,argv)
	int argc;
	char **argv;
{
	char *cp_ret;
	char cs_inname[1024];
	FILE *fp_in;
	char cs_readbuf[DN_MAX_OBJSIZE];
	int i_ret;
	int i_datasize;
	T_OPIF *t1p_ret;
	HINSTANCE x_dllhandle;
	typedef T_OPIF *(CALLBACK* LPFNDLLFUNC1)(T_OPIF *);
	LPFNDLLFUNC1 x_funchandle;
	LPFNDLLFUNC1 x_funcfmhandle;
	DWORD dw_ret=0;
	LPTSTR lpMsgBuf;
	char *cp_data;

	/* 引数解析 */
	cp_ret = anarg(argc,argv);

	if( cp_ret == NULL ){
		fprintf(stderr,"\nPMDRCC Ver.%s P.M.D. Reversal Compiler for Win32 by JUD at %s\n\n",systemvarsion,systemdate);
		fprintf(stderr,"USAGE : pmdrc.exe [/options] input-filename\n\n");
		fprintf(stderr,"    /nobarline : 小節区切りしない (Default=可能ならする)\n");
		fprintf(stderr,"    /dobarline : 小節区切り必ずする (Default=可能ならする)\n");
		fprintf(stderr,"    /novdump   : ＦＭ音色を出力しない (Default=する)\n");
		fprintf(stderr,"    /dotx81z   : ＴＸ８１Ｚ向けＦＭ音色出力をする\n");
		fprintf(stderr,"    /L<value>  : 出力レベル (Default=3)\n");
		fprintf(stderr,"                  出力しない : 5 <- 4 -- 3 -> 2 : 出力する\n");
		fprintf(stderr,"    /dodump    : トラックダンプモード (出力レベル=5に固定)\n");
		fprintf(stderr,"    /dosdump   : スーパートラックダンプモード (出力レベル=5に固定)\n");
		fprintf(stderr,"    /dodialog  : 対話モード (/Z,/l,/b 相当)\n");
		fprintf(stderr,"    /noautolen : lコマンド 長さ自動判別しない (Default=する)\n");
		fprintf(stderr,"    /l<value>  : lコマンド相当 暗黙の音長 (符点不可/Default=16 0:使用しない)\n");
		fprintf(stderr,"    /b<value>  : 最短音符表現長 音符表現指定(符点不可/Default=16/32)\n");
		fprintf(stderr,"    /Z<value>  : #Zenlen <value> 相当 (48/96 Default=192)\n");
		fprintf(stderr,"    /B<value>  : 小節線起点の指定 (Default/0:Auto 1:[L] 2:データ先頭)\n");
		fprintf(stderr,"    /F<value>  : 小節線起点のオフセット指定 (Default=0)\n");
		fprintf(stderr,"    /@<file>   : F2PCV.DLL一時ファイル出力先\n");
		fprintf(stderr,"    /.<file>   : MMLファイル出力先 (default=標準出力)\n");
		exit(0);
	}

	/* ファイル名 */
	strcpy(cs_inname,cp_ret);
	fprintf(stderr," [JPR] Input file name   : %s\n",cs_inname);

	memset( cs_readbuf , 0x00 , DN_MAX_OBJSIZE );
	i_ret = readdatafile( cs_inname , cs_readbuf );
	switch( i_ret ){
		case D_FTYPE_PMD:
			fprintf(stderr," [JPR] File type is PMD.\n");
			cp_data = cs_readbuf;
			break;
		case D_FTYPE_OPI:
		case D_FTYPE_OVI:
		case D_FTYPE_OZI:
			/* FMP->PMD変換 */
			fprintf(stderr," [JPR] File type is FMP.\n");
			cp_data = f2pconvert( cs_readbuf , i_ret );
			break;
		case D_FTYPE_PMF:
			/* VEDデータファイル */
			fprintf(stderr," [JPR] File type is VED.\n");
			cp_data = v2pconvert( cs_readbuf , i_ret );
			break;
		case D_FTYPE_UK:
		default:
			fprintf(stderr,"E[JPR] Unknown file type.\n");
			return( -1 );
	}
	t_opif.cp_indata = cp_data;

	/* 対話モード */
	if( gi_dodialog == DA_ON ){
		/* 対話モードでは、/Z /l /b について質疑応答する */
		startdialog();
	}

	/* DLLリンク */
	x_dllhandle = LoadLibrary( DS_DLLFILENAME );
	if( x_dllhandle == NULL ){
		/* DLLリンクできず */
		fprintf(stderr,"E[JPR] Could not load %s.\n",DS_DLLFILENAME);
		return( -1 );
	}

	/* 関数アドレス取得 */
	x_funchandle = (LPFNDLLFUNC1)GetProcAddress( x_dllhandle , DS_DLLFUNCNAME );
	if( x_funchandle == NULL )
	{
		/* 関数リンクできず */
		fprintf(stderr,"E[JPR] Could not load %s/%s(). [%d]\n",
			DS_DLLFILENAME,DS_DLLFUNCNAME,dw_ret);
		FreeLibrary( x_dllhandle );
		return( -2 );
	}
	x_funcfmhandle = (LPFNDLLFUNC1)GetProcAddress( x_dllhandle , DS_DLLFMFUNCNAME );
	if( x_funcfmhandle == NULL )
	{
		/* 関数リンクできず */
		fprintf(stderr,"E[JPR] Could not load %s/%s(). [%d]\n",
			DS_DLLFILENAME,DS_DLLFMFUNCNAME,dw_ret);
		FreeLibrary( x_dllhandle );
		return( -3 );
	}

	fprintf(stderr," [JPR] Start analysis by DLL.\n");

	/* 関数[pmdrc.dll/pmdrc_dllmain()]実行 */
	t1p_ret = x_funchandle( &t_opif );

	fprintf(stderr," [JPR] End analysis by DLL.\n");

	/* 結果解析 */
	if( t1p_ret == NULL ){
		/* 実行エラー */
		if( t1p_ret->i_result > 0 ){
			/* warning レベルなのでとりあえず表示はする */
			fprintf(stderr,"W[JPR] MML incomplete.\n");
			fprintf(stdout," ; Incomplete reverse-compiling.\n");
			outmml( t1p_ret );
		}else{
			/* 完全にエラーなので表示しない */
			fprintf(stderr,"E[JPR] Error [%d].\n",t1p_ret->i_result);
			FreeLibrary( x_dllhandle );
			exit( t1p_ret->i_result );
		}
	}else{
		/* 結果表示 */
		outmml( t1p_ret );
	}

	fprintf(stderr," [JPR] Succeseed output MML.\n");

	/* このままプロセスの終了なのでメモリ解放の必要はないのだが */
	/* サンプルも兼ねているので 一応メモリ解放をやっておく */
	t1p_ret = x_funcfmhandle( t1p_ret );

	FreeLibrary( x_dllhandle );
	exit(0);
}
/*--------------------------------------------------------------------*/
/* 初期処理 */
/* VED→PMD変換ルーチンのコール */
char *v2pconvert( char *cp_in , int i_type ){

#define DN_SIZE_DUMMYPMDFILE_VED 8192
#define DN_SIZE_DUMMYPMDHEAD_VED 42
#define DN_SIZE_DUMMYPMDFOOT1_VED 7
#define DN_SIZE_DUMMYPMDFOOT3_VED 2
#define DN_SIZE_UNIT_VEDFMDATA 32
#define DN_SIZE_UNIT_VEDFMBODY 25

typedef	struct	_t_ff2{
	short addrpps;		/* PPSアドレス */
	short addrppc;		/* PPCアドレス */
	short addrtitle;	/* Titleアドレス */
	short addrcomp;		/* Composerアドレス */
	short addrarr;		/* Arrengerアドレス */
}T_FF2;
static char cs_pconv[DN_SIZE_DUMMYPMDFILE_VED];
	char cs_dummyhead[DN_SIZE_DUMMYPMDHEAD_VED]={
		0x00, 0x1A, 0x00, 0x1B, 0x00, 0x1C, 0x00, 0x1D,
		0x00, 0x1E, 0x00, 0x1F, 0x00, 0x20, 0x00, 0x21,
		0x00, 0x22, 0x00, 0x23, 0x00, 0x24, 0x00, 0x25,
		0x00, 0x29, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80,
		0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x30, 0x1A,
		0x47, 0xFE };
	char cs_dummyfoot1[DN_SIZE_DUMMYPMDFOOT1_VED]={
		0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00 };
	T_FF2 t_dummyfoot2={
		0, 0, 0, 0, 0};
	char cs_dummyfoot3[DN_SIZE_DUMMYPMDFOOT3_VED]={
		0x00, 0x00 };

	char *cp_cur1;
	char *cp_cur2;
	unsigned char *ucp_cur1;
	unsigned int ui_number;
FILE *fp_io;

	memset( cs_pconv , 0x00 , DN_SIZE_DUMMYPMDFILE_VED );
	memcpy( cs_pconv , cs_dummyhead , DN_SIZE_DUMMYPMDHEAD_VED );
	cp_cur1 = cs_pconv + DN_SIZE_DUMMYPMDHEAD_VED;
	cp_cur2 = cp_in;

	for( ui_number=0 ; ui_number<256 ; ui_number++ ){
		ucp_cur1 = cp_cur1;
		*ucp_cur1 = (unsigned char)ui_number;
		cp_cur1++;
		memcpy( cp_cur1 , cp_cur2 , DN_SIZE_UNIT_VEDFMBODY );
		cp_cur1 += DN_SIZE_UNIT_VEDFMBODY;
		cp_cur2 += DN_SIZE_UNIT_VEDFMDATA;
	}

	memcpy( cp_cur1 , cs_dummyfoot1 , DN_SIZE_DUMMYPMDFOOT1_VED );

	t_dummyfoot2.addrpps = (short)((cp_cur1-cs_pconv) + 2 - 1 + 1);
	t_dummyfoot2.addrppc = t_dummyfoot2.addrpps + 1;
	t_dummyfoot2.addrtitle = t_dummyfoot2.addrppc + 1;
	t_dummyfoot2.addrcomp = t_dummyfoot2.addrtitle + 1;
	t_dummyfoot2.addrarr = t_dummyfoot2.addrcomp + 1;
	cp_cur1 += DN_SIZE_DUMMYPMDFOOT1_VED;
	memcpy( cp_cur1 , &t_dummyfoot2 , sizeof(t_dummyfoot2) );

	cp_cur1 += sizeof(t_dummyfoot2);
	memcpy( cp_cur1 , cs_dummyfoot3 , DN_SIZE_DUMMYPMDFOOT3_VED );

	return( cs_pconv );
}
/*--------------------------------------------------------------------*/
/* 初期処理 */
/* F2PCV.DLLのコール */
char *f2pconvert( char *cp_in , int i_type ){

	HINSTANCE x_dllhandle;
	typedef T_F2PIF *(CALLBACK* LPFNDLLFUNC1)(T_F2PIF *);
	LPFNDLLFUNC1 x_funchandle;
	LPFNDLLFUNC1 x_funcfmhandle;
	DWORD dw_ret=0;
	LPTSTR lpMsgBuf;
	char *cp_ret;
	char cs_debug[1024];

	T_F2PIF *t1p_ret;

	/* DLLリンク */
	x_dllhandle = LoadLibrary( DS_F2PFILENAME );
	if( x_dllhandle == NULL ){
		/* DLLリンクできず */
		fprintf(stderr,"E[JPR] Could not load %s.\n",DS_F2PFILENAME);
		return( NULL );
	}
	/* 関数アドレス取得 */
	x_funchandle = (LPFNDLLFUNC1)GetProcAddress( x_dllhandle , DS_F2PFUNCNAME );
	if( x_funchandle == NULL )
	{
		/* 関数リンクできず */
		fprintf(stderr,"E[JPR] Could not load %s/%s(). [%d]\n",
			DS_F2PFILENAME,DS_F2PFUNCNAME,dw_ret);
		FreeLibrary( x_dllhandle );
		return( NULL );
	}
	x_funcfmhandle = (LPFNDLLFUNC1)GetProcAddress( x_dllhandle , DS_F2PFMFUNCNAME );
	if( x_funcfmhandle == NULL )
	{
		/* 関数リンクできず */
		fprintf(stderr,"E[JPR] Could not load %s/%s(). [%d]\n",
			DS_F2PFILENAME,DS_F2PFMFUNCNAME,dw_ret);
		FreeLibrary( x_dllhandle );
		return( NULL );
	}

	t_f2pif.i_iv = si_f2pIFVersion;
	t_f2pif.i_directmes = DA_ON;
	t_f2pif.i_datatype = i_type;
	t_f2pif.cp_indata = cp_in;

	/* FMP→PMD変換 */
	fprintf(stderr," [JPR] Start convert FMP->PMD.\n");
	t1p_ret = x_funchandle( &t_f2pif );

	/* 結果解析 */
	if( t1p_ret == NULL ){
		/* 実行エラー */
		if( t1p_ret->i_result > 0 ){
			/* warning レベルなのでとりあえず続行はする */
			fprintf(stderr,"W[JPR] convert FMP->PMD incompleteness.\n");
		}else{
			/* 完全にエラーなので表示しない */
			fprintf(stderr,"E[JPR] Convert error [%d].\n",t1p_ret->i_result);
			FreeLibrary( x_dllhandle );
			exit(-1);
		}
	}

	fprintf(stderr," [JPR] Succeseed convert FMP->PMD.\n");

	cp_ret = malloc( t_f2pif.i_datalen + 1 );
	memcpy( cp_ret , t_f2pif.cp_outdata , t_f2pif.i_datalen );

	t1p_ret = x_funcfmhandle( t1p_ret );
	FreeLibrary( x_dllhandle );

	return( cp_ret );
}
/*--------------------------------------------------------------------*/
/* 初期処理 */
/* ファイルの読み取り 拡張子が存在しない場合は自動的に付ける */
/* 優先順位 MZ M2 M OZI OVI OZI */
/* 返却値：-1:ERROR 0-:取得したファイルの種類 */
int readdatafile( char *cp_name , char *cp_outarea ){

	char cs_fname[1024];
	char cs_addfname[1024];
	char *cp_rcur;
	int i_type=D_FTYPE_UK;
	int i_count;
	int i_ret;
	FILE *fp_in;
	char css_extname[DN_MAX_EXTNAME][8]={
		".MZ", ".M2", ".M", ".OPI", ".OVI", ".OZI", ".FF", ""
	};
	int is_exttype[DN_MAX_EXTNAME]={
		D_FTYPE_PMD,D_FTYPE_PMD,D_FTYPE_PMD,
		D_FTYPE_OPI,D_FTYPE_OVI,D_FTYPE_OZI,
		D_FTYPE_PMF,D_FTYPE_UK };

	strcpy( cs_fname , cp_name );

	/* 比較のため、全文字をキャピタルにしておく */
	for(cp_rcur=cs_fname;(*cp_rcur)!=0x00;cp_rcur++){
		if( (char)'a' <= *cp_rcur && *cp_rcur <= (char)'z' ){
			*cp_rcur = *cp_rcur - 0x20;
		}
	}

	/* 拡張子の検査 */
	cp_rcur = strrchr( cs_fname , '.' );
	if( cp_rcur != NULL ){
		/* 拡張子が見つかった */
		if( strcmp( cp_rcur , ".M" ) == 0 ){
			i_type = D_FTYPE_PMD;
		}else if( strcmp( cp_rcur , ".M2" ) == 0 ){
			i_type = D_FTYPE_PMD;
		}else if( strcmp( cp_rcur , ".MZ" ) == 0 ){
			i_type = D_FTYPE_PMD;
		}else if( strcmp( cp_rcur , ".OPI" ) == 0 ){
			i_type = D_FTYPE_OPI;
		}else if( strcmp( cp_rcur , ".OVI" ) == 0 ){
			i_type = D_FTYPE_OVI;
		}else if( strcmp( cp_rcur , ".OZI" ) == 0 ){
			i_type = D_FTYPE_OZI;
		}else if( strcmp( cp_rcur , ".FF" ) == 0 ){
			i_type = D_FTYPE_PMF;
		}else {
			/* unknown拡張子→ＰＭＤ形式として扱う */
			i_type = D_FTYPE_PMD;
		}
	}else{
		/* 拡張子が見つからない */
		i_type = D_FTYPE_UK;
	}

	/* ファイルを開く */
	if( i_type == D_FTYPE_UK ){
		/* ファイルタイプが拡張子から特定できないので順に試す */
		for(i_count=0;i_count<DN_MAX_EXTNAME;i_count++){
			/* 拡張子を変えながら次々と試す */
			strcpy( cs_addfname , cs_fname );
			strcat( cs_addfname , css_extname[i_count] );
			fp_in = fopen( cs_addfname , "rb" );
			if( fp_in != NULL ){
				/* ファイル発見 */
				i_type = is_exttype[i_count];
				break;
			}
		}
	}else{
		/* 確定している拡張子で開く */
		fp_in = fopen( cs_fname , "rb" );
	}

	if( fp_in == NULL ){
		/* ファイルが見つからなかった場合 */
		fprintf(stderr,"E[JPR] Error input file not found. [%s]\n",cp_name);
		return( -1 );
	}

	i_ret = fread(cp_outarea,1,DN_MAX_OBJSIZE,fp_in);
	if( i_ret >= DN_MAX_OBJSIZE ){
		fprintf(stderr,"E[JPR] File too big.\n");
		exit(-1);
	}else if( i_ret < 0 ){
		fprintf(stderr,"E[JPR] File read error.\n");
		exit(-1);
	}
	fclose( fp_in );

	/* サイズ等のセット */
	t_opif.i_datasize = i_ret;

	fprintf(stderr," [JPR] Succeed read file.\n");

	return( i_type );
}
/*--------------------------------------------------------------------*/
/* 出力 */
void outmml( T_OPIF *tp_out ){

	int i_count;
	FILE *fp_out;

	if( gcp_outfilename == NULL ){
		/* 出力ファイル指定 なし */
		fp_out = stdout;
	}else{
		fp_out = fopen( gcp_outfilename , "w" );
		if( fp_out == NULL ){
			/* ファイルオープン失敗時は標準出力に書き出す */
			fp_out = stdout;
			fprintf(stderr," [JPR] Failed open output-file, output to stdout.\n");
		}else{
			fprintf(stderr," [JPR] Output file name  : %s\n",gcp_outfilename);
		}

	}

	/* 出力バッファをチェック */
	/* i_count=1で始まるのは、cps_out[0]（制御メッセージ）が */
	/* すでにリアルタイムで出力されているため */
	for(i_count=1;i_count<DN_MAX_OUTBUFFER;i_count++){
		if( tp_out->cps_out[i_count] != NULL ){
			/* 出力文字列あり */
			fprintf(fp_out,"%s",tp_out->cps_out[i_count]);
		}
	}

	/* ファイルのクローズは標準出力以外に出力したときだけ */
	if( fp_out != stdout ){
		fclose( fp_out );
	}

	return;
}
/*--------------------------------------------------------------------*/
/* 初期処理のデフォルト値セット */
void initinit( ){

	/* PMDRCインターフェース設定 */
	t_opif.i_iv = si_IFVersion;
	t_opif.i_av = si_APVersion;
	t_opif.i_at = DO_APTYPE_UK;
	t_opif.i_mmllevel = DN_OL_NORMAL;
	t_opif.i_dumpmode = DN_DL_NODUMP;
	t_opif.i_voicemode = DA_ON;
	t_opif.i_definemode = DA_ALL;
	t_opif.i_zenlen = 96;
	t_opif.i_lendef = t_opif.i_zenlen / 8;
	t_opif.i_lenmin = t_opif.i_lendef / 2;
	t_opif.i_len3min = (t_opif.i_lenmin/3)*4;
	t_opif.i_barlevel = DA_AUTO;
	t_opif.i_manoffset = 0;
	t_opif.i_directmes = DA_ON;
	t_opif.cp_indata = NULL;
	t_opif.i_autolendef = DA_ON;

	/* F2PCVインターフェース設定 */
	t_f2pif.i_iv = si_f2pIFVersion;
	t_f2pif.i_directmes = DA_ON;
	t_f2pif.i_datatype = 0;
	t_f2pif.cp_indata = NULL;
	t_f2pif.cp_debugdata = NULL;

	return;
}
/*--------------------------------------------------------------------*/
/* 初期処理 */
/* 引数解析メイン 返却値：ファイル名 */
/* 最初に登場する、'/'or'-'以外で始まる引数をファイル名と認識し打切り */
char *anarg( int argc , char **argv ){

	int i_argcnt;
	char *cp_curarg;

	/* 初期化前の初期化（デフォ値のセット） */
	initinit( );

	if( argc < 2 ){
		/* 引数なし */
		return(NULL);
	}

	for(i_argcnt=1;i_argcnt<argc;i_argcnt++){
		cp_curarg = argv[i_argcnt];
		if( *cp_curarg == (char)'-' || *cp_curarg == (char)'/' ){
			/* コマンドライン引数 */
			setargvalue( cp_curarg+1 );
		}else{
			/* ファイル名発見 */
			return( cp_curarg );
		}
	}

	return(NULL);
}
/*--------------------------------------------------------------------*/
/* 初期処理 */
/* ダイアログモード */
void startdialog( ){

	char cs_input[256];
	char *cp_ret;
	int i_value;

	fprintf(stderr,"       Z(zenlen           now=%3d) :",t_opif.i_zenlen);
	cp_ret = gets( cs_input );
	if( strlen(cp_ret) != 0 ){
		i_value = atoi( cs_input );
		switch( i_value ){
			case 48:
			case 96:
			case 192:
				t_opif.i_zenlen = i_value;
				/* デフォルトlコマンドの値を、zenlenに関わりなく16にする */
				t_opif.i_lendef = t_opif.i_zenlen / 16;
				break;
			default:
				fprintf( stderr , "E[JPR] Wrong length %d, you can specifies 48, 96, and 192.\n", i_value );
				exit(0);
				break;
		}
	}
	fprintf(stderr,"       l(default length / now=%3d) :",
		t_opif.i_zenlen/t_opif.i_lendef);
	cp_ret = gets( cs_input );
	if( strlen(cp_ret) != 0 ){
		i_value = atoi( cs_input );
		if( t_opif.i_zenlen%i_value == 0 ){
			t_opif.i_lendef = t_opif.i_zenlen/i_value;
		}else{
			fprintf( stderr , "E[JPR] %d cannot divide %d.\n",
				t_opif.i_zenlen , i_value );
			exit(0);
		}
	}
	fprintf(stderr,"       b(minimum length / now=%3d) :",
		t_opif.i_zenlen/t_opif.i_lenmin);
	cp_ret = gets( cs_input );
	if( strlen(cp_ret) != 0 ){
		i_value = atoi( cs_input );
		if( t_opif.i_zenlen%i_value == 0 ){
			if( t_opif.i_zenlen/i_value <= t_opif.i_lendef ){
				t_opif.i_lenmin = t_opif.i_zenlen/i_value;
			}else{
				fprintf( stderr , "E[JPR] Too big so.\n");
				exit(0);
			}
		}else{
			fprintf( stderr , "E[JPR] %d cannot divide %d.\n",
				t_opif.i_zenlen , i_value );
			exit(0);
		}
	}

	return;
}
/*--------------------------------------------------------------------*/
/* 初期処理 */
/* 引数解析詳細 */
void setargvalue( char *cp_in ){

	int i_value;

	switch( *cp_in ){
		case 'Z':
			/* #Zenlen default=192 */
			i_value = atoi( cp_in+1 );
			switch( i_value ){
				/* ムチャクチャな値は却下する */
				case 48:
				case 96:
				case 192:
				case 64:
				case 128:
				case 84:
				case 168:
				/* とかいっておきながら14/8拍子対策(笑) */
					t_opif.i_zenlen = i_value;
					/* デフォルトlコマンドの値を、zenlenに関わりなく16にする */
					t_opif.i_lendef = t_opif.i_zenlen / 16;
					return;
			}
			break;
		case 'l':
			/* lコマンド(暗黙音長) 音符長指定 default=16(%12) */
			i_value = atoi( cp_in+1 );
			if( i_value == 0 ){
				/* ゼロが指定された場合は標準音長を使用しない (1.00c) */
				fprintf( stderr , " [JPR] Forbidden use l command.\n" );
				t_opif.i_lendef = 0;
				return;
			}else if( t_opif.i_zenlen%i_value == 0 ){
				t_opif.i_lendef = t_opif.i_zenlen/i_value;
				return;
			}
			break;
		case 'b':
			/* 音符で示す最小長さ 音符長指定 default=32(%6) */
			i_value = atoi( cp_in+1 );
			if( t_opif.i_zenlen%i_value == 0 ){
				t_opif.i_lenmin = t_opif.i_zenlen/i_value;
				t_opif.i_len3min = (t_opif.i_lenmin/3)*2;
				return;
			}
			break;
		case 'B':
			/* 小節線の起点 def/0:auto 1:[L] 2:top */
			t_opif.i_autothresh = atoi( cp_in+1 );
			fprintf( stderr , " [JPR] Bar-line origin is top of data.\n" );
			return;
		case 'F':
			/* 小節線の起点オフセット */
			t_opif.i_manoffset = atoi( cp_in+1 );
			if( t_opif.i_manoffset > t_opif.i_zenlen ){
				/* 小節長より長い場合、商余をとる */
				t_opif.i_manoffset = t_opif.i_manoffset % t_opif.i_zenlen;
			}
			fprintf( stderr , " [JPR] Bar-line offset is [%d].\n" ,
				t_opif.i_manoffset);
			return;
		case '@':
			/* ＦＭＰデータ読み込み時のテンポラリＰＭＤデータを */
			/* ファイルに出力する場合 */
			t_f2pif.cp_debugdata = (cp_in+1);
			return;
		case '.':
			/* 出力ＭＭＬファイル名指定 */
			gcp_outfilename = (cp_in+1);
			return;
		case 'L':
			/* 出力レベル */
			i_value = atoi( cp_in+1 );
			if( i_value >= 2  && i_value <= 5 ){
				t_opif.i_mmllevel = i_value;
				fprintf( stderr , " [JPR] OUTPUT-Level is %d.\n" , i_value );
				if( i_value >= 4 ){
					/* Level4のときは'@'がないので音色定義も不要 */
					t_opif.i_voicemode = DA_OFF;
					/* Level4のときはdefineもパート増設のみ */
					t_opif.i_definemode = DA_DEFL_PARTEXTEND;
				}
				return;
			}else{
				fprintf( stderr , "E[JPR] OUTPUT-Level is %d." , i_value );
				exit(1);
			}
			break;
		case 'n':
			/* "no******" の一団 */
			if( 0 ==
				strncmp(cp_in,"nobarline",strlen("nobarline")) ){
				t_opif.i_barlevel = DA_OFF;
				fprintf( stderr , " [JPR] Forbidden insert disregard-bar.\n" );
				return;
			}
			if( 0 ==
				strncmp(cp_in,"noautolen",strlen("noautolen")) ){
				t_opif.i_autolendef = DA_OFF;
				fprintf( stderr , " [JPR] Forbidden automatic l command.\n" );
				return;
			}
			if( 0 ==
				strncmp(cp_in,"novdump",strlen("novdump")) ){
				t_opif.i_voicemode = DA_OFF;
				fprintf( stderr , " [JPR] Voice-mode is DISABLE.\n" );
				return;
			}
			break;
		case 'd':
			/* "do******" の一団 */
			if( 0 ==
				strncmp(cp_in,"dobarline",strlen("dobarline")) ){
				t_opif.i_barlevel = DA_FORCED;
				fprintf( stderr , " [JPR] Compulsion insert disregard-bar.\n" );
				return;
			}
			if( 0 ==
				strncmp(cp_in,"dodump",strlen("dodump")) ){
				t_opif.i_dumpmode = DN_DL_FRAMEDUMP;
				t_opif.i_mmllevel = 5;
				fprintf( stderr , " [JPR] Running-mode is TRACKDUMP.\n" );
				return;
			}
			if( 0 ==
				strncmp(cp_in,"dosdump",strlen("dosdump")) ){
				t_opif.i_dumpmode = DN_DL_SUPERDUMP;
				t_opif.i_mmllevel = 5;
				fprintf( stderr , " [JPR] Running-mode is SUPERTRACKDUMP.\n" );
				return;
			}
			if( 0 ==
				strncmp(cp_in,"dodebug",strlen("dodebug")) ){
				t_opif.i_dumpmode = DN_DL_SUPERDUMP;
				t_opif.i_mmllevel = 5;
				t_opif.i_directmes = t_opif.i_directmes | DA_DEBUG;
				fprintf( stderr , " [JPR] Running-mode is DEBUGMODE.\n" );
				return;
			}
			if( 0 ==
				strncmp(cp_in,"dodialog",strlen("dodialog")) ){
				gi_dodialog = DA_ON;
				fprintf( stderr , " [JPR] Running-mode is DIALOG.\n" );
				return;
			}
			if( 0 ==
				strncmp(cp_in,"dotx81z",strlen("dotx81z")) ){
				t_opif.i_voicemode = DA_TX81ZVOICE;
				fprintf( stderr , " [JPR] Voice-mode is \"TX81Z\".\n" );
				return;
			}
	/* V1.21 2008/7/14 start */
			if( 0 ==
				strncmp(cp_in,"doautotrans",strlen("doautotrans")) ){
				t_opif.c_autotrans = D_ON;
				fprintf( stderr , " [JPR] enable, auto-Transpose mode.\n" );
				return;
			}
			break;
	/* V1.21 2008/7/14 end */
		default:
			/* 知らない引数 */
			fprintf( stderr , "W[JPR] Arg. unknown. [/%s]\n\n" , cp_in );
			return;
	}


	fprintf( stderr , "E[JPR] Arg. invalid value. [/%s]\n\n" , cp_in );

	exit(1);
}
