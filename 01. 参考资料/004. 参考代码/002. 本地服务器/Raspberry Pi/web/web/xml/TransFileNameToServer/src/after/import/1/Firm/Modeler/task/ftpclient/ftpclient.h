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

#ifndef _FTPCLIENT_H		/* 多重インクルード防止 */
#define _FTPCLIENT_H

#if defined(USE_SCAN2FTP) || defined(USE_SCAN2NW) || defined(USE_LOG2NW) || defined(USE_SCAN2USB)

/* FTP/CIFSライブラリのヘッダーファイル */
#include	"componentlib/netprotocollib/ftpclib.h"
#include	"componentlib/netprotocollib/cifsclib.h"

/*------------------------------------------------------------------------*/
/****** global defines ****************************************************/
/*------------------------------------------------------------------------*/
#if defined(USE_SCAN_BLANK_DETECT) && defined(USE_SEPARATE_UI)
/* ADS固有の機能のため、本線マージ時には「FTPC_COMPLETE_SEQUENCE」をundefにすること */
#undef FTPC_COMPLETE_SEQUENCE		/* スキャン完了時の完了画面表示機能 */
#endif /* defined(USE_SCAN_BLANK_DETECT) && defined(USE_SEPARATE_UI) */

/* タスク、メッセージ関係 */
#define	FTPC_TASK_NAME			"ftpc"
#define	FTPC_MSG_NAME			"ftpm"
#define FTPC_MSG_COUNT			16
#define FTPC_MSG_LENGTH			16
#define	CMD_SCANSTART			401			/* スキャン開始           */
#define	CMD_SCANSTOP			402			/* スキャン中断終了       */
#define	CMD_NEXTPAGE			403			/* 次ページをスキャン     */
#define	CMD_SCANEND				404			/* スキャン正常終了       */
#ifdef FUNC_ADF_SCAN_SPEC_SHEETS_STOP
#define	CMD_SCAN_LIMIT_SND		405			/* スキャン制限による送信 */
#define	CMD_SCAN_LIMIT_DEL		406			/* スキャン制限による削除 */
#endif
#ifdef FTPC_COMPLETE_SEQUENCE
#define CMD_CLOSE_COMPLETE_NG	0xFFFF
#define CMD_CLOSE_COMPLETE_OK	0x0001
#define CMD_CLOSE_COMPLETE_ST	0x0002
#endif /* FTPC_COMPLETE_SEQUENCE */

/* Scan to FTP,NWの自動確認用 */
#define	CMD_SCAN_TO_FTPTEST_STA		(407)	/* Scan to FTP 接続確認開始 */
#define	CMD_SCAN_TO_NWTEST_STA		(408)	/* Scan to NW 接続確認開始 */
#ifdef	USE_SCAN2SFTP
#define	CMD_SCAN_TO_SFTPTEST_STA	(409)	/* Secure Scan to FTP 接続確認開始*/
#endif	/* USE_SCAN2SFTP */

/* ユーザー入力情報関係 */
#define FTPC_SPDF_PASSWD_MAXLEN		32

/* FTP/NW関連 */
#define SCAN2FTP_FILE_STRMAX		32
#define SCAN2NW_FILE_STRMAX			64

#define SCAN2FTP_SVR_MAXLEN			( SCANFTP_SERVER_STRMAX * BR_MB_LEN_MAX )
#define SCAN2FTP_SVRADR_MAXLEN		64
#define SCAN2FTP_USER_MAXLEN		32
#define SCAN2FTP_PASSWD_MAXLEN		32
#define SCAN2FTP_PAMODE_MAXLEN		1
#define SCAN2FTP_PORT_MAXLEN		2
#define SCAN2FTP_STDIR_MAXLEN		60
#define SCAN2FTP_FNAME_MAXLEN		(SCAN2FTP_FILE_STRMAX + 15)		/* ファイル名+"_"+6+"_"+JPEG連番(MAX3桁)+4(拡張子[.JPG]) */
#define SCAN2FTP_FNAME_MAXSIZE		((SCAN2NW_FILE_STRMAX * BR_MB_LEN_MAX) + 15 + 1)	/* ファイル名（UTF8）+"_"+6+"_"+JPEG連番(MAX3桁)+4(拡張子[.JPG]+1(ﾀｰﾐﾈｰﾄ)) */
#define	SCAN2FTP_MAXLEN_OF_ALL		(CIFSCLIB_USERNAME_SIZE) + 16
                                                /* FTP/CIFSを含めたdefine群の最大値(400)                           */

/* SFTP関連 */
#ifdef	USE_SCAN2SFTP
#define SCAN2SFTP_SVR_MAXLEN		( SCANFTP_SERVER_STRMAX * BR_MB_LEN_MAX )
#define SCAN2SFTP_SVRADR_MAXLEN		64
#define SCAN2SFTP_USER_MAXLEN		32
#define SCAN2SFTP_PASSWD_MAXLEN		32
#define SCAN2SFTP_STDIR_MAXLEN		60
#define	SCAN2SFTP_FNAME_MAXLEN		(SCAN2FTP_FILE_STRMAX + 15)		/* ファイル名+"_"+6+"_"+JPEG連番(MAX3桁)+4(拡張子[.JPG]) */

