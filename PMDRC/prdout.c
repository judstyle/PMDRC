/**********************************************************************/
/* PMDRC - P.M.D. reversal compiler by JUD(T.Terata)                  */
/* Output functions                                                   */
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
/* �o�͏��� */
/*--------------------------------------------------------------------*/
/* �o�͌n */
/* �g���b�N�o�͏��� */
void trackout( int i_partno ){

	long l_curclock;
	T_BOC *tp_boccur;
	T_BON *tp_boncur;

	int i_pdeflen=0;
	int i_count1;
	int i_maxflen=0;
	int i_2ndflen=0;
	int i_maxfnum=0;
	int i_2ndfnum=0;
	int i_totalnotes=0;
	double d_maxrate;
	double d_2ndrate;
	int i_flenbig2H;
	int i_flenbig2L;

	/* B.O.Addresses (�ŏ��̈�͎g��Ȃ�) */
	tp_boccur = tp_curtra->caddr;
	tp_boccur++;
	tp_boncur = tp_curtra->naddr;
	tp_boncur++;

	l_curclock=0;

	/* �W�����̑I�o */
	/* �W�����̐ݒ���@�� */
	/* �E�W�������w�肳��Ă��Ȃ��ꍇ���g�p���Ȃ� */
	/* �E���ŕW�������w�肳��Ă���ꍇ���΂肪�傫���ꍇ�̂ݕύX���� */
	/* �E���ŕW�������w�肳��Ă���ꍇ���ύX���Ȃ��Ŏw��ʂ�̒l���g�p���� */
	/* �����Z�o����ꍇ�� */
	/* �E�w��ʂ�̂��̂��ő��̕p�x�ŏo�����Q�Ԗڂ�(1)���ȉ����ύX���Ȃ� */
	/* �E����v�f��(2)���ȏ���߂Ă���ꍇ��1st */
	/* �E����v�f��(3-1)���ȏ���߂Ă���A�Q�Ԗڂ�(3-2)���ȉ��̏ꍇ��1st */
	/* �E�Q�Ԗڂ�(4)���ȉ��̏ꍇ��1st */
	/* �E�Q�Ԗڂ�(5)���ȏ�łP�E�Q�Ԗڂ��W���ȏ�E�P�U���������ꍇ���P�U�� */
	/* �E�ő��p�x��(6)���ȉ��̏ꍇ���[�� */
	/* �E��L�̑S�Ăɍ��v���Ȃ��ꍇ���ύX���Ȃ� */
	if( gtp_gop->i_lendef == 0 ){
		i_pdeflen = 0;
	}else if( gtp_gop->i_autolendef == D_OFF ){
		i_pdeflen = gtp_gop->i_lendef;
	}else if( gtp_gop->i_autolendef == D_ON ){
		/* �������f */
		/* �z����������A�ő��ƂQ�Ԗڂ𒊏o���� */
		/* �X�^�[�g�n�_�͂P���灩�����[���͑���ɂ��Ȃ� */
//		for(i_count1=1;i_count1<(DN_MAX_NOTELENFREQ+1);i_count1++){
		for(i_count1=1;i_count1<(gi_zenlen/2);i_count1++){
			if( iss_notelenfreq[i_partno][i_count1] > i_maxfnum ){
				/* �ő��X�V */
				i_2ndflen = i_maxflen;
				i_maxflen = i_count1;
				i_2ndfnum = i_maxfnum;
				i_maxfnum = iss_notelenfreq[i_partno][i_count1];
			}else if( iss_notelenfreq[i_partno][i_count1] > i_2ndflen ){
				/* �Q�ԖڍX�V */
				i_2ndflen = i_count1;
				i_2ndfnum = iss_notelenfreq[i_partno][i_count1];
			}
			i_totalnotes+=iss_notelenfreq[i_partno][i_count1];
		}
		/* �p�x�Z�o */
		d_maxrate = (double)((double)iss_notelenfreq[i_partno][i_maxflen]/
			(double)i_totalnotes);
		d_2ndrate = (double)((double)iss_notelenfreq[i_partno][i_2ndflen]/
			(double)i_totalnotes);

		/* �������� */
		if( d_2ndrate <= DN_PCT_12P && gtp_gop->i_lendef == i_maxflen ){
			/* �ύX�i�V */
			i_pdeflen = gtp_gop->i_lendef;
		}else if( d_maxrate >= DN_PCT_21P ){
			/* max */
			i_pdeflen = i_maxflen;
		}else if( d_maxrate >= DN_PCT_31P && d_2ndrate <= DN_PCT_32P ){
			/* max */
			i_pdeflen = i_maxflen;
		}else if( d_2ndrate <= DN_PCT_42P ){
			/* max */
			i_pdeflen = i_maxflen;
		}else if( d_2ndrate >= DN_PCT_52P ){
			/* �Q�Ԗڂ�(5)���ȏ�ł���ꍇ�A�܂����� */
			if( d_2ndrate > d_maxrate ){
				i_flenbig2H = i_2ndflen;
				i_flenbig2L = i_maxflen;
			}else{
				i_flenbig2L = i_2ndflen;
				i_flenbig2H = i_maxflen;
			}
			if( i_flenbig2L == (gi_zenlen/16) &&
				i_flenbig2H >= (gi_zenlen/8) ){
				/* �P�E�Q�Ԗڂ��P�U���ƂW���i�ȏ�j�̑g�ݍ��킹 */
				i_pdeflen = gi_zenlen/16;
			}
		}else if( d_maxrate <= DN_PCT_61P ){
			i_pdeflen = 0;
		}else{
			/* �ύX�i�V */
			i_pdeflen = gtp_gop->i_lendef;
		}
	}

	/* �g���b�N�w�b�_������o�� */
	if( gtp_gop->i_mmllevel < DN_OL_THROUGH ){
		/* ���x���S�����̂Ƃ��̂ݕ\�� */
		if( i_pdeflen == 0 ){
			/* �W�����Ƀ[�����w�肳�ꂽ�ꍇ�͕W�������g�p���Ȃ� */
			/* ���̂��߁Al�L���͕\�����Ȃ� (1.00c-) */
			sprintf(gcs_line,"%c o%d\t",
				tp_curtra->partname,
				tp_curtra->startoct);
			i_pdeflen = 0;
		}else{
			/* ���̏ꍇ�́A�������f���g�p���� */
			sprintf(gcs_line,"%c o%dl%d\t",
				tp_curtra->partname,
				tp_curtra->startoct,
				gi_zenlen/i_pdeflen);
		}
		hfpf( DH_BMML , gcs_line );
	}

	for(;;){
		/* V1.21 2008/7/14 start ���[�v�I�����O�̓]�������[�v����ɏo��bugfix*/
		if( tp_boncur->note1 == (char)0x80 ){ break; }
		if( tp_boccur->toffset == l_curclock &&
			tp_boncur->address < tp_boccur->address ){
			/* clock��notes/control���� ���� address��notes��s */
			outcharnote(tp_boncur,i_pdeflen);
			gc_lastoutcat = D_LC_NOTE;
			gc_lastoutchar = tp_boncur->note1;
			tp_boncur = tp_boncur->newarea;
			continue;
		}
		/* V1.21 2008/7/14 end */
		if( tp_boccur->toffset <= l_curclock ){
			if( tp_boccur->code == (char)0x80 ){ break; }
			/* �o�͂��ׂ�code���� */
			outcharcode(tp_boccur);
			gc_lastoutcat = D_LC_CODE;
			gc_lastoutchar = tp_boccur->code;
			tp_boccur = tp_boccur->newarea;
			continue;
		}
		if( tp_boncur->note1 == (char)0x80 ){ break; }

		/* note�o�� */
		if( tp_boncur->note1 != 0x00 ){
			outcharnote(tp_boncur,i_pdeflen);
			gc_lastoutcat = D_LC_NOTE;
			gc_lastoutchar = tp_boncur->note1;
		}
		tp_boncur = tp_boncur->newarea;
		l_curclock = tp_boncur->toffset;
	}

	/* �g���b�N�t�b�^������o�� */
	if( gtp_gop->i_mmllevel < DN_OL_THROUGH ){
		sprintf(gcs_line,"\r\n");
		hfpf( DH_BMML , gcs_line );
	}
	return;
}
/*--------------------------------------------------------------------*/
/* �o�͌n */
/* ����nMML�o�͏��� */
void outcharcode( T_BOC *tp_boccur ){

	T_TCODE *tp_curcode;
	char cs_codes[256];
	char *cp_ret;

	/* control-code �T�[�` */
	tp_curcode = codefinderbystr( &(tp_boccur->code) );

	/* ���x���`�F�b�N */
	if( tp_curcode->level < gtp_gop->i_mmllevel ){
		/* ���x���ɖ����Ȃ��̂ŕ\�����Ȃ� */
		return;
	}

	/* V1.21 2008/7/14 start �]���������ɔ��f������ */
	if( gtp_gop->c_autotrans == D_ON &&
		(tp_boccur->code==(char)0xF5 || tp_boccur->code==(char)0xE7 ) ){
		/* c_autotrans��ON�ł���A��Γ]��/���Γ]���̏ꍇ�̓X�L�b�v */
		return;
	}
	/* V1.21 2008/7/14 end */

	/* MML�ŃJ���}�ɑ���������̂͏��ߐ���؂���֎~���� */
	/* ex) @�R�}���h�̃��[�v�����́A�f�[�^��ʃR�}���h�ɂȂ��Ă��邽�� */
	/*     �O�q�����i�͂��́j@�R�}���h�Ɛڑ����Ȃ��ƈӖ����Ȃ� */
	if( *(tp_curcode->str) == (char)',' ){
		tp_boccur->barline[0] = 0x00;
	}

	if( tp_curcode == NULL ){
		/* �� */
		return;}
	if( tp_curcode->format == 0 ||
		tp_curcode->format == 1 ||
		tp_curcode->format == 2 ){
		/* ���� */
		return;
	}else if( tp_curcode->format == 3 ){
		/* �����\�����邾�� */
		sprintf(cs_codes,"%s%s",tp_boccur->barline,tp_curcode->str);
	}else if( tp_curcode->format == 4 ){
		/* �f�[�^(1bytes)�����̂܂ܕ\�� */
		sprintf(cs_codes,"%s%s%d",tp_boccur->barline,tp_curcode->str,tp_boccur->value);
	}else if( tp_curcode->format == 5 ){
		/* �f�[�^(1bytes)�����̂܂ܕ\��(�����Ȃ�) */
		sprintf(cs_codes,"%s%s%d",tp_boccur->barline,tp_curcode->str,(unsigned char)tp_boccur->value);
	}else if( tp_curcode->format == 7 ){
		/* �f�[�^(1bytes)��K�����������ĕ\�� */
		sprintf(cs_codes,"%s%s%+d",tp_boccur->barline,tp_curcode->str,tp_boccur->value);
	}else if( tp_curcode->format == 8 ){
		/* �f�[�^(1bytes)�����̂܂ܕ\��(��������) �[�����͐��l�ȗ� */
		sprintf(cs_codes,"%s%s%.d",tp_boccur->barline,tp_curcode->str,(unsigned char)tp_boccur->value);
	}else if( tp_curcode->format == 6 ){
		/* �f�[�^���w�肵���o�̓t�H�[�}�b�g�ɉ��H���ĕ\�� */
		cp_ret = editprint( tp_curcode->outformat , tp_boccur->address );
		sprintf(cs_codes,"%s%s%s",tp_boccur->barline,tp_curcode->str,cp_ret);
	}

	if( gtp_gop->i_mmllevel <= tp_curcode->level ){
		/* �o�̓��x���ȏ�̂��̂������o�� */
		sprintf( gcs_line , "%s" , cs_codes );
		hfpf( DH_BMML , gcs_line );
	}

	return;
}
/*--------------------------------------------------------------------*/
/* �o�͌n */
/* cc����t�H�[�}�b�g�o�͏��� */
char *editprint( char *format , char *indata ){

	char *cp_curf;
static	char cs_outbuf[256];
	char cs_midbuf[4];
	char *cp_curo;
	int i_culc;
	int i_mid;
	char *cp_curd;
	char *cp_fmt;

	cp_curd = indata+1;
	cp_curf = format;
	memset(cs_outbuf,0x00,256);
	memset(cs_midbuf,0x00,4);
	cp_curo = cs_outbuf;

	for(cp_curf=format;*cp_curf!=0x00;cp_curf++){
		/* ���(���l�Z�o) */
		switch(*cp_curf){
			case '#':
			case '(':
				i_culc =  *cp_curd;
				break;
			case '$':
			case ')':
				i_culc =  *(cp_curd+1);
				i_culc =  i_culc * 256;
				i_culc += (int)(unsigned char)(*cp_curd);
				break;
			case '_':
				i_culc =  (int)(unsigned char)*cp_curd;
				break;
			case '~':
				i_culc =  (int)(unsigned char)*(cp_curd+1);
				i_culc =  i_culc * 256;
				i_culc += (int)(unsigned char)(*cp_curd);
				break;
			case ' ':
				i_culc = 0;
				break;
			case 'a':	case 'b':	case 'c':	case 'd':
			case 'e':	case 'f':	case 'g':	case 'h':
				/* �K���L�� �L���r�b�g���̎Z�o a=8/h=1 */
				i_mid = (int)(8- ( (*cp_curf) - 0x61 ));
				i_culc = powxy(2,i_mid) - 1;
				i_culc = i_culc & (int)(unsigned char)cp_curd;
				break;
			case 's':	case 't':	case 'u':	case 'v':
			case 'w':	case 'x':	case 'y':	case 'z':
				/* �����L�� �����r�b�g���̎Z�o z=0/s=7 */
				i_mid = (int)( 0x7A - (*cp_curf) );
				i_culc = powxy(2,i_mid) - 1;
				i_culc = (!i_culc) & (int)(unsigned char)cp_curd;
				break;
			default:
				i_culc = 0;
				break;
		}
		/* ���(�o�̓t�H�[�}�b�g) */
		switch(*cp_curf){
			case '(':
			case ')':
				cp_fmt = "%+d";
				break;
			case '#':
			case '$':
			case '_':
			case '~':
			case 'a':	case 'b':	case 'c':	case 'd':
			case 'e':	case 'f':	case 'g':	case 'h':
			case 's':	case 't':	case 'u':	case 'v':
			case 'w':	case 'x':	case 'y':	case 'z':
				cp_fmt = "%d";
				break;
			case ' ':
				cp_fmt = "";
				break;
			default:
				cs_midbuf[0] = *cp_curf;
				cs_midbuf[1] = 0;
				cp_fmt = cs_midbuf;
				break;
		}
		/* ���(�f�[�^�A�h���X�i�s) */
		switch(*cp_curf){
			case '(':
			case '#':
			case '_':
			case 'a':	case 'b':	case 'c':	case 'd':
			case 'e':	case 'f':	case 'g':	case 'h':
			case 's':	case 't':	case 'u':	case 'v':
			case 'w':	case 'x':	case 'y':	case 'z':
			case ' ':
				cp_curd++;
				break;
			case ')':
			case '$':
			case '~':
				cp_curd+=2;
				break;
			default:
				break;
		}
		sprintf(cp_curo,cp_fmt,i_culc);
		cp_curo+=strlen(cp_curo);
	}

	return(cs_outbuf);
}
/*--------------------------------------------------------------------*/
/* �o�͌n */
/* �m�[�gMML�o�͏��� */
void outcharnote( T_BON *tp_boncur , int i_pdeflen ){

	T_TCODE *tp_curcode;
	char cs_op[256];
	char cs_preoct[32];
	char cs_poroct[32];
	char c_octdiff;
	char *cp_curop;
	int i_tmp;
	int i_cnt1;
	unsigned char uc_curlen;
	int i_deflen;
	char c_connector='&';
	char c_addsymbol=DC_ADDSYMBOL;
	char cs_codes[256];

	cp_curop = cs_op;
	memset(cs_op,0x00,256);

	if( tp_boncur->note1 > 0 ){
		tp_curcode = codefinderbystr( &(tp_boncur->note1) );
		/* �������� */
		/* oct�̓|���^�����g�̂Ƃ��ɂ͓���ȎZ�o */
		if( tp_boncur->note2 != 0x00 ){
			/* �|���^�����g�̏ꍇ */
			c_octdiff = 1+(0x70&tp_boncur->note1)/0x10 -
				tp_boncur->befarea->oct;
		}else{
			c_octdiff = tp_boncur->oct - tp_boncur->befarea->oct;
		}
		if( c_octdiff != 0){
			/* oct�L�q */
			value2str_oct(c_octdiff,cs_preoct);
			sprintf(cp_curop,"%s",cs_preoct);
			cp_curop+=strlen(cp_curop);
		}
		if( tp_boncur->note2 != 0x00 ){
			/* �|���^�����g : poroct �|���^�����g����oct���Z�o */
			c_octdiff = tp_boncur->note2/0x10 - tp_boncur->note1/0x10;
			value2str_oct(c_octdiff,cs_poroct);
//			tp_boncur->oct += c_octdiff; /* ���̉����̂��߂�oct���� */
			sprintf(cp_curop,"{%s%s%s}",
				cps_note[tp_boncur->note1&0x0F],
				cs_poroct,
				cps_note[tp_boncur->note2&0x0F]);
			cp_curop+=strlen(cp_curop);
		}else{
			/* ���� or �x�� */
			i_tmp = (int)((char)(tp_boncur->note1)&(char)0x0F);
			sprintf(cp_curop,"%s",cps_note[i_tmp]);
			if( i_tmp == 0x0F ){
				c_connector = 'r';
			}else{
				c_connector = '&';
			}
			cp_curop+=strlen(cp_curop);
		}
		/* �����o�� */
/* �d�l�ύX�F */
/* i_pdeflen�Ƀ[�����Z�b�g���ꂽ�ꍇ�A�]���́u�f�t�H���g�l���̗p�v���� */
/* �u�W���������g�p���Ȃ��v�ɕύX (1.00c) */
		if( i_pdeflen != 0 ){
			i_deflen = i_pdeflen;
		}else{
			/* �ƂĂ��傫���l�ɂ���ΕW�������Ƃ��Ă͐������Ȃ��Ȃ邾�낤 */
			/* �Ȃ����f���Ȃ炳��Ɋm���͉����� */
			i_deflen = DN_LENGTH_DEFLIMIT;
		}
		if( tp_boncur->length == i_deflen ){
			/* nop l�R�}���h�ʂ�̉��� */
		}else if( tp_boncur->length%3 == 0 &&
			tp_boncur->length*2/3 == i_deflen ){
			/* nop l�R�}���h���{�t�_�̉��� */
			sprintf(cp_curop,".");
		}else if( tp_boncur->length%7 == 0 &&
			tp_boncur->length*4/7 == i_deflen ){
			/* nop l�R�}���h���{�Q�t�_�̉��� */
			sprintf(cp_curop,"..");
		}else if( tp_boncur->length%15 == 0 &&
			tp_boncur->length*8/15 == i_deflen ){
			/* nop l�R�}���h���{�R�t�_�̉��� */
			sprintf(cp_curop,"...");
		}else if( tp_boncur->length >= gtp_gop->i_lenmin ){
			/* MML�ŉ��y�I�\���ŋL�q���鉹���� */
			if( gi_zenlen < tp_boncur->length ){
				/* �S������蒷�� */
				sprintf(cp_curop,"%%%d",tp_boncur->length);
			}else if( gi_zenlen == tp_boncur->length ){
				/* �S�����Ɠ��� */
				sprintf(cp_curop,"1");
			}else if( gi_zenlen%tp_boncur->length == 0 ){
				/* ��t�_���� */
				sprintf(cp_curop,"%d",gi_zenlen/tp_boncur->length);
			}else if( tp_boncur->length % 9 == 0 &&
				/* �t�_���� */
				gi_zenlen%(tp_boncur->length*2/3) == 0 ){
				sprintf(cp_curop,"%d.",gi_zenlen/(tp_boncur->length*2/3));
			}else if( tp_boncur->length % 21 == 0 &&
				/* �Q�t�_���� */
				gi_zenlen%(tp_boncur->length*4/7) == 0 ){
				sprintf(cp_curop,"%d..",gi_zenlen/(tp_boncur->length*4/7));
			}else if( tp_boncur->length % 15 == 0 &&
				/* �R�t�_���� */
				tp_boncur->length / 15 * 16 == gi_zenlen ){
				sprintf(cp_curop,"%d...",gi_zenlen/(tp_boncur->length*8/15));
			}else if( tp_boncur->length % 5 == 0 &&
				tp_boncur->length / 5 * 8 == gi_zenlen ){
				/* �Q���{�W������ */
// 1.00b�Œ�����
//				/* �R���o�C���֎~���������֎~�Ƃ��Ĕ��� */
//				if( tp_boncur->nocombine != D_ON ){
//					if( i_deflen == tp_boncur->length / 5 ){
//						sprintf(cp_curop,"%c2",c_connector,tp_boncur->length);
//					}else{
						sprintf(cp_curop,"8%c2",c_addsymbol,tp_boncur->length);
//					}
//				}
			}else if( tp_boncur->length % 9 == 0 &&
				/* �Q���{�P�U������ */
				tp_boncur->length / 9 * 16 == gi_zenlen ){
// 1.00b�Œ�����
//				/* �R���o�C���֎~���������֎~�Ƃ��Ĕ��� */
//				if( tp_boncur->nocombine != D_ON ){
//					if( i_deflen == tp_boncur->length / 9 ){
//						sprintf(cp_curop,"%c2",c_connector,tp_boncur->length);
//					}else{
						sprintf(cp_curop,"16%c2",c_addsymbol,tp_boncur->length);
//					}
//				}
			}else if( tp_boncur->length % 5 == 0 &&
				/* �S���{�P�U������ */
				tp_boncur->length / 5 * 16 == gi_zenlen ){
// 1.00b�Œ�����
//				/* �R���o�C���֎~���������֎~�Ƃ��Ĕ��� */
//				if( tp_boncur->nocombine != D_ON ){
//					if( i_deflen == tp_boncur->length / 5 ){
//						sprintf(cp_curop,"&4",tp_boncur->length);
//					}else{
						sprintf(cp_curop,"16%c4",c_addsymbol,tp_boncur->length);
//					}
//				}
			}else{
				/* �t�_�ł͕\��������Ȃ������� */
				sprintf(cp_curop,"%%%d",tp_boncur->length);
			}
		}else if( tp_boncur->length % gtp_gop->i_len3min == 0 ){
			/* MML�ŉ��y�I�\���ŋL�q���鉹�����i�R�A���j */
			if( gi_zenlen < tp_boncur->length ){
				/* �S������蒷�� */
				sprintf(cp_curop,"%%%d",tp_boncur->length);
			}else if( gi_zenlen == tp_boncur->length ){
				/* �S�����Ɠ��� */
				sprintf(cp_curop,"1");
			}else if( gi_zenlen%tp_boncur->length == 0 &&
				tp_boncur->length >= gtp_gop->i_len3min ){
				/* ��t�_���� */
				sprintf(cp_curop,"%d",gi_zenlen/tp_boncur->length);
			}else{
				/* �i�R�A���ł͕t�_�������g��Ȃ��j */
				sprintf(cp_curop,"%%%d",tp_boncur->length);
			}
		}else{
			/* �p�[�Z���g�L���ŋL�q���鉹���� */
			sprintf(cp_curop,"%%%d",tp_boncur->length);
		}

		if( gtp_gop->i_mmllevel <= tp_curcode->level ){
			sprintf(gcs_line,"%s%s",tp_boncur->barline,cs_op);
			hfpf(DH_BMML,gcs_line);
			if( tp_boncur->note1 < 0x7F && tp_boncur->ties == (char)'&' ){
				/* '&'�L�����������̏ꍇ(���ߐ���Ő؂����ꍇ�Ȃ�) */
				/* �������A�x���͏��� */
				if( (char)(tp_boncur->note1&0x0F) != (char)0x0F ){
					sprintf(gcs_line,"%c",tp_boncur->ties);
					hfpf(DH_BMML,gcs_line);
				}
			}
		}

	}else{
		/* control-code �T�[�` */
		tp_curcode = codefinderbystr( &(tp_boncur->note1) );
		if( tp_curcode == NULL ){
			/* �� */
			return;}
		if( tp_curcode->format == 0 ||
			tp_curcode->format == 1 ||
			tp_curcode->format == 2 ){
			/* ���� */
			return;
		/* V1.21 2008/7/14 start �]���������ɔ��f������ */
		}else if( gtp_gop->c_autotrans == D_ON &&
			(tp_curcode->code1==(char)0xF5||tp_curcode->code1==(char)0xE7 ) ){
			/* c_autotrans��ON�ł���A��Γ]��/���Γ]���̏ꍇ�̓X�L�b�v */
			return;
		/* V1.21 2008/7/14 end */
		}else if( tp_curcode->format == 3 ){
			/* �����\�����邾�� */
			sprintf(cs_codes,"%s%s",tp_boncur->barline,tp_curcode->str);
		}else if( tp_curcode->format == 4 ){
			/* �f�[�^(1bytes)�����̂܂ܕ\�� */
			sprintf(cs_codes,"%s%s%d",tp_boncur->barline,tp_curcode->str,
				tp_boncur->note2);
		}else if( tp_curcode->format == 5 ){
			/* �f�[�^(1bytes)�����̂܂ܕ\��(�����Ȃ�) */
			sprintf(cs_codes,"%s%s%d",tp_boncur->barline,tp_curcode->str,
				(unsigned char)tp_boncur->note2);
		}else if( tp_curcode->format == 7 ){
			/* �f�[�^(1bytes)�𕄍������\�� */
			sprintf(cs_codes,"%s%s%+d",tp_boncur->barline,tp_curcode->str,
				tp_boncur->note2);
		}else if( tp_curcode->format == 8 ){
			/* �f�[�^(1bytes)�𕄍��t�[���������Ȃ��\�� */
			sprintf(cs_codes,"%s%s%.d",tp_boncur->barline,tp_curcode->str,
				tp_boncur->note2);
		}

		if( gtp_gop->i_mmllevel <= tp_curcode->level ){
			/* �����̂Ȃ����͕̂\�����Ȃ��ꍇ������ */
			sprintf(gcs_line,"%s%s",tp_boncur->barline,cs_codes);
			hfpf(DH_BMML,gcs_line);
		}
	}

	return;
}
/*--------------------------------------------------------------------*/
/* �o�͌n */
/* �I�N�^�[�u������ҏW���� +�Ȃ�> -�Ȃ�< �����Aoct�L���������o�� */
void value2str_oct( char c_in , char *cp_in ){

	int i_repeat;
	char c_outchar;

	if( c_in < 0 ){
		c_outchar = (char)'<';
		i_repeat = (int)(0-c_in);
	}else if( c_in > 0){
		c_outchar = (char)'>';
		i_repeat = (int)c_in;
	}else{
		*cp_in = 0x00;
		return;
	}

	cp_in[i_repeat] = 0x00;
	for(;i_repeat!=0;i_repeat--){
		cp_in[i_repeat-1] = c_outchar;
	}

	return;
}
/*--------------------------------------------------------------------*/
/* �o�͌n */
/* ��`�f�[�^���o�͂��� */
void defineout( ){

	int i_count;


	if( gt_ginfo.cp_title != NULL &&
		((gtp_gop->i_definemode)&DA_DEFL_NORMAL) != 0 ){
		sprintf( gcs_line , DS_DEFINE_TITLE , gt_ginfo.cp_title );
		hfpf( DH_BDEFINE , gcs_line );
	}
	if( gt_ginfo.cp_composer != NULL &&
		((gtp_gop->i_definemode)&DA_DEFL_NORMAL) != 0 ){
		sprintf( gcs_line , DS_DEFINE_COMPOSER , gt_ginfo.cp_composer );
		hfpf( DH_BDEFINE , gcs_line );
	}
	if( gt_ginfo.cp_arrenger != NULL &&
		((gtp_gop->i_definemode)&DA_DEFL_NORMAL) != 0 ){
		sprintf( gcs_line , DS_DEFINE_ARRENGER , gt_ginfo.cp_arrenger );
		hfpf( DH_BDEFINE , gcs_line );
	}
	if( gt_ginfo.i_fPPZExtend == D_ON &&
		((gtp_gop->i_definemode)&DA_DEFL_PARTEXTEND) != 0 ){
		strcpy( gcs_line , DS_DEFINE_PPZEXTEND );
		hfpf( DH_BDEFINE , gcs_line );
	}
	if( gt_ginfo.i_fFM3Extend == D_ON &&
		((gtp_gop->i_definemode)&DA_DEFL_PARTEXTEND) != 0 ){
		strcpy( gcs_line , DS_DEFINE_FM3EXTEND );
		hfpf( DH_BDEFINE , gcs_line );
	}
	if( gt_ginfo.cp_pziname != NULL &&
		((gtp_gop->i_definemode)&DA_DEFL_NORMAL) != 0 ){
		sprintf( gcs_line , DS_DEFINE_PZINAME , gt_ginfo.cp_pziname );
		hfpf( DH_BDEFINE , gcs_line );
	}
	if( gt_ginfo.cp_ppsname != NULL &&
		((gtp_gop->i_definemode)&DA_DEFL_NORMAL) != 0 ){
		sprintf( gcs_line , DS_DEFINE_PPSNAME , gt_ginfo.cp_ppsname );
		hfpf( DH_BDEFINE , gcs_line );
	}
	if( gt_ginfo.cp_ppcname != NULL &&
		((gtp_gop->i_definemode)&DA_DEFL_NORMAL) != 0 ){
		sprintf( gcs_line , DS_DEFINE_PPCNAME , gt_ginfo.cp_ppcname );
		hfpf( DH_BDEFINE , gcs_line );
	}

	/* MEMO���͕������蓾�� */
	if( (gtp_gop->i_definemode)&DA_DEFL_NORMAL != 0 ){
		for( i_count=0 ; i_count<DN_MAX_MEMOPOINT ; i_count++ ){
			if( gt_ginfo.cps_memo[i_count] != NULL ){
				sprintf( gcs_line , DS_DEFINE_MEMO ,
					gt_ginfo.cps_memo[i_count] );
				hfpf( DH_BDEFINE , gcs_line );
			}else{
				break;
			}
		}
	}

	return;
}
/*--------------------------------------------------------------------*/
/* �_���v�o�͏��� */
/*--------------------------------------------------------------------*/
/* �o�͌n */
/* �{�C�X�f�[�^���_���v���� */
void voiceout( ){

	char *cp_vcur;
	T_FMT t_fmtcur;
	int i_cnt;
	int is_op[4];
const int is_soptx[4]={3,1,2,0};
const int is_soppmd[4]={0,2,1,3};
	int i_fmnum;
	int i_fmalg;
	int i_fmfb;
	T_OFMT t_txvoice[4];
	T_IFMT t_voice[4];

	if( cp_voiceaddr == NULL ){
		/* �{�C�X�A�h���X�����炸���{�C�X�f�[�^�Ȃ��f�[�^ */
		sprintf(gcs_line," [DLL] not built in voice-datas.\n");
		hfpf( DH_BMESSAGES , gcs_line );
		return;
	}

	switch( gtp_gop->i_voicemode ){
		case D_ON:
			memcpy(is_op,is_soppmd,sizeof(int)*4);
			break;
		case DA_TX81ZVOICE:
			memcpy(is_op,is_soptx,sizeof(int)*4);
			break;
		default:
			return;
	}


	cp_vcur = cp_voiceaddr;
	while((*cp_vcur!=0x00)||(*(cp_vcur+1)!=-1)){
		/* �擪�̉��F�ԍ��Ƀ[����������vocedatas�I���̃|�C���g */
		/* V1.01a�C�� 0x00/0xFF�̂Q�o�C�g��������I�� */
		memcpy(&t_fmtcur,cp_vcur,sizeof(T_FMT));

		/* FM���F�𐮗� */
		i_fmnum = (int)(t_fmtcur.tnumber);
		i_fmalg = (int)(t_fmtcur.fbal & 0x07);
		i_fmfb  = (int)((t_fmtcur.fbal & 0x38)/0x08);

		/* ���ʍ��� */
		if( gtp_gop->i_voicemode == DA_TX81ZVOICE ){
			/* TX81Z�^ */
			sprintf(gcs_line," @%03d\n",i_fmnum);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," Algorithm   ALG   %2d\n",i_fmalg+1);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," FeedBack    FB    %2d\n",i_fmfb);
			hfpf(DH_BVOICE,gcs_line);
		}else if( gtp_gop->i_voicemode == D_ON ){
			/* �]��PMD�^ */
			sprintf(gcs_line,"; NM  AG  FB\n");
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line,"@%03d %3d %3d\n", i_fmnum , i_fmalg , i_fmfb );
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line,"; AR  DR  SR  RR  SL  TL  KS  ML  DT AMS\n");
			hfpf(DH_BVOICE,gcs_line);
		}

		/* �e�l���F����荞��œ����t�H�[�}�b�g�ɕϊ� */
		for(i_cnt=0;i_cnt<4;i_cnt++){
			t_voice[i_cnt].i_ml =
				(int)(t_fmtcur.dtml[is_op[i_cnt]]&0x00F);
			t_voice[i_cnt].i_dt =
				(int)((t_fmtcur.dtml[is_op[i_cnt]]&0x0F0)/0x10);
			t_voice[i_cnt].i_ar =
				(int)(t_fmtcur.ksar[is_op[i_cnt]]&0x01F);
			t_voice[i_cnt].i_dr =
				(int)(t_fmtcur.amdr[is_op[i_cnt]]&0x01F);
			t_voice[i_cnt].i_sl =
				(int)((t_fmtcur.slrr[is_op[i_cnt]]&0x0F0)/0x10);
			t_voice[i_cnt].i_sr =
				(int)(t_fmtcur.sr[is_op[i_cnt]]);
			t_voice[i_cnt].i_rr =
				(int)(t_fmtcur.slrr[is_op[i_cnt]]&0x00F);
			t_voice[i_cnt].i_tl =
				(int)(t_fmtcur.tl[is_op[i_cnt]]);
			t_voice[i_cnt].i_ks =
				(int)((t_fmtcur.ksar[is_op[i_cnt]]&0x0C0)/0x40);
			t_voice[i_cnt].i_ams =
				(int)(( t_fmtcur.amdr[is_op[i_cnt]] & 0x0E0 ) / 0x20);
		}

		if( gtp_gop->i_voicemode == D_ON ){
			/* �]��PMD�^ */
			for( i_cnt=0 ; i_cnt<4 ; i_cnt++ ){
				sprintf(gcs_line," %3d %3d %3d %3d %3d %3d %3d %3d %3d %3d\n",
					t_voice[i_cnt].i_ar ,
					t_voice[i_cnt].i_dr ,
					t_voice[i_cnt].i_sr ,
					t_voice[i_cnt].i_rr ,
					t_voice[i_cnt].i_sl ,
					t_voice[i_cnt].i_tl ,
					t_voice[i_cnt].i_ks ,
					t_voice[i_cnt].i_ml ,
					t_voice[i_cnt].i_dt ,
					t_voice[i_cnt].i_ams );
				hfpf(DH_BVOICE,gcs_line);
			}
		}else if( gtp_gop->i_voicemode == DA_TX81ZVOICE ){
			/* TX81Z�^ */
			for( i_cnt=0 ; i_cnt<4 ; i_cnt++ ){
				if( t_voice[i_cnt].i_ml == 0 ){
					/* Frequency : 0��0.5�ɕϊ� */
					t_txvoice[i_cnt].f_freq=(float)0.5;
				}else{
					t_txvoice[i_cnt].f_freq=
						(float)(t_voice[i_cnt].i_ml);
				}
				t_txvoice[i_cnt].i_det =
					t_voice[i_cnt].i_dt ;
				if( t_txvoice[i_cnt].i_det >= 4 ){
					/* Detune : 4�`7��0�`-3�ɕϊ� */
					t_txvoice[i_cnt].i_det =
						4 - t_txvoice[i_cnt].i_det;
				}
				t_txvoice[i_cnt].i_ar  = t_voice[i_cnt].i_ar;
				t_txvoice[i_cnt].i_d1r = t_voice[i_cnt].i_dr;
				t_txvoice[i_cnt].i_d1l = t_voice[i_cnt].i_sl;
				t_txvoice[i_cnt].i_d2r = t_voice[i_cnt].i_sr;
				t_txvoice[i_cnt].i_rr  = t_voice[i_cnt].i_rr;
				t_txvoice[i_cnt].i_out =
					(127-t_voice[i_cnt].i_tl) * 99 / 127;
				t_txvoice[i_cnt].i_rs  = t_voice[i_cnt].i_ks;
			}
			/* TX81Z�^ */
			sprintf(gcs_line," Frequency   FREQ  %4.1f %4.1f %4.1f %4.1f\n",
				t_txvoice[0].f_freq,t_txvoice[1].f_freq,
				t_txvoice[2].f_freq,t_txvoice[3].f_freq);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," Detune      DET   %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_det,t_txvoice[1].i_det,
				t_txvoice[2].i_det,t_txvoice[3].i_det);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," AttackRate  AR    %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_ar,t_txvoice[1].i_ar,
				t_txvoice[2].i_ar,t_txvoice[3].i_ar);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," Decay1Rate  D1R   %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_d1r,t_txvoice[1].i_d1r,
				t_txvoice[2].i_d1r,t_txvoice[3].i_d1r);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," Decay1Level D1L   %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_d1l,t_txvoice[1].i_d1l,
				t_txvoice[2].i_d1l,t_txvoice[3].i_d1l);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," Decay2Rate  D2R   %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_d2r,t_txvoice[1].i_d2r,
				t_txvoice[2].i_d2r,t_txvoice[3].i_d2r);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," ReleaseRate RR    %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_rr,t_txvoice[1].i_rr,
				t_txvoice[2].i_rr,t_txvoice[3].i_rr);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," OutputLevel OUT   %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_out,t_txvoice[1].i_out,
				t_txvoice[2].i_out,t_txvoice[3].i_out);
			hfpf(DH_BVOICE,gcs_line);
			sprintf(gcs_line," K.RateScale RS    %2d   %2d   %2d   %2d\n",
				t_txvoice[0].i_rs,t_txvoice[1].i_rs,
				t_txvoice[2].i_rs,t_txvoice[3].i_rs);
			hfpf(DH_BVOICE,gcs_line);
		}else{
			/* ���ɂȂ� */
		}

		cp_vcur += ( sizeof(T_FMT)-1 );
		sprintf(gcs_line,"\n");
		hfpf(DH_BVOICE,gcs_line);
	}

	return;
}
/*--------------------------------------------------------------------*/
/* �o�͌n */
/* �P�Z���e���X���_���v���� */
void dump1op( char *cp_in ){

	T_TCODE *tp_cur;
static char c_partname=(char)' ';
static int i_numonline=0;
static int i_linenum=0;
	int i_curcodelen;

	if( cp_in == NULL ){
		/* �g���b�N�I�[ */
		if( i_numonline >= 15 ){
			sprintf(gcs_line,"\n%03X| %80\n\n",i_linenum);
		}else if( i_numonline == 0 ){
			sprintf(gcs_line,"%03X| 80\n\n",i_linenum);
		}else{
			sprintf(gcs_line,"80\n\n");
		}
		hfpf( DH_BDUMP , gcs_line );
		return;
	}

	if( c_partname != tp_curtra->partname ){
		/* ������ */
		c_partname = tp_curtra->partname;
		i_numonline = 0;
		i_linenum = 0;
		sprintf(gcs_line,"==========================================================\n");
		hfpf( DH_BDUMP , gcs_line );
		sprintf(gcs_line," %c |  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10\n",c_partname);
		hfpf( DH_BDUMP , gcs_line );
		sprintf(gcs_line,"---+----------------------------------------------------\n");
		hfpf( DH_BDUMP , gcs_line );
	}

	tp_cur = codefinderbystr( cp_in );
	if( tp_cur->code2 == 0x00 ){
		/* �P�o�C�gcode */
		if( i_numonline == 15 ){
			sprintf(gcs_line,"%02X\n",0x000000FF&*cp_in);
			i_linenum++;
			i_numonline = 0;
		}else if( i_numonline == 0 ){
			sprintf(gcs_line,"%03X| %02X ",i_linenum,0x000000FF&*cp_in);
			i_numonline++;
		}else{
			sprintf(gcs_line,"%02X ",0x000000FF&*cp_in);
			i_numonline++;
		}
	}else{
		/* �Q�o�C�gcode */
		if( i_numonline == 15 ){
			sprintf(gcs_line,"%02X-%02X\n%03X|z    "
				,0x000000FF&*cp_in,0x000000FF&*(cp_in+1),i_linenum+1);
			i_numonline = 1;
			i_linenum++;
		}else if( i_numonline == 14 ){
			sprintf(gcs_line,"%02X-%02X\n",0x000000FF&*cp_in,
				0x000000FF&*(cp_in+1));
			i_numonline = 0;
			i_linenum++;
		}else if( i_numonline == 0 ){
			sprintf(gcs_line,"%03X| %02X-%02X ",i_linenum
				,0x000000FF&*cp_in,0x000000FF&*(cp_in+1));
			i_numonline = 2;
		}else{
			sprintf(gcs_line,"%02X-%02X ",0x000000FF&*cp_in,
				0x000000FF&*(cp_in+1));
			i_numonline+=2;
		}
	}
	hfpf( DH_BDUMP , gcs_line );

	return;
}
/*--------------------------------------------------------------------*/
/* �o�͌n */
/* �P�Z���e���X���X�[�p�[�_���v���� */
void dumps1op( char *cp_in ){

	T_TCODE *tp_cur;
	int i_curcode;
	int i_pnote;
	int i_ret;
	unsigned char uc_ret;
	char *cp_curstr;
	char cs_outbuf[256];
	char *cp_ret;
static long l_totalclock=0;
static char c_partname=' ';
	char c_octdiff;
	char cs_poroct[32];
	char cs_pormml[32];


	if( cp_in == NULL ){
		/* �g���b�N�I�[ */
		l_totalclock = tracklen( D_OP_TLTOTAL , 0 );
		sprintf(gcs_line," 80    |   |%5d|code trackend\n",
			l_totalclock);
		hfpf( DH_BSDUMP , gcs_line );
		return;
	}

	if( c_partname != tp_curtra->partname ){
		/* ������ */
		c_partname = tp_curtra->partname;
		sprintf(gcs_line,"PART:%c ==========================================================\n",c_partname);
		hfpf( DH_BSDUMP , gcs_line );
		sprintf(gcs_line," codes |len| pos |     MML\n");
		hfpf( DH_BSDUMP , gcs_line );
	}

	tp_cur = codefinderbystr( cp_in );
	if( (unsigned char)0x7F >= (unsigned char)(tp_cur->code1) ){
		/* note */
		i_pnote = (int)0x0000000F&(int)(*cp_in);
		i_ret = tracklen( D_OP_TLLEVEL , 0 );
		if( i_ret == 0 ){
			l_totalclock = tracklen( D_OP_TLTOTAL , 0 );
			sprintf(cs_outbuf," %02X    |%3d|%5d|     %2s %3d\n",
				(char)*cp_in,
				(unsigned char)*(cp_in+1),
				l_totalclock,
				cps_note[i_pnote],
				(unsigned char)*(cp_in+1) );
		}else{
			sprintf(cs_outbuf," %02X    |%3d|     |     %2s %3d\n",
				(char)*cp_in,
				(unsigned char)*(cp_in+1),
				cps_note[i_pnote],
				(unsigned char)*(cp_in+1) );
		}
	}else if( (unsigned char)0xDA == (unsigned char)(tp_cur->code1) ){
		/* �|���^�����g */

		c_octdiff = *(cp_in+1)/0x10 - *(cp_in+2)/0x10;
		value2str_oct(c_octdiff,cs_poroct);
		sprintf(cs_pormml,"{%s%s%s}",
			cps_note[*(cp_in+1)&0x0F],
			cs_poroct,
			cps_note[*(cp_in+2)&0x0F]);

		i_ret = tracklen( D_OP_TLLEVEL , 0 );
		i_curcode = (int)(*cp_in)&(int)0x000000FF;
		if( i_ret == 0 ){
			l_totalclock = tracklen( D_OP_TLTOTAL , 0 );
			sprintf(cs_outbuf," %02X    |%3d|%5d|%7s %3d\n",
				i_curcode,
				(unsigned char)*(cp_in+3),
				l_totalclock,
				cs_pormml,
				(unsigned char)*(cp_in+3) );
		}else{
			sprintf(cs_outbuf," %02X    |%3d|     |%7s %3d\n",
				i_curcode,
				(unsigned char)*(cp_in+3),
				cs_pormml,
				(unsigned char)*(cp_in+3) );
		}
	}else{
		/* code */
		i_curcode = (int)tp_cur->code1&(int)0x000000FF;
		cp_curstr = tp_cur->str;
		if( *cp_in == (char)0xF9 ){
			/* �n�_ */
		}else if( *cp_in == (char)0xF8 ){
			/* �I�_ */
			uc_ret = (unsigned char)(*(cp_in+1));
		}else if( *cp_in == (char)0xF6 ){
			/* L */
		}else if( *cp_in == (char)0xF7 ){
			/* : */
		}
		if( (char)*cp_in == (char)0xF8 || (char)*cp_in == (char)0xF9 ){
			i_ret = tracklen( D_OP_TLLEVEL , 0 );
			if( i_ret == 0 && (char)*cp_in == (char)0xF8 ){
				l_totalclock = tracklen( D_OP_TLTOTAL , 0 );
				sprintf(cs_outbuf," %02X    |   |%5d|code %s%d\n",
					i_curcode,
					l_totalclock,
					cp_curstr,
					(unsigned char)*(cp_in+1));
			}else if( i_ret == 1 && (char)*cp_in == (char)0xF9){
				l_totalclock = tracklen( D_OP_TLTOTAL , 0 );
				sprintf(cs_outbuf," %02X    |   |%5d|code %s\n",
					i_curcode,
					l_totalclock,
					cp_curstr);
			}else if( (char)*cp_in == (char)0xF8 ){
				sprintf(cs_outbuf," %02X    |   |     |code %s%d\n",
					i_curcode,
					cp_curstr,
					(unsigned char)*(cp_in+1));
			}else if( (char)*cp_in == (char)0xF9 ){
				sprintf(cs_outbuf," %02X    |   |     |code %s\n",
					i_curcode,
					cp_curstr);
			}
		}else{
			switch(tp_cur->format){
				case 0:
				case 1:
					sprintf(cs_outbuf," %02X    |   |     |code %s\n",
						i_curcode,
						cp_curstr);
					break;
				case 3:
					sprintf(cs_outbuf," %02X    |   |     |code %s\n",
						i_curcode,
						cp_curstr);
					break;
				case 4:
				case 7:
					sprintf(cs_outbuf," %02X    |   |     |code %s%d\n",
						i_curcode,
						cp_curstr,
						(char)*(cp_in+1));
					break;
				case 5:
					sprintf(cs_outbuf," %02X    |   |     |code %s%d\n",
						i_curcode,
						cp_curstr,
						(unsigned char)*(cp_in+1));
					break;
				case 8:
					sprintf(cs_outbuf," %02X    |   |     |code %s%.d\n",
						i_curcode,
						cp_curstr,
						(unsigned char)*(cp_in+1));
					break;
				case 6:
					cp_ret = editprint( tp_cur->outformat , cp_in );
					sprintf(cs_outbuf," %02X    |   |     |code %s%s\n",
						i_curcode,
						cp_curstr,
						cp_ret );
					break;
				default:
					sprintf(cs_outbuf," %02X    |   |     |\n",i_curcode);
					break;
			}
		}
	}

	sprintf(gcs_line,cs_outbuf);
	hfpf( DH_BSDUMP , gcs_line );

	return;
}
