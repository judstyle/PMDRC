/**********************************************************************/
/* PMDRC - P.M.D. reversal compiler by JUD(T.Terata)                  */
/* Definitions                                                        */
/**********************************************************************/
#ifndef PRD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "../common/pmdrcif.h"

/*--------------------------------------------------------------------*/
/* ��{�萔 */
#define D_LAST 0
#define D_NOTLAST 1

#ifndef JST_D_ONOFF
#define D_ON 1
#define D_OFF 0
#endif
#define JST_D_ONOFF

#define DS_DEFINE_PPZEXTEND "#PPZExtend  abcdefgh\n"
#define DS_DEFINE_FM3EXTEND "#FM3Extend  XYZ\n"

#define DS_DEFINE_PZINAME   "#PPZFile    %s\n"
#define DS_DEFINE_PPSNAME   "#PPSFile    %s\n"
#define DS_DEFINE_PPCNAME   "#PCMFile    %s\n"
#define DS_DEFINE_TITLE     "#Title      %s\n"
#define DS_DEFINE_COMPOSER  "#Composer   %s\n"
#define DS_DEFINE_ARRENGER  "#Arranger   %s\n"
#define DS_DEFINE_MEMO      "#Memo       %s\n"

#define DS_OK "OK"
#define DS_NG "NG"

#define DC_ADDSYMBOL '&'
/*--------------------------------------------------------------------*/
/* ���������萔 */

#define DN_SIZE_LINEBUFFER 1024
#define DN_MAX_TRACKNUM 21
//#define DN_MAX_OBJSIZE 32768
#define DN_MAX_CONTROLCODE 256
#define DN_MAX_OUTUNITSIZE 1024 /* �o�̓o�b�t�@���ݒP�� */
/* �����e�[�u�����������ݒP�� */
#define DN_GETBON 20
#define DN_GETBOC 16
#define DN_GETBOM 16
#define DN_SIZE_MTBUNIT 64
#define DN_MAX_NOTELENFREQ 192

#define DN_LENGTH_DEFLIMIT 98765

/* �W�����Z�o�萔 */
#define DN_PCT_12P 0.05
#define DN_PCT_21P 0.9
#define DN_PCT_31P 0.7
#define DN_PCT_32P 0.2
#define DN_PCT_42P 0.3
#define DN_PCT_52P 0.25
#define DN_PCT_61P 0.1

/* �o�b�t�@�n���h�� */
#define DH_BMESSAGES 0
#define DH_BMML 1
#define DH_BDUMP 2
#define DH_BSDUMP 3
#define DH_BVOICE 4
#define DH_BDEFINE 5
#define DH_BLAST 6

/* �o�l�c�t�H�[�}�b�g�萔 */
#define DC_START_CHAR_FMSSG 'A'
#define DC_START_CHAR_ADPCM 'J'
#define DC_START_CHAR_FM3EX 'X'
#define DC_START_CHAR_PPZ 'a'
#define DN_START_NUM_FMSSG 0
#define DN_START_NUM_ADPCM 9
#define DN_START_NUM_FM3EX 10
#define DN_START_NUM_PPZ 13
/* �o�l�c�f�[�^���̂e�l���F�f�[�^�T�C�Y */
#define DN_FMTONE_SIZE 26
/* #MEMO �̍ő�� */
#define DN_MAX_MEMOPOINT 128

/* ���������p�R�}���h�萔 */
#define D_OP_TLINIT 1
#define D_OP_TLADD 2
#define D_OP_TLLSTART 3
#define D_OP_TLLEND 4
#define D_OP_TLTOTAL 5
#define D_OP_TLBEFOREL 6
#define D_OP_TLFINDL 7
#define D_OP_TLFINDCL 8
#define D_OP_TLTLEVEL 9
#define D_OP_TLLEVEL 10

/* �e�[�u����ʗp */
#define D_LC_NOTE 1
#define D_LC_CODE 2

/* ���ߐ��������[�h�����萔 */
#define D_BAR_UNDEFINE -1
	/* ������ */