#endif	/* USE_SCAN2SFTP */

/* 重送検知 */
#define	FTPNW_MULTIFEED_DETECT_ON	0x01
#define	FTPNW_MULTIFEED_DETECT_OFF	0x00
/* 白紙除去 */
#define	FTPNW_BLANK_DETECT_ON		0x01
#define	FTPNW_BLANK_DETECT_OFF		0x00
/* 斜行補正 */
#define	FTPNW_DESKEW_ON				0x01
#define	FTPNW_DESKEW_OFF			0x00

#define	FTPC_PGCNT_INVALID_VAL	(0xFFFF)	/**< 頁枚数 無効値 ftpclient_getScanPage - 現在の読取ページ総数と白紙検出ページ数 */

/* CIFS関連はCIFSライブラリのヘッダファイル（cifsclib.h）を使用 */

/*------------------------------------------------------------------------*/
/****** public structures *************************************************/
/*------------------------------------------------------------------------*/
/* スキャナメニューから受け取る情報 */
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
	SCAN2FTP_QUALITY		quality;										/* 画質                 */
	SCAN2FTP_FILEFORMAT		fileformat;										/* ファイルフォーマット */
	SCAN2FTP_FILESIZE		filesize;										/* ファイルサイズ       */
	SCAN2FTP_DUALSCAN		dualscan;										/* 両面読み取り         */
	SCAN2FTP_FNAMETYPE		filenametype;									/* 先頭文字列のタイプ   */
	UINT8					servername    [ SCAN2FTP_SVR_MAXLEN    +1 ];	/* FTP Server 名        */
	UINT8					serveraddress [ SCAN2FTP_SVRADR_MAXLEN +1 ];	/* FTP ServerAddress    */
	UINT8					username      [ SCAN2FTP_USER_MAXLEN   +1 ];	/* USER名               */
	UINT8					password      [ SCAN2FTP_PASSWD_MAXLEN +1 ];	/* PASSWORD             */
	UINT8					storedir      [ SCAN2FTP_STDIR_MAXLEN  +1 ];	/* 格納DIRECTORY        */
	UINT8					filename      [ SCAN2FTP_FNAME_MAXSIZE ];		/* ファイル名           */
	UINT8					spdfpass      [ FTPC_SPDF_PASSWD_MAXLEN+1 ];	/* Secure PDF PASSWORD  */
    UINT8                   ispassive;                                      /* Passive or active    */
    UINT32                  portnum;                                        /* ポート番号           */
    UINT8                   scan_quality;                                   /* スキャン圧縮率       */
#ifdef USE_TMP_SCANSIZE
	UINT8                   scan_doc_size;                                  /* Glass Scan Size      */
#endif /* USE_TMP_SCANSIZE */                                 /* スキャン圧縮率       */
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	UINT8               	scan_src;										/* 原稿読み取り位置		*/
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
	UINT8					scan_multifeed_detect;							/* 重送検知 ON/OFF */
	UINT8					scan_blank_detect;								/* 白紙除去機能 ON/OFF */ 
	UINT8					scan_deskew_adjust;								/* 斜行補正 ON/OFF */ 
	UINT8					scan_quality_gray;								/* 色数Auto時のGray圧縮率 */
#ifdef    USE_GNDCOLOR_REMOVAL
	UINT8					gndcolor_removal;								/* カラー地色除去		*//* M-BHS13-648 */
	UINT8					gndcolor_level;									/* カラー地色除去設定	*//* M-BHS13-648 */
#endif /* USE_GNDCOLOR_REMOVAL */
#ifdef	USE_SCAN2SFTP
	UINT8					AuthenticationMethod;							/* 認証方式 */
	INT32					PubKeyIdx;										/* Publib Key Index */
	INT32					PairKeyIdx;										/* MFC鍵Pair */
#endif	/* USE_SCAN2SFTP */
	UINT8					FileNameFixed;
}ACCESS_INFO;

