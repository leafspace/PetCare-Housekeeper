/*****************************************************************************
 *
 *	ftpclient.c :
 *		main source file transfer task
 *
 *	Copyright 2004 - 2008 Brother Industries , Ltd
 *
 *	Aug.08 2008 : ABS : created for FTP/Network(CIFS) spec.
 *
 *	$Id: //depot/Firm/Commonfile/BC2/base/fb_sst_cloud_bsi/Laser/task/ftpclient/ftpclient.c#1 $
 *
 *****************************************************************************/



/***************************** include files *********************************/
/* general                   */
#include	"spec.h"
#include	"stdtype.h"
#include	"message.h"
#include	"fos.h"
#include	"debug.h"

/* about display & key-input */

#ifndef	USE_SEPARATE_UI
#include	"lib/displib/dispcntllib.h"
#include	"lib/displib/dispstr.h"
#include	"lib/displib/dispcom.h"
#include	"lib/string/stringid.h"
#endif	/* USE_SEPARATE_UI */
#include 	"task/panel/soundlib.h"
#ifdef USE_BSI
#include	"componentlib/serio/serio_log_rec.h"
#endif /* USE_BSI */
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
#include 	"lib/seriocn/serio_connector.h"
#include	"lib/seriocn/serio_task_common.h"
#include 	"lib/seriocn/object_instance.h"
#include 	"lib/seriocn/ObserverSubject.h"
#include 	"lib/seriocn/serio_seriomfp_app.h"
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
#include	"componentlib/keylib/aplid.h"
#include	"componentlib/jobcancel/jobcancel.h"
#include	"componentlib/state/statecntl.h"
#include	"componentlib/string/chartable.h"
#include	"componentlib/string/stringcntl.h"
/* about settings            */
#include	"driver/e2prom/e2prom.h"
#include	"lib/funcset/funcset.h"
#include	"lib/modellib/modelget.h"
#include	"componentlib/objacc/objacc.h"

/* about scan & memory       */
#include	"modeltable/sysmemtable/sysmemModel.h"
#include	"componentlib/resource/resource.h"
#include	"scanning/scan/scanTask.h"
#include	"subos/imagelib/image_lib.h"
#include	"subos/sysmem/sysmemLib.h"

/* about LAN                 */
#include	"lan/net_if/bn_ftpc.h"

#ifdef	USE_T38
#include "tsushin/t38fax/t38fax_apl.h"
#endif

/* in module                 */
#include	"ftpclient.h"
#include	"ftpclient_disp.h"

#include	"componentlib/objacc/func_objacc.h"
#include	"lib/funcset/funcsetcntl.h"
#include	"driver/scan/scan.h"

#include	"lib/scanning_sub/doc_scan_area_spec.h"

#ifdef	USE_SEPARATE_UI
#include	"lib/cplib/cp_sts_apl_sta.h"
#include	"lib/cplib/cp_api_lib.h"
#include 	"task/pc_scanner/pcScan_lib.h"
#endif	/* USE_SEPARATE_UI */

#include "task/pc_scanner/pcScanUif.h"

#if defined(USE_SCAN2FTP) || defined(USE_SCAN2NW) || defined(USE_SCAN2USB)
/*----------------------------------------------------------------------------*/
/* LOCAL DEFINES                                                              */
/*----------------------------------------------------------------------------*/
#define	MSG_QUE_SIZE		(12)             /* ��M���b�Z�[�W�̍ő咷        */
#define	NO_SYSMEMID			(-1)             /* SYSMEM ID���Ȃ��ꍇ           */
#define	FTPFLNAME_MAX		(256)            /* �t�@�C�����̍ő啶����        */
#define SCAN2FTP_SERVICE    (0)              /* Service:Scan to FTP           */
#define SCAN2CIFS_SERVICE   (1)              /* Service:Scan to CIFS          */
#ifdef	USE_SCAN2SFTP
#define	SCAN2SFTP_SERVICE	(2)              /* Service:Secure Scan to FTP    */
#endif	/* USE_SCAN2SFTP */

#define JPEG_SCAN_CNT_SIZE	(10 + 1 + 1)
                                      /* 10(UINT32:10��) + 1('_') + 1(\0)     */
#define	FILETYPE_JPG	    "jpg"

#ifdef  FUNC_ADF_SCAN_SPEC_SHEETS_STOP
#if         RT_MEM_SCAN_LIMITEND == RT_MEM_SCAN_ALLEND
#undef          RT_MEM_SCAN_LIMITEND
#define         RT_MEM_SCAN_LIMITEND RT_SCAN_LIMITEND
#endif      /* RT_MEM_SCAN_LIMITEND       */
#endif  /* FUNC_ADF_SCAN_SPEC_SHEETS_STOP */

#define BitOn(x, y)         x |= (y)
#define BitOff(x, y)        x &= ~(y)

#define FTPC_DEFAULT_RESO		RESOLUTION_300DPI
#define FTPC_DEFAULT_COLOR		COLOR_MODE_BW

#ifdef FTPC_COMPLETE_SEQUENCE
#define	COMPLETE_TIMEOUT			6000		/* 60�b( 10ms�~6000 ) */
#endif /* FTPC_COMPLETE_SEQUENCE */

/*----------------------------------------------------------------------------*/
/* STRUCTURES                                                                 */
/*----------------------------------------------------------------------------*/
typedef struct
{
	SCAN2FTP_QUALITY		quality;         /* (common) �掿                 */
	SCAN2FTP_FILEFORMAT		fileformat;      /* (common) �t�@�C���t�H�[�}�b�g */
	SCAN2FTP_DUALSCAN		dualscan;        /* (common) ���ʓǂݎ��         */
	SCAN2FTP_FNAMETYPE		filenametype;    /* (common) �擪������̃^�C�v   */
                                             /* (FTP)    FTP Server ��        */
	UINT8					servername   [ SCAN2FTP_SVR_MAXLEN      +1 ];
	                                         /* (common) �T�[�o�[��           */
	UINT8					serveraddress[ FTPCLIB_HOSTADDR_SIZE   +1 ];
                                             /* (common) CIFS ServerAddress   */
	UINT8					username     [ CIFSCLIB_USERNAME_SIZE  +1 ];
                                             /* (common) ���[�U�[��           */
	UINT8					password     [ CIFSCLIB_PASSWORD_SIZE  +1 ];
                                             /* (common) PASSWORD             */
	UINT8					storedir     [ CIFSCLIB_DIRECTORY_SIZE +1 ];
                                             /* (common) �i�[DIRECTORY        */
	UINT8					filename     [ SCAN2FTP_FNAME_MAXSIZE];
                                             /* (common) �t�@�C����           */
	UINT8					AuthenticationMethod;
                                             /* (CIFS/SFTP)   �F�ؕ��@             */
	UINT8					KerberosServerAddress
                                         [ CIFSCLIB_KERBADDR_SIZE  +1 ];
                                             /* (CIFS)   �P���x���X�T�[�o��   */
                                             /*          �܂���IP�A�h���X     */
    UINT8                   ispassive;       /* (FTP)    Passive or active    */
    UINT32                  portnum;         /* (FTP)    �|�[�g�ԍ�           */
                                             /* SPDF�p�X���[�h                */
    UINT8                   spdfpass     [ FTPC_SPDF_PASSWD_MAXLEN +1 ];
    UINT8                   scan_quality;    /* Scan���k��                    */
#ifdef USE_TMP_SCANSIZE
	UINT8                   scan_doc_size;   /* (Common)Glass Scan Size       */
#endif /* USE_TMP_SCANSIZE */
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	UINT8               	scan_src;
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
	UINT8					scan_multifeed_detect;							/* �d�����m ON/OFF */
	UINT8					scan_blank_detect;								/* ���������@�\ ON/OFF */ 
	UINT8					scan_deskew_adjust;								/* �΍s�␳ ON/OFF */ 
	UINT8					scan_quality_gray;								/* �F��Auto����Gray���k�� */
#ifdef    USE_GNDCOLOR_REMOVAL
	UINT8					gndcolor_removal;								/* �J���[�n�F����		*//* M-BHS13-648 */
	UINT8					gndcolor_level;									/* �J���[�n�F�����ݒ�	*//* M-BHS13-648 */
#endif /* USE_GNDCOLOR_REMOVAL */
#ifdef	USE_SCAN2SFTP
	INT32					PubKeyIdx;
	INT32					PairKeyIdx;
#endif	/* USE_SCAN2SFTP */
	UINT8					FileNameFixed;
}FTP_CIFSACCESS_INFO;

/* ��M���b�Z�[�W�p���p�� */
typedef union
{
	ftpclt_msg					ftpclt_tskmsg;
	macstatemsg_t				macstate_msg;
	scannertask_retun_msg_t		scan_msg;
}ftpclt_get_msg;

/**
* @struct	FTPC_PGCNT_ACT
* @par		Scan�����J�E���^�֐�
* @par		ftpclient_ScanPage / ftpclient_BlankPage �������̒�`
*/
typedef enum {
	FTPC_PGCNT_INIT		= 1,	/* ������				*/
	FTPC_PGCNT_START,			/* �ǂݎ��J�n�l�̐ݒ�	*/
	FTPC_PGCNT_UNUSE,			/* �y�[�W�J�E���g���Ȃ�	*/
	FTPC_PGCNT_INCREMENT,		/* 1�œǂݎ�芮��		*/
	FTPC_PGCNT_END,				/* �ǂݎ��I��			*/
	FTPC_PGCNT_GET,				/* �y�[�W�J�E���g�擾	*/
} FTPC_PGCNT_ACT;


typedef struct
{
	SCAN2FTP_QUALITY		ftp_quality;			/* FTP Quality */
	UINT16					resolution;				/* doc_scan_area_spec.h�Œ�`����Ă���u�𑜓x�v*/
	UINT8					color_num;				/* doc_scan_area_spec.h�Œ�`����Ă���u�F���v*/
} FTPC_CONV_TBL;
	
	
	
const FTPC_CONV_TBL conv_quality_to_color_scan_quality[] =
			{ 	/* SCAN2FTP_QUALITY		doc_scan_area_spec		color */
				 { FTP_Color100,		RESOLUTION_100DPI,		COLOR_MODE_COLOR	}
				,{ FTP_Color200,		RESOLUTION_200DPI,		COLOR_MODE_COLOR	}
				,{ FTP_Color300,		RESOLUTION_300DPI,		COLOR_MODE_COLOR	}
				,{ FTP_Color600,		RESOLUTION_600DPI,		COLOR_MODE_COLOR	}
				,{ FTP_Gray100,			RESOLUTION_100DPI,		COLOR_MODE_GRAY		}
				,{ FTP_Gray200,			RESOLUTION_200DPI,		COLOR_MODE_GRAY		}
				,{ FTP_Gray300,			RESOLUTION_300DPI,		COLOR_MODE_GRAY		}
				,{ FTP_BW200,			RESOLUTION_200DPI,		COLOR_MODE_BW		}
				,{ FTP_BW100,			RESOLUTION_100DPI,		COLOR_MODE_BW		}
				,{ FTP_BW300,			RESOLUTION_300DPI,		COLOR_MODE_BW		}
#if 0	/* ADS�ł�400DPI�����݂��Ȃ��B�K�v�ɂȂ����Ƃ��ɗL���ɂ��� */
				,{ FTP_Color400,		RESOLUTION_400DPI,		COLOR_MODE_COLOR	}
				,{ FTP_Gray400,			RESOLUTION_400DPI,		COLOR_MODE_GRAY		}
#endif
#if defined(USE_SCAN_AUTO_RESOLUTION)
				/* ColorAuto/GrayAuto�ɂ��Ă�COLOR_MODE�Ƃ̕R�����ړI�̂��߉𑜓xDefault�l��Ԃ� */
				,{ FTP_ColorAuto,		FTPC_DEFAULT_RESO,		COLOR_MODE_COLOR	}
				,{ FTP_GrayAuto,		FTPC_DEFAULT_RESO,		COLOR_MODE_GRAY		}
#endif
				,{ FTP_Gray600,			RESOLUTION_600DPI,		COLOR_MODE_GRAY		}
				,{ FTP_BW150,			RESOLUTION_150DPI,		COLOR_MODE_BW		}
				,{ FTP_Color150,		RESOLUTION_150DPI,		COLOR_MODE_COLOR	}
				,{ FTP_Gray150,			RESOLUTION_150DPI,		COLOR_MODE_GRAY		}
#ifdef USE_SCAN_COLOR_DETECT
				,{ FTP_Auto150,			RESOLUTION_150DPI,		COLOR_MODE_AUTO		}
				,{ FTP_Auto200,			RESOLUTION_200DPI,		COLOR_MODE_AUTO		}
				,{ FTP_Auto300,			RESOLUTION_300DPI,		COLOR_MODE_AUTO		}
				,{ FTP_Auto600,			RESOLUTION_600DPI,		COLOR_MODE_AUTO		}
#endif
				,{FTP_BW600,			RESOLUTION_600DPI,		COLOR_MODE_BW		}
			 };

/*----------------------------------------------------------------------------*/
/* LOCAL VALUES                                                               */
/*----------------------------------------------------------------------------*/
STATIC	UINT8               scan_src;
STATIC	UINT32              qid_ftpclient, qid_scanbase;
STATIC	FTPSCAN_STATE       ftpscan_state;
STATIC	ACCESS_INFO         access_info;         /* Panel Menu�̊m��l(FTP)   */
STATIC	CIFSACCESS_INFO     access_info_cfs;     /* Panel Menu�̊m��l(CIFS)  */
STATIC  FTP_CIFSACCESS_INFO ftp_cifs_access_info;
                                               /* Panel Menu�̊m��l(FTP/CIFS)*/
STATIC	UINT32              jpeg_scan_counter; /* JPEG�t�@�C�����߂̘A���ԍ�  */
STATIC  UINT32              service_kind;      /* FTP/CIFS�T�[�r�X���        */
STATIC  UINT8               servername_zl[ SCAN2FTP_SVR_MAXLEN  +1 ];
STATIC  UINT8               filename_zl  [ SCAN2FTP_FNAME_MAXSIZE ];
#ifndef	USE_SEPARATE_UI
#ifdef	GRAPHIC_LCD
STATIC	UINT32			    Quality_DSP;       /* �܍s���f���p�A              */
                                               /* Scanning�\�����̉𑜓x      */
#endif	/* GRAPHIC_LCD */
#endif	/* USE_SEPARATE_UI */

/* Scan to FTP,NW�̎����m�F�p */
STATIC SCAN_CHECK_RESULT	scan_check_result;							/* �ڑ�Check���ʊi�[�p�ϐ� */
STATIC UINT32				scan_check_result_time;						/* �ڑ�Check���Ԍ��ʊi�[�p�ϐ� [�ŏ��P�ʁF10ms] */

#ifdef FTPC_COMPLETE_SEQUENCE
STATIC	UINT32				timer_id;	/* �^�C�}�[�o�^ID�i�[�p */
#endif /* FTPC_COMPLETE_SEQUENCE */

/*----------------------------------------------------------------------------*/
/* EXTERNAL VALUES                                                            */
/*----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*/
/* STATIC FUNCTIONS                                                           */
/*----------------------------------------------------------------------------*/
STATIC	INT32	ftpc_init           ( void  );
STATIC  void	start_process       ( UINT16, UINT16, INT32*, INT32* );
STATIC  void	scan_process        ( UINT16, UINT32, INT32,  INT32  );
STATIC  INT32	scan_prepare        ( INT32*, INT32* );
STATIC  void	scan_start          ( INT32,  INT32  );
STATIC  void	send_scan_stop      ( void  );
STATIC  void	send_scan_start_end ( UINT16, UINT8,  INT32,  INT32  );
STATIC  void	transfer_process    ( INT32 );
STATIC  void	resource_release    ( INT32 );
STATIC  UINT32	decide_error_id     ( INT32 );
STATIC  UINT32	decide_error_id_cifs( INT32 );
#ifdef	USE_SCAN2SFTP
STATIC	UINT32	decide_error_id_sftp( INT32 );
#endif	/* USE_SCAN2SFTP */
STATIC  void	decide_filename     ( UINT32, UINT8, UINT8* , UINT8);
#ifndef	USE_SEPARATE_UI
STATIC  void	dispDIconFor5Line   ( void  );
#endif	/* USE_SEPARATE_UI */
STATIC  UINT32  scan_cnt            ( INT32 );
STATIC  void    scan_stop_after     ( BOOL  );
STATIC  void    scan_stop_memfull   ( void  );
STATIC  void    scan_pnltask        ( UINT16, INT32,  INT32 );
STATIC  void    scan_pnltask        ( UINT16, INT32,  INT32 );
STATIC  void    scan_machstatus     ( UINT32);
STATIC  void    scan_scanbase       ( UINT16, INT32,  INT32 );
STATIC  INT32   get_filetype        ( INT32 );
#ifdef COLOR_NETWORK_SCANNER
STATIC  UINT8   get_qual_index      ( SCAN2FTP_QUALITY );
STATIC  UINT8   get_scan_qual_index ( UINT8 );
#endif
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
STATIC	VOID	ScanEnd(INT32 Reason, INT32 ErrorInfo);
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
#ifdef USE_BSI
#ifdef	USE_SERIO_LOG
STATIC	INT32	set_scan_ftp_cifs_setting_bsilog( void );
STATIC	INT32	scan_bsiuserlog_pagecount( INT32 image_id, INT32 sysmem_id );
#endif /* USE_SERIO_LOG */
#endif /* USE_BSI */

#ifndef	USE_SEPARATE_UI
#ifdef LCD_5LINE
STATIC	VOID	scan_clear_cursor	(void);
#endif
#endif

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
/* NextPage����̍ĊJ�ʒm�pObserver class */
STATIC CObserver_t *g_Observer = NULL;
STATIC CObserver_t *g_ObserverStop = NULL;
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

STATIC VOID	conv_scan_quality_to_color_reso(UINT16 quality,UINT8 *color, UINT16 *resolution);
STATIC VOID ftpclient_ScanPage( FTPC_PGCNT_ACT iAction, UINT32* ioPage );
STATIC VOID ftpclient_BlankPage( FTPC_PGCNT_ACT iAction, UINT32* ioPage );

STATIC VOID scan_check_setresult(const SCAN_CHECK_RESULT rslt);		/* �ڑ��m�F���ʕۑ��֐� */
STATIC void scanftp_connection_check(UINT8 profilenumber);			/* �ڑ��m�F�֐� */
#ifdef	USE_SCAN2NW
STATIC	VOID	scannw_connection_check(UINT8 profilenumber);			/* �ڑ��m�F�֐� */
#endif	/* USE_SCAN2NW */
#ifdef	USE_SCAN2SFTP
STATIC	void	scansftp_connection_setup(UINT8 profilenumber,stcSFTPConnect* sftp_conn);
STATIC	void	scansftp_connection_check(UINT8 profilenumber);			/* ScantoSFTP�p�ڑ��m�F�֐� */
#endif	/* USE_SCAN2SFTP */

#ifdef FTPC_COMPLETE_SEQUENCE
STATIC VOID	ftpclient_CompleteStatus_end(UINT16 event, INT32 image_id);
STATIC VOID	set_ftpc_CompleteStatus_end_time(UINT32 time);
STATIC VOID	cancel_ftpc_CompleteStatus_end_time(VOID);
#endif /* #ifdef FTPC_COMPLETE_SEQUENCE */

#ifdef	FB_SCAN_TYPE
STATIC void msg_nulltask(UINT16, INT32, INT32);
#else	/* FB_SCAN_TYPE */
STATIC void msg_nulltask(UINT16);
#endif	/* FB_SCAN_TYPE */

/** for debug ****************************************************************/
#define	DEBUG_LEVEL	0
#if	(DEBUG_LEVEL > 0)
#undef	DPRINTF(x)
#define	DPRINTF(x)	EPRINTF(x)
#endif

/**
* @par	�t�@�C��Transfer�^�X�N ���C���֐�
* @param	�Ȃ�
* @return	�Ȃ�
*
* @par <�O���d�l>
*		Scan to FTP��Scan to Network�����j���[����w���Profile�Ɋւ��Ď��{����B
* @par <�����d�l>
*		�w���Profile�ɑ΂�Scan����̃C���[�W�Ǎ����s���t�@�C���]�������{����B
*
* @par <M�[>
*		M-BCL-945
*/

GLOBAL void
ftpclient_main( void )
{
	STATIC INT32 image_id;
	STATIC INT32 sysmem_id;

    UINT32       cmd_id;
	INT32        ret_value;
	ftpclt_msg   *message;
	macstatemsg_t   *message_state;
	ftpclt_get_msg  recv_msg;

	/* ������                            */
	memset( &recv_msg, NULL, sizeof(ftpclt_get_msg) );
	message   = &(recv_msg.ftpclt_tskmsg);

	/* �^�X�N����������                  */
	ret_value = ftpc_init();
	if ( ret_value != OK ) {
		return;
	}

    /* �^�X�N��Ԃ��p�l�������          */
    /* �w���҂��֕ύX                    */
    ftpscan_state = FTPC_START_WAIT;

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	if (Serio_Is_Enabled() == TRUE) {
	    /* NextPage����̍ĊJ�ʒm�p��Observer class�̃C���X�^���X����� */
	    if (! g_Observer) {
	        g_Observer = new( (void*)ctor(CObserver), sizeof(CObserver_t) );
	        if (! g_Observer) {
	            EPRINTF(("disp_nextexist - CObserver object instance doesn't get.\n"));
	            return;
	        }

	        /* Update�֐��̒u������ */
	        g_Observer->vptr_Init(g_Observer, UpdateFunc_FtpClient);
	    }

	    if (! g_ObserverStop) {
	        g_ObserverStop = new( (void*)ctor(CObserver), sizeof(CObserver_t) );
	        if (! g_ObserverStop) {
	            EPRINTF(("disp_nextexist - CObserver object instance doesn't get.\n"));
	            return;
	        }

	        /* Update�֐��̒u������ */
	        g_ObserverStop->vptr_Init(g_ObserverStop, UpdateFunc_FtpClient_StopScan);
	    }
	}
#endif	/* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

	/* ���b�Z�[�W��MLOOP                */
	while (1) {
		/* ���b�Z�[�W��M                */
		ret_value = FOS_MSGRECEIVE( qid_ftpclient, 
                                    (UINT8 *)(&recv_msg), sizeof(ftpclt_get_msg), WAIT_FOREVER );

		if ( ret_value == ERROR ) {
			continue;
		}
#ifdef DBG_RECEIVE_CMD
	DPRINTF(("[Ftpclient], ftpscan_state=%d, from_task=%d, cmd_id=%d\n", ftpscan_state, message->from_task, message->cmd_id));
#endif /* DBG_RECEIVE_CMD */

        /* �^�X�N��Ԗ��̏���            */
		switch ( ftpscan_state ) {
			/* �҂��󂯒�                */
			case FTPC_START_WAIT:
				if(CMD_SCAN_TO_FTPTEST_STA == message->cmd_id) {			/* Scan to FTP�ݒ�Check�J�n�v�� */
					scanftp_connection_check(message->profilenumber);		/* �T�[�o�[�ڑ��m�F */
				}
#ifdef	USE_SCAN2NW
				else if(CMD_SCAN_TO_NWTEST_STA == message->cmd_id) {		/* Scan to NW�ݒ�Check�J�n�v�� */
					scannw_connection_check(message->profilenumber);		/* �T�[�o�[�ڑ��m�F */
				}
#endif	/* USE_SCAN2NW */
#ifdef	USE_SCAN2SFTP
				else if(CMD_SCAN_TO_SFTPTEST_STA == message->cmd_id) {		/* Secure Scan to FTP�ݒ�Check�J�n�v�� */
					scansftp_connection_check(message->profilenumber);
				}
#endif	/* USE_SCAN2SFTP */
				else {
					start_process(message->from_task, message->cmd_id, &image_id, &sysmem_id);
				}
				break;
			/* �ǂݎ��&���M             */
			case FTPC_SCANNING  :
            case FTPC_SELECTNEXT:
	        case FTPC_WAITNEXT  :
	        case FTPC_SCANSTOP  :
            case FTPC_MEM_ERR   :
#ifdef FUNC_ADF_SCAN_SPEC_SHEETS_STOP
            case FTPC_SCAN_LIMIT_STOPPING:
#endif /* FUNC_ADF_SCAN_SPEC_SHEETS_STOP */
				if((CMD_SCAN_TO_FTPTEST_STA == message->cmd_id)
					||(CMD_SCAN_TO_NWTEST_STA == message->cmd_id)
#ifdef	USE_SCAN2SFTP
					||(CMD_SCAN_TO_SFTPTEST_STA == message->cmd_id)
#endif	/* USE_SCAN2SFTP */
				){	/* Scan to FTP,NW,SFTP�ݒ�Check�J�n�v�� */
					/* �ݒ�Check�������ʂɁu�������s:ftpscan_state != FTPC_START_WAIT�v��ݒ� */
					scan_check_setresult(SCAN_CHECK_ERR_BUSY);
					break;
				} else {
					if (message->from_task == MACSTATUS_LIBRARY){
						message_state = &(recv_msg.macstate_msg);
						cmd_id        = message_state->stateid;
					}else{
						cmd_id        = message->cmd_id;
					}
					/* �ǎ��&���M��ԂɑJ�� */
					scan_process (message->from_task, cmd_id,  image_id,  sysmem_id );
					if (ftpscan_state == FTPC_SEND){
					    /* ���M�˗�          */
					    transfer_process(image_id);
					}
				}
#ifdef FTPC_COMPLETE_SEQUENCE
			case FTPC_COMPLETE:
				if (message->from_task == FTPC_APL_TASK
					&& (( message->cmd_id == CMD_CLOSE_COMPLETE_OK)
				    ||  ( message->cmd_id == CMD_CLOSE_COMPLETE_ST)) ) {
					ftpclient_CompleteStatus_end(message->cmd_id,image_id);
				}
				break;
#endif /* FTPC_COMPLETE_SEQUENCE */
				break;
	            /* ���҂��Ȃ����            */
			default:
				if((CMD_SCAN_TO_FTPTEST_STA == message->cmd_id)
					||(CMD_SCAN_TO_NWTEST_STA == message->cmd_id)
#ifdef	USE_SCAN2SFTP
					||(CMD_SCAN_TO_SFTPTEST_STA == message->cmd_id)
#endif	/* USE_SCAN2SFTP */
				){	/* Scan to FTP,NW,SFTP�ݒ�Check�J�n�v�� */
					/* �ݒ�Check�������ʂɁu�������s:ftpscan_state != FTPC_START_WAIT�v��ݒ� */
					scan_check_setresult(SCAN_CHECK_ERR_BUSY);
					break;
				}
                /* Error����             */
                DPRINTF((" ftpc : Unexpected Status@ftpclient_main\n"));
				break;
        }
	}
}

/**
* @par	���j���[�Ŏw�肳�ꂽFTP��Profile�̏����擾����B
* @param	*uif_settings (input) Profile���̍\����
* @return	�Ȃ�
*
* @par <�O���d�l>
*		���j���[�Ŏw�肳�ꂽProfile�̊e����{�^�X�N��STATIC�G���A�Ɋi�[����B
* @par <�����d�l>
*		���j���[�Ŏw�肳�ꂽProfile�̊e����Profile�Ɋ֘A����K�v�ȏ���s�����G���A
*		����擾���Ė{�^�X�N��STATIC�G���A�Ɋi�[����B
*
* @par <M�[>
*	   M-BCL-945
*/

#ifdef USE_SCAN2FTP
GLOBAL void
ftpclient_setaccessinfo( ACCESS_INFO *uif_settings )
{
	/* ������             */
	memset( &access_info, NULL,         sizeof(ACCESS_INFO) );
	memcpy( &access_info, uif_settings, sizeof(ACCESS_INFO) );
	memset( &ftp_cifs_access_info,
                          NULL,         sizeof(FTP_CIFSACCESS_INFO) );

    /* �T�[�r�X��ʂ̐ݒ� */
    service_kind = SCAN2FTP_SERVICE;

    /* �T�[�o�[��/�t�@�C�����̕ێ�                         */
    strncpy( (MD_CHAR *)servername_zl,
             (MD_CHAR *)access_info.servername,        SCAN2FTP_SVR_MAXLEN      );
    strncpy( (MD_CHAR *)filename_zl,
             (MD_CHAR *)access_info.filename,          SCAN2FTP_FNAME_MAXLEN    );

	/* JPEG�̃t�@�C�����͖���ς���Ă��܂����߁A          */
    /* �����ŘA���ԍ����v�����Ă���                        */
	if ( uif_settings->fileformat == FTP_JPEG ){
        scan_counter(&jpeg_scan_counter);
	}
#ifndef	USE_SEPARATE_UI
#ifdef	GRAPHIC_LCD
	switch (uif_settings->quality) {
		case FTP_Color100:
			Quality_DSP = COLOR_100DPI_DSP;
			break;
		case FTP_Color200:
			Quality_DSP = COLOR_200DPI_DSP;
			break;
		case FTP_Color300:
			Quality_DSP = COLOR_300DPI_DSP;
			break;
		case FTP_Color600:
			Quality_DSP = COLOR_600DPI_DSP;
			break;
		case FTP_Gray100 :
			Quality_DSP = GRAY_100DPI_DSP;
			break;
		case FTP_Gray200 :
			Quality_DSP = GRAY_200DPI_DSP;
			break;
		case FTP_Gray300 :
			Quality_DSP = GRAY_300DPI_DSP;
			break;
		case FTP_BW300   :
			Quality_DSP = BW_300DPI_DSP;
			break;
		case FTP_BW200   :
			Quality_DSP = BW_200DPI_DSP;
			break;
		case FTP_BW100   :
			Quality_DSP = BW_200X100_DSP;
			break;
#if defined(USE_SCAN_AUTO_RESOLUTION)
		case FTP_ColorAuto:
			Quality_DSP = COLOR_AUTO_DSP;
			break;
		case FTP_GrayAuto :
			Quality_DSP = GRAY_AUTO_DSP;
			break;
#endif
		default          :
			Quality_DSP = COLOR_150DPI_DSP;
			break;
	}
#endif	/* GRAPHIC_LCD */
#endif	/* USE_SEPARATE_UI */


    /* Panel Menu�̊m��l(FTP/CIFS)�Ɋi�[ */
    /* �掿                               */
    ftp_cifs_access_info.quality    = access_info.quality;
    /* �t�@�C���t�H�[�}�b�g               */
    ftp_cifs_access_info.fileformat = access_info.fileformat;
    /* ���ʓǂݎ��                       */
    ftp_cifs_access_info.dualscan   = access_info.dualscan;
    /* �擪������̃^�C�v                 */
    ftp_cifs_access_info.filenametype
                                    = access_info.filenametype;
    /* FTP Server ��                      */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.servername,
             (MD_CHAR *)access_info.servername,         SCAN2FTP_SVR_MAXLEN    );

    /* FTP ServerAddress                  */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.serveraddress,
             (MD_CHAR *)access_info.serveraddress,      SCAN2FTP_SVRADR_MAXLEN );

    /* USER��                             */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.username,
             (MD_CHAR *)access_info.username,           SCAN2FTP_USER_MAXLEN   );

	/* PASSWORD                           */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.password,
             (MD_CHAR *)access_info.password,           SCAN2FTP_PASSWD_MAXLEN );

    /* �i�[DIRECTORY                      */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.storedir,
             (MD_CHAR *)access_info.storedir,           SCAN2FTP_STDIR_MAXLEN  );

    /* �t�@�C����                         */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.filename,
             (MD_CHAR *)access_info.filename,           SCAN2FTP_FNAME_MAXLEN  );

    /* Passive or Active                  */
    ftp_cifs_access_info.ispassive  = access_info.ispassive;

    /* Port�ԍ�                           */
    ftp_cifs_access_info.portnum    = access_info.portnum;

    /* Secure PDF �p�X���[�h              */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.spdfpass,
             (MD_CHAR *)access_info.spdfpass,           FTPC_SPDF_PASSWD_MAXLEN);

    /* Scan���k��                         */
    ftp_cifs_access_info.scan_quality = access_info.scan_quality;

