/*****************************************************************************
 *
 *	ftpclient.h :
 *		main header file transfer task
 *
 *	Copyright 2004 - 2008 Brother Industries , Ltd
 *
 *	Aug.08 2008 : ABS : created for FTP/Network(CIFS) spec.
 *
 *	$Id: //depot/Firm/Commonfile/BC2/base/fb_sst_cloud_bsi/Laser/task/ftpclient/ftpclient.h#1 $
 *
 *****************************************************************************/

#ifndef _FTPCLIENT_H		/* ���d�C���N���[�h�h�~ */
#define _FTPCLIENT_H

#if defined(USE_SCAN2FTP) || defined(USE_SCAN2NW) || defined(USE_LOG2NW) || defined(USE_SCAN2USB)

/* FTP/CIFS���C�u�����̃w�b�_�[�t�@�C�� */
#include	"componentlib/netprotocollib/ftpclib.h"
#include	"componentlib/netprotocollib/cifsclib.h"

/*------------------------------------------------------------------------*/
/****** global defines ****************************************************/
/*------------------------------------------------------------------------*/
#if defined(USE_SCAN_BLANK_DETECT) && defined(USE_SEPARATE_UI)
/* ADS�ŗL�̋@�\�̂��߁A�{���}�[�W���ɂ́uFTPC_COMPLETE_SEQUENCE�v��undef�ɂ��邱�� */
#undef FTPC_COMPLETE_SEQUENCE		/* �X�L�����������̊�����ʕ\���@�\ */
#endif /* defined(USE_SCAN_BLANK_DETECT) && defined(USE_SEPARATE_UI) */

/* �^�X�N�A���b�Z�[�W�֌W */
#define	FTPC_TASK_NAME			"ftpc"
#define	FTPC_MSG_NAME			"ftpm"
#define FTPC_MSG_COUNT			16
#define FTPC_MSG_LENGTH			16
#define	CMD_SCANSTART			401			/* �X�L�����J�n           */
#define	CMD_SCANSTOP			402			/* �X�L�������f�I��       */
#define	CMD_NEXTPAGE			403			/* ���y�[�W���X�L����     */
#define	CMD_SCANEND				404			/* �X�L��������I��       */
#ifdef FUNC_ADF_SCAN_SPEC_SHEETS_STOP
#define	CMD_SCAN_LIMIT_SND		405			/* �X�L���������ɂ�鑗�M */
#define	CMD_SCAN_LIMIT_DEL		406			/* �X�L���������ɂ��폜 */
#endif
#ifdef FTPC_COMPLETE_SEQUENCE
#define CMD_CLOSE_COMPLETE_NG	0xFFFF
#define CMD_CLOSE_COMPLETE_OK	0x0001
#define CMD_CLOSE_COMPLETE_ST	0x0002
#endif /* FTPC_COMPLETE_SEQUENCE */

/* Scan to FTP,NW�̎����m�F�p */
#define	CMD_SCAN_TO_FTPTEST_STA		(407)	/* Scan to FTP �ڑ��m�F�J�n */
#define	CMD_SCAN_TO_NWTEST_STA		(408)	/* Scan to NW �ڑ��m�F�J�n */
#ifdef	USE_SCAN2SFTP
#define	CMD_SCAN_TO_SFTPTEST_STA	(409)	/* Secure Scan to FTP �ڑ��m�F�J�n*/
#endif	/* USE_SCAN2SFTP */

/* ���[�U�[���͏��֌W */
#define FTPC_SPDF_PASSWD_MAXLEN		32

/* FTP/NW�֘A */
#define SCAN2FTP_FILE_STRMAX		32
#define SCAN2NW_FILE_STRMAX			64