typedef struct
{
	SCAN2FTP_QUALITY		quality;										/* 画質                 */
	SCAN2FTP_FILEFORMAT		fileformat;										/* ファイルフォーマット */
	SCAN2FTP_DUALSCAN		dualscan;										/* 両面読み取り         */
	SCAN2FTP_FNAMETYPE		filenametype;									/* 先頭文字列のタイプ   */
	UINT8					serveraddress [ CIFSCLIB_HOSTADDR_SIZE +1 ];	/* CIFS ServerAddress    */
	UINT8					username      [ CIFSCLIB_USERNAME_SIZE +1 ];	/* USER名               */
	UINT8					password      [ CIFSCLIB_PASSWORD_SIZE +1 ];	/* PASSWORD             */
	UINT8					storedir      [ CIFSCLIB_DIRECTORY_SIZE+1 ];	/* 格納DIRECTORY        */
	UINT8					filename      [ SCAN2FTP_FNAME_MAXSIZE ];		/* ファイル名           */
	UINT8					AuthenticationMethod;							/* 認証方法        */
	UINT8					KerberosServerAddress    [ CIFSCLIB_KERBADDR_SIZE+1 ];	/* ケルベロスサーバ名またはIPアドレス */
	UINT8					spdfpass      [ FTPC_SPDF_PASSWD_MAXLEN+1 ];	/* Secure PDF PASSWORD  */
    UINT8                   scan_quality;                                   /* スキャン圧縮率       */
#ifdef USE_TMP_SCANSIZE
	UINT8                   scan_doc_size;                                  /* Glass Scan Size      */
#endif /* USE_TMP_SCANSIZE */                                /* スキャン圧縮率       */
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	UINT8               	scan_src;										/* 原稿読み取り位置		*/
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
	UINT8					scan_multifeed_detect;							/* 重送検知 ON/OFF */
	UINT8					scan_blank_detect;								/* 白紙除去機能 ON/OFF */
	UINT8					scan_deskew_adjust;								/* 斜行補正 ON/OFF */
	UINT8					scan_quality_gray;								/* 色数Auto時のGray圧縮率 */
#ifdef    USE_GNDCOLOR_REMOVAL
	UINT8		gndcolor_removal;			/* カラー地色除去		*//* M-BHS13-648 */
	UINT8		gndcolor_level;				/* カラー地色除去設定	*//* M-BHS13-648 */
#endif /* USE_GNDCOLOR_REMOVAL */
	UINT8					FileNameFixed;
}CIFSACCESS_INFO;

/* メッセージ構造体 */
typedef struct
{
	UINT16			from_task;
	UINT16			cmd_id;
	UINT8			profilenumber;	/* 接続先profile number */
}ftpclt_msg;

/* FTP Clientタスク状態一覧 */
typedef enum
{
	FTPC_SCANNING = 401,			/* 読み取り中                 */
	FTPC_SELECTNEXT,				/* 次ページ読み込み選択中     */
	FTPC_WAITNEXT,					/* 次ページ読み込み決定       */
                                    /* SETキー待ち                */
	FTPC_SCANSTOP,					/* 読み取り中断               */
	FTPC_MEM_ERR,					/* メモリエラー発生状態       */
	FTPC_IDLE,						/* 待ち受け中                 */
    FTPC_START_WAIT,                /* 開始指示待ち               */
	FTPC_SEND,						/* ファイル送信中             */
#ifdef FTPC_COMPLETE_SEQUENCE
	FTPC_COMPLETE,					/* 完了画面表示中			  */
#endif /* FTPC_COMPLETE_SEQUENCE */
#ifdef FUNC_ADF_SCAN_SPEC_SHEETS_STOP
	FTPC_SCAN_LIMIT_STOPPING		/* 35枚連続での読取停止発生中 */
#endif /* FUNC_ADF_SCAN_SPEC_SHEETS_STOP */
}FTPSCAN_STATE;

/* ftpclient.c 内部情報へのアクセス */
typedef enum
{
	INTERNAL_READ = 411,			/* 内部情報を読み出す     */
	INTERNAL_WRITE					/* 内部情報を書き込む     */
}INTERNAL_ACCESS;

#ifdef USE_SEPARATE_UI
#define FTPC_UIEVT_OK	0x00000001	/* 完了画面Close OKキー押下 */
#define FTPC_UIEVT_STOP	0x00000002	/* 完了画面Close STOPキー押下 */
#endif /* #ifdef USE_SEPARATE_UI */
/* Scan to FTP,NWの自動確認用 このenum、状態と確認結果詳細を兼任していうので機会があれば、それぞれしっかりと分離した設計にしたい。 */
typedef enum {	
	SCAN_CHECK_INIT			= 0		/* 初期状態 */
	,SCAN_CHECK_BUSY				/* 書込中 */	
	,SCAN_CHECK_OK					/* 書込成功 */	
	,SCAN_CHECK_ERR_TOUT			/* 書込失敗:ServerTimeout */	
	,SCAN_CHECK_ERR_AUTH			/* 書込失敗:AuthenticationError */	
	,SCAN_CHECK_ERR_SEND			/* 書込失敗:SendError */	
	,SCAN_CHECK_ERR_WRDT			/* 書込失敗:WrongDate&Time */	
	,SCAN_CHECK_ERR_BUSY			/* 書込失敗:ftpscan_state != FTPC_START_WAIT（ftpc task is BUSY） */	
	/* ココより上に追加 */
	,SCAN_CHECK_MAX
}	SCAN_CHECK_RESULT;
#define	SCAN_CHECK_RESULT_RESET_TIME	1000	/* 10秒(10ms単位) */
#define	SCAN_CHECK_BUSSY_RESET_TIME		30000	/* 300秒(10ms単位) */
/* 
 *  SCAN_CHECK_BUSSY_RESET_TIME
 *  TCP TimeOutの時間は0～32767分まで設定できるが、
 *  32767分に設定してもSeverTimeoutの時間は180秒ほどで返って来ている。
 *  ServerTimeout時間より長く設定されていれば問題無いので
 *  TCP TimeOutの時間では無く固定値300秒で設定する。(M-DCSL-867)
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

/* Scan to FTP,NWの自動確認用 */
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
