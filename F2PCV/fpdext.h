/**********************************************************************/
/* F2PCV - FMP -> P.M.D. data converter by JUD(T.Terata)              */
/* Exturnals                                                          */
/**********************************************************************/
#ifndef FPDEXT_H
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

extern char *DS_PMD_ONLYTERMINATION;

extern T_F2PIF *gtp_gop;
extern char gcs_line[DN_SIZE_LINEBUFFER];
extern char *gcp_comment;
extern char *gcp_FMparam;
extern char gcs_wbuf[DN_SIZE_VOICEPMD*256];

extern T_FPARTSTATE ts_tsf[DN_PART_FELEM];
extern T_PPARTSTATE ts_tsp[DN_PART_PDELEM];

extern T_TCODE ts_tc[DN_MAX_CONTROLCODE];


int makepmdhead( char * );
int outpmd( int , char );
int anlmain( T_F2PIF * );
int convertpart( T_FPARTSTATE * );
int getflen( char * , int );
int searchloop( T_FPARTSTATE * );
int readheader( char * );
int makepmdfoot( char * , int , int );
char *stackaddr( char * );
int powxy( int,int );
void *apmalloc( int );
int apfree( void * );
char *hfpf( int , char * );
T_TCODE *searchbycode( char * , int );
int outsporta( char *, int , int );
int outsloopend( char *, int );
int analdatatype( char * );
int getFMparam( char * );


#endif
#define FPDEXT_H