#ifdef USE_TMP_SCANSIZE
    ftp_cifs_access_info.scan_doc_size = access_info.scan_doc_size;
#endif /* USE_TMP_SCANSIZE */

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	if (Serio_Is_Enabled() == TRUE) {
		ftp_cifs_access_info.scan_src = access_info.scan_src;
	}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

	/* �d�����m */
	ftp_cifs_access_info.scan_multifeed_detect = access_info.scan_multifeed_detect;	/* �d�����m ON/OFF */

	/*���������@�\ */
	ftp_cifs_access_info.scan_blank_detect = access_info.scan_blank_detect;			/* ���������@�\ ON/OFF */ 

	/* �΍s�␳ */
	ftp_cifs_access_info.scan_deskew_adjust = access_info.scan_deskew_adjust;		/* �΍s�␳ ON/OFF */ 

	/* �F��Auto����Gray���k�� */
	ftp_cifs_access_info.scan_quality_gray = access_info.scan_quality_gray;		/* �F��Auto����Gray���k�� */ 
	
#ifdef    USE_GNDCOLOR_REMOVAL
	ftp_cifs_access_info.gndcolor_removal = access_info.gndcolor_removal;
	ftp_cifs_access_info.gndcolor_level = access_info.gndcolor_level;
#endif /* USE_GNDCOLOR_REMOVAL */
	ftp_cifs_access_info.FileNameFixed = access_info.FileNameFixed;
	return;
}
#endif /* USE_SCAN2FTP */

#ifdef	USE_SCAN2NW
/**
* @par	���j���[�Ŏw�肳�ꂽCIFS��Profile�̏����擾����B
* @param	*uif_settings (input) Profile���̍\����
* @return	�Ȃ�
*
* @par <�O���d�l>
*		���j���[�Ŏw�肳�ꂽProfile�̊e����{�^�X�N��STATIC�G���A�Ɋi�[����B
* @par <�����d�l>
*		���j���[�Ŏw�肳�ꂽProfile�̊e����Profile�Ɋ֘A����K�v�ȏ���s�����G���A
*		����擾���Ė{�^�X�N��STATIC�G���A�Ɋi�[����B
*
* @par <M�[>
*	   M-BCL-945
*/

GLOBAL void
cifsclient_setaccessinfo( CIFSACCESS_INFO *uif_settings )
{
	/* ������             */
	memset( &access_info_cfs, NULL,         sizeof(CIFSACCESS_INFO) );
	memcpy( &access_info_cfs, uif_settings, sizeof(CIFSACCESS_INFO) );
	memset( &ftp_cifs_access_info,
                              NULL,         sizeof(FTP_CIFSACCESS_INFO) );

    /* �T�[�r�X��ʂ̐ݒ�                                  */
    service_kind = SCAN2CIFS_SERVICE;

    /* �T�[�o�[��/�t�@�C�����̕ێ�                         */
    strncpy( (MD_CHAR *)servername_zl,
             (MD_CHAR *)access_info_cfs.serveraddress,
                                                    CIFSCLIB_HOSTADDR_SIZE);

    strncpy( (MD_CHAR *)filename_zl,
             (MD_CHAR *)access_info_cfs.filename,   SCAN2FTP_FNAME_MAXSIZE );

	/* JPEG�̃t�@�C�����͖���ς���Ă��܂����߁A          */
    /* �����ŘA���ԍ����v�����Ă���                        */
	if ( uif_settings->fileformat == FTP_JPEG ){
        scan_counter(&jpeg_scan_counter);
	}

#ifndef	USE_SEPARATE_UI
#ifdef	GRAPHIC_LCD
	switch (uif_settings->quality) {
		case FTP_Color100:
			Quality_DSP = COLOR_100DPI_DSP;
			break;
		case FTP_Color200:
			Quality_DSP = COLOR_200DPI_DSP;
			break;
		case FTP_Color300:
			Quality_DSP = COLOR_300DPI_DSP;
			break;
		case FTP_Color600:
			Quality_DSP = COLOR_600DPI_DSP;
			break;
		case FTP_Gray100 :
			Quality_DSP = GRAY_100DPI_DSP;
			break;
		case FTP_Gray200 :
			Quality_DSP = GRAY_200DPI_DSP;
			break;
		case FTP_Gray300 :
			Quality_DSP = GRAY_300DPI_DSP;
			break;
		case FTP_BW300   :
			Quality_DSP = BW_300DPI_DSP;
			break;
		case FTP_BW200   :
			Quality_DSP = BW_200DPI_DSP;
			break;
		case FTP_BW100   :
			Quality_DSP = BW_200X100_DSP;
			break;
#if defined(USE_SCAN_AUTO_RESOLUTION)
		case FTP_ColorAuto:
			Quality_DSP = COLOR_AUTO_DSP;
			break;
		case FTP_GrayAuto :
			Quality_DSP = GRAY_AUTO_DSP;
			break;
#endif
		default          :
			Quality_DSP = COLOR_150DPI_DSP;
			break;
	}
#endif	/* GRAPHIC_LCD */
#endif	/* USE_SEPARATE_UI */

    /* Panel Menu�̊m��l(FTP/CIFS)�Ɋi�[ */
    /* �掿                               */
    ftp_cifs_access_info.quality    = access_info_cfs.quality;
    /* �t�@�C���t�H�[�}�b�g               */
    ftp_cifs_access_info.fileformat = access_info_cfs.fileformat;
    /* ���ʓǂݎ��                       */
    ftp_cifs_access_info.dualscan   = access_info_cfs.dualscan;
    /* �擪������̃^�C�v                 */
    ftp_cifs_access_info.filenametype
                                    = access_info_cfs.filenametype;

    /* CIFS ServerAddress                 */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.serveraddress,
             (MD_CHAR *)access_info_cfs.serveraddress,  CIFSCLIB_HOSTADDR_SIZE   );

    /* USER��                             */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.username,
             (MD_CHAR *)access_info_cfs.username,       CIFSCLIB_USERNAME_SIZE   );

	/* PASSWORD                           */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.password,
             (MD_CHAR *)access_info_cfs.password,       CIFSCLIB_PASSWORD_SIZE   );

    /* �i�[DIRECTORY                      */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.storedir,
             (MD_CHAR *)access_info_cfs.storedir,       CIFSCLIB_DIRECTORY_SIZE  );

    /* �t�@�C����                         */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.filename,
             (MD_CHAR *)access_info_cfs.filename,       SCAN2FTP_FNAME_MAXSIZE   );

    /* �F�ؕ��@                           */
    ftp_cifs_access_info.AuthenticationMethod =
                                      access_info_cfs.AuthenticationMethod;

    /* �P���x���X�T�[�o���܂���IP�A�h���X */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.KerberosServerAddress,
             (MD_CHAR *)access_info_cfs.KerberosServerAddress, 
                                                        CIFSCLIB_KERBADDR_SIZE   );

    /* Secure PDF �p�X���[�h              */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.spdfpass,
             (MD_CHAR *)access_info_cfs.spdfpass,       FTPC_SPDF_PASSWD_MAXLEN  );

    /* Scan���k��                         */
    ftp_cifs_access_info.scan_quality = access_info_cfs.scan_quality;

#ifdef USE_TMP_SCANSIZE
    ftp_cifs_access_info.scan_doc_size = access_info_cfs.scan_doc_size;
#endif /* USE_TMP_SCANSIZE */

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	if (Serio_Is_Enabled() == TRUE) {
		ftp_cifs_access_info.scan_src = access_info_cfs.scan_src;
	}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

	/* �d�����m */
	ftp_cifs_access_info.scan_multifeed_detect = access_info_cfs.scan_multifeed_detect;	/* �d�����m ON/OFF */

	/*���������@�\ */
	ftp_cifs_access_info.scan_blank_detect = access_info_cfs.scan_blank_detect;			/* ���������@�\ ON/OFF */ 

	/* �΍s�␳ */
	ftp_cifs_access_info.scan_deskew_adjust = access_info_cfs.scan_deskew_adjust;		/* �΍s�␳ ON/OFF */ 

	/* �F��Auto����Gray���k�� */
	ftp_cifs_access_info.scan_quality_gray = access_info_cfs.scan_quality_gray;		/* �F��Auto����Gray���k�� */ 

#ifdef    USE_GNDCOLOR_REMOVAL
	ftp_cifs_access_info.gndcolor_removal = access_info_cfs.gndcolor_removal;
	ftp_cifs_access_info.gndcolor_level = access_info_cfs.gndcolor_level;
#endif /* USE_GNDCOLOR_REMOVAL */
	ftp_cifs_access_info.FileNameFixed = access_info_cfs.FileNameFixed;
	return;
}
#endif /* USE_SCAN2NW */

#ifdef	USE_SCAN2SFTP
/**
* @par	���j���[�Ŏw�肳�ꂽSFTP��Profile�̏����擾����B
* @param	*uif_settings (input) Profile���̍\����
* @return	�Ȃ�
*
* @par <�O���d�l>
*		���j���[�Ŏw�肳�ꂽProfile�̊e����{�^�X�N��STATIC�G���A�Ɋi�[����B
* @par <�����d�l>
*		���j���[�Ŏw�肳�ꂽProfile�̊e����Profile�Ɋ֘A����K�v�ȏ���s�����G���A
*		����擾���Ė{�^�X�N��STATIC�G���A�Ɋi�[����B
*
* @par <M�[>
*	   
*/
GLOBAL void
sftpclient_setaccessinfo( ACCESS_INFO *uif_settings )
{
	/* ������ */
	(void)memset( &access_info, NULL, sizeof(ACCESS_INFO) );
	(void)memcpy( &access_info, uif_settings, sizeof(ACCESS_INFO) );
	(void)memset( &ftp_cifs_access_info, NULL, sizeof(FTP_CIFSACCESS_INFO) );

	/* Service��ʐݒ� */
	service_kind = SCAN2SFTP_SERVICE;

	/* �T�[�o�[���ێ� */
	(void)strncpy( (MD_CHAR*)servername_zl, (MD_CHAR*)access_info.servername, SCAN2SFTP_SVR_MAXLEN );
	/* �t�@�C�����ێ� */
	(void)strncpy( (MD_CHAR*)filename_zl, (MD_CHAR*)access_info.filename, SCAN2SFTP_FNAME_MAXLEN );
	/* JPEG��File���͖���ω����邽�߁A�����ŘA���ԍ����v�� */
	if( uif_settings->fileformat == FTP_JPEG ) {
		(void)scan_counter( &jpeg_scan_counter );
	}

	/* Panel Menu�̊m��l(FTP/CIFS/SFTP)�Ɋi�[ */
	/* 1.�掿 */
	ftp_cifs_access_info.quality = access_info.quality;
	/* 2.�t�@�C���t�H�[�}�b�g */
	ftp_cifs_access_info.fileformat = access_info.fileformat;
	/* 3.���ʓǎ� */
	ftp_cifs_access_info.dualscan = access_info.dualscan;
	/* 4.�擪������̃^�C�v */
	ftp_cifs_access_info.filenametype = access_info.filenametype;
	/* 5.SFTP Server�� */
	(void)strncpy( (MD_CHAR*)ftp_cifs_access_info.servername, (MD_CHAR*)access_info.servername, SCAN2SFTP_SVR_MAXLEN );
	/* 6.FTP Server Address */
	(void)strncpy( (MD_CHAR*)ftp_cifs_access_info.serveraddress, (MD_CHAR*)access_info.serveraddress, SCAN2SFTP_SVRADR_MAXLEN );
	/* 7.User�� */
	(void)strncpy( (MD_CHAR*)ftp_cifs_access_info.username, (MD_CHAR*)access_info.username, SCAN2SFTP_USER_MAXLEN );
	/* 8.Password */
	(void)strncpy( (MD_CHAR*)ftp_cifs_access_info.password, (MD_CHAR*)access_info.password, SCAN2SFTP_PASSWD_MAXLEN );
	/* 9.�i�[Directory */
	(void)strncpy( (MD_CHAR*)ftp_cifs_access_info.storedir, (MD_CHAR*)access_info.storedir, SCAN2SFTP_STDIR_MAXLEN );
	/* 10.�t�@�C���� */
	(void)strncpy( (MD_CHAR*)ftp_cifs_access_info.filename, (MD_CHAR*)access_info.filename, SCAN2SFTP_FNAME_MAXLEN );
	/* 11.�|�[�g�ԍ� */
	ftp_cifs_access_info.portnum = access_info.portnum;
	/* 12.Secure PDF�p�X���[�h */
	(void)strncpy( (MD_CHAR*)ftp_cifs_access_info.spdfpass, (MD_CHAR*)access_info.spdfpass, FTPC_SPDF_PASSWD_MAXLEN );
	/* 13.�X�L�������k�� */
	ftp_cifs_access_info.scan_quality = access_info.scan_quality;
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	/* 14.���e�ǂݎ��ʒu */
	if( Serio_Is_Enabled() == TRUE ) {
		ftp_cifs_access_info.scan_src = access_info.scan_src;
	}
#endif	/* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
	/* 14.Document Size */
	ftp_cifs_access_info.scan_doc_size = access_info.scan_doc_size;
	/* 15.�d�����m */
	ftp_cifs_access_info.scan_multifeed_detect = access_info.scan_multifeed_detect;
	/* 16.�������� */
	ftp_cifs_access_info.scan_blank_detect = access_info.scan_blank_detect;
	/* 17.�ΌX�␳ */
	ftp_cifs_access_info.scan_deskew_adjust = access_info.scan_deskew_adjust;
	/* 18.�F��Auto����Gray���k�� */
	ftp_cifs_access_info.scan_quality_gray = access_info.scan_quality_gray;
	/* 19.�n�F�␳ */
	ftp_cifs_access_info.gndcolor_removal = access_info.gndcolor_removal;
	ftp_cifs_access_info.gndcolor_level = access_info.gndcolor_level;
	/* 20.User�F�ؕ��� */
	ftp_cifs_access_info.AuthenticationMethod = access_info.AuthenticationMethod;
	/* 21.SFTP Server���J�� */
	ftp_cifs_access_info.PubKeyIdx = access_info.PubKeyIdx;
	/* 22.MFC��pair��ID */
	ftp_cifs_access_info.PairKeyIdx = access_info.PairKeyIdx;

	ftp_cifs_access_info.FileNameFixed = access_info.FileNameFixed;
	return;

}
#endif	/* USE_SCAN2SFTP */

/**
* @par	�{�^�X�N�̏�Ԃ�Read/Write����B�iLCD�\���p�Ɍ��J�j
* @param	*state (input/output) Read������ԁ^Write������
* @param	access_type (input) Read/Write���
* @return	�Ȃ�
*
* @par <�O���d�l>
*		�{�^�X�N�̏�Ԃ�STATIC�̈��Read/Write����B
* @par <�����d�l>
*		�{�^�X�N�̏�Ԃ�STATIC�̈��Read/Write����B
*
* @par <M�[>
*	   M-BCL-945
*/

GLOBAL void
ftpclient_state( FTPSCAN_STATE *state, INTERNAL_ACCESS access_type )
{
	if ( access_type == INTERNAL_READ ) {
		*state        = ftpscan_state;
	}
	else if ( access_type == INTERNAL_WRITE ) {
		ftpscan_state = *state;
	}
	return;
}


/**
* @par	�t�@�C��Transfer�^�X�N �������֐�
* @param	�Ȃ�
* @return	OK�F����I��
*			ERROR�F�ُ�I��
*
* @par <�O���d�l>
*		�{�^�X�N���֘A���鏉�������s���B
* @par <�����d�l>
*		�^�X�N��Ԃ̏������A���^�X�N�N���A�L�[���͂̏��������s���B
*
* @par <M�[>
*	   M-BCL-945
*/

STATIC INT32
ftpc_init( void )
{
	/* �錾                          */
	INT32 ret_value;

	/* ������                        */
	ret_value     = OK;

    /* �A�v����Ԃ� INIT �ɂ���      */
	manaplset( S_APL_FTPCLIENT, APL_INIT );

	/* �֐��O�ϐ��̏�����            */
	scan_src      = SCAN_SRC_ADF;
	qid_scanbase  = FOS_MSGGETID( SCAN_MSG_NAME );

	/* �^�X�N�N��                    */
	qid_ftpclient = FOS_MSGCREATE( FTPC_MSG_NAME, FTPC_MSG_COUNT, sizeof(ftpclt_get_msg) );
	ftpscan_state = FTPC_IDLE;
	memset( &access_info,          NULL, sizeof(ACCESS_INFO) );
	memset( &access_info_cfs,      NULL, sizeof(CIFSACCESS_INFO) );
	memset( &ftp_cifs_access_info, NULL, sizeof(FTP_CIFSACCESS_INFO) );

	/* �@�\���Ȃ��ꍇ�͉������Ȃ�    */
	if( (ModelFuncGet(MODEL_FUNC_SCAN2FTP, MODEL_FUNC_REF_IOFSW) == FALSE) && 
	    (ModelFuncGet(MODEL_FUNC_SCAN2NW,  MODEL_FUNC_REF_IOFSW) == FALSE)	 ) {
		ret_value = ERROR;
	}

    /* LAN�@�\���Ȃ��ꍇ�͉������Ȃ� */
	if(ModelFuncGet(MODEL_FUNC_NETWORK,       MODEL_FUNC_REF_IOFSW) == FALSE ) {
		ret_value = ERROR;
    }

#if !defined(ONCHIP_LAN) && !defined(USE_OPT_LAN)
	/* LAN�{�[�h����������Ă��Ȃ���΂Ȃɂ����Ȃ� */
	if ( aioChkBoard() != OK ) {
		ret_value = ERROR;
	}
#endif

#ifndef	USE_SEPARATE_UI
	ftpc_keyin_init();
#endif	/* USE_SEPARATE_UI */

	/* �ݒ�Check�������ʂɁu������ԁv��ݒ� */
	scan_check_setresult(SCAN_CHECK_INIT);

	/* �ǂݎ�薇��/�������o������������ */
	ftpclient_ScanPage(FTPC_PGCNT_INIT, NULL);
	ftpclient_BlankPage(FTPC_PGCNT_INIT, NULL);

#ifdef FTPC_COMPLETE_SEQUENCE
	timer_id	= 0;		/* �^�C�}�[�o�^ID�i�[�p�ϐ��̏����� */
#endif /* FTPC_COMPLETE_SEQUENCE */

	/* �A�v����Ԃ� READY �ɂ���     */
	manaplset( S_APL_FTPCLIENT, APL_READY );

	return ( ret_value );
}

/**
* @par	�X�L���i����ǎ���J�n����
* @param	from_task (input) ���b�Z�[�W���M���^�X�NID
* @param	cmd_id (input) ���b�Z�[�W���̃R�}���hID�i���b�Z�[�W���M���^�X�N�̏ꍇ�͑��u��ԁj
* @param	*image_id (output) ��f�[�^ID
* @param	*sysmem_id (output) �V�X�e��������ID
* @return	�Ȃ�
*
* @par <�O���d�l>
*		�T�[�o�[�̐ڑ��m�F���s�Ȃ��A�X�L���i����ǎ���J�n����B
* @par <�����d�l>
*		�T�[�o�[�̐ڑ��m�F���s�Ȃ��A���\�[�X�̊m�ۂƃX�L���i����ǎ���s���p�����[�^��ݒ肵
*		�J�n�v���𑗐M����B
*
* @par <M�[>
*	   M-BCL-945
*/

STATIC void
start_process( UINT16 from_task, UINT16 cmd_id, INT32 *image_id, INT32 *sysmem_id )
{
	INT32 ret_value;

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	if (Serio_Is_Enabled() == TRUE) {
		/* NextPage�ʒm��Observer�o�^ */
		if (g_SubjectFtp) {
			if (! g_SubjectFtp->vptr_Attach(g_SubjectFtp, g_Observer)) {
				EPRINTF(("disp_nextexist - CSubject::Attach error.\n"));
				return;
			}
		}

		if (g_SubjectJobStop) {
			if (! g_SubjectJobStop->vptr_Attach(g_SubjectJobStop, g_ObserverStop)) {
				EPRINTF(("disp_nextexist - g_ObserverStop::Attach error.\n"));
				return;
			}
		}
	}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

    if (cmd_id    == CMD_SCANSTART &&
        from_task == PANEL_BASE_TASK){

		/* �ǂݎ�薇��/�������o������0�ɏ����� */
	    ftpclient_ScanPage(FTPC_PGCNT_START, NULL);
		ftpclient_BlankPage(FTPC_PGCNT_START, NULL);
	
    	/* �T�[�o�[�̐ڑ��m�F           */
	    ret_value     = scan_prepare( image_id, sysmem_id );
    	if ( ret_value != OK ) {
#ifdef	USE_SEPARATE_UI
			cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif
	    	return;
    	}
    	
        /* �^�X�N�̏�Ԃ��X�L���j���O�� */
    	ftpscan_state = FTPC_SCANNING;
	    /* �X�L�����X�^�[�g����         */
	    scan_start( *image_id, *sysmem_id );
    }
}

/**
* @par	�X�L�����J�n�̏������s���B
* @param	*image_id (output) ��f�[�^ID
* @param	*sysmem_id (output) �V�X�e��������ID
* @return	OK�F����I��
*			ERROR�F�ُ�I��
*
* @par <�O���d�l>
*      �X�L�����J�n�̏����ׂ̈̃��\�[�X�m�ۂ��s���B
* @par <�����d�l>
*      �X�L�����J�n�̏����ׂ̈̃��\�[�X�m�ۂ��s���ALCD�̕\�����X�V����B
*
* @par <M�[>
*	   M-BCL-945
*/

STATIC INT32
scan_prepare( INT32 *image_id, INT32 *sysmem_id )
{
    INT32			ret_value;
    INT32			ret_value_ftp;
    INT32			ret_value_cifs;
    UINT32			str_id;       /* �G���[�\�����镶����     */
    UINT32			image_use;    /* ��f�[�^���p�ړI         */
    stcFTPConnect   ftp_conn;     /* ftp�ڑ����\����        */
    stcCIFSConnect  cifs_conn;    /* CIFS�ڑ����\����       */
#ifdef	USE_SCAN2SFTP
    INT32			ret_value_sftp;
	stcSFTPConnect	sftp_conn;	 	/* sftp�ڑ����\���� */
#endif	/* USE_SCAN2SFTP */
#ifdef USE_SCAN_COLOR_DETECT /* �F���������� */
	UINT8			color_num;
#endif /* USE_SCAN_COLOR_DETECT */

#ifndef	USE_SEPARATE_UI
#ifdef	LCD_5LINE
    /* PIN_NO_DSP(�|���Pin No:XXXX)���A                      */
    /* LCD�\�����邽�߂̃p�����[�^                            */
    pstring_t		str_st;
#endif	/* LCD_5LINE */
#endif	/* USE_SEPARATE_UI */

	/* ������                 */
    ret_value           = ERROR;
    ret_value_ftp       = ERROR;
    ret_value_cifs      = ERROR;
#ifdef	USE_SCAN2SFTP
    ret_value_sftp      = ERROR;
#endif	/* USE_SCAN2SFTP */
#ifdef	USE_SEPARATE_UI
    str_id              = OK; /* QAC�΍�F���ۂɎg�p����邱�Ƃ͂Ȃ� */
#else	/* USE_SEPARATE_UI */
    str_id              = MAIL_SENDERR_DSP;
#endif	/* USE_SEPARATE_UI */

    image_use           = IMAGE_USE_TIFF_F;
	(void)memset( &ftp_conn,  NULL, sizeof(stcFTPConnect ) );
	(void)memset( &cifs_conn, NULL, sizeof(stcCIFSConnect) );
#ifdef	USE_SCAN2SFTP
	(void)memset( &sftp_conn, NULL, sizeof(stcSFTPConnect) );
#endif	/* USE_SCAN2SFTP */

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	if (Serio_Is_Enabled() == TRUE) {
		/* ���s���ʒm */
		SendJobStatus(SERIOFW_JOBSTS_PROCESSING, SERIO_JOB_SCANSEND);
	}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

#ifndef	USE_SEPARATE_UI
    /* �p�l�����́E�\�����m��                                 */
    ret_value		    = ftpc_key_rightchg( GET_RIGHT );
    if ( ret_value != OK ) {
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, ERROR);
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
        return ( ERROR );
    }
#endif	/* USE_SEPARATE_UI */

    /*�E�T�[�o�[�̐ڑ��m�F�͏�ʊ֐��̃X�L���i�[�ǎ�J�n      */
    /*  (scan_start)�Ŏ��{����B                              */
    /*�E�A���ł�LCD�̕\������                                 */
    /*  ftpc_disp_string( DUMMY_LINE3, NULL );                */
    /*  ftpc_disp_string( DUMMY_LINE4, NULL );                */
    /*  ftpc_disp_string( DUMMY_LINE5, NULL );                */
    /*  �����ʊ֐��iftpc_disp_string_NULL�j�Ŏ��{����B       */



#ifndef	USE_SEPARATE_UI
#ifdef LCD_5LINE
	/* �t�@�C���t�H�[�}�b�g�p�̔C�ӕ�����̏�����             */
	memset( &str_st, NULL, sizeof(pstring_t) );

	/* �|��ꂩ��t�@�C���t�H�[�}�b�g�p�̔C�ӕ�����֕ϊ����� */
    lcd_strcpy(str_st.str_data, Quality_DSP);
#endif	/* LCD_5LINE */

	/* �S�̕\���FConnecting                                   */
#ifdef LCD_5LINE
    /* ��s��                                                 */
    if (service_kind      == SCAN2FTP_SERVICE){
        ftpc_disp_string( SCANTO_FTP_DSP    ,  NULL );
    }else if(service_kind == SCAN2CIFS_SERVICE){
        ftpc_disp_string( SCANTO_NETWORK_DSP,  NULL );
    }
#else	/* LCD_2LINE                                          */
    /* ��s��                                                 */
	ftpc_disp_string( SERVER_CNCT_DSP, NULL );
#endif	/* LCD_5LINE LCD_2LINE                                */

	/* �\��                                                   */
    /* ��s��                                                 */
	ftpc_disp_string( DUMMY_LINE2, servername_zl );

	/* �܍s�\���p 3,4,5�s��                                   */
#ifdef LCD_5LINE

	/* �\��                                                   */
	ftpc_disp_string( DUMMY_LINE3, str_st.str_data );
	ftpc_disp_string( DUMMY_LINE4, filename_zl );
	ftpc_disp_string( SERVER_CNCT_DSP, NULL );

	/* ���ʓǎ�̏ꍇ�͂c�A�C�R�����\��                       */
	dispDIconFor5Line();

#endif	/* LCD_5LINE */
#else	/* USE_SEPARATE_UI */
	/* CP����Ăяo����̈�Ƀt�@�C������ݒ肷�� */
	memcpy(Scan2Ftp_SendFileName, ftp_cifs_access_info.filename, SCAN2FTP_FNAME_MAXSIZE);
	cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_CONNECT );
#endif	/* USE_SEPARATE_UI */

    /* Service��Scan To FTP�̏ꍇ                             */
    if (service_kind == SCAN2FTP_SERVICE){
#ifdef USE_SCAN2FTP
        /* FTP�z�X�g�ڑ����ݒ�                              */
        /* �p�b�V�u���[�h or �A�N�e�B�u���[�h                 */
        ftp_conn.IsPassive      = ftp_cifs_access_info.ispassive;

        /* Port�ԍ�                                           */
        ftp_conn.PortNumber     = ftp_cifs_access_info.portnum;

        /* FTP�z�X�g���܂���IP�A�h���X                        */
        ftp_conn.HostAddress    = ftp_cifs_access_info.serveraddress;

        /* �t�@�C���ۑ���t�H���_                             */
        ftp_conn.StoreDirectory = ftp_cifs_access_info.storedir;

        /* �F�؂̂��߂̃��[�U�[��                             */
        ftp_conn.UserName       = ftp_cifs_access_info.username;

        /* �F�؂̂��߂̃p�X���[�h                             */
        ftp_conn.Password       = ftp_cifs_access_info.password;

        /* FTP�z�X�g�ڑ��m�F                                  */
        ret_value_ftp = ftpclib_IsConnect(&ftp_conn);

    	if ( ret_value_ftp != FTPCLIB_SUCCESS ) {
            ret_value = ERROR;
        }
#ifdef	USE_SEPARATE_UI
		else{
            ret_value = OK;
		}
#endif
#endif
    /* Service��Scan To CIFS�̏ꍇ                            */
    }else if(service_kind == SCAN2CIFS_SERVICE){
#ifdef	USE_SCAN2NW
        /* CIFS�z�X�g�ڑ����ݒ�                             */
        /* CIFS�z�X�g���܂���IP�A�h���X                       */
        cifs_conn.HostAddress    = ftp_cifs_access_info.serveraddress;

        /* �t�@�C���ۑ���t�H���_                             */
        cifs_conn.StoreDirectory = ftp_cifs_access_info.storedir;

        /* �F�؂̂��߂̃��[�U�[��                             */
        cifs_conn.UserName       = ftp_cifs_access_info.username;

        /* �F�؂̂��߂̃p�X���[�h                             */
        cifs_conn.Password       = ftp_cifs_access_info.password;

        /* �F�ؕ��@                                           */
        cifs_conn.AuthenticationMethod
                                 = ftp_cifs_access_info.AuthenticationMethod;

        /* �P���x���X�T�[�o�������IP�A�h���X                 */
        cifs_conn.kerberosServerAddress
                                 = ftp_cifs_access_info.KerberosServerAddress;

        /* CIFS�z�X�g�ڑ��m�F                                 */
        ret_value_cifs = cifsclib_IsConnect(&cifs_conn);

    	if ( ret_value_cifs != CIFSCLIB_SUCCESS ) {
            ret_value  = ERROR;
        }
#ifdef	USE_SEPARATE_UI
		else{
            ret_value = OK;
		}
#endif
#endif /* USE_SCAN2NW */
    }