#define SCAN2FTP_SVR_MAXLEN			( SCANFTP_SERVER_STRMAX * BR_MB_LEN_MAX )
#define SCAN2FTP_SVRADR_MAXLEN		64
#define SCAN2FTP_USER_MAXLEN		32
#define SCAN2FTP_PASSWD_MAXLEN		32
#define SCAN2FTP_PAMODE_MAXLEN		1
#define SCAN2FTP_PORT_MAXLEN		2
#define SCAN2FTP_STDIR_MAXLEN		60
#define SCAN2FTP_FNAME_MAXLEN		(SCAN2FTP_FILE_STRMAX + 15)		/* �t�@�C����+"_"+6+"_"+JPEG�A��(MAX3��)+4(�g���q[.JPG]) */
#define SCAN2FTP_FNAME_MAXSIZE		((SCAN2NW_FILE_STRMAX * BR_MB_LEN_MAX) + 15 + 1)	/* �t�@�C�����iUTF8�j+"_"+6+"_"+JPEG�A��(MAX3��)+4(�g���q[.JPG]+1(���Ȱ�)) */
#define	SCAN2FTP_MAXLEN_OF_ALL		(CIFSCLIB_USERNAME_SIZE) + 16
                                                /* FTP/CIFS���܂߂�define�Q�̍ő�l(400)                           */

/* SFTP�֘A */
#ifdef	USE_SCAN2SFTP
#define SCAN2SFTP_SVR_MAXLEN		( SCANFTP_SERVER_STRMAX * BR_MB_LEN_MAX )
#define SCAN2SFTP_SVRADR_MAXLEN		64
#define SCAN2SFTP_USER_MAXLEN		32
#define SCAN2SFTP_PASSWD_MAXLEN		32
#define SCAN2SFTP_STDIR_MAXLEN		60
#define	SCAN2SFTP_FNAME_MAXLEN		(SCAN2FTP_FILE_STRMAX + 15)		/* �t�@�C����+"_"+6+"_"+JPEG�A��(MAX3��)+4(�g���q[.JPG]) */

#endif	/* USE_SCAN2SFTP */

/* �d�����m */
#define	FTPNW_MULTIFEED_DETECT_ON	0x01
#define	FTPNW_MULTIFEED_DETECT_OFF	0x00
/* �������� */
#define	FTPNW_BLANK_DETECT_ON		0x01
#define	FTPNW_BLANK_DETECT_OFF		0x00
/* �΍s�␳ */
#define	FTPNW_DESKEW_ON				0x01
#define	FTPNW_DESKEW_OFF			0x00

#define	FTPC_PGCNT_INVALID_VAL	(0xFFFF)	/**< �Ŗ��� �����l ftpclient_getScanPage - ���݂̓ǎ�y�[�W�����Ɣ������o�y�[�W�� */

/* CIFS�֘A��CIFS���C�u�����̃w�b�_�t�@�C���icifsclib.h�j���g�p */

/*------------------------------------------------------------------------*/
/****** public structures *************************************************/
/*------------------------------------------------------------------------*/
/* �X�L���i���j���[����󂯎���� */
typedef enum
{
	FTP_Color100 = 140,
	FTP_Color200,
	FTP_Color300,
	FTP_Color600,
	FTP_Gray100,
	FTP_Gray200,
	FTP_Gray300,
	FTP_BW200,
	FTP_BW100
#if defined(USE_SCAN_AUTO_RESOLUTION)
	,FTP_ColorAuto
	,FTP_GrayAuto
#endif
	,FTP_BW300
	,FTP_Color400
	,FTP_Gray400
	,FTP_Gray600
	,FTP_BW150
	,FTP_Color150
	,FTP_Gray150
#ifdef USE_SCAN_COLOR_DETECT
	,FTP_Auto150
	,FTP_Auto200
	,FTP_Auto300
	,FTP_Auto600
#endif
	,FTP_BW600
}SCAN2FTP_QUALITY;

typedef enum
{
	FTP_PDF = 150,
#ifdef USE_PDFA
	FTP_PDFA,
#endif /* USE_PDFA */
	FTP_SPDF,
#ifdef USE_SIGNEDPDF
	FTP_SIPDF,
#endif /* USE_SIGNEDPDF */
	FTP_JPEG,
	FTP_Tiff,
	FTP_XPS
}SCAN2FTP_FILEFORMAT;

typedef enum
{
	FTP_SIZE_LARGE = 0,
	FTP_SIZE_MIDDLE,
	FTP_SIZE_SMALL,
	FTP_SIZE_MANUAL,
	FTP_SIZE_NONE,
}SCAN2FTP_FILESIZE;