#define D_BAR_FULLAUTO1 1
	/* L�ȍ~���S�p�[�g�������Azenlen�̔{�� */
#define D_BAR_FULLAUTO2 2
	/* L�ȍ~���S�p�[�g�������Ȃ��A�S�p�[�gzenlen�̔{�� */
#define D_BAR_FULLAUTO3 3
	/* L�ȍ~���S�p�[�g�������Ȃ��A�Œ��p�[�g���S�p�[�g���̍ŏ����{�� */
#define D_BAR_HALFAUTO5 4
	/* ������ L�����݂��Ȃ����A�S�p�[�g��zenlen�̔{�� */
#define D_BAR_TOPSTART 5
	/* �N�_���f�[�^�擪�ɌŒ� /B2 ���w�肳�ꂽ�ꍇ */

/* L�ȍ~��zenlen�ŁAL�ȑO��L���O����t������zenlen�P�ʂ� */
#define D_BAR_HALFAUTO1 101
	/* ������ L�ȍ~���S�p�[�g�������Azenlen�̔�{�� */
	/* �w�肪�����barlen�ŕ��� */
#define D_BAR_HALFAUTO2 102
	/* ������ L�����݂��Ȃ����A�S�p�[�g���� */
	/* �d�ݔ��f�ɂ�鎩������ */
#define D_BAR_MANUAL 999
	/* �ǂ̖@���ɂ����Ă͂܂�Ȃ��ꍇ�A���ߕ������s��Ȃ� */
#define D_BAR_FORCEDMANUAL 998
	/* �����I�ɋ֎~�B���ߕ������s��Ȃ� */
#define D_BAR_FORCEDAUTO 997
	/* �����I�Ɏ��{�B���ߕ�����K���s�� */

/* notes , portaments , r 28bytes V1.21 */
typedef	struct	_t_bon{
	long toffset;	/* �n�_����̑����� */
	short length;	/* �I�u�W�F�N�g���� */
	char note1;		/* note/�|���^�����g�n�_/control-code */
	char note2;		/* �|���^�����g�I�_/control-value */
	char trans;		/* �ݐσg�����X�|�[�Y */
	char mtrans;	/* �}�X�^�[�g�����X�|�[�Y */
	char oct;		/* ���݂�oct */
	char ties;		/* �^�C�E�X���[ (&/&&) */
	char barline[2];/* ���ߐ� */
	char nocombine;	/* �㑱�Ƃ̃R���o�C���֎~ */
	char nouse6;
	char *address;	/* �R�[�h�̃A�h���X��ۊǁ��o�͑O�ɎQ�Ƃ��� V1.21 */
	struct _t_bon *newarea;	/* ���̗̈�̎n�_addr NULL:Empty */
	struct _t_bon *befarea;	/* �O�̗̈�̎n�_addr NULL:StartPoint */
}T_BON;
/* �i�[������R�[�h */
/* �����E�x���A�|���^�����g�A�^�C�E�X���[�A�󔒁A�]���n */

/* controlles 24bytes */
typedef	struct	_t_boc{
	long toffset;	/* �n�_����̑����� */
	char code;		/* code */
	char value;		/* ���ߒ���(Z) ���[�v��(]) */
	char nouse1;
	char disable;	/* D_ON:���ׂ��ς� */
	char barline[2];/* ���ߐ� */
	char nocombine;	/* �㑱�Ƃ̃R���o�C���֎~ */
	char nouse6;
	char *address;	/* �R�[�h�̃A�h���X��ۊǁ��o�͑O�ɎQ�Ƃ��� */
	struct _t_boc *newarea;	/* ���̗̈�̎n�_addr NULL:Empty */
	struct _t_boc *befarea;	/* �O�̗̈�̎n�_addr NULL:StartPoint */
}T_BOC;