#ifdef	USE_SCAN2SFTP
    else if( service_kind == SCAN2SFTP_SERVICE ) {
		/*1.Host Address*/
		sftp_conn.HostAddress = ftp_cifs_access_info.serveraddress;
		/* 2.Port�ԍ� */
		sftp_conn.PortNumber = ftp_cifs_access_info.portnum;
		/* 3.User�F�ؕ��� */
		sftp_conn.AuthMeth = ftp_cifs_access_info.AuthenticationMethod;
		/* 4.User�� */
		sftp_conn.UserName = ftp_cifs_access_info.username;
		/* 5.Password */
		sftp_conn.Password = ftp_cifs_access_info.password;
		/* 6.SFTP�T�[�o���J�� */
		sftp_conn.PubKeyIdx =  ftp_cifs_access_info.PubKeyIdx;
		/* 7.MFC��pair��ID */
		sftp_conn.PairKeyIdx = ftp_cifs_access_info.PairKeyIdx;
		/* 8.Store Directory */
		sftp_conn.StoreDirectory = ftp_cifs_access_info.storedir;

		/* SFTP�z�X�g�ڑ��m�F */
		ret_value_sftp = sftpclib_IsConnect(&sftp_conn);
		if( ret_value_sftp != SFTPCLIB_ERR_NONE ) {
			ret_value = ERROR;
		}
#ifdef	USE_SEPARATE_UI
		else{
			ret_value = OK;
		}
#endif	/* USE_SEPARATE_UI */
	}
#endif	/* USE_SCAN2SFTP */

#if	defined(USE_BSI) && defined(USE_SERIO_LOG)
	if (Serio_Is_Enabled() == TRUE) {
		/* BSI����User�̍ő�ۑ�Log���𒴂��Ă��邩�̊m�F */
		if(SerioLog_Check_MaxLog_Already() != OK)
		{
			/* Scan�J�n�����ERROR�ŏ㏑������ */
			ret_value  = ERROR;
		}
	}
#endif	/* defined(USE_BSI) && defined(USE_SERIO_LOG) */

	if ( ret_value != OK ) {
		/* �X�L�����ł̎�t���I����҂���                   */
		FOS_TASKSLEEP(20);
		buzzer_start( BUZZER_COMERR, APL_FTPCLIENT );

        if(service_kind       == SCAN2FTP_SERVICE){
		    str_id = decide_error_id     ( ret_value_ftp  );
        }else if(service_kind == SCAN2CIFS_SERVICE){
            str_id = decide_error_id_cifs( ret_value_cifs );
        }
#ifdef	USE_SCAN2SFTP
        else if(service_kind == SCAN2SFTP_SERVICE){
			str_id = decide_error_id_sftp( ret_value_sftp );
		}
#endif	/* USE_SCAN2SFTP */

#ifndef	USE_SEPARATE_UI
		/* �S�̕\���FSendingErr �Ȃ�                          */
		ftpc_disp_string( str_id, NULL );
#ifdef LCD_5LINE
		/* �܍s�\�� */
		ftpc_disp_icon( ERASE_OBJ );
		ftpc_disp_line();
        ftpc_disp_string_NULL( NULL_LINE3 | NULL_LINE4 | NULL_LINE5 );
#else	/* LCD_5LINE */
        /* ��s�ڂ͋�s�Ƃ���(Server���͕\�����Ȃ�)           */
		ftpc_disp_string( DUMMY_LINE2, NULL );	
#endif /* LCD_5LINE */
#else	/* USE_SEPARATE_UI */
		cp_Sts_Apl_Entry( CP_STS_APL_FTPC, (UINT8)str_id );
#endif	/* USE_SEPARATE_UI */

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		if (Serio_Is_Enabled() == TRUE) {
			/* ���s��~�ʒm */
			SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL, SERIO_JOB_SCANSEND);
		}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

		FOS_TASKSLEEP( 500 );
		resource_release( NO_SYSMEMID );

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		if (Serio_Is_Enabled() == TRUE) {
			SendUiEnd_Restart(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL);
			ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_COMMFAIL);
		}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
		set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
		return ( ERROR );
	}

	/* ADF���\�[�X�̊l��                                      */
	if ( RES_OK != resourceforceget( RES_ADF, qid_ftpclient ) ) {
#if defined(TP_SIZE_37) && defined(USE_SEPARATE_UI)
		/* �X�L�����ł̎�t���I����҂���                   */
		FOS_TASKSLEEP( 20 );
		buzzer_refusal();
#endif /* defined(TP_SIZE_37) && defined(USE_SEPARATE_UI) */
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		if (Serio_Is_Enabled() == TRUE) {
			ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
		}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
		resource_release( NO_SYSMEMID );
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
		/* �f�t�H���g���[�h���A�p�^�C�}�Z�b�g                 */
		set_default_mode_ret( DM_SCAN_APL_END, DM_RET_ON );
#endif
#endif	/* USE_SEPARATE_UI */
		return ( ERROR );
	}

	/* VIDEO���\�[�X�̊l��                                    */
	if ( RES_OK != resourceforceget( RES_SCAN_VIDEO, qid_ftpclient ) ) {
#if defined(TP_SIZE_37) && defined(USE_SEPARATE_UI)
		/* �X�L�����ł̎�t���I����҂���                   */
		FOS_TASKSLEEP( 20 );
		buzzer_refusal();
#endif /* defined(TP_SIZE_37) && defined(USE_SEPARATE_UI) */
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		if (Serio_Is_Enabled() == TRUE) {
			ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
		}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
		resource_release( NO_SYSMEMID );
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
		/* �f�t�H���g���[�h���A�p�^�C�}�Z�b�g                 */
		set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
		return ( ERROR );
	}

	/* �A�v����Ԃ�ݒ�                                       */
	manaplset( S_APL_FTPCLIENT, ADF_STOP_KEY_OK+VIDEO_STOP_KEY_OK );

	/* Connecting �\���ύX                                    */
#ifdef FB_SCAN_TYPE
#ifndef	USE_SEPARATE_UI
	/* Connecting -> Scanning �\��                        */
	ftpc_disp_string( SCANNING_DSP, NULL );
#else  /* USE_SEPARATE_UI */
	cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_SCANNING );
#endif	/* USE_SEPARATE_UI */
#else
#ifndef	USE_SEPARATE_UI
	ftpc_disp_string( SCANNING_DSP, NULL );
#else  /* USE_SEPARATE_UI */
	cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_SCANNING );
#endif	/* USE_SEPARATE_UI */

#endif /* FB_SCAN_TYPE */

#ifndef	USE_SEPARATE_UI
	/* ���ʓǎ�̏ꍇ�͂c�A�C�R����\��                       */
	dispDIconFor5Line();
#endif	/* USE_SEPARATE_UI */

	/* ��f�[�^�t�@�C���쐬                                   */
	switch ( ftp_cifs_access_info.fileformat ) {
		case FTP_PDF:
			image_use	= IMAGE_USE_PDF;
			break;
#ifdef USE_PDFA
		case FTP_PDFA:
			image_use	= IMAGE_USE_PDFA;
			break;
#endif /* USE_PDFA */
#ifdef USE_SIGNEDPDF
		case FTP_SIPDF:
			image_use	= IMAGE_USE_SIGNEDPDF;
			break;
#endif /* USE_SIGNEDPDF */
		case FTP_SPDF:
			image_use	= IMAGE_USE_SPDF;
			break;
		case FTP_JPEG:
			image_use	= IMAGE_USE_JPEG;
			break;
		case FTP_Tiff:
			image_use	= IMAGE_USE_TIFF_F;
			break;
		case FTP_XPS:
			image_use	= IMAGE_USE_XPS;
			break;
		default:
			break;
	}
    /* �C���[�W�����C�u�����G���g�����{                       */
	*image_id      = ImageEntry( IMAGE_FILE, image_use );

	if ( *image_id == ERROR ) {

		/* M-AC-2558 Image Entry�ł��Ȃ��Ƃ���                */
        /* Out of Memory <20061127>                           */
#ifndef	USE_SEPARATE_UI
		/* �X�L�����ł̎�t���I����҂���                   */
		FOS_TASKSLEEP( 20 );
		buzzer_refusal();
		ftpc_disp_string( MEMFUL_DSP, NULL );
#ifdef	LCD_5LINE
		ftpc_disp_icon( ERASE_OBJ );
		ftpc_disp_line();
        /* Error�\�����͂R�`�T�s�ڂ���������                  */
        ftpc_disp_string_NULL( NULL_LINE3 | NULL_LINE4 | NULL_LINE5 );
#else	/* LCD_5LINE */
		ftpc_disp_string( DUMMY_LINE2, NULL );
#endif
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		/* ���s��~�ʒm */
		SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL, SERIO_JOB_SCANSEND);
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
#ifdef	STATUS_LED
		/* memory full ���� LED ��ԐF�_��                    */
		dispcntl_statusledcntl( APL_FTPCLIENT, PNL_LED_ON, PNL_LED_RED );
		FOS_TASKSLEEP( 500 );
		dispcntl_statusledcntl( APL_FTPCLIENT, PNL_LED_OFF, PNL_LED_RED );
#elif	defined(USE_ERROR_LED)
		dispcntl_errorledcntl(APL_FTPCLIENT, PNL_LED_ON, PNL_LED_ERROR);
		FOS_TASKSLEEP(500);
		dispcntl_errorledcntl(APL_FTPCLIENT, PNL_LED_OFF, PNL_LED_ERROR);
#else	/* STATUS_LED */
		FOS_TASKSLEEP( 200 );
#endif	/* STATUS_LED */
#else	/* USE_SEPARATE_UI */
		cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_MEMFULL );
		/* �X�L�����ł̎�t���I����҂���                   */
		FOS_TASKSLEEP( 40 );
		soundBuzzerStop(BUZZER_ACCEPT);
		buzzer_refusal();
#endif	/* USE_SEPARATE_UI */

		resource_release( NO_SYSMEMID );
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
		/* �f�t�H���g���[�h���A�p�^�C�}�Z�b�g                 */
		set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		if (Serio_Is_Enabled() == TRUE) {
			SendUiEnd_Restart(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL);
			ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
		}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
		return ( ERROR );
	}

#ifdef USE_SCAN_COLOR_DETECT /* �F���������� */
	conv_scan_quality_to_color_reso(ftp_cifs_access_info.quality, &color_num, NULL);
	if( color_num == COLOR_MODE_AUTO )
	{
		ImageSetParameter( *image_id, IMAGE_COMP, IMAGE_COMP_NO ); /* �F��Auto���A���k�ݒ�͍s��Ȃ� */
	}
	else
#endif /* USE_SCAN_COLOR_DETECT  �F���������� */
	{
		if ( ftp_cifs_access_info.scan_quality == P_SCAN_QUAL_NORMAL ) {
			/* JPEG File Size = S�̂Ƃ��͈��k�ݒ���s�� */
			ImageSetParameter( *image_id, IMAGE_COMP, IMAGE_COMP_FLATE );
		} else {
			/* �����𖞂����Ȃ��ꍇ�͈��k�ݒ薳����ݒ� */
			ImageSetParameter( *image_id, IMAGE_COMP, IMAGE_COMP_NO );
		}
	}

    /* SPDF�̎��A�p�X���[�h��ǉ�����                         */
    if ( ftp_cifs_access_info.fileformat == FTP_SPDF ){
        ImageSetParameter(*image_id, IMAGE_PDF_PASSWORD, 
                          (UINT32)ftp_cifs_access_info.spdfpass);
    }
	/* �V�X�����m��                                           */
    *sysmem_id = scan_get_sysmid_try(	conv_ftp_e2p_code(ftp_cifs_access_info.quality, QUALITY_TBL_FTP_TO_E2P),
#ifdef	USE_TMP_SCANSIZE
    									ftp_cifs_access_info.scan_doc_size,
#else
    									0,
#endif
    									conv_ftp_e2p_code(ftp_cifs_access_info.dualscan, DUALSCAN_TBL_FTP_TO_E2P),
    									scanmenu_conv_quality_to_rate(ftp_cifs_access_info.scan_quality),
    									image_use,
#ifdef USE_SCAN_BLANK_DETECT	/*���������@�\ */
    									conv_ftp_e2p_code(ftp_cifs_access_info.scan_blank_detect, BLANKP_TBL_FTP_TO_E2P),
#else /* USE_SCAN_BLANK_DETECT */
										USW_SCAN_BLNKP_DETECT_OFF,
#endif /* USE_SCAN_BLANK_DETECT */
#ifdef USE_SKEW_ADJUST	/* �΍s�␳ */
										conv_ftp_e2p_code(ftp_cifs_access_info.scan_deskew_adjust, DESKEW_TBL_FTP_TO_E2P)
#else /* USE_SKEW_ADJUST */
										USW_SCAN_DESKEW_OFF
#endif /* USE_SKEW_ADJUST */
									);

	if ( *sysmem_id == ERROR ) {
		/* memory full ���̃G���[����                         */

#ifndef	USE_SEPARATE_UI
		/* memory full �\���A�G���[���ALED�_���Ȃ�            */
#ifdef USE_FAX
		if( ( resourceref(RES_MODEM) != 0 )	/*FAX����M��?*/
#ifdef USE_T38
			|| ( t38fa_use_t38_line() == YES )	/*	T38�ʐM���H	*/
#endif
			){
			ftpc_disp_string( PLSWAIT_DSP, NULL );
		}
		else
#endif /* USE_FAX */
		{
			ftpc_disp_string( MEMFUL_DSP, NULL );
		}
#ifdef	LCD_5LINE
		ftpc_disp_icon( ERASE_OBJ );
		ftpc_disp_line();
        /* Error�\�����͂R�`�T�s�ڂ���������                  */
        ftpc_disp_string_NULL( NULL_LINE3 | NULL_LINE4 | NULL_LINE5 );
#else	/* LCD_5LINE */
		ftpc_disp_string( DUMMY_LINE2, NULL );	
#endif
		/* �X�L�����ł̎�t���I����҂���                   */
		FOS_TASKSLEEP( 20 );
		buzzer_refusal();
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		/* ���s��~�ʒm */
		SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL, SERIO_JOB_SCANSEND);
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
#ifdef	STATUS_LED
		/* memory full ���� LED ��ԐF�_��                    */
		dispcntl_statusledcntl( APL_FTPCLIENT, PNL_LED_ON,  PNL_LED_RED );
		FOS_TASKSLEEP( 500 );
		dispcntl_statusledcntl( APL_FTPCLIENT, PNL_LED_OFF, PNL_LED_RED );
#elif	defined(USE_ERROR_LED)
		dispcntl_errorledcntl(APL_FTPCLIENT, PNL_LED_ON, PNL_LED_ERROR);
		FOS_TASKSLEEP(500);
		dispcntl_errorledcntl(APL_FTPCLIENT, PNL_LED_OFF, PNL_LED_ERROR);
#else	/* STATUS_LED */
		FOS_TASKSLEEP( 200 );
#endif	/* STATUS_LED */
#else	/* USE_SEPARATE_UI */
		cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_MEMFULL );
		/* �X�L�����ł̎�t���I����҂���                   */
		FOS_TASKSLEEP( 40 );
		soundBuzzerStop(BUZZER_ACCEPT);
		buzzer_refusal();
#endif	/* USE_SEPARATE_UI */

		ImageDelete( *image_id );
		resource_release( NO_SYSMEMID );
#ifndef	USE_SEPARATE_UI
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		SendUiEnd_Restart(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL);
		ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
#ifdef	USE_SELECTMODE_KEY
		/* �f�t�H���g���[�h���A�p�^�C�}�Z�b�g                 */
		set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
		return ( ERROR );
	}

	return ( OK );
}


/**
* @par	�X�L�����J�n
* @param	image_id (input) ��f�[�^ID
* @param	sysmem_id (input) �V�X�e��������ID
* @return	�Ȃ�
*
* @par <�O���d�l>
*		�X�L�������J�n����ׂ̃p�����[�^�ݒ�ƃX�L�����J�n�v�����s���B
* @par <�����d�l>
*		�X�L�������J�n����ׂ̃p�����[�^�ݒ�ƃX�L�����J�n�v����Scan Base Task��
*		���b�Z�[�W���M����B
*
* @par <M�[>
*	   M-BCL-945
*/

