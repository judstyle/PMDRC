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

#include "prd.h"
#include "prdext.h"




/*--------------------------------------------------------------------*/
/* ��̓��C�� */
T_OPIF *anlmain( T_OPIF *tp_indata ){

	int i_ret;
	int i_cnt;
	int i_cnt2;
	long l_beforeLcur;
	T_BON *tp_boncur;
	char *cp_infile;
	int i_maxloop=0;
	int i_maxtotal=0;
	char *cp_checkresult_l;
	char *cp_checkresult_t;
	int i_L_incoordination = D_OFF;

	/* ������ */
	memset(&gt_ginfo,0x00,sizeof(T_GINFO));
	gt_ginfo.l_mctotal=-1;
	gt_ginfo.l_mcloop=-1;
	gt_ginfo.i_nmemo=-1;
	for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
		for(i_cnt2=0;i_cnt2<(DN_MAX_NOTELENFREQ+1);i_cnt2++){
			iss_notelenfreq[DN_MAX_TRACKNUM][DN_MAX_NOTELENFREQ+1] = 0;
		}
	}
	/* �g���b�N������ */
	for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
		ts_ts[i_cnt].use = D_OFF;
	}
	/* �������ė��p�Ǘ��̈揉���� */
	for(i_cnt=0;i_cnt<256;i_cnt++){
		gtps_stockbon[i_cnt] = NULL;
	}

	/* �O���[�o���ϐ��֎�荞�� */
	gi_zenlen = tp_indata->i_zenlen;

	/* �w�b�_�ǂݎ��P */
	readheader( tp_indata->cp_indata );
	sprintf(gcs_line," [DLL] Succeed read header 1st.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* �w�b�_�ǂݎ��Q */
	if( (tp_indata->i_definemode)&DA_DEFL_NORMAL != 0 ){
		/* i_definemode��DA_DEFINEONLYPARTEXTEND�ł���ꍇ���l�� */
		readhead2( tp_indata->cp_indata );
		sprintf(gcs_line," [DLL] Succeed read header 2nd.\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
	}

	/* �g���b�N�����O �g���b�N�ǂݎ�� */
	for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
		if( ts_ts[i_cnt].use != D_ON ){ continue; }
		tp_curtra = &(ts_ts[i_cnt]);
		anltrack( );
	}
	sprintf(gcs_line," [DLL] Succeed read MML on tracks.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* �g���b�N�����P�E�Q�E�R�i�S�p�[�g�R���g���[���E�����g���b�N�j */
	for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
		if( ts_ts[i_cnt].use != D_ON ){ continue; }
		tp_curtra = &(ts_ts[i_cnt]);
		/* Z�R�}���h���}�X�^�[�g���b�N�ֈړ� */
		part2master( );

		/* notes-table�̏I�[(note1=0x80)�܂Ō������A */
		/* ����܂łɁA�������Ȃ��ꍇ�͖��������� */
		i_ret = delsilenttrack( );
		if( i_ret == D_OFF ){ continue; }

		/* �A�����Ă���x���͌q���� �^�C�Ōq�����Ă��铯�����͌q���� */
		combinenotes( i_cnt );
	}
	sprintf(gcs_line," [DLL] Succeed make master-track, Succeed combine notes.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );


	/* �g���b�N�����S�i�S�p�[�g�����g���b�N�j oct/�]���̗�����날�킹 */
	for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
		if( ts_ts[i_cnt].use != D_ON ){ continue; }
		tp_curtra = &(ts_ts[i_cnt]);
		editoct( );
	}
	sprintf(gcs_line," [DLL] Succeed edit octerves.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* �g���b�N�����T �񂹏W�߂������̃}�X�^�[�g���b�N���\�[�g */
	sortmaster( );
	sprintf(gcs_line," [DLL] Succeed sorting mastertrack.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );

	/* �g���b�N�����U ���ߐ����� */
	if( gtp_gop->i_barlevel != D_OFF ){
		i_ret = barcheck( );
		i_ret = analclocklist( i_ret );
//fprintf(stderr,"BARTYPE:%d\n",i_ret);
		sprintf(gcs_line," [DLL] Succeed analysis length of part.\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
		if( i_ret != D_BAR_MANUAL ){
			for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
				if( ts_ts[i_cnt].use != D_ON ){ continue; }
				tp_curtra = &(ts_ts[i_cnt]);
				dividebybar( i_ret );
			}
		}
		sprintf(gcs_line," [DLL] Succeed insert blank on the disregard-bar.\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
	}

	/* �V�[�P���V�����ɕ��ׂăe�L�X�g�o�� */
	for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
		if( ts_ts[i_cnt].use != D_ON ){ continue; }
		tp_curtra = &(ts_ts[i_cnt]);
		trackout( i_cnt );
	}
	sprintf(gcs_line," [DLL] Succeed analysis MML.\r\n");
	hfpf( DH_BMESSAGES , gcs_line );
	/* �T�[�r�X��� */

	/* �t�b�^����̂e�l���F��` */
	if( gtp_gop->i_mmllevel < 5 &&
		gtp_gop->i_voicemode != D_OFF ){
		voiceout();
	}

	/* DEFINE�\�� */
	if( gtp_gop->i_definemode != D_OFF ){
		defineout();
	}


	/* �T�[�r�X��� */
	sprintf(gcs_line," [DLL] Pa| front | Loop' | Total | data\n");
	hfpf( DH_BMESSAGES , gcs_line );
	sprintf(gcs_line," [DLL] rt| at 'L'| clock | clock |offset\n");
	hfpf( DH_BMESSAGES , gcs_line );
	sprintf(gcs_line," [DLL] --+-------+-------+-------+------\n");
	hfpf( DH_BMESSAGES , gcs_line );
	for(i_cnt=0;i_cnt<DN_MAX_TRACKNUM;i_cnt++){
		if( ts_ts[i_cnt].use != D_ON ){ continue; }
		tp_curtra = &(ts_ts[i_cnt]);
		sprintf(gcs_line," [DLL] %c | %5d | %5d | %5d | %04X\r\n",
			tp_curtra->partname,
			tp_curtra->tracklenbl,
			tp_curtra->tracklenal,
			tp_curtra->tracktotal,
			tp_curtra->text-gtp_gop->cp_indata);
		hfpf( DH_BMESSAGES , gcs_line );
		if( tp_curtra->tracklenal > i_maxloop ){
			i_maxloop = tp_curtra->tracklenal;
		}
		if( tp_curtra->tracktotal > i_maxtotal ){
			i_maxtotal = tp_curtra->tracktotal;
		}
	}

	/* MC.EXE�̋L�^�����N���b�N���̕\���iV4.8�ȍ~�̂݁j */
	if( gt_ginfo.c_varsion >= 0x48 ){
		sprintf(gcs_line," [DLL] --+-------+-------+-------+------\n");
		hfpf( DH_BMESSAGES , gcs_line );
		sprintf(gcs_line," [DLL]   |       | %5d | %5d |MC.EXE\n",
			gt_ginfo.l_mcloop , gt_ginfo.l_mctotal );
		hfpf( DH_BMESSAGES , gcs_line );
		if( i_maxloop == gt_ginfo.l_mcloop ){
			cp_checkresult_l = DS_OK;
		}else{
			cp_checkresult_l = DS_NG;
			i_L_incoordination = D_ON;
		}
		if( i_maxtotal == gt_ginfo.l_mctotal ){
			cp_checkresult_t = DS_OK;
		}else{
			cp_checkresult_t = DS_NG;
			i_L_incoordination = D_ON;
		}
		/* �����`�F�b�N�̌��ʂ��m�f�������ꍇ */
		if( i_L_incoordination == D_ON ){
			tp_indata->i_result = DR_W_BADMML;
		}
		sprintf(gcs_line," [DLL]   |       |    %s |    %s | Check\n",
			cp_checkresult_l , cp_checkresult_t );
		hfpf( DH_BMESSAGES , gcs_line );
	}

	sprintf(gcs_line," [DLL] --+-------+-------+-------+------\n");
	hfpf( DH_BMESSAGES , gcs_line );

	return( tp_indata );
}
/*--------------------------------------------------------------------*/
/* ��͌n */
/* �w�b�_�ǂݍ��ݏ����������Q�i#Title���j */
void readhead2( char *cp_infile ){

	char *cp_cur1;
	short *sp_cur1;
	long *lp_cur1;
	int i_count;
	int i_offset;

	if( gt_ginfo.cp_voice == NULL ){
		/* �{�C�X�f�[�^�����݂��Ȃ� = �o�[�W�����L�q�ӏ�������ł��� */
		return;
	}

	cp_cur1 = gt_ginfo.cp_voice;
	cp_cur1--;
	if( (unsigned char)(*cp_cur1) != (unsigned char)(0xFE) ){ return; }

	/* �o�[�W������� */
	cp_cur1--;
	gt_ginfo.c_varsion = *cp_cur1;
	sprintf(gcs_line," [DLL] MC.EXE Version is [%d.%d].\n",
		gt_ginfo.c_varsion/16 , gt_ginfo.c_varsion%16 );
	hfpf( DH_BMESSAGES , gcs_line );

	/* �e�L�X�g�i�[�A�h���X�Q�A�h���X */
	cp_cur1--;
	i_offset = ((int)(unsigned char)*(cp_cur1))*256;
	cp_cur1--;
	i_offset += ((int)(unsigned char)*(cp_cur1));
	if( i_offset == 0 ){
		/* �A�h���X�Q�I�t�Z�b�g���[�������擾�ł��� */
		return;
	}else{
		gt_ginfo.cp_dtaddr = cp_infile;
		gt_ginfo.cp_dtaddr += i_offset;
		gt_ginfo.cp_dtaddr++;
	}

	/* �A�h���X�Q�I�t�Z�b�g��̎擾 */
	sp_cur1 = (short *)(gt_ginfo.cp_dtaddr);
	/* ���[�v�����̎擾�iV4.8�ȍ~�̂݁j */
	if( gt_ginfo.c_varsion >= 0x48 ){
		cp_cur1 -= 8;
		lp_cur1 = (long *)(cp_cur1);
		gt_ginfo.l_mctotal = *lp_cur1;
		lp_cur1++;
		gt_ginfo.l_mcloop = *lp_cur1;
	}

	/* �e�e�L�X�g�̈���擾���� */
	/* �A���A�e�L�X�g�����[���̏ꍇ�A�|�C���^�ɂ�NULL�����Ă��� */
	if( gt_ginfo.c_varsion >= 0x48 ){
		/* PPZ�Ή��� = "PZINAME�����݂���" */
		gt_ginfo.cp_pziname = cp_infile + (int)(*sp_cur1) + 1;
		if( *(gt_ginfo.cp_pziname) == 0x00 ){ gt_ginfo.cp_pziname = NULL; }
		sp_cur1++;
	}
	gt_ginfo.cp_ppsname = cp_infile + (int)(*sp_cur1) + 1;
	if( *(gt_ginfo.cp_ppsname) == 0x00 ){ gt_ginfo.cp_ppsname = NULL; }
	sp_cur1++;
	gt_ginfo.cp_ppcname = cp_infile + (int)(*sp_cur1) + 1;
	if( *(gt_ginfo.cp_ppcname) == 0x00 ){ gt_ginfo.cp_ppcname = NULL; }
	sp_cur1++;
	gt_ginfo.cp_title = cp_infile + (int)(*sp_cur1) + 1;
	if( *(gt_ginfo.cp_title) == 0x00 ){ gt_ginfo.cp_title = NULL; }
	sp_cur1++;
	gt_ginfo.cp_composer = cp_infile + (int)(*sp_cur1) + 1;
	if( *(gt_ginfo.cp_composer) == 0x00 ){ gt_ginfo.cp_composer = NULL; }
	sp_cur1++;
	gt_ginfo.cp_arrenger = cp_infile + (int)(*sp_cur1) + 1;
	if( *(gt_ginfo.cp_arrenger) == 0x00 ){ gt_ginfo.cp_arrenger = NULL; }
	sp_cur1++;
	for( i_count=0 ; *sp_cur1!=0 ; sp_cur1++ , i_count++ ){
		gt_ginfo.cps_memo[i_count] = cp_infile + (int)(*sp_cur1) + 1;
		if( gt_ginfo.i_nmemo < 0 ){ gt_ginfo.i_nmemo=0; }
		gt_ginfo.i_nmemo++;
	}

	return;
}
/*--------------------------------------------------------------------*/
/* ��͌n */
/* �w�b�_�ǂݍ��ݏ����������P */
void readheader( char *cp_infile ){

	char *cp_cur;
	char *cp_tmp;
	int i_offset;
	int i_cnt;

	cp_cur = cp_infile;
	cp_cur++;
	/* part A-J */
	for(i_cnt=DN_START_NUM_FMSSG;i_cnt<=DN_START_NUM_ADPCM;i_cnt++){
		ts_ts[i_cnt].use = D_ON;
		i_offset = (int)((unsigned char)(*cp_cur));
		i_offset += ( 256 * (int)((unsigned char)(*(cp_cur+1))) );
		ts_ts[i_cnt].text = cp_infile;
		ts_ts[i_cnt].text += (1 + i_offset);
		ts_ts[i_cnt].partname = ((char)DC_START_CHAR_FMSSG+(char)i_cnt);
		if( i_cnt == 0){
			if( *cp_cur != 0x18 ){
				/* with on voice-datas */
				/* V1.01b �{�C�X�f�[�^���Ȃ��ꍇ������(�Â�mc.exe��) */
				if( *(cp_infile+1+0x18) == *(cp_infile+1+0x16) &&
					*(cp_infile+1+0x18+1) == *(cp_infile+1+0x16+1) ){
					/* �{�C�X�f�[�^�A�h���X���q�p�[�g�A�h���X�Ɠ����ꍇ */
					/* �{�C�X�f�[�^�͑��݂��Ȃ�(Ver4.5�ȑO?) */
				}else{
					cp_tmp = cp_infile + 1 + 0x18;
					i_offset = (int)((unsigned char)(*cp_tmp));
					i_offset += ( 256 * (int)((unsigned char)(*(cp_tmp+1))) );
					cp_voiceaddr = cp_infile + 1 + i_offset;
					gt_ginfo.cp_voice = cp_voiceaddr;
					sprintf(gcs_line," [DLL] Findout voice-datas.\n");
					hfpf( DH_BMESSAGES , gcs_line );
				}
			}
		}
		if( *(ts_ts[i_cnt].text) == (char)0x80 ){
			/* �����[���̃p�[�g */
			sprintf(gcs_line," [DLL] Part-%c no-datas.\n",
				ts_ts[i_cnt].partname);
			hfpf( DH_BMESSAGES , gcs_line );
			ts_ts[i_cnt].use = D_OFF;

		}else{
			/* ������������^���� */
			ts_ts[i_cnt].naddr = newbon(NULL);
			ts_ts[i_cnt].caddr = newboc(NULL);
		}

		cp_cur++;
		cp_cur++;
	}

	tp_BOMtop = newbom(NULL);
	gi_bomnum = 0;

	return;
}
/*--------------------------------------------------------------------*/
/* ��͌n */
/* �g���b�N��́i�V�[�P���V�����j */
void anltrack( ){

	char *cp_cur;
	int i_steps;
	int i_ret;
	long l_ret;
	unsigned char uc_ret;
	T_BON *tp_boncur;
	T_BOC *tp_boccur;
	long l_clock;
	T_TCODE *tp_cur;
	int i_lasttype=-1;
	long l_nextaddclock;

	/* B.O.Addresses (�ŏ��̈�͎g��Ȃ�) */
	tp_boncur = tp_curtra->naddr;
	tp_boncur++;
	tp_boccur = tp_curtra->caddr;
	tp_boccur++;
	l_clock = 0;


	tracklen( D_OP_TLINIT , 0 );
	for(cp_cur=tp_curtra->text;;){
		l_nextaddclock = 0;
		if( *cp_cur == (char)0x80 ){
			/* �g���b�N�I������ */
			if( (gtp_gop->i_dumpmode & DN_DL_SUPERDUMP) != 0 ){
				/* �X�[�p�[�_���v�I������ */
				dumps1op( NULL );
			}
			if( (gtp_gop->i_dumpmode & DN_DL_FRAMEDUMP) != 0 ){
				/* �_���v�I������ */
				dump1op( NULL );
			}
			tp_boncur->toffset = l_clock;
			tp_boncur->note1 = *cp_cur;
			tp_boccur->toffset = l_clock;
			tp_boccur->code = *cp_cur;
			tp_curtra->partlen = (int)tracklen( D_OP_TLTOTAL , 0 );
			tp_curtra->tracklenbl = (int)tracklen( D_OP_TLBEFOREL , 0 );
			tp_curtra->tracktotal = tp_curtra->partlen;
			if( tp_curtra->tracklenbl >= 0 ){
				tp_curtra->tracklenal =
					tp_curtra->tracktotal - tp_curtra->tracklenbl;
			}else{
				tp_curtra->tracklenal = -1;
			}
			break;
		}
		/* �R�[�h�T�[�` */
		tp_cur = codefinderbystr( cp_cur );
		if( tp_cur == NULL ){
			sprintf(gcs_line,"E[DLL] Syntax error(unknown code). [%c:%02X]\r\n",
				tp_curtra->partname,*cp_cur);
			hfpf( DH_BMESSAGES , gcs_line );
			exit(-1);
		}
		/* �f�[�^���̊m�� */
		i_steps = tp_cur->datasize + 1;

		/* �S�̂̋L�^ */
		switch( (int)((unsigned char)(*cp_cur)) ){
			case 0xF6:
				/* 'L' */
				gt_ginfo.i_Lcount++;
				break;
			case 0xD2:
				/* 'F' */
				gt_ginfo.i_Fcount++;
				break;
			default:
				break;
		}

		/* �����ɗ����ꍇ�͖ⓚ���p�Ŏ擾���� */
		if( tp_cur->table == 1 ){
			/* �R�[�h�T�[�` table1(=note)�i�[���� */
			/* notes or control code (���K�����F�|���^�����g�^�]��) */
			i_lasttype = 1;
			findnote( cp_cur , tp_boncur , l_clock );
			if( tp_cur->steps == D_ON ){
				/* ���������ꍇ�̂� */
				l_clock = tp_boncur->toffset + tp_boncur->length;
				/* �����̌v�� */
				l_nextaddclock = (long)tp_boncur->length;
			}
			tp_boncur = tp_boncur->newarea;
		}else if( tp_cur->table == 0 ){
			/* �R�[�h�T�[�` table1(=cc)�i�[���� */
			/* control code (���K�E�����������Ȃ�) */
			i_ret = findcontrol( cp_cur , tp_boccur , l_clock );
			if( i_ret == 0 ){
				tp_boccur = tp_boccur->newarea;

				/* �㑱�Ƃ̃R���o�C�����֎~���� */
				if( i_lasttype == 0 ){
					tp_boccur->befarea->nocombine = D_ON;
				}else if( i_lasttype == 1 ){
					tp_boncur->befarea->nocombine = D_ON;
				}
				i_lasttype = 0;

				/* [:] ���[�v�̏ꍇ�͂��̋�Ԃ���Z���鏈�� */
				if( *cp_cur == (char)0xF9 ){
					/* �n�_ */
					tracklen( D_OP_TLLSTART , 0 );
				}else if( *cp_cur == (char)0xF8 ){
					/* �I�_ */
					uc_ret = (unsigned char)(*(cp_cur+1));
					tracklen( D_OP_TLLEND , (long)(uc_ret) );
				}else if( *cp_cur == (char)0xF6 ){
					/* L */
					tracklen( D_OP_TLFINDL , 0 );
				}else if( *cp_cur == (char)0xF7 ){
					/* : */
					tracklen( D_OP_TLFINDCL , 0 );
				}
			}else if( i_ret > 0 ){
				/* ��������R�[�h�������ꍇ */
				/* �Ȃɂ����Ȃ� */
			}else if( i_ret < 0 ){
				/* ����ȂƂ���ɉ����𔭌����Ă��܂����ꍇ�����W�b�N�� */
				sprintf(gcs_line,"E[DLL] Bad sequence.\r\n");
				hfpf( DH_BMESSAGES , gcs_line );
				exit(-1);
			}
		}else{
			/* error */
			sprintf(gcs_line,"E[DLL] Syntax error(unknown code).\r\n");
			hfpf( DH_BMESSAGES , gcs_line );
			exit(-1);
		}

		/* �_���v�E�X�[�p�[�_���v */
		if( (gtp_gop->i_dumpmode & DN_DL_SUPERDUMP) != 0 ){
			dumps1op( cp_cur );
		}
		if( (gtp_gop->i_dumpmode & DN_DL_FRAMEDUMP) != 0 ){
			dump1op( cp_cur );
		}

		/* �o�߃N���b�N�̉��Z�͂����� */
		tracklen( D_OP_TLADD , l_nextaddclock );

		cp_cur += i_steps;
	}
	return;
}
/*--------------------------------------------------------------------*/
/* ��͌n */
/* �g���b�N����(control-track����master-track�ֈړ�) */
void part2master( ){

	int i_steps;
	int i_ret;
	T_BOM *tp_bomcur;
	T_BOC *tp_boccur;
	long l_clock;
	T_TCODE *tp_cur;

	/* B.O.Addresses (�ŏ��̈�͎g��Ȃ�) */
	tp_bomcur = tp_BOMtop;
	tp_bomcur++;
	tp_boccur = tp_curtra->caddr;
	tp_boccur++;

	for(;tp_boccur->code != (char)0x80;){
		if( (unsigned char)(tp_boccur->code) == (unsigned char)0xDF ){
			/* Z ���� */
			tp_bomcur->toffset = tp_boccur->toffset;
			tp_bomcur->barlen = tp_boccur->value;
			tp_bomcur->code = tp_boccur->code;
			tp_bomcur = tp_bomcur->newarea;
			gi_bomnum++;
			/* control-table �� Z ������ */
			tp_boccur->newarea->befarea = tp_boccur->befarea;
			tp_boccur->befarea->newarea = tp_boccur->newarea;
		}
		tp_boccur = tp_boccur->newarea;
		/* �I�[�ɓ��B���Ă���ꍇ�͎��̃G���A���m�� */
		if( tp_bomcur->newarea == NULL ){ newbom( tp_bomcur ); }
	}

	return;
}
/*--------------------------------------------------------------------*/
/* ��͌n */
/* �g���b�N����(silent-track��delete) */
/* �ŏ��̉��������I�u�W�F�N�g���x���������ꍇ�A���ł�oct�����킹��*/
int delsilenttrack( ){

	T_BON *tp_boncur;
	T_BON *tp_bon1r;
	int i_firstflag;

	i_firstflag = D_OFF;
	/* B.O.Addresses (�ŏ��̈�͎g��Ȃ�) */
	tp_boncur = tp_curtra->naddr;
	tp_boncur++;

	for(;tp_boncur->note1!=(char)0x80;){
		if( ((char)(tp_boncur->note1)&0x80) == 0 &&
			((char)(tp_boncur->note1)&0x0F) != 0x0F ){
			if( i_firstflag == D_ON ){
				tp_bon1r->oct = tp_boncur->oct;
				tp_curtra->startoct = tp_boncur->oct;
				(tp_curtra->naddr)->oct = tp_boncur->oct;
				return( D_ON );
			}else{
				return( D_ON );
			}
		}else if( i_firstflag != D_ON &&
			((char)(tp_boncur->note1)&0x0F) == 0x0F ){
			i_firstflag = D_ON;
			tp_bon1r = tp_boncur;
		}
		tp_boncur = tp_boncur->newarea;
	}

	sprintf(gcs_line," [DLL] Silent part:%c.\r\n",tp_curtra->partname);
	hfpf( DH_BMESSAGES , gcs_line );
	tp_curtra->use = D_OFF;
	return(D_OFF);
}
/*--------------------------------------------------------------------*/
/* ��͌n */
/* �g���b�N����(�^�C�Ō��΂ꂽ����m�[�g�����E�x�����m����x�q����) */
/* �܂��A�ŏ��̉��������������_�ŁA�擪�܂ők����oct�������߂� */
int combinenotes( int i_partno ){

	T_BON *tp_boncur;
	T_BON *tp_bonrev;
	int i_firstnote;

	/* B.O.Addresses (�ŏ��̈�͎g��Ȃ�) */
	tp_boncur = tp_curtra->naddr;
	tp_boncur++;

	i_firstnote=D_OFF;

	for(;tp_boncur->note1!=(char)0x80;){
		if( tp_boncur->note1 > 0 &&
			(char)(tp_boncur->note1&0x0F) != (char)0x0F &&
			i_firstnote == D_OFF ){
			/* �p�[�g���ōŏ��̉����������ꍇ */
			/* �ŏ��̉�����o�l������ */
			tp_curtra->startoct = 1+(0x70&tp_boncur->note1)/0x10;
			/* ����ȑO��bon�ɂ�����o�l����v������K�v������ */
			for(tp_bonrev=tp_boncur->befarea
				;tp_bonrev!=NULL
				;tp_bonrev=tp_bonrev->befarea){
				/* boncur��k��Ȃ���oct���t�����ɉ��� */
				tp_bonrev->oct = tp_curtra->startoct;
			}
			i_firstnote = D_ON;
		}
		if( tp_boncur->nocombine == D_ON ){
			/* �R���o�C���֎~ */
			/* �������̏W�v�i�W�����̎Z�o�Ɏg�p�j v1.01 */
			if( tp_boncur->length <= DN_MAX_NOTELENFREQ ){
				iss_notelenfreq[i_partno][(int)tp_boncur->length]++;
			}
			tp_boncur = tp_boncur->newarea;
			continue;
		}
		if( tp_boncur->note1 > 0 ){
			if( (char)(tp_boncur->note1&0x0F) == (char)0x0F ){
				/* �x���̏ꍇ */
				if( tp_boncur->newarea->note1 > 0 &&
					(char)(tp_boncur->newarea->note1) == (char)0x0F ){
					/* �����ċx�� */
					tp_boncur->length =
						tp_boncur->length + tp_boncur->newarea->length;
					delbon( tp_boncur->newarea );
					/* ���̎��ƌq����\��������̂Ń|�C���^�i�߂�������x */
					continue;
				}
			}else{
				/* �����̏ꍇ */
				if( (char)(tp_boncur->newarea->note1) == (char)0xFB &&
					( (char)(tp_boncur->newarea->newarea->note1) ==
						(char)(tp_boncur->note1) ||
					(char)(tp_boncur->newarea->newarea->note1) ==
						(char)0x0F ) ){
					/* �^�C�Ōq����ē��������o�Ă��邩�x���Ɍq����ꍇ */
					tp_boncur->length = tp_boncur->length +
						tp_boncur->newarea->newarea->length;
					delbon( tp_boncur->newarea );
					delbon( tp_boncur->newarea );
					/* ���̎��ƌq����\��������̂Ń|�C���^�i�߂�������x */
					continue;
				}
			}
		}

		/* �������̏W�v�i�W�����̎Z�o�Ɏg�p�j v1.01 */
		if( tp_boncur->length <= DN_MAX_NOTELENFREQ ){
			iss_notelenfreq[i_partno][(int)tp_boncur->length]++;
		}

		tp_boncur = tp_boncur->newarea;
	}
	return(0);
}
/*--------------------------------------------------------------------*/
/* ��͌n */
/* oct���덇�킹 */
void editoct(){

	T_BON *tp_boncur;
	int i_firstnote=D_OFF;
	int i_ctranso;
	int i_ctransn;

	/* B.O.Addresses (�ŏ��̈�͎g��Ȃ�) */
	tp_boncur = tp_curtra->naddr;
	tp_boncur++;

	for(;tp_boncur->note1!=(char)0x80;){
		if( (char)(tp_boncur->note1&0x0F) == (char)0x0F ||
			tp_boncur->note1 < 0 ){
			/* �x���܂��̓R���g���[���R�[�h�̏ꍇ */
			tp_boncur->oct = tp_boncur->befarea->oct;
		}
		tp_boncur = tp_boncur->newarea;
	}

	/* V1.21 2008/7/14 start �]���������ɔ��f������ */
	if( gtp_gop->c_autotrans != D_ON ){ return; }

	tp_boncur = tp_curtra->naddr;
	tp_boncur++;
	for(;tp_boncur->note1!=(char)0x80;){
		if( (char)(tp_boncur->note1) > 0 &&
			(char)(tp_boncur->note1&0x0F) != (char)0x0F  ){
			/* �x���ȊO�ł���ꍇ */
			i_ctranso = tp_boncur->trans / 12;
			i_ctransn = tp_boncur->trans % 12;
			tp_boncur->oct += i_ctranso;
			if( (tp_boncur->note1&0x0F)+i_ctransn > 0x0b ){
				/* �I�N�^�[�uup�Ȃ̂�+4 */
				tp_boncur->note1 += (i_ctransn+4);
				tp_boncur->oct++;
			}else if( (tp_boncur->note1&0x0F)+i_ctransn < 0x00 ){
				/* �I�N�^�[�udown�Ȃ̂�-4 */
				tp_boncur->note1 += (i_ctransn-4);
				tp_boncur->oct--;
			}else{
				tp_boncur->note1 += i_ctransn;
			}
			if( tp_boncur->note2 != 0x00 ){
				if( (tp_boncur->note2&0x0F)+i_ctransn > 0x0b ){
					/* �I�N�^�[�uup�Ȃ̂�+4 */
					tp_boncur->note2 += (i_ctransn+4);
				}else if( (tp_boncur->note2&0x0F)+i_ctransn < 0x01 ){
					/* �I�N�^�[�udown�Ȃ̂�-4 */
					tp_boncur->note2 += (i_ctransn-4);
				}else{
					tp_boncur->note2 += i_ctransn;
				}
			}
		}
		tp_boncur = tp_boncur->newarea;
	}
	/* V1.21 2008/7/14 end */

	return;
}
/*--------------------------------------------------------------------*/
/* ��͌n */
/* �g���b�N����(master-track��sort ���łɏd�����Ă���Z�𐮗�) */
int sortmaster( ){

	char cp_cur;
	int i_cnt;
	int i_steps;
	int i_ret;
	T_BOM *tp_bomcur;
	T_BOM *tp_lastZbomcur;
	long l_lastZclock=0;
	char c_barlencur=96;
	int i_firstZ=D_OFF;
	int i_Znotonline=D_OFF;
	long l_befsectionlen;

	/* �V�[�P���X���ɐ��� */
	for(i_steps=gi_bomnum-1;i_steps>0;i_steps--){
		tp_bomcur = tp_BOMtop;
		tp_bomcur = tp_bomcur->newarea;
		for(i_cnt=0;i_cnt<i_steps;i_cnt++){
			if( tp_bomcur->toffset > tp_bomcur->newarea->toffset ){
				tp_bomcur->newarea->newarea->befarea = tp_bomcur;
				tp_bomcur->befarea->newarea = tp_bomcur->newarea;
				tp_bomcur->newarea->befarea = tp_bomcur->befarea;
				tp_bomcur->befarea = tp_bomcur->newarea;
				tp_bomcur->newarea = tp_bomcur->befarea->newarea;
				tp_bomcur->befarea->newarea = tp_bomcur;
				tp_bomcur = tp_bomcur->newarea;
			}else{
				tp_bomcur = tp_bomcur->newarea;
			}
		}
	}

	/* �R���g���[���g���b�N�̏I�_�𖾎� */
	tp_bomcur = tp_BOMtop;
	tp_bomcur++;
	tp_bomcur+=gi_bomnum;
	tp_bomcur->last = D_ON;

	/* �A������A���l���邢�͓����̃R���g���[���R�}���h�𐮗� */
	tp_bomcur = tp_BOMtop;
	tp_bomcur++;
	for(;tp_bomcur->last!=D_ON;){
		if( (unsigned char)(tp_bomcur->code) == (unsigned char)0xDF ){
			/* Z�̏ꍇ */
			if( i_firstZ == D_OFF ){
				/* �ŏ��̈�� */
				i_firstZ = D_ON;
			}else if( tp_bomcur->barlen == (unsigned char)c_barlencur ){
				/* ���ݓK�p���̂��̂Ɠ��꒷��barlen���P���ɏ��� */
				tp_bomcur->newarea->befarea = tp_bomcur->befarea;
				tp_bomcur->befarea->newarea = tp_bomcur->newarea;
				tp_bomcur = tp_bomcur->newarea;
				continue;
			}else if( l_lastZclock != 0 &&
					tp_bomcur->toffset == l_lastZclock ){
				/* �O��w�莞�Ɠ���O���b�h���barlen���Â��ق������� */
				/* �Â��ق������������F */
				/*   �������w��̏ꍇ�A���̃g���b�N���D�悳���K�� */
				tp_lastZbomcur->befarea->newarea = tp_lastZbomcur->newarea;
				tp_lastZbomcur->newarea->befarea = tp_lastZbomcur->befarea;
				gt_ginfo.i_Zcount--;
			}
			/* �y�R�}���h�����ߐ���ɑ��݂��邩�ǂ����̃`�F�b�N */
			l_befsectionlen = tp_bomcur->toffset - l_lastZclock;
			if( l_befsectionlen % (long)((unsigned char)(c_barlencur)) != 0){
				/* ���݂̂y�R�}���h�����ߐ���łȂ��Ƃ���ɑ��݂��� */
				i_Znotonline = D_ON;
			}
			/* ����̋L�^ */
			tp_lastZbomcur = tp_bomcur;
			l_lastZclock = tp_bomcur->toffset;
			c_barlencur = tp_bomcur->barlen;
			gt_ginfo.i_Zcount++;
		}
		tp_bomcur = tp_bomcur->newarea;
	}

	gt_ginfo.i_Znotonline = i_Znotonline;
	return( i_Znotonline );
}
/*--------------------------------------------------------------------*/
/* ��͌n */
/* �y���l���������ߔ��� -1:no 0:Z�� 1-:�w�萔�l */
int barcheck( ){

	T_BOM *tp_bomcur;

	if( gtp_gop->i_barlevel == D_OFF ){
		/* ���ߋ�؂肵�Ȃ� */
		return( -1 );
	}else if( gtp_gop->i_barlevel == DA_FORCED ){
		/* �����I�Ɏ��{ */
		if( gtp_gop->i_zenlen > 0 ){
			return( gtp_gop->i_zenlen );
		}else{
			return( -1 );
		}
	}

	if( gt_ginfo.i_Zcount <= 0 ){
		/* ���ߒ��w�肪�Ȃ� */
		if( gt_ginfo.i_ILLcount > 0 ){
			/* �Ǐ��������[�v�����遨����s�\ */
			return( -1 );
		}
		if( gt_ginfo.i_Fcount > 0 || gt_ginfo.i_Lcount <= 0 ){
			/* �t�F�[�h�A�E�g������ or ���[�v���Ȃ� �����[�v���Ȃ��� */
			return( gtp_gop->i_zenlen );
		}else{
			/* ���[�v����� */
			return( gtp_gop->i_zenlen );
		}
	}else{
		if( gt_ginfo.i_Znotonline == D_OFF ){
			/* ���ߒ��w�肪�S�ď��ߐ���ɑ��� */
			tp_bomcur = tp_BOMtop;
			tp_bomcur++;
			for(;tp_bomcur->last!=D_ON;tp_bomcur=tp_bomcur->newarea){
				
			}
			return( 0 );
		}else{
			/* ���ߐ���łȂ��Ƃ���ɏ��ߒ������� */
			return( -1 );
		}
	}

	return( -1 );
}
/*--------------------------------------------------------------------*/
/* ��͌n */
/* �N���b�N�E���ߔ��� */
int analclocklist( int i_bartype ){

	int i_cnt1;
	int i_cnt2;
	long ls_plen[DN_MAX_TRACKNUM];
	int i_plennum=0;
	long ls_blen[DN_MAX_TRACKNUM];
	int i_blennum=0;
	int i_flag1;
	int i_flag2;
	long l_maxlen;

	long l_maxvalue;
	long l_tmp;
	int i_loopcount;

	/* ���[�v�㒷�� */
	for(i_cnt1=0;i_cnt1<DN_MAX_TRACKNUM;i_cnt1++){ ls_plen[i_cnt1] = 0; }
	for(i_cnt1=0;i_cnt1<DN_MAX_TRACKNUM;i_cnt1++){
		if( ts_ts[i_cnt1].use != D_ON ){ continue; }
		for(i_cnt2=0;i_cnt2<DN_MAX_TRACKNUM;i_cnt2++){
			if( ls_plen[i_cnt2] == ts_ts[i_cnt1].tracklenal ){
				/* �ߋ��ɋL�^����partlength�Ɠ������ꍇ�� */
				break;
			}
			/* ���߂ďo������partlength�͋L�^���� */
			if( ls_plen[i_cnt2] == 0){
				ls_plen[i_cnt2] = ts_ts[i_cnt1].tracklenal;
				break;
			}
		}
	}
	/* ���[�v�O���� */
	for(i_cnt1=0;i_cnt1<DN_MAX_TRACKNUM;i_cnt1++){ ls_blen[i_cnt1] = 0; }
	for(i_cnt1=0;i_cnt1<DN_MAX_TRACKNUM;i_cnt1++){
		if( ts_ts[i_cnt1].use != D_ON ){ continue; }
		for(i_cnt2=0;i_cnt2<DN_MAX_TRACKNUM;i_cnt2++){
			if( ls_blen[i_cnt2] == ts_ts[i_cnt1].tracklenbl ){
				/* �ߋ��ɋL�^����partlength�Ɠ������ꍇ�� */
				break;
			}
			/* ���߂ďo������partlength�͋L�^���� */
			if( ls_blen[i_cnt2] == 0){
				ls_blen[i_cnt2] = ts_ts[i_cnt1].tracklenbl;
				break;
			}
		}
	}

	/* ���[�v�㒷�����X�g�̓\�[�g���� -1 �� �� ���� */
	for(i_loopcount=0;i_loopcount<DN_MAX_TRACKNUM;i_loopcount++){
		for(i_cnt1=0;ls_plen[i_cnt1]!=0&&ls_plen[i_cnt1+1]!=0;i_cnt1++){
			if( ls_plen[i_cnt1+1] == -1 ||
				ls_plen[i_cnt1] > ls_plen[i_cnt1+1] ){
				l_tmp = ls_plen[i_cnt1];
				ls_plen[i_cnt1] = ls_plen[i_cnt1+1];
				ls_plen[i_cnt1+1] = l_tmp;
			}
		}
	}

	/* ���ߐ������̋N�_�̎w�肪����ꍇ */
	switch( gtp_gop->i_autothresh ){
		case 2:
			/* �f�[�^�擪�ɌŒ� */
			return(D_BAR_TOPSTART);
		default:
			break;
	}

	/* ���[�v���Ă���A�S�p�[�g�����zenlen�̔{�� */
	if( ls_plen[1] == 0 && ls_plen[0] % gi_zenlen == 0 ){
		return(D_BAR_FULLAUTO1);
	}

	/* L �����݂��Ă��Ȃ��p�[�g�ȊO�͒��������� */
	if( ls_plen[2]==0&&ls_plen[0]==-1 ){
		return(D_BAR_FULLAUTO1);
	}

	/* L �����݂��Ă���p�[�g�͒������S��zenlen�̔{���A */
	/* ���ő�̂��̂����S�Ẵp�[�g�̍ŏ����{�� */
	i_flag1 = D_OFF;
	i_flag2 = D_ON;
	l_maxlen = 0;
	for(i_cnt1=0;ls_plen[i_cnt1]!=0;i_cnt1++){
		if( ls_plen[i_cnt1] % gi_zenlen != 0 ){ i_flag1=D_ON; }
		if( ls_plen[i_cnt1] > l_maxlen ){ l_maxlen = ls_plen[i_cnt1]; }
	}
	if( i_flag1 == D_OFF && l_maxlen > 0 ){
		/* zenlen�̔{���Ƃ����_�̓N���A */
		i_flag2 = D_OFF;
		for(i_cnt1=0;ls_plen[i_cnt1]!=0;i_cnt1++){
			if( ls_plen[i_cnt1] == -1 ){ continue; }
			if( l_maxlen % ls_plen[i_cnt1] != 0 ){
				i_flag2 = D_ON;
			}
		}
	}
	if( i_flag1 == D_OFF && i_flag2 == D_OFF ){
		/* �����Ƃ��N���A���Ă���̂ŁAFULLAUTO�ɏ����� */
		return(D_BAR_FULLAUTO1);
	}



	/* �� */
	return(D_BAR_MANUAL);
}
/*--------------------------------------------------------------------*/
/* ��͌n */
/* ���ߐ����ׂ��ł��鉹���𕪊����A���ߐ��i�󔒂ŕ\���j��}�� */
/* �P��̏����łP�p�[�g�̂ݎ��{ */
void dividebybar( int i_type ){

	T_BOC *tp_boccur;
	T_BON *tp_boncur;
	T_BON *tp_bonadd;
	long l_curcnt=0;
	long l_befcnt=0;
	long l_nxtcnt=0;
	long l_toffset;
	long l_coffset;
	long l_curpoint=0;
	long l_curbar=0;
	int i_ret;
	long l_NGlevel=-1;
	int i_updateclock=D_ON;

	/* B.O.Addresses (�ŏ��̈�͎g��Ȃ�) */
	tp_boccur = tp_curtra->caddr;
	tp_boccur++;
	tp_boncur = tp_curtra->naddr;
	tp_boncur++;

	/* �������Ă��Ȃ���̓��[�`��TYPE�͒��˕Ԃ� */
	switch(i_type){
		case D_BAR_TOPSTART:
			break;
		case D_BAR_FULLAUTO1:
			break;
		case D_BAR_FORCEDAUTO:
			break;
		default:
			return;
	}

	tracklen(D_OP_TLINIT,0);
	/* D_BAR_FULLAUTO1 */
	/* L���Ȃ��ꍇ�͌��ʉ��p�[�g�Ƃ��Ė��� */
	if( tp_curtra->tracklenbl < 0 ){ return; }
	/* �I�t�Z�b�g�Z�o */
	if( i_type == D_BAR_TOPSTART ){
		l_toffset = gtp_gop->i_manoffset;
	}else{
		l_toffset = ( tp_curtra->tracklenbl + gtp_gop->i_manoffset ) %
			gi_zenlen;
	}
	/* note/cc�̃e�[�u���𑍟��� */
	while(tp_boncur->newarea!=NULL||tp_boccur->newarea!=NULL){
		/* notes�g���b�N�I���ŏ��ߐ��}���������̂��I�� */
		if( (char)tp_boncur->note1 == (char)0x80 ){
			break;
		}
		/* ����pointer���`�F�b�N */

		/*------------------------------------------------------------*/
		/* �V���[�`�� */
		/* ����pointer��cc/notes��ǂ��z���Ă͂����Ȃ��O�� */
		/* �����l�̋L�^���x����-1 */
		/* TLLEVEL TLTLEVEL��V�݂��� */

		/* "["�Ń��x�����オ������ATLLSTART�̂��Ƃ�clock������ */
		/* clock�����ߐ�����Y���Ă�����ATLLEVEL�߂�l�̃��x�����L�^���� */
		/* (TLLEVEL�́A���݂̃��x����Ԃ�) */

		/* "]"�Ń��x��������������ATLEND�̂���TLLEVEL������ */
		/* �߂�l���L�^���Ă��郌�x�����Ⴍ�Ȃ�����A */
		/* ���̃��x��������(-1)���� �����łȂ���Ε��u */

		/* ���ߐ�����Ɏg���̂�TLTLEVEL���g�� */
		/* (TLTLEVEL�́A���݂̃��x���ɂȂ��Ă����clock��Ԃ�) */
		/* �A���ATLLEVEL���[���̂Ƃ�������TLTOTAL���g�� */
		/* ���ߐ����莞�A-1�Ȃ疳������TLTOTAL��M���� */
		/* �����łȂ���΁A�����Ă���̂ŐM�p���Ȃ� */
		/*------------------------------------------------------------*/

		/* cc��notes�ǂ��炪��ɂ��邩����(�����toffset�̔�r�łł���) */
		/* ���Ȃ݂ɁAcc��notes����ɗ��邱�Ƃ͂��肦�Ȃ��̂� */
		/* �p�^�[���́unotes���撅�v���u�����v�����Ȃ� */
		if( tp_boncur->toffset == tp_boccur->toffset ){
			/* �����ł������ꍇ */
			if( (char)tp_boccur->code == (char)0xF9 ){
				/* �n�_ */
				/* ���x�����オ�����̂ŃY�����Ȃ����`�F�b�N���� */
				i_ret = tracklen( D_OP_TLLEVEL , 0 );
				if( i_ret == 0 ){
					/* ���x���[���̏ꍇ�̓I�t�Z�b�g���l������TOTAL������ */
					i_ret = tracklen( D_OP_TLTOTAL , 0 );
					if( i_ret % gi_zenlen != l_toffset ){
						/* �Y���Ă��� */
						l_NGlevel = 1;
					}
				}else{
					/* ���x���P�ȏ�̏ꍇ�̓I�t�Z�b�g�𖳎���TLTLEVEL������ */
					i_ret = tracklen( D_OP_TLTLEVEL , 0 );
					if( i_ret % gi_zenlen != 0 ){
						/* �Y���Ă��� */
						l_NGlevel = tracklen( D_OP_TLLEVEL , 0 ) + 1;
					}
				}
				/* ���x�����グ�� */
				tracklen( D_OP_TLLSTART , 0 );
			}else if( (char)tp_boccur->code == (char)0xF8 ){
				/* �I�_ */
				/* ���x�������������̂ŃY�����Ȃ��Ƃ���܂Ŗ߂��Ă��邩 */
				i_ret = tracklen( D_OP_TLLEVEL , 0 );
				if( l_NGlevel == i_ret ){
					/* �Y���Ă��Ȃ����x���܂Ŗ߂��Ă��� */
					l_NGlevel = -1;
				}
				tracklen( D_OP_TLLEND , (char)tp_boccur->value );
			}else if( (char)tp_boccur->code == (char)0xF7 ){
				/* : */
				tracklen( D_OP_TLFINDCL , 0 );
			}else{
				/* ���x�����ς��Ȃ� */
			}

			/* clock�M�����`�F�b�N */
			if( l_NGlevel != -1 ){
				/* clock�M�p�ł��Ȃ���� */
				/* ����cc���ڂ܂Ői�� */
				tp_boccur = tp_boccur->newarea;
				continue;
			}

			/* �܂����̎��_�œ����Ȃ̂ŁA�`�F�b�N����OK�������珬�ߐ����� */
			if( 0 == tracklen( D_OP_TLLEVEL , 0 ) ){
				/* ���x���[�� */
				i_ret = tracklen( D_OP_TLTOTAL , 0 );
				if( i_ret % gi_zenlen == l_toffset ){
					/* ���܂��ɏ��ߐ���ɂ���̂ŏ��ߐ����� */
					tp_boccur->barline[0] = 0x20;
					tp_boccur->barline[1] = 0x0;
					/* ��clock�ɂȂ��ł���CC�͑S�ăp�X���邽�߁A */
					/* �u���ߐ����瓮������D_ON�ɂȂ�t���O��D_OFF�ɂ��� */
					/* �������A���[�v�I���L���̒���͏��ߐ��������� */
					if( tp_boccur->code != (char)0xF8 ){
						i_updateclock = D_OFF;
					}
				}
			}else{
				/* ���x���P�ȏ� */
				i_ret = tracklen( D_OP_TLTLEVEL , 0 );
				if( i_ret % gi_zenlen == 0 ){
					/* ���܂��ɏ��ߐ���ɂ���̂ŏ��ߐ����� */
					tp_boccur->barline[0] = 0x20;
					tp_boccur->barline[1] = 0x0;
					/* ��clock�ɂȂ��ł���CC�͑S�ăp�X���邽�߁A */
					/* �u���ߐ����瓮������D_ON�ɂȂ�t���O��D_OFF�ɂ��� */
					/* �������A���[�v�I���L���̒���͏��ߐ��������� */
					if( tp_boccur->code != (char)0xF8 ){
						i_updateclock = D_OFF;
					}
				}
			}
			/* ] �L���̎�O�ɂ͏��ߐ��͒u���Ȃ� */
			if( tp_boccur->code == (char)0xF8 ){
				tp_boccur->barline[0] = 0;
			}
			/* ����cc���ڂ܂Ői�� */
			tp_boccur = tp_boccur->newarea;
			continue;
		}else{
			/* �����ɗ��鎞�_�ŁAcc����notes���撅���Ă��� */

			/* clock�M�����`�F�b�N */
			if( l_NGlevel != -1 ){
				/* clock�M�p�ł��Ȃ���� */
				/* notes�����̏���(TLADD)��������Ď��� */
				tracklen( D_OP_TLADD , (long)tp_boncur->length );
				/* ����notes���ڂ܂Ői�� */
				tp_boncur = tp_boncur->newarea;
				/* clock���i�񂾃t���O��D_ON */
				i_updateclock = D_ON;
				continue;
			}

			/* ���ߐ��̖{���� */
			if( 0 == tracklen( D_OP_TLLEVEL , 0 ) ){
				/* ���x���[���̏ꍇ */
				i_ret = tracklen( D_OP_TLTOTAL , 0 );
				if( i_ret % gi_zenlen == l_toffset ){
					/* ���܂��ɏ��ߐ���ɂ���̂ŏ��ߐ����� */
					if( i_updateclock != D_OFF ){
						/* �O�񏬐ߐ��������Ă���clock�������Ă���ꍇ�̂� */
						tp_boncur->barline[0] = 0x20;
						tp_boncur->barline[1] = 0x0;
					}
				}
			}else{
				/* ���x���P�ȏ�̏ꍇ */
				i_ret = tracklen( D_OP_TLTLEVEL , 0 );
				if( i_ret % gi_zenlen == 0 ){
					/* ���܂��ɏ��ߐ���ɂ���̂ŏ��ߐ����� */
					if( i_updateclock != D_OFF ){
						/* �O�񏬐ߐ��������Ă���clock�������Ă���ꍇ�̂� */
						tp_boncur->barline[0] = 0x20;
						tp_boncur->barline[1] = 0x0;
					}
				}
			}

			/* ���ߐ�����ׂ��ŉ��������݂��Ă���ꍇ�̏��� */
			if( 0 == tracklen( D_OP_TLLEVEL , 0 ) ){
				/* ���x���[���̏ꍇ */
				i_ret = tracklen( D_OP_TLTOTAL , 0 );
				i_ret = (i_ret-l_toffset) % gi_zenlen;
			}else{
				/* ���x���P�ȏ�̏ꍇ */
				i_ret = tracklen(D_OP_TLTLEVEL,0) % gi_zenlen;
			}
			/* l_coffset ���̏��ߐ��܂ł̋��� */
			l_coffset = gi_zenlen - i_ret;
			/* ���ߐ��܂ł̋��� l_coffset �������݂̉��̉����������ꍇ */
			if( l_coffset < tp_boncur->length && tp_boncur->note2 == 0x00 ){
				/* �����ł��Ȃ����͕̂������Ȃ�(portaments) */
				/* �ʕ����荞�܂� */
				insbon( tp_boncur , tp_boncur->newarea );
				/* �����p����p�����[�^�͑S�Ĉ����p���ŁA����ȊO�͏C�� */
				tp_boncur->newarea->toffset = tp_boncur->toffset + l_coffset;
				tp_boncur->newarea->length =
					tp_boncur->length - (short)l_coffset;
				tp_boncur->newarea->note1 = tp_boncur->note1;
				tp_boncur->newarea->trans = tp_boncur->trans;
				tp_boncur->newarea->mtrans = tp_boncur->mtrans;
				tp_boncur->newarea->note1 = tp_boncur->note1;
				tp_boncur->newarea->oct = tp_boncur->oct;
				tp_boncur->length = (short)l_coffset;
				tp_boncur->ties = (char)'&';
			}

			/* notes�����̏���(TLADD) */
			tracklen( D_OP_TLADD , (long)tp_boncur->length );
			/* ����notes���ڂ܂Ői�� */
			tp_boncur = tp_boncur->newarea;
			/* clock���i�񂾃t���O��D_ON */
			i_updateclock = D_ON;
			continue;
		}
	}

	return;
}
/*--------------------------------------------------------------------*/
/* ��͌n���i */
/* ��������code */
int findnote( char *cp_cur,T_BON *tp_boncur,long l_clock ){

	T_TCODE *tp_cur;

	tp_cur = codefinderbystr( cp_cur );
	/* B.O.Notes�ɏ������� */
	tp_boncur->toffset = l_clock;
	tp_boncur->trans = tp_boncur->befarea->trans;
	tp_boncur->mtrans = tp_boncur->befarea->trans;
	tp_boncur->oct = tp_boncur->befarea->oct;
	/* V1.21 2008/7/14 �A�h���X�͏�ɕۑ� */
	/* �d������control��notes-control�̑O��֌W��ۏ؂��邽�� */
	tp_boncur->address = cp_cur;
	switch( (char)(tp_cur->code1) ){
		case (char)0x7F:
			/* �ʏ��notes */
			tp_boncur->length = (unsigned char)*(cp_cur+1);
			tp_boncur->note1 = *cp_cur;
			tp_boncur->note2 = 0;
			if( (char)(tp_boncur->note1&0x0F) == (char)0x0F ){
				/* �x���̏ꍇ�́A���̑O����oct�������p�� */
				tp_boncur->oct = tp_boncur->befarea->oct;
			}else{
				tp_boncur->oct = 1+((*cp_cur)&0x70)/16;
			}
			tp_boncur->ties = D_OFF;
			break;
		case (char)0xDA:
			/* �|���^�����g */
			tp_boncur->length = (unsigned char)*(cp_cur+3);
			tp_boncur->note1 = *(cp_cur+1);
			tp_boncur->note2 = *(cp_cur+2);
			/* oct�ɂ̓|���^�����g�I����̒l������ */
			/* �|���^�����g��oct���E���ׂ��悤�ȏꍇ�̖��� */
			/* �|���^�����gMML��\������ۂɏC������ */
			tp_boncur->oct = 1+((*cp_cur+2)&0x70)/16;
			tp_boncur->ties = D_OFF;
			tp_boncur->nocombine = D_ON;
			break;
		case (char)0xC1:
			/* && �X���[ */
			tp_boncur->note1 = *cp_cur;
			tp_boncur->ties = *cp_cur;
			tp_boncur->oct = tp_boncur->befarea->oct;
			break;
		case (char)0xFB:
			/* & �^�C�E�X���[ */
			tp_boncur->note1 = *cp_cur;
			tp_boncur->ties = *cp_cur;
			tp_boncur->oct = tp_boncur->befarea->oct;
			break;
		case (char)0xB2:
			/* _M �}�X�^�[�]�� */
			tp_boncur->note1 = *cp_cur;
			tp_boncur->note2 = *(cp_cur+1);
			tp_boncur->trans = 0;
			tp_boncur->mtrans = *(cp_cur+1);
			break;
		/* V1.21 2008/7/14 start ����/��Γ]����bug��fix */
		case (char)0xE7:
			/* __ ���Γ]�� */
			tp_boncur->note1 = *cp_cur;
			tp_boncur->note2 = *(cp_cur+1);
			tp_boncur->trans = tp_boncur->befarea->trans + *(cp_cur+1);
			break;
		case (char)0xF5:
			/* _ ��Γ]�� */
			tp_boncur->note1 = *cp_cur;
			tp_boncur->note2 = *(cp_cur+1);
			tp_boncur->trans = *(cp_cur+1);
			break;
		/* V1.21 2008/7/14 end */
		default:
			break;
	}

	/* �I�[�ɓ��B���Ă���ꍇ�͎��̃G���A���m�� */
	if( tp_boncur->newarea == NULL ){
		newbon( tp_boncur );
	}

	return(tp_cur->datasize+1);
}
/*--------------------------------------------------------------------*/
/* ��͌n���i */
/* �߂�l 0:OK 1:��������code -1:ERROR */
int findcontrol( char *cp_cur , T_BOC *tp_boccur , long l_clock ){

	int mmllength;
	T_TCODE *tp_cur;

	tp_cur = codefinderbystr( cp_cur );
	if( tp_cur == NULL ){
		/* ���W�b�N��΂� */
		sprintf(gcs_line,"E[DLL] Internal error(unknown code 2).\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
		exit(-1);
	}
	if( tp_cur->format == 0 ){
		/* ��������R�[�h */
		return(1);
	}
	if( tp_cur->format == 2 ){
		/* �����B���肦�Ȃ� */
		sprintf(gcs_line,"E[DLL] Internal error(unknown code 3).\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
		exit(-1);
	}

	tp_boccur->toffset = l_clock;
	tp_boccur->code = *cp_cur;
	tp_boccur->disable = D_OFF;
	/* L ���������ꍇ�̓t���O�𗧂Ă� */
	if( 0xF6 == *cp_cur || gi_llfindout == D_OFF ){
		gi_llfindout = D_ON;
	}

	/* V1.21 2008/7/14 �A�h���X�͏�ɕۑ� */
	/* �d������control��notes-control�̑O��֌W��ۏ؂��邽�� */
	tp_boccur->address = cp_cur;

	switch(tp_cur->format){
		case 1:
			/* ���������֐� #PPZ/FM3Extend */
			expset(cp_cur);
			break;
		case 3:
			/* �����o�������̂��� L : [ */
			tp_boccur->value = 0;
			break;
		case 4:
		case 5:
		case 7:
		case 8:
			/* �r���̕ҏW�ɐ��l���K�v�Ȃ��̂̓p�����[�^���i�[ (Z/])�͕K�{ */
			tp_boccur->value = *(cp_cur+1);
		case 6:
			/* �o�͗l����ʓr��߂Ă�����́��A�h���X�ۑ�(�̂�-V1.21) */
			break;
		default:
			/* ��������R�[�h�A���邢�͉����B���肦�Ȃ� */
			sprintf(gcs_line,"E[DLL] Internal error(unknown code 4).\r\n");
			hfpf( DH_BMESSAGES , gcs_line );
			exit(-1);
	}

	/* �I�[�ɓ��B���Ă���ꍇ�͎��̃G���A���m�� */
	if( tp_boccur->newarea == NULL ){
		newboc( tp_boccur );
	}

	return( 0 );
}