typedef enum
{
	FTP_SIMPLEX = 160,
	FTP_DUPLEXLONG,
	FTP_DUPLEXSHORT
}SCAN2FTP_DUALSCAN;

typedef enum
{
	FTP_FNAMETYPE0_NODE =170,
	FTP_FNAMETYPE1_ESTM,
	FTP_FNAMETYPE2_REPO,
	FTP_FNAMETYPE3_ORDE,
	FTP_FNAMETYPE4_CONT,
	FTP_FNAMETYPE5_CHCK,
	FTP_FNAMETYPE6_RCPT,
	FTP_FNAMETYPE7_OPT1,
	FTP_FNAMETYPE8_OPT2,
	FTP_FNAMETYPE9_MANU,
}SCAN2FTP_FNAMETYPE;

typedef struct
{
	SCAN2FTP_QUALITY		quality;										/* �掿                 */
	SCAN2FTP_FILEFORMAT		fileformat;										/* �t�@�C���t�H�[�}�b�g */
	SCAN2FTP_FILESIZE		filesize;										/* �t�@�C���T�C�Y       */
	SCAN2FTP_DUALSCAN		dualscan;										/* ���ʓǂݎ��         */
	SCAN2FTP_FNAMETYPE		filenametype;									/* �擪������̃^�C�v   */
	UINT8					servername    [ SCAN2FTP_SVR_MAXLEN    +1 ];	/* FTP Server ��        */
	UINT8					serveraddress [ SCAN2FTP_SVRADR_MAXLEN +1 ];	/* FTP ServerAddress    */
	UINT8					username      [ SCAN2FTP_USER_MAXLEN   +1 ];	/* USER��               */
	UINT8					password      [ SCAN2FTP_PASSWD_MAXLEN +1 ];	/* PASSWORD             */
	UINT8					storedir      [ SCAN2FTP_STDIR_MAXLEN  +1 ];	/* �i�[DIRECTORY        */
	UINT8					filename      [ SCAN2FTP_FNAME_MAXSIZE ];		/* �t�@�C����           */
	UINT8					spdfpass      [ FTPC_SPDF_PASSWD_MAXLEN+1 ];	/* Secure PDF PASSWORD  */
    UINT8                   ispassive;                                      /* Passive or active    */
    UINT32                  portnum;                                        /* �|�[�g�ԍ�           */
    UINT8                   scan_quality;                                   /* �X�L�������k��       */
#ifdef USE_TMP_SCANSIZE
	UINT8                   scan_doc_size;                                  /* Glass Scan Size      */
#endif /* USE_TMP_SCANSIZE */                                 /* �X�L�������k��       */
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	UINT8               	scan_src;										/* ���e�ǂݎ��ʒu		*/
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
	UINT8					AuthenticationMethod;							/* �F�ؕ��� */
	INT32					PubKeyIdx;										/* Publib Key Index */
	INT32					PairKeyIdx;										/* MFC��Pair */
#endif	/* USE_SCAN2SFTP */
	UINT8					FileNameFixed;
}ACCESS_INFO;