STATIC void
scan_start( INT32 image_id, INT32 sysmem_id )
{
	UINT8                  i;
    /* �X�L�������샂�[�h           */
	UINT8                  apli_mode; 
    /* ���m�N�����M�ݒ�̃p�����[�^ */
	mult_scan_t            info;
    /* �X�L�����p�����[�^           */
	scannertask_cntl_msg_t scan_param;

#ifndef SCAN_AREA_RC_SEPARATED
	UINT16	doc_scan_area_spec_quality = 0;
#endif /* SCAN_AREA_RC_SEPARATED */
	UINT16	resolution = 0;
	UINT8	doc_scan_area_spec_src     = 0;
	UINT16	doc_scan_area_spec_size    = 0;
	UINT8	color_num = 0;
#ifdef	COLOR_NETWORK_SCANNER
#ifdef	USE_SCAN_AUTO_RESOLUTION
	UINT8	tmp_resolution = SCAN2_RESO_BW200100;
#endif
#endif
	SCAN2FTP_QUALITY	quality;
	
	/* ������           */
	apli_mode					= MEM_SCAN_FAX;
	memset( &info,       NULL, sizeof(mult_scan_t) );
	memset( &scan_param, NULL, sizeof(scannertask_cntl_msg_t) );

	/* ���e ADF or FB �̐ݒ�A                        */
    /*�����ł̐ݒ�l�̓X�L�����I���܂ŕێ�����        */
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	if (Serio_Is_Enabled() == TRUE) {
#ifdef FB_SCAN_TYPE
		if(ftp_cifs_access_info.scan_src == 0){
			if ( MacStateRef( STID_MAN_SOURCE_IN ) == MAC_ON ) {
				scan_src               = SCAN_SRC_ADF;
				doc_scan_area_spec_src = SCAN_SRC_ADF;
			}
			else {
				scan_src               = SCAN_SRC_FB;
				doc_scan_area_spec_src = SCAN_SRC_FB;
			}
		}else if(ftp_cifs_access_info.scan_src == SCAN_SRC_ADF && 
				 MacStateRef( STID_MAN_SOURCE_IN ) == MAC_OFF){
			/* ADF�w�莞�ɁAADF�Ɍ��e�������ꍇ��FB�Ɏw�肵�Ȃ��� */
			scan_src               = SCAN_SRC_FB;
			doc_scan_area_spec_src = SCAN_SRC_FB;
		}else{
			scan_src               = ftp_cifs_access_info.scan_src;
			doc_scan_area_spec_src = ftp_cifs_access_info.scan_src;
		}
#else
		scan_src                   = SCAN_SRC_ADF;
		doc_scan_area_spec_src     = SCAN_SRC_ADF;
#endif
	}
	else
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
	{
#ifdef FB_SCAN_TYPE
		if ( MacStateRef(STID_MAN_SOURCE_IN ) == MAC_ON ) {
			scan_src               = SCAN_SRC_ADF;
			doc_scan_area_spec_src = SCAN_SRC_ADF;
		}
		else {
			scan_src               = SCAN_SRC_FB;
			doc_scan_area_spec_src = SCAN_SRC_FB;
		}
#else
		scan_src                   = SCAN_SRC_ADF;
		doc_scan_area_spec_src     = SCAN_SRC_ADF;
#endif
	}

	/* ���m�N�����M�ݒ� */
	info.outreso_x = P_RES_X_FAX;
	info.edge      = P_EDGE_NORMAL;
	info.mode      = P_TEXT_LEVEL;
	switch ( ftp_cifs_access_info.quality ) {
		case FTP_BW600:
		case FTP_BW300:
		case FTP_BW200:
		case FTP_BW150:
		case FTP_BW100:
			if ( ftp_cifs_access_info.quality == FTP_BW300 ) {
				info.outreso_y = P_RES_Y_SFINE;
			}
			else if ( ftp_cifs_access_info.quality == FTP_BW200 ) {
				info.outreso_y = P_RES_Y_FINE;
			}
			else {
				info.outreso_y = P_RES_Y_STD;
			}
			for ( i=0; i<MAX_SCAN_MAULT; i++ ) {
				resourceSetscanmult( i, &info );
			}
			break;
		default:
			break;
	}

	/* �X�L�����p�����[�^�ݒ�P  �f�t�H���g�l�̃Z�b�g */
	GetScanModeDefault( (scantask_set_msg *)&(scan_param.item.scan_set) );

	/* �X�L�����p�����[�^�ݒ�Q  �Œ�l�̃Z�b�g       */
	scan_param.com_msg.from_task             = FTPC_APL_TASK;
	scan_param.com_msg.cmd_id                = SCAN_SET_COM;
	scan_param.item.scan_set.apli_id         = qid_ftpclient;
	scan_param.item.scan_set.scan_inreso_x   = SCAN_INX_AUTO;
	scan_param.item.scan_set.scan_inreso_y   = SCAN_INY_AUTO;
	scan_param.item.scan_set.scan_x_offset   = 0;
	scan_param.item.scan_set.scan_y_offset   = 0;
	scan_param.item.scan_set.scan_density    = 50;
	scan_param.item.scan_set.scan_contrast   = 50;
	scan_param.item.scan_set.scan_color_jyun = PC_SCAN_RGB;
	scan_param.item.scan_set.scan_kakusyuku  = 100;
	scan_param.item.scan_set.scan_edge       = SCAN_EDGE_AUTO;
	BitOff(scan_param.item.scan_set.scan_image_correct, PAGE_SIZE_DETECT_ON);
	
	/* �p���T�C�Y�擾   */
#ifdef USE_SKEW_ADJUST
	/* �΍s�␳��ON�̂Ƃ��A���e�T�C�Y��Auto�̐ݒ�ɂ��� */
	if( ftp_cifs_access_info.scan_deskew_adjust == FTPNW_DESKEW_ON )
	{
#ifdef USE_SCAN_DOCSIZE_DETECT
#ifndef USE_TMP_SCANSIZE
		fstset_data( FUNC_SCANSIZE , SCANSIZE_AUTO);
#else /* USE_TMP_SCANSIZE */
		ftp_cifs_access_info.scan_doc_size = SCANSIZE_AUTO;
#endif /* USE_TMP_SCANSIZE */
#endif /* USE_SCAN_DOCSIZE_DETECT */
		BitOn(scan_param.item.scan_set.scan_image_correct, DESKEW_ON);
	}
#endif	/* #ifdef USE_SCAN_DOCSIZE_DETECT */

#ifndef USE_TMP_SCANSIZE
	switch(fstget_data( FUNC_SCANSIZE ))
#else	/* USE_TMP_SCANSIZE */
	switch(ftp_cifs_access_info.scan_doc_size)
#endif	/* USE_TMP_SCANSIZE */
	{
		case SCANSIZE_LETTER:
			doc_scan_area_spec_size = SPEC_SIZE_LETTER;
			scan_param.item.scan_set.scan_docsize = SCAN_DOCSIZE_LTR;
			break;
		case SCANSIZE_LEGAL:
			doc_scan_area_spec_size = SPEC_SIZE_LEGAL;
			scan_param.item.scan_set.scan_docsize = SCAN_DOCSIZE_LGL;
			break;
		case SCANSIZE_B5:
			doc_scan_area_spec_size = SPEC_SIZE_B5;
			scan_param.item.scan_set.scan_docsize = SCAN_DOCSIZE_B5;
			break;
		case SCANSIZE_A5:
			doc_scan_area_spec_size = SPEC_SIZE_A5;
			scan_param.item.scan_set.scan_docsize = SCAN_DOCSIZE_A5;
			break;
		case SCANSIZE_B6:
			doc_scan_area_spec_size = SPEC_SIZE_B6;
			scan_param.item.scan_set.scan_docsize = SCAN_DOCSIZE_B6;
			break;
		case SCANSIZE_A6:
			doc_scan_area_spec_size = SPEC_SIZE_A6;
			scan_param.item.scan_set.scan_docsize = SCAN_DOCSIZE_A6;
			break;
		case SCANSIZE_BCARD:
			doc_scan_area_spec_size = SPEC_SIZE_BUSINESS_CARD;
			scan_param.item.scan_set.scan_docsize = SCAN_DOCSIZE_BUSINESS;
			break;
		case SCANSIZE_LONG:
			doc_scan_area_spec_size = SPEC_SIZE_LONG;
			scan_param.item.scan_set.scan_docsize = SCAN_DOCSIZE_LONG_LENGTH;
			break;
#ifdef USE_SCAN_DOCSIZE_DETECT
		case SCANSIZE_AUTO:
			doc_scan_area_spec_size = SPEC_SIZE_AUTO;
			scan_param.item.scan_set.scan_docsize = SCAN_DOCSIZE_AUTO;
			BitOn(scan_param.item.scan_set.scan_image_correct, PAGE_SIZE_DETECT_ON);
			break;
#endif	/* #ifdef USE_SCAN_DOCSIZE_DETECT */
		case SCANSIZE_A4:
		default:
			doc_scan_area_spec_size = SPEC_SIZE_A4;
			scan_param.item.scan_set.scan_docsize = SCAN_DOCSIZE_A4;
			break;
	}

	/* �X�L�����p�����[�^�ݒ�R  �𑜓x�ˑ��l�̃Z�b�g */
	quality = ftp_cifs_access_info.quality;
#ifndef SCAN_AREA_RC_SEPARATED
	switch ( quality ) {
#ifdef	COLOR_NETWORK_SCANNER
		case FTP_Color100:
			doc_scan_area_spec_quality = QUALITY_COLOR_100DPI;
			break;
		case FTP_Color200:
			doc_scan_area_spec_quality = QUALITY_COLOR_200DPI;
			break;
		case FTP_Color300:
			doc_scan_area_spec_quality = QUALITY_COLOR_300DPI;
			break;
		case FTP_Color600:
			doc_scan_area_spec_quality = QUALITY_COLOR_600DPI;
			break;
		case FTP_Gray100:
			doc_scan_area_spec_quality = QUALITY_GRAY_100DPI;
			break;
		case FTP_Gray200:
			doc_scan_area_spec_quality = QUALITY_GRAY_200DPI;
			break;
		case FTP_Gray300:
			doc_scan_area_spec_quality = QUALITY_GRAY_300DPI;
			break;

#if defined(USE_SCAN_AUTO_RESOLUTION)
		case FTP_ColorAuto:
		case FTP_GrayAuto :
			/* Scan to FTP��Scan to Network�����̃��[�g�ŏ�������� */
			if(scanmenu_EiScanGetResolution(SCAN_APP_FTP, ftp_cifs_access_info.quality, ftp_cifs_access_info.fileformat, ftp_cifs_access_info.scan_quality, &doc_scan_area_spec_quality) != OK)
			{
				EPRINTF(("%s(%d)	scanmenu_EiScanGetResolution ERROR!!\n",__FILE__,__LINE__));
			}
			else
			{
				/* Sending�\�����Ɏ��ۂ̉𑜓x�\�����s�����߁Atmp�̓��e���X�V���� */
				switch ( doc_scan_area_spec_quality )
				{
					case QUALITY_COLOR_100DPI:
						tmp_resolution = SCAN2_RESO_CL100DPI;
						break;
					case QUALITY_COLOR_150DPI:
						tmp_resolution = SCAN2_RESO_CL150DPI;
						break;
					case QUALITY_COLOR_200DPI:
						tmp_resolution = SCAN2_RESO_CL200DPI;
						break;
					case QUALITY_COLOR_300DPI:
						tmp_resolution = SCAN2_RESO_CL300DPI;
						break;
					case QUALITY_COLOR_400DPI:
						tmp_resolution = SCAN2_RESO_CL400DPI;
						break;
					case QUALITY_COLOR_600DPI:
						tmp_resolution = SCAN2_RESO_CL600DPI;
						break;
					case QUALITY_GRAY_100DPI:
						tmp_resolution = SCAN2_RESO_GR100DPI;
						break;
					case QUALITY_GRAY_150DPI:
						tmp_resolution = SCAN2_RESO_GR150DPI;
						break;
					case QUALITY_GRAY_200DPI:
						tmp_resolution = SCAN2_RESO_GR200DPI;
						break;
					case QUALITY_GRAY_300DPI:
						tmp_resolution = SCAN2_RESO_GR300DPI;
						break;
					case QUALITY_GRAY_400DPI:
						tmp_resolution = SCAN2_RESO_GR400DPI;
						break;
					case QUALITY_GRAY_600DPI:
						tmp_resolution = SCAN2_RESO_GR600DPI;
						break;
					default:
						/* �z��O�̏ꍇ��Default�Ƃ���BW200100�����Ă��� */
						tmp_resolution = SCAN2_RESO_BW200100;
						break;
				}

				if( (service_kind == SCAN2FTP_SERVICE)
#ifdef	USE_SCAN2SFTP
					|| (service_kind == SCAN2SFTP_SERVICE)
#endif	/* USE_SCAN2SFTP */
					)
				{
					fstset_data(FUNC_TMP_SFTP_RESO,tmp_resolution);
				}
				else if(service_kind == SCAN2CIFS_SERVICE)
				{
					fstset_data(FUNC_TMP_SNW_RESO,tmp_resolution);
				}
			}
			break;
#endif

#endif	/* COLOR_NETWORK_SCANNER */
		case FTP_BW300:
			doc_scan_area_spec_quality = QUALITY_BW_300DPI;
			break;
		case FTP_BW200:
			doc_scan_area_spec_quality = QUALITY_BW_200DPI;
			break;
		case FTP_BW100:
		default:
			doc_scan_area_spec_quality = QUALITY_BW_200100DPI;
			break;
	}
	/* �呖�������̓ǎ�͈͂̉�f���擾               */
	scan_param.item.scan_set.scan_area_x    =         GetDocScanAreaSizeX(doc_scan_area_spec_quality, doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* �����������̓ǎ�͈͂̃��C�����擾             */
	scan_param.item.scan_set.scan_area_y    =         GetDocScanAreaSizeY(doc_scan_area_spec_quality,  doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* �呖�������̓ǎ�͈͂̍��[�]���擾             */
	scan_param.item.scan_set.scan_x_offset  =         GetDocScanOffsetX  (doc_scan_area_spec_quality,  doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* �����������̓ǎ�͈͂̐�[�]��                 */
	scan_param.item.scan_set.scan_y_offset  =         GetDocScanOffsetY  (doc_scan_area_spec_quality, doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* �呖�������̓ǎ�͈͂̉𑜓x�擾               */
	scan_param.item.scan_set.scan_outreso_x =         GetDocScanResoX    (doc_scan_area_spec_quality, doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* �����������̓ǎ�͈͂̉𑜓x�擾               */
	scan_param.item.scan_set.scan_outreso_y =         GetDocScanResoY    (doc_scan_area_spec_quality, doc_scan_area_spec_src, doc_scan_area_spec_size);

#endif /* SCAN_AREA_RC_SEPARATED */

	/* �F���Ɠǎ�𑜓x�̎擾 */
	conv_scan_quality_to_color_reso( quality, &color_num, &resolution);

#ifdef USE_SCAN_COLOR_DETECT /* �F���������� */
	if(color_num == COLOR_MODE_AUTO) {	/* �F���������ʐݒ� */
		BitOn(scan_param.item.scan_set.scan_image_correct, COLOR_MODE_DETECT_ON);

		scan_param.item.scan_set.scan_mode_switch	= SCAN_MODE_COLOR_MULTIPAGE;
		scan_param.item.scan_set.scan_mode_colorpage	= P_COLOR24BIT;
		scan_param.item.scan_set.scan_mode_graypage	= P_GRAY256;
		scan_param.item.scan_set.scan_mode_bwpage	= P_TEXT_LEVEL;

		scan_param.item.scan_set.scan_color_switch	= SCAN_COLOR_MULTIPAGE;
		scan_param.item.scan_set.scan_color_colorpage= P_COLOR_YCbCr;
		scan_param.item.scan_set.scan_color_graypage	= P_BLACKWHITE;
		scan_param.item.scan_set.scan_color_bwpage	= P_BLACKWHITE;

		scan_param.item.scan_set.scan_compress_switch	= SCAN_COMPRESS_COLOR_MULTIPAGE;
		scan_param.item.scan_set.scan_compress_colorpage	= SCAN_JPEG;
		scan_param.item.scan_set.scan_compress_graypage	= SCAN_JPEG;
		scan_param.item.scan_set.scan_compress_bwpage	= SCAN_MH_REVERSE;

		scan_param.item.scan_set.scan_quality_switch		= SCAN_QUALITY_COLOR_MULTIPAGE;
		scan_param.item.scan_set.scan_quality_colorpage	= ftp_cifs_access_info.scan_quality;
		scan_param.item.scan_set.scan_quality_graypage	= ftp_cifs_access_info.scan_quality_gray;
		scan_param.item.scan_set.scan_quality_bwpage		= P_SCAN_QUAL_NORMAL;

	} else {
		BitOff(scan_param.item.scan_set.scan_image_correct, COLOR_MODE_DETECT_ON);
		scan_param.item.scan_set.scan_mode_switch	 = SCAN_MODE_NORMAL;
		scan_param.item.scan_set.scan_color_switch	 = SCAN_COLOR_NORMAL;
		scan_param.item.scan_set.scan_compress_switch = SCAN_COMPRESS_NORMAL;
		scan_param.item.scan_set.scan_quality_switch	 = SCAN_QUALITY_NORMAL;
	}

#else	/* #ifdef USE_SCAN_COLOR_DETECT */
	BitOff(scan_param.item.scan_set.scan_image_correct, COLOR_MODE_DETECT_ON);
	scan_param.item.scan_set.scan_mode_switch	 = SCAN_MODE_NORMAL;
	scan_param.item.scan_set.scan_color_switch	 = SCAN_COLOR_NORMAL;
	scan_param.item.scan_set.scan_compress_switch = SCAN_COMPRESS_NORMAL;
	scan_param.item.scan_set.scan_quality_switch	 = SCAN_QUALITY_NORMAL;
#endif	/* #ifdef USE_SCAN_COLOR_DETECT */

#ifdef	USE_SCAN_BLANK_DETECT	/* ���������ݒ� */
	if(ftp_cifs_access_info.scan_blank_detect == FTPNW_BLANK_DETECT_ON) {
		BitOn(scan_param.item.scan_set.scan_image_correct, BLANK_PAGE_DETECT_ON);
	} else {
		BitOff(scan_param.item.scan_set.scan_image_correct, BLANK_PAGE_DETECT_ON);
		ftpclient_BlankPage(FTPC_PGCNT_UNUSE, NULL);
	}
#else
	BitOff(scan_param.item.scan_set.scan_image_correct, BLANK_PAGE_DETECT_ON);
	ftpclient_BlankPage(FTPC_PGCNT_UNUSE, NULL);
#endif	/* USE_SCAN_BLANK_DETECT */

#ifdef	SCAN_MF_DETECT /* �d�����m */
	if(ftp_cifs_access_info.scan_multifeed_detect == FTPNW_MULTIFEED_DETECT_ON) {
		scan_param.item.scan_set.scan_multifeed_detect	 = SCAN_MULTIFEED_DETECT_ON;
	} else {
		scan_param.item.scan_set.scan_multifeed_detect	 = SCAN_MULTIFEED_DETECT_OFF;
	}
#else	/* #ifdef	SCAN_MF_DETECT */
	scan_param.item.scan_set.scan_multifeed_detect	 = SCAN_MULTIFEED_DETECT_OFF;
#endif	/* #ifdef	SCAN_MF_DETECT */

#ifdef SCAN_AREA_RC_SEPARATED
	/* �呖�������̓ǎ�͈͂̉�f���擾 */
	scan_param.item.scan_set.scan_area_x = GetDocScanAreaSizeX_RCseparated(color_num, resolution,doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* �呖�������̓ǎ�͈͂̍��[�]���擾 */
	scan_param.item.scan_set.scan_x_offset = GetDocScanOffsetX_RCseparated(color_num, resolution,doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* �呖�������̓ǎ�͈͂̉𑜓x�擾 */
	scan_param.item.scan_set.scan_outreso_x = GetDocScanResoX_RCseparated(color_num, resolution,doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* �����������̓ǎ�͈͂̃��C�����擾 */
	scan_param.item.scan_set.scan_area_y = GetDocScanAreaSizeY_RCseparated(color_num, resolution,doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* �����������̓ǎ�͈͂̐�[�]�� */
	scan_param.item.scan_set.scan_y_offset = GetDocScanOffsetY_RCseparated(color_num, resolution,doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* �����������̓ǎ�͈͂̉𑜓x�擾 */
	scan_param.item.scan_set.scan_outreso_y = GetDocScanResoY_RCseparated(color_num, resolution,doc_scan_area_spec_src, doc_scan_area_spec_size);
#endif /* SCAN_AREA_RC_SEPARATED */

	/* �X�L�����p�����[�^�ݒ�S                       */
    /* �J���[�^���m�N���ŋ�ʂ���l�̃Z�b�g           */
	switch ( color_num ) {
#ifdef	COLOR_NETWORK_SCANNER
#ifdef USE_SCAN_COLOR_DETECT /* �F���������� */
        case COLOR_MODE_AUTO:
			apli_mode                                  = MEM_SCAN_SCANNER;
			scan_param.item.scan_set.scan_doc_type     = P_ORIGINAL_PHOTO;
			break;
#endif
        case COLOR_MODE_COLOR:
			apli_mode                                  = MEM_SCAN_SCANNER;
			scan_param.item.scan_set.scan_mode         = P_COLOR24BIT;
			scan_param.item.scan_set.scan_color        = P_COLOR_YCbCr;
			scan_param.item.scan_set.scan_doc_type     = P_ORIGINAL_PHOTO;
			scan_param.item.scan_set.scan_compress     = SCAN_JPEG;
			break;
		case COLOR_MODE_GRAY:
			apli_mode                                  = MEM_SCAN_SCANNER;
			scan_param.item.scan_set.scan_mode         = P_GRAY256;
			scan_param.item.scan_set.scan_color        = P_BLACKWHITE;
			scan_param.item.scan_set.scan_doc_type     = P_ORIGINAL_TEXT;
			scan_param.item.scan_set.scan_compress     = SCAN_JPEG;
			break;
#endif	/* COLOR_NETWORK_SCANNER */
		case COLOR_MODE_BW:
		default:
			apli_mode                                  = MEM_SCAN_FAX;
			scan_param.item.scan_set.scan_mode         = SCAN_MODE_AUTO;
			scan_param.item.scan_set.scan_color        = P_BLACKWHITE;
			scan_param.item.scan_set.scan_doc_type     = P_ORIGINAL_TEXT;
			if ( (ftp_cifs_access_info.fileformat == FTP_PDF ) ||
#ifdef USE_PDFA
                 (ftp_cifs_access_info.fileformat == FTP_PDFA) ||
#endif /* USE_PDFA */
#ifdef USE_SIGNEDPDF
                 (ftp_cifs_access_info.fileformat == FTP_SIPDF) ||
#endif /* USE_SIGNEDPDF */
                 (ftp_cifs_access_info.fileformat == FTP_SPDF)   ) {
				scan_param.item.scan_set.scan_compress = SCAN_MH_REVERSE;
			}
			else {
				scan_param.item.scan_set.scan_compress = SCAN_MH;
			}
			break;
	}

	/* �X�L�����p�����[�^�ݒ�T                       */
    /* ���ʓǎ�ݒ�l�̃Z�b�g                         */
#ifdef	USE_DUPLEX_SCAN
	if ( fstget_data( FSW_DUPLEX_SCAN_FUNC ) == FSW_DUPLEX_SCAN_FUNC_ON ) {
		if ( ftp_cifs_access_info.dualscan == FTP_DUPLEXLONG ) {
            scan_param.item.scan_set.scan_duplex = SCAN_DX_LONG;
		}
		else if ( ftp_cifs_access_info.dualscan == FTP_DUPLEXSHORT ) {
            scan_param.item.scan_set.scan_duplex = SCAN_DX_SHORT;
		}
		else {
            scan_param.item.scan_set.scan_duplex = SCAN_SIMPLEX;
		}
	}
	else {
        scan_param.item.scan_set.scan_duplex     = SCAN_SIMPLEX;
	}
#endif	/* USE_DUPLEX_SCAN */

#if	defined(USE_BSI) && defined(USE_SERIO_LOG)
	if (Serio_Is_Enabled() == TRUE) {
		/* BSI����User�̗��p����ۑ��J�n */
		set_scan_ftp_cifs_setting_bsilog();
	}
#endif	/* defined(USE_BSI) && defined(USE_SERIO_LOG) */

    /* Scan���k��                         */
    scan_param.item.scan_set.scan_quality = ftp_cifs_access_info.scan_quality;

	/* Ground Color Removal OFF�Œ� */
#ifdef    USE_GNDCOLOR_REMOVAL
	scan_param.item.scan_set.gndcolor_removal = ftp_cifs_access_info.gndcolor_removal;
	scan_param.item.scan_set.gndcolor_level = ftp_cifs_access_info.gndcolor_level;
#endif /* USE_GNDCOLOR_REMOVAL */

	/* �X�L�����p�����[�^���M                         */
	FOS_MSGSEND( qid_scanbase, (UINT8 *)&scan_param, sizeof(scannertask_cntl_msg_t) );

	/* �X�L�����J�n�R�}���h���M                       */
	send_scan_start_end( MEM_SCAN_START, apli_mode, image_id, sysmem_id );

	return;
}

/**
* @par	�X�L�������f
* @param	from_task (input) ���b�Z�[�W���M���^�X�NID
* @param	cmd_id (input) ���b�Z�[�W���̃R�}���hID�i���b�Z�[�W���M���^�X�N�̏ꍇ�͑��u��ԁj
* @param	image_id (input) ��f�[�^ID
* @param	sysmem_id (input) �V�X�e��������ID
* @return	OK�F����I��
*			ERROR�F�ُ�I��
*
* @par <�O���d�l>
*		�X�L�����𒆒f����ׂ̏������s���X�L�����𒆒f����B
* @par <�����d�l>
*		�X�L�������f��LCD�\����u�U�[��炵�A�X�L�������f�v����Scan Base Task��
*		���b�Z�[�W���M����B
*
* @par <M�[>
*	   M-BCL-945
*/

STATIC void
send_scan_stop( void )
{
	scannertask_cntl_msg_t scan_param;

	/* ������ */
	memset( &scan_param, NULL, sizeof(scannertask_cntl_msg_t) );

	scan_param.com_msg.from_task               = FTPC_APL_TASK;
	scan_param.com_msg.cmd_id                  = MEM_SCAN_STOP;
	scan_param.item.memory_scan_stop.save_mode = FSAVE_PAGEEND;
	FOS_MSGSEND( qid_scanbase, (UINT8 *)&scan_param, sizeof(scannertask_cntl_msg_t) );

	return;
}

/**
* @par �X�L�����J�n�E�I�����b�Z�[�W���M
* @param	cmd_id (input) ���b�Z�[�W���̃R�}���hID�i���b�Z�[�W���M���^�X�N�̏ꍇ�͑��u��ԁj
* @param	apli_mode (input) �X�L�������[�h
* @param	image_id (input) ��f�[�^ID
* @param	sysmem_id (input) �V�X�e��������ID
* @return	�Ȃ�
*
* @par <�O���d�l>
*		�X�L�����J�n�ƃX�L�����I���̃��b�Z�[�W�𑗐M����B
* @par <�����d�l>
*		�X�L�����J�n�ƃX�L�����I���̃��b�Z�[�W��Scan Base Task�֑��M����B
*
* @par <M�[>
*	   M-BCL-945
*/
STATIC void
send_scan_start_end( UINT16 cmd_id, UINT8 apli_mode, INT32 image_id, INT32 sysmem_id )
{
	scannertask_cntl_msg_t		scan_param;

	/* ������                 */
	memset( &scan_param, NULL, sizeof(scannertask_cntl_msg_t) );

	/* �X�L�����p�����[�^�ݒ� */
	scan_param.com_msg.from_task					= FTPC_APL_TASK;
	scan_param.com_msg.cmd_id						= cmd_id;
	scan_param.item.memory_scan.mem_scan_apli_mode	= apli_mode;
	if ( apli_mode == MEM_SCAN_SCANNER ) {
		scan_param.item.memory_scan.stid_ctrl		= SCAN_STID_NONE;
	}
	scan_param.item.memory_scan.mem_scan_file_id	= image_id;
	scan_param.item.memory_scan.rsvmem_id			= sysmem_id;
#ifdef FB_SCAN_TYPE
	if ( scan_src == SCAN_SRC_ADF ) {
		scan_param.item.memory_scan.start_ctrl		= SCAN_START_NOWAIT;
	}
	else {
		scan_param.item.memory_scan.start_ctrl		= SCAN_START_WAIT;
	}
#else
	scan_param.item.memory_scan.start_ctrl			= SCAN_START_NOWAIT;
#endif
	scan_param.item.memory_scan.scan_src			= scan_src;
	/* �X�L�����p�����[�^���M */
	FOS_MSGSEND( qid_scanbase, (UINT8 *)&scan_param, sizeof(scannertask_cntl_msg_t) );

	return;
}

/**
* @par	�T�[�o�[�ւ̃f�[�^���M����
* @param	image_id (input) ��f�[�^ID
* @return	�Ȃ�
*
* @par <�O���d�l>
*		�X�L���i�[�œǎ悵���f�[�^���t�@�C���]������B
* @par <�����d�l>
*		�X�L���i�[�œǎ悵���f�[�^���w��̃T�[�o�[�֎w��̃v���g�R���œ]������B
*
* @par <M�[>
*	   M-BCL-945
*/
STATIC void
transfer_process( INT32 image_id )
{
	BOOL               error_flag;       /* �G���[�������������ǂ��� TRUE:�������� */
	UINT32             str_id;           /* �\�����镶����ID                       */
	/* ��f�[�^�����֘A�̕ϐ�      */
	INT32              file_id;
	INT32              page_exist;
	INT32              page_no;
	image_pageinfo_t   image_pageinfo;
	/* �l�b�g���[�N���M�֘A�̕ϐ�  */
	BN_FTPC_HANDLE     server_handle;    /* �T�[�o�ڑ��œ�����n���h��           */
#ifdef	USE_SCAN2SFTP
	BN_SFTPC_HANDLE    server_handle_sftp;
#endif	/* USE_SCAN2SFTP */
	INT32              ret_value;        /* API�֐��̕Ԃ�l                        */
    INT32			   ret_value_ftp;
    INT32			   ret_value_cifs;
#ifdef	USE_SCAN2SFTP
	INT32			   ret_value_sftp;
#endif	/* USE_SCAN2SFTP */
	UINT8              filename[ SCAN2FTP_FNAME_MAXSIZE ];
                                         /* �T�[�o�ւ̏o�̓t�@�C����               */
	SCAN2FTP_FILEFORMAT		
					   fileformat;	     /* �t�@�C���t�H�[�}�b�g */
	UINT32             counter; 	 	 /* JPEG�t�@�C�����߂̘A���ԍ�(FB+ADF�J�E���^�l) */

    stcPROTOCOLImageData
                        snd_img_data;    /* ���M����C���[�W�f�[�^���\����       */
    stcFTPConnect       ftp_conn;        /* FTP�ڑ����\����                      */
#ifdef	USE_SCAN2NW
    stcCIFSConnect      cifs_conn;       /* CIFS�ڑ����\����                     */
#endif
#ifdef	USE_SCAN2SFTP
	stcSFTPConnect		sftp_conn;		 /* SFTP�ڑ����\���� */
#endif	/* USE_SCAN2SFTP */

	/* ������ */
	error_flag       = FALSE;
#ifdef	USE_SEPARATE_UI
	str_id           =  OK; /* QAC�΍�F���ۂɎg�p����邱�Ƃ͂Ȃ� */
#else	/* USE_SEPARATE_UI */
    str_id           = MAIL_SENDERR_DSP;
#endif	/* USE_SEPARATE_UI */
    ret_value        = ERROR;
    ret_value_ftp    = ERROR;
    ret_value_cifs   = ERROR;
#ifdef	USE_SCAN2SFTP
    ret_value_sftp   = ERROR;
#endif	/* USE_SCAN2SFTP */
	/* ��f�[�^�����֘A�̕ϐ�      */
	file_id          = ERROR;            /* �t�@�C���f�B�X�N���v�^  ImageOpen�p    */
	page_exist       = ERROR;            /* �y�[�W�����݂��邩�ǂ���               */
	page_no          = 0;                /* �y�[�W�ԍ�                             */
	memset( &image_pageinfo, NULL, sizeof(image_pageinfo_t) );
	/* �l�b�g���[�N���M�֘A�̕ϐ�  */
	server_handle    = NULL;
#ifdef	USE_SCAN2SFTP
	server_handle_sftp = NULL;
#endif	/* USE_SCAN2SFTP */
	ret_value        = FTPCLIB_ERROR_EXEC;
	memset(  filename, NULL,                 SCAN2FTP_FNAME_MAXSIZE );
	memcpy ( filename, ftp_cifs_access_info.filename,
                                             SCAN2FTP_FNAME_MAXSIZE );
	fileformat		 = ftp_cifs_access_info.fileformat;
	counter	         = jpeg_scan_counter;

	/* ��������                    */
#ifndef	USE_SEPARATE_UI
    /* �A�v���\�����m��            */
	ftpc_disp_rightchg( GET_RIGHT );
    /* Sending �\��                */
	ftpc_disp_string  ( MAIL_SEND_DSP, NULL );
#else  /* USE_SEPARATE_UI */
	cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_SENDING );
#endif	/* USE_SEPARATE_UI */

    /* Service��ScanToFtp�̏ꍇ    */
    if (service_kind       == SCAN2FTP_SERVICE) {
#ifdef USE_SCAN2FTP
        /* FTP�z�X�g�ڑ����{       */
        ftp_conn.HostAddress    = ftp_cifs_access_info.serveraddress;
        ftp_conn.StoreDirectory = ftp_cifs_access_info.storedir;
        ftp_conn.IsPassive      = ftp_cifs_access_info.ispassive;
        ftp_conn.PortNumber     = ftp_cifs_access_info.portnum;
        ftp_conn.UserName       = ftp_cifs_access_info.username;
        ftp_conn.Password       = ftp_cifs_access_info.password;

        ret_value_ftp   = ftpclib_Connect(&ftp_conn, &server_handle);

    	if ( ret_value_ftp == FTPCLIB_SUCCESS ) {
            ret_value = OK;
        }else{
            ret_value = ERROR;
        }
#endif

    /* Service��ScanToCIFS�̏ꍇ   */
    }
#ifdef	USE_SCAN2NW
    else if (service_kind == SCAN2CIFS_SERVICE)
    {
        /* CIFS�z�X�g�ڑ����{      */
        cifs_conn.HostAddress    = ftp_cifs_access_info.serveraddress;

        cifs_conn.StoreDirectory = ftp_cifs_access_info.storedir;

        cifs_conn.UserName       = ftp_cifs_access_info.username;

        cifs_conn.Password       = ftp_cifs_access_info.password;

        cifs_conn.AuthenticationMethod
                                 = ftp_cifs_access_info.AuthenticationMethod;

        cifs_conn.kerberosServerAddress
                                 = ftp_cifs_access_info.KerberosServerAddress;

        /* CIFS���OK              */
        ret_value = OK;
    }
#endif
#ifdef	USE_SCAN2SFTP
	else if(service_kind == SCAN2SFTP_SERVICE)
	{
		/* SFTP�����{�̂��߂̏��� */
		sftp_conn.HostAddress = ftp_cifs_access_info.serveraddress;
		sftp_conn.PortNumber = ftp_cifs_access_info.portnum;
		sftp_conn.AuthMeth = ftp_cifs_access_info.AuthenticationMethod;
		sftp_conn.UserName = ftp_cifs_access_info.username;
		sftp_conn.Password = ftp_cifs_access_info.password;
		sftp_conn.PubKeyIdx = ftp_cifs_access_info.PubKeyIdx;
		sftp_conn.PairKeyIdx = ftp_cifs_access_info.PairKeyIdx;
		sftp_conn.StoreDirectory = ftp_cifs_access_info.storedir;

		/* SFTP Server�ڑ����{ */
		ret_value_sftp = sftpclib_Connect( &sftp_conn, &server_handle_sftp );

		if( ret_value_sftp == SFTPCLIB_ERR_NONE ) {
			ret_value = OK;
		} else {
			ret_value = ERROR;
		}
	}
#endif	/* USE_SCAN2SFTP */

	/* �T�[�o�[�ڑ�����            */
	if ( ret_value == OK ) {
		/* ��f�[�^�I�[�v��        */
		file_id	= ImageOpen( image_id, IMAGE_READ );

		/* ��f�[�^�I�[�v������    */
		if ( file_id != ERROR ) {
            /* �t�@�C���t�H�[�}�b�g JPEG�̏ꍇ            */
            if (fileformat == FTP_JPEG){
                for ( page_no=1; ; page_no++ ) {
                    /* �o�̓t�@�C�������쐬               */
                    
                    decide_filename( counter, page_no, filename, ftp_cifs_access_info.FileNameFixed);
                    /* �y�[�W���ǂݎ��                 */
					page_exist	= ImageReadPageInf( file_id, page_no, &image_pageinfo   );
					/* �Ō�̃y�[�W�܂œǂݍ���         */
					if ( page_exist != OK ) {
						break;
					}
                    /* �I�[�v��������f�[�^�̃t�@�C��ID   */
                    snd_img_data.FileId   = file_id;
                    /* �y�[�W�ԍ����w��                   */
                    snd_img_data.PageNo   = page_no;
                    /* �t�@�C�������w��                   */
                    snd_img_data.FileName = filename;
                    /* �t�@�C���`��                       */
                    snd_img_data.FileType = get_filetype(fileformat);

                    /* Service��ScanToFtp�̏ꍇ           */
                    if (service_kind      == SCAN2FTP_SERVICE) {
#ifdef USE_SCAN2FTP
                        /* FTP �C���[�W����               */
                        ret_value_ftp = ftpclib_WriteImage (server_handle, &snd_img_data);
    	                if ( ret_value_ftp == FTPCLIB_SUCCESS ) {
                            ret_value = OK;
							#if	defined(USE_BSI)
								if (Serio_Is_Enabled() == TRUE) {
                            		SendJobProgress_TransFileName(SERIO_JOB_SCANSEND, filename);
								}
							#endif	
                        }else{
                            ret_value = ERROR;
                        }
#endif /* USE_SCAN2FTP */
                    /* Service��ScanToCIFS�̏ꍇ          */
                    }else if (service_kind == SCAN2CIFS_SERVICE){
#ifdef	USE_SCAN2NW
                        /* CIFS �C���[�W����              */
                        ret_value_cifs = cifsclib_WriteImage(&cifs_conn, &snd_img_data);
    	                if ( ret_value_cifs == CIFSCLIB_SUCCESS ) {
                            ret_value = OK;
							#if	defined(USE_BSI)
								if (Serio_Is_Enabled() == TRUE) {
                            		SendJobProgress_TransFileName(SERIO_JOB_SCANSEND, filename);
								}
							#endif	
                        }else{
                            ret_value = ERROR;
                        }
#endif /* USE_SCAN2NW  */
                    }
#ifdef	USE_SCAN2SFTP
					else if(service_kind == SCAN2SFTP_SERVICE) {
						ret_value_sftp = sftpclib_WriteImage( server_handle_sftp, &snd_img_data );
						if ( ret_value_sftp == SFTPCLIB_ERR_NONE ) {
							ret_value = OK;
							#if	defined(USE_BSI)
								if (Serio_Is_Enabled() == TRUE) {
                            		SendJobProgress_TransFileName(SERIO_JOB_SCANSEND, filename);
								}
							#endif	
						}else{
							ret_value = ERROR;
						}
					}
#endif	/* USE_SCAN2SFTP */

                    /* �������ݎ��s                       */
                    if ( ret_value != OK ) {
					    error_flag		  = TRUE;
                    }
				    
#if	defined(USE_BSI) && defined(USE_SERIO_LOG)
					/* ���p����Log�ۑ��̐��ۊm�F */
					if (Serio_Is_Enabled() == TRUE) {
						if( SerioLog_Rec_CntPrnPage( LOG_DATA_FUNC_SCAN, NULL ) != OK ){
							error_flag = TRUE;
						}
					}
#endif	/* defined(USE_BSI) && defined(USE_SERIO_LOG) */
					
                    /* ���炩�̃G���[�����������ꍇ�́A   */
                    /* ���̃y�[�W�ɐi�܂��ɏI��           */
				    if ( error_flag == TRUE ) {
					    break;
				    }
                }
            }else {
                /* �I�[�v��������f�[�^�̃t�@�C��ID       */
                snd_img_data.FileId       = file_id;

                /* �y�[�W�ԍ�(0)���w��                    */
                snd_img_data.PageNo       = 0;
                /* �t�@�C������ݒ�                       */
                snd_img_data.FileName     = filename;

                /* �t�@�C���`��                           */
                snd_img_data.FileType     = get_filetype(fileformat);

                /* Service��ScanToFtp�̏ꍇ               */
                if (service_kind       == SCAN2FTP_SERVICE) {
#ifdef USE_SCAN2FTP
                    /* FTP �C���[�W����                   */
                    ret_value_ftp  = ftpclib_WriteImage (server_handle, &snd_img_data);
  	                if ( ret_value_ftp == FTPCLIB_SUCCESS ) {
                        ret_value  = OK;
                    }else{
                        ret_value  = ERROR;
                    }
#endif /* USE_SCAN2FTP */
                /* Service��ScanToCIFS�̏ꍇ              */
                }else if (service_kind == SCAN2CIFS_SERVICE){
#ifdef	USE_SCAN2NW
                    /* CIFS �C���[�W����                  */
                    ret_value_cifs = cifsclib_WriteImage(&cifs_conn, &snd_img_data);
  	                if ( ret_value_cifs == CIFSCLIB_SUCCESS ) {
                        ret_value  = OK;
                    }else{
                        ret_value  = ERROR;
                    }
#endif /* USE_SCAN2NW */
                }
#ifdef	USE_SCAN2SFTP
				else if( service_kind == SCAN2SFTP_SERVICE ) {
					ret_value_sftp = sftpclib_WriteImage( server_handle_sftp, &snd_img_data );
					if( ret_value_sftp == SFTPCLIB_ERR_NONE ) {
						ret_value = OK;
					} else {
						ret_value = ERROR;
					}
				}
#endif	/* USE_SCAN2SFTP */

                /* �������ݎ��s                           */
                if ( ret_value != OK ) {
                    error_flag		      = TRUE;
                }
			}
			/* ��f�[�^�N���[�Y      */
			ImageClose( file_id );
		}
		/* ��f�[�^�I�[�v�����s      */
		else {
			error_flag = TRUE;
		}

        /* FTP�z�X�g�ؒf���{         */
        if (service_kind == SCAN2FTP_SERVICE){
#ifdef USE_SCAN2FTP
            ftpclib_Close(server_handle);
#endif
        }
#ifdef	USE_SCAN2SFTP
        else if(service_kind == SCAN2SFTP_SERVICE){
			(void)sftpclib_Close(server_handle_sftp);
		}
#endif	/* USE_SCAN2SFTP */
	}
	/* �z�X�g�ڑ����s                */
	else {
		error_flag     = TRUE;
	}
	
	/* �I������                      */
	if ( error_flag == TRUE ) {
		buzzer_start( BUZZER_COMERR, APL_FTPCLIENT );

        if(service_kind      == SCAN2FTP_SERVICE){
		    str_id = decide_error_id     ( ret_value_ftp  );
        }else if(service_kind == SCAN2CIFS_SERVICE){
            str_id = decide_error_id_cifs( ret_value_cifs );
        }
#ifdef	USE_SCAN2SFTP
        else if(service_kind == SCAN2SFTP_SERVICE){
			str_id = decide_error_id_sftp( ret_value_sftp );
		}
#endif	/* USE_SCAN2SFTP */
#ifndef	USE_SEPARATE_UI
		/* �p�l�����́E�\�����m��    */
		ftpc_key_rightchg( GET_RIGHT );

		/* �S�̕\���FSendingErr �Ȃ� */
		ftpc_disp_string( str_id, NULL );
#ifdef	LCD_5LINE 
		/* �܍s���f�� */
		ftpc_disp_icon( ERASE_OBJ );
		ftpc_disp_line();
        /* Error�\�����͂R�`�T�s�ڂ���������            */
        ftpc_disp_string_NULL( NULL_LINE3 | NULL_LINE4 | NULL_LINE5 );
#else	/* LCD_5Line */
		ftpc_disp_string( DUMMY_LINE2, NULL );
#endif	/* LCD_5LINE */
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		/* ���s��~�ʒm */
		SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL, SERIO_JOB_SCANSEND);
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
		FOS_TASKSLEEP( 500 );

		/* �p�l�����́E�\�����m��    */
		ftpc_key_rightchg( FREE_RIGHT );
#else	/* USE_SEPARATE_UI */
		cp_Sts_Apl_Entry( CP_STS_APL_FTPC, (UINT8)str_id );
#endif	/* USE_SEPARATE_UI */
		
#ifdef USE_BSI
		if (Serio_Is_Enabled() == TRUE) {
			/* ���s��~�ʒm(���u��ԃG���[) */
			SendJobProgress_TransEnd(SERIO_JOB_SCANSEND);
			if (fileformat == FTP_JPEG){
				/* JPEG�̓y�[�W���ɑ��M�o���Ă��܂����ߑ��M�����Ƃ��ďI������B */
				ScanEnd(SERIOFW_JOBSTS_END_COMPLETE, SERIO_EXTERR_COMPLETE);
			}else{
				ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
			}
		}
#endif /* USE_BSI */
	}
	else {
		/* ����I����                */
		buzzer_start( BUZZER_FINISH, APL_FTPCLIENT );
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		if (Serio_Is_Enabled() == TRUE) {
#ifdef USE_SERIO_LOG
			if (fileformat != FTP_JPEG){
				/* JPEG���M�ł͂Ȃ������ꍇ�A������SCAN���ɃJ�E���g����������ۑ����� */
				SerioLog_Rec_AddPage(LOG_DATA_FUNC_SCAN);
			}
#endif	/* USE_SERIO_LOG */
			SendJobProgress_TransEnd(SERIO_JOB_SCANSEND);
			ScanEnd(SERIOFW_JOBSTS_END_COMPLETE, SERIO_EXTERR_COMPLETE);
		}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
	}


#ifndef	USE_SEPARATE_UI
	/* �A�v���\�������              */
	ftpc_disp_rightchg( FREE_RIGHT );
#endif	/* USE_SEPARATE_UI */

	ImageDelete ( image_id );

#ifndef	USE_SEPARATE_UI
#ifdef USE_SELECTMODE_KEY
    /* Default Mode���A�p�^�C�}�Z�b�g*/
    set_default_mode_ret( DM_SCAN_APL_END, DM_RET_ON );
#endif
#endif	/* USE_SEPARATE_UI */
	/* �X�e�[�^�X��ҋ@��� */
    ftpscan_state = FTPC_START_WAIT;
#ifdef	USE_SEPARATE_UI
	cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif

	return;
}

/**
* @par	���\�[�X���
* @param	sysmem_id (input/output) �V�X�e��������ID
* @return	�Ȃ�
*
* @par <�O���d�l>
*		�^�X�N�Ŏg�p�������\�[�X���������B
* @par <�����d�l>
*		�^�X�N�Ŏg�p�������\�[�X�A�p�l�����͌��A�\�������������B
*
* @par <M�[>
*	   M-BCL-945
*/
STATIC void
resource_release( INT32 sysmem_id )
{
    /* ADF   ���\�[�X���        */
	resourcefree     ( RES_ADF );
    /* VIDEO ���\�[�X���        */
	resourcefree     ( RES_SCAN_VIDEO );

#ifndef	USE_SEPARATE_UI
#ifdef LCD_5LINE
	/* ���̉�ʂ֑J�ڂ����Ƃ��ɔ��]�\�����c��Ȃ��悤�ɁA���]�\����߂��Ă��� */
	scan_clear_cursor();
#endif	/* LCD_5LINE */
    /* �p�l�����́E�\�������    */
	ftpc_key_rightchg( FREE_RIGHT );
#endif	/* USE_SEPARATE_UI */

    /* �A�v����Ԃ� READY �ɂ��� */
	manaplset        ( S_APL_FTPCLIENT, APL_READY ); 

	if ( sysmem_id != NO_SYSMEMID ) {
        /* �V�X�������          */
        sysmApliFree ( sysmem_id );
	}

	return;
}

/**
* @par	�ڑ��G���[�������̕\�������񌈒菈��
* @param	error_type (input) FTP���C�u�����̃G���[�R�[�h
* @return	�G���[�R�[�h�ɑΉ����鎫��DB��ID
*
* @par <�O���d�l>
*		FTP/CIFS���C�u�����̃G���[�R�[�h�ɑΉ�����G���[���b�Z�[�WID���擾����B
* @par <�����d�l>
*		FTP/CIFS���C�u�����̃G���[�R�[�h�ɑΉ�����G���[���b�Z�[�WID���擾����B
*
* @par <M�[>
*	   M-BCL-945
*/

STATIC UINT32
decide_error_id( INT32 error_type )
{
	INT32 str_id;

	switch ( error_type ) {
        /* �F�؃G���[                      */
		case FTPCLIB_ERROR_AUTH       :
#ifndef	USE_SEPARATE_UI
			str_id	= AUTHEN_ERR_DSP;
#else	/* USE_SEPARATE_UI */
			str_id = CP_STS_FTPC_AUTHERROR;
#endif	/* USE_SEPARATE_UI */
			break;
		/* �^�C���A�E�g�G���[              */
		case FTPCLIB_ERROR_TIMEOUT    :
#ifndef	USE_SEPARATE_UI
			str_id	= SERVER_TOUT_DSP;
#else	/* USE_SEPARATE_UI */
			str_id	= CP_STS_FTPC_TIMEOUT;
#endif	/* USE_SEPARATE_UI */
			break;
        /* �V���^�b�N�X�G���[�i�����G���[�j*/
        case FTPCLIB_ERROR_SYNTAX     :
        /* ��ԑJ�ڃG���[                  */
        case FTPCLIB_ERROR_STATUS     :
        /* ���s�G���[�i�T�[�o�[���̃G���[�j*/
        case FTPCLIB_ERROR_EXEC       :
        /* �Z�b�V����/�ݒ�G���[           */
        case FTPCLIB_ERROR_SESSION    :
        /* �����G���[                      */
        case FTPCLIB_ERROR_PARAMETER  :
		default:
#ifndef	USE_SEPARATE_UI
			str_id	= MAIL_SENDERR_DSP;
#else	/* USE_SEPARATE_UI */
			str_id	= CP_STS_FTPC_SENDERROR;
#endif	/* USE_SEPARATE_UI */
			break;
	}
	return ( str_id );
}

/**
* @par	�ڑ��G���[�������̕\�������񌈒菈��
* @param	error_type (input) CIFS���C�u�����̃G���[�R�[�h
* @return	�G���[�R�[�h�ɑΉ����鎫��DB��ID
*
* @par <�O���d�l>
*		FTP/CIFS���C�u�����̃G���[�R�[�h�ɑΉ�����G���[���b�Z�[�WID���擾����B
* @par <�����d�l>
*		FTP/CIFS���C�u�����̃G���[�R�[�h�ɑΉ�����G���[���b�Z�[�WID���擾����B
*
* @par <M�[>
*	   M-BCL-945
*/

STATIC UINT32
decide_error_id_cifs( INT32 error_type )
{
	INT32 str_id;

	switch ( error_type ) {
        /* �F�؃G���[                      */
        case CIFSCLIB_ERROR_AUTH      :
#ifndef	USE_SEPARATE_UI
			str_id	= AUTHEN_ERR_DSP;
#else	/* USE_SEPARATE_UI */
			str_id	= CP_STS_FTPC_AUTHERROR;
#endif	/* USE_SEPARATE_UI */
			break;
		/* �^�C���A�E�g�G���[              */
        case CIFSCLIB_ERROR_TIMEOUT   :
#ifndef	USE_SEPARATE_UI
			str_id	= SERVER_TOUT_DSP;
#else	/* USE_SEPARATE_UI */
			str_id	= CP_STS_FTPC_TIMEOUT;
#endif	/* USE_SEPARATE_UI */
			break;
		case CIFSCLIB_ERROR_CLKNOREADY:
		/* �������ݒ�G���[                */
#ifndef	USE_SEPARATE_UI
			str_id	= WRGDATATIME_DSP;
#else	/* USE_SEPARATE_UI */
			str_id	= CP_STS_FTPC_TIMEERROR;
#endif	/* USE_SEPARATE_UI */
			break;
        case CIFSCLIB_ERROR_PARAMETER :
		/* Sending Error                   */
        case CIFSCLIB_ERROR_SENDING   :
		default:
#ifndef	USE_SEPARATE_UI
			str_id	= MAIL_SENDERR_DSP;
#else	/* USE_SEPARATE_UI */
			str_id	= CP_STS_FTPC_SENDERROR;
#endif	/* USE_SEPARATE_UI */
			break;
	}
	return ( str_id );
}

#ifdef	USE_SCAN2SFTP
/**
* @par	�ڑ��G���[�������̕\�������񌈒菈��
* @param	error_type (input) SFTPC���C�u�����̃G���[�R�[�h
* @return	�G���[�R�[�h�ɑΉ����鎫��DB��ID
*
* @par <�O���d�l>
*		FTP/CIFS���C�u�����̃G���[�R�[�h�ɑΉ�����G���[���b�Z�[�WID���擾����B
* @par <�����d�l>
*		FTP/CIFS���C�u�����̃G���[�R�[�h�ɑΉ�����G���[���b�Z�[�WID���擾����B
*
* @par <M�[>
*	   
*/
STATIC UINT32
decide_error_id_sftp( INT32 error_type )
{
	INT32	str_id;

	switch( error_type ) {
		case	SFTPCLIB_ERR_SSHINIT:
		case	SFTPCLIB_ERR_SUBSYS:
		case	SFTPCLIB_ERR_INIT:
		case	SFTPCLIB_ERR_DIR:
		case	SFTPCLIB_ERR_UPLOAD:
		case	SFTPCLIB_ERR_QUIT:
		case	SFTPCLIB_ERROR_BUSY:
		case	SFTPCLIB_ERROR_PARAMETER:
			str_id = CP_STS_FTPC_SENDERROR;
			break;
		case	SFTPCLIB_ERR_TIMEOUT:
		case	SFTPCLIB_ERR_SSHDISCON:
			str_id = CP_STS_FTPC_TIMEOUT;
			break;
		case	SFTPCLIB_ERR_SSHVER:
		case	SFTPCLIB_ERR_SSHKEX:
		case	SFTPCLIB_ERR_USRAUTH:
		case	SFTPCLIB_ERR_SESSION:
			str_id = CP_STS_FTPC_AUTHERROR;
			break;
		default:
			str_id = CP_STS_FTPC_SENDERROR;
			break;
	}
	return( str_id );
}
#endif	/* USE_SCAN2SFTP */

/**
* @par	�t�@�C�����쐬����
* @param	counter (input) FB+ADF�J�E���^�l 
* @param	page_no (input) �y�[�W�ԍ�
* @param	*filename (output) �쐬�����t�@�C����
* @return	�Ȃ�
*
* @par <�O���d�l>
*		JPEG�t�@�C���̃t�@�C�������쐬����B�i�g���q�͍쐬���Ȃ��B�j
* @par <�����d�l>
*		JPEG�t�@�C���̃t�@�C�������ȉ��̂悤�ɍ쐬����B
*			hostname_[FB+ADF�J�E���^�l]		�i1�y�[�W�ځj
*			hostname_[FB+ADF�J�E���^�l]_[�y�[�W�ԍ�]	�i�P�y�[�W�ȍ~�j
*
* @par <M�[>
*	   M-BCL-945
*/

STATIC void
decide_filename( UINT32 counter, UINT8 page_no, UINT8 *filename, UINT8 FileNameFixed )
{
	UINT8		filename_top[ SCAN2FTP_FNAME_MAXSIZE ];  /* �t�@�C�����̐擪������ */
	UINT8		extension[ 3 + 1 ];                       /* �g���q                 */
	MD_CHAR		jpeg_scan_counter_str[JPEG_SCAN_CNT_SIZE]; 
	MD_CHAR		*p;

	/* ������ */
	memset( filename_top,  NULL, SCAN2FTP_FNAME_MAXSIZE );
	memset( extension, NULL, 3+1 );
	memset( jpeg_scan_counter_str, NULL, JPEG_SCAN_CNT_SIZE );

	strncpy( (MD_CHAR*)extension, FILETYPE_JPG, 3 );
	
	/* �t�@�C�����o��                                            */
	/* �o�̓t�@�C�����͎��̒ʂ�i�@�\�d�l�j                      */
	/*       hostname_[FB+ADF�J�E���^�l].[�g���q]                */
	/*       hostname_[FB+ADF�J�E���^�l]_[�y�[�W�ԍ�].[�g���q]   */
	
	if(FALSE == FileNameFixed){
		sprintf( jpeg_scan_counter_str, "_%06ld", counter );

	/* �t�@�C�����擾(counter������Ō���)                      */
	p = strstr( (MD_CHAR *)filename, jpeg_scan_counter_str );
	if ( p==NULL ) {
		strncpy( (MD_CHAR *)filename_top, (MD_CHAR *)filename, SCAN2FTP_FNAME_MAXSIZE );
	}
	else {
		strncpy( (MD_CHAR *)filename_top, (MD_CHAR *)filename, (p - (MD_CHAR *)filename) );
	}

		if ( page_no==1 ) {
			sprintf( (char *)filename, "%s%s.%s",
	                 (MD_CHAR *)filename_top, jpeg_scan_counter_str,          (MD_CHAR *)extension );
		}
		else if ( page_no > 1 ) {
			sprintf( (char *)filename, "%s%s_%d.%s",
	                 (MD_CHAR *)filename_top, jpeg_scan_counter_str, page_no, (MD_CHAR *)extension );
		}	
	}
	/* �t�@�C�����o��                                            */
	/* �o�̓t�@�C�����͎��̒ʂ�i�@�\�d�l�j                      */
	/*       hostname.[�g���q]                */
	/*       hostname_[�y�[�W�ԍ�].[�g���q]   */	
	else{
		if ( page_no == 1 ) {
			/*DO NOTHING*/
			/*      filename ==  hostname.[�g���q]                */
		}else if ( page_no > 1 ) {
			memset(  (MD_CHAR *)filename, NULL, SCAN2FTP_FNAME_MAXSIZE );
			memcpy ( (MD_CHAR *)filename, ftp_cifs_access_info.filename, SCAN2FTP_FNAME_MAXSIZE );
			p = strstr( (MD_CHAR *)filename, (MD_CHAR*)extension );
			if ( p==NULL ) {
				strncpy( (MD_CHAR *)filename_top, (MD_CHAR *)filename, SCAN2FTP_FNAME_MAXSIZE );
			}
			else {
				strncpy( (MD_CHAR *)filename_top, (MD_CHAR *)filename, (p - (MD_CHAR *)filename)-1 );
			}
			
			sprintf( (MD_CHAR *)filename, "%s_%d.%s",  (MD_CHAR *)filename_top, page_no, (MD_CHAR *)extension );
		}	
	}

	return;
}

/**
* @par	�ǎ�J�E���^�擾
* @param *scan_counter (output) �ǎ�J�E���^
* @return	�Ȃ�
*
* @par <�O���d�l>
*		JPEG�t�@�C�����Ɏg�p����ǎ�J�E���^�擾����B
* @par <�����d�l>
*		JPEG�t�@�C�����Ɏg�p����ǎ�J�E���^�𗼖ʓǂ̗L���𔻒f���擾����B
*
* @par <M�[>
*	   M-BCL-945
*/
GLOBAL INT32
scan_counter( UINT32 *scan_counter )
{
    /* FB���X�L���i                           */
#ifdef FB_SCAN_TYPE

    /* ���ʓǎ惂�f��                           */
#ifdef	USE_DUPLEX_SCAN

	/* ���ʓǎ�@�\�L��̃t�@���N�V�����X�C�b�` */
	if ( fstget_data(FSW_DUPLEX_SCAN_FUNC) == FSW_DUPLEX_SCAN_FUNC_ON ) {
        *scan_counter	= scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADFDX) +
                          scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADF  ) +
                          scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_FB   ) ;
	}
	/* ���ʓǎ�@�\�Ȃ�                         */
	else {
		*scan_counter	= scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADF  ) +
                          scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_FB   ) ;
	}
#else /* USE_DUPLEX_SCAN                        */

    /* �Жʓǎ惂�f��                           */
	*scan_counter      	= scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADF  ) +
                          scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_FB   ) ;
#endif /* USE_DUPLEX_SCAN                       */

#else /* FB_SCAN_TYPE                           */

    /* FB�Ȃ��X�L���i                           */
#ifdef	USE_DUPLEX_SCAN

    /* ���ʓǎ惂�f��                           */
	/* ���ʓǎ�@�\�L��̃t�@���N�V�����X�C�b�` */
	if ( fstget_data( FSW_DUPLEX_SCAN_FUNC ) == FSW_DUPLEX_SCAN_FUNC_ON ) {
        *scan_counter	= scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADFDX) +
                          scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADF  ) ;
	}
	/* ���ʓǎ�@�\�Ȃ�                         */
	else {
		*scan_counter	= scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADF);
	}