/* mastertrack 16bytes */
typedef	struct	_t_bom{
	long toffset;	/* �n�_����̑����� */
	char code;
	unsigned char barlen;	/* ���ߒ��� */
	char nouse1;
	char last;
	struct _t_bom *newarea;	/* ���̗̈�̎n�_addr NULL:Empty */
	struct _t_bom *befarea;	/* �O�̗̈�̎n�_addr NULL:StartPoint */
}T_BOM;

/* track status 32bytes x 21 */
typedef	struct	_t_tstatus{
	char use;		/* �g�p����ꍇ:D_ON ���݂��Ȃ��y�ђ����[��:D_OFF */
	char partname;	/* ���O�͌Œ� A-F G-I J X-Z a-h */
	char offset;	/* �p�[�g�擪��r%1���ɑΉ�����I�t�Z�b�g */
	char startoct;	/* �ŏ���o�R�}���h�Ŏw�肷��l */
	int partlen;	/* �p�[�g�̒��� 0x80�̓o�ꂷ��l_clock */
	T_BON *naddr;	/* notes�̈�̃�������̃A�h���X */
	T_BOC *caddr;	/* controlles�̈�̃�������̃A�h���X */
	char *text;		/* �ǂݍ��݃t�@�C���̃p�[�g�擪�A�h���X */
	int tracktotal;	/* �g���b�N�S�̂̒����i�������[�v[]0�̎���-1�j*/
	int tracklenbl;	/* �g���b�N����L���O�̒����iL���Ȃ��ꍇ-1�j*/
	int tracklenal;	/* �g���b�N����L����̒����iL���Ȃ��ꍇ-1�j*/
}T_TRACKTABLE;

/* FM-Tone Bank 27(26)bytes */
typedef	struct	_t_fmt{
	unsigned char tnumber;	/* ToneNumber */
	unsigned char dtml[4];	/* 1/3-DT/4-ML */
	unsigned char tl[4];	/* 1/7-TL */
	unsigned char ksar[4];	/* 1/2-KS/5-AR */
	unsigned char amdr[4];	/* 2/1-AMS/5-DR */
	unsigned char sr[4];	/* 3/5-SR */
	unsigned char slrr[4];	/* 4-SL/4-RR */
	unsigned char fbal;		/* 2/3-FeedBack/3-Algorhythm */
	unsigned char nextnum;	/* ToneNumber */
}T_FMT;

/* FM-Tone Format(INTERNAL) */
typedef	struct	_t_ifmt{
	int i_ml;	/* ML */
	int i_dt;		/* DT */
	int i_ar;		/* AR */
	int i_dr;		/* DR */
	int i_sl;		/* SL */
	int i_sr;		/* SR */
	int i_rr;		/* RR */
	int i_tl;		/* TL */
	int i_ks;		/* KS */
	int i_ams;		/* AMS */
}T_IFMT;
/* FM-Tone Format(OUTPUT) */
typedef	struct	_t_ofmt{
	float f_freq;	/* ML */
	int i_det;		/* DT */
	int i_ar;		/* AR */
	int i_d1r;		/* DR */
	int i_d1l;		/* SL */
	int i_d2r;		/* SR */
	int i_rr;		/* RR */
	int i_out;		/* TL */
	int i_rs;		/* KS */
}T_OFMT;

