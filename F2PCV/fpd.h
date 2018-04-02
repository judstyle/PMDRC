/**********************************************************************/
/* F2PCV - FMP -> P.M.D. data converter by JUD(T.Terata)              */
/* Definitions                                                        */
/**********************************************************************/
#ifndef FPD_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "../common/f2pcvif.h"

#define DN_PART_PHELEM 12
#define DN_PART_PDELEM 11
#define DN_PART_FELEM 11
#define DN_MAX_CONTROLCODE 512
#define DH_BMESSAGES 0
#define DN_SIZE_LINEBUFFER 1024
#define DN_SIZE_STACKUNIT 64
#define DN_SIZE_MTBUNIT 64
#define DN_SIZE_PMDFOOTREAL 25
#define DN_SIZE_PMDHEADREAL 27
#define DN_SIZE_PMDHEADBUF 28
#define DN_MAX_OUTUNITSIZE 1024
#define DN_SIZE_VOICEPMD 26
#define DN_SIZE_VOICEFMP 25
#define DN_VOFFSET_OPI 28
#define DN_VOFFSET_OVI 50

#define D_VERSION_OUTPMDDATA 0x47

#define DN_SIZE_OUTBUFUNIT 512

#define D_PARTTYPE_FM 0
#define D_PARTTYPE_SSG 1
#define D_PARTTYPE_PCM 2
#define D_PARTTYPE_UK -1

#define D_OBJ_TIE ((char)(0x66))
#define D_OBJ_SLUR ((char)(0x7A))

#define DS_DEBUGFILENAMESTR "F2PCVTEST"

/*--------------------------------------------------------------------*/
/* �����\����(PMD) */
typedef	struct	_t_ppartstate{
	int use;		/* D_ON/D_OFF */
	int partlength;	/* ���Y�p�[�g�̌��݂̒���(�I�[�܂�) */
	int bufsize;	/* ���݂̃o�b�t�@�T�C�Y */
	short offset;	/* �o�l�c�f�[�^�Ɏ��܂�Ƃ��̃I�t�Z�b�g�l */
	short nouse;
	char *buffer;	/* ���Y�p�[�g�̃o�b�t�@�A�h���X */
	char *bufcur;	/* ���̃o�b�t�@�������݃A�h���X */
}T_PPARTSTATE;

/* �����\����(FMP) */
typedef	struct	_t_fpartstate{
	int use;
	int parttype;	/* �p�[�g�^�C�v */
	int ppartnum;	/* �ϊ���̃p�[�g�ԍ� */
	char *pdata;	/* �p�[�g�f�[�^�擪�A�h���X */
	char *lladdr;	/* 'L' �����݂���i�ׂ��j�A�h���X */
	char **lsaddr;	/* '['���[�v�J�n�L�������݂���i�ׂ��j�A�h���X�Q */
	char **lscur;	/* �A�h���X�Q�Ŏ��̒l */
	char *nextlp;	/* ����'L'�܂���'['���o�ꂷ��A�h���X */
}T_FPARTSTATE;

/* control codes */
typedef	struct	_t_tcode{
	int cont;
	int f_opelen[3];	/* �ϊ��O�̃I�y���[�V������ */
	int f_datalen[3];	/* �ϊ��O�̃f�[�^�� */
	int p_opelen;		/* �ϊ���̃I�y���[�V������ */
	int p_datalen;		/* �ϊ���̃f�[�^�� */
	char fcode[7];		/* �ϊ��O�̃f�[�^ �V�o�C�g(7c,01�n�̂��ߑ���) */
	char pcode1;		/* �ϊ���̃f�[�^ �P�o�C�g�� */
	int convtype;		/* 0:���� 1:�����u�� 2:�f�[�^�t���P���u�� */
						/* 3:�f�[�^�Ȃ��P���u�� 4:�ʏ��� */
}T_TCODE;








#endif
#define FPD_H
