char scs_APVersion[]  ="1.21a";
int  si_APVersion     = 1211;
char scs_IFVersion[]  ="1.21";
int  si_IFVersion     = 1210;
char scs_f2pIFVersion[]  ="1.00";
int  si_f2pIFVersion     = 1000;

char JSign[]="@(#)JPR V.1.21a by JUD(T.Terata) at 2008/ 7/14";
char systemvarsion[]="1.21c"; /* ���ߍ��݃o�[�W������� */
char systemdate[]="2008/ 7/14"; /* ���ߍ��ݓ��t */

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

/* �ő�PMD�f�[�^�T�C�Y��16kBytes���炢���� */
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

	/* ������� */
	cp_ret = anarg(argc,argv);

	if( cp_ret == NULL ){
		fprintf(stderr,"\nPMDRCC Ver.%s P.M.D. Reversal Compiler for Win32 by JUD at %s\n\n",systemvarsion,systemdate);
		fprintf(stderr,"USAGE : pmdrc.exe [/options] input-filename\n\n");
		fprintf(stderr,"    /nobarline : ���ߋ�؂肵�Ȃ� (Default=�\�Ȃ炷��)\n");
		fprintf(stderr,"    /dobarline : ���ߋ�؂�K������ (Default=�\�Ȃ炷��)\n");
		fprintf(stderr,"    /novdump   : �e�l���F���o�͂��Ȃ� (Default=����)\n");
		fprintf(stderr,"    /dotx81z   : �s�w�W�P�y�����e�l���F�o�͂�����\n");
		fprintf(stderr,"    /L<value>  : �o�̓��x�� (Default=3)\n");
		fprintf(stderr,"                  �o�͂��Ȃ� : 5 <- 4 -- 3 -> 2 : �o�͂���\n");
		fprintf(stderr,"    /dodump    : �g���b�N�_���v���[�h (�o�̓��x��=5�ɌŒ�)\n");
		fprintf(stderr,"    /dosdump   : �X�[�p�[�g���b�N�_���v���[�h (�o�̓��x��=5�ɌŒ�)\n");
		fprintf(stderr,"    /dodialog  : �Θb���[�h (/Z,/l,/b ����)\n");
		fprintf(stderr,"    /noautolen : l�R�}���h �����������ʂ��Ȃ� (Default=����)\n");
		fprintf(stderr,"    /l<value>  : l�R�}���h���� �Öق̉��� (���_�s��/Default=16 0:�g�p���Ȃ�)\n");
		fprintf(stderr,"    /b<value>  : �ŒZ�����\���� �����\���w��(���_�s��/Default=16/32)\n");
		fprintf(stderr,"    /Z<value>  : #Zenlen <value> ���� (48/96 Default=192)\n");
		fprintf(stderr,"    /B<value>  : ���ߐ��N�_�̎w�� (Default/0:Auto 1:[L] 2:�f�[�^�擪)\n");
		fprintf(stderr,"    /F<value>  : ���ߐ��N�_�̃I�t�Z�b�g�w�� (Default=0)\n");
		fprintf(stderr,"    /@<file>   : F2PCV.DLL�ꎞ�t�@�C���o�͐�\n");
		fprintf(stderr,"    /.<file>   : MML�t�@�C���o�͐� (default=�W���o��)\n");
		exit(0);
	}

	/* �t�@�C���� */
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
			/* FMP->PMD�ϊ� */
			fprintf(stderr," [JPR] File type is FMP.\n");
			cp_data = f2pconvert( cs_readbuf , i_ret );
			break;
		case D_FTYPE_PMF:
			/* VED�f�[�^�t�@�C�� */
			fprintf(stderr," [JPR] File type is VED.\n");
			cp_data = v2pconvert( cs_readbuf , i_ret );
			break;
		case D_FTYPE_UK:
		default:
			fprintf(stderr,"E[JPR] Unknown file type.\n");
			return( -1 );
	}
	t_opif.cp_indata = cp_data;

	/* �Θb���[�h */
	if( gi_dodialog == DA_ON ){
		/* �Θb���[�h�ł́A/Z /l /b �ɂ��Ď��^�������� */
		startdialog();
	}

	/* DLL�����N */
	x_dllhandle = LoadLibrary( DS_DLLFILENAME );
	if( x_dllhandle == NULL ){
		/* DLL�����N�ł��� */
		fprintf(stderr,"E[JPR] Could not load %s.\n",DS_DLLFILENAME);
		return( -1 );
	}

	/* �֐��A�h���X�擾 */
	x_funchandle = (LPFNDLLFUNC1)GetProcAddress( x_dllhandle , DS_DLLFUNCNAME );
	if( x_funchandle == NULL )
	{
		/* �֐������N�ł��� */
		fprintf(stderr,"E[JPR] Could not load %s/%s(). [%d]\n",
			DS_DLLFILENAME,DS_DLLFUNCNAME,dw_ret);
		FreeLibrary( x_dllhandle );
		return( -2 );
	}
	x_funcfmhandle = (LPFNDLLFUNC1)GetProcAddress( x_dllhandle , DS_DLLFMFUNCNAME );
	if( x_funcfmhandle == NULL )
	{
		/* �֐������N�ł��� */
		fprintf(stderr,"E[JPR] Could not load %s/%s(). [%d]\n",
			DS_DLLFILENAME,DS_DLLFMFUNCNAME,dw_ret);
		FreeLibrary( x_dllhandle );
		return( -3 );
	}

	fprintf(stderr," [JPR] Start analysis by DLL.\n");

	/* �֐�[pmdrc.dll/pmdrc_dllmain()]���s */
	t1p_ret = x_funchandle( &t_opif );

	fprintf(stderr," [JPR] End analysis by DLL.\n");

	/* ���ʉ�� */
	if( t1p_ret == NULL ){
		/* ���s�G���[ */
		if( t1p_ret->i_result > 0 ){
			/* warning ���x���Ȃ̂łƂ肠�����\���͂��� */
			fprintf(stderr,"W[JPR] MML incomplete.\n");
			fprintf(stdout," ; Incomplete reverse-compiling.\n");
			outmml( t1p_ret );
		}else{
			/* ���S�ɃG���[�Ȃ̂ŕ\�����Ȃ� */
			fprintf(stderr,"E[JPR] Error [%d].\n",t1p_ret->i_result);
			FreeLibrary( x_dllhandle );
			exit( t1p_ret->i_result );
		}
	}else{
		/* ���ʕ\�� */
		outmml( t1p_ret );
	}

	fprintf(stderr," [JPR] Succeseed output MML.\n");

	/* ���̂܂܃v���Z�X�̏I���Ȃ̂Ń���������̕K�v�͂Ȃ��̂��� */
	/* �T���v�������˂Ă���̂� �ꉞ���������������Ă��� */
	t1p_ret = x_funcfmhandle( t1p_ret );

	FreeLibrary( x_dllhandle );
	exit(0);
}
/*--------------------------------------------------------------------*/
/* �������� */
/* VED��PMD�ϊ����[�`���̃R�[�� */
char *v2pconvert( char *cp_in , int i_type ){

#define DN_SIZE_DUMMYPMDFILE_VED 8192
#define DN_SIZE_DUMMYPMDHEAD_VED 42
#define DN_SIZE_DUMMYPMDFOOT1_VED 7
#define DN_SIZE_DUMMYPMDFOOT3_VED 2
#define DN_SIZE_UNIT_VEDFMDATA 32
#define DN_SIZE_UNIT_VEDFMBODY 25

typedef	struct	_t_ff2{
	short addrpps;		/* PPS�A�h���X */
	short addrppc;		/* PPC�A�h���X */
	short addrtitle;	/* Title�A�h���X */
	short addrcomp;		/* Composer�A�h���X */
	short addrarr;		/* Arrenger�A�h���X */
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
/* �������� */
/* F2PCV.DLL�̃R�[�� */
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

	/* DLL�����N */
	x_dllhandle = LoadLibrary( DS_F2PFILENAME );
	if( x_dllhandle == NULL ){
		/* DLL�����N�ł��� */
		fprintf(stderr,"E[JPR] Could not load %s.\n",DS_F2PFILENAME);
		return( NULL );
	}
	/* �֐��A�h���X�擾 */
	x_funchandle = (LPFNDLLFUNC1)GetProcAddress( x_dllhandle , DS_F2PFUNCNAME );
	if( x_funchandle == NULL )
	{
		/* �֐������N�ł��� */
		fprintf(stderr,"E[JPR] Could not load %s/%s(). [%d]\n",
			DS_F2PFILENAME,DS_F2PFUNCNAME,dw_ret);
		FreeLibrary( x_dllhandle );
		return( NULL );
	}
	x_funcfmhandle = (LPFNDLLFUNC1)GetProcAddress( x_dllhandle , DS_F2PFMFUNCNAME );
	if( x_funcfmhandle == NULL )
	{
		/* �֐������N�ł��� */
		fprintf(stderr,"E[JPR] Could not load %s/%s(). [%d]\n",
			DS_F2PFILENAME,DS_F2PFMFUNCNAME,dw_ret);
		FreeLibrary( x_dllhandle );
		return( NULL );
	}

	t_f2pif.i_iv = si_f2pIFVersion;
	t_f2pif.i_directmes = DA_ON;
	t_f2pif.i_datatype = i_type;
	t_f2pif.cp_indata = cp_in;

	/* FMP��PMD�ϊ� */
	fprintf(stderr," [JPR] Start convert FMP->PMD.\n");
	t1p_ret = x_funchandle( &t_f2pif );

	/* ���ʉ�� */
	if( t1p_ret == NULL ){
		/* ���s�G���[ */
		if( t1p_ret->i_result > 0 ){
			/* warning ���x���Ȃ̂łƂ肠�������s�͂��� */
			fprintf(stderr,"W[JPR] convert FMP->PMD incompleteness.\n");
		}else{
			/* ���S�ɃG���[�Ȃ̂ŕ\�����Ȃ� */
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
/* �������� */
/* �t�@�C���̓ǂݎ�� �g���q�����݂��Ȃ��ꍇ�͎����I�ɕt���� */
/* �D�揇�� MZ M2 M OZI OVI OZI */
/* �ԋp�l�F-1:ERROR 0-:�擾�����t�@�C���̎�� */
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

	/* ��r�̂��߁A�S�������L���s�^���ɂ��Ă��� */
	for(cp_rcur=cs_fname;(*cp_rcur)!=0x00;cp_rcur++){
		if( (char)'a' <= *cp_rcur && *cp_rcur <= (char)'z' ){
			*cp_rcur = *cp_rcur - 0x20;
		}
	}

	/* �g���q�̌��� */
	cp_rcur = strrchr( cs_fname , '.' );
	if( cp_rcur != NULL ){
		/* �g���q���������� */
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
			/* unknown�g���q���o�l�c�`���Ƃ��Ĉ��� */
			i_type = D_FTYPE_PMD;
		}
	}else{
		/* �g���q��������Ȃ� */
		i_type = D_FTYPE_UK;
	}

	/* �t�@�C�����J�� */
	if( i_type == D_FTYPE_UK ){
		/* �t�@�C���^�C�v���g���q�������ł��Ȃ��̂ŏ��Ɏ��� */
		for(i_count=0;i_count<DN_MAX_EXTNAME;i_count++){
			/* �g���q��ς��Ȃ��玟�X�Ǝ��� */
			strcpy( cs_addfname , cs_fname );
			strcat( cs_addfname , css_extname[i_count] );
			fp_in = fopen( cs_addfname , "rb" );
			if( fp_in != NULL ){
				/* �t�@�C������ */
				i_type = is_exttype[i_count];
				break;
			}
		}
	}else{
		/* �m�肵�Ă���g���q�ŊJ�� */
		fp_in = fopen( cs_fname , "rb" );
	}

	if( fp_in == NULL ){
		/* �t�@�C����������Ȃ������ꍇ */
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

	/* �T�C�Y���̃Z�b�g */
	t_opif.i_datasize = i_ret;

	fprintf(stderr," [JPR] Succeed read file.\n");

	return( i_type );
}
/*--------------------------------------------------------------------*/
/* �o�� */
void outmml( T_OPIF *tp_out ){

	int i_count;
	FILE *fp_out;

	if( gcp_outfilename == NULL ){
		/* �o�̓t�@�C���w�� �Ȃ� */
		fp_out = stdout;
	}else{
		fp_out = fopen( gcp_outfilename , "w" );
		if( fp_out == NULL ){
			/* �t�@�C���I�[�v�����s���͕W���o�͂ɏ����o�� */
			fp_out = stdout;
			fprintf(stderr," [JPR] Failed open output-file, output to stdout.\n");
		}else{
			fprintf(stderr," [JPR] Output file name  : %s\n",gcp_outfilename);
		}

	}

	/* �o�̓o�b�t�@���`�F�b�N */
	/* i_count=1�Ŏn�܂�̂́Acps_out[0]�i���䃁�b�Z�[�W�j�� */
	/* ���łɃ��A���^�C���ŏo�͂���Ă��邽�� */
	for(i_count=1;i_count<DN_MAX_OUTBUFFER;i_count++){
		if( tp_out->cps_out[i_count] != NULL ){
			/* �o�͕����񂠂� */
			fprintf(fp_out,"%s",tp_out->cps_out[i_count]);
		}
	}

	/* �t�@�C���̃N���[�Y�͕W���o�͈ȊO�ɏo�͂����Ƃ����� */
	if( fp_out != stdout ){
		fclose( fp_out );
	}

	return;
}
/*--------------------------------------------------------------------*/
/* ���������̃f�t�H���g�l�Z�b�g */
void initinit( ){

	/* PMDRC�C���^�[�t�F�[�X�ݒ� */
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

	/* F2PCV�C���^�[�t�F�[�X�ݒ� */
	t_f2pif.i_iv = si_f2pIFVersion;
	t_f2pif.i_directmes = DA_ON;
	t_f2pif.i_datatype = 0;
	t_f2pif.cp_indata = NULL;
	t_f2pif.cp_debugdata = NULL;

	return;
}
/*--------------------------------------------------------------------*/
/* �������� */
/* ������̓��C�� �ԋp�l�F�t�@�C���� */
/* �ŏ��ɓo�ꂷ��A'/'or'-'�ȊO�Ŏn�܂�������t�@�C�����ƔF�����Ő؂� */
char *anarg( int argc , char **argv ){

	int i_argcnt;
	char *cp_curarg;

	/* �������O�̏������i�f�t�H�l�̃Z�b�g�j */
	initinit( );

	if( argc < 2 ){
		/* �����Ȃ� */
		return(NULL);
	}

	for(i_argcnt=1;i_argcnt<argc;i_argcnt++){
		cp_curarg = argv[i_argcnt];
		if( *cp_curarg == (char)'-' || *cp_curarg == (char)'/' ){
			/* �R�}���h���C������ */
			setargvalue( cp_curarg+1 );
		}else{
			/* �t�@�C�������� */
			return( cp_curarg );
		}
	}

	return(NULL);
}
/*--------------------------------------------------------------------*/
/* �������� */
/* �_�C�A���O���[�h */
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
				/* �f�t�H���gl�R�}���h�̒l���Azenlen�Ɋւ��Ȃ�16�ɂ��� */
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
/* �������� */
/* ������͏ڍ� */
void setargvalue( char *cp_in ){

	int i_value;

	switch( *cp_in ){
		case 'Z':
			/* #Zenlen default=192 */
			i_value = atoi( cp_in+1 );
			switch( i_value ){
				/* ���`���N�`���Ȓl�͋p������ */
				case 48:
				case 96:
				case 192:
				case 64:
				case 128:
				case 84:
				case 168:
				/* �Ƃ������Ă����Ȃ���14/8���q�΍�(��) */
					t_opif.i_zenlen = i_value;
					/* �f�t�H���gl�R�}���h�̒l���Azenlen�Ɋւ��Ȃ�16�ɂ��� */
					t_opif.i_lendef = t_opif.i_zenlen / 16;
					return;
			}
			break;
		case 'l':
			/* l�R�}���h(�Öى���) �������w�� default=16(%12) */
			i_value = atoi( cp_in+1 );
			if( i_value == 0 ){
				/* �[�����w�肳�ꂽ�ꍇ�͕W���������g�p���Ȃ� (1.00c) */
				fprintf( stderr , " [JPR] Forbidden use l command.\n" );
				t_opif.i_lendef = 0;
				return;
			}else if( t_opif.i_zenlen%i_value == 0 ){
				t_opif.i_lendef = t_opif.i_zenlen/i_value;
				return;
			}
			break;
		case 'b':
			/* �����Ŏ����ŏ����� �������w�� default=32(%6) */
			i_value = atoi( cp_in+1 );
			if( t_opif.i_zenlen%i_value == 0 ){
				t_opif.i_lenmin = t_opif.i_zenlen/i_value;
				t_opif.i_len3min = (t_opif.i_lenmin/3)*2;
				return;
			}
			break;
		case 'B':
			/* ���ߐ��̋N�_ def/0:auto 1:[L] 2:top */
			t_opif.i_autothresh = atoi( cp_in+1 );
			fprintf( stderr , " [JPR] Bar-line origin is top of data.\n" );
			return;
		case 'F':
			/* ���ߐ��̋N�_�I�t�Z�b�g */
			t_opif.i_manoffset = atoi( cp_in+1 );
			if( t_opif.i_manoffset > t_opif.i_zenlen ){
				/* ���ߒ���蒷���ꍇ�A���]���Ƃ� */
				t_opif.i_manoffset = t_opif.i_manoffset % t_opif.i_zenlen;
			}
			fprintf( stderr , " [JPR] Bar-line offset is [%d].\n" ,
				t_opif.i_manoffset);
			return;
		case '@':
			/* �e�l�o�f�[�^�ǂݍ��ݎ��̃e���|�����o�l�c�f�[�^�� */
			/* �t�@�C���ɏo�͂���ꍇ */
			t_f2pif.cp_debugdata = (cp_in+1);
			return;
		case '.':
			/* �o�͂l�l�k�t�@�C�����w�� */
			gcp_outfilename = (cp_in+1);
			return;
		case 'L':
			/* �o�̓��x�� */
			i_value = atoi( cp_in+1 );
			if( i_value >= 2  && i_value <= 5 ){
				t_opif.i_mmllevel = i_value;
				fprintf( stderr , " [JPR] OUTPUT-Level is %d.\n" , i_value );
				if( i_value >= 4 ){
					/* Level4�̂Ƃ���'@'���Ȃ��̂ŉ��F��`���s�v */
					t_opif.i_voicemode = DA_OFF;
					/* Level4�̂Ƃ���define���p�[�g���݂̂� */
					t_opif.i_definemode = DA_DEFL_PARTEXTEND;
				}
				return;
			}else{
				fprintf( stderr , "E[JPR] OUTPUT-Level is %d." , i_value );
				exit(1);
			}
			break;
		case 'n':
			/* "no******" �̈�c */
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
			/* "do******" �̈�c */
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
			/* �m��Ȃ����� */
			fprintf( stderr , "W[JPR] Arg. unknown. [/%s]\n\n" , cp_in );
			return;
	}


	fprintf( stderr , "E[JPR] Arg. invalid value. [/%s]\n\n" , cp_in );

	exit(1);
}
