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

#include "fpd.h"
#include "fpdext.h"



/*--------------------------------------------------------------------*/
/* ��̓��C�� */
int anlmain( T_F2PIF *tp_in ){

	int i_ret;
	char *cp_ret;
	int i_count;
	int i_partelem;
	char *cp_pbuf;
	char *cp_pcur;
	int i_pbufsize;
	int i_totalpartsize;
	int i_FMparam;

	sprintf(gcs_line," [F2P] Start analysis.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* �����P �w�b�_�̎Q�ƁA�e�p�[�g�̎n�_�A���[�v�A�h���X�̎�荞�� */
	i_ret = readheader( tp_in->cp_indata );
	if( i_ret < 0 ){
		return( i_ret );
	}
	sprintf(gcs_line," [F2P] Complete read header.\n");
	hfpf( DH_BMESSAGES , gcs_line );
	i_partelem = i_ret;

	/* �����P�D�T �e�l���F�f�[�^�T�[�` (V1.01c) */
	i_FMparam = 0;
	i_ret = getFMparam( tp_in->cp_indata );
	if( i_ret > 0 ){
		/* �e�l�p�����[�^�擾���� */
		i_FMparam = i_ret;
		sprintf(gcs_line," [F2P] Complete read FM-parameters [%d].\n",i_ret);
		hfpf( DH_BMESSAGES , gcs_line );
	}else if( i_ret < 0 ){
		sprintf(gcs_line," [F2P] Failed read FM-parameter.\n");
		hfpf( DH_BMESSAGES , gcs_line );
	}

	/* �����Q �e�p�[�g�̃��[�v�I�_���������A�Ή�����n�_�A�h���X��� */
	for(i_count=0;i_count<i_partelem;i_count++){
		if( ts_tsf[i_count].use == D_ON ){
			i_ret = searchloop( &(ts_tsf[i_count]) );
			if( i_ret < 0 ){
				return( i_ret );
			}
		}
	}
	sprintf(gcs_line," [F2P] Complete pickup looppoints.\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* �����R �e�p�[�g�̃f�[�^���R���o�[�g���Ȃ���o�b�t�@�ɏ����o�� */
	for(i_count=0;i_count<i_partelem;i_count++){
		if( ts_tsf[i_count].use == D_ON ){
			i_ret = convertpart( &(ts_tsf[i_count]) );
			if( i_ret < 0 ){
				return( i_ret );
			}
		}
	}
	sprintf(gcs_line," [F2P] Complete convert trackdata.\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* �����S ���g�p�o�l�c�g���b�N�ɏI�[���d����ŉ�� */
	for(i_count=0;i_count<DN_PART_PDELEM;i_count++){
		if( ts_tsp[i_count].use == D_OFF ){
			ts_tsp[i_count].buffer = (char *)&DS_PMD_ONLYTERMINATION;
//			ts_tsp[i_count].use = D_ON;
			ts_tsp[i_count].partlength = 1;
			ts_tsp[i_count].bufsize = 1;
		}
	}
	sprintf(gcs_line," [F2P] Odd trackdata.\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* �o�l�c�w�b�_��� �f�[�^�T�C�Y���� */
	i_totalpartsize = makepmdhead( NULL );
	if( i_totalpartsize < 0 ){
		sprintf(gcs_line,"E[F2P] Error in makepmdhead(NULL).\n");
		hfpf( DH_BMESSAGES , gcs_line );
		return( i_totalpartsize );
	}

	/* �o�b�t�@�m�� */
	/* makepmdhead�ԋp�l=�p�[�g���v + �w�b�_�T�C�Y + �t�b�^�T�C�Y */
	/* 1.01c�Œǉ� ����ɉ��F�f�[�^�P�ɂ�26bytes */
	i_pbufsize = i_totalpartsize;
	i_pbufsize += DN_SIZE_PMDHEADREAL;
	i_pbufsize += DN_SIZE_PMDFOOTREAL;
	i_pbufsize += DN_SIZE_VOICEPMD * i_FMparam;
	if( gcp_comment != NULL ){
		i_pbufsize += (strlen(gcp_comment)+1);
	}
	cp_pbuf = malloc( i_pbufsize );
	cp_pcur = cp_pbuf;
	/* �h�e�ɃA�h���X�L�^�ƃo�b�t�@�T�C�Y��Y�ꂸ�� */
	tp_in->i_datalen = i_pbufsize;
	tp_in->cp_outdata = cp_pbuf;

	/* �w�b�_�쐬 */
	makepmdhead( cp_pcur );
	cp_pcur += DN_SIZE_PMDHEADREAL;
	sprintf(gcs_line," [F2P] Complete make pmdhead.\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* �o�l�c�f�[�^�̈�ɏ������� */
	for(i_count=0;i_count<DN_PART_PDELEM;i_count++){
		memcpy( cp_pcur,ts_tsp[i_count].buffer,ts_tsp[i_count].partlength );
		cp_pcur += ts_tsp[i_count].partlength;
		if( ts_tsp[i_count].use == D_ON ){
			apfree( ts_tsp[i_count].buffer );
		}
		sprintf(gcs_line," [F2P] Part%2d : %6d Bytes.\n",
			i_count , ts_tsp[i_count].partlength );
		hfpf( DH_BMESSAGES , gcs_line );
	}

	/* �t�b�^�쐬 V1.01c�ŕύX */
	makepmdfoot( cp_pcur , i_totalpartsize , i_FMparam );
	sprintf(gcs_line," [F2P] Complete make pmdfoot.\n");
	hfpf( DH_BMESSAGES , gcs_line );

	return( 0 );
}
/*--------------------------------------------------------------------*/
/* ��� �e�l�o�̃f�[�^�A�h���X����A�Y�������`�\���̃A�h���X��Ԃ� */
T_TCODE *searchbycode( char *cp_in , int i_in ){

	T_TCODE *tp_ret;
	int i_count;
	int i_c1;
	int i_c2;
	int i_c3;
	int i_isdiff;
	char css_PNAME[4][4]={" FM","SSG","PCM",""};

	for(i_count=0;i_count<DN_MAX_CONTROLCODE;i_count++){
		if( ts_tc[i_count].cont == D_OFF ){
			/* ���X�g�̏I�[�܂ŗ������Y���i�V */
			break;
		}
		if( ts_tc[i_count].f_opelen[i_in] == 0 ){
			/* ���̃p�[�g�^�ł͖��g�p�̃f�[�^ */
			continue;
		}
		i_isdiff = D_OFF;
		for(i_c1=0;i_c1<ts_tc[i_count].f_opelen[i_in];i_c1++){
			if( (char)(ts_tc[i_count].fcode[i_c1]) !=
				(char)(*(cp_in+i_c1)) ){
				/* ����� */
				i_isdiff = D_ON;
				break;
			}
		}
		if( i_isdiff != D_ON ){
			/* �����œ�����̏��� */
			return( &(ts_tc[i_count]) );
		}
	}

	/* ���̎��_�Ńn�Y���Ȃ̂ŁA��蕔����\������ */
	sprintf(gcs_line,
		"E[F2P] ERROR in searchbycode(). [%s:%02X/%02X/%02X/%02X]\n",
		css_PNAME[i_in],
		(unsigned char)(*(cp_in)),
		(unsigned char)(*(cp_in+1)),
		(unsigned char)(*(cp_in+2)),
		(unsigned char)(*(cp_in+3)) );
	hfpf( DH_BMESSAGES , gcs_line );

	return( NULL );
}
/*--------------------------------------------------------------------*/
/* ��� �p�[�g�P�ʂŃR���o�[�g */
int convertpart( T_FPARTSTATE *tp_in ){

	char *cp_fcur;
	char *cp_pcur;
	int i_ret;

	char *cp_ret;
	T_TCODE *tp_tcode;

	T_TCODE *tp_pcode;
	char c_tieorslur;

	int i_tieflag=D_OFF;
	int i_slurflag=D_OFF;
	int i_stdlen=8;
	int i_count;

	unsigned char uc_tmp;

	/* �o�l�c�o�b�t�@�̏����� */
	for(i_count=0;i_count>DN_PART_PDELEM;i_count++){
		ts_tsp[i_count].bufsize = 0;
		ts_tsp[i_count].partlength = 0;
		ts_tsp[i_count].buffer = NULL;
		ts_tsp[i_count].bufcur = NULL;
	}

	/* �R���o�[�g��̐ݒ� */
	ts_tsp[tp_in->ppartnum].partlength = 0;
	ts_tsp[tp_in->ppartnum].use = D_ON;

	for(cp_fcur=tp_in->pdata;;){
//fprintf(stderr,"%1X:STEP:%02X\n",tp_in->ppartnum,*cp_fcur);
		/* �I�������̃`�F�b�N���I������ */
		if( *cp_fcur == 0x74 ){
			outpmd( tp_in->ppartnum , (char)0x80 );
			break;
		}

		tp_tcode = searchbycode( cp_fcur , tp_in->parttype );
		if( tp_tcode == NULL ){
			/* UNKNOWN-CODE */
			sprintf(gcs_line,"E[F2P] Search error at searchbycode().\n");
			hfpf( DH_BMESSAGES , gcs_line );
			return( -1 );
		}

		/* ���[�v�L���̃q�b�g�`�F�b�N */
		if( cp_fcur == tp_in->nextlp ){
			/* L �܂��� [ ���[�v���䕶���̏o�� */
			if( tp_in->nextlp == tp_in->lladdr ){
				/* L �̕��� */
				outpmd( tp_in->ppartnum , (char)0xF6 );
				/* 'L'�͂P��g������I��� */
				tp_in->lladdr = NULL;
			}else{
				/* [ �̕����i�ƃf�[�^���̃n���{�e2byte�j */
				outpmd( tp_in->ppartnum , (char)0xF9 );
				outpmd( tp_in->ppartnum , (char)0x00 );
				outpmd( tp_in->ppartnum , (char)0x00 );
				tp_in->lscur++;
			}
			/* ���񃋁[�v�L���ʒu�`�F�b�N�i'L'��'['���D�悾���j */
			if( tp_in->lladdr != NULL && *(tp_in->lscur) > tp_in->lladdr ){
				/* ��ɗ���̂�'L'�A����'L'���ݒu */
				tp_in->nextlp = tp_in->lladdr;
			}else if( tp_in->lscur != NULL ){
				/* ��ɗ���̂�'['�A�܂���'L'�ݒu�ς� */
				tp_in->nextlp = *(tp_in->lscur);
			}else{
				/* '[' �����݂��Ȃ��ꍇ */
				tp_in->nextlp = NULL;
			}
			/* �����ł͎��ۂɂe�l�o���̃f�[�^��ǂ�ł���킯�ł͂Ȃ��̂� */
			/* �|�C���^��i�߂��ɂ�����x�������o�� */
			continue;
		}

		/* �f�[�^�̂P�X�e�b�v��� */
		switch( tp_tcode->convtype ){
			case 0:
				/* ���� */
				break;
			case 1:
				/* �����^�u�� �����Ȃ���ΕW���l���g�p */
				if( tp_tcode->f_datalen[tp_in->parttype] == 0 ){
					/* �����̂Ȃ����� */
					uc_tmp = i_stdlen;
				}else{
					/* �ʏ�o�l�c�^�̉��� */
					uc_tmp = *(cp_fcur+1);
				}
				outpmd( tp_in->ppartnum , tp_tcode->pcode1 );
				outpmd( tp_in->ppartnum , (char)uc_tmp );

				/* �^�C�E�X���[ */
				if( i_tieflag == D_ON ){
					/* ������& */
					c_tieorslur = D_OBJ_TIE;
					i_tieflag = D_OFF;
					tp_pcode = searchbycode( &c_tieorslur , tp_in->parttype );
					if( tp_pcode == NULL ){
						/* �R�[�h�Y���i�V */
						return( -1 );
					}
					outpmd( tp_in->ppartnum , tp_pcode->pcode1 );
				}else if( i_slurflag == D_ON ){
					/* ������&& */
					c_tieorslur = D_OBJ_SLUR;
					i_slurflag = D_OFF;
					tp_pcode = searchbycode( &c_tieorslur , tp_in->parttype );
					if( tp_pcode == NULL ){
						/* �R�[�h�Y���i�V */
						return( -1 );
					}
					outpmd( tp_in->ppartnum , tp_pcode->pcode1 );
				}
				break;
			case 2:
				/* �f�[�^�t���P���u���i�T�C�Y�͎����ǐ��j */
				outpmd( tp_in->ppartnum , tp_tcode->pcode1 );
				for(i_count=0;i_count<tp_tcode->f_datalen[tp_in->parttype];
					i_count++){
					outpmd( tp_in->ppartnum , *(cp_fcur+1+i_count) );
				}
				break;
			case 3:
				/* �f�[�^�Ȃ��P���u���i�ϊ���̃T�C�Y�̓i�V�j */
				outpmd( tp_in->ppartnum , tp_tcode->pcode1 );
				break;
			case 4:
				/* �^�C�A�t���O���Ă邾�� */
				i_tieflag=D_ON;
				break;
			case 5:
				/* �X���[�A�t���O���Ă邾�� */
				i_slurflag=D_ON;
				break;
			case 6:
				/* �W�������A�����I�ɍX�V���邾�� */
				i_stdlen = (int)((unsigned char)(*(cp_fcur+1)));
				break;
			case 7:
				/* �f�[�^���̓n���{�e�Ƃ��Ĉ����I�y���[�g */
				/* �I�y���[�g�����ϊ����A */
				/* �f�[�^���͕ϊ���f�[�^�����ɏ]���[���p�f�B���O */
				outpmd( tp_in->ppartnum , tp_tcode->pcode1 );
				for(i_count=0;i_count<tp_tcode->f_datalen[tp_in->parttype];
					i_count++){
					outpmd( tp_in->ppartnum , 0 );
				}
				break;
			case 8:
				/* ���[�v�I�_ */
				outsloopend( cp_fcur , tp_in->ppartnum );
				break;
			case 9:
				/* �|���^�����g */
				outsporta( cp_fcur , tp_in->ppartnum , i_stdlen );
				break;
			default:
				/* �o�O */
				break;
		}

		/* �����̎擾�E�ǂݎ�葤�̃|�C���^�X�L�b�v */
		i_ret = getflen( cp_fcur , tp_in->parttype );
		if( i_ret < 0 ){
			/* �R�[�h�Y���i�V */
			return( -1 );
		}
		cp_fcur += i_ret;
	}

	/* �I�������́A�Ȃ� */
	return( 0 );
}
/*--------------------------------------------------------------------*/
/* ��� �p�[�g���ƂɈقȂ�Y������I�y���[�g�̒�����Ԃ� */
/* �����P�F���̓f�[�^ */
/* �����Q�F�p�[�g�^(FM/SSG/PCM) */
int getflen( char *cp_in , int i_type ){

	T_TCODE *tp_tcode;
	int i_totallen;

	tp_tcode = searchbycode( cp_in , i_type );
	if( tp_tcode == NULL ){
		sprintf(gcs_line,"E[F2P] Search error in getflen().\n");
		hfpf( DH_BMESSAGES , gcs_line );
		return( -1 );
	}

	i_totallen = tp_tcode->f_opelen[i_type] + tp_tcode->f_datalen[i_type];

	return( i_totallen );
}
/*--------------------------------------------------------------------*/
/* ��� ���[�v�n�_�̌��� */
int searchloop( T_FPARTSTATE *tp_in ){

	char *cp_cur;
	int i_curopesize;
	int i_rewindsize;
	char *cp_startpoint;

	if( tp_in->use != D_ON ){
		/* ���g�p�p�[�g�Ȃ�Ȃ�ɂ����Ȃ� */
		return( 0 );
	}

	/* �I�[�܂Ń��[�v */
	for(cp_cur=tp_in->pdata;(*cp_cur)!=(char)0x74;){
		if( *cp_cur != (char)0x64 ){
			/* ���[�v�I�_�łȂ� */
			i_curopesize = getflen( cp_cur , tp_in->parttype );
			if( i_curopesize < 0 ){
				/* �R�[�h�Y���i�V */
				return( -1 );
			}
			cp_cur += i_curopesize;
			continue;
		}
		/* ���[�v���� */
		i_rewindsize = (int)((unsigned char)(*(cp_cur+2)));
		i_rewindsize += ( 256 * (int)((unsigned char)(*(cp_cur+3))) );
		cp_startpoint = cp_cur - (i_rewindsize-4);
		stackaddr( (void *)cp_startpoint );
		i_curopesize = getflen( cp_cur , tp_in->parttype );
		if( i_curopesize < 0 ){
			/* �R�[�h�Y���i�V */
			return( -1 );
		}
		cp_cur += i_curopesize;
	}

	/* �X�^�b�N�̐���Ǝ擾 */
	tp_in->lsaddr = (char **)stackaddr( NULL );
	tp_in->lscur = tp_in->lsaddr;

	/* 'L' �܂��� '[' ���荞�݃A�h���X�A�ŏ��̂P���Z�b�g */
	if( tp_in->lsaddr == NULL && tp_in->lladdr == NULL ){
		tp_in->nextlp = NULL;
	}else if( tp_in->lsaddr != NULL && tp_in->lladdr != NULL ){
		if( *(tp_in->lsaddr) < tp_in->lladdr ){
			tp_in->nextlp = *(tp_in->lsaddr);
		}else{
			tp_in->nextlp = tp_in->lladdr;
		}
	}else if( tp_in->lsaddr!= NULL ){
		tp_in->nextlp = *(tp_in->lsaddr);
	}else{
		tp_in->nextlp = tp_in->lladdr;
	}

	return( 0 );
}
/*--------------------------------------------------------------------*/
/* ��� �f�[�^�^�̉�� */
/* �e�l�o�f�[�^�t�@�C���̃p�[�g���𔻒f���� */
/* �߂�l  0x20:�U�p�[�g  0x40:�P�P�p�[�g */
int analdatatype( char *cp_indata ){

	short *sp_offset;
	char *cp_cur;

	/* �R���p�C�����܂� */
	sp_offset = (short *)cp_indata;
	cp_cur = cp_indata;
	cp_cur += (int)(*sp_offset);
	/* �R���p�C���o�[�W�����܂ňړ� */
	cp_cur += 3;

	/* 1.01b �R�����g���̐擪�A�h���X���擾 */
	gcp_comment = cp_cur;
	gcp_comment++;
	if( *gcp_comment == 0x00 ){
		gcp_comment = NULL;
	}

	return( (int)(*cp_cur)&0xF0 );
}
/*--------------------------------------------------------------------*/
/* ��� �w�b�_�̓ǂݍ��� */
int readheader( char *cp_in ){

	/* �e�f�[�^�^�C�v�ɂ�����p�[�g�� */
	int is_partelem[]={6,11,-1,0};
	/* �e�f�[�^�^�C�v�ɂ�����p�[�g�^�C�v */
	int iss_parttype[4][12]={
		{ 0, 0, 0, 1, 1, 1,-1,-1,-1,-1,-1,-1},
		{ 0, 0, 0, 0, 0, 0, 1, 1, 1,-1, 2,-1},
		{-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1}
	};
	/* �e�f�[�^�^�C�v�ɂ�����p�[�g���̕ϊ���p�[�g�ԍ� */
	int iss_ppartnum[4][12]={
		{0,1,2,6,7,8,0,0,0,0,0,0},
		{0,1,2,3,4,5,6,7,8,0,9,0},
		{0,0,0,0,0,0,0,0,0,0,0,0}
	};
	int i_count;
	int i_offset;
	int i_loopoffset;
	int i_loop2offset;
	char *cp_cur;
	int i_ret;
	int i_datatype;

	i_ret = analdatatype( cp_in );
	/* �f�[�^�^�C�v�̐U�藎�Ƃ� */
	switch( i_ret ){
		case 0x20:
			/* �U�p�[�g */
			i_datatype = 0;
			break;
		case 0x40:
			/* �P�P�p�[�g */
			i_datatype = 1;
			break;
		default:
			sprintf(gcs_line," [F2P] BAD FMP Format.\n");
			return( DR_E_NSDTVERSION );
			hfpf( DH_BMESSAGES , gcs_line );
	}
	sprintf(gcs_line," [F2P] FMP Format is 0x%02X.\n",i_ret);
	hfpf( DH_BMESSAGES , gcs_line );

	/* �ǂݍ��݃f�[�^�̐擪�Q�o�C�g���� */
	cp_cur = (char *)(cp_in);
	cp_cur += 2;
	i_loop2offset = is_partelem[i_datatype] * 2;
	for(i_count=0;i_count<is_partelem[i_datatype];i_count++,cp_cur+=2){
		if( iss_parttype[i_datatype][i_count] == -1 ){
			/* ���Y�p�[�g�͎g�p���Ȃ��^�C�v */
			ts_tsf[i_count].use = D_OFF;
			continue;
		}
		/* �p�[�g�^�C�v�̃Z�b�g */
		ts_tsf[i_count].parttype = iss_parttype[i_datatype][i_count];
		ts_tsf[i_count].ppartnum = iss_ppartnum[i_datatype][i_count];
		/* �J�n�A�h���X�E���[�v�A�h���X�̎Z�o */
		i_offset = (int)((unsigned char)(*cp_cur));
		i_offset += ( 256 * (int)((unsigned char)(*(cp_cur+1))) );
		i_loopoffset = (int)((unsigned char)(*(cp_cur+i_loop2offset)));
		i_loopoffset += ( 256 *
			(int)((unsigned char)(*(cp_cur+1+i_loop2offset))) );
		if( i_offset == 0 || i_offset == 0x0FFFF ){
			/* ����Ȃ��Ƃ͂Ȃ��Ǝv�������Y�p�[�g���g�p */
			ts_tsf[i_count].use = D_OFF;
			continue;
		}
		ts_tsf[i_count].use = D_ON;
		ts_tsf[i_count].pdata = cp_in + i_offset;
		if( (char)(*(ts_tsf[i_count].pdata)) == (char)0x74 ){
			/* ���Y�p�[�g�̓f�[�^�Ȃ� */
			ts_tsf[i_count].use = D_OFF;
			sprintf(gcs_line," [F2P] Silent part:[%d]\n",i_count);
			hfpf( DH_BMESSAGES , gcs_line );
			continue;
		}
		if( i_loopoffset != 0x0FFFF ){
			/* ���Y�p�[�g�������[�v���� */
			ts_tsf[i_count].lladdr = cp_in + i_loopoffset;
		}
	}

	return( is_partelem[i_datatype] );
}
/*--------------------------------------------------------------------*/
/* ��� �w�b�_�̓ǂݍ���(�e�l���F) */
/* 1.01d 0x40�n��11�p�[�g�f�[�^�őS�R�_���Ȓp���������o�O���C�� */
int getFMparam( char *cp_in ){

typedef	struct	_t_voiceconv{
	unsigned char uc_number;
	char cs_data[DN_SIZE_VOICEFMP];
}T_VOICECONV;

typedef	struct	_t_dataheader{
	unsigned short us_nouse1;
	unsigned short us_partAaddr;
	unsigned short uss_nouse2[11];
	unsigned short us_tempo;
	char c_voicestart;
}T_DATAHEADER;

typedef	struct	_t_data40header{
	unsigned short us_nouse1;
	unsigned short us_partAaddr;
	unsigned short uss_nouse2[21];
	unsigned short us_tempo;
	unsigned short us_voicetermaddr;
	char c_voicestart;
}T_DATA40HEADER;

	T_DATAHEADER *tp_header;
	T_DATA40HEADER *tp_40header;
	T_VOICECONV *tp_pvoice;
	int i_count;
	unsigned short us_vsize;
	int i_voicenum;
	char *cp_rcur;
	char *cp_wcur;
	int i_datatype;
	int i_voffset;

	unsigned short us_startAaddr;
	unsigned short us_startVaddr;
	unsigned short us_endVaddr;

	gcp_FMparam = gcs_wbuf;
	cp_wcur = gcs_wbuf;
	for( i_count=0 ; i_count<256 ; i_count++){
		tp_pvoice = (T_VOICECONV *)cp_wcur;
		tp_pvoice->uc_number = (unsigned char)i_count;
		cp_wcur += DN_SIZE_VOICEPMD;
	}

	/* �p�[�g������ */
	i_datatype = analdatatype( cp_in );
fprintf(stderr,"TYPE:%X\n",i_datatype);
	if( i_datatype == 0x20 ){
		/* �U�p�[�g�^ */
		i_voffset = DN_VOFFSET_OPI;
	}else if( i_datatype == 0x40 ){
		/* �P�P�p�[�g�^ */
		i_voffset = DN_VOFFSET_OVI;
	}else{
		/* 1.01d�C�� �p�[�g���̊m�肪�o���Ȃ��ꍇ���� */
		return(-1);
	}

fprintf(stderr,"TYPE:%X/%d\n",i_datatype,i_voffset);

	/* �e�l���F���邩���� */
	tp_header = (T_DATAHEADER *)cp_in;
	tp_40header = (T_DATA40HEADER *)cp_in;
	if( i_datatype == 0x20 ){
		/* �U�p�[�g�^ */
		us_startVaddr = DN_VOFFSET_OPI;
	}else if( i_datatype == 0x40 ){
		/* �P�P�p�[�g�^ */
		us_startVaddr = DN_VOFFSET_OVI;
	}
fprintf(stderr,"ADDR:%X/%X\n",us_startVaddr,tp_header->us_partAaddr);
	if( us_startVaddr == tp_header->us_partAaddr ){
		/* �e�l���F�Ȃ� */
		return(0);
	}

	/* �e�l���F���擾 */
	/* �e�l���F�̈�n�_�E�I�[�擾 */
	if( i_datatype == 0x20 ){
		/* �U�p�[�g�^ */
		us_endVaddr = tp_header->us_partAaddr;
	}else if( i_datatype == 0x40 ){
		/* �P�P�p�[�g�^ */
		us_endVaddr = tp_40header->us_voicetermaddr;
	}
fprintf(stderr,"SIZE:%d/%d/%d\n",us_endVaddr,us_startVaddr,DN_SIZE_VOICEFMP);
	if( (us_endVaddr-us_startVaddr)%DN_SIZE_VOICEFMP != 0 ){
		/* �e�l���F�̈�T�C�Y���e�l���W�X�^�T�C�Y�ƈ�v���� */
		return(-2);
	}
	i_voicenum = ((int)(us_endVaddr-us_startVaddr))/DN_SIZE_VOICEFMP;
fprintf(stderr,"VOCES:%d\n",i_voicenum);

	cp_rcur = cp_in + ((int)(us_startVaddr));

	/* �e�l���F�R���o�[�g */
	cp_wcur = gcs_wbuf;
	for( i_count=0 ; i_count<i_voicenum ; i_count++){
		tp_pvoice = (T_VOICECONV *)cp_wcur;
		memcpy( tp_pvoice->cs_data , cp_rcur , DN_SIZE_VOICEFMP );
		tp_pvoice->uc_number = (unsigned char)i_count;
		cp_rcur += DN_SIZE_VOICEFMP;
		cp_wcur += DN_SIZE_VOICEPMD;
	}

	return( i_voicenum );
}