#else /* USE_DUPLEX_SCAN                        */
    /* �Жʓǎ惂�f��                           */
	*scan_counter       = scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADF);
#endif /* USE_DUPLEX_SCAN */
	
#endif /* FB_SCAN_TYPE */

	return ( OK );
}

/**
* @par	D�A�C�R���̕\��
* @return	�Ȃ�
*
* @par <�O���d�l>
*		�T�s���f���̂Ƃ�D�A�C�R����\������B
* @par <�����d�l>
*		�T�s���f���̂Ƃ�D�A�C�R����\������B
*
* @par <M�[>
*	   M-BCL-945
*/

#ifndef	USE_SEPARATE_UI
STATIC void
dispDIconFor5Line( void )
{
#ifdef LCD_5LINE
	if (( ftp_cifs_access_info.dualscan == FTP_DUPLEXLONG  ) ||
		( ftp_cifs_access_info.dualscan == FTP_DUPLEXSHORT ) ){
		ftpc_disp_icon( WRITE_OBJ );
	}
#endif	/* LCD_5LINE */
	return;
}
#endif	/* USE_SEPARATE_UI */

/**
* @par Scan Page ADF��Ԃ�
* @param *buff   (input) �i�[�o�b�t�@�ւ̃A�h���X
* @param mode    (input) Plain/XML
* @return �Ȃ�
*
* @par <�O���d�l>
*
*
* @par <�����d�l>
*
*
* @par <M�[>
*	   M-BCL-945
*/
STATIC UINT32 scan_cnt(INT32 kind)
{
    UINT32 adf_cnt;
    INT32  len = 0;
    UINT32 index;
	INT32  obj_knd;

    /* kind = MIB_VAL_BR_SCAN_COUNT_TYPE_ADF     */
    /*        MIB_VAL_BR_SCAN_COUNT_TYPE_ADFDX   */
    /*        MIB_VAL_BR_SCAN_COUNT_TYPE_FB      */

	/* get the first index of objacc(scan count) */
	if ( OK != objacc(OBJ_BR_SCAN_COUNT_INDEX, FSTINDEX, &index, 0, NULL, &len) )
	{
		return 0;
	}

	/* search for the index of the demanded object */
	while ( OK == objacc(OBJ_BR_SCAN_COUNT_TYPE, GETVAL, &index, 0, &obj_knd, &len) )
	{
		if ( obj_knd == kind )
		{
			break;
		}
		index++;
	}

    /* Page Count���擾                          */
	if ( OK != objacc(OBJ_BR_SCAN_COUNT_COUNTER, GETVAL ,
                      &index, 0, (INT32*)&adf_cnt , &len) ) {
        return 0;
	}

    return adf_cnt;
}

/**
* @par	�X�L���i���猴�e��ǎ��
* @param	from_task (input) ���b�Z�[�W���M���^�X�NID
* @param	cmd_id (input) ���b�Z�[�W���̃R�}���hID�i���b�Z�[�W���M���^�X�N�̏ꍇ�͑��u��ԁj
* @param	image_id (input/output) ��f�[�^ID
* @param	sysmem_id (input/output) �V�X�e��������ID
* @return	�Ȃ�
*
* @par <�O���d�l>
*		�X�L���i���猴�e��ǎ��A���f�A�I�����s���B
* @par <�����d�l>
*		�X�L���i���猴�e��ǎ�ׂ̑���y�ђ��f�A�I�����s���B
*
* @par <M�[>
*	   M-BCL-945
*/

STATIC void
scan_process( UINT16 from_task, UINT32 cmd_id, INT32 image_id, INT32 sysmem_id )
{
	switch ( from_task ) {

		/* ���u��ԃ^�X�N����       */
		case MACSTATUS_LIBRARY:
			scan_machstatus( cmd_id );
			break;

		/* �X�L�����x�[�X�^�X�N���� */
		case SCAN_BASE_TASK:
			scan_scanbase( cmd_id, image_id, sysmem_id );
			break;

		/* �p�l���x�[�X�^�X�N����   */
		case PANEL_BASE_TASK:
			scan_pnltask(  cmd_id, image_id, sysmem_id );
			break;

		/* NULL�x�[�X�^�X�N����   */
		case NULL_BASE_TASK:
#ifdef	FB_SCAN_TYPE
			msg_nulltask( cmd_id, image_id, sysmem_id  );
#else	/* FB_SCAN_TYPE */
			msg_nulltask( cmd_id );
#endif	/* FB_SCAN_TYPE */
			break;

		default:
			break;
	}
	return;
}


/**
* @par	�X�L�������̑��u��ԃ^�X�N����̃��b�Z�[�W����
* @param	cmd_id (input) ���u���
* @return	�Ȃ�
*
* @par <�O���d�l>
*		���u��Ԃ𔻕ʂ���LCD�\�����X�V����B
* @par <�����d�l>
*		���u��Ԃ𔻕ʂ���LCD�\����WarmUp����Scanning�X�V����B
*
* @par <M�[>
*	   M-BCL-945
*/
STATIC void
scan_machstatus( UINT32 cmd_id )
{
	return;
}

/**
* @par	�X�L��������Scan Base�^�X�N����̃��b�Z�[�W����
* @param	cmd_id (input) ���b�Z�[�W���̃R�}���hID
* @param	image_id (input) ��f�[�^ID
* @param	sysmem_id (input) �V�X�e��������ID
* @return	�Ȃ�
*
* @par <�O���d�l>
*		Scan Base�^�X�N����̃R�}���h�ɑΉ������ǎ�A���f�A�I�����s���B
* @par <�����d�l>
*		Scan Base�^�X�N����̃R�}���h�Ɠǎ摕�u�iADF/FB�j�ɑΉ������ǎ�A���f�A�I�����s���B
*
* @par <M�[>
*	   M-BCL-945
*/
STATIC void
scan_scanbase( UINT16 cmd_id, INT32 image_id, INT32 sysmem_id )
{
    /* memory full ���N�����Ă��邩�ǂ�����ێ�           */
    /* TRUE : MemoryFull                                  */
	STATIC BOOL memful_flag = FALSE;
#if	defined(USE_BSI) && defined(USE_SERIO_LOG)
	STATIC BOOL adf_scanstart_flag = FALSE;
#endif	/* defined(USE_BSI) && defined(USE_SERIO_LOG) */
	/* �X�L�������샂�[�h           */
#ifdef USE_SCAN_BLANK_DETECT
	UINT8                  apli_mode; 
#endif /* USE_SCAN_BLANK_DETECT */
	UINT32				   ScanPage;
#ifdef FTPC_COMPLETE_SEQUENCE
	UINT32				   BlankPage;
#endif /* FTPC_COMPLETE_SEQUENCE */

#ifdef	FB_SCAN_TYPE
	if ( scan_src == SCAN_SRC_ADF ) {
#endif
        /* ADF �œǂݎ��                                 */
		switch ( cmd_id ) {

			/* ����I������                               */
#ifdef	USE_SCAN_BLANK_DETECT
	/*** ������ǎ��A���A�㑱�y�[�W���Ȃ��ꍇ ***/
			case RT_MEM_SCAN_BLANK_PAGE_ALLEND:	/* fall through*/
				ftpclient_BlankPage(FTPC_PGCNT_INCREMENT, NULL);
#endif	/* USE_SCAN_BLANK_DETECT */
			case RT_MEM_SCAN_ALLEND:	/* fall through*/
				if( cmd_id == RT_MEM_SCAN_ALLEND )
				{
					ftpclient_ScanPage(FTPC_PGCNT_INCREMENT, NULL);
#if	defined(USE_BSI) && defined(USE_SERIO_LOG)
					/* BSI����User���O�p�J�E���g */
					if (Serio_Is_Enabled() == TRUE) {
						scan_bsiuserlog_pagecount( image_id, sysmem_id );
					}
#endif	/* USE_BSI && USE_SERIO_LOG */
				}
			case RT_MEM_SCAN_FFEND:
			case RT_MEM_SCAN_FFSTOP:
				send_scan_start_end( MEM_SCAN_END, NULL, image_id, sysmem_id );
				resource_release( sysmem_id );
                DPRINTF((" ftpc : Scan normal end @ftpclient_main\n"));
				/* �f�[�^���M�ֈڍs                       */
				ftpclient_ScanPage(FTPC_PGCNT_GET, &ScanPage);
#ifdef FTPC_COMPLETE_SEQUENCE
				ftpclient_BlankPage(FTPC_PGCNT_GET, &BlankPage);
				/* ������ʕ\�����s�������� */
				if( (ftp_cifs_access_info.scan_blank_detect == FTPNW_BLANK_DETECT_ON)
				 && (BlankPage > 0)
				 && (BlankPage != FTPC_PGCNT_INVALID_VAL) ){
					ftpscan_state   = FTPC_COMPLETE;
#ifdef USE_SEPARATE_UI
					cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_COMPLETE );
#endif /* USE_SEPARATE_UI */
					/* 60�b�^�C���A�E�g��ݒ肷�� */
					set_ftpc_CompleteStatus_end_time(COMPLETE_TIMEOUT);
					break;
				}
#endif /* FTPC_COMPLETE_SEQUENCE */
				if(ScanPage == 0 || ScanPage == FTPC_PGCNT_INVALID_VAL) {
					ImageDelete ( image_id );
					/* �X�e�[�^�X��ҋ@��� */
					ftpscan_state   = FTPC_START_WAIT;
#ifdef	USE_SEPARATE_UI
					cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif /* USE_SEPARATE_UI */
				}
				else {
					ftpscan_state   = FTPC_SEND;
				}
                break;
#ifdef FUNC_ADF_SCAN_SPEC_SHEETS_STOP
			case RT_MEM_SCAN_LIMITEND:
#ifndef	USE_SEPARATE_UI
                /* �p�l�����́E�\�����m��                 */
                ftpc_key_rightchg( GET_RIGHT );
#endif	/* USE_SEPARATE_UI */
                /* Reached ADFLimit�\��                   */
                DPRINTF((" ftpc : Scan Limit @ftpclient_main\n"));

#ifndef	USE_SEPARATE_UI
                /* ���ۉ���                             */
                buzzer_refusal();
                /* ��s���f��                             */
                /* �܍s���f���͂܂��Ȃ�                   */
#ifdef LCD_2LINE
                ftpc_disp_string( REACH_ADF_LIMIT_DSP, NULL );
                /* TX_YES_NO_DSP [1.Yes 2.No(Send)]       */
                ftpc_disp_string( SEND_DELETE_DSP, NULL );
#endif /* LCD_2LINE                                       */
#else	/* USE_SEPARATE_UI */
				cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_ADF_LIMIT );
#endif	/* USE_SEPARATE_UI */

				/* �A���ǎ搧����~���ֈڍs               */
				ftpscan_state = FTPC_SCAN_LIMIT_STOPPING;

				send_scan_start_end( MEM_SCAN_END,
				                     NULL, image_id, sysmem_id );
				break;
#endif /* FUNC_ADF_SCAN_SPEC_SHEETS_STOP                  */
			/* Memory Full�ُ픭��                        */
			case RT_MEM_SCAN_MEMORYFULL:
				if ( memful_flag == FALSE ) {
					scan_stop_memfull();
					memful_flag = TRUE;
				}
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
				/* �f�t�H���g���[�h���A�p�^�C�}�Z�b�g     */
				set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
				break;
			case	RT_MEM_SCAN_SCANMEMORYFULL:
				if(memful_flag == FALSE) {
					scan_stop_memfull();
					memful_flag = TRUE;
				}
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
				/* �f�t�H���g���[�h���A�p�^�C�}�Z�b�g */
				set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
				break;
			/* �X�L�������f/�X�L�����G���[����            */
			case RT_MEM_SCAN_STOP:
			case RT_MEM_SCAN_ERROR:
				if (( ftpscan_state == FTPC_SCANSTOP ) ||
					( memful_flag == TRUE )			   ){
					scan_stop_after( memful_flag );
				}
				ImageDelete ( image_id );
				send_scan_start_end( MEM_SCAN_END, NULL, image_id, sysmem_id );
				resource_release( sysmem_id );
#ifdef USE_BSI
				if (Serio_Is_Enabled() == TRUE) {
					if (memful_flag == TRUE){
						SendUiEnd_Restart(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL);
						ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
					}else if (ftpscan_state == FTPC_SCANSTOP && memful_flag == FALSE){
						/* STOP�L�[�ł̒�~ */
#ifndef	USE_SEPARATE_UI
						SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_STATE, SERIO_JOB_SCANSEND);
#endif /* USE_SEPARATE_UI */
						ScanEnd(SERIOFW_JOBSTS_END_CANCEL, SERIO_EXTERR_ABORTBYUSER);
					}else{
						/* ���s��~�ʒm(���u��ԃG���[) */
#ifndef	USE_SEPARATE_UI
						SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_STATE, SERIO_JOB_SCANSEND);
#endif /* USE_SEPARATE_UI */
						ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
					}
				}