/* control codes */
typedef	struct	_t_tcode{
	int cont;
	char code1;		/* �P�o�C�g�� */
	char code2;		/* 0x00�ȊO�̂Ƃ� �Q�o�C�g�� */
	char dmusk;		/* �Q�o�C�g�ڂ𔻒f����t�}�X�N(0x80�̎�bit7�����𔻒f) */
	char nouse;		/* nouse */
	int datasize;	/* ���ɑ����f�[�^�T�C�Y */
	int format;		/* �t�H�[�}�b�g */
	int level;
		/*-�V�t�H�[�}�b�g---------------------*/
		/* �����F                             */
		/*  0:���� */
		/*  1:expset�֐����R�[�����A�������������{(FM3Ex/PPZEx) */
		/*  2:���� */
		/*  3:���Y�������\�����邾�� */
		/*  4:�f�[�^(1byte)��t�����ĕ\��(�������� -128/127) */
		/*  5:�f�[�^(1byte)��t�����ĕ\��(�����Ȃ� 0/255) */
		/*  6:2bytes�ȏ�̃f�[�^��t�����ĕ\��(�l���͕ʓr) */
		/*  7:�f�[�^(1byte)��t�����ĕ\��(�������� -128/127) */
		/*  8:�f�[�^(1byte)��t�����ĕ\��(�������� -128/127) �[�����͖��o�� */
		/* ���x���F                           */
		/*  0:��ɔ�\���A���ԃI�u�W�F�N�g�ɂ��ϊ����Ȃ� */
		/*  1:�擾���s��Ȃ� */
		/*  2:�O���[�o�����ɕϊ� */
		/*  3:�S�o�͎w�莞�̂ݎ擾 */
		/*  4:�ʏ�\�� */
		/*  5:�ȗ��\�����ł��\�� */
		/*------------------------------------*/
	int table;		/* ��������table 1:note 0:control */
	int steps;		/* D_ON/1:������L����ꍇ D_OFF/0:�����[�� */
	char *str;		/* MML������ */
	char *outformat;/* MML�o�͎��t�H�[�}�b�g������ format=6�̎��̂ݗL�� */
					/* '#':�P�o�C�g�������� '$':�Q�o�C�g�������� */
					/* '(':�P�o�C�g�������� ')':�Q�o�C�g�������� */
					/* '_':�P�o�C�g�����Ȃ� '~':�Q�o�C�g�����Ȃ� */
					/* 'a':�W�r�b�g�����Ȃ� 'b':�V�r�b�g�����Ȃ� */
					/* 'z':�t�����W�r�b�g�����Ȃ� 'y':�t�����V�r�b�g�����Ȃ� */
					/* ' ':�P�o�C�g�\�����Ȃ� */
					/* ����ȊO:���̂܂ܕ\�� */
					/* ex) "_#$#" �� 1(unsigned)/1/2/1 */
}T_TCODE;

/* �V�X�e���ϐ� */
/* ����������i����`���j��0x00�p�f�B���O����B�ʂɏ�����������̂����� */
typedef	struct	_t_ginfo{
	char c_varsion;		/* mc.exe�o�[�W������� */
	char nouse1;		
	char nouse2;		
	char nouse3;		
	long l_mctotal;		/* mc.exe�ɂ��totalclock ����������-1 */
	long l_mcloop;		/* mc.exe�ɂ��loopclock ����������-1 */
	char *cp_voice;		/* FM���F��`�擪�A�h���X */
	char *cp_dtaddr;	/* �e��e�L�X�g��`�A�h���X�Q�̃A�h���X */
	char *cp_pziname;	/* PPZFile�̃A�h���X */
	char *cp_ppsname;	/* PPSFile�̃A�h���X */
	char *cp_ppcname;	/* PPCFile�̃A�h���X */
	char *cp_title;		/* Title�̃A�h���X */
	char *cp_composer;	/* Composer�̃A�h���X */
	char *cp_arrenger;	/* Arrenger�̃A�h���X */
	char *cps_memo[DN_MAX_MEMOPOINT];
						/* memo�̃A�h���X�z�� */
	int i_nmemo;		/* memo�� ����������-1 */
	int i_fPPZExtend;	/*-PPZExtend 0:OFF 1:ON */
	int i_fFM3Extend;	/*-FM3Extend 0:OFF 1:ON */
	int i_Zcount;		/*-Z�R�}���h�̓o��� */
	int i_Znotonline;	/*-Z�R�}���h�����ߐ���ȊO�ɑ��݂��� 0:no 1;yes */
	int i_Fcount;		/*-F�R�}���h�o��� */
	int i_Lcount;		/*-L�R�}���h�o��� */
	int i_ILLcount;		/*-�������[�v�Ǐ����[�v"]0"�R�}���h�o��� */
}T_GINFO;




#endif
#define PRD_H
