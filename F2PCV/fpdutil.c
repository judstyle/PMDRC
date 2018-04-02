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

#include "fpd.h"
#include "fpdext.h"


/*--------------------------------------------------------------------*/
/* ��� �A�h���X�̃X�^�b�N */
/* arg. NULL:�W�v���� other:�X�^�b�N */
char *stackaddr( char *cp_getm ){

static char *cp_mtb=NULL;
static char **cpp_mcur=NULL;
static int i_mtbcount=0;
static int i_mtbsize=0;
static int i_mtbunit=0;
	char *cp_tmp;
	char **cpp_tmp;
	char *cp_ret;
	int i_count;
	int i_doublecount;
	char **cpp_sorter;
	char **cpp_nsorter;
	char **cpp_scur=NULL;
long *lp_tmp;

	/* �W�v����̏ꍇ */
	if( cp_getm == NULL ){
		/* ���{�E�E�E�E���Đ�����Ăǁ[��񂾁H */
		/* �Ƃ肠�����A���邮��񂷂����Ȃ����E�E�E�E�E */
		if( i_mtbcount == 0 ){
			/* �P���Ȃ��ꍇ */
			return( NULL );
		}
		if( i_mtbcount > 1 ){
			/* �Q�ȏ゠�����ꍇ�͐��� */
			for(i_doublecount=0;i_doublecount<(i_mtbcount-1);i_doublecount++){
				cpp_scur = (char **)cp_mtb;
				for(i_count=0;i_count<(i_mtbcount-1);i_count++){
					cpp_sorter = cpp_scur;
					cpp_scur++;
					cpp_nsorter = cpp_scur;
					if( *cpp_sorter > *cpp_nsorter ){
						/* ���Ԃ����ւ��� */
						cp_tmp = *cpp_nsorter;
						*cpp_nsorter = *cpp_sorter;
						*cpp_sorter = cp_tmp;
					}
				}
			}
		}

		/* ����ƁA�V����apmalloc�Ŏ�蒼���Ȃ��ƃ}�Y�C */
		cp_ret = apmalloc( sizeof(char *) * (i_mtbcount+1) );
		memset( cp_ret , 0x00 , sizeof(char *) * (i_mtbcount) );
		memcpy( cp_ret , cp_mtb , sizeof(char *) * (i_mtbcount) );
//		apfree( cp_mtb );

		/* static�ϐ��̏����� */
		cp_mtb=NULL;
		cpp_mcur=NULL;
		i_mtbcount=0;
		i_mtbsize=0;
		i_mtbunit=0;

		/* �Ԃ��̂�(char *)�� */
		return( cp_ret );
	}

	/* �������̊��蓖�Ă��K�v�ȏꍇ������ */
	if( cp_mtb == NULL && i_mtbcount == 0 ){
		/* �������蓖�� */
		cp_mtb = apmalloc( (1+DN_SIZE_STACKUNIT) * (sizeof(char *)) );
		cpp_mcur = (char **)(cp_mtb);
		i_mtbcount = 0;
		i_mtbunit = 1;
		i_mtbsize = DN_SIZE_STACKUNIT * sizeof(void *);
	}else if( i_mtbcount > 0 && i_mtbcount % DN_SIZE_STACKUNIT == 0 ){
		/* �Ċ��蓖�� */
		i_mtbunit++;
		cp_tmp = apmalloc( (1+DN_SIZE_STACKUNIT*i_mtbunit) * (sizeof(char *)) );
		memcpy( cp_tmp , cp_mtb ,
			(1+DN_SIZE_STACKUNIT*(i_mtbunit-1)) * (sizeof(char *)) );
//		apfree( cp_mtb );
		cp_mtb = cp_tmp;
		cpp_mcur = (char **)(cp_mtb);
		cpp_mcur += i_mtbcount;
		i_mtbsize += DN_SIZE_STACKUNIT * sizeof(char *);
	}

	/* �擾�����������A�h���X���X�^�b�N */
	*cpp_mcur = cp_getm;
	i_mtbcount++;
	cpp_mcur++;

	return( NULL );
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
				apfree( *vpp_mcur );
			}
			vpp_mcur++;
		}
		/* �Ō�Ɏ������g�Ŏg�p���Ă��郁�����������[�X */
		apfree( vp_mtb );
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

//		vp_tmp = malloc( (1+DN_SIZE_MTBUNIT*i_mtbunit) * (sizeof(void *)) );
//		memcpy( vp_tmp , vp_mtb ,
//			(1+DN_SIZE_MTBUNIT*(i_mtbunit-1)) * (sizeof(void *)) );
//		apfree( vp_mtb );
//		vp_mtb = vp_tmp;
		vp_mtb = realloc( vp_mtb ,
			(1+DN_SIZE_MTBUNIT*i_mtbunit) * (sizeof(void *)) );
//
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
/*  arg.1  -1:init  0-(DN_MAX_F2POUTBUFFER-1):handle */
/*  arg.2  �o�͕�����ւ̃|�C���^ �ԋp�l�� NULL:���� cp_in:�ُ� */
/*         NULL��:���Y�n���h���̃o�b�t�@�擪�A�h���X��ԋp */
int apfree( void *vp_in ){

	int i_ret;

	if( vp_in != NULL ){
		free( vp_in );
	}

	return( 0 );
}
/*--------------------------------------------------------------------*/
/* ������o�͕⏕�֐� */
/*  arg.1  -1:init  0-(DN_MAX_F2POUTBUFFER-1):handle */
/*  arg.2  �o�͕�����ւ̃|�C���^ �ԋp�l�� NULL:���� cp_in:�ُ� */
/*         NULL��:���Y�n���h���̃o�b�t�@�擪�A�h���X��ԋp */
char *hfpf( int i_handle , char *cp_in ){

static int is_outbuffersize[DN_MAX_F2POUTBUFFER];
static int is_cursize[DN_MAX_F2POUTBUFFER];
static char *cps_cur[DN_MAX_F2POUTBUFFER];
static char *cps_buffer[DN_MAX_F2POUTBUFFER];
static int is_unit[DN_MAX_F2POUTBUFFER];
	char *cp_ret;
	int i_count;

	if( i_handle == -1 ){
		/* �S������ */
		for(i_count=0;i_count<DN_MAX_F2POUTBUFFER;i_count++){
			is_outbuffersize[i_count] = 0;
			is_cursize[i_count] = 0;
			cps_cur[i_count] =  NULL;
			cps_buffer[i_count] = NULL;
			is_unit[i_count] = 0;
		}
		sprintf(gcs_line," [F2P] Initialize output-buffers.\n");
		if( (gtp_gop->i_directmes&D_ON) != 0 ){
			/* ���䃁�b�Z�[�W�̃_�C���N�g�o�͑Ή� */
			fprintf(stderr,gcs_line);
		}
		return( NULL );
	}
	/* �n���h���͈͊O */
	if( i_handle < 0 || i_handle >= DN_MAX_F2POUTBUFFER ){
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

	/* �����`�F�b�N */
	if( cp_in != NULL && (strlen(cp_in) >= DN_SIZE_LINEBUFFER) ){
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
		apfree( cps_buffer[i_handle] );
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
