#ifndef COMRCIF_H
/**********************************************************************/
/* P.M.D. reversal compiler interface for PMDRC.DLL                   */
/**********************************************************************/

#ifndef JST_D_ONOFF
#define D_ON 1
#define D_OFF 0
#endif
#define JST_D_ONOFF

/*--------------------------------------------------------------------*/
/* ê›íËã§í íËêî */
#define DA_ON 1
#define DA_OFF 0
#define DA_AUTO 1
#define DA_FORCED 2
#define DA_DEBUG 128

/* ÇcÇkÇkåãâ íËêî */
#define DR_NORMAL 0			/* Normal */
#define DR_E_FATALERROR -1	/* Fatal error in DLL */
#define DR_E_NEIFVERSION -2	/* Not equal IF-version */
#define DR_E_NSAPVERSION -3	/* Not support AP-version */
#define DR_E_NGIF -4		/* NG Interface */
#define DR_E_BADDATA -5		/* Error in data */
#define DR_E_NSDTVERSION -6	/* Not support data-version */
#define DR_E_SHORTBUF -7	/* Buffer size was too short to texts */
#define DR_W_BADMML 1		/* Incompleteness MML */

#endif
#define COMRCIF_H