#endif /* USE_BSI */
				memful_flag		= FALSE;
				/* �X�e�[�^�X��ҋ@��� */
				ftpscan_state	= FTPC_START_WAIT;
#ifdef	USE_SEPARATE_UI
				cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
				/* �f�t�H���g���[�h���A�p�^�C�}�Z�b�g     */
				set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
				break;

#ifdef USE_SCAN_BLANK_DETECT
		/*** ������ǎ��A���A�㑱�y�[�W������ꍇ ***/
			case RT_MEM_SCAN_BLANK_PAGE:
				ftpclient_BlankPage(FTPC_PGCNT_INCREMENT, NULL);
#ifdef	USE_SEPARATE_UI
				cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_BLANK_PAGE );	/* 1�������ʒm */
#endif /* USE_SEPARATE_UI */
				/* ���y�[�W�̓ǎ���J�n���� */
				switch(ftp_cifs_access_info.quality )
				{
					case	FTP_Color100:
					case	FTP_Color200:
					case	FTP_Color300:
					case	FTP_Color600:
					case	FTP_Gray100:
					case	FTP_Gray200:
					case	FTP_Gray300:
#if defined(USE_SCAN_AUTO_RESOLUTION)
					case	FTP_ColorAuto:
					case	FTP_GrayAuto:
#endif /* USE_SEPARATE_UI */
					case	FTP_Color400:
					case	FTP_Gray400:
					case	FTP_Gray600:
					case	FTP_Color150:
					case	FTP_Gray150:
#ifdef USE_SCAN_COLOR_DETECT
					case	FTP_Auto150:
					case	FTP_Auto200:
					case	FTP_Auto300:
					case	FTP_Auto600:
#endif /* USE_SCAN_COLOR_DETECT */
						apli_mode = MEM_SCAN_SCANNER;
						break;
					case	FTP_BW600:
					case	FTP_BW300:
					case	FTP_BW200:
					case	FTP_BW150:
					case	FTP_BW100:
					default:
						apli_mode = MEM_SCAN_FAX;
						break;
				}
				/* �X�L�����J�n�R�}���h���M                       */
				send_scan_start_end( MEM_SCAN_START, apli_mode, image_id, sysmem_id );
				break;
#endif	/* #ifdef USE_SCAN_BLANK_DETECT */

			case RT_MEM_SCAN_END:
				ftpclient_ScanPage(FTPC_PGCNT_INCREMENT, NULL);
#if	defined(USE_BSI) && defined(USE_SERIO_LOG)
				if (Serio_Is_Enabled() == TRUE) {
					if( adf_scanstart_flag == TRUE )
					{
						scan_bsiuserlog_pagecount( image_id, sysmem_id );
						adf_scanstart_flag = FALSE;
					}
				}
#endif	/* defined(USE_BSI) && defined(USE_SERIO_LOG) */
#ifdef	USE_SEPARATE_UI
				cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_END );	/* 1���ǎ�ʒm */
#endif /* USE_SEPARATE_UI */
				break;
			/* ���̑��̃R�}���h�͏������Ȃ�               */
			case RT_MEM_SCAN_START:
#if	defined(USE_BSI) && defined(USE_SERIO_LOG)
				if (Serio_Is_Enabled() == TRUE) {
					adf_scanstart_flag = TRUE;
				}
#endif	/* defined(USE_BSI) && defined(USE_SERIO_LOG) */
			default:
				break;
		}
#ifdef	FB_SCAN_TYPE
	}
	else {
        /* FB �œǂݎ��                                  */
		switch( cmd_id ) {
			
			/* ���y�[�W�Ǎ�                               */
			case RT_MEM_SCAN_END:
				/* �S�̕\���FNextPage?                    */
#if defined(USE_BSI) && defined(USE_SERIO_LOG)
				if (Serio_Is_Enabled() == TRUE) {
					if( scan_bsiuserlog_pagecount( image_id, sysmem_id ) != OK )
					{
						/* ������ */
						break;
					}
				}
#endif	/* USE_BSI && USE_SERIO_LOG */
#ifndef	USE_SEPARATE_UI
#ifdef LCD_5LINE /* �܍s���f�� */
				ftpc_disp_string_NULL( NULL_LINE2 | NULL_LINE3 | NULL_LINE4 | NULL_LINE5 );
				ftpc_disp_string( FLATBED_SCAN_DSP, NULL );
				ftpc_disp_string( NEXT_DOC_DSP, NULL );
				ftpc_disp_string_special( YES_DSP, NULL );
				ftpc_disp_string_special( NO_SEND_DSP, NULL );
				ftpc_disp_string( UP_DOWN_DSP, NULL );
				ftpc_disp_reverse( DISP_LINE3, 3, LCD_REVERSE_ON  );
#endif /* �܍s���f�� */

#ifdef LCD_2LINE /* ��s���f�� */
				ftpc_disp_string( NEXT_DOC_DSP, NULL );
                /* TX_YES_NO_DSP [1.Yes 2.No(Send)]       */
				ftpc_disp_string( TX_YES_NO_DSP, NULL );
#endif	/* ��s���f�� */
#else	/* USE_SEPARATE_UI */
				cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_NEXT );
#endif	/* USE_SEPARATE_UI */

				ftpscan_state		= FTPC_SELECTNEXT;
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
				if (Serio_Is_Enabled() == TRUE) {
					/* ���s��~�ʒm */
					SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_SCANNEXT, RESTART_NEXT_PAGE, SERIO_JOB_SCANSEND);
				}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
#ifndef	USE_SEPARATE_UI
                /* �^�C�}�[��~                           */
				dispcntl_keytimeout( APL_FTPCLIENT, PNL_TIMEOUT_STOP, 0 );
                /* �^�C�}�[�J�n        */
				dispcntl_keytimeout( APL_FTPCLIENT, PNL_TIMEOUT, 600 );
#endif	/* USE_SEPARATE_UI */
				break;

			/* ����I��/�X�L�������f  */
			case RT_MEM_SCAN_ALLEND:
			case RT_MEM_SCAN_STOP:
#ifdef FUNC_ADF_SCAN_SPEC_SHEETS_STOP
			case RT_MEM_SCAN_LIMITEND:
#endif /* FUNC_ADF_SCAN_SPEC_SHEETS_STOP                  */
				if (( ftpscan_state == FTPC_SCANSTOP ) ||
					( memful_flag == TRUE )			   ){
					scan_stop_after( memful_flag );
					ImageDelete ( image_id );
				}
				send_scan_start_end( MEM_SCAN_END, 0x00, image_id, sysmem_id );
				resource_release( sysmem_id );
				
				if (( ftpscan_state != FTPC_SCANSTOP ) &&
					( memful_flag != TRUE )			   ){

					/* ����I�����̓f�[�^���M�ֈڍs       */
					ftpscan_state   = FTPC_SEND;
				}
				else{
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
					if (Serio_Is_Enabled() == TRUE) {
						if (memful_flag == TRUE){
							SendUiEnd_Restart(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL);
							ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
						}else if (ftpscan_state == FTPC_SCANSTOP && memful_flag == FALSE){
							/* STOP�L�[�ł̒�~ */
#ifndef	USE_SEPARATE_UI
							SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_STATE, SERIO_JOB_SCANSEND);
#endif /* USE_SEPARATE_UI */
							ScanEnd(SERIOFW_JOBSTS_END_CANCEL, SERIO_EXTERR_ABORTBYUSER);
						}else{
							/* ���s��~�ʒm */
#ifndef	USE_SEPARATE_UI
							SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_STATE, SERIO_JOB_SCANSEND);
#endif /* USE_SEPARATE_UI */
							ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
						}
					}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
					/* �X�e�[�^�X��ҋ@��� */
					ftpscan_state   = FTPC_START_WAIT;
#ifdef	USE_SEPARATE_UI
					cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
					/* �f�t�H���g���[�h���A�p�^�C�}�Z�b�g */
					set_default_mode_ret( DM_SCAN_APL_END, DM_RET_ON );
#endif
#endif	/* USE_SEPARATE_UI */
				}
                /* Memory Full��Ԃ͏����l�ɖ߂�          */
				memful_flag	= FALSE;
				break;

			/* Memory Full�ُ픭��                        */
			case RT_MEM_SCAN_MEMORYFULL:
				if ( memful_flag == FALSE ) {
					scan_stop_memfull();
					memful_flag     = TRUE;
				}
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
				/* �f�t�H���g���[�h���A�p�^�C�}�Z�b�g     */
				set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
				break;				
			case	RT_MEM_SCAN_SCANMEMORYFULL:
				if(memful_flag == FALSE) {
					scan_stop_memfull();
					memful_flag = TRUE;
				}
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
				/* �f�t�H���g���[�h���A�p�^�C�}�Z�b�g */
				set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
				break;
			/* �X�L�����G���[����                         */
			case RT_MEM_SCAN_ERROR:
				buzzer_start( BUZZER_FINISH, APL_FTPCLIENT );
				if (( ftpscan_state == FTPC_SCANSTOP ) ||
					( memful_flag == TRUE )			   ){
					scan_stop_after( memful_flag );
				}
				ImageDelete ( image_id );
				send_scan_start_end( MEM_SCAN_END, 0x00, image_id, sysmem_id );
				resource_release( sysmem_id );
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
				if (Serio_Is_Enabled() == TRUE) {
					if (memful_flag == TRUE){
						SendUiEnd_Restart(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL);
						ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
					}else{
						/* ���s��~�ʒm(���u��ԃG���[) */
#ifndef	USE_SEPARATE_UI
						SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_STATE, SERIO_JOB_SCANSEND);
#endif /* USE_SEPARATE_UI */
						ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
					}
				}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
				memful_flag		= FALSE;
				/* �X�e�[�^�X��ҋ@��� */
				ftpscan_state	= FTPC_START_WAIT;
#ifdef	USE_SEPARATE_UI
				cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
				/* �f�t�H���g���[�h���A�p�^�C�}�Z�b�g     */
				set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
				break;

			/* ���̑��̃R�}���h�͏������Ȃ�               */
			case RT_MEM_SCAN_START:
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
				if (Serio_Is_Enabled() == TRUE) {
					/* ���s���ʒm */
					SendJobStatus(SERIOFW_JOBSTS_PROCESSING, SERIO_JOB_SCANSEND);
				}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
			case RT_MEM_SCAN_FFEND:
			case RT_MEM_SCAN_FFSTOP:
			default:
				break;
		}
	}
#endif		/* FB_SCAN_TYPE */
	return;
}


/**
* @par	�X�L��������Panel�^�X�N����̃��b�Z�[�W����
* @param	cmd_id (input) ���b�Z�[�W���̃R�}���hID
* @param	image_id (input) ��f�[�^ID
* @param	sysmem_id (input) �V�X�e��������ID
* @return	�Ȃ�
*
* @par <�O���d�l>
*		Panel�^�X�N����̃R�}���h�ɑΉ������ǎ�A���f�A�I�����s���B
* @par <�����d�l>
*		Panel�^�X�N����̃R�}���h�ɑΉ������ǎ�A���f�̕\���ƏI�����b�Z�[�W���M���s���B
*
* @par <M�[>
*	   M-BCL-945
*/
STATIC void
scan_pnltask( UINT16 cmd_id, INT32 image_id, INT32 sysmem_id )
{
	UINT8     apli_mode; /* �X�L�������샂�[�h  */
#ifndef	USE_SEPARATE_UI
#ifdef	LCD_5LINE
	pstring_t str_st;    /* �𑜓x�\���p�̍\����*/
#endif	/* LCD_5LINE */
#endif	/* USE_SEPARATE_UI */

	/* ������                                   */
	apli_mode = MEM_SCAN_FAX;

	/*------------------------------------------*/
	/* Scan�J�n��Command��M(���j���[����I��)  */
	/*------------------------------------------*/
	switch ( cmd_id ) {
		/* �X�L�������f�I���̃R�}���h��M       */
		case CMD_SCANSTOP:

#ifndef	USE_SEPARATE_UI
			/* StopKeyPressed �\��              */
			ftpc_disp_string( PRDSTP_DSP, NULL );

			/* ���ʓǎ�̏ꍇ�͂c�A�C�R����\�� */
			dispDIconFor5Line();
#else	/* USE_SEPARATE_UI */
			cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_STOP );
#endif	/* USE_SEPARATE_UI */

			/* �X�L�������f����                 */
			send_scan_stop();
#ifdef	FB_SCAN_TYPE
			if ( scan_src == SCAN_SRC_FB ) {
				send_scan_start_end( MEM_SCAN_END, NULL, image_id, sysmem_id );
			}
#endif
			ftpscan_state     = FTPC_SCANSTOP;
			break;

		/* ���y�[�W�X�L�����̃R�}���h��M       */
		case CMD_NEXTPAGE:

#ifndef	USE_SEPARATE_UI
			/*------- �S�̕\���FScanning -------*/
#ifdef	LCD_5LINE
			/* �t�@�C���t�H�[�}�b�g�p�̔C�ӕ�����̏�����             */
			memset( &str_st, NULL, sizeof(pstring_t) );
			/* �|��ꂩ��t�@�C���t�H�[�}�b�g�p�̔C�ӕ�����֕ϊ����� */
            lcd_strcpy(str_st.str_data, Quality_DSP);
			/* �\��                             */
            /* Scan to FTP                      */
            if (service_kind      == SCAN2FTP_SERVICE) {
                ftpc_disp_string( SCANTO_FTP_DSP    ,  NULL );
            }else if(service_kind == SCAN2CIFS_SERVICE){
                ftpc_disp_string( SCANTO_NETWORK_DSP,  NULL );
            }
            /* (Server��)                       */
			ftpc_disp_string( DUMMY_LINE2,    servername_zl   );
            /* (�𑜓x)                         */
			ftpc_disp_string( DUMMY_LINE3,    str_st.str_data );
            /* (�t�@�C����)                     */
			ftpc_disp_string( DUMMY_LINE4,    filename_zl     );
            /* Scanning                         */
			ftpc_disp_string( SCANNING_DSP,   NULL            );
#endif	/* LCD_5LINE */
#ifdef	LCD_2LINE
			/* �\��                             */
			ftpc_disp_string( SCANNING_DSP,   NULL );
			ftpc_disp_string( DUMMY_LINE2,    servername_zl   );
#endif	/* LCD_2LINE */
#else	/* USE_SEPARATE_UI */
			cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_SCANNING );
#endif	/* USE_SEPARATE_UI */

			switch ( ftp_cifs_access_info.quality ) {
#ifdef	COLOR_NETWORK_SCANNER
				case FTP_Color100:
				case FTP_Color150:
				case FTP_Color200:
				case FTP_Color300:
				case FTP_Color600:
				case FTP_Gray100:
				case FTP_Gray150:
				case FTP_Gray200:
				case FTP_Gray300:
				case FTP_Gray600:
#if defined(USE_SCAN_AUTO_RESOLUTION)
				case FTP_ColorAuto:
				case FTP_GrayAuto:
#endif
#ifdef USE_SCAN_COLOR_DETECT
				case	FTP_Auto150:
				case	FTP_Auto200:
				case	FTP_Auto300:
				case	FTP_Auto600:
#endif
					apli_mode = MEM_SCAN_SCANNER;
					break;
#endif	/* COLOR_NETWORK_SCANNER */
				case FTP_BW600:
				case FTP_BW300:
				case FTP_BW200:
				case FTP_BW100:
				default:
					apli_mode = MEM_SCAN_FAX;
					break;
			}
			send_scan_start_end( MEM_SCAN_START, apli_mode, image_id, sysmem_id );
			break;

		/* �X�L��������I���̃R�}���h��M       */
		case CMD_SCANEND:
			send_scan_stop();
            break;
#ifdef FUNC_ADF_SCAN_SPEC_SHEETS_STOP
        /* �X�L���������ɂ��"���M"�̃R�}���h��M */
        case CMD_SCAN_LIMIT_SND:
            /* FTP�N���C�A���g�̏�Ԃ�"���M"��    */
            ftpscan_state = FTPC_SEND;
            resource_release( sysmem_id );
            DPRINTF((" ftpc : CMD_SCAN_LIMIT_SND@ftpclient_main\n"));
			break;
        /* �X�L���������ɂ��"�폜"�̃R�}���h��M */
        case CMD_SCAN_LIMIT_DEL:
	        /* �A�v���\�������                   */
	        ftpc_disp_rightchg( FREE_RIGHT );
	        ImageDelete ( image_id );
            resource_release( sysmem_id );
#ifndef	USE_SEPARATE_UI
#ifdef USE_SELECTMODE_KEY
            /* Default Mode���A�p�^�C�}�Z�b�g     */
            set_default_mode_ret( DM_SCAN_APL_END, DM_RET_ON );
#endif
#endif	/* USE_SEPARATE_UI */
            /* FTP�N���C�A���g�̏�Ԃ�"�ҋ@"��    */
#ifdef	USE_SEPARATE_UI
			cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif	/* USE_SEPARATE_UI */
			/* �X�e�[�^�X��ҋ@��� */
            ftpscan_state = FTPC_START_WAIT;
            DPRINTF((" ftpc : CMD_SCAN_LIMIT_DEL@ftpclient_main\n"));
            break;
#endif  /* FUNC_ADF_SCAN_SPEC_SHEETS_STOP         */
	}

	return;
}

/**
* @par	    NULL-TASK����MSG���������̏���
* @param	cmd_id  (input) ���b�Z�[�W���M�R�}���h
* @param	image_id  (input) ��f�[�^ID
* @param	sysmem_id  (input) �V�X����ID
* @return	�Ȃ�
*
* @par <�O���d�l>
*		NULL-TASK����MSG���������̏���
* @par <�����d�l>
*
* @par <M�[>
*
* @par <�t�H�[�}�b�g�o�[�W����>
*      1.0.0
*/
#ifdef	FB_SCAN_TYPE
STATIC void msg_nulltask(UINT16 cmd_id, INT32 image_id, INT32 sysmem_id)
#else
STATIC void msg_nulltask(UINT16 cmd_id)
#endif
{
	DPRINTF(( "ftpclient.c : msg_nulltask, cmd_id = %d\n", cmd_id ));

	/*--------------------------------*/
	/* �X�L�������f�I���̃R�}���h��M */
	/*--------------------------------*/
	if(cmd_id == CMD_SCANSTOP){
#ifndef	USE_SEPARATE_UI
		/* StopKeyPressed �\�� */
		ftpc_disp_string( PRDSTP_DSP, NULL );
#else
		cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_STOP );
#endif

		/* �X�L�������f���� */
		send_scan_stop();

#ifdef	FB_SCAN_TYPE
		if(scan_src == SCAN_SRC_FB){
			send_scan_start_end( MEM_SCAN_END, NIL, image_id, sysmem_id );
		}
#endif
		ftpscan_state		= FTPC_SCANSTOP;
	}else{
		DPRINTF(( "ftpclient.c : msg_nulltask other cmd_id\n" ));
	}

	return;
}

/**
* @par	�X�L��������Scan Base�^�X�N�����Memoru Full���b�Z�[�W����
* @return	�Ȃ�
*
* @par <�O���d�l>
*		Scan Base�^�X�N�����Memoru Full�R�}���h�ɑΉ������������s���B
* @par <�����d�l>
*		Scan Base�^�X�N�����Memoru Full�R�}���h�ɑΉ������\���A���b�Z�[�W���M�����s���B
*
* @par <M�[>
*	   M-BCL-945
*/
STATIC void
scan_stop_memfull( void )
{
	buzzer_refusal();
#ifndef	USE_SEPARATE_UI
    /* memory full �\��                  */
	ftpc_disp_string( MEMFUL_DSP, NULL );

#ifdef	LCD_5LINE
	ftpc_disp_icon( ERASE_OBJ );
	ftpc_disp_line();
    /* Error�\�����͂R�`�T�s�ڂ��������� */
	ftpc_disp_string_NULL( NULL_LINE3 | NULL_LINE4 | NULL_LINE5 );
#else	/* LCD_5LINE */
    ftpc_disp_string( DUMMY_LINE2, NULL );	
#endif

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	if (Serio_Is_Enabled() == TRUE) {
		/* ���s��~�ʒm */
		/* �ĊJ�ʒm��RT_MEM_SCAN_STOP��M����scan_stop_after()�֐����ōs���Ă��� */
		SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL, SERIO_JOB_SCANSEND);
	}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

#ifdef	STATUS_LED
	/* memory full ���� LED ��ԐF�_��   */
	dispcntl_statusledcntl( APL_FTPCLIENT, PNL_LED_ON,  PNL_LED_RED );
	FOS_TASKSLEEP( 500 );
	dispcntl_statusledcntl( APL_FTPCLIENT, PNL_LED_OFF, PNL_LED_RED );
#elif	defined(USE_ERROR_LED)
	dispcntl_errorledcntl(APL_FTPCLIENT, PNL_LED_ON, PNL_LED_ERROR);
	FOS_TASKSLEEP(500);
	dispcntl_errorledcntl(APL_FTPCLIENT, PNL_LED_OFF, PNL_LED_ERROR);
#else	/* STATUS_LED */
	FOS_TASKSLEEP( 200 );
#endif	/* STATUS_LED */
#else	/* USE_SEPARATE_UI */
		cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_MEMFULL );
#endif	/* USE_SEPARATE_UI */


    /* �X�L�������f���b�Z�[�W���M        */
	send_scan_stop();

	ftpscan_state = FTPC_MEM_ERR;

	return;
}


/**
* @par	�X�L�������f�̌㏈��
* @param	memful (input) Memoru Full���
* @return	�Ȃ�
*
* @par <�O���d�l>
*		�X�L�������f���̃p�l���������s���B
* @par <�����d�l>
*		�X�L�������f���̃u�U�[�A�p�l���\���̏������s���B
*
* @par <M�[>
*	   M-BCL-945
*/
STATIC void
scan_stop_after( BOOL memful )
{
	/* �X�L�������f���Cmemory full ���͏I������炷 */
	buzzer_start( BUZZER_FINISH, APL_FTPCLIENT );

	if ( memful == TRUE ) {
#ifndef	USE_SEPARATE_UI
#ifdef	STATUS_LED
		/* memory full ���� LED ��ԐF�_��            */
		dispcntl_statusledcntl( APL_FTPCLIENT, PNL_LED_ON, PNL_LED_RED );
		FOS_TASKSLEEP( 500 );
		dispcntl_statusledcntl( APL_FTPCLIENT, PNL_LED_OFF, PNL_LED_RED );
#elif	defined(USE_ERROR_LED)
		dispcntl_errorledcntl(APL_FTPCLIENT, PNL_LED_ON, PNL_LED_ERROR);
		FOS_TASKSLEEP(500);
		dispcntl_errorledcntl(APL_FTPCLIENT, PNL_LED_OFF, PNL_LED_ERROR);
#else	/* STATUS_LED                                 */
		FOS_TASKSLEEP( 200 );
#endif	/* STATUS_LED                                 */
#endif	/* USE_SEPARATE_UI */
	}

	return;
}

/**
* @par	��`�l�̒u���������� FTP�N���C�A���g��Net�v���g�R��
* @param	file_format (input) FTP�N���C�A���g�ł̃t�@�C���t�H�[�}�b�g
* @return	�Ȃ�
*
* @par <�O���d�l>
*		FTP�N���C�A���g�ł̒�`����Net�v���g�R���̒�`�֕ϊ�����
* @par <�����d�l>
*
* @par <M�[>
*	   M-BCL-945
*/
STATIC INT32
get_filetype( INT32 file_format )
{
    INT32 file_type;

    switch (file_format){
        case FTP_PDF:
#ifdef USE_PDFA
        case FTP_PDFA:
#endif /* USE_PDFA */
#ifdef USE_SIGNEDPDF
        case FTP_SIPDF:
#endif /* USE_SIGNEDPDF */
            file_type = PROTOCOLLIB_FILETYPE_PDF;
            break;

        case FTP_JPEG:
            file_type = PROTOCOLLIB_FILETYPE_JPEG;
            break;
        case FTP_Tiff:
            file_type = PROTOCOLLIB_FILETYPE_TIFF;
            break;
        case FTP_XPS:
            file_type = PROTOCOLLIB_FILETYPE_XPS;
            break;
        case FTP_SPDF:
            file_type = PROTOCOLLIB_FILETYPE_SPDF;
            break;
        default:
            file_type = PROTOCOLLIB_FILETYPE_PDF;
            break;
    }
    return file_type;
}

/**
* @par	�V�X�����̗\����{(�g���C����)
* @param	quality      (input) �𑜓x
* @param	dup_scan     (input) duplex�X�L�������ۂ��H
* @param	scan_quality (input) ���k��
* @param	file_format  (input) �t�@�C���t�H�[�}�b�g
* @return	�\�񂳂ꂽ�V�X����ID
*
* @par <�O���d�l>
*		�𑜓x�Aduplex�X�L�����t���O�A�t�@�C���^�C�v����V�X�����̗\���ʂ𔻒�
*		����ɏ]���A�V�X������\�񂷂�B
* @par <�����d�l>
*
* @par <M�[>
*	   M-BCL-2941
*/
GLOBAL INT32
#if defined(USE_SCAN_AUTO_RESOLUTION)
get_sysmid_try( SCAN2FTP_QUALITY quality, BOOL dup_scan, UINT8 scan_quality, UINT32 file_format )
#else
get_sysmid_try( SCAN2FTP_QUALITY quality, BOOL dup_scan, UINT8 scan_quality )
#endif
{
    INT32  sysmem_id  ;
#if defined(USE_SCAN_AUTO_RESOLUTION)
#ifdef	COLOR_NETWORK_SCANNER
	UINT16	auto_resolution;
#endif
#endif
#ifdef COLOR_NETWORK_SCANNER
    /* �𑜓x/�t�@�C���T�C�Y���猈�܂�V�X�����\��g���C�񐔒�` */
    UINT8 sysm_try_tbl[][3] ={
                      /*           Large/Middle/Small */
        { 1, 1, 1 },  /* Color100    1      1     1   */
        { 1, 1, 1 },  /* Color200    1      1     1   */
        { 2, 1, 1 },  /* Color300    2      1     1   */
		{ 2, 1, 1 },  /* Color400    2      1     1   */
        { 3, 2, 1 },  /* Color600    3      2     1   */
        { 1, 1, 1 },  /* Gray 100    1      1     1   */
        { 1, 1, 1 },  /* Gray 200    1      1     1   */
        { 2, 1, 1 },  /* Gray 300    2      1     1   */
		{ 2, 1, 1 },  /* Gray 400    2      1     1   */
		{ 3, 1, 1 },  /* Gray 600    3      1     1   */
    };

    /* �V�X�����\�� �c�g���C�� */
    UINT8  try_num    ;
    UINT8  qual_index ;
    UINT8  scan_qual_index;
    /* �V�X�����\�� �A�v��ID     */
    UINT32 sysm_apl_id;

    /* �cTry�񐔂ɉ������V�X�����A�v��ID(Color) */
    UINT32 apl_id_color[] = { SYSM_AP_SCANTO_READ_COLOR_DUPLEX_ROTATE,
                              SYSM_AP_SCANTO_READ_COLOR_DUPLEX_ROTATE_MID,
                              SYSM_AP_SCANTO_READ_COLOR_DUPLEX_ROTATE_LARGE };

    /* �cTry�񐔂ɉ������V�X�����A�v��ID(Gray)  */
    UINT32 apl_id_gray [] = { SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE,
                              SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE_MID,
                              SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE_LARGE };

    try_num     = 0;
    sysm_apl_id = 0;

#if defined(USE_SCAN_AUTO_RESOLUTION)
	/* �ǎ掩���𑜓x�ݒ�̏ꍇ�A�𑜓x���擾���� */
    switch ( quality ) {
#ifdef COLOR_NETWORK_SCANNER
		case FTP_ColorAuto:
		case FTP_GrayAuto:
			if(scanmenu_EiScanGetResolution(SCAN_APP_GET_SYSMID_TRY, quality, file_format, scan_quality, &auto_resolution) != OK)
			{
				EPRINTF(("%s(%d)	scanmenu_EiScanGetResolution ERROR!!\n",__FILE__,__LINE__));
			}
			quality = (SCAN2FTP_QUALITY)auto_resolution;	/* �����擾�����𑜓x���Z�b�g���� */
			break;
#endif /* COLOR_NETWORK_SCANNER */
		default:
			/* �ݒ�l�̂܂� */
			break;
	}
#endif	/* USE_SCAN_AUTO_RESOLUTION */

    /* �𑜓x�ɉ�����index���擾����            */
    qual_index      = get_qual_index     ( quality );
    /* �t�@�C���T�C�Y�ɉ�����index���擾����    */
    scan_qual_index = get_scan_qual_index( scan_quality );
#endif /* COLOR_NETWORK_SCANNE */

    sysmem_id   = ERROR;

    switch ( quality ) {
#ifdef COLOR_NETWORK_SCANNER
        case FTP_Color100:
        case FTP_Color200:
        case FTP_Color300:
        case FTP_Color400:
        case FTP_Color600:
            if ( dup_scan == TRUE ) {
                /* �𑜓x�A�t�@�C���T�C�Y����g���C�񐔂��擾���� */
                try_num = sysm_try_tbl[qual_index][scan_qual_index];
                DPRINTF((" ftpc : try_num(CL) [%d]@get_sysmid_try\n", try_num));
                /* Try�񐔂ɉ������V�X�����\��ID���擾            */ 
                while( (sysmem_id == ERROR) && (try_num > 0) ){
                    sysm_apl_id = apl_id_color[try_num - 1];
                    sysmem_id   = sysmApliRsv( sysm_apl_id );
                    DPRINTF((" ftpc : try_num(CL) [%d]@Loop get_sysmid_try\n", try_num));
                    try_num--;
                } 
            }
            else {
                sysmem_id = sysmApliRsv( SYSM_AP_SCANTO_READ_COLOR );
                DPRINTF((" ftpc : sysmem_aplid  [%d]@get_sysmid_try\n", SYSM_AP_SCANTO_READ_COLOR));
            }
            DPRINTF((" ftpc : sysmem_aplid  [%d]@get_sysmid_try\n", sysm_apl_id));
            break;
        case FTP_Gray100:
        case FTP_Gray200:
        case FTP_Gray300:
        case FTP_Gray400:
        case FTP_Gray600:
            if ( dup_scan == TRUE ) {
                /* �𑜓x�A�t�@�C���T�C�Y����g���C�񐔂��擾���� */
                try_num = sysm_try_tbl[qual_index][scan_qual_index];
                DPRINTF((" ftpc : try_num Gray [%d]@get_sysmid_try\n", try_num));
                /* Try�񐔂ɉ������V�X�����\��ID���擾            */ 
                while( (sysmem_id == ERROR) && (try_num > 0) ){
                    sysm_apl_id = apl_id_gray[try_num - 1];
                    sysmem_id   = sysmApliRsv( sysm_apl_id );
                    DPRINTF((" ftpc : try_num Gray [%d]@Loop get_sysmid_try\n", try_num));
                    try_num--;
                } 
            }
            else {
                sysmem_id = sysmApliRsv( SYSM_AP_SCANTO_READ_GRAY );
                DPRINTF((" ftpc : sysmem_aplid  [%d]@get_sysmid_try\n", SYSM_AP_SCANTO_READ_GRAY));
            }
            DPRINTF((" ftpc : sysmem_id  [%d]@get_sysmid_try\n", sysm_apl_id));
            break;
#endif /* COLOR_NETWORK_SCANNER */
		case FTP_BW600:
        case FTP_BW300:
        case FTP_BW200:
        case FTP_BW100:
        default:
            if ( dup_scan == TRUE ) {
                sysmem_id = sysmApliRsv( SYSM_AP_SCANTO_READ_MONO_DUPLEX_ROTATE );
            }
            else {
                sysmem_id = sysmApliRsv( SYSM_AP_SCANTO_READ_MONO );
            }
            break;
    }
    return sysmem_id;
}

