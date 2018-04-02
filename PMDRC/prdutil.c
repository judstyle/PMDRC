/**********************************************************************/
/* PMDRC - P.M.D. reversal compiler by JUD(T.Terata)                  */
/* Utility functions                                                  */
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
/* ���������n */
/* EX�p�[�g���������� �p�[�g��`��ǉ� */
int expset( char *cp_findout ){

	char *cp_cur;
	int i_offset;
	int i_cnt;
	int i_zcnt;
	T_TCODE *tp_tccur;

	int i_spoint;
	int i_epoint;
	char c_schar;

	cp_cur = cp_findout;
	cp_cur++;
	if( *cp_findout == (char)0xC6 ){
		/* FM3Extend */
		sprintf(gcs_line," [DLL] Findout FM3Extend. [Part:XYZ]\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
		i_spoint = DN_START_NUM_FM3EX;
		i_epoint = DN_START_NUM_PPZ;
		c_schar = (char)DC_START_CHAR_FM3EX;
		gt_ginfo.i_fFM3Extend = D_ON;
	}else if( *cp_findout == (char)0xB4 ){
		/* PPZExtend */
		sprintf(gcs_line," [DLL] Findout PPZExtend. [Part:abcdefgh]\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
		i_spoint = DN_START_NUM_PPZ;
		i_epoint = DN_MAX_TRACKNUM;
		c_schar = (char)DC_START_CHAR_PPZ;
		gt_ginfo.i_fPPZExtend = D_ON;
	}else{
		/* �G���[�ł� */
		sprintf(gcs_line,"E[DLL] Syntax error(ExPart code). [%02X]\r\n",
			*cp_findout );
		hfpf( DH_BMESSAGES , gcs_line );
		exit(-1);
	}

	for(i_cnt=i_spoint,i_zcnt=0;i_cnt<i_epoint;i_cnt++,i_zcnt++){
		ts_ts[i_cnt].use = D_ON;
		i_offset = (int)((unsigned char)(*cp_cur));
		i_offset += ( 256 * (int)((unsigned char)(*(cp_cur+1))) );
		if( i_offset == 0 ){
			/* Extend�ő��₳�ꂽ�p�[�g��3(PPZ�Ȃ�8)�����̏ꍇ */
			ts_ts[i_cnt].text = NULL;
			ts_ts[i_cnt].partname = (c_schar+(char)i_zcnt);
			ts_ts[i_cnt].use = D_OFF;
			sprintf(gcs_line," [DLL] Part-%c no-extend.\n",
				ts_ts[i_cnt].partname);
			hfpf( DH_BMESSAGES , gcs_line );
		}else{
			/* Extend�Ńp�[�g�𑝂₷ */
			ts_ts[i_cnt].text = gtp_gop->cp_indata;
			ts_ts[i_cnt].text += (1 + i_offset);
			ts_ts[i_cnt].partname = (c_schar+(char)i_zcnt);
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
		}
		cp_cur++;
		cp_cur++;
	}

	tp_tccur = codefinderbystr(cp_findout);
	if(tp_tccur == NULL){
		sprintf(gcs_line,"E[DLL] Internal error(ExPart/notExist).\r\n");
		hfpf( DH_BMESSAGES , gcs_line );
		exit(-1);
	}
	return(tp_tccur->datasize);
}
/*--------------------------------------------------------------------*/
/* �����̃R�[�h���L�q�����e�[�u���A�h���X��ԋp */
/* ��ɁA��΂��p�r�Ɏg�p����i�������A��͂ɂ��g���j */
/* NULL:������Ȃ��Ƃ��A�I���R�[�h�̂Ƃ� */
T_TCODE *codefinderbystr( char *cp_code ){

	T_TCODE *tp_tc;
	T_TCODE *tp_keep;
	char c_keep;

	/* notes or trackend */
	if( 0x00 <= *cp_code && *cp_code <= 0x7F ){
		return(&(ts_tc[0]));
	}else if( *cp_code == (char)0x80 ){
		return(NULL);
	}

	tp_keep = NULL;
	c_keep = 0x81;
	tp_tc = &(ts_tc[1]);
	for(;tp_tc->cont==D_NOTLAST;tp_tc++){
		if( *cp_code == tp_tc->code1 ){
			if( tp_tc->code2 == 0 ){
				/* findout */
				return(tp_tc);
			}else if( (char)(tp_tc->code2) == (char)(*(cp_code+1)) ){
				/* findout */
				return(tp_tc);
			}
		}
	}

	return(NULL);
}

/*--------------------------------------------------------------------*/
/* ���[�e�B���e�B */
/* BON�ʕ��ǉ� ���[�ɋ󂫃�������ǉ����� */
/* �����Ŏw�肳�ꂽ����OBJ�e�[�u�����Ō���Ƃ݂Ȃ��A�ǉ��m�ہE�A�� */
/* ����NULL�̏ꍇ�́A�ŏ��̂P�ł���̂ŃA�h���X��Ԃ� */
T_BON *newbon( T_BON *tp_in ){
	T_BON *tp_bon;
	T_BON *tp_cur;
	int i_cnt;
	tp_bon = apmalloc(sizeof(T_BON)*DN_GETBON);
	memset(tp_bon,0x00,sizeof(T_BON)*DN_GETBON);
	tp_cur = tp_bon;
	for(i_cnt=0;i_cnt<DN_GETBON;i_cnt++,tp_cur++){
		if( i_cnt >= (DN_GETBON-1) ){
			tp_cur->newarea = NULL;
			tp_cur->befarea = tp_cur-1;
			break;}
		tp_cur->newarea = tp_cur+1;
		if( i_cnt != 0 ){
			tp_cur->befarea = tp_cur-1;
		}else if( tp_in != NULL ){
			tp_cur->befarea = tp_in;
		}else{
			tp_cur->befarea = NULL;} }
	if(tp_in!=NULL){
		tp_in->newarea = tp_bon;
		return(tp_bon);
	}else{return(tp_bon);}
}
/*--------------------------------------------------------------------*/
/* ���[�e�B���e�B */
/* BON�ʕ��폜 �O��̌ʕ����V���[�g�J�b�g���A���Y�ʕ����폜���� */
void delbon( T_BON *tp_in ){

	if( tp_in->newarea == NULL && tp_in->befarea == NULL ){
		/* �O�オ�Ȃ��Ɨ������ʕ����G���[ */
		return;
	}

	if( tp_in->newarea != NULL ){
		/* ���̌ʕ����Ȃ� */
		tp_in->befarea->newarea = NULL;
	}

	if( tp_in->befarea != NULL ){
		/* ��O�̌ʕ����Ȃ� */
		tp_in->newarea->befarea = NULL;
	}

	if( tp_in->befarea != NULL && tp_in->newarea != NULL ){
		/* �O��̌ʕ������� */
		tp_in->newarea->befarea = tp_in->befarea;
		tp_in->befarea->newarea = tp_in->newarea;
	}

	/* ���Y�ʕ����̂Ă� */
	wastebon( tp_in );
	return;
}
/*--------------------------------------------------------------------*/
/* ���[�e�B���e�B */
/* BON�ʕ��}�� �O��̌ʕ��̊ԂɁA�V�ʕ���ǉ����� */
void insbon( T_BON *tp_bef , T_BON *tp_new ){

	T_BON *tp_cur;

	if( tp_bef == NULL || tp_bef == NULL ){
		/* �O��ǂ��炩���������Ă��遁�G���[ */
		return;
	}

	tp_cur = get1bon( NULL );

	tp_cur->newarea = tp_new;
	tp_cur->befarea = tp_bef;
	tp_cur->newarea->befarea = tp_cur;
	tp_cur->befarea->newarea = tp_cur;

	return;
}
/*--------------------------------------------------------------------*/
/* ���[�e�B���e�B */
/* �p������ʕ����X�g�b�N���� */
void wastebon( T_BON *tp_in ){

	if( gi_stockbons > 254 ){
		/* �X�g�b�N���Ă���ʕ������t */
		return;
	}
	/* �p���ʕ����E���ď����� */
	gtps_stockbon[gi_stockbons] = tp_in;
	memset(tp_in,0x00,sizeof(T_BON));

	/* �X�g�b�N�J�E���^���X�V */
	gi_stockbons++;

	return;
}
/*--------------------------------------------------------------------*/
/* ���[�e�B���e�B */
/* �X�g�b�N���Ă���ʕ����ė��p���� */
T_BON *reusebon( ){

	T_BON *tp_out;

	if( gi_stockbons <= 0 ){
		/* �X�g�b�N���Ă���ʕ����Ȃ� */
		return(NULL);
	}

	/* �X�g�b�N�ʕ��̍Ō�̈���E�� */
	tp_out = gtps_stockbon[gi_stockbons-1];

	/* �X�g�b�N�J�E���^���X�V */
	gi_stockbons--;

	return(tp_out);
}
/*--------------------------------------------------------------------*/
/* ���[�e�B���e�B */
/* �P�����ʕ����擾���� �\�Ȃ�ΈȑO�p���������̂��ė��p���� */
/* newbon()�ƈႢ�A���g�͂܂����疔�͕ۏ؂ł��Ȃ���ԂȂ̂Œ��ӂ��邱�� */
T_BON *get1bon( T_BON *tp_in ){

static int i_lastbon=0;
static T_BON *tp_addbon;
	T_BON *tp_out;

	/* �X�g�b�N���m�F */
	tp_out = reusebon();
	if( tp_out != NULL ){
		/* �p����������ł����̂ł����Ԃ� */
		memset(tp_out,0x00,sizeof(T_BON));
		if( tp_in != NULL ){
			tp_out->befarea = tp_in;
		}
		return(tp_out);
	}

	/* �p���������݂��Ȃ��̂ŐV���������������蓖�Ă� */
	if( i_lastbon <= 0 ){
		/* �V�������������Ȃ��̂ŁAapmalloc�Ŋm�ۂ��� */
		tp_addbon = newbon( NULL );
		i_lastbon = DN_GETBON;
	}
	tp_out = tp_addbon;
	tp_out += ( i_lastbon-1 );
	i_lastbon--;

	memset(tp_out,0x00,sizeof(T_BON));
	if( tp_in != NULL ){
		tp_out->befarea = tp_in;
	}
	return(tp_out);
}
/*--------------------------------------------------------------------*/
/* ���[�e�B���e�B */
/* BOC�ʕ��ǉ� ���[�ɋ󂫃�������ǉ����� */
/* �����Ŏw�肳�ꂽ����OBJ�e�[�u�����Ō���Ƃ݂Ȃ��A�ǉ��m�ہE�A�� */
/* ����NULL�̏ꍇ�́A�ŏ��̂P�ł���̂ŃA�h���X��Ԃ� */
T_BOC *newboc( T_BOC *tp_in ){
	T_BOC *tp_boc;
	T_BOC *tp_cur;
	int i_cnt;
	tp_boc = apmalloc(sizeof(T_BOC)*DN_GETBOC);
	memset((void *)tp_boc,0x00,sizeof(T_BOC)*DN_GETBOC);
	tp_cur = tp_boc;
	for(i_cnt=0;i_cnt<DN_GETBOC;i_cnt++,tp_cur++){
		if( i_cnt >= (DN_GETBOC-1) ){
			tp_cur->newarea = NULL;
			tp_cur->befarea = tp_cur-1;
			break;}
		tp_cur->newarea = tp_cur+1;
		if( i_cnt != 0 ){
			tp_cur->befarea = tp_cur-1;
		}else if( tp_in != NULL ){
			tp_cur->befarea = tp_in;
		}else{
			tp_cur->befarea = NULL;} }
	if(tp_in!=NULL){
		tp_in->newarea = tp_boc;
		return(NULL);
	}else{return(tp_boc);}
}
/*--------------------------------------------------------------------*/
/* ���[�e�B���e�B */
/* BOM�ʕ��ǉ� ���[�ɋ󂫃�������ǉ����� */
/* �����Ŏw�肳�ꂽ����OBJ�e�[�u�����Ō���Ƃ݂Ȃ��A�ǉ��m�ہE�A�� */
/* ����NULL�̏ꍇ�́A�ŏ��̂P�ł���̂ŃA�h���X��Ԃ� */
T_BOM *newbom( T_BOM *tp_in ){
	T_BOM *tp_bom;
	T_BOM *tp_cur;
	int i_cnt;
	tp_bom = apmalloc(sizeof(T_BOM)*DN_GETBOM);
	memset(tp_bom,0x00,sizeof(T_BOM)*DN_GETBOM);
	tp_cur = tp_bom;
	for(i_cnt=0;i_cnt<DN_GETBOM;i_cnt++,tp_cur++){
		if( i_cnt >= (DN_GETBOM-1) ){
			tp_cur->newarea = NULL;
			tp_cur->befarea = tp_cur-1;
			break;}
		tp_cur->newarea = tp_cur+1;
		if( i_cnt != 0 ){
			tp_cur->befarea = tp_cur-1;
		}else if( tp_in != NULL ){
			tp_cur->befarea = tp_in;
		}else{
			tp_cur->befarea = NULL;} }
	if(tp_in!=NULL){
		tp_in->newarea = tp_bom;
		return(NULL);
	}else{return(tp_bom);}
}
/*--------------------------------------------------------------------*/
/* ���[�e�B���e�B */
/* �ׂ��� */
int powxy(int x,int y){
	int i_loop;
	int	i_ans;

	i_ans = 1;
	for(i_loop=0;i_loop<y;i_loop++){
		i_ans = i_ans * x;
	}
	return( i_ans );
}
/*--------------------------------------------------------------------*/
/* �Ǘ��֐� */
/* �g���b�N�����Ǘ��֐� */
/* �������[�v�����݂���ꍇ�͕ԋp�l�}�C�i�X�P */
long tracklen( int i_op , long value ){

static long l_totallen;
static long ls_looplen[257];
static long ls_lcllen[257];
static long l_nestcount;
static long l_beforeL;
static int i_lc;
static int is_lcl[257];
static int i_init;
	long l_tmp;


	if( gi_tlinit == D_OFF ){
		gi_tlinit = D_ON;
		i_init = D_OFF;
		l_beforeL = 0;
		l_totallen = 0;
		l_nestcount = 0;
		for(i_lc=0;i_lc<256;i_lc++){ls_looplen[i_lc]=0;}
		for(i_lc=0;i_lc<256;i_lc++){ls_lcllen[i_lc]=0;}
		for(i_lc=0;i_lc<256;i_lc++){is_lcl[i_lc]=D_OFF;}
	}

	if( i_op != D_OP_TLINIT && l_nestcount == -1 ){
		return( -1 );
	}

	switch( i_op ){
		case D_OP_TLINIT:
			/* ���������� */
			i_init = D_ON;
			l_beforeL = 0;
			l_totallen = 0;
			l_nestcount = 0;
			for(i_lc=0;i_lc<256;i_lc++){ls_looplen[i_lc]=0;}
			for(i_lc=0;i_lc<256;i_lc++){ls_lcllen[i_lc]=0;}
			for(i_lc=0;i_lc<256;i_lc++){is_lcl[i_lc]=D_OFF;}
			break;
		case D_OP_TLADD:
			/* �����ǉ� value:���� */
			ls_looplen[l_nestcount] += value;
			if( is_lcl[l_nestcount] == D_ON ){
				ls_lcllen[l_nestcount] += value;
			}
			break;
		case D_OP_TLLSTART:
			/* ���[�v�J�n */
			l_nestcount++;
			break;
		case D_OP_TLLEND:
			/* ���[�v�I�� value:���[�v�� */
			if( value == 0 ){
				/* �������[�v */
				l_nestcount = -1;
				return(-1);
			}
			l_tmp = ls_looplen[l_nestcount] * value - ls_lcllen[l_nestcount];
			ls_looplen[l_nestcount-1] += l_tmp;
			if( is_lcl[l_nestcount-1] == D_ON ){
				ls_lcllen[l_nestcount-1] += l_tmp;
			}
			ls_looplen[l_nestcount] = 0;
			ls_lcllen[l_nestcount] = 0;
			is_lcl[l_nestcount] = D_OFF;
			l_nestcount--;
			break;
		case D_OP_TLFINDL:
			/* L�R�}���h���� */
			if( l_nestcount != 0 ){
				/* [ ] ���[�v���� L ���� */
				return(-1);
			}
			l_beforeL = ls_looplen[0];
			break;
		case D_OP_TLFINDCL:
			/* :�R�}���h���� */
			is_lcl[l_nestcount] = D_ON;
			break;
		case D_OP_TLTOTAL:
			/* �����g�[�^���\�� */
			if( l_nestcount == -1 ){
				return( -1 );
			}else{
				return(ls_looplen[0]);
			}
		case D_OP_TLTLEVEL:
			/* ���݂̃��x���ɂȂ��Ă���̃g�[�^����\�� */
			return(ls_looplen[l_nestcount]);
		case D_OP_TLLEVEL:
			/* ���݂̃��x����\�� */
			return(l_nestcount);
		case D_OP_TLBEFOREL:
			/* L�ȑO�����\�� */
			return(l_beforeL);
		default:
			break;
	}

	return(0);
}
/*--------------------------------------------------------------------*/
/* �������ꊇ����⏕�֐� */
/*  arg.  -1:Rerease  0-:size */
void *apmalloc( int i_size ){

static void *vp_mtb=NULL;
static void **vpp_mcur=NULL;
static int i_mtbcount=0;
static int i_mtbsize=0;
static int i_mtbunit=0;
	void *vp_tmp;
static void *vp_getm;
	int i_count;

	/* ����������̏ꍇ */
	if( i_size == -1 ){
		if( i_mtbcount == 0 ){
			/* ������т��Ȃ��̂ŉ������Ȃ� */
			return( NULL );
		}
		/* ���{ */
		vpp_mcur = vp_mtb;
		for(i_count=0;i_count<i_mtbcount;i_count++){
			if( *vpp_mcur != NULL ){
				free( *vpp_mcur );
			}
			vpp_mcur++;
		}
		/* �Ō�Ɏ������g�Ŏg�p���Ă��郁�����������[�X */
		free( vp_mtb );
		return( NULL );
	}

	/* �������擾�{�� */
	vp_getm = malloc( i_size );
	if( vp_getm == NULL ){
		return( NULL );
	}

	/* �������̊��蓖�Ă��K�v�ȏꍇ������ */
	if( vp_mtb == NULL && i_mtbcount == 0 ){
		/* �������蓖�� */
		vp_mtb = malloc( (1+DN_SIZE_MTBUNIT) * (sizeof(void *)) );
		vpp_mcur = (void *)(vp_mtb);
		i_mtbcount = 0;
		i_mtbunit = 1;
		i_mtbsize = DN_SIZE_MTBUNIT * sizeof(void *);
	}else if( i_mtbcount > 0 && i_mtbcount % DN_SIZE_MTBUNIT == 0 ){
		/* �Ċ��蓖�� */
		i_mtbunit++;
		vp_tmp = malloc( (1+DN_SIZE_MTBUNIT*i_mtbunit) * (sizeof(void *)) );
		memcpy( vp_tmp , vp_mtb ,
			(1+DN_SIZE_MTBUNIT*(i_mtbunit-1)) * (sizeof(void *)) );
		free( vp_mtb );
		vp_mtb = vp_tmp;
		vpp_mcur = (void *)(vp_mtb);
		vpp_mcur += i_mtbcount;
		i_mtbsize += DN_SIZE_MTBUNIT * sizeof(void *);
	}

	/* �擾�����������A�h���X���X�^�b�N */
	*vpp_mcur = vp_getm;
	i_mtbcount++;
	vpp_mcur++;

	return( vp_getm );
}
/*--------------------------------------------------------------------*/
/* ������o�͕⏕�֐� */
/*  arg.1  -1:init  0-(DN_MAX_OUTBUFFER-1):handle */
/*  arg.2  �o�͕�����ւ̃|�C���^ �ԋp�l�� NULL:���� cp_in:�ُ� */
/*         NULL��:���Y�n���h���̃o�b�t�@�擪�A�h���X��ԋp */
char *hfpf( int i_handle , char *cp_in ){

static int is_outbuffersize[DN_MAX_OUTBUFFER];
static int is_cursize[DN_MAX_OUTBUFFER];
static char *cps_cur[DN_MAX_OUTBUFFER];
static char *cps_buffer[DN_MAX_OUTBUFFER];
static int is_unit[DN_MAX_OUTBUFFER];
	char *cp_ret;
	int i_count;

	if( i_handle == -1 ){
		/* �S������ */
		for(i_count=0;i_count<DN_MAX_OUTBUFFER;i_count++){
			is_outbuffersize[i_count] = 0;
			is_cursize[i_count] = 0;
			cps_cur[i_count] =  NULL;
			cps_buffer[i_count] = NULL;
			is_unit[i_count] = 0;
		}
		sprintf(gcs_line," [DLL] Initialize output-buffers.\n");
		if( (gtp_gop->i_directmes&D_ON) != 0 ){
			/* ���䃁�b�Z�[�W�̃_�C���N�g�o�͑Ή� */
			fprintf(stderr,gcs_line);
		}
		return( NULL );
	}
	/* �n���h���͈͊O */
	if( i_handle < 0 || i_handle >= DN_MAX_OUTBUFFER ){
		return( cp_in );
	}

	if( cp_in == NULL ){
		/* �e�n���h���Ή��̃o�b�t�@��Ԃ��ꍇ */
		return( cps_buffer[i_handle] );
	}

	if( (gtp_gop->i_directmes&D_ON) != 0 && i_handle == DH_BMESSAGES ){
		/* ���䃁�b�Z�[�W�̃_�C���N�g�o�͑Ή� */
		fprintf(stderr,"%s",cp_in);
	}

	/* ����J�t�@���N�V���� */
	if( (gtp_gop->i_directmes&DA_DEBUG) != 0 && i_handle == DH_BSDUMP ){
		/* �f�o�b�O���X�[�p�[�_���v�̃_�C���N�g�o�͑Ή� */
		fprintf(stdout,"%s",cp_in);
		return( NULL );
	}

	/* �����`�F�b�N */
	if( cp_in != NULL && strlen(cp_in) >= DN_SIZE_LINEBUFFER ){
		return( cp_in );
	}

	/* ���������蓖�� */
	if( is_outbuffersize[i_handle] == 0 ){
		/* �������m��(����) */
		cps_buffer[i_handle] = malloc( DN_MAX_OUTUNITSIZE );
		cps_cur[i_handle] = cps_buffer[i_handle];
		is_unit[i_handle] = 1;
		is_outbuffersize[i_handle] = DN_MAX_OUTUNITSIZE;
	}else if( is_cursize[i_handle]+strlen(cp_in)+1 >
				is_outbuffersize[i_handle] ){
		/* �������s�����ǉ����蓖�� */
		is_unit[i_handle]++;
		cp_ret = malloc( DN_MAX_OUTUNITSIZE * is_unit[i_handle] );
		memcpy( cp_ret , cps_buffer[i_handle] ,
			DN_MAX_OUTUNITSIZE * (is_unit[i_handle]-1) );
		free( cps_buffer[i_handle] );
		cps_buffer[i_handle] = cp_ret;
		/* �J�����g�|�C���^�Ĕz�u */
		cps_cur[i_handle] = cps_buffer[i_handle] + is_cursize[i_handle];
		is_outbuffersize[i_handle] = DN_MAX_OUTUNITSIZE * is_unit[i_handle];
	}
	/* �o�� */
	strcpy( cps_cur[i_handle] , cp_in );
	/* �J�����g�T�C�Y���Z�E�J�����g�|�C���^�ړ��E�������|�� */
	cps_cur[i_handle] += strlen( cp_in );
	is_cursize[i_handle] += strlen( cp_in );
	if( cp_in != NULL ){
		memset( cp_in , 0x00 , DN_SIZE_LINEBUFFER );
	}

	return( NULL );
}
