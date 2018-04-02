#ifndef F2PCVIF_H
/**********************************************************************/
/* FMP -> P.M.D. data converter interface for F2PCV.DLL               */
/**********************************************************************/

#include "comrcif.h"

#ifndef JST_D_ONOFF
#define D_ON 1
#define D_OFF 0
#endif
#define JST_D_ONOFF

/*--------------------------------------------------------------------*/
/* �ݒ苤�ʒ萔 */

#define D_TYPE_UK 0
#define D_TYPE_OPI 2
#define D_TYPE_OVI 3
#define D_TYPE_OZI 4

#define D_TYPE_06PART 0x20
#define D_TYPE_11PART 0x40

#define DS_F2PFILENAME "f2pcv.dll"
#define DS_F2PFUNCNAME "f2pcv_dllmain"
#define DS_F2PFMFUNCNAME "f2pcv_dllfreem"
/*--------------------------------------------------------------------*/
/* �C���^�[�t�F�[�X�\���� V1.10 */
#define DN_MAX_F2POUTBUFFER 4 /* �o�̓o�b�t�@�� */
typedef	struct	_t_f2pif{

	int i_iv;			/* ���o�� �C���^�[�t�F�[�X�o�[�W������� */
	int i_directmes;	/* ���� ���䃁�b�Z�[�W�̃��A���^�C���o�� 0:OFF 1:ON */
	int i_datatype;		/* ���o�� �f�[�^�t�@�C���̃^�C�v(OPI/OVI/OZI) */
	int i_result;		/* �o�� ���� 0:OK +:Warning -:Error */

	char *cp_indata;	/* ���� �f�[�^�A�h���X */
	char *cp_debugdata;	/* ���� �f�o�b�O�f�[�^ */
	char *cp_outdata;	/* �o�� �f�[�^�A�h���X */
	int i_datalen;		/* �o�� �f�[�^�T�C�Y */

	int i_voiceconv;	/* ���� ���F�R���o�[�g�L�� 0:OFF 1:ON */
	int i_nouse31;
	int i_nouse32;
	int i_nouse33;

	/* �o�̓o�b�t�@ 0:Messages */
	char *cps_out[DN_MAX_F2POUTBUFFER];
}T_F2PIF;

/* �t�R���p�C�����C�������֐� */
extern T_F2PIF __declspec(dllexport) *__stdcall f2pcv_dllmain( T_F2PIF * );
/* �o�͗̈��������֐� */
extern T_F2PIF __declspec(dllexport) *__stdcall f2pcv_dllfreem( T_F2PIF * );

#endif
#define F2PCVIF_H