typedef struct
{
	SCAN2FTP_QUALITY		quality;										/* �掿                 */
	SCAN2FTP_FILEFORMAT		fileformat;										/* �t�@�C���t�H�[�}�b�g */
	SCAN2FTP_DUALSCAN		dualscan;										/* ���ʓǂݎ��         */
	SCAN2FTP_FNAMETYPE		filenametype;									/* �擪������̃^�C�v   */
	UINT8					serveraddress [ CIFSCLIB_HOSTADDR_SIZE +1 ];	/* CIFS ServerAddress    */
	UINT8					username      [ CIFSCLIB_USERNAME_SIZE +1 ];	/* USER��               */
	UINT8					password      [ CIFSCLIB_PASSWORD_SIZE +1 ];	/* PASSWORD             */
	UINT8					storedir      [ CIFSCLIB_DIRECTORY_SIZE+1 ];	/* �i�[DIRECTORY        */
	UINT8					filename      [ SCAN2FTP_FNAME_MAXSIZE ];		/* �t�@�C����           */
	UINT8					AuthenticationMethod;							/* �F�ؕ��@        */
	UINT8					KerberosServerAddress    [ CIFSCLIB_KERBADDR_SIZE+1 ];	/* �P���x���X�T�[�o���܂���IP�A�h���X */
	UINT8					spdfpass      [ FTPC_SPDF_PASSWD_MAXLEN+1 ];	/* Secure PDF PASSWORD  */
    UINT8                   scan_quality;                                   /* �X�L�������k��       */
#ifdef USE_TMP_SCANSIZE
	UINT8                   scan_doc_size;                                  /* Glass Scan Size      */
#endif /* USE_TMP_SCANSIZE */                                /* �X�L�������k��       */
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	UINT8               	scan_src;										/* ���e�ǂݎ��ʒu		*/
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
	UINT8					scan_multifeed_detect;							/* �d�����m ON/OFF */
	UINT8					scan_blank_detect;								/* ���������@�\ ON/OFF */
	UINT8					scan_deskew_adjust;								/* �΍s�␳ ON/OFF */
	UINT8					scan_quality_gray;								/* �F��Auto����Gray���k�� */
#ifdef    USE_GNDCOLOR_REMOVAL
	UINT8		gndcolor_removal;			/* �J���[�n�F����		*//* M-BHS13-648 */
	UINT8		gndcolor_level;				/* �J���[�n�F�����ݒ�	*//* M-BHS13-648 */
#endif /* USE_GNDCOLOR_REMOVAL */
	UINT8					FileNameFixed;
}CIFSACCESS_INFO;

/* ���b�Z�[�W�\���� */
typedef struct
{
	UINT16			from_task;
	UINT16			cmd_id;
	UINT8			profilenumber;	/* �ڑ���profile number */
}ftpclt_msg;

/* FTP Client�^�X�N��Ԉꗗ */
typedef enum
{
	FTPC_SCANNING = 401,			/* �ǂݎ�蒆                 */
	FTPC_SELECTNEXT,				/* ���y�[�W�ǂݍ��ݑI��     */
	FTPC_WAITNEXT,					/* ���y�[�W�ǂݍ��݌���       */
                                    /* SET�L�[�҂�                */
	FTPC_SCANSTOP,					/* �ǂݎ�蒆�f               */
	FTPC_MEM_ERR,					/* �������G���[�������       */
	FTPC_IDLE,						/* �҂��󂯒�                 */
    FTPC_START_WAIT,                /* �J�n�w���҂�               */
	FTPC_SEND,						/* �t�@�C�����M��             */
#ifdef FTPC_COMPLETE_SEQUENCE
	FTPC_COMPLETE,					/* ������ʕ\����			  */
#endif /* FTPC_COMPLETE_SEQUENCE */
#ifdef FUNC_ADF_SCAN_SPEC_SHEETS_STOP
	FTPC_SCAN_LIMIT_STOPPING		/* 35���A���ł̓ǎ��~������ */
#endif /* FUNC_ADF_SCAN_SPEC_SHEETS_STOP */
}FTPSCAN_STATE;

/* ftpclient.c �������ւ̃A�N�Z�X */
typedef enum
{
	INTERNAL_READ = 411,			/* ��������ǂݏo��     */
	INTERNAL_WRITE					/* ����������������     */
}INTERNAL_ACCESS;

