#ifndef PMDRCIF_H
/**********************************************************************/
/* P.M.D. reversal compiler interface for PMDRC.DLL at 2004/ 5/31     */
/* IFVersion : 1.01                                                   */
/**********************************************************************/

#include "comrcif.h"

#ifndef JST_D_ONOFF
#define D_ON 1
#define D_OFF 0
#endif
#define JST_D_ONOFF

/*--------------------------------------------------------------------*/
/* �ݒ苤�ʒ萔 */
#define DA_ON 1
#define DA_OFF 0
#define DA_AUTO 1
#define DA_FORCED 2
#define DA_TX81ZVOICE 2
#define DA_DEBUG 128
#define DA_ALL 255

/* define�o�̓��x���萔 */
#define DA_DEFL_NORMAL 1
#define DA_DEFL_PARTEXTEND 2

/* �o�̓��x���萔 */
#define DN_OL_THROUGH 5
#define DN_OL_FRAME 4
#define DN_OL_NORMAL 3
#define DN_OL_DETAIL 2

/* �_���v�o�̓��x���萔 */
#define DN_DL_BOTHDUMP 3
#define DN_DL_SUPERDUMP 2
#define DN_DL_FRAMEDUMP 1
#define DN_DL_NODUMP 0

/* ���ߐ������������x���萔 */
#define DN_BARSTART_AUTO 0
#define DN_BARSTART_MARKL 1
#define DN_BARSTART_TOP 2

/* �I�y���[�V�����p�萔(�\��F������) */
#define DO_APTYPE_UK 0		/* �ŗL����Ȃ� */

#define DS_DLLFILENAME "pmdrc.dll"
#define DS_DLLFUNCNAME "pmdrc_dllmain"
#define DS_DLLFMFUNCNAME "pmdrc_dllfreem"

/*--------------------------------------------------------------------*/
/* �C���^�[�t�F�[�X�\���� V1.00 */
#define DN_MAX_OUTBUFFER 8 /* �o�̓o�b�t�@�� */
typedef	struct	_t_opif{
						/*   1.00 -> 1000 / 1.23b -> 1232 */
	int i_iv;			/* ���� �C���^�[�t�F�[�X�o�[�W������� */
	int i_av;			/* ���� �A�v���P�[�V�����o�[�W������� */
	int i_at;			/* ���� �A�v���P�[�V�����^�C�v */
	int i_lv;			/* �o�� ���C�u�����o�[�W������� */

						/*   2:Detail,3:Normal,4:Frame,5:Through */
	int i_mmllevel;		/* ���� MML�o�̓��x�� */
	int i_dumpmode;		/* ���� �_���v�o�̓��x�� 0:no 1:Dump,2:SDump,3:Both */
	int i_voicemode;	/* ���� �e�l���F�o�̓��x�� 0:OFF,1:ON */
	int i_definemode;	/* ���� define�o�̓��x�� 0:OFF,1:ON  */

	int i_zenlen;		/* ���� zenlen�l clock�\�� */
	int i_lendef;		/* ���� �W������ clock�\�� */
	int i_lenmin;		/* ���� �Œቹ�� clock�\�� */
	int i_len3min;		/* ���� �Œቹ�� clock�\���i�R�A�j */

	int i_barlevel;		/* ���� ���ߐ��������x�� 0:OFF,1:ON,2:Forced */
	int i_autothresh;	/* ���� ���ߐ������������x�� 0:AUTO 1:[L] 2:TOP */
	int i_autodelay;	/* ���� �����f�B���C 0:OFF,1:ON�i�������j */
	int i_mandelay;		/* ���� �蓮�f�B���C clock�\���i�������j */

	int i_barlen;		/* ���� �蓮���ߒ� clock�\���i�������j */
	int i_manoffset;	/* ���� �蓮�I�t�Z�b�g clock�\�� */
	int i_datasize;		/* ���� �f�[�^�T�C�Y */
	char *cp_indata;	/* ���� �f�[�^�A�h���X */

	int i_result;		/* �o�� ���� 0:OK +:Warning -:Error */
	int i_directmes;	/* ���� ���䃁�b�Z�[�W�̃��A���^�C���o�� 0:OFF 1:ON */
	int i_autolendef;	/* ���� �W�������̎����� 0:OFF 1:ON */
	/* V1.21 2008/7/14 start */
	char c_autotrans;	/* ���� �]����MML�ɔ��f */
	char c_nouse61;
	char c_nouse62;
	char c_nouse63;
	/* V1.21 2008/7/14 end */

	/* �o�̓o�b�t�@ 1:MML 2:Dump 3:SDump 4:Voice 5:Define 0:Messages */
	char *cps_out[DN_MAX_OUTBUFFER];
}T_OPIF;

/* �t�R���p�C�����C�������֐� */
extern T_OPIF __declspec(dllexport) *__stdcall pmdrc_dllmain( T_OPIF * );
/* �o�͗̈��������֐� */
extern T_OPIF __declspec(dllexport) *__stdcall pmdrc_dllfreem( T_OPIF * );

#endif
#define PMDRCIF_H