#ifdef COLOR_NETWORK_SCANNER
/**
* @par	    �e�[�u��sysm_try_tbl���index��Ԃ�
* @param	quality   (input) �𑜓x
* @return	�e�[�u��sysm_try_tbl���index
*
* @par <�O���d�l>
*		�𑜓x����e�[�u��sysm_try_tbl���index��Ԃ�
* @par <�����d�l>
*
* @par <M�[>
*	   M-BCL-2941
*/
STATIC  UINT8
get_qual_index( SCAN2FTP_QUALITY quality )
{
    UINT8 index;

    index = 0;

    switch( quality ){
       case FTP_Color100:
            index = 0;
            break;
        case FTP_Color200:
            index = 1;
            break;
        case FTP_Color300:
            index = 2;
            break;
        case FTP_Color400:
            index = 3;
            break;
        case FTP_Color600:
            index = 4;
            break;
        case FTP_Gray100:
            index = 5;
            break;
        case FTP_Gray200:
            index = 6;
            break;
        case FTP_Gray300:
            index = 7;
            break;
        case FTP_Gray400:
            index = 8;
            break;
        case FTP_Gray600:
            index = 9;
            break;
    }
    return index;
}

/**
* @par	    �e�[�u��sysm_try_tbl���index��Ԃ�
* @param	scan_quality  (input) �t�@�C�����k��
* @return	�e�[�u��sysm_try_tbl���index
*
* @par <�O���d�l>
*		�t�@�C�����k������e�[�u��sysm_try_tbl���index��Ԃ�
* @par <�����d�l>
*
* @par <M�[>
*	   M-BCL-2941
*/
STATIC  UINT8
get_scan_qual_index( UINT8 scan_quality )
{
    UINT8 index;

    typedef enum{
	    LARGE_IDX,
	    MIDLE_IDX,
	    SMALL_IDX,
    } enum_file_size_idx;

    index = SMALL_IDX;
    switch( scan_quality ){
        /* �t�@�C���T�C�Y Large (���掿)  */
        case P_SCAN_QUAL_HIGH:
            index = LARGE_IDX;
            break;
        /* �t�@�C���T�C�Y Middle(���掿)  */
        case P_SCAN_QUAL_MID:
            index = MIDLE_IDX;
            break;
        /* �t�@�C���T�C�Y Small (��掿)  */
        case P_SCAN_QUAL_NORMAL:
        default:
            index = SMALL_IDX;
            break;
    }
    return index;
}
#endif /* COLOR_NETWORK_SCANNER */


#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
STATIC	VOID
ScanEnd(INT32 Reason, INT32 ErrorInfo)
{
	if (g_SubjectJobStop) {
		g_SubjectJobStop->vptr_Detach(g_SubjectJobStop, g_ObserverStop);
	}
	
	if(Reason != SERIOFW_JOBSTS_END_SYSBUSY)
	{
		Serio_Chk_Scan_Resource_Free();
	}

	SendJobStatus_End(Reason, ErrorInfo, SERIO_JOB_SCANSEND);
}

/**
* @par	Panel Menu�̊m��l(FTP/CIFS)�̉𑜓x���o�͂���B
* @param	�Ȃ�
* @return	Panel Menu�̊m��l(FTP/CIFS)�̉𑜓x
*
* @par <�O���d�l>
*		Panel Menu�̊m��l(FTP/CIFS)�̉𑜓x���o�͂���B
* @par <�����d�l>
*		����B
*
*/
GLOBAL SCAN2FTP_QUALITY
get_ftp_cifs_access_info_quality(VOID)
{
	return ftp_cifs_access_info.quality;
}

#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

#if	defined(USE_BSI) && defined(USE_SERIO_LOG)
/**
* @par	BSI����User��SCAN2FTP/SCAN2CIFS�̗��p����ۑ��֐�
* @param	�Ȃ�
* @return	�Ȃ�
*
* @par <�O���d�l>
*		BSI����User��SCAN2FTP/SCAN2CIFS�̗��p������ۑ�����B
* @par <�����d�l>
*		����B
*
*/
STATIC	INT32	
set_scan_ftp_cifs_setting_bsilog( void )
{
	SERIOLOG_SCAN_SETTING_T		scan_set_log;
	
	/* PaperSize */
#ifndef USE_TMP_SCANSIZE
	switch(fstget_data( FUNC_SCANSIZE )){
#else	/* USE_TMP_SCANSIZE */
	switch(ftp_cifs_access_info.scan_doc_size){
#endif	/* USE_TMP_SCANSIZE */
		case SCANSIZE_LETTER:
			scan_set_log.PaperSize = LOG_DATA_SCAN_DOCSIZE_LETTER;
			break;
		case SCANSIZE_LEGAL:
			scan_set_log.PaperSize = LOG_DATA_SCAN_DOCSIZE_LEGAL;
			break;
		case SCANSIZE_A4:
		default:
			scan_set_log.PaperSize = LOG_DATA_SCAN_DOCSIZE_A4;
			break;
	}
	
	/* Colot�ݒ� */
	/* Resolution�ݒ� */
	switch(ftp_cifs_access_info.quality){
		case FTP_Color100:
			DPRINTF(("[%s]case FTP_Color100:\n", __FUNCTION__));
			scan_set_log.Color      = LOG_DATA_SCAN_COLOR;
			scan_set_log.Resolution = LOG_DATA_RESO_100;
			break;
		case FTP_Color200:
			DPRINTF(("[%s]case FTP_Color200:\n", __FUNCTION__));
			scan_set_log.Color      = LOG_DATA_SCAN_COLOR;
			scan_set_log.Resolution = LOG_DATA_RESO_200;
			break;
		case FTP_Color300:
			DPRINTF(("[%s]case FTP_Color300:\n", __FUNCTION__));
			scan_set_log.Color      = LOG_DATA_SCAN_COLOR;
			scan_set_log.Resolution = LOG_DATA_RESO_300;
			break;
		case FTP_Color600:
			DPRINTF(("[%s]case FTP_Color600:\n", __FUNCTION__));
			scan_set_log.Color      = LOG_DATA_SCAN_COLOR;
			scan_set_log.Resolution = LOG_DATA_RESO_600;
			break;
#if defined(USE_SCAN_AUTO_RESOLUTION)
		case FTP_ColorAuto:
			DPRINTF(("[%s]case FTP_ColorAuto:\n", __FUNCTION__));
			scan_set_log.Color      = LOG_DATA_SCAN_COLOR;
			scan_set_log.Resolution = LOG_DATA_RESO_300;
			break;
#endif	/* USE_SCAN_AUTO_RESOLUTION */
		case FTP_Gray100:
			DPRINTF(("[%s]case FTP_Gray100:\n", __FUNCTION__));
			scan_set_log.Color      = LOG_DATA_SCAN_GRAY;
			scan_set_log.Resolution = LOG_DATA_RESO_100;
			break;
		case FTP_Gray200:
			DPRINTF(("[%s]case FTP_Gray200:\n", __FUNCTION__));
			scan_set_log.Color      = LOG_DATA_SCAN_GRAY;
			scan_set_log.Resolution = LOG_DATA_RESO_200;
			break;
		case FTP_Gray300:
			DPRINTF(("[%s]case FTP_Gray300:\n", __FUNCTION__));
			scan_set_log.Color      = LOG_DATA_SCAN_GRAY;
			scan_set_log.Resolution = LOG_DATA_RESO_300;
			break;
#if defined(USE_SCAN_AUTO_RESOLUTION)
		case FTP_GrayAuto:
			DPRINTF(("[%s]case FTP_GrayAuto:\n", __FUNCTION__));
			scan_set_log.Color      = LOG_DATA_SCAN_GRAY;
			scan_set_log.Resolution = LOG_DATA_RESO_200;
			break;
#endif	/* USE_SCAN_AUTO_RESOLUTION */
		case FTP_BW300:
			DPRINTF(("[%s]case FTP_BW300:\n", __FUNCTION__));
			scan_set_log.Color      = LOG_DATA_SCAN_B_W;
			scan_set_log.Resolution = LOG_DATA_RESO_300;
			break;
		case FTP_BW200:
			DPRINTF(("[%s]case FTP_BW200:\n", __FUNCTION__));
			scan_set_log.Color      = LOG_DATA_SCAN_B_W;
			scan_set_log.Resolution = LOG_DATA_RESO_200;
			break;
		case FTP_BW100:
			DPRINTF(("[%s]case FTP_BW100:\n", __FUNCTION__));
			scan_set_log.Color      = LOG_DATA_SCAN_B_W;
			scan_set_log.Resolution = LOG_DATA_RESO_200_100;
			break;
		default:
			DPRINTF(("[%s]default:\n", __FUNCTION__));
			break;
	}
	
	/* Duplex�ݒ� */
	switch(ftp_cifs_access_info.dualscan){
		case FTP_DUPLEXLONG:
		case FTP_DUPLEXSHORT:
			scan_set_log.Duplex = LOG_DATA_DUPLEX_ENABLE;
			break;
		default:
			scan_set_log.Duplex = LOG_DATA_DUPLEX_DISABLE;
			break;
	}
	
	/* Scan��� */
	switch(service_kind){
		case SCAN2FTP_SERVICE:
			DPRINTF(("[%s]case SCAN2FTP_SERVICE:\n", __FUNCTION__));
			scan_set_log.ScanTo = LOG_DATA_SCANTO_FTP;
			break;
		case SCAN2CIFS_SERVICE:
			DPRINTF(("[%s]case SCAN2CIFS_SERVICE:\n", __FUNCTION__));
			scan_set_log.ScanTo = LOG_DATA_SCANTO_CIFS;
			break;
#ifdef	USE_SCAN2SFTP
		case SCAN2SFTP_SERVICE:
			scan_set_log.ScanTo = LOG_DATA_SCANTO_SFTP;
			break;
#endif	/* USE_SCAN2SFTP */
		default:
			DPRINTF(("[%s]default:\n", __FUNCTION__));
			scan_set_log.ScanTo = LOG_DATA_SCANTO_UNKNOWN_SERVICE;
			break;
	}
	
	/* �ȉ��ARAM�L���p�̃f�[�^�Q */
	/* Server Address */
	switch(scan_set_log.ScanTo){
		case LOG_DATA_SCANTO_FTP:
		case LOG_DATA_SCANTO_CIFS:
#ifdef	USE_SCAN2SFTP
		case LOG_DATA_SCANTO_SFTP:
#endif	/* USE_SCAN2SFTP */
			scan_set_log.str_data = ftp_cifs_access_info.serveraddress;
			break;
		default:
			break;
	}
	
	/* File Type */
	switch(ftp_cifs_access_info.fileformat){
		case FTP_PDF:
			scan_set_log.filetype = LOG_DATA_FLTYPE_PDF;
			break;
#ifdef USE_PDFA
		case FTP_PDFA:
			scan_set_log.filetype = LOG_DATA_FLTYPE_PDFA;
			break;
#endif /* USE_PDFA */
#ifdef USE_SIGNEDPDF
		case FTP_SIPDF:
			scan_set_log.filetype = LOG_DATA_FLTYPE_SIPDF;
			break;
#endif /* USE_SIGNEDPDF */
		case FTP_SPDF:
			scan_set_log.filetype = LOG_DATA_FLTYPE_SPDF;
			break;
		case FTP_JPEG:
			scan_set_log.filetype = LOG_DATA_FLTYPE_JPEG;
			break;
		case FTP_Tiff:
			scan_set_log.filetype = LOG_DATA_FLTYPE_TIFF;
			break;
		case FTP_XPS:
			scan_set_log.filetype = LOG_DATA_FLTYPE_XPS;
			break;
		default:
			break;
	}
	
	/* Scan�n�ݒ��Log�ۑ� */
	SerioLog_Rec_Start(LOG_DATA_FUNC_SCAN, &scan_set_log);
	
	/* Log�ۑ��J�n����Scan����Count�p�ϐ��������� */
	/* �K����L���O�ۑ��J�n�֐�������ɍs����     */
	if(ftp_cifs_access_info.fileformat != FTP_JPEG)
	{
		SerioLog_Start_ScanPages_LocalCount();
	}
	
	return OK;
}


STATIC INT32 
scan_bsiuserlog_pagecount( INT32 image_id, INT32 sysmem_id )
{
	INT32 ret = OK;
	
	if(ftp_cifs_access_info.fileformat != FTP_JPEG)
	{
		if( SerioLog_Countup_ScanPages_LocalCount() != OK )
		{
			ImageDelete ( image_id );
			send_scan_start_end( MEM_SCAN_END, NULL, image_id, sysmem_id );
			resource_release( sysmem_id );
#ifdef	USE_SEPARATE_UI
			cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_SENDERROR );
#else	/* USE_SEPARATE_UI */
			ftpc_disp_string( MAIL_SENDERR_DSP, NULL );
#endif	/* USE_SEPARATE_UI */
			/* ���s��~�ʒm(���u��ԃG���[) */
#ifndef	USE_SEPARATE_UI
			SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_STATE, SERIO_JOB_SCANSEND);
#endif /* USE_SEPARATE_UI */
			ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
			
#ifdef	USE_SEPARATE_UI
			cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif
			/* �X�L�������f���b�Z�[�W���M        */
			send_scan_stop();
			
			/* FTP�̏�Ԃ�Ready�ɖ߂� */
			ftpscan_state	= FTPC_START_WAIT;
			
			/* 1JOB�������SCAN�����������������Ăяo�����ɓ`����B */
			ret = NG;
		}
	}
	
	return ret;
}
#endif	/* USE_BSI && USE_SERIO_LOG */

#ifndef	USE_SEPARATE_UI
#ifdef LCD_5LINE
/*****************************************************************************
* @par �J�[�\���폜����
*
* @par <�O���d�l>
*			�J�[�\���폜����
* @par <�����d�l>
*			1�`4�s�ڂ̃J�[�\�����폜����
******************************************************************************/
STATIC	VOID
scan_clear_cursor(void)
{
	UINT8	i;

	for(i = DISP_LINE1; i < DISP_LINE5; i++){
		ftpc_disp_reverse( i, 3, LCD_REVERSE_OFF );
	}
}
#endif	/* LCD_5LINE */
#endif	/* USE_SEPARATE_UI */


/****************************************************************************
* @par		Scan to FTP,NW,SFTP�ڑ�Check���ʐݒ�(local)
* @param	SCAN_CHECK_RESULT rslt�FScan to FTP,NW,SFTP�ݒ�Check�������ʕێ��ϐ�
* @return	����
*
* @par <�O���d�l>
*		Check�������ʂ�ݒ肷��B
* @par <�����d�l>
*		�����̒l�𔻒肵�A����l�Ȃ猋�ʗp�ϐ�(STATIC)�Ɋi�[����B
* @par <M�[>
*		M-DCSL-867
*****************************************************************************/
STATIC VOID
scan_check_setresult(const SCAN_CHECK_RESULT rslt)
{
	INT32	val_min;
	INT32	val_max;
	INT32	val;
	
	val_min = SCAN_CHECK_INIT;
	val_max = SCAN_CHECK_MAX;
	val = rslt;
	
	if((val_min <= val) && (val_max > val)) {
		scan_check_result = rslt;
		scan_check_result_time = FOS_GETTICK();
		DPRINTF(("[%s] scan_check_result=%d, scan_check_result_time=%d\n", __FUNCTION__, scan_check_result, scan_check_result_time));
	} else {
		DPRINTF(("[%s](%d) Scan to SettingCheck Err\n", __FUNCTION__, __LINE__));
	}
	return;
}
/****************************************************************************
* @par		Scan to FTP,NW,SFTP�ڑ�Check���ʎ擾(local)
* @param	����
* @return	SCAN_CHECK_RESULT�FScan to FTP,NW,SFTP�ݒ�Check�������ʕێ��ϐ�
*
* @par <�O���d�l>
*		�O������J
* @par <�����d�l>
*		Scan to FTP,NW,SFTP�ݒ�Check�������ʎ擾�p�Alocal�֐�
*		ScanFTP_Check_GetResult�AScanNW_Check_GetResult�A
*		ScanSFTP_Check_GetResult�ȊO�����Call�֎~
* @par <M�[>
*		M-DCSL-867
*****************************************************************************/
STATIC SCAN_CHECK_RESULT
scan_check_getresult(void)
{
	SCAN_CHECK_RESULT loc_scan_check_result = SCAN_CHECK_BUSY;	/* ���ʑ��M�p */
	
	if((SCAN_CHECK_INIT != scan_check_result)&&(SCAN_CHECK_BUSY != scan_check_result)){
		/* ���ʑ��M�p�ɋl�߂Ȃ��� */
		loc_scan_check_result = scan_check_result;
		/* �ݒ�Check�������ʂɁu������ԁv��ݒ� */
		scan_check_setresult(SCAN_CHECK_INIT);
	}
	if(SCAN_CHECK_BUSY == scan_check_result){
		if(SCAN_CHECK_BUSSY_RESET_TIME < FOS_GETTICK() - scan_check_result_time){
			/* �m�F����40�b�ȏ�p�����Ă���ꍇ */
			loc_scan_check_result = SCAN_CHECK_ERR_BUSY;
			scan_check_setresult(SCAN_CHECK_INIT);
		}
	}
	return loc_scan_check_result;
}
/****************************************************************************
* @par		Scan to FTP,NW,SFTP�ڑ�Check�J�n�v��(local)
* @param	UINT8 profilenumber(input) �ڑ���profile number
* @param	UINT16 cmd_id(input) ���b�Z�[�W���̃R�}���hID�i���b�Z�[�W���M���^�X�N�̏ꍇ�͑��u��ԁj
* @return	TRUE�F����I��
*			FALSE�F�ُ�I��(����)
*
* @par <�O���d�l>
*		�O������J
* @par <�����d�l>
*		Scan to FTP,NW,SFTP�ڑ�Check�J�n�v���ʒm�p�Alocal�֐�
*		ScanFTP_Check_Start�AScanNW_Check_Start�A
*		ScanSFTP_Check_Start�ȊO�����Call�֎~
* @par <M�[>
*		M-DCSL-867
*****************************************************************************/
STATIC BOOL
scan_check_start(UINT8 profilenumber,UINT16 cmd_id)
{
	ftpclt_msg	message;		/* �X�L�����p�����[�^ */
	UINT32		qid_FTPC;	/* ���b�Z�[�WID */
	UINT32		msgsend_ret;
	
	/* ����Check */
	if((CMD_SCAN_TO_FTPTEST_STA != cmd_id)
		&&(CMD_SCAN_TO_NWTEST_STA != cmd_id)
#ifdef	USE_SCAN2SFTP
		&&(CMD_SCAN_TO_SFTPTEST_STA != cmd_id)
#endif	/* USE_SCAN2SFTP */
	){
		/* cmd_id���s���̏ꍇ */
		return FALSE;
	}
	
	/* ���Check */
	if((SCAN_CHECK_INIT != scan_check_result)&&(SCAN_CHECK_BUSY != scan_check_result)){
		if(SCAN_CHECK_RESULT_RESET_TIME < FOS_GETTICK() - scan_check_result_time){
			/* �������݊�����10�b�ȏ�o�߂��Ă���ꍇ */
			scan_check_setresult(SCAN_CHECK_INIT);
		}
	}
	if(SCAN_CHECK_BUSY == scan_check_result){
		if(SCAN_CHECK_BUSSY_RESET_TIME < FOS_GETTICK() - scan_check_result_time){
			/* �m�F����40�b�ȏ�p�����Ă���ꍇ */
			scan_check_setresult(SCAN_CHECK_INIT);
		}
	}
	if(SCAN_CHECK_INIT != scan_check_result){
		/* ������Ԃł͖����ꍇ */
		return FALSE;
	}
	
	/* �ݒ�Check�������ʂɁu�������ݒ��v��ݒ� */
	scan_check_setresult(SCAN_CHECK_BUSY);
	
	/* ���b�Z�[�WID�擾 */
	qid_FTPC = FOS_MSGGETID(FTPC_MSG_NAME);
	
	/* �X�L�����p�����[�^�ݒ� */
	memset(&message, NULL, sizeof(ftpclt_msg));
	message.from_task = FTPC_APL_TASK;		/* ���b�Z�[�W���M���^�X�NID */
	message.cmd_id = cmd_id;				/* ���b�Z�[�W���̃R�}���hID */
	message.profilenumber = profilenumber;	/* �ڑ���profile number */
	
	/* ���b�Z�[�W���M(�ݒ�Check) */
	msgsend_ret = FOS_MSGSEND(qid_FTPC,(UINT8 *)&message,sizeof(ftpclt_msg));
	if(OK != msgsend_ret){
		scan_check_setresult(SCAN_CHECK_INIT);
		DPRINTF(("[%s](%d) Scan to SettingCheck Err\n", __FUNCTION__, __LINE__));
		return FALSE;
	}
	return TRUE;
}
/****************************************************************************
* @par		Scan to FTP�ڑ����ݒ�
* @param	UINT8 profilenumber(input) �ڑ���profile number
* @param	stcFTPConnect* ftp_conn(input/output)
* @return	����
*
* @par <�O���d�l>
*		�O������J
* @par <�����d�l>
*		Scan to FTP�ڑ�����ݒ肷��B
* @par <M�[>
*		M-DCSL-867
*****************************************************************************/
STATIC void
scanftp_connection_setup(UINT8 profilenumber,stcFTPConnect* ftp_conn)
{
	/* Port�ԍ� */
	UINT8	ftp_port_num[5+1];	/* Port�ԍ� */
	memset(ftp_port_num, NULL, sizeof(ftp_port_num));
	
	/* FTP�z�X�g�ڑ����ݒ� */
	FUNC_GET_INDEXSTR(OBJ_SCAN2FTP_SVRADR	,profilenumber	,ftp_conn->HostAddress		,SCAN2FTP_SVRADR_MAXLEN	);	/* FTP�z�X�g���܂���IP�A�h���X */
	FUNC_GET_INDEXSTR(OBJ_SCAN2FTP_STDIR	,profilenumber	,ftp_conn->StoreDirectory	,SCAN2FTP_STDIR_MAXLEN	);	/* �t�@�C���ۑ���t�H���_ */
	FUNC_GET_INDEXVAL(OBJ_SCAN2FTP_PAMODE	,profilenumber	,&(ftp_conn->IsPassive)		);							/* �p�b�V�u���[�h or �A�N�e�B�u���[�h */
	FUNC_GET_INDEXSTR(OBJ_SCAN2FTP_PORT		,profilenumber	,ftp_port_num				,sizeof(ftp_port_num)	);	/* Port�ԍ� */
	ftp_conn->PortNumber = atoi((const char *)ftp_port_num);
	FUNC_GET_INDEXSTR(OBJ_SCAN2FTP_USER		,profilenumber	,ftp_conn->UserName			,SCAN2FTP_USER_MAXLEN	);	/* �F�؂̂��߂̃��[�U�[�� */
	FUNC_GET_INDEXSTR(OBJ_SCAN2FTP_PASSWD	,profilenumber	,ftp_conn->Password			,SCAN2FTP_PASSWD_MAXLEN	);	/* �F�؂̂��߂̃p�X���[�h */
	
	DPRINTF(("[%s] HostAddress=%s, StoreDirectory=%s, IsPassive=%d, PortNumber=%d, UserName=%s, Password=%s\n",
				__FUNCTION__, ftp_conn->HostAddress, ftp_conn->StoreDirectory, ftp_conn->IsPassive, ftp_conn->PortNumber, ftp_conn->UserName, ftp_conn->Password));
	return;
}
/****************************************************************************
* @par		Scan to FTP�̐ڑ��m�F
* @param	UINT8 profilenumber(input) �ڑ���profile number
* @return	����
*
* @par <�O���d�l>
*		�O������J
* @par <�����d�l>
*		�ڑ����̎擾�A�ڑ��m�F�A���ʂ̐ݒ���s���B
* @par <M�[>
*		M-DCSL-867
*****************************************************************************/
STATIC	void
scanftp_connection_check(UINT8 profilenumber)
{
	INT32	ret_value_ftp = FTPCLIB_ERROR_EXEC;	/* �ڑ����� */
	/* FTP�ڑ���� */
	stcFTPConnect	ftp_conn;
	UINT8	buf_HostAddress[SCAN2FTP_SVRADR_MAXLEN+1];
	UINT8	buf_StoreDirectory[SCAN2FTP_STDIR_MAXLEN+1];
	UINT8	buf_UserName[SCAN2FTP_USER_MAXLEN+1];
	UINT8	buf_Password[SCAN2FTP_PASSWD_MAXLEN+1];
	memset(buf_HostAddress		,NULL	,SCAN2FTP_SVRADR_MAXLEN+1		);
	memset(buf_StoreDirectory	,NULL	,SCAN2FTP_STDIR_MAXLEN+1	);
	memset(buf_UserName			,NULL	,SCAN2FTP_USER_MAXLEN+1		);
	memset(buf_Password			,NULL	,SCAN2FTP_PASSWD_MAXLEN+1	);
	ftp_conn.HostAddress		= buf_HostAddress;
	ftp_conn.StoreDirectory		= buf_StoreDirectory;
	ftp_conn.IsPassive			= 0;
	ftp_conn.PortNumber			= 0;
	ftp_conn.UserName			= buf_UserName;
	ftp_conn.Password			= buf_Password;
	
	/* FTP�ڑ����̎擾 */
	DPRINTF(("[%s] profilenumber=%d\n", __FUNCTION__, profilenumber));
	scanftp_connection_setup(profilenumber,&ftp_conn);
	
	/* FTP�z�X�g�ڑ��m�F */
	ret_value_ftp = ftpclib_IsConnect(&ftp_conn);
	
	/* �ڑ�Check���ʂ̓o�^ */
	switch(ret_value_ftp){
		case FTPCLIB_SUCCESS:			/* �ڑ����� */
			scan_check_setresult(SCAN_CHECK_OK);	/* �������� */
			break;
		case FTPCLIB_ERROR_AUTH:		/* �F�؃G���[ */
			scan_check_setresult(SCAN_CHECK_ERR_AUTH);	/* �������s:AuthenticationError */
			break;
		case FTPCLIB_ERROR_TIMEOUT:		/* �^�C���A�E�g�G���[ */
			scan_check_setresult(SCAN_CHECK_ERR_TOUT);	/* �������s:ServerTimeout */
			break;
		case FTPCLIB_ERROR_SYNTAX:		/* �V���^�b�N�X�G���[(�����G���[) */
		case FTPCLIB_ERROR_STATUS:		/* ��ԑJ�ڃG���[ */
		case FTPCLIB_ERROR_EXEC:		/* ���s�G���[(�T�[�o���̃G���[) */
		case FTPCLIB_ERROR_SESSION:		/* �Z�b�V�����^�ݒ�G���[ */
		case FTPCLIB_ERROR_PARAMETER:	/* �����G���[ */
			scan_check_setresult(SCAN_CHECK_ERR_SEND);	/* �������s:SendError */
			break;
		default:
			scan_check_setresult(SCAN_CHECK_ERR_SEND);	/* �������s:SendError */
			DPRINTF(("[%s](%d) Scan to SettingCheck Err\n", __FUNCTION__, __LINE__));
			break;
	}
	return;
}