#ifdef USE_SEPARATE_UI
#define FTPC_UIEVT_OK	0x00000001	/* �������Close OK�L�[���� */
#define FTPC_UIEVT_STOP	0x00000002	/* �������Close STOP�L�[���� */
#endif /* #ifdef USE_SEPARATE_UI */
/* Scan to FTP,NW�̎����m�F�p ����enum�A��ԂƊm�F���ʏڍׂ����C���Ă����̂ŋ@�����΁A���ꂼ�ꂵ������ƕ��������݌v�ɂ������B */
typedef enum {	
	SCAN_CHECK_INIT			= 0		/* ������� */
	,SCAN_CHECK_BUSY				/* ������ */	
	,SCAN_CHECK_OK					/* �������� */	
	,SCAN_CHECK_ERR_TOUT			/* �������s:ServerTimeout */	
	,SCAN_CHECK_ERR_AUTH			/* �������s:AuthenticationError */	
	,SCAN_CHECK_ERR_SEND			/* �������s:SendError */	
	,SCAN_CHECK_ERR_WRDT			/* �������s:WrongDate&Time */	
	,SCAN_CHECK_ERR_BUSY			/* �������s:ftpscan_state != FTPC_START_WAIT�iftpc task is BUSY�j */	
	/* �R�R����ɒǉ� */
	,SCAN_CHECK_MAX
}	SCAN_CHECK_RESULT;
#define	SCAN_CHECK_RESULT_RESET_TIME	1000	/* 10�b(10ms�P��) */
#define	SCAN_CHECK_BUSSY_RESET_TIME		30000	/* 300�b(10ms�P��) */
/* 
 *  SCAN_CHECK_BUSSY_RESET_TIME
 *  TCP TimeOut�̎��Ԃ�0�`32767���܂Őݒ�ł��邪�A
 *  32767���ɐݒ肵�Ă�SeverTimeout�̎��Ԃ�180�b�قǂŕԂ��ė��Ă���B
 *  ServerTimeout���Ԃ�蒷���ݒ肳��Ă���Ζ�薳���̂�
 *  TCP TimeOut�̎��Ԃł͖����Œ�l300�b�Őݒ肷��B(M-DCSL-867)
*/
/*------------------------------------------------------------------------*/
/****** global functions **************************************************/
/*------------------------------------------------------------------------*/
GLOBAL void     ftpclient_main          ( void );
GLOBAL void     ftpclient_setaccessinfo ( ACCESS_INFO* );
GLOBAL void     cifsclient_setaccessinfo( CIFSACCESS_INFO* );
#ifdef	USE_SCAN2SFTP
GLOBAL void		sftpclient_setaccessinfo( ACCESS_INFO* );
#endif	/* USE_SCAN2SFTP */
GLOBAL void     ftpclient_state         ( FTPSCAN_STATE*, INTERNAL_ACCESS );
GLOBAL INT32    scan_counter            ( UINT32 *scan_counter );

GLOBAL VOID		ftpclient_getScanPage	(UINT32 *page, UINT32 *blankpage);
#ifdef USE_SEPARATE_UI
GLOBAL VOID		ftpclient_closeCompleteStatus(INT32 event);
#endif /* USE_SEPARATE_UI */
#if defined(USE_SCAN_AUTO_RESOLUTION)
GLOBAL INT32    get_sysmid_try          ( SCAN2FTP_QUALITY quality, BOOL dup_scan, UINT8 scan_quality, UINT32 file_format );
#else
GLOBAL INT32    get_sysmid_try          ( SCAN2FTP_QUALITY quality, BOOL dup_scan, UINT8 scan_quality );
#endif

/* Scan to FTP,NW�̎����m�F�p */
GLOBAL	BOOL	ScanFTP_Check_Start(UINT8 profilenumber);
GLOBAL	BOOL	ScanNW_Check_Start(UINT8 profilenumber);
#ifdef	USE_SCAN2SFTP
GLOBAL	BOOL	ScanSFTP_Check_Start(UINT8 profilenumber);
#endif	/* USE_SCAN2SFTP */
GLOBAL	SCAN_CHECK_RESULT	ScanFTP_Check_GetResult(void);
GLOBAL	SCAN_CHECK_RESULT	ScanNW_Check_GetResult(void);
#ifdef	USE_SCAN2SFTP
GLOBAL	SCAN_CHECK_RESULT	ScanSFTP_Check_GetResult(void);
#endif	/* USE_SCAN2SFTP */
#endif	/* defined(USE_SCAN2FTP) || defined(USE_SCAN2NW) || defined(USE_LOG2NW) || defined(USE_SCAN2USB)*/

#endif	/* _FTPCLIENT_H */

/****** end of "ftpclient.h" ****************************************/