#ifdef	USE_SCAN2NW
/****************************************************************************
* @par		Scan to NW�ڑ����ݒ�
* @param	UINT8 profilenumber(input) �ڑ���profile number
* @param	stcCIFSConnect* cifs_conn(input/output) 
* @return	����
*
* @par <�O���d�l>
*		�O������J
* @par <�����d�l>
*		Scan to NW�ڑ�����ݒ肷��B
* @par <M�[>
*		M-DCSL-867
*****************************************************************************/
STATIC void
scannw_connection_setup(UINT8 profilenumber,stcCIFSConnect* cifs_conn)
{
	/* CIFS���UTF8�ϊ�buffer */
	UINT8	cifs_transUTF[SCAN2FTP_MAXLEN_OF_ALL+1];
	UINT8	AuthenticationMethod = SCANNW_AUTH_METHOD_AUTO;
	
	/* CIFS�z�X�g�ڑ����ݒ� */
	FUNC_GET_INDEXSTR(OBJ_SCAN2NW_SVRADR			,profilenumber	,cifs_conn->HostAddress				,CIFSCLIB_HOSTADDR_SIZE);	/* CIFS�z�X�g���܂���IP�A�h���X */
	
	memset(cifs_transUTF, NULL, sizeof(cifs_transUTF));
	FUNC_GET_INDEXSTR(OBJ_SCAN2NW_STDIR				,profilenumber	,cifs_transUTF						,CIFSCLIB_DIRECTORY_SIZE);	/* �t�@�C���ۑ���t�H���_ */
	ConvertUTF8( CONVERT_TO_UTF8, cifs_transUTF, cifs_conn->StoreDirectory, CIFSCLIB_DIRECTORY_SIZE+1);
	
	memset(cifs_transUTF, NULL, sizeof(cifs_transUTF));
	FUNC_GET_INDEXSTRLONG(OBJ_SCAN2NW_USER			,profilenumber	,cifs_transUTF						,CIFSCLIB_USERNAME_SIZE);	/* �F�؂̂��߂̃��[�U�[�� */
	ConvertUTF8( CONVERT_TO_UTF8, cifs_transUTF, cifs_conn->UserName, CIFSCLIB_USERNAME_SIZE+1);
	
	memset(cifs_transUTF, NULL, sizeof(cifs_transUTF));
	FUNC_GET_INDEXSTR(OBJ_SCAN2NW_PASSWD			,profilenumber, cifs_transUTF						,CIFSCLIB_PASSWORD_SIZE);	/* �F�؂̂��߂̃p�X���[�h */
	ConvertUTF8( CONVERT_TO_UTF8, cifs_transUTF, cifs_conn->Password, CIFSCLIB_PASSWORD_SIZE+1);
	
	FUNC_GET_INDEXVAL(OBJ_SCAN2NW_AUTH_METHOD		,profilenumber	,&AuthenticationMethod				);							/* �F�ؕ��@ */
	cifs_conn->AuthenticationMethod = scan_convert_cifs_auth_method(AuthenticationMethod);
	
	FUNC_GET_INDEXSTR(OBJ_SCAN2NW_KERBEROS_SVRADR	,profilenumber	,cifs_conn->kerberosServerAddress	,CIFSCLIB_KERBADDR_SIZE);	/* �P���x���X�T�[�o�������IP�A�h���X */
	
	DPRINTF(("[%s] HostAddress=%s, StoreDirectory=%s, UserName=%s, Password=%s, AuthenticationMethod=%d, kerberosServerAddress=%s\n",
			__FUNCTION__, cifs_conn->HostAddress, cifs_conn->StoreDirectory, cifs_conn->UserName, cifs_conn->Password, cifs_conn->AuthenticationMethod, cifs_conn->kerberosServerAddress));
	return;
}
#endif

#ifdef	USE_SCAN2NW
/****************************************************************************
* @par		Scan to NW�̐ڑ��m�F
* @param	UINT8 profilenumber(input) �ڑ���profile number
* @return	����
*
* @par <�O���d�l>
*		�O������J
* @par <�����d�l>
*		�ڑ����̎擾�A�ڑ��m�F�A���ʂ̐ݒ���s���B
* @par <M�[>
*		M-DCSL-867
*****************************************************************************/
STATIC	VOID
scannw_connection_check(UINT8 profilenumber)
{
	INT32	ret_value_cifs = CIFSCLIB_ERROR_SENDING;	/* �ڑ����� */
	/* CIFS�ڑ���� */
	stcCIFSConnect	cifs_conn;
	UINT8	buf_hostaddr[CIFSCLIB_HOSTADDR_SIZE+1];
	UINT8	buf_directory[CIFSCLIB_DIRECTORY_SIZE+1];
	UINT8	buf_username[CIFSCLIB_USERNAME_SIZE+1];
	UINT8	buf_password[CIFSCLIB_PASSWORD_SIZE+1];
	UINT8	buf_kerbaddr[CIFSCLIB_KERBADDR_SIZE+1];
	memset(buf_hostaddr		,NULL	,CIFSCLIB_HOSTADDR_SIZE+1	);
	memset(buf_directory	,NULL	,CIFSCLIB_DIRECTORY_SIZE+1	);
	memset(buf_username		,NULL	,CIFSCLIB_USERNAME_SIZE+1	);
	memset(buf_password		,NULL	,CIFSCLIB_PASSWORD_SIZE+1	);
	memset(buf_kerbaddr		,NULL	,CIFSCLIB_KERBADDR_SIZE+1	);
	cifs_conn.HostAddress			= buf_hostaddr;
	cifs_conn.StoreDirectory		= buf_directory;
	cifs_conn.UserName				= buf_username;
	cifs_conn.Password				= buf_password;
	cifs_conn.AuthenticationMethod	= CIFSCLIB_AUTHMETH_AUTO;
	cifs_conn.DomainName			= (UINT8*)"";
	cifs_conn.kerberosServerAddress	= buf_kerbaddr;
	
	DPRINTF(("[%s] profilenumber=%d\n", __FUNCTION__, profilenumber));
	/* CIFS�ڑ����̎擾 */
	scannw_connection_setup(profilenumber,&cifs_conn);
	
	/* CIFS�z�X�g�ڑ��m�F */
	ret_value_cifs = cifsclib_IsConnect(&cifs_conn);
	
	/* �ڑ�Check���ʂ̓o�^ */
	switch(ret_value_cifs){
		case CIFSCLIB_SUCCESS:			/* �ڑ����� */
			scan_check_setresult(SCAN_CHECK_OK);	/* �������� */
			break;
		case CIFSCLIB_ERROR_AUTH:		/* �F�؃G���[ */
			scan_check_setresult(SCAN_CHECK_ERR_AUTH);	/* �������s:AuthenticationError */
			break;
		case CIFSCLIB_ERROR_TIMEOUT:	/* �^�C���A�E�g */
			scan_check_setresult(SCAN_CHECK_ERR_TOUT);	/* �������s:ServerTimeout */
			break;
		case CIFSCLIB_ERROR_CLKNOREADY:	/* ���v���ݒ� */
			scan_check_setresult(SCAN_CHECK_ERR_WRDT);	/* �������s:WrongDate&Time */
			break;
		case CIFSCLIB_ERROR_SENDING:	/* ���M�G���[:�F�؁A�^�C���A�E�g�A�����G���[�ȊO�̃G���[ */
		case CIFSCLIB_ERROR_PARAMETER:	/* �����G���[ */
			scan_check_setresult(SCAN_CHECK_ERR_SEND);	/* �������s:SendError */
			break;
		default:
			scan_check_setresult(SCAN_CHECK_ERR_SEND);	/* �������s:SendError */
			DPRINTF(("[%s](%d) Scan to SettingCheck Err\n", __FUNCTION__, __LINE__));
			break;
	}
	return;
}
#endif

#ifdef	USE_SCAN2SFTP
/****************************************************************************
* @par		Scan to SFTP�ڑ����ݒ�
* @param	UINT8 profilenumber(input) �ڑ���profilenumber
* @param	stcSFTPConnect* sftp_conn(input/output) SFTP�ڑ����ۊǍ\���̂ւ�Pointer
* @return	�Ȃ�
*
* @par <�O���d�l>
*		�O������J
* @par <�����d�l>
*		Secure Scan to FTP�ڑ�����ݒ肷��B
* @par <M�[>
*		
*****************************************************************************/
STATIC	void
scansftp_connection_setup(UINT8 profilenumber,stcSFTPConnect* sftp_conn)
{
	UINT8	sftp_port_num[5 + 1];		/* Port�ԍ� */
	UINT8	l_PairKeyIdx;
	UINT8	l_PubKeyIdx;

	memset( sftp_port_num, NULL, sizeof(sftp_port_num) );
	/* SFTP Server Address */
	FUNC_GET_INDEXSTR( OBJ_SCAN2SFTP_SVRADR, profilenumber, sftp_conn->HostAddress, SCAN2SFTP_SVRADR_MAXLEN );
	/* Server Port Number */
	FUNC_GET_INDEXSTR( OBJ_SCAN2SFTP_PORT, profilenumber, sftp_port_num, sizeof(sftp_port_num) );
	sftp_conn->PortNumber = atoi( (const char *)sftp_port_num );
	/* User�F�ؕ��� */
	FUNC_GET_INDEXVAL( OBJ_SCAN2SFTP_AUTH_METHOD, profilenumber, &(sftp_conn->AuthMeth) );
	/* User Name */
	FUNC_GET_INDEXSTR( OBJ_SCAN2SFTP_USER, profilenumber, sftp_conn->UserName, SCAN2SFTP_USER_MAXLEN );
	/* Password */
	FUNC_GET_INDEXSTR( OBJ_SCAN2SFTP_PASSWD, profilenumber, sftp_conn->Password, SCAN2SFTP_PASSWD_MAXLEN );
	/* SFTP�T�[�o���J�� */
	FUNC_GET_INDEXVAL( OBJ_SCAN2SFTP_SERVER_PUBKEY_INDEX, profilenumber, &l_PairKeyIdx );
	sftp_conn->PubKeyIdx = (INT32)l_PairKeyIdx;
	/* MFC���y�A��ID */
	FUNC_GET_INDEXVAL( OBJ_SCAN2SFTP_KEY_PAIR_INDEX, profilenumber, &l_PubKeyIdx );
	sftp_conn->PairKeyIdx = (INT32)l_PubKeyIdx;
	/* Store Directory */
	FUNC_GET_INDEXSTR( OBJ_SCAN2SFTP_STDIR, profilenumber, sftp_conn->StoreDirectory, SCAN2SFTP_STDIR_MAXLEN );
	return;
}

/****************************************************************************
* @par		Scan to SFTP�̐ڑ��m�F
* @param	UINT8 profilenumber(input) �ڑ���profile number
* @return	����
*
* @par <�O���d�l>
*		�O������J
* @par <�����d�l>
*		�ڑ����̎擾�A�ڑ��m�F�A���ʂ̐ݒ���s���B
* @par <M�[>
*		
*****************************************************************************/
STATIC	void
scansftp_connection_check(UINT8 profilenumber)
{
	INT32	ret_value_sftp = SFTPCLIB_ERROR_BUSY;	/* �ڑ����� */

	/* SFTP�ڑ���� */
	stcSFTPConnect	sftp_conn;
	UINT8	buf_HostAddress[SCAN2SFTP_SVRADR_MAXLEN + 1];
	UINT8	buf_UserName[SCAN2SFTP_USER_MAXLEN + 1];
	UINT8	buf_Password[SCAN2SFTP_PASSWD_MAXLEN + 1];
	UINT8	buf_StoreDirectory[SCAN2SFTP_STDIR_MAXLEN + 1];

	/* �eParameter���������� */
	memset( buf_HostAddress, NULL, SCAN2SFTP_SVRADR_MAXLEN + 1 );
	memset( buf_StoreDirectory, NULL, SCAN2SFTP_STDIR_MAXLEN + 1 );
	memset( buf_UserName, NULL, SCAN2SFTP_USER_MAXLEN + 1 );
	memset( buf_Password, NULL, SCAN2SFTP_PASSWD_MAXLEN + 1 );
	sftp_conn.HostAddress = buf_HostAddress;
	sftp_conn.PortNumber = 0;
	sftp_conn.AuthMeth = 0;
	sftp_conn.UserName = buf_UserName;
	sftp_conn.Password = buf_Password;
	sftp_conn.PubKeyIdx = 0;
	sftp_conn.PairKeyIdx = 0;
	sftp_conn.StoreDirectory = buf_StoreDirectory;

	/* SFTP�ڑ����̎擾 */
	scansftp_connection_setup( profilenumber, &sftp_conn );

	/* SFTP�z�X�g�ڑ��m�F */
	ret_value_sftp = sftpclib_IsConnect( &sftp_conn );

	/* �ڑ�Check���ʓo�^ */
	switch( ret_value_sftp ){
		case	SFTPCLIB_ERR_NONE:			/* �ڑ����� */
			scan_check_setresult( SCAN_CHECK_OK );
			break;
		case	SFTPCLIB_ERR_SSHINIT:		/* SSH���������s */
		case	SFTPCLIB_ERR_SUBSYS:		/* SSH�T�u�V�X�e��SFTP�N���ł��Ȃ� */
		case	SFTPCLIB_ERR_INIT:			/* SFTP�̏��������s */
		case	SFTPCLIB_ERR_DIR:			/* �f�B���N�g�������݂��Ă��Ȃ� */
		case	SFTPCLIB_ERR_UPLOAD:		/* SFTP�f�[�^�A�b�v���[�h���s */
		case	SFTPCLIB_ERR_QUIT:			/* SFTP�T�[�o�Ƃ̐ؒf���s */
		case	SFTPCLIB_ERROR_BUSY:		/* BUSY ERROR */
		case	SFTPCLIB_ERROR_PARAMETER:	/* �����G���[ */
			scan_check_setresult( SCAN_CHECK_ERR_SEND );
			break;
		case	SFTPCLIB_ERR_TIMEOUT:		/* Time Out */
		case	SFTPCLIB_ERR_SSHDISCON:		/* SSH�T�[�o�ɐڑ��ł��Ȃ� */
			scan_check_setresult( SCAN_CHECK_ERR_TOUT );
			break;
		case	SFTPCLIB_ERR_SSHVER:		/* SSH�o�[�W�����������s */
		case	SFTPCLIB_ERR_SSHKEX:		/* SSH���������s */
		case	SFTPCLIB_ERR_USRAUTH:		/* ���[�U�[�F�؎��s */
		case	SFTPCLIB_ERR_SESSION:		/* SSH�T�[�o�Ƃ�Session�쐬���s */
			scan_check_setresult( SCAN_CHECK_ERR_AUTH );
			break;
		default:
			/* ���ʐݒ�F�������ݎ��s�iSend Error�j */
			scan_check_setresult( SCAN_CHECK_ERR_SEND );
			break;
	}
	return;
}
#endif	/* USE_SCAN2SFTP */

/****************************************************************************
* @par		Scan to FTP�ڑ�Check�J�n�v��
* @param	UINT8 profilenumber(input) 
* @return	TRUE�F����I��
*			FALSE�F�ُ�I��(����)
*
* @par <�O���d�l>
*		Scan to FTP���ڑ�Check�J�n�v�����󂯕t����B(��Ԃɂ���Ď�t�����ۂ���B)
*		���OCheck���J�n����B
* @par <�����d�l>
*		�����֐�(scan_check_start)��Call����B
* @par <M�[>
*		M-DCSL-867
*****************************************************************************/
GLOBAL	BOOL
ScanFTP_Check_Start(UINT8 profilenumber)
{
	BOOL ret = TRUE;
	ret = scan_check_start(profilenumber,(UINT16)CMD_SCAN_TO_FTPTEST_STA);
	return ret;
}
/****************************************************************************
* @par		Scan to NW�ڑ�Check�J�n�v��
* @param	UINT8 profilenumber(input)
* @return	TRUE�F����I��
*			FALSE�F�ُ�I��(����)
*
* @par <�O���d�l>
*		Scan to NW���ڑ�Check�J�n�v�����󂯕t����B(��Ԃɂ���Ď�t�����ۂ���B)
*		���OCheck���J�n����B
* @par <�����d�l>
*		�����֐�(scan_check_start)��Call����B
* @par <M�[>
*		M-DCSL-867
*****************************************************************************/
GLOBAL	BOOL
ScanNW_Check_Start(UINT8 profilenumber)
{
	BOOL ret = TRUE;
	ret = scan_check_start(profilenumber,(UINT16)CMD_SCAN_TO_NWTEST_STA);
	return ret;
}

#ifdef	USE_SCAN2SFTP
/****************************************************************************
* @par		Secure Scan to FTP�ڑ�Check�J�n�v��
* @param	UINT8 profilenumber(input)
* @return	TRUE�F����I��
*			FALSE�F�ُ�I��(����)
*
* @par <�O���d�l>
*		Secure Scan to FTP���ڑ�Check�J�n�v�����󂯕t����B(��Ԃɂ���Ď�t�����ۂ���B)
*		���OCheck���J�n����B
* @par <�����d�l>
*		�����֐�(scan_check_start)��Call����B
* @par <M�[>
*		
*****************************************************************************/
GLOBAL BOOL ScanSFTP_Check_Start(UINT8 profilenumber)
{
	BOOL	ret = TRUE;

	ret = scan_check_start(profilenumber, (UINT16)CMD_SCAN_TO_SFTPTEST_STA);
	return ret;
}
#endif	/* USE_SCAN2SFTP */

/****************************************************************************
* @par		Scan to FTP�ڑ�Check���ʎ擾
* @param	����
* @return	SCAN_CHECK_RESULT�FScan to FTP,NW�ݒ�Check�������ʕێ��ϐ�
*
* @par <�O���d�l>
*		Scan to FTP�ڑ�Check���ʂ��擾����B
* @par <�����d�l>
*		�����֐�(scan_check_getresult)��Call����B
*
* @par <M�[>
*		M-DCSL-867
*****************************************************************************/
GLOBAL	SCAN_CHECK_RESULT
ScanFTP_Check_GetResult(void)
{
	return scan_check_getresult();
}
/****************************************************************************
* @par		Scan to NW�ڑ�Check���ʎ擾
* @param	����
* @return	SCAN_CHECK_RESULT�FScan to FTP,NW�ݒ�Check�������ʕێ��ϐ�
*
* @par <�O���d�l>
*		Scan to NW�ڑ�Check���ʂ��擾����B
* @par <�����d�l>
*		�����֐�(scan_check_getresult)��Call����B
*
* @par <M�[>
*		M-DCSL-867
*****************************************************************************/
GLOBAL	SCAN_CHECK_RESULT
ScanNW_Check_GetResult(void)
{
	return scan_check_getresult();
}

#ifdef	USE_SCAN2SFTP
/****************************************************************************
* @par		Secure Scan to FTP�ڑ�Check���ʎ擾
* @param	����
* @return	SCAN_CHECK_RESULT�FScan to FTP,NW,SFTP�ݒ�Check�������ʕێ��ϐ�
*
* @par <�O���d�l>
*		Secure Scan to FTP�ڑ�Check���ʂ��擾����B
* @par <�����d�l>
*		�����֐�(scan_check_getresult)��Call����B
*
* @par <M�[>
*		
*****************************************************************************/
GLOBAL SCAN_CHECK_RESULT
ScanSFTP_Check_GetResult(void)
{
	return scan_check_getresult();
}
#endif	/* USE_SCAN2SFTP */

/*****************************************************************************
* @par ScanToFTP/NW�p�̉𑜓x���A�F���Ɖ𑜓x�ɕ�����
*
* @param	quality						(input)  �F���{�ǎ�𑜓x(SCAN2FTP_QUALITY)
* @param	color						(output) �F��(COLOR_MODE_XXXX)
* @param	resolution					(output) �ǎ�𑜓x(RESOLUTION_XXXDPI)
* @return
*
* @par <�O���d�l>
*
* @par <�����d�l>
*	�{�֐���ColorAuto/GrayAuto�̉𑜓x���擾���悤�Ƃ����ꍇ�A
*	FTPC_DEFAULT_RESO(Default�𑜓x)��Ԃ�
*	���{���́Ascanmenu_EiScanGetResolution�ɂĉ𑜓x���擾����
******************************************************************************/
STATIC	VOID
conv_scan_quality_to_color_reso(UINT16 quality,UINT8 *color, UINT16 *resolution)
{
	UINT8	index;
	UINT8	temp_color;
	UINT8	temp_reso;

	for(index = 0; index < sizeof(conv_quality_to_color_scan_quality) / sizeof(FTPC_CONV_TBL); index++)
	{
		if(conv_quality_to_color_scan_quality[index].ftp_quality == quality)
		{
			temp_color = conv_quality_to_color_scan_quality[index].color_num;
			temp_reso = conv_quality_to_color_scan_quality[index].resolution;
			break;
		}
	}
	
	if(index >= sizeof(conv_quality_to_color_scan_quality) / sizeof(FTPC_CONV_TBL))
	{
		temp_color = FTPC_DEFAULT_COLOR;
		temp_reso = FTPC_DEFAULT_RESO;
	}

	if(color != NULL){
		*color = temp_color;
	}
	if(resolution != NULL){
		*resolution = temp_reso;
	}

	return;
}

/*****************************************************************************
* @par �ǂݎ��y�[�W����ێ�����i�����y�[�W���܂܂Ȃ� �ǂݎ�芮���y�[�W���j
*
* @param	iAction	(in)	�y�[�W���̐�����@
* @param	ioPage	(out)	�y�[�W��
*
******************************************************************************/
STATIC VOID ftpclient_ScanPage( FTPC_PGCNT_ACT iAction, UINT32* ioPage )
{
	STATIC UINT32 sFtpcScanPage = FTPC_PGCNT_INVALID_VAL;
	DPRINTF(("%s ",__FUNCTION__));

	switch(iAction)
	{
	case FTPC_PGCNT_START:
		DPRINTF((" FTPC_PGCNT_START\n"));
		sFtpcScanPage = 0;
		break;
	case FTPC_PGCNT_INCREMENT:
		DPRINTF((" FTPC_PGCNT_INCREMENT\n"));
		if(sFtpcScanPage != FTPC_PGCNT_INVALID_VAL){
			sFtpcScanPage++;
		}
		break;
	case FTPC_PGCNT_INIT:
	case FTPC_PGCNT_END:
		DPRINTF((" FTPC_PGCNT_INIT/END sFtpcScanPage[%d]\n",sFtpcScanPage));
		sFtpcScanPage = FTPC_PGCNT_INVALID_VAL;
		break;
	case FTPC_PGCNT_GET:
		DPRINTF((" FTPC_PGCNT_GET[%d]\n", sFtpcScanPage));
		if(ioPage != NULL) {
			*ioPage = sFtpcScanPage;
		}
		break;
	default:
		DPRINTF((" default[%d]\n", iAction));
		break;
	}
}

/*****************************************************************************
* @par �������o�y�[�W����ێ�����
*
* @param	iAction	(in)	�����y�[�W���̐�����@
* @param	ioPage	(out)	�����y�[�W��
*
******************************************************************************/
STATIC VOID ftpclient_BlankPage( FTPC_PGCNT_ACT iAction, UINT32* ioPage )
{
	STATIC UINT32 sFtpcScanBlankPage = FTPC_PGCNT_INVALID_VAL;
	DPRINTF(("%s",__FUNCTION__));
	switch(iAction)
	{
	case FTPC_PGCNT_START:
		DPRINTF((" FTPC_PGCNT_START\n"));
		sFtpcScanBlankPage = 0;
		break;
	case FTPC_PGCNT_INCREMENT:
		DPRINTF((" FTPC_PGCNT_INCREMENT\n"));
		if(sFtpcScanBlankPage != FTPC_PGCNT_INVALID_VAL){
			sFtpcScanBlankPage++;
		}
		break;
	case FTPC_PGCNT_INIT:
	case FTPC_PGCNT_UNUSE:
	case FTPC_PGCNT_END:
		DPRINTF((" FTPC_PGCNT_INIT/UNUSE/END sFtpcScanBlankPage[%d]\n",sFtpcScanBlankPage));
		sFtpcScanBlankPage = FTPC_PGCNT_INVALID_VAL;
		break;
	case FTPC_PGCNT_GET:
		DPRINTF((" FTPC_PGCNT_GET[%d]\n", sFtpcScanBlankPage));
		if(ioPage != NULL) {
			*ioPage = sFtpcScanBlankPage;
		}
		break;
	default:
		DPRINTF((" default[%d]\n", iAction));
		break;
	}
}

/*****************************************************************************
* @par ���݂̓ǎ�y�[�W�����Ɣ������o�y�[�W����Ԃ�
*
* @param	page		(output)�������܂ށA�ǎ抮���������y�[�W��
* @param	blankpage	(output)�������m�����y�[�W��
* @return
*
* @par <�O���d�l>
*
* @par <�����d�l>
*
******************************************************************************/
GLOBAL VOID ftpclient_getScanPage(UINT32 *page, UINT32 *blankpage)
{
	UINT32 ScanPage_local;
	UINT32 BlankPage_local;

	ftpclient_ScanPage(FTPC_PGCNT_GET, &ScanPage_local);
	ftpclient_BlankPage(FTPC_PGCNT_GET, &BlankPage_local);

	if(page != NULL){
		*page = ScanPage_local;
		if(BlankPage_local != FTPC_PGCNT_INVALID_VAL) {
			*page += BlankPage_local;
		}
	}
	if(blankpage != NULL) {
		*blankpage = BlankPage_local;
	}
	return;
}

#ifdef USE_SEPARATE_UI
/*****************************************************************************
* @par �X�L����������̉����V�[�P���X
* @param	event		(input) ������ʂ�����ꂽ�v��
*							FTPC_UIEVT_OK	OK�{�^������
*							FTPC_UIEVT_STOP	STOP�{�^������
* @return
*
* @par <�O���d�l>
*		������ʂ�����ꂽ�Ƃ��ɃR�[�������
* @par <�����d�l>
*		FTPClient�^�X�N�ɁAComplete��Ԃ���Ready��Ԃ֕��A���邽�߂̃��b�Z�[�W�𑗐M����
******************************************************************************/
GLOBAL VOID	ftpclient_closeCompleteStatus(INT32 event)
{
#ifdef FTPC_COMPLETE_SEQUENCE
	TASK_MSG_COM_T	com_msg;
	
	com_msg.from_task	= FTPC_APL_TASK;
	
	switch( event )
	{
	case FTPC_UIEVT_OK:		/* OK�{�^������ */
		com_msg.cmd_id	= CMD_CLOSE_COMPLETE_OK;
		break;
	case FTPC_UIEVT_STOP:	/* STOP�{�^������ */
		com_msg.cmd_id	= CMD_CLOSE_COMPLETE_ST;
		break;
	default:				/* ���̑��̈��� */
		com_msg.cmd_id	= CMD_CLOSE_COMPLETE_NG;
		break;
	}
	
	cancel_ftpc_CompleteStatus_end_time();
	FOS_MSGSEND(qid_ftpclient,(UINT8 *)&com_msg,sizeof(TASK_MSG_COM_T));
#endif /* #ifdef FTPC_COMPLETE_SEQUENCE */
	return;
}
#endif /* #ifdef USE_SEPARATE_UI */

#ifdef FTPC_COMPLETE_SEQUENCE
/*****************************************************************************
* @par �X�L����������̉����V�[�P���X
* @param	event		(input) ������ʂ�����ꂽ�v��
*							CMD_CLOSE_COMPLETE_OK	OK�{�^������
*							CMD_CLOSE_COMPLETE_ST	STOP�{�^������
*			image_id	(input) ��f�[�^ID
* @return
*
* @par <�O���d�l>
*		�A�v���^�X�N�̏�Ԃ�COMPLETE��READY��Ԃɂ���(���̏�Ԃ����READY�ɂȂ�Ȃ�)
* @par <�����d�l>
*
******************************************************************************/
STATIC VOID	ftpclient_CompleteStatus_end(UINT16 event, INT32 image_id)
{
	UINT32 ScanPage;

	if( ftpscan_state == FTPC_COMPLETE ){
		/* �f�[�^���M�ֈڍs���邩���� */
		ftpclient_ScanPage(FTPC_PGCNT_GET, &ScanPage);
		if( (ScanPage == 0 || ScanPage == FTPC_PGCNT_INVALID_VAL) || event == CMD_CLOSE_COMPLETE_ST ) {
			ImageDelete ( image_id );
			/* �X�e�[�^�X��ҋ@��� */
			ftpscan_state   = FTPC_START_WAIT;
#ifdef	USE_SEPARATE_UI
			cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif
		}
		else if( event == CMD_CLOSE_COMPLETE_OK ){
			/* ���M�˗�          */
			ftpscan_state   = FTPC_SEND;
			transfer_process(image_id);
		}
	}
	return;
}

/*********************************************************************************************
* @par		Complete��Ԃ��^�C���A�E�g�����Ƃ����b�Z�[�W�𑗐M�̃^�C�}�[���Z�b�g
* @param	time(input)	�^�C���A�E�g����(10ms�P��)
* @retval	�Ȃ�
* @par	<�O���d�l>
*			time���Ԍ�ɁAComplete���������p���b�Z�[�W�𑗐M����
* @par	<�����d�l>
*			�Q�l�FpanelTaskMain.c�@set_flock_polling_time()				
*********************************************************************************************/
STATIC VOID
set_ftpc_CompleteStatus_end_time(UINT32 time)
{
	TASK_MSG_COM_T		   com_msg;

	DPRINTF(("set_ftpc_CompleteStatus_end_time(%d)\n",time));

	cancel_ftpc_CompleteStatus_end_time();
	/* �^�C���A�E�g��ݒ肷�� */
	com_msg.from_task	= FTPC_APL_TASK;
	com_msg.cmd_id		= CMD_CLOSE_COMPLETE_OK; 
	timer_id			= FOS_SETTIMEMSG( time,qid_ftpclient,*((UINT32*)&com_msg) );

	return;
}


/*********************************************************************************************
* @par		�^�C���A�E�g�p�ɓo�^�����^�C�}�[�̉������s��
* @param	�Ȃ�
* @retval	�Ȃ�
* @par	<�O���d�l>
*			�^�C���A�E�g��Complete���������p���b�Z�[�W���M�̉������s��
* @par	<�����d�l>
*			�Q�l�FpanelTaskMain.c�@cancel_flock_polling_time()
*********************************************************************************************/
STATIC VOID
cancel_ftpc_CompleteStatus_end_time(VOID)
{
	DPRINTF(("cancel_ftpc_CompleteStatus_end_time\n"));
	if(timer_id != 0)
	{
		FOS_CLEARTIMEMSG(timer_id);
		timer_id = 0;
	}

	return;
}
#endif /* FTPC_COMPLETE_SEQUENCE */
#endif	/* USE_SCAN2FTP || USE_SCAN2NW || USE_SCAN2USB */



	
/****** end of "ftpclient.c" ***************************************/
