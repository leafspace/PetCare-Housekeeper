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
#define	MSG_QUE_SIZE		(12)             /* 受信メッセージの最大長        */
#define	NO_SYSMEMID			(-1)             /* SYSMEM IDがない場合           */
#define	FTPFLNAME_MAX		(256)            /* ファイル名の最大文字数        */
#define SCAN2FTP_SERVICE    (0)              /* Service:Scan to FTP           */
#define SCAN2CIFS_SERVICE   (1)              /* Service:Scan to CIFS          */
#ifdef	USE_SCAN2SFTP
#define	SCAN2SFTP_SERVICE	(2)              /* Service:Secure Scan to FTP    */
#endif	/* USE_SCAN2SFTP */

#define JPEG_SCAN_CNT_SIZE	(10 + 1 + 1)
                                      /* 10(UINT32:10桁) + 1('_') + 1(\0)     */
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
#define	COMPLETE_TIMEOUT			6000		/* 60秒( 10ms×6000 ) */
#endif /* FTPC_COMPLETE_SEQUENCE */

/*----------------------------------------------------------------------------*/
/* STRUCTURES                                                                 */
/*----------------------------------------------------------------------------*/
typedef struct
{
	SCAN2FTP_QUALITY		quality;         /* (common) 画質                 */
	SCAN2FTP_FILEFORMAT		fileformat;      /* (common) ファイルフォーマット */
	SCAN2FTP_DUALSCAN		dualscan;        /* (common) 両面読み取り         */
	SCAN2FTP_FNAMETYPE		filenametype;    /* (common) 先頭文字列のタイプ   */
                                             /* (FTP)    FTP Server 名        */
	UINT8					servername   [ SCAN2FTP_SVR_MAXLEN      +1 ];
	                                         /* (common) サーバー名           */
	UINT8					serveraddress[ FTPCLIB_HOSTADDR_SIZE   +1 ];
                                             /* (common) CIFS ServerAddress   */
	UINT8					username     [ CIFSCLIB_USERNAME_SIZE  +1 ];
                                             /* (common) ユーザー名           */
	UINT8					password     [ CIFSCLIB_PASSWORD_SIZE  +1 ];
                                             /* (common) PASSWORD             */
	UINT8					storedir     [ CIFSCLIB_DIRECTORY_SIZE +1 ];
                                             /* (common) 格納DIRECTORY        */
	UINT8					filename     [ SCAN2FTP_FNAME_MAXSIZE];
                                             /* (common) ファイル名           */
	UINT8					AuthenticationMethod;
                                             /* (CIFS/SFTP)   認証方法             */
	UINT8					KerberosServerAddress
                                         [ CIFSCLIB_KERBADDR_SIZE  +1 ];
                                             /* (CIFS)   ケルベロスサーバ名   */
                                             /*          またはIPアドレス     */
    UINT8                   ispassive;       /* (FTP)    Passive or active    */
    UINT32                  portnum;         /* (FTP)    ポート番号           */
                                             /* SPDFパスワード                */
    UINT8                   spdfpass     [ FTPC_SPDF_PASSWD_MAXLEN +1 ];
    UINT8                   scan_quality;    /* Scan圧縮率                    */
#ifdef USE_TMP_SCANSIZE
	UINT8                   scan_doc_size;   /* (Common)Glass Scan Size       */
#endif /* USE_TMP_SCANSIZE */
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	UINT8               	scan_src;
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
	INT32					PubKeyIdx;
	INT32					PairKeyIdx;
#endif	/* USE_SCAN2SFTP */
	UINT8					FileNameFixed;
}FTP_CIFSACCESS_INFO;

/* 受信メッセージ用共用体 */
typedef union
{
	ftpclt_msg					ftpclt_tskmsg;
	macstatemsg_t				macstate_msg;
	scannertask_retun_msg_t		scan_msg;
}ftpclt_get_msg;

/**
* @struct	FTPC_PGCNT_ACT
* @par		Scan枚数カウンタ関数
* @par		ftpclient_ScanPage / ftpclient_BlankPage 第一引数の定義
*/
typedef enum {
	FTPC_PGCNT_INIT		= 1,	/* 初期化				*/
	FTPC_PGCNT_START,			/* 読み取り開始値の設定	*/
	FTPC_PGCNT_UNUSE,			/* ページカウントしない	*/
	FTPC_PGCNT_INCREMENT,		/* 1頁読み取り完了		*/
	FTPC_PGCNT_END,				/* 読み取り終了			*/
	FTPC_PGCNT_GET,				/* ページカウント取得	*/
} FTPC_PGCNT_ACT;


typedef struct
{
	SCAN2FTP_QUALITY		ftp_quality;			/* FTP Quality */
	UINT16					resolution;				/* doc_scan_area_spec.hで定義されている「解像度」*/
	UINT8					color_num;				/* doc_scan_area_spec.hで定義されている「色数」*/
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
#if 0	/* ADSでは400DPIが存在しない。必要になったときに有効にする */
				,{ FTP_Color400,		RESOLUTION_400DPI,		COLOR_MODE_COLOR	}
				,{ FTP_Gray400,			RESOLUTION_400DPI,		COLOR_MODE_GRAY		}
#endif
#if defined(USE_SCAN_AUTO_RESOLUTION)
				/* ColorAuto/GrayAutoについてはCOLOR_MODEとの紐つけが目的のため解像度Default値を返す */
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
STATIC	ACCESS_INFO         access_info;         /* Panel Menuの確定値(FTP)   */
STATIC	CIFSACCESS_INFO     access_info_cfs;     /* Panel Menuの確定値(CIFS)  */
STATIC  FTP_CIFSACCESS_INFO ftp_cifs_access_info;
                                               /* Panel Menuの確定値(FTP/CIFS)*/
STATIC	UINT32              jpeg_scan_counter; /* JPEGファイル初めの連続番号  */
STATIC  UINT32              service_kind;      /* FTP/CIFSサービス種別        */
STATIC  UINT8               servername_zl[ SCAN2FTP_SVR_MAXLEN  +1 ];
STATIC  UINT8               filename_zl  [ SCAN2FTP_FNAME_MAXSIZE ];
#ifndef	USE_SEPARATE_UI
#ifdef	GRAPHIC_LCD
STATIC	UINT32			    Quality_DSP;       /* 五行モデル用、              */
                                               /* Scanning表示中の解像度      */
#endif	/* GRAPHIC_LCD */
#endif	/* USE_SEPARATE_UI */

/* Scan to FTP,NWの自動確認用 */
STATIC SCAN_CHECK_RESULT	scan_check_result;							/* 接続Check結果格納用変数 */
STATIC UINT32				scan_check_result_time;						/* 接続Check時間結果格納用変数 [最小単位：10ms] */

#ifdef FTPC_COMPLETE_SEQUENCE
STATIC	UINT32				timer_id;	/* タイマー登録ID格納用 */
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
/* NextPageからの再開通知用Observer class */
STATIC CObserver_t *g_Observer = NULL;
STATIC CObserver_t *g_ObserverStop = NULL;
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

STATIC VOID	conv_scan_quality_to_color_reso(UINT16 quality,UINT8 *color, UINT16 *resolution);
STATIC VOID ftpclient_ScanPage( FTPC_PGCNT_ACT iAction, UINT32* ioPage );
STATIC VOID ftpclient_BlankPage( FTPC_PGCNT_ACT iAction, UINT32* ioPage );

STATIC VOID scan_check_setresult(const SCAN_CHECK_RESULT rslt);		/* 接続確認結果保存関数 */
STATIC void scanftp_connection_check(UINT8 profilenumber);			/* 接続確認関数 */
#ifdef	USE_SCAN2NW
STATIC	VOID	scannw_connection_check(UINT8 profilenumber);			/* 接続確認関数 */
#endif	/* USE_SCAN2NW */
#ifdef	USE_SCAN2SFTP
STATIC	void	scansftp_connection_setup(UINT8 profilenumber,stcSFTPConnect* sftp_conn);
STATIC	void	scansftp_connection_check(UINT8 profilenumber);			/* ScantoSFTP用接続確認関数 */
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
* @par	ファイルTransferタスク メイン関数
* @param	なし
* @return	なし
*
* @par <外部仕様>
*		Scan to FTPとScan to Networkをメニューから指定のProfileに関して実施する。
* @par <内部仕様>
*		指定のProfileに対しScanからのイメージ読込を行いファイル転送を実施する。
*
* @par <M票>
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

	/* 初期化                            */
	memset( &recv_msg, NULL, sizeof(ftpclt_get_msg) );
	message   = &(recv_msg.ftpclt_tskmsg);

	/* タスク初期化処理                  */
	ret_value = ftpc_init();
	if ( ret_value != OK ) {
		return;
	}

    /* タスク状態をパネルからの          */
    /* 指示待ちへ変更                    */
    ftpscan_state = FTPC_START_WAIT;

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	if (Serio_Is_Enabled() == TRUE) {
	    /* NextPageからの再開通知用のObserver classのインスタンスを作る */
	    if (! g_Observer) {
	        g_Observer = new( (void*)ctor(CObserver), sizeof(CObserver_t) );
	        if (! g_Observer) {
	            EPRINTF(("disp_nextexist - CObserver object instance doesn't get.\n"));
	            return;
	        }

	        /* Update関数の置き換え */
	        g_Observer->vptr_Init(g_Observer, UpdateFunc_FtpClient);
	    }

	    if (! g_ObserverStop) {
	        g_ObserverStop = new( (void*)ctor(CObserver), sizeof(CObserver_t) );
	        if (! g_ObserverStop) {
	            EPRINTF(("disp_nextexist - CObserver object instance doesn't get.\n"));
	            return;
	        }

	        /* Update関数の置き換え */
	        g_ObserverStop->vptr_Init(g_ObserverStop, UpdateFunc_FtpClient_StopScan);
	    }
	}
#endif	/* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

	/* メッセージ受信LOOP                */
	while (1) {
		/* メッセージ受信                */
		ret_value = FOS_MSGRECEIVE( qid_ftpclient, 
                                    (UINT8 *)(&recv_msg), sizeof(ftpclt_get_msg), WAIT_FOREVER );

		if ( ret_value == ERROR ) {
			continue;
		}
#ifdef DBG_RECEIVE_CMD
	DPRINTF(("[Ftpclient], ftpscan_state=%d, from_task=%d, cmd_id=%d\n", ftpscan_state, message->from_task, message->cmd_id));
#endif /* DBG_RECEIVE_CMD */

        /* タスク状態毎の処理            */
		switch ( ftpscan_state ) {
			/* 待ち受け中                */
			case FTPC_START_WAIT:
				if(CMD_SCAN_TO_FTPTEST_STA == message->cmd_id) {			/* Scan to FTP設定Check開始要求 */
					scanftp_connection_check(message->profilenumber);		/* サーバー接続確認 */
				}
#ifdef	USE_SCAN2NW
				else if(CMD_SCAN_TO_NWTEST_STA == message->cmd_id) {		/* Scan to NW設定Check開始要求 */
					scannw_connection_check(message->profilenumber);		/* サーバー接続確認 */
				}
#endif	/* USE_SCAN2NW */
#ifdef	USE_SCAN2SFTP
				else if(CMD_SCAN_TO_SFTPTEST_STA == message->cmd_id) {		/* Secure Scan to FTP設定Check開始要求 */
					scansftp_connection_check(message->profilenumber);
				}
#endif	/* USE_SCAN2SFTP */
				else {
					start_process(message->from_task, message->cmd_id, &image_id, &sysmem_id);
				}
				break;
			/* 読み取り&送信             */
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
				){	/* Scan to FTP,NW,SFTP設定Check開始要求 */
					/* 設定Check処理結果に「書込失敗:ftpscan_state != FTPC_START_WAIT」を設定 */
					scan_check_setresult(SCAN_CHECK_ERR_BUSY);
					break;
				} else {
					if (message->from_task == MACSTATUS_LIBRARY){
						message_state = &(recv_msg.macstate_msg);
						cmd_id        = message_state->stateid;
					}else{
						cmd_id        = message->cmd_id;
					}
					/* 読取り&送信状態に遷移 */
					scan_process (message->from_task, cmd_id,  image_id,  sysmem_id );
					if (ftpscan_state == FTPC_SEND){
					    /* 送信依頼          */
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
	            /* 期待しない状態            */
			default:
				if((CMD_SCAN_TO_FTPTEST_STA == message->cmd_id)
					||(CMD_SCAN_TO_NWTEST_STA == message->cmd_id)
#ifdef	USE_SCAN2SFTP
					||(CMD_SCAN_TO_SFTPTEST_STA == message->cmd_id)
#endif	/* USE_SCAN2SFTP */
				){	/* Scan to FTP,NW,SFTP設定Check開始要求 */
					/* 設定Check処理結果に「書込失敗:ftpscan_state != FTPC_START_WAIT」を設定 */
					scan_check_setresult(SCAN_CHECK_ERR_BUSY);
					break;
				}
                /* Error処理             */
                DPRINTF((" ftpc : Unexpected Status@ftpclient_main\n"));
				break;
        }
	}
}

/**
* @par	メニューで指定されたFTPのProfileの情報を取得する。
* @param	*uif_settings (input) Profile情報の構造体
* @return	なし
*
* @par <外部仕様>
*		メニューで指定されたProfileの各情報を本タスクのSTATICエリアに格納する。
* @par <内部仕様>
*		メニューで指定されたProfileの各情報とProfileに関連する必要な情報を不揮発エリア
*		から取得して本タスクのSTATICエリアに格納する。
*
* @par <M票>
*	   M-BCL-945
*/

#ifdef USE_SCAN2FTP
GLOBAL void
ftpclient_setaccessinfo( ACCESS_INFO *uif_settings )
{
	/* 初期化             */
	memset( &access_info, NULL,         sizeof(ACCESS_INFO) );
	memcpy( &access_info, uif_settings, sizeof(ACCESS_INFO) );
	memset( &ftp_cifs_access_info,
                          NULL,         sizeof(FTP_CIFSACCESS_INFO) );

    /* サービス種別の設定 */
    service_kind = SCAN2FTP_SERVICE;

    /* サーバー名/ファイル名の保持                         */
    strncpy( (MD_CHAR *)servername_zl,
             (MD_CHAR *)access_info.servername,        SCAN2FTP_SVR_MAXLEN      );
    strncpy( (MD_CHAR *)filename_zl,
             (MD_CHAR *)access_info.filename,          SCAN2FTP_FNAME_MAXLEN    );

	/* JPEGのファイル名は毎回変わってしまうため、          */
    /* ここで連続番号を計測しておく                        */
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


    /* Panel Menuの確定値(FTP/CIFS)に格納 */
    /* 画質                               */
    ftp_cifs_access_info.quality    = access_info.quality;
    /* ファイルフォーマット               */
    ftp_cifs_access_info.fileformat = access_info.fileformat;
    /* 両面読み取り                       */
    ftp_cifs_access_info.dualscan   = access_info.dualscan;
    /* 先頭文字列のタイプ                 */
    ftp_cifs_access_info.filenametype
                                    = access_info.filenametype;
    /* FTP Server 名                      */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.servername,
             (MD_CHAR *)access_info.servername,         SCAN2FTP_SVR_MAXLEN    );

    /* FTP ServerAddress                  */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.serveraddress,
             (MD_CHAR *)access_info.serveraddress,      SCAN2FTP_SVRADR_MAXLEN );

    /* USER名                             */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.username,
             (MD_CHAR *)access_info.username,           SCAN2FTP_USER_MAXLEN   );

	/* PASSWORD                           */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.password,
             (MD_CHAR *)access_info.password,           SCAN2FTP_PASSWD_MAXLEN );

    /* 格納DIRECTORY                      */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.storedir,
             (MD_CHAR *)access_info.storedir,           SCAN2FTP_STDIR_MAXLEN  );

    /* ファイル名                         */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.filename,
             (MD_CHAR *)access_info.filename,           SCAN2FTP_FNAME_MAXLEN  );

    /* Passive or Active                  */
    ftp_cifs_access_info.ispassive  = access_info.ispassive;

    /* Port番号                           */
    ftp_cifs_access_info.portnum    = access_info.portnum;

    /* Secure PDF パスワード              */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.spdfpass,
             (MD_CHAR *)access_info.spdfpass,           FTPC_SPDF_PASSWD_MAXLEN);

    /* Scan圧縮率                         */
    ftp_cifs_access_info.scan_quality = access_info.scan_quality;

#ifdef USE_TMP_SCANSIZE
    ftp_cifs_access_info.scan_doc_size = access_info.scan_doc_size;
#endif /* USE_TMP_SCANSIZE */

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	if (Serio_Is_Enabled() == TRUE) {
		ftp_cifs_access_info.scan_src = access_info.scan_src;
	}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

	/* 重送検知 */
	ftp_cifs_access_info.scan_multifeed_detect = access_info.scan_multifeed_detect;	/* 重送検知 ON/OFF */

	/*白紙除去機能 */
	ftp_cifs_access_info.scan_blank_detect = access_info.scan_blank_detect;			/* 白紙除去機能 ON/OFF */ 

	/* 斜行補正 */
	ftp_cifs_access_info.scan_deskew_adjust = access_info.scan_deskew_adjust;		/* 斜行補正 ON/OFF */ 

	/* 色数Auto時のGray圧縮率 */
	ftp_cifs_access_info.scan_quality_gray = access_info.scan_quality_gray;		/* 色数Auto時のGray圧縮率 */ 
	
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
* @par	メニューで指定されたCIFSのProfileの情報を取得する。
* @param	*uif_settings (input) Profile情報の構造体
* @return	なし
*
* @par <外部仕様>
*		メニューで指定されたProfileの各情報を本タスクのSTATICエリアに格納する。
* @par <内部仕様>
*		メニューで指定されたProfileの各情報とProfileに関連する必要な情報を不揮発エリア
*		から取得して本タスクのSTATICエリアに格納する。
*
* @par <M票>
*	   M-BCL-945
*/

GLOBAL void
cifsclient_setaccessinfo( CIFSACCESS_INFO *uif_settings )
{
	/* 初期化             */
	memset( &access_info_cfs, NULL,         sizeof(CIFSACCESS_INFO) );
	memcpy( &access_info_cfs, uif_settings, sizeof(CIFSACCESS_INFO) );
	memset( &ftp_cifs_access_info,
                              NULL,         sizeof(FTP_CIFSACCESS_INFO) );

    /* サービス種別の設定                                  */
    service_kind = SCAN2CIFS_SERVICE;

    /* サーバー名/ファイル名の保持                         */
    strncpy( (MD_CHAR *)servername_zl,
             (MD_CHAR *)access_info_cfs.serveraddress,
                                                    CIFSCLIB_HOSTADDR_SIZE);

    strncpy( (MD_CHAR *)filename_zl,
             (MD_CHAR *)access_info_cfs.filename,   SCAN2FTP_FNAME_MAXSIZE );

	/* JPEGのファイル名は毎回変わってしまうため、          */
    /* ここで連続番号を計測しておく                        */
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

    /* Panel Menuの確定値(FTP/CIFS)に格納 */
    /* 画質                               */
    ftp_cifs_access_info.quality    = access_info_cfs.quality;
    /* ファイルフォーマット               */
    ftp_cifs_access_info.fileformat = access_info_cfs.fileformat;
    /* 両面読み取り                       */
    ftp_cifs_access_info.dualscan   = access_info_cfs.dualscan;
    /* 先頭文字列のタイプ                 */
    ftp_cifs_access_info.filenametype
                                    = access_info_cfs.filenametype;

    /* CIFS ServerAddress                 */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.serveraddress,
             (MD_CHAR *)access_info_cfs.serveraddress,  CIFSCLIB_HOSTADDR_SIZE   );

    /* USER名                             */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.username,
             (MD_CHAR *)access_info_cfs.username,       CIFSCLIB_USERNAME_SIZE   );

	/* PASSWORD                           */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.password,
             (MD_CHAR *)access_info_cfs.password,       CIFSCLIB_PASSWORD_SIZE   );

    /* 格納DIRECTORY                      */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.storedir,
             (MD_CHAR *)access_info_cfs.storedir,       CIFSCLIB_DIRECTORY_SIZE  );

    /* ファイル名                         */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.filename,
             (MD_CHAR *)access_info_cfs.filename,       SCAN2FTP_FNAME_MAXSIZE   );

    /* 認証方法                           */
    ftp_cifs_access_info.AuthenticationMethod =
                                      access_info_cfs.AuthenticationMethod;

    /* ケルベロスサーバ名またはIPアドレス */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.KerberosServerAddress,
             (MD_CHAR *)access_info_cfs.KerberosServerAddress, 
                                                        CIFSCLIB_KERBADDR_SIZE   );

    /* Secure PDF パスワード              */
    strncpy( (MD_CHAR *)ftp_cifs_access_info.spdfpass,
             (MD_CHAR *)access_info_cfs.spdfpass,       FTPC_SPDF_PASSWD_MAXLEN  );

    /* Scan圧縮率                         */
    ftp_cifs_access_info.scan_quality = access_info_cfs.scan_quality;

#ifdef USE_TMP_SCANSIZE
    ftp_cifs_access_info.scan_doc_size = access_info_cfs.scan_doc_size;
#endif /* USE_TMP_SCANSIZE */

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	if (Serio_Is_Enabled() == TRUE) {
		ftp_cifs_access_info.scan_src = access_info_cfs.scan_src;
	}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

	/* 重送検知 */
	ftp_cifs_access_info.scan_multifeed_detect = access_info_cfs.scan_multifeed_detect;	/* 重送検知 ON/OFF */

	/*白紙除去機能 */
	ftp_cifs_access_info.scan_blank_detect = access_info_cfs.scan_blank_detect;			/* 白紙除去機能 ON/OFF */ 

	/* 斜行補正 */
	ftp_cifs_access_info.scan_deskew_adjust = access_info_cfs.scan_deskew_adjust;		/* 斜行補正 ON/OFF */ 

	/* 色数Auto時のGray圧縮率 */
	ftp_cifs_access_info.scan_quality_gray = access_info_cfs.scan_quality_gray;		/* 色数Auto時のGray圧縮率 */ 

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
* @par	メニューで指定されたSFTPのProfileの情報を取得する。
* @param	*uif_settings (input) Profile情報の構造体
* @return	なし
*
* @par <外部仕様>
*		メニューで指定されたProfileの各情報を本タスクのSTATICエリアに格納する。
* @par <内部仕様>
*		メニューで指定されたProfileの各情報とProfileに関連する必要な情報を不揮発エリア
*		から取得して本タスクのSTATICエリアに格納する。
*
* @par <M票>
*	   
*/
GLOBAL void
sftpclient_setaccessinfo( ACCESS_INFO *uif_settings )
{
	/* 初期化 */
	(void)memset( &access_info, NULL, sizeof(ACCESS_INFO) );
	(void)memcpy( &access_info, uif_settings, sizeof(ACCESS_INFO) );
	(void)memset( &ftp_cifs_access_info, NULL, sizeof(FTP_CIFSACCESS_INFO) );

	/* Service種別設定 */
	service_kind = SCAN2SFTP_SERVICE;

	/* サーバー名保持 */
	(void)strncpy( (MD_CHAR*)servername_zl, (MD_CHAR*)access_info.servername, SCAN2SFTP_SVR_MAXLEN );
	/* ファイル名保持 */
	(void)strncpy( (MD_CHAR*)filename_zl, (MD_CHAR*)access_info.filename, SCAN2SFTP_FNAME_MAXLEN );
	/* JPEGのFile名は毎回変化するため、ここで連続番号を計測 */
	if( uif_settings->fileformat == FTP_JPEG ) {
		(void)scan_counter( &jpeg_scan_counter );
	}

	/* Panel Menuの確定値(FTP/CIFS/SFTP)に格納 */
	/* 1.画質 */
	ftp_cifs_access_info.quality = access_info.quality;
	/* 2.ファイルフォーマット */
	ftp_cifs_access_info.fileformat = access_info.fileformat;
	/* 3.両面読取 */
	ftp_cifs_access_info.dualscan = access_info.dualscan;
	/* 4.先頭文字列のタイプ */
	ftp_cifs_access_info.filenametype = access_info.filenametype;
	/* 5.SFTP Server名 */
	(void)strncpy( (MD_CHAR*)ftp_cifs_access_info.servername, (MD_CHAR*)access_info.servername, SCAN2SFTP_SVR_MAXLEN );
	/* 6.FTP Server Address */
	(void)strncpy( (MD_CHAR*)ftp_cifs_access_info.serveraddress, (MD_CHAR*)access_info.serveraddress, SCAN2SFTP_SVRADR_MAXLEN );
	/* 7.User名 */
	(void)strncpy( (MD_CHAR*)ftp_cifs_access_info.username, (MD_CHAR*)access_info.username, SCAN2SFTP_USER_MAXLEN );
	/* 8.Password */
	(void)strncpy( (MD_CHAR*)ftp_cifs_access_info.password, (MD_CHAR*)access_info.password, SCAN2SFTP_PASSWD_MAXLEN );
	/* 9.格納Directory */
	(void)strncpy( (MD_CHAR*)ftp_cifs_access_info.storedir, (MD_CHAR*)access_info.storedir, SCAN2SFTP_STDIR_MAXLEN );
	/* 10.ファイル名 */
	(void)strncpy( (MD_CHAR*)ftp_cifs_access_info.filename, (MD_CHAR*)access_info.filename, SCAN2SFTP_FNAME_MAXLEN );
	/* 11.ポート番号 */
	ftp_cifs_access_info.portnum = access_info.portnum;
	/* 12.Secure PDFパスワード */
	(void)strncpy( (MD_CHAR*)ftp_cifs_access_info.spdfpass, (MD_CHAR*)access_info.spdfpass, FTPC_SPDF_PASSWD_MAXLEN );
	/* 13.スキャン圧縮率 */
	ftp_cifs_access_info.scan_quality = access_info.scan_quality;
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	/* 14.原稿読み取り位置 */
	if( Serio_Is_Enabled() == TRUE ) {
		ftp_cifs_access_info.scan_src = access_info.scan_src;
	}
#endif	/* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
	/* 14.Document Size */
	ftp_cifs_access_info.scan_doc_size = access_info.scan_doc_size;
	/* 15.重送検知 */
	ftp_cifs_access_info.scan_multifeed_detect = access_info.scan_multifeed_detect;
	/* 16.白紙除去 */
	ftp_cifs_access_info.scan_blank_detect = access_info.scan_blank_detect;
	/* 17.斜傾補正 */
	ftp_cifs_access_info.scan_deskew_adjust = access_info.scan_deskew_adjust;
	/* 18.色数Auto時のGray圧縮率 */
	ftp_cifs_access_info.scan_quality_gray = access_info.scan_quality_gray;
	/* 19.地色補正 */
	ftp_cifs_access_info.gndcolor_removal = access_info.gndcolor_removal;
	ftp_cifs_access_info.gndcolor_level = access_info.gndcolor_level;
	/* 20.User認証方式 */
	ftp_cifs_access_info.AuthenticationMethod = access_info.AuthenticationMethod;
	/* 21.SFTP Server公開鍵 */
	ftp_cifs_access_info.PubKeyIdx = access_info.PubKeyIdx;
	/* 22.MFC鍵pairのID */
	ftp_cifs_access_info.PairKeyIdx = access_info.PairKeyIdx;

	ftp_cifs_access_info.FileNameFixed = access_info.FileNameFixed;
	return;

}
#endif	/* USE_SCAN2SFTP */

/**
* @par	本タスクの状態をRead/Writeする。（LCD表示用に公開）
* @param	*state (input/output) Readした状態／Writeする状態
* @param	access_type (input) Read/Write種別
* @return	なし
*
* @par <外部仕様>
*		本タスクの状態をSTATIC領域にRead/Writeする。
* @par <内部仕様>
*		本タスクの状態をSTATIC領域にRead/Writeする。
*
* @par <M票>
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
* @par	ファイルTransferタスク 初期化関数
* @param	なし
* @return	OK：正常終了
*			ERROR：異常終了
*
* @par <外部仕様>
*		本タスクが関連する初期化を行う。
* @par <内部仕様>
*		タスク状態の初期化、自タスク起動、キー入力の初期化を行う。
*
* @par <M票>
*	   M-BCL-945
*/

STATIC INT32
ftpc_init( void )
{
	/* 宣言                          */
	INT32 ret_value;

	/* 初期化                        */
	ret_value     = OK;

    /* アプリ状態を INIT にする      */
	manaplset( S_APL_FTPCLIENT, APL_INIT );

	/* 関数外変数の初期化            */
	scan_src      = SCAN_SRC_ADF;
	qid_scanbase  = FOS_MSGGETID( SCAN_MSG_NAME );

	/* タスク起動                    */
	qid_ftpclient = FOS_MSGCREATE( FTPC_MSG_NAME, FTPC_MSG_COUNT, sizeof(ftpclt_get_msg) );
	ftpscan_state = FTPC_IDLE;
	memset( &access_info,          NULL, sizeof(ACCESS_INFO) );
	memset( &access_info_cfs,      NULL, sizeof(CIFSACCESS_INFO) );
	memset( &ftp_cifs_access_info, NULL, sizeof(FTP_CIFSACCESS_INFO) );

	/* 機能がない場合は何もしない    */
	if( (ModelFuncGet(MODEL_FUNC_SCAN2FTP, MODEL_FUNC_REF_IOFSW) == FALSE) && 
	    (ModelFuncGet(MODEL_FUNC_SCAN2NW,  MODEL_FUNC_REF_IOFSW) == FALSE)	 ) {
		ret_value = ERROR;
	}

    /* LAN機能がない場合は何もしない */
	if(ModelFuncGet(MODEL_FUNC_NETWORK,       MODEL_FUNC_REF_IOFSW) == FALSE ) {
		ret_value = ERROR;
    }

#if !defined(ONCHIP_LAN) && !defined(USE_OPT_LAN)
	/* LANボードが装着されていなければなにもしない */
	if ( aioChkBoard() != OK ) {
		ret_value = ERROR;
	}
#endif

#ifndef	USE_SEPARATE_UI
	ftpc_keyin_init();
#endif	/* USE_SEPARATE_UI */

	/* 設定Check処理結果に「初期状態」を設定 */
	scan_check_setresult(SCAN_CHECK_INIT);

	/* 読み取り枚数/白紙検出枚数を初期化 */
	ftpclient_ScanPage(FTPC_PGCNT_INIT, NULL);
	ftpclient_BlankPage(FTPC_PGCNT_INIT, NULL);

#ifdef FTPC_COMPLETE_SEQUENCE
	timer_id	= 0;		/* タイマー登録ID格納用変数の初期化 */
#endif /* FTPC_COMPLETE_SEQUENCE */

	/* アプリ状態を READY にする     */
	manaplset( S_APL_FTPCLIENT, APL_READY );

	return ( ret_value );
}

/**
* @par	スキャナから読取を開始する
* @param	from_task (input) メッセージ送信元タスクID
* @param	cmd_id (input) メッセージ内のコマンドID（メッセージ送信元タスクの場合は装置状態）
* @param	*image_id (output) 画データID
* @param	*sysmem_id (output) システムメモリID
* @return	なし
*
* @par <外部仕様>
*		サーバーの接続確認を行ない、スキャナから読取を開始する。
* @par <内部仕様>
*		サーバーの接続確認を行ない、リソースの確保とスキャナから読取を行うパラメータを設定し
*		開始要求を送信する。
*
* @par <M票>
*	   M-BCL-945
*/

STATIC void
start_process( UINT16 from_task, UINT16 cmd_id, INT32 *image_id, INT32 *sysmem_id )
{
	INT32 ret_value;

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	if (Serio_Is_Enabled() == TRUE) {
		/* NextPage通知のObserver登録 */
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

		/* 読み取り枚数/白紙検出枚数を0に初期化 */
	    ftpclient_ScanPage(FTPC_PGCNT_START, NULL);
		ftpclient_BlankPage(FTPC_PGCNT_START, NULL);
	
    	/* サーバーの接続確認           */
	    ret_value     = scan_prepare( image_id, sysmem_id );
    	if ( ret_value != OK ) {
#ifdef	USE_SEPARATE_UI
			cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif
	    	return;
    	}
    	
        /* タスクの状態をスキャニングへ */
    	ftpscan_state = FTPC_SCANNING;
	    /* スキャンスタート処理         */
	    scan_start( *image_id, *sysmem_id );
    }
}

/**
* @par	スキャン開始の準備を行う。
* @param	*image_id (output) 画データID
* @param	*sysmem_id (output) システムメモリID
* @return	OK：正常終了
*			ERROR：異常終了
*
* @par <外部仕様>
*      スキャン開始の準備の為のリソース確保を行う。
* @par <内部仕様>
*      スキャン開始の準備の為のリソース確保を行い、LCDの表示を更新する。
*
* @par <M票>
*	   M-BCL-945
*/

STATIC INT32
scan_prepare( INT32 *image_id, INT32 *sysmem_id )
{
    INT32			ret_value;
    INT32			ret_value_ftp;
    INT32			ret_value_cifs;
    UINT32			str_id;       /* エラー表示する文字列     */
    UINT32			image_use;    /* 画データ利用目的         */
    stcFTPConnect   ftp_conn;     /* ftp接続情報構造体        */
    stcCIFSConnect  cifs_conn;    /* CIFS接続情報構造体       */
#ifdef	USE_SCAN2SFTP
    INT32			ret_value_sftp;
	stcSFTPConnect	sftp_conn;	 	/* sftp接続情報構造体 */
#endif	/* USE_SCAN2SFTP */
#ifdef USE_SCAN_COLOR_DETECT /* 色数自動判別 */
	UINT8			color_num;
#endif /* USE_SCAN_COLOR_DETECT */

#ifndef	USE_SEPARATE_UI
#ifdef	LCD_5LINE
    /* PIN_NO_DSP(翻訳語Pin No:XXXX)を、                      */
    /* LCD表示するためのパラメータ                            */
    pstring_t		str_st;
#endif	/* LCD_5LINE */
#endif	/* USE_SEPARATE_UI */

	/* 初期化                 */
    ret_value           = ERROR;
    ret_value_ftp       = ERROR;
    ret_value_cifs      = ERROR;
#ifdef	USE_SCAN2SFTP
    ret_value_sftp      = ERROR;
#endif	/* USE_SCAN2SFTP */
#ifdef	USE_SEPARATE_UI
    str_id              = OK; /* QAC対策：実際に使用されることはない */
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
		/* 実行中通知 */
		SendJobStatus(SERIOFW_JOBSTS_PROCESSING, SERIO_JOB_SCANSEND);
	}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

#ifndef	USE_SEPARATE_UI
    /* パネル入力・表示権確保                                 */
    ret_value		    = ftpc_key_rightchg( GET_RIGHT );
    if ( ret_value != OK ) {
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, ERROR);
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
        return ( ERROR );
    }
#endif	/* USE_SEPARATE_UI */

    /*・サーバーの接続確認は上位関数のスキャナー読取開始      */
    /*  (scan_start)で実施する。                              */
    /*・連続でのLCDの表示消去                                 */
    /*  ftpc_disp_string( DUMMY_LINE3, NULL );                */
    /*  ftpc_disp_string( DUMMY_LINE4, NULL );                */
    /*  ftpc_disp_string( DUMMY_LINE5, NULL );                */
    /*  を共通関数（ftpc_disp_string_NULL）で実施する。       */



#ifndef	USE_SEPARATE_UI
#ifdef LCD_5LINE
	/* ファイルフォーマット用の任意文字列の初期化             */
	memset( &str_st, NULL, sizeof(pstring_t) );

	/* 翻訳語からファイルフォーマット用の任意文字列へ変換する */
    lcd_strcpy(str_st.str_data, Quality_DSP);
#endif	/* LCD_5LINE */

	/* 全体表示：Connecting                                   */
#ifdef LCD_5LINE
    /* 一行目                                                 */
    if (service_kind      == SCAN2FTP_SERVICE){
        ftpc_disp_string( SCANTO_FTP_DSP    ,  NULL );
    }else if(service_kind == SCAN2CIFS_SERVICE){
        ftpc_disp_string( SCANTO_NETWORK_DSP,  NULL );
    }
#else	/* LCD_2LINE                                          */
    /* 一行目                                                 */
	ftpc_disp_string( SERVER_CNCT_DSP, NULL );
#endif	/* LCD_5LINE LCD_2LINE                                */

	/* 表示                                                   */
    /* 二行目                                                 */
	ftpc_disp_string( DUMMY_LINE2, servername_zl );

	/* 五行表示用 3,4,5行目                                   */
#ifdef LCD_5LINE

	/* 表示                                                   */
	ftpc_disp_string( DUMMY_LINE3, str_st.str_data );
	ftpc_disp_string( DUMMY_LINE4, filename_zl );
	ftpc_disp_string( SERVER_CNCT_DSP, NULL );

	/* 両面読取の場合はＤアイコンも表示                       */
	dispDIconFor5Line();

#endif	/* LCD_5LINE */
#else	/* USE_SEPARATE_UI */
	/* CPから呼び出せる領域にファイル名を設定する */
	memcpy(Scan2Ftp_SendFileName, ftp_cifs_access_info.filename, SCAN2FTP_FNAME_MAXSIZE);
	cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_CONNECT );
#endif	/* USE_SEPARATE_UI */

    /* ServiceがScan To FTPの場合                             */
    if (service_kind == SCAN2FTP_SERVICE){
#ifdef USE_SCAN2FTP
        /* FTPホスト接続情報設定                              */
        /* パッシブモード or アクティブモード                 */
        ftp_conn.IsPassive      = ftp_cifs_access_info.ispassive;

        /* Port番号                                           */
        ftp_conn.PortNumber     = ftp_cifs_access_info.portnum;

        /* FTPホスト名またはIPアドレス                        */
        ftp_conn.HostAddress    = ftp_cifs_access_info.serveraddress;

        /* ファイル保存先フォルダ                             */
        ftp_conn.StoreDirectory = ftp_cifs_access_info.storedir;

        /* 認証のためのユーザー名                             */
        ftp_conn.UserName       = ftp_cifs_access_info.username;

        /* 認証のためのパスワード                             */
        ftp_conn.Password       = ftp_cifs_access_info.password;

        /* FTPホスト接続確認                                  */
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
    /* ServiceがScan To CIFSの場合                            */
    }else if(service_kind == SCAN2CIFS_SERVICE){
#ifdef	USE_SCAN2NW
        /* CIFSホスト接続情報設定                             */
        /* CIFSホスト名またはIPアドレス                       */
        cifs_conn.HostAddress    = ftp_cifs_access_info.serveraddress;

        /* ファイル保存先フォルダ                             */
        cifs_conn.StoreDirectory = ftp_cifs_access_info.storedir;

        /* 認証のためのユーザー名                             */
        cifs_conn.UserName       = ftp_cifs_access_info.username;

        /* 認証のためのパスワード                             */
        cifs_conn.Password       = ftp_cifs_access_info.password;

        /* 認証方法                                           */
        cifs_conn.AuthenticationMethod
                                 = ftp_cifs_access_info.AuthenticationMethod;

        /* ケルベロスサーバ名およびIPアドレス                 */
        cifs_conn.kerberosServerAddress
                                 = ftp_cifs_access_info.KerberosServerAddress;

        /* CIFSホスト接続確認                                 */
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
		/* 2.Port番号 */
		sftp_conn.PortNumber = ftp_cifs_access_info.portnum;
		/* 3.User認証方式 */
		sftp_conn.AuthMeth = ftp_cifs_access_info.AuthenticationMethod;
		/* 4.User名 */
		sftp_conn.UserName = ftp_cifs_access_info.username;
		/* 5.Password */
		sftp_conn.Password = ftp_cifs_access_info.password;
		/* 6.SFTPサーバ公開鍵 */
		sftp_conn.PubKeyIdx =  ftp_cifs_access_info.PubKeyIdx;
		/* 7.MFC鍵pairのID */
		sftp_conn.PairKeyIdx = ftp_cifs_access_info.PairKeyIdx;
		/* 8.Store Directory */
		sftp_conn.StoreDirectory = ftp_cifs_access_info.storedir;

		/* SFTPホスト接続確認 */
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
		/* BSI制限Userの最大保存Log数を超えているかの確認 */
		if(SerioLog_Check_MaxLog_Already() != OK)
		{
			/* Scan開始判定をERRORで上書きする */
			ret_value  = ERROR;
		}
	}
#endif	/* defined(USE_BSI) && defined(USE_SERIO_LOG) */

	if ( ret_value != OK ) {
		/* スキャンでの受付音終了を待つため                   */
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
		/* 全体表示：SendingErr など                          */
		ftpc_disp_string( str_id, NULL );
#ifdef LCD_5LINE
		/* 五行表示 */
		ftpc_disp_icon( ERASE_OBJ );
		ftpc_disp_line();
        ftpc_disp_string_NULL( NULL_LINE3 | NULL_LINE4 | NULL_LINE5 );
#else	/* LCD_5LINE */
        /* 二行目は空行とする(Server名は表示しない)           */
		ftpc_disp_string( DUMMY_LINE2, NULL );	
#endif /* LCD_5LINE */
#else	/* USE_SEPARATE_UI */
		cp_Sts_Apl_Entry( CP_STS_APL_FTPC, (UINT8)str_id );
#endif	/* USE_SEPARATE_UI */

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		if (Serio_Is_Enabled() == TRUE) {
			/* 実行停止通知 */
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

	/* ADFリソースの獲得                                      */
	if ( RES_OK != resourceforceget( RES_ADF, qid_ftpclient ) ) {
#if defined(TP_SIZE_37) && defined(USE_SEPARATE_UI)
		/* スキャンでの受付音終了を待つため                   */
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
		/* デフォルトモード復帰用タイマセット                 */
		set_default_mode_ret( DM_SCAN_APL_END, DM_RET_ON );
#endif
#endif	/* USE_SEPARATE_UI */
		return ( ERROR );
	}

	/* VIDEOリソースの獲得                                    */
	if ( RES_OK != resourceforceget( RES_SCAN_VIDEO, qid_ftpclient ) ) {
#if defined(TP_SIZE_37) && defined(USE_SEPARATE_UI)
		/* スキャンでの受付音終了を待つため                   */
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
		/* デフォルトモード復帰用タイマセット                 */
		set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
		return ( ERROR );
	}

	/* アプリ状態を設定                                       */
	manaplset( S_APL_FTPCLIENT, ADF_STOP_KEY_OK+VIDEO_STOP_KEY_OK );

	/* Connecting 表示変更                                    */
#ifdef FB_SCAN_TYPE
#ifndef	USE_SEPARATE_UI
	/* Connecting -> Scanning 表示                        */
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
	/* 両面読取の場合はＤアイコンを表示                       */
	dispDIconFor5Line();
#endif	/* USE_SEPARATE_UI */

	/* 画データファイル作成                                   */
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
    /* イメージをライブラリエントリ実施                       */
	*image_id      = ImageEntry( IMAGE_FILE, image_use );

	if ( *image_id == ERROR ) {

		/* M-AC-2558 Image Entryできないときの                */
        /* Out of Memory <20061127>                           */
#ifndef	USE_SEPARATE_UI
		/* スキャンでの受付音終了を待つため                   */
		FOS_TASKSLEEP( 20 );
		buzzer_refusal();
		ftpc_disp_string( MEMFUL_DSP, NULL );
#ifdef	LCD_5LINE
		ftpc_disp_icon( ERASE_OBJ );
		ftpc_disp_line();
        /* Error表示時は３～５行目を消去する                  */
        ftpc_disp_string_NULL( NULL_LINE3 | NULL_LINE4 | NULL_LINE5 );
#else	/* LCD_5LINE */
		ftpc_disp_string( DUMMY_LINE2, NULL );
#endif
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		/* 実行停止通知 */
		SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL, SERIO_JOB_SCANSEND);
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
#ifdef	STATUS_LED
		/* memory full 時は LED を赤色点灯                    */
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
		/* スキャンでの受付音終了を待つため                   */
		FOS_TASKSLEEP( 40 );
		soundBuzzerStop(BUZZER_ACCEPT);
		buzzer_refusal();
#endif	/* USE_SEPARATE_UI */

		resource_release( NO_SYSMEMID );
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
		/* デフォルトモード復帰用タイマセット                 */
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

#ifdef USE_SCAN_COLOR_DETECT /* 色数自動判別 */
	conv_scan_quality_to_color_reso(ftp_cifs_access_info.quality, &color_num, NULL);
	if( color_num == COLOR_MODE_AUTO )
	{
		ImageSetParameter( *image_id, IMAGE_COMP, IMAGE_COMP_NO ); /* 色数Auto時、圧縮設定は行わない */
	}
	else
#endif /* USE_SCAN_COLOR_DETECT  色数自動判別 */
	{
		if ( ftp_cifs_access_info.scan_quality == P_SCAN_QUAL_NORMAL ) {
			/* JPEG File Size = Sのときは圧縮設定を行う */
			ImageSetParameter( *image_id, IMAGE_COMP, IMAGE_COMP_FLATE );
		} else {
			/* 条件を満たさない場合は圧縮設定無しを設定 */
			ImageSetParameter( *image_id, IMAGE_COMP, IMAGE_COMP_NO );
		}
	}

    /* SPDFの時、パスワードを追加する                         */
    if ( ftp_cifs_access_info.fileformat == FTP_SPDF ){
        ImageSetParameter(*image_id, IMAGE_PDF_PASSWORD, 
                          (UINT32)ftp_cifs_access_info.spdfpass);
    }
	/* シスメム確保                                           */
    *sysmem_id = scan_get_sysmid_try(	conv_ftp_e2p_code(ftp_cifs_access_info.quality, QUALITY_TBL_FTP_TO_E2P),
#ifdef	USE_TMP_SCANSIZE
    									ftp_cifs_access_info.scan_doc_size,
#else
    									0,
#endif
    									conv_ftp_e2p_code(ftp_cifs_access_info.dualscan, DUALSCAN_TBL_FTP_TO_E2P),
    									scanmenu_conv_quality_to_rate(ftp_cifs_access_info.scan_quality),
    									image_use,
#ifdef USE_SCAN_BLANK_DETECT	/*白紙除去機能 */
    									conv_ftp_e2p_code(ftp_cifs_access_info.scan_blank_detect, BLANKP_TBL_FTP_TO_E2P),
#else /* USE_SCAN_BLANK_DETECT */
										USW_SCAN_BLNKP_DETECT_OFF,
#endif /* USE_SCAN_BLANK_DETECT */
#ifdef USE_SKEW_ADJUST	/* 斜行補正 */
										conv_ftp_e2p_code(ftp_cifs_access_info.scan_deskew_adjust, DESKEW_TBL_FTP_TO_E2P)
#else /* USE_SKEW_ADJUST */
										USW_SCAN_DESKEW_OFF
#endif /* USE_SKEW_ADJUST */
									);

	if ( *sysmem_id == ERROR ) {
		/* memory full 時のエラー処理                         */

#ifndef	USE_SEPARATE_UI
		/* memory full 表示、エラー音、LED点灯など            */
#ifdef USE_FAX
		if( ( resourceref(RES_MODEM) != 0 )	/*FAX送受信中?*/
#ifdef USE_T38
			|| ( t38fa_use_t38_line() == YES )	/*	T38通信中？	*/
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
        /* Error表示時は３～５行目を消去する                  */
        ftpc_disp_string_NULL( NULL_LINE3 | NULL_LINE4 | NULL_LINE5 );
#else	/* LCD_5LINE */
		ftpc_disp_string( DUMMY_LINE2, NULL );	
#endif
		/* スキャンでの受付音終了を待つため                   */
		FOS_TASKSLEEP( 20 );
		buzzer_refusal();
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		/* 実行停止通知 */
		SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL, SERIO_JOB_SCANSEND);
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
#ifdef	STATUS_LED
		/* memory full 時は LED を赤色点灯                    */
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
		/* スキャンでの受付音終了を待つため                   */
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
		/* デフォルトモード復帰用タイマセット                 */
		set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
		return ( ERROR );
	}

	return ( OK );
}


/**
* @par	スキャン開始
* @param	image_id (input) 画データID
* @param	sysmem_id (input) システムメモリID
* @return	なし
*
* @par <外部仕様>
*		スキャンを開始する為のパラメータ設定とスキャン開始要求を行う。
* @par <内部仕様>
*		スキャンを開始する為のパラメータ設定とスキャン開始要求をScan Base Taskに
*		メッセージ送信する。
*
* @par <M票>
*	   M-BCL-945
*/

STATIC void
scan_start( INT32 image_id, INT32 sysmem_id )
{
	UINT8                  i;
    /* スキャン動作モード           */
	UINT8                  apli_mode; 
    /* モノクロ送信設定のパラメータ */
	mult_scan_t            info;
    /* スキャンパラメータ           */
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
	
	/* 初期化           */
	apli_mode					= MEM_SCAN_FAX;
	memset( &info,       NULL, sizeof(mult_scan_t) );
	memset( &scan_param, NULL, sizeof(scannertask_cntl_msg_t) );

	/* 原稿 ADF or FB の設定、                        */
    /*ここでの設定値はスキャン終了まで保持する        */
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
			/* ADF指定時に、ADFに原稿が無い場合はFBに指定しなおす */
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

	/* モノクロ送信設定 */
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

	/* スキャンパラメータ設定１  デフォルト値のセット */
	GetScanModeDefault( (scantask_set_msg *)&(scan_param.item.scan_set) );

	/* スキャンパラメータ設定２  固定値のセット       */
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
	
	/* 用紙サイズ取得   */
#ifdef USE_SKEW_ADJUST
	/* 斜行補正がONのとき、原稿サイズをAutoの設定にする */
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

	/* スキャンパラメータ設定３  解像度依存値のセット */
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
			/* Scan to FTPもScan to Networkもこのルートで処理される */
			if(scanmenu_EiScanGetResolution(SCAN_APP_FTP, ftp_cifs_access_info.quality, ftp_cifs_access_info.fileformat, ftp_cifs_access_info.scan_quality, &doc_scan_area_spec_quality) != OK)
			{
				EPRINTF(("%s(%d)	scanmenu_EiScanGetResolution ERROR!!\n",__FILE__,__LINE__));
			}
			else
			{
				/* Sending表示時に実際の解像度表示を行うため、tmpの内容を更新する */
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
						/* 想定外の場合はDefaultとしてBW200100を入れておく */
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
	/* 主走査方向の読取範囲の画素数取得               */
	scan_param.item.scan_set.scan_area_x    =         GetDocScanAreaSizeX(doc_scan_area_spec_quality, doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* 副走査方向の読取範囲のライン数取得             */
	scan_param.item.scan_set.scan_area_y    =         GetDocScanAreaSizeY(doc_scan_area_spec_quality,  doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* 主走査方向の読取範囲の左端余白取得             */
	scan_param.item.scan_set.scan_x_offset  =         GetDocScanOffsetX  (doc_scan_area_spec_quality,  doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* 副走査方向の読取範囲の先端余白                 */
	scan_param.item.scan_set.scan_y_offset  =         GetDocScanOffsetY  (doc_scan_area_spec_quality, doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* 主走査方向の読取範囲の解像度取得               */
	scan_param.item.scan_set.scan_outreso_x =         GetDocScanResoX    (doc_scan_area_spec_quality, doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* 副走査方向の読取範囲の解像度取得               */
	scan_param.item.scan_set.scan_outreso_y =         GetDocScanResoY    (doc_scan_area_spec_quality, doc_scan_area_spec_src, doc_scan_area_spec_size);

#endif /* SCAN_AREA_RC_SEPARATED */

	/* 色数と読取解像度の取得 */
	conv_scan_quality_to_color_reso( quality, &color_num, &resolution);

#ifdef USE_SCAN_COLOR_DETECT /* 色数自動判別 */
	if(color_num == COLOR_MODE_AUTO) {	/* 色数自動判別設定 */
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

#ifdef	USE_SCAN_BLANK_DETECT	/* 白紙除去設定 */
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

#ifdef	SCAN_MF_DETECT /* 重送検知 */
	if(ftp_cifs_access_info.scan_multifeed_detect == FTPNW_MULTIFEED_DETECT_ON) {
		scan_param.item.scan_set.scan_multifeed_detect	 = SCAN_MULTIFEED_DETECT_ON;
	} else {
		scan_param.item.scan_set.scan_multifeed_detect	 = SCAN_MULTIFEED_DETECT_OFF;
	}
#else	/* #ifdef	SCAN_MF_DETECT */
	scan_param.item.scan_set.scan_multifeed_detect	 = SCAN_MULTIFEED_DETECT_OFF;
#endif	/* #ifdef	SCAN_MF_DETECT */

#ifdef SCAN_AREA_RC_SEPARATED
	/* 主走査方向の読取範囲の画素数取得 */
	scan_param.item.scan_set.scan_area_x = GetDocScanAreaSizeX_RCseparated(color_num, resolution,doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* 主走査方向の読取範囲の左端余白取得 */
	scan_param.item.scan_set.scan_x_offset = GetDocScanOffsetX_RCseparated(color_num, resolution,doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* 主走査方向の読取範囲の解像度取得 */
	scan_param.item.scan_set.scan_outreso_x = GetDocScanResoX_RCseparated(color_num, resolution,doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* 副走査方向の読取範囲のライン数取得 */
	scan_param.item.scan_set.scan_area_y = GetDocScanAreaSizeY_RCseparated(color_num, resolution,doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* 副走査方向の読取範囲の先端余白 */
	scan_param.item.scan_set.scan_y_offset = GetDocScanOffsetY_RCseparated(color_num, resolution,doc_scan_area_spec_src, doc_scan_area_spec_size);
	/* 副走査方向の読取範囲の解像度取得 */
	scan_param.item.scan_set.scan_outreso_y = GetDocScanResoY_RCseparated(color_num, resolution,doc_scan_area_spec_src, doc_scan_area_spec_size);
#endif /* SCAN_AREA_RC_SEPARATED */

	/* スキャンパラメータ設定４                       */
    /* カラー／モノクロで区別する値のセット           */
	switch ( color_num ) {
#ifdef	COLOR_NETWORK_SCANNER
#ifdef USE_SCAN_COLOR_DETECT /* 色数自動判別 */
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

	/* スキャンパラメータ設定５                       */
    /* 両面読取設定値のセット                         */
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
		/* BSI制限Userの利用履歴保存開始 */
		set_scan_ftp_cifs_setting_bsilog();
	}
#endif	/* defined(USE_BSI) && defined(USE_SERIO_LOG) */

    /* Scan圧縮率                         */
    scan_param.item.scan_set.scan_quality = ftp_cifs_access_info.scan_quality;

	/* Ground Color Removal OFF固定 */
#ifdef    USE_GNDCOLOR_REMOVAL
	scan_param.item.scan_set.gndcolor_removal = ftp_cifs_access_info.gndcolor_removal;
	scan_param.item.scan_set.gndcolor_level = ftp_cifs_access_info.gndcolor_level;
#endif /* USE_GNDCOLOR_REMOVAL */

	/* スキャンパラメータ送信                         */
	FOS_MSGSEND( qid_scanbase, (UINT8 *)&scan_param, sizeof(scannertask_cntl_msg_t) );

	/* スキャン開始コマンド送信                       */
	send_scan_start_end( MEM_SCAN_START, apli_mode, image_id, sysmem_id );

	return;
}

/**
* @par	スキャン中断
* @param	from_task (input) メッセージ送信元タスクID
* @param	cmd_id (input) メッセージ内のコマンドID（メッセージ送信元タスクの場合は装置状態）
* @param	image_id (input) 画データID
* @param	sysmem_id (input) システムメモリID
* @return	OK：正常終了
*			ERROR：異常終了
*
* @par <外部仕様>
*		スキャンを中断する為の処理を行いスキャンを中断する。
* @par <内部仕様>
*		スキャン中断のLCD表示やブザーを鳴らし、スキャン中断要求をScan Base Taskに
*		メッセージ送信する。
*
* @par <M票>
*	   M-BCL-945
*/

STATIC void
send_scan_stop( void )
{
	scannertask_cntl_msg_t scan_param;

	/* 初期化 */
	memset( &scan_param, NULL, sizeof(scannertask_cntl_msg_t) );

	scan_param.com_msg.from_task               = FTPC_APL_TASK;
	scan_param.com_msg.cmd_id                  = MEM_SCAN_STOP;
	scan_param.item.memory_scan_stop.save_mode = FSAVE_PAGEEND;
	FOS_MSGSEND( qid_scanbase, (UINT8 *)&scan_param, sizeof(scannertask_cntl_msg_t) );

	return;
}

/**
* @par スキャン開始・終了メッセージ送信
* @param	cmd_id (input) メッセージ内のコマンドID（メッセージ送信元タスクの場合は装置状態）
* @param	apli_mode (input) スキャンモード
* @param	image_id (input) 画データID
* @param	sysmem_id (input) システムメモリID
* @return	なし
*
* @par <外部仕様>
*		スキャン開始とスキャン終了のメッセージを送信する。
* @par <内部仕様>
*		スキャン開始とスキャン終了のメッセージをScan Base Taskへ送信する。
*
* @par <M票>
*	   M-BCL-945
*/
STATIC void
send_scan_start_end( UINT16 cmd_id, UINT8 apli_mode, INT32 image_id, INT32 sysmem_id )
{
	scannertask_cntl_msg_t		scan_param;

	/* 初期化                 */
	memset( &scan_param, NULL, sizeof(scannertask_cntl_msg_t) );

	/* スキャンパラメータ設定 */
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
	/* スキャンパラメータ送信 */
	FOS_MSGSEND( qid_scanbase, (UINT8 *)&scan_param, sizeof(scannertask_cntl_msg_t) );

	return;
}

/**
* @par	サーバーへのデータ送信処理
* @param	image_id (input) 画データID
* @return	なし
*
* @par <外部仕様>
*		スキャナーで読取したデータをファイル転送する。
* @par <内部仕様>
*		スキャナーで読取したデータを指定のサーバーへ指定のプロトコルで転送する。
*
* @par <M票>
*	   M-BCL-945
*/
STATIC void
transfer_process( INT32 image_id )
{
	BOOL               error_flag;       /* エラーが発生したかどうか TRUE:発生した */
	UINT32             str_id;           /* 表示する文字列ID                       */
	/* 画データ処理関連の変数      */
	INT32              file_id;
	INT32              page_exist;
	INT32              page_no;
	image_pageinfo_t   image_pageinfo;
	/* ネットワーク送信関連の変数  */
	BN_FTPC_HANDLE     server_handle;    /* サーバ接続で得られるハンドル           */
#ifdef	USE_SCAN2SFTP
	BN_SFTPC_HANDLE    server_handle_sftp;
#endif	/* USE_SCAN2SFTP */
	INT32              ret_value;        /* API関数の返り値                        */
    INT32			   ret_value_ftp;
    INT32			   ret_value_cifs;
#ifdef	USE_SCAN2SFTP
	INT32			   ret_value_sftp;
#endif	/* USE_SCAN2SFTP */
	UINT8              filename[ SCAN2FTP_FNAME_MAXSIZE ];
                                         /* サーバへの出力ファイル名               */
	SCAN2FTP_FILEFORMAT		
					   fileformat;	     /* ファイルフォーマット */
	UINT32             counter; 	 	 /* JPEGファイル初めの連続番号(FB+ADFカウンタ値) */

    stcPROTOCOLImageData
                        snd_img_data;    /* 送信するイメージデータ情報構造体       */
    stcFTPConnect       ftp_conn;        /* FTP接続情報構造体                      */
#ifdef	USE_SCAN2NW
    stcCIFSConnect      cifs_conn;       /* CIFS接続情報構造体                     */
#endif
#ifdef	USE_SCAN2SFTP
	stcSFTPConnect		sftp_conn;		 /* SFTP接続情報構造体 */
#endif	/* USE_SCAN2SFTP */

	/* 初期化 */
	error_flag       = FALSE;
#ifdef	USE_SEPARATE_UI
	str_id           =  OK; /* QAC対策：実際に使用されることはない */
#else	/* USE_SEPARATE_UI */
    str_id           = MAIL_SENDERR_DSP;
#endif	/* USE_SEPARATE_UI */
    ret_value        = ERROR;
    ret_value_ftp    = ERROR;
    ret_value_cifs   = ERROR;
#ifdef	USE_SCAN2SFTP
    ret_value_sftp   = ERROR;
#endif	/* USE_SCAN2SFTP */
	/* 画データ処理関連の変数      */
	file_id          = ERROR;            /* ファイルディスクリプタ  ImageOpen用    */
	page_exist       = ERROR;            /* ページが存在するかどうか               */
	page_no          = 0;                /* ページ番号                             */
	memset( &image_pageinfo, NULL, sizeof(image_pageinfo_t) );
	/* ネットワーク送信関連の変数  */
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

	/* 初期処理                    */
#ifndef	USE_SEPARATE_UI
    /* アプリ表示権確保            */
	ftpc_disp_rightchg( GET_RIGHT );
    /* Sending 表示                */
	ftpc_disp_string  ( MAIL_SEND_DSP, NULL );
#else  /* USE_SEPARATE_UI */
	cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_SENDING );
#endif	/* USE_SEPARATE_UI */

    /* ServiceがScanToFtpの場合    */
    if (service_kind       == SCAN2FTP_SERVICE) {
#ifdef USE_SCAN2FTP
        /* FTPホスト接続実施       */
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

    /* ServiceがScanToCIFSの場合   */
    }
#ifdef	USE_SCAN2NW
    else if (service_kind == SCAN2CIFS_SERVICE)
    {
        /* CIFSホスト接続実施      */
        cifs_conn.HostAddress    = ftp_cifs_access_info.serveraddress;

        cifs_conn.StoreDirectory = ftp_cifs_access_info.storedir;

        cifs_conn.UserName       = ftp_cifs_access_info.username;

        cifs_conn.Password       = ftp_cifs_access_info.password;

        cifs_conn.AuthenticationMethod
                                 = ftp_cifs_access_info.AuthenticationMethod;

        cifs_conn.kerberosServerAddress
                                 = ftp_cifs_access_info.KerberosServerAddress;

        /* CIFS常にOK              */
        ret_value = OK;
    }
#endif
#ifdef	USE_SCAN2SFTP
	else if(service_kind == SCAN2SFTP_SERVICE)
	{
		/* SFTP情報実施のための準備 */
		sftp_conn.HostAddress = ftp_cifs_access_info.serveraddress;
		sftp_conn.PortNumber = ftp_cifs_access_info.portnum;
		sftp_conn.AuthMeth = ftp_cifs_access_info.AuthenticationMethod;
		sftp_conn.UserName = ftp_cifs_access_info.username;
		sftp_conn.Password = ftp_cifs_access_info.password;
		sftp_conn.PubKeyIdx = ftp_cifs_access_info.PubKeyIdx;
		sftp_conn.PairKeyIdx = ftp_cifs_access_info.PairKeyIdx;
		sftp_conn.StoreDirectory = ftp_cifs_access_info.storedir;

		/* SFTP Server接続実施 */
		ret_value_sftp = sftpclib_Connect( &sftp_conn, &server_handle_sftp );

		if( ret_value_sftp == SFTPCLIB_ERR_NONE ) {
			ret_value = OK;
		} else {
			ret_value = ERROR;
		}
	}
#endif	/* USE_SCAN2SFTP */

	/* サーバー接続成功            */
	if ( ret_value == OK ) {
		/* 画データオープン        */
		file_id	= ImageOpen( image_id, IMAGE_READ );

		/* 画データオープン成功    */
		if ( file_id != ERROR ) {
            /* ファイルフォーマット JPEGの場合            */
            if (fileformat == FTP_JPEG){
                for ( page_no=1; ; page_no++ ) {
                    /* 出力ファイル名を作成               */
                    
                    decide_filename( counter, page_no, filename, ftp_cifs_access_info.FileNameFixed);
                    /* ページ情報読み取り                 */
					page_exist	= ImageReadPageInf( file_id, page_no, &image_pageinfo   );
					/* 最後のページまで読み込んだ         */
					if ( page_exist != OK ) {
						break;
					}
                    /* オープンした画データのファイルID   */
                    snd_img_data.FileId   = file_id;
                    /* ページ番号を指定                   */
                    snd_img_data.PageNo   = page_no;
                    /* ファイル名を指定                   */
                    snd_img_data.FileName = filename;
                    /* ファイル形式                       */
                    snd_img_data.FileType = get_filetype(fileformat);

                    /* ServiceがScanToFtpの場合           */
                    if (service_kind      == SCAN2FTP_SERVICE) {
#ifdef USE_SCAN2FTP
                        /* FTP イメージ書込               */
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
                    /* ServiceがScanToCIFSの場合          */
                    }else if (service_kind == SCAN2CIFS_SERVICE){
#ifdef	USE_SCAN2NW
                        /* CIFS イメージ書込              */
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

                    /* 書き込み失敗                       */
                    if ( ret_value != OK ) {
					    error_flag		  = TRUE;
                    }
				    
#if	defined(USE_BSI) && defined(USE_SERIO_LOG)
					/* 利用履歴Log保存の成否確認 */
					if (Serio_Is_Enabled() == TRUE) {
						if( SerioLog_Rec_CntPrnPage( LOG_DATA_FUNC_SCAN, NULL ) != OK ){
							error_flag = TRUE;
						}
					}
#endif	/* defined(USE_BSI) && defined(USE_SERIO_LOG) */
					
                    /* 何らかのエラーが発生した場合は、   */
                    /* 次のページに進まずに終了           */
				    if ( error_flag == TRUE ) {
					    break;
				    }
                }
            }else {
                /* オープンした画データのファイルID       */
                snd_img_data.FileId       = file_id;

                /* ページ番号(0)を指定                    */
                snd_img_data.PageNo       = 0;
                /* ファイル名を設定                       */
                snd_img_data.FileName     = filename;

                /* ファイル形式                           */
                snd_img_data.FileType     = get_filetype(fileformat);

                /* ServiceがScanToFtpの場合               */
                if (service_kind       == SCAN2FTP_SERVICE) {
#ifdef USE_SCAN2FTP
                    /* FTP イメージ書込                   */
                    ret_value_ftp  = ftpclib_WriteImage (server_handle, &snd_img_data);
  	                if ( ret_value_ftp == FTPCLIB_SUCCESS ) {
                        ret_value  = OK;
                    }else{
                        ret_value  = ERROR;
                    }
#endif /* USE_SCAN2FTP */
                /* ServiceがScanToCIFSの場合              */
                }else if (service_kind == SCAN2CIFS_SERVICE){
#ifdef	USE_SCAN2NW
                    /* CIFS イメージ書込                  */
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

                /* 書き込み失敗                           */
                if ( ret_value != OK ) {
                    error_flag		      = TRUE;
                }
			}
			/* 画データクローズ      */
			ImageClose( file_id );
		}
		/* 画データオープン失敗      */
		else {
			error_flag = TRUE;
		}

        /* FTPホスト切断実施         */
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
	/* ホスト接続失敗                */
	else {
		error_flag     = TRUE;
	}
	
	/* 終了処理                      */
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
		/* パネル入力・表示権確保    */
		ftpc_key_rightchg( GET_RIGHT );

		/* 全体表示：SendingErr など */
		ftpc_disp_string( str_id, NULL );
#ifdef	LCD_5LINE 
		/* 五行モデル */
		ftpc_disp_icon( ERASE_OBJ );
		ftpc_disp_line();
        /* Error表示時は３～５行目を消去する            */
        ftpc_disp_string_NULL( NULL_LINE3 | NULL_LINE4 | NULL_LINE5 );
#else	/* LCD_5Line */
		ftpc_disp_string( DUMMY_LINE2, NULL );
#endif	/* LCD_5LINE */
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		/* 実行停止通知 */
		SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL, SERIO_JOB_SCANSEND);
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
		FOS_TASKSLEEP( 500 );

		/* パネル入力・表示権確保    */
		ftpc_key_rightchg( FREE_RIGHT );
#else	/* USE_SEPARATE_UI */
		cp_Sts_Apl_Entry( CP_STS_APL_FTPC, (UINT8)str_id );
#endif	/* USE_SEPARATE_UI */
		
#ifdef USE_BSI
		if (Serio_Is_Enabled() == TRUE) {
			/* 実行停止通知(装置状態エラー) */
			SendJobProgress_TransEnd(SERIO_JOB_SCANSEND);
			if (fileformat == FTP_JPEG){
				/* JPEGはページ毎に送信出来てしまうため送信成功として終了する。 */
				ScanEnd(SERIOFW_JOBSTS_END_COMPLETE, SERIO_EXTERR_COMPLETE);
			}else{
				ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
			}
		}
#endif /* USE_BSI */
	}
	else {
		/* 正常終了時                */
		buzzer_start( BUZZER_FINISH, APL_FTPCLIENT );
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
		if (Serio_Is_Enabled() == TRUE) {
#ifdef USE_SERIO_LOG
			if (fileformat != FTP_JPEG){
				/* JPEG送信ではなかった場合、ここでSCAN時にカウントした枚数を保存する */
				SerioLog_Rec_AddPage(LOG_DATA_FUNC_SCAN);
			}
#endif	/* USE_SERIO_LOG */
			SendJobProgress_TransEnd(SERIO_JOB_SCANSEND);
			ScanEnd(SERIOFW_JOBSTS_END_COMPLETE, SERIO_EXTERR_COMPLETE);
		}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
	}


#ifndef	USE_SEPARATE_UI
	/* アプリ表示権解放              */
	ftpc_disp_rightchg( FREE_RIGHT );
#endif	/* USE_SEPARATE_UI */

	ImageDelete ( image_id );

#ifndef	USE_SEPARATE_UI
#ifdef USE_SELECTMODE_KEY
    /* Default Mode復帰用タイマセット*/
    set_default_mode_ret( DM_SCAN_APL_END, DM_RET_ON );
#endif
#endif	/* USE_SEPARATE_UI */
	/* ステータスを待機状態 */
    ftpscan_state = FTPC_START_WAIT;
#ifdef	USE_SEPARATE_UI
	cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif

	return;
}

/**
* @par	リソース解放
* @param	sysmem_id (input/output) システムメモリID
* @return	なし
*
* @par <外部仕様>
*		タスクで使用したリソースを解放する。
* @par <内部仕様>
*		タスクで使用したリソース、パネル入力権、表示権を解放する。
*
* @par <M票>
*	   M-BCL-945
*/
STATIC void
resource_release( INT32 sysmem_id )
{
    /* ADF   リソース解放        */
	resourcefree     ( RES_ADF );
    /* VIDEO リソース解放        */
	resourcefree     ( RES_SCAN_VIDEO );

#ifndef	USE_SEPARATE_UI
#ifdef LCD_5LINE
	/* 他の画面へ遷移したときに反転表示が残らないように、反転表示を戻しておく */
	scan_clear_cursor();
#endif	/* LCD_5LINE */
    /* パネル入力・表示権解放    */
	ftpc_key_rightchg( FREE_RIGHT );
#endif	/* USE_SEPARATE_UI */

    /* アプリ状態を READY にする */
	manaplset        ( S_APL_FTPCLIENT, APL_READY ); 

	if ( sysmem_id != NO_SYSMEMID ) {
        /* シスメム解放          */
        sysmApliFree ( sysmem_id );
	}

	return;
}

/**
* @par	接続エラー発生時の表示文字列決定処理
* @param	error_type (input) FTPライブラリのエラーコード
* @return	エラーコードに対応する辞書DBのID
*
* @par <外部仕様>
*		FTP/CIFSライブラリのエラーコードに対応するエラーメッセージIDを取得する。
* @par <内部仕様>
*		FTP/CIFSライブラリのエラーコードに対応するエラーメッセージIDを取得する。
*
* @par <M票>
*	   M-BCL-945
*/

STATIC UINT32
decide_error_id( INT32 error_type )
{
	INT32 str_id;

	switch ( error_type ) {
        /* 認証エラー                      */
		case FTPCLIB_ERROR_AUTH       :
#ifndef	USE_SEPARATE_UI
			str_id	= AUTHEN_ERR_DSP;
#else	/* USE_SEPARATE_UI */
			str_id = CP_STS_FTPC_AUTHERROR;
#endif	/* USE_SEPARATE_UI */
			break;
		/* タイムアウトエラー              */
		case FTPCLIB_ERROR_TIMEOUT    :
#ifndef	USE_SEPARATE_UI
			str_id	= SERVER_TOUT_DSP;
#else	/* USE_SEPARATE_UI */
			str_id	= CP_STS_FTPC_TIMEOUT;
#endif	/* USE_SEPARATE_UI */
			break;
        /* シンタックスエラー（内部エラー）*/
        case FTPCLIB_ERROR_SYNTAX     :
        /* 状態遷移エラー                  */
        case FTPCLIB_ERROR_STATUS     :
        /* 実行エラー（サーバー側のエラー）*/
        case FTPCLIB_ERROR_EXEC       :
        /* セッション/設定エラー           */
        case FTPCLIB_ERROR_SESSION    :
        /* 引数エラー                      */
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
* @par	接続エラー発生時の表示文字列決定処理
* @param	error_type (input) CIFSライブラリのエラーコード
* @return	エラーコードに対応する辞書DBのID
*
* @par <外部仕様>
*		FTP/CIFSライブラリのエラーコードに対応するエラーメッセージIDを取得する。
* @par <内部仕様>
*		FTP/CIFSライブラリのエラーコードに対応するエラーメッセージIDを取得する。
*
* @par <M票>
*	   M-BCL-945
*/

STATIC UINT32
decide_error_id_cifs( INT32 error_type )
{
	INT32 str_id;

	switch ( error_type ) {
        /* 認証エラー                      */
        case CIFSCLIB_ERROR_AUTH      :
#ifndef	USE_SEPARATE_UI
			str_id	= AUTHEN_ERR_DSP;
#else	/* USE_SEPARATE_UI */
			str_id	= CP_STS_FTPC_AUTHERROR;
#endif	/* USE_SEPARATE_UI */
			break;
		/* タイムアウトエラー              */
        case CIFSCLIB_ERROR_TIMEOUT   :
#ifndef	USE_SEPARATE_UI
			str_id	= SERVER_TOUT_DSP;
#else	/* USE_SEPARATE_UI */
			str_id	= CP_STS_FTPC_TIMEOUT;
#endif	/* USE_SEPARATE_UI */
			break;
		case CIFSCLIB_ERROR_CLKNOREADY:
		/* 時刻未設定エラー                */
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
* @par	接続エラー発生時の表示文字列決定処理
* @param	error_type (input) SFTPCライブラリのエラーコード
* @return	エラーコードに対応する辞書DBのID
*
* @par <外部仕様>
*		FTP/CIFSライブラリのエラーコードに対応するエラーメッセージIDを取得する。
* @par <内部仕様>
*		FTP/CIFSライブラリのエラーコードに対応するエラーメッセージIDを取得する。
*
* @par <M票>
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
* @par	ファイル名作成処理
* @param	counter (input) FB+ADFカウンタ値 
* @param	page_no (input) ページ番号
* @param	*filename (output) 作成したファイル名
* @return	なし
*
* @par <外部仕様>
*		JPEGファイルのファイル名を作成する。（拡張子は作成しない。）
* @par <内部仕様>
*		JPEGファイルのファイル名を以下のように作成する。
*			hostname_[FB+ADFカウンタ値]		（1ページ目）
*			hostname_[FB+ADFカウンタ値]_[ページ番号]	（１ページ以降）
*
* @par <M票>
*	   M-BCL-945
*/

STATIC void
decide_filename( UINT32 counter, UINT8 page_no, UINT8 *filename, UINT8 FileNameFixed )
{
	UINT8		filename_top[ SCAN2FTP_FNAME_MAXSIZE ];  /* ファイル名の先頭文字列 */
	UINT8		extension[ 3 + 1 ];                       /* 拡張子                 */
	MD_CHAR		jpeg_scan_counter_str[JPEG_SCAN_CNT_SIZE]; 
	MD_CHAR		*p;

	/* 初期化 */
	memset( filename_top,  NULL, SCAN2FTP_FNAME_MAXSIZE );
	memset( extension, NULL, 3+1 );
	memset( jpeg_scan_counter_str, NULL, JPEG_SCAN_CNT_SIZE );

	strncpy( (MD_CHAR*)extension, FILETYPE_JPG, 3 );
	
	/* ファイル名出力                                            */
	/* 出力ファイル名は次の通り（機能仕様）                      */
	/*       hostname_[FB+ADFカウンタ値].[拡張子]                */
	/*       hostname_[FB+ADFカウンタ値]_[ページ番号].[拡張子]   */
	
	if(FALSE == FileNameFixed){
		sprintf( jpeg_scan_counter_str, "_%06ld", counter );

	/* ファイル名取得(counter文字列で検索)                      */
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
	/* ファイル名出力                                            */
	/* 出力ファイル名は次の通り（機能仕様）                      */
	/*       hostname.[拡張子]                */
	/*       hostname_[ページ番号].[拡張子]   */	
	else{
		if ( page_no == 1 ) {
			/*DO NOTHING*/
			/*      filename ==  hostname.[拡張子]                */
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
* @par	読取カウンタ取得
* @param *scan_counter (output) 読取カウンタ
* @return	なし
*
* @par <外部仕様>
*		JPEGファイル名に使用する読取カウンタ取得する。
* @par <内部仕様>
*		JPEGファイル名に使用する読取カウンタを両面読の有無を判断し取得する。
*
* @par <M票>
*	   M-BCL-945
*/
GLOBAL INT32
scan_counter( UINT32 *scan_counter )
{
    /* FBつきスキャナ                           */
#ifdef FB_SCAN_TYPE

    /* 両面読取モデル                           */
#ifdef	USE_DUPLEX_SCAN

	/* 両面読取機能有りのファンクションスイッチ */
	if ( fstget_data(FSW_DUPLEX_SCAN_FUNC) == FSW_DUPLEX_SCAN_FUNC_ON ) {
        *scan_counter	= scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADFDX) +
                          scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADF  ) +
                          scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_FB   ) ;
	}
	/* 両面読取機能なし                         */
	else {
		*scan_counter	= scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADF  ) +
                          scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_FB   ) ;
	}
#else /* USE_DUPLEX_SCAN                        */

    /* 片面読取モデル                           */
	*scan_counter      	= scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADF  ) +
                          scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_FB   ) ;
#endif /* USE_DUPLEX_SCAN                       */

#else /* FB_SCAN_TYPE                           */

    /* FBなしスキャナ                           */
#ifdef	USE_DUPLEX_SCAN

    /* 両面読取モデル                           */
	/* 両面読取機能有りのファンクションスイッチ */
	if ( fstget_data( FSW_DUPLEX_SCAN_FUNC ) == FSW_DUPLEX_SCAN_FUNC_ON ) {
        *scan_counter	= scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADFDX) +
                          scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADF  ) ;
	}
	/* 両面読取機能なし                         */
	else {
		*scan_counter	= scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADF);
	}
#else /* USE_DUPLEX_SCAN                        */
    /* 片面読取モデル                           */
	*scan_counter       = scan_cnt(MIB_VAL_BR_SCAN_COUNT_TYPE_ADF);
#endif /* USE_DUPLEX_SCAN */
	
#endif /* FB_SCAN_TYPE */

	return ( OK );
}

/**
* @par	Dアイコンの表示
* @return	なし
*
* @par <外部仕様>
*		５行モデルのときDアイコンを表示する。
* @par <内部仕様>
*		５行モデルのときDアイコンを表示する。
*
* @par <M票>
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
* @par Scan Page ADFを返す
* @param *buff   (input) 格納バッファへのアドレス
* @param mode    (input) Plain/XML
* @return なし
*
* @par <外部仕様>
*
*
* @par <内部仕様>
*
*
* @par <M票>
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

    /* Page Countを取得                          */
	if ( OK != objacc(OBJ_BR_SCAN_COUNT_COUNTER, GETVAL ,
                      &index, 0, (INT32*)&adf_cnt , &len) ) {
        return 0;
	}

    return adf_cnt;
}

/**
* @par	スキャナから原稿を読取る
* @param	from_task (input) メッセージ送信元タスクID
* @param	cmd_id (input) メッセージ内のコマンドID（メッセージ送信元タスクの場合は装置状態）
* @param	image_id (input/output) 画データID
* @param	sysmem_id (input/output) システムメモリID
* @return	なし
*
* @par <外部仕様>
*		スキャナから原稿を読取り、中断、終了を行う。
* @par <内部仕様>
*		スキャナから原稿を読取為の操作及び中断、終了を行う。
*
* @par <M票>
*	   M-BCL-945
*/

STATIC void
scan_process( UINT16 from_task, UINT32 cmd_id, INT32 image_id, INT32 sysmem_id )
{
	switch ( from_task ) {

		/* 装置状態タスクから       */
		case MACSTATUS_LIBRARY:
			scan_machstatus( cmd_id );
			break;

		/* スキャンベースタスクから */
		case SCAN_BASE_TASK:
			scan_scanbase( cmd_id, image_id, sysmem_id );
			break;

		/* パネルベースタスクから   */
		case PANEL_BASE_TASK:
			scan_pnltask(  cmd_id, image_id, sysmem_id );
			break;

		/* NULLベースタスクから   */
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
* @par	スキャン中の装置状態タスクからのメッセージ処理
* @param	cmd_id (input) 装置状態
* @return	なし
*
* @par <外部仕様>
*		装置状態を判別してLCD表示を更新する。
* @par <内部仕様>
*		装置状態を判別してLCD表示をWarmUpからScanning更新する。
*
* @par <M票>
*	   M-BCL-945
*/
STATIC void
scan_machstatus( UINT32 cmd_id )
{
	return;
}

/**
* @par	スキャン中のScan Baseタスクからのメッセージ処理
* @param	cmd_id (input) メッセージ内のコマンドID
* @param	image_id (input) 画データID
* @param	sysmem_id (input) システムメモリID
* @return	なし
*
* @par <外部仕様>
*		Scan Baseタスクからのコマンドに対応した読取、中断、終了を行う。
* @par <内部仕様>
*		Scan Baseタスクからのコマンドと読取装置（ADF/FB）に対応した読取、中断、終了を行う。
*
* @par <M票>
*	   M-BCL-945
*/
STATIC void
scan_scanbase( UINT16 cmd_id, INT32 image_id, INT32 sysmem_id )
{
    /* memory full が起こっているかどうかを保持           */
    /* TRUE : MemoryFull                                  */
	STATIC BOOL memful_flag = FALSE;
#if	defined(USE_BSI) && defined(USE_SERIO_LOG)
	STATIC BOOL adf_scanstart_flag = FALSE;
#endif	/* defined(USE_BSI) && defined(USE_SERIO_LOG) */
	/* スキャン動作モード           */
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
        /* ADF で読み取り                                 */
		switch ( cmd_id ) {

			/* 正常終了処理                               */
#ifdef	USE_SCAN_BLANK_DETECT
	/*** 白紙を読取り、かつ、後続ページがない場合 ***/
			case RT_MEM_SCAN_BLANK_PAGE_ALLEND:	/* fall through*/
				ftpclient_BlankPage(FTPC_PGCNT_INCREMENT, NULL);
#endif	/* USE_SCAN_BLANK_DETECT */
			case RT_MEM_SCAN_ALLEND:	/* fall through*/
				if( cmd_id == RT_MEM_SCAN_ALLEND )
				{
					ftpclient_ScanPage(FTPC_PGCNT_INCREMENT, NULL);
#if	defined(USE_BSI) && defined(USE_SERIO_LOG)
					/* BSI制限Userログ用カウント */
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
				/* データ送信へ移行                       */
				ftpclient_ScanPage(FTPC_PGCNT_GET, &ScanPage);
#ifdef FTPC_COMPLETE_SEQUENCE
				ftpclient_BlankPage(FTPC_PGCNT_GET, &BlankPage);
				/* 完了画面表示を行うか判定 */
				if( (ftp_cifs_access_info.scan_blank_detect == FTPNW_BLANK_DETECT_ON)
				 && (BlankPage > 0)
				 && (BlankPage != FTPC_PGCNT_INVALID_VAL) ){
					ftpscan_state   = FTPC_COMPLETE;
#ifdef USE_SEPARATE_UI
					cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_COMPLETE );
#endif /* USE_SEPARATE_UI */
					/* 60秒タイムアウトを設定する */
					set_ftpc_CompleteStatus_end_time(COMPLETE_TIMEOUT);
					break;
				}
#endif /* FTPC_COMPLETE_SEQUENCE */
				if(ScanPage == 0 || ScanPage == FTPC_PGCNT_INVALID_VAL) {
					ImageDelete ( image_id );
					/* ステータスを待機状態 */
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
                /* パネル入力・表示権確保                 */
                ftpc_key_rightchg( GET_RIGHT );
#endif	/* USE_SEPARATE_UI */
                /* Reached ADFLimit表示                   */
                DPRINTF((" ftpc : Scan Limit @ftpclient_main\n"));

#ifndef	USE_SEPARATE_UI
                /* 拒否音鳴動                             */
                buzzer_refusal();
                /* 二行モデル                             */
                /* 五行モデルはまだない                   */
#ifdef LCD_2LINE
                ftpc_disp_string( REACH_ADF_LIMIT_DSP, NULL );
                /* TX_YES_NO_DSP [1.Yes 2.No(Send)]       */
                ftpc_disp_string( SEND_DELETE_DSP, NULL );
#endif /* LCD_2LINE                                       */
#else	/* USE_SEPARATE_UI */
				cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_ADF_LIMIT );
#endif	/* USE_SEPARATE_UI */

				/* 連続読取制限停止中へ移行               */
				ftpscan_state = FTPC_SCAN_LIMIT_STOPPING;

				send_scan_start_end( MEM_SCAN_END,
				                     NULL, image_id, sysmem_id );
				break;
#endif /* FUNC_ADF_SCAN_SPEC_SHEETS_STOP                  */
			/* Memory Full異常発生                        */
			case RT_MEM_SCAN_MEMORYFULL:
				if ( memful_flag == FALSE ) {
					scan_stop_memfull();
					memful_flag = TRUE;
				}
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
				/* デフォルトモード復帰用タイマセット     */
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
				/* デフォルトモード復帰用タイマセット */
				set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
				break;
			/* スキャン中断/スキャンエラー発生            */
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
						/* STOPキーでの停止 */
#ifndef	USE_SEPARATE_UI
						SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_STATE, SERIO_JOB_SCANSEND);
#endif /* USE_SEPARATE_UI */
						ScanEnd(SERIOFW_JOBSTS_END_CANCEL, SERIO_EXTERR_ABORTBYUSER);
					}else{
						/* 実行停止通知(装置状態エラー) */
#ifndef	USE_SEPARATE_UI
						SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_STATE, SERIO_JOB_SCANSEND);
#endif /* USE_SEPARATE_UI */
						ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
					}
				}
#endif /* USE_BSI */
				memful_flag		= FALSE;
				/* ステータスを待機状態 */
				ftpscan_state	= FTPC_START_WAIT;
#ifdef	USE_SEPARATE_UI
				cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
				/* デフォルトモード復帰用タイマセット     */
				set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
				break;

#ifdef USE_SCAN_BLANK_DETECT
		/*** 白紙を読取り、かつ、後続ページがある場合 ***/
			case RT_MEM_SCAN_BLANK_PAGE:
				ftpclient_BlankPage(FTPC_PGCNT_INCREMENT, NULL);
#ifdef	USE_SEPARATE_UI
				cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_BLANK_PAGE );	/* 1枚白紙通知 */
#endif /* USE_SEPARATE_UI */
				/* 次ページの読取を開始する */
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
				/* スキャン開始コマンド送信                       */
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
				cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_END );	/* 1枚読取通知 */
#endif /* USE_SEPARATE_UI */
				break;
			/* その他のコマンドは処理しない               */
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
        /* FB で読み取り                                  */
		switch( cmd_id ) {
			
			/* 次ページ読込                               */
			case RT_MEM_SCAN_END:
				/* 全体表示：NextPage?                    */
#if defined(USE_BSI) && defined(USE_SERIO_LOG)
				if (Serio_Is_Enabled() == TRUE) {
					if( scan_bsiuserlog_pagecount( image_id, sysmem_id ) != OK )
					{
						/* 抜ける */
						break;
					}
				}
#endif	/* USE_BSI && USE_SERIO_LOG */
#ifndef	USE_SEPARATE_UI
#ifdef LCD_5LINE /* 五行モデル */
				ftpc_disp_string_NULL( NULL_LINE2 | NULL_LINE3 | NULL_LINE4 | NULL_LINE5 );
				ftpc_disp_string( FLATBED_SCAN_DSP, NULL );
				ftpc_disp_string( NEXT_DOC_DSP, NULL );
				ftpc_disp_string_special( YES_DSP, NULL );
				ftpc_disp_string_special( NO_SEND_DSP, NULL );
				ftpc_disp_string( UP_DOWN_DSP, NULL );
				ftpc_disp_reverse( DISP_LINE3, 3, LCD_REVERSE_ON  );
#endif /* 五行モデル */

#ifdef LCD_2LINE /* 二行モデル */
				ftpc_disp_string( NEXT_DOC_DSP, NULL );
                /* TX_YES_NO_DSP [1.Yes 2.No(Send)]       */
				ftpc_disp_string( TX_YES_NO_DSP, NULL );
#endif	/* 二行モデル */
#else	/* USE_SEPARATE_UI */
				cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_NEXT );
#endif	/* USE_SEPARATE_UI */

				ftpscan_state		= FTPC_SELECTNEXT;
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
				if (Serio_Is_Enabled() == TRUE) {
					/* 実行停止通知 */
					SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_SCANNEXT, RESTART_NEXT_PAGE, SERIO_JOB_SCANSEND);
				}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
#ifndef	USE_SEPARATE_UI
                /* タイマー停止                           */
				dispcntl_keytimeout( APL_FTPCLIENT, PNL_TIMEOUT_STOP, 0 );
                /* タイマー開始        */
				dispcntl_keytimeout( APL_FTPCLIENT, PNL_TIMEOUT, 600 );
#endif	/* USE_SEPARATE_UI */
				break;

			/* 正常終了/スキャン中断  */
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

					/* 正常終了時はデータ送信へ移行       */
					ftpscan_state   = FTPC_SEND;
				}
				else{
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
					if (Serio_Is_Enabled() == TRUE) {
						if (memful_flag == TRUE){
							SendUiEnd_Restart(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL);
							ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
						}else if (ftpscan_state == FTPC_SCANSTOP && memful_flag == FALSE){
							/* STOPキーでの停止 */
#ifndef	USE_SEPARATE_UI
							SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_STATE, SERIO_JOB_SCANSEND);
#endif /* USE_SEPARATE_UI */
							ScanEnd(SERIOFW_JOBSTS_END_CANCEL, SERIO_EXTERR_ABORTBYUSER);
						}else{
							/* 実行停止通知 */
#ifndef	USE_SEPARATE_UI
							SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_STATE, SERIO_JOB_SCANSEND);
#endif /* USE_SEPARATE_UI */
							ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
						}
					}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
					/* ステータスを待機状態 */
					ftpscan_state   = FTPC_START_WAIT;
#ifdef	USE_SEPARATE_UI
					cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
					/* デフォルトモード復帰用タイマセット */
					set_default_mode_ret( DM_SCAN_APL_END, DM_RET_ON );
#endif
#endif	/* USE_SEPARATE_UI */
				}
                /* Memory Full状態は初期値に戻す          */
				memful_flag	= FALSE;
				break;

			/* Memory Full異常発生                        */
			case RT_MEM_SCAN_MEMORYFULL:
				if ( memful_flag == FALSE ) {
					scan_stop_memfull();
					memful_flag     = TRUE;
				}
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
				/* デフォルトモード復帰用タイマセット     */
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
				/* デフォルトモード復帰用タイマセット */
				set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
				break;
			/* スキャンエラー発生                         */
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
						/* 実行停止通知(装置状態エラー) */
#ifndef	USE_SEPARATE_UI
						SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_STATE, SERIO_JOB_SCANSEND);
#endif /* USE_SEPARATE_UI */
						ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
					}
				}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */
				memful_flag		= FALSE;
				/* ステータスを待機状態 */
				ftpscan_state	= FTPC_START_WAIT;
#ifdef	USE_SEPARATE_UI
				cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif
#ifndef	USE_SEPARATE_UI
#ifdef	USE_SELECTMODE_KEY
				/* デフォルトモード復帰用タイマセット     */
				set_default_mode_ret(DM_SCAN_APL_END,DM_RET_ON);
#endif
#endif	/* USE_SEPARATE_UI */
				break;

			/* その他のコマンドは処理しない               */
			case RT_MEM_SCAN_START:
#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
				if (Serio_Is_Enabled() == TRUE) {
					/* 実行中通知 */
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
* @par	スキャン中のPanelタスクからのメッセージ処理
* @param	cmd_id (input) メッセージ内のコマンドID
* @param	image_id (input) 画データID
* @param	sysmem_id (input) システムメモリID
* @return	なし
*
* @par <外部仕様>
*		Panelタスクからのコマンドに対応した読取、中断、終了を行う。
* @par <内部仕様>
*		Panelタスクからのコマンドに対応した読取、中断の表示と終了メッセージ送信を行う。
*
* @par <M票>
*	   M-BCL-945
*/
STATIC void
scan_pnltask( UINT16 cmd_id, INT32 image_id, INT32 sysmem_id )
{
	UINT8     apli_mode; /* スキャン動作モード  */
#ifndef	USE_SEPARATE_UI
#ifdef	LCD_5LINE
	pstring_t str_st;    /* 解像度表示用の構造体*/
#endif	/* LCD_5LINE */
#endif	/* USE_SEPARATE_UI */

	/* 初期化                                   */
	apli_mode = MEM_SCAN_FAX;

	/*------------------------------------------*/
	/* Scan開始のCommand受信(メニュー操作終了)  */
	/*------------------------------------------*/
	switch ( cmd_id ) {
		/* スキャン中断終了のコマンド受信       */
		case CMD_SCANSTOP:

#ifndef	USE_SEPARATE_UI
			/* StopKeyPressed 表示              */
			ftpc_disp_string( PRDSTP_DSP, NULL );

			/* 両面読取の場合はＤアイコンを表示 */
			dispDIconFor5Line();
#else	/* USE_SEPARATE_UI */
			cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_STOP );
#endif	/* USE_SEPARATE_UI */

			/* スキャン中断処理                 */
			send_scan_stop();
#ifdef	FB_SCAN_TYPE
			if ( scan_src == SCAN_SRC_FB ) {
				send_scan_start_end( MEM_SCAN_END, NULL, image_id, sysmem_id );
			}
#endif
			ftpscan_state     = FTPC_SCANSTOP;
			break;

		/* 次ページスキャンのコマンド受信       */
		case CMD_NEXTPAGE:

#ifndef	USE_SEPARATE_UI
			/*------- 全体表示：Scanning -------*/
#ifdef	LCD_5LINE
			/* ファイルフォーマット用の任意文字列の初期化             */
			memset( &str_st, NULL, sizeof(pstring_t) );
			/* 翻訳語からファイルフォーマット用の任意文字列へ変換する */
            lcd_strcpy(str_st.str_data, Quality_DSP);
			/* 表示                             */
            /* Scan to FTP                      */
            if (service_kind      == SCAN2FTP_SERVICE) {
                ftpc_disp_string( SCANTO_FTP_DSP    ,  NULL );
            }else if(service_kind == SCAN2CIFS_SERVICE){
                ftpc_disp_string( SCANTO_NETWORK_DSP,  NULL );
            }
            /* (Server名)                       */
			ftpc_disp_string( DUMMY_LINE2,    servername_zl   );
            /* (解像度)                         */
			ftpc_disp_string( DUMMY_LINE3,    str_st.str_data );
            /* (ファイル名)                     */
			ftpc_disp_string( DUMMY_LINE4,    filename_zl     );
            /* Scanning                         */
			ftpc_disp_string( SCANNING_DSP,   NULL            );
#endif	/* LCD_5LINE */
#ifdef	LCD_2LINE
			/* 表示                             */
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

		/* スキャン正常終了のコマンド受信       */
		case CMD_SCANEND:
			send_scan_stop();
            break;
#ifdef FUNC_ADF_SCAN_SPEC_SHEETS_STOP
        /* スキャン制限による"送信"のコマンド受信 */
        case CMD_SCAN_LIMIT_SND:
            /* FTPクライアントの状態を"送信"へ    */
            ftpscan_state = FTPC_SEND;
            resource_release( sysmem_id );
            DPRINTF((" ftpc : CMD_SCAN_LIMIT_SND@ftpclient_main\n"));
			break;
        /* スキャン制限による"削除"のコマンド受信 */
        case CMD_SCAN_LIMIT_DEL:
	        /* アプリ表示権解放                   */
	        ftpc_disp_rightchg( FREE_RIGHT );
	        ImageDelete ( image_id );
            resource_release( sysmem_id );
#ifndef	USE_SEPARATE_UI
#ifdef USE_SELECTMODE_KEY
            /* Default Mode復帰用タイマセット     */
            set_default_mode_ret( DM_SCAN_APL_END, DM_RET_ON );
#endif
#endif	/* USE_SEPARATE_UI */
            /* FTPクライアントの状態を"待機"へ    */
#ifdef	USE_SEPARATE_UI
			cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif	/* USE_SEPARATE_UI */
			/* ステータスを待機状態 */
            ftpscan_state = FTPC_START_WAIT;
            DPRINTF((" ftpc : CMD_SCAN_LIMIT_DEL@ftpclient_main\n"));
            break;
#endif  /* FUNC_ADF_SCAN_SPEC_SHEETS_STOP         */
	}

	return;
}

/**
* @par	    NULL-TASKからMSGが来た時の処理
* @param	cmd_id  (input) メッセージ送信コマンド
* @param	image_id  (input) 画データID
* @param	sysmem_id  (input) シスメムID
* @return	なし
*
* @par <外部仕様>
*		NULL-TASKからMSGが来た時の処理
* @par <内部仕様>
*
* @par <M票>
*
* @par <フォーマットバージョン>
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
	/* スキャン中断終了のコマンド受信 */
	/*--------------------------------*/
	if(cmd_id == CMD_SCANSTOP){
#ifndef	USE_SEPARATE_UI
		/* StopKeyPressed 表示 */
		ftpc_disp_string( PRDSTP_DSP, NULL );
#else
		cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_STOP );
#endif

		/* スキャン中断処理 */
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
* @par	スキャン中のScan BaseタスクからのMemoru Fullメッセージ処理
* @return	なし
*
* @par <外部仕様>
*		Scan BaseタスクからのMemoru Fullコマンドに対応した処理を行う。
* @par <内部仕様>
*		Scan BaseタスクからのMemoru Fullコマンドに対応した表示、メッセージ送信等を行う。
*
* @par <M票>
*	   M-BCL-945
*/
STATIC void
scan_stop_memfull( void )
{
	buzzer_refusal();
#ifndef	USE_SEPARATE_UI
    /* memory full 表示                  */
	ftpc_disp_string( MEMFUL_DSP, NULL );

#ifdef	LCD_5LINE
	ftpc_disp_icon( ERASE_OBJ );
	ftpc_disp_line();
    /* Error表示時は３～５行目を消去する */
	ftpc_disp_string_NULL( NULL_LINE3 | NULL_LINE4 | NULL_LINE5 );
#else	/* LCD_5LINE */
    ftpc_disp_string( DUMMY_LINE2, NULL );	
#endif

#if	defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)
	if (Serio_Is_Enabled() == TRUE) {
		/* 実行停止通知 */
		/* 再開通知はRT_MEM_SCAN_STOP受信時のscan_stop_after()関数内で行っている */
		SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_NORMAL, SERIO_JOB_SCANSEND);
	}
#endif /* defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF) */

#ifdef	STATUS_LED
	/* memory full 時は LED を赤色点灯   */
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


    /* スキャン中断メッセージ送信        */
	send_scan_stop();

	ftpscan_state = FTPC_MEM_ERR;

	return;
}


/**
* @par	スキャン中断の後処理
* @param	memful (input) Memoru Full状態
* @return	なし
*
* @par <外部仕様>
*		スキャン中断時のパネル処理を行う。
* @par <内部仕様>
*		スキャン中断時のブザー、パネル表示の処理を行う。
*
* @par <M票>
*	   M-BCL-945
*/
STATIC void
scan_stop_after( BOOL memful )
{
	/* スキャン中断時，memory full 時は終了音を鳴らす */
	buzzer_start( BUZZER_FINISH, APL_FTPCLIENT );

	if ( memful == TRUE ) {
#ifndef	USE_SEPARATE_UI
#ifdef	STATUS_LED
		/* memory full 時は LED を赤色点灯            */
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
* @par	定義値の置き換え処理 FTPクライアント→Netプロトコル
* @param	file_format (input) FTPクライアントでのファイルフォーマット
* @return	なし
*
* @par <外部仕様>
*		FTPクライアントでの定義からNetプロトコルの定義へ変換する
* @par <内部仕様>
*
* @par <M票>
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
* @par	シスメモの予約実施(トライあり)
* @param	quality      (input) 解像度
* @param	dup_scan     (input) duplexスキャンか否か？
* @param	scan_quality (input) 圧縮率
* @param	file_format  (input) ファイルフォーマット
* @return	予約されたシスメモID
*
* @par <外部仕様>
*		解像度、duplexスキャンフラグ、ファイルタイプからシスメモの予約種別を判定
*		判定に従い、シスメモを予約する。
* @par <内部仕様>
*
* @par <M票>
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
    /* 解像度/ファイルサイズから決まるシスメモ予約トライ回数定義 */
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

    /* シスメム予約 残トライ回数 */
    UINT8  try_num    ;
    UINT8  qual_index ;
    UINT8  scan_qual_index;
    /* シスメム予約 アプリID     */
    UINT32 sysm_apl_id;

    /* 残Try回数に応じたシスメモアプリID(Color) */
    UINT32 apl_id_color[] = { SYSM_AP_SCANTO_READ_COLOR_DUPLEX_ROTATE,
                              SYSM_AP_SCANTO_READ_COLOR_DUPLEX_ROTATE_MID,
                              SYSM_AP_SCANTO_READ_COLOR_DUPLEX_ROTATE_LARGE };

    /* 残Try回数に応じたシスメモアプリID(Gray)  */
    UINT32 apl_id_gray [] = { SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE,
                              SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE_MID,
                              SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE_LARGE };

    try_num     = 0;
    sysm_apl_id = 0;

#if defined(USE_SCAN_AUTO_RESOLUTION)
	/* 読取自動解像度設定の場合、解像度を取得する */
    switch ( quality ) {
#ifdef COLOR_NETWORK_SCANNER
		case FTP_ColorAuto:
		case FTP_GrayAuto:
			if(scanmenu_EiScanGetResolution(SCAN_APP_GET_SYSMID_TRY, quality, file_format, scan_quality, &auto_resolution) != OK)
			{
				EPRINTF(("%s(%d)	scanmenu_EiScanGetResolution ERROR!!\n",__FILE__,__LINE__));
			}
			quality = (SCAN2FTP_QUALITY)auto_resolution;	/* 自動取得した解像度をセットする */
			break;
#endif /* COLOR_NETWORK_SCANNER */
		default:
			/* 設定値のまま */
			break;
	}
#endif	/* USE_SCAN_AUTO_RESOLUTION */

    /* 解像度に応じたindexを取得する            */
    qual_index      = get_qual_index     ( quality );
    /* ファイルサイズに応じたindexを取得する    */
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
                /* 解像度、ファイルサイズからトライ回数を取得する */
                try_num = sysm_try_tbl[qual_index][scan_qual_index];
                DPRINTF((" ftpc : try_num(CL) [%d]@get_sysmid_try\n", try_num));
                /* Try回数に応じたシスメモ予約IDを取得            */ 
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
                /* 解像度、ファイルサイズからトライ回数を取得する */
                try_num = sysm_try_tbl[qual_index][scan_qual_index];
                DPRINTF((" ftpc : try_num Gray [%d]@get_sysmid_try\n", try_num));
                /* Try回数に応じたシスメモ予約IDを取得            */ 
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
* @par	    テーブルsysm_try_tbl上のindexを返す
* @param	quality   (input) 解像度
* @return	テーブルsysm_try_tbl上のindex
*
* @par <外部仕様>
*		解像度からテーブルsysm_try_tbl上のindexを返す
* @par <内部仕様>
*
* @par <M票>
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
* @par	    テーブルsysm_try_tbl上のindexを返す
* @param	scan_quality  (input) ファイル圧縮率
* @return	テーブルsysm_try_tbl上のindex
*
* @par <外部仕様>
*		ファイル圧縮率からテーブルsysm_try_tbl上のindexを返す
* @par <内部仕様>
*
* @par <M票>
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
        /* ファイルサイズ Large (高画質)  */
        case P_SCAN_QUAL_HIGH:
            index = LARGE_IDX;
            break;
        /* ファイルサイズ Middle(中画質)  */
        case P_SCAN_QUAL_MID:
            index = MIDLE_IDX;
            break;
        /* ファイルサイズ Small (低画質)  */
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
* @par	Panel Menuの確定値(FTP/CIFS)の解像度を出力する。
* @param	なし
* @return	Panel Menuの確定値(FTP/CIFS)の解像度
*
* @par <外部仕様>
*		Panel Menuの確定値(FTP/CIFS)の解像度を出力する。
* @par <内部仕様>
*		同上。
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
* @par	BSI制限UserのSCAN2FTP/SCAN2CIFSの利用履歴保存関数
* @param	なし
* @return	なし
*
* @par <外部仕様>
*		BSI制限UserのSCAN2FTP/SCAN2CIFSの利用履歴を保存する。
* @par <内部仕様>
*		同上。
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
	
	/* Colot設定 */
	/* Resolution設定 */
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
	
	/* Duplex設定 */
	switch(ftp_cifs_access_info.dualscan){
		case FTP_DUPLEXLONG:
		case FTP_DUPLEXSHORT:
			scan_set_log.Duplex = LOG_DATA_DUPLEX_ENABLE;
			break;
		default:
			scan_set_log.Duplex = LOG_DATA_DUPLEX_DISABLE;
			break;
	}
	
	/* Scan種別 */
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
	
	/* 以下、RAM記憶用のデータ群 */
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
	
	/* Scan系設定のLog保存 */
	SerioLog_Rec_Start(LOG_DATA_FUNC_SCAN, &scan_set_log);
	
	/* Log保存開始時にScan枚数Count用変数を初期化 */
	/* 必ず上記ログ保存開始関数完了後に行う事     */
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
			/* 実行停止通知(装置状態エラー) */
#ifndef	USE_SEPARATE_UI
			SendJobStatus_Paused(SERIOFW_JOBSTS_PAUSEDBY_ERROR, RESTART_ERROR_STATE, SERIO_JOB_SCANSEND);
#endif /* USE_SEPARATE_UI */
			ScanEnd(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY);
			
#ifdef	USE_SEPARATE_UI
			cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif
			/* スキャン中断メッセージ送信        */
			send_scan_stop();
			
			/* FTPの状態をReadyに戻す */
			ftpscan_state	= FTPC_START_WAIT;
			
			/* 1JOBあたりのSCAN枚数が超えた事を呼び出し元に伝える。 */
			ret = NG;
		}
	}
	
	return ret;
}
#endif	/* USE_BSI && USE_SERIO_LOG */

#ifndef	USE_SEPARATE_UI
#ifdef LCD_5LINE
/*****************************************************************************
* @par カーソル削除処理
*
* @par <外部仕様>
*			カーソル削除処理
* @par <内部仕様>
*			1～4行目のカーソルを削除する
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
* @par		Scan to FTP,NW,SFTP接続Check結果設定(local)
* @param	SCAN_CHECK_RESULT rslt：Scan to FTP,NW,SFTP設定Check処理結果保持変数
* @return	無し
*
* @par <外部仕様>
*		Check処理結果を設定する。
* @par <内部仕様>
*		引数の値を判定し、正常値なら結果用変数(STATIC)に格納する。
* @par <M票>
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
* @par		Scan to FTP,NW,SFTP接続Check結果取得(local)
* @param	無し
* @return	SCAN_CHECK_RESULT：Scan to FTP,NW,SFTP設定Check処理結果保持変数
*
* @par <外部仕様>
*		外部非公開
* @par <内部仕様>
*		Scan to FTP,NW,SFTP設定Check処理結果取得用、local関数
*		ScanFTP_Check_GetResult、ScanNW_Check_GetResult、
*		ScanSFTP_Check_GetResult以外からのCall禁止
* @par <M票>
*		M-DCSL-867
*****************************************************************************/
STATIC SCAN_CHECK_RESULT
scan_check_getresult(void)
{
	SCAN_CHECK_RESULT loc_scan_check_result = SCAN_CHECK_BUSY;	/* 結果送信用 */
	
	if((SCAN_CHECK_INIT != scan_check_result)&&(SCAN_CHECK_BUSY != scan_check_result)){
		/* 結果送信用に詰めなおす */
		loc_scan_check_result = scan_check_result;
		/* 設定Check処理結果に「初期状態」を設定 */
		scan_check_setresult(SCAN_CHECK_INIT);
	}
	if(SCAN_CHECK_BUSY == scan_check_result){
		if(SCAN_CHECK_BUSSY_RESET_TIME < FOS_GETTICK() - scan_check_result_time){
			/* 確認中が40秒以上継続している場合 */
			loc_scan_check_result = SCAN_CHECK_ERR_BUSY;
			scan_check_setresult(SCAN_CHECK_INIT);
		}
	}
	return loc_scan_check_result;
}
/****************************************************************************
* @par		Scan to FTP,NW,SFTP接続Check開始要求(local)
* @param	UINT8 profilenumber(input) 接続先profile number
* @param	UINT16 cmd_id(input) メッセージ内のコマンドID（メッセージ送信元タスクの場合は装置状態）
* @return	TRUE：正常終了
*			FALSE：異常終了(拒否)
*
* @par <外部仕様>
*		外部非公開
* @par <内部仕様>
*		Scan to FTP,NW,SFTP接続Check開始要求通知用、local関数
*		ScanFTP_Check_Start、ScanNW_Check_Start、
*		ScanSFTP_Check_Start以外からのCall禁止
* @par <M票>
*		M-DCSL-867
*****************************************************************************/
STATIC BOOL
scan_check_start(UINT8 profilenumber,UINT16 cmd_id)
{
	ftpclt_msg	message;		/* スキャンパラメータ */
	UINT32		qid_FTPC;	/* メッセージID */
	UINT32		msgsend_ret;
	
	/* 引数Check */
	if((CMD_SCAN_TO_FTPTEST_STA != cmd_id)
		&&(CMD_SCAN_TO_NWTEST_STA != cmd_id)
#ifdef	USE_SCAN2SFTP
		&&(CMD_SCAN_TO_SFTPTEST_STA != cmd_id)
#endif	/* USE_SCAN2SFTP */
	){
		/* cmd_idが不正の場合 */
		return FALSE;
	}
	
	/* 状態Check */
	if((SCAN_CHECK_INIT != scan_check_result)&&(SCAN_CHECK_BUSY != scan_check_result)){
		if(SCAN_CHECK_RESULT_RESET_TIME < FOS_GETTICK() - scan_check_result_time){
			/* 書き込み完了後10秒以上経過している場合 */
			scan_check_setresult(SCAN_CHECK_INIT);
		}
	}
	if(SCAN_CHECK_BUSY == scan_check_result){
		if(SCAN_CHECK_BUSSY_RESET_TIME < FOS_GETTICK() - scan_check_result_time){
			/* 確認中が40秒以上継続している場合 */
			scan_check_setresult(SCAN_CHECK_INIT);
		}
	}
	if(SCAN_CHECK_INIT != scan_check_result){
		/* 初期状態では無い場合 */
		return FALSE;
	}
	
	/* 設定Check処理結果に「書き込み中」を設定 */
	scan_check_setresult(SCAN_CHECK_BUSY);
	
	/* メッセージID取得 */
	qid_FTPC = FOS_MSGGETID(FTPC_MSG_NAME);
	
	/* スキャンパラメータ設定 */
	memset(&message, NULL, sizeof(ftpclt_msg));
	message.from_task = FTPC_APL_TASK;		/* メッセージ送信元タスクID */
	message.cmd_id = cmd_id;				/* メッセージ内のコマンドID */
	message.profilenumber = profilenumber;	/* 接続先profile number */
	
	/* メッセージ送信(設定Check) */
	msgsend_ret = FOS_MSGSEND(qid_FTPC,(UINT8 *)&message,sizeof(ftpclt_msg));
	if(OK != msgsend_ret){
		scan_check_setresult(SCAN_CHECK_INIT);
		DPRINTF(("[%s](%d) Scan to SettingCheck Err\n", __FUNCTION__, __LINE__));
		return FALSE;
	}
	return TRUE;
}
/****************************************************************************
* @par		Scan to FTP接続情報設定
* @param	UINT8 profilenumber(input) 接続先profile number
* @param	stcFTPConnect* ftp_conn(input/output)
* @return	無し
*
* @par <外部仕様>
*		外部非公開
* @par <内部仕様>
*		Scan to FTP接続情報を設定する。
* @par <M票>
*		M-DCSL-867
*****************************************************************************/
STATIC void
scanftp_connection_setup(UINT8 profilenumber,stcFTPConnect* ftp_conn)
{
	/* Port番号 */
	UINT8	ftp_port_num[5+1];	/* Port番号 */
	memset(ftp_port_num, NULL, sizeof(ftp_port_num));
	
	/* FTPホスト接続情報設定 */
	FUNC_GET_INDEXSTR(OBJ_SCAN2FTP_SVRADR	,profilenumber	,ftp_conn->HostAddress		,SCAN2FTP_SVRADR_MAXLEN	);	/* FTPホスト名またはIPアドレス */
	FUNC_GET_INDEXSTR(OBJ_SCAN2FTP_STDIR	,profilenumber	,ftp_conn->StoreDirectory	,SCAN2FTP_STDIR_MAXLEN	);	/* ファイル保存先フォルダ */
	FUNC_GET_INDEXVAL(OBJ_SCAN2FTP_PAMODE	,profilenumber	,&(ftp_conn->IsPassive)		);							/* パッシブモード or アクティブモード */
	FUNC_GET_INDEXSTR(OBJ_SCAN2FTP_PORT		,profilenumber	,ftp_port_num				,sizeof(ftp_port_num)	);	/* Port番号 */
	ftp_conn->PortNumber = atoi((const char *)ftp_port_num);
	FUNC_GET_INDEXSTR(OBJ_SCAN2FTP_USER		,profilenumber	,ftp_conn->UserName			,SCAN2FTP_USER_MAXLEN	);	/* 認証のためのユーザー名 */
	FUNC_GET_INDEXSTR(OBJ_SCAN2FTP_PASSWD	,profilenumber	,ftp_conn->Password			,SCAN2FTP_PASSWD_MAXLEN	);	/* 認証のためのパスワード */
	
	DPRINTF(("[%s] HostAddress=%s, StoreDirectory=%s, IsPassive=%d, PortNumber=%d, UserName=%s, Password=%s\n",
				__FUNCTION__, ftp_conn->HostAddress, ftp_conn->StoreDirectory, ftp_conn->IsPassive, ftp_conn->PortNumber, ftp_conn->UserName, ftp_conn->Password));
	return;
}
/****************************************************************************
* @par		Scan to FTPの接続確認
* @param	UINT8 profilenumber(input) 接続先profile number
* @return	無し
*
* @par <外部仕様>
*		外部非公開
* @par <内部仕様>
*		接続情報の取得、接続確認、結果の設定を行う。
* @par <M票>
*		M-DCSL-867
*****************************************************************************/
STATIC	void
scanftp_connection_check(UINT8 profilenumber)
{
	INT32	ret_value_ftp = FTPCLIB_ERROR_EXEC;	/* 接続結果 */
	/* FTP接続情報 */
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
	
	/* FTP接続情報の取得 */
	DPRINTF(("[%s] profilenumber=%d\n", __FUNCTION__, profilenumber));
	scanftp_connection_setup(profilenumber,&ftp_conn);
	
	/* FTPホスト接続確認 */
	ret_value_ftp = ftpclib_IsConnect(&ftp_conn);
	
	/* 接続Check結果の登録 */
	switch(ret_value_ftp){
		case FTPCLIB_SUCCESS:			/* 接続成功 */
			scan_check_setresult(SCAN_CHECK_OK);	/* 書込成功 */
			break;
		case FTPCLIB_ERROR_AUTH:		/* 認証エラー */
			scan_check_setresult(SCAN_CHECK_ERR_AUTH);	/* 書込失敗:AuthenticationError */
			break;
		case FTPCLIB_ERROR_TIMEOUT:		/* タイムアウトエラー */
			scan_check_setresult(SCAN_CHECK_ERR_TOUT);	/* 書込失敗:ServerTimeout */
			break;
		case FTPCLIB_ERROR_SYNTAX:		/* シンタックスエラー(内部エラー) */
		case FTPCLIB_ERROR_STATUS:		/* 状態遷移エラー */
		case FTPCLIB_ERROR_EXEC:		/* 実行エラー(サーバ側のエラー) */
		case FTPCLIB_ERROR_SESSION:		/* セッション／設定エラー */
		case FTPCLIB_ERROR_PARAMETER:	/* 引数エラー */
			scan_check_setresult(SCAN_CHECK_ERR_SEND);	/* 書込失敗:SendError */
			break;
		default:
			scan_check_setresult(SCAN_CHECK_ERR_SEND);	/* 書込失敗:SendError */
			DPRINTF(("[%s](%d) Scan to SettingCheck Err\n", __FUNCTION__, __LINE__));
			break;
	}
	return;
}

#ifdef	USE_SCAN2NW
/****************************************************************************
* @par		Scan to NW接続情報設定
* @param	UINT8 profilenumber(input) 接続先profile number
* @param	stcCIFSConnect* cifs_conn(input/output) 
* @return	無し
*
* @par <外部仕様>
*		外部非公開
* @par <内部仕様>
*		Scan to NW接続情報を設定する。
* @par <M票>
*		M-DCSL-867
*****************************************************************************/
STATIC void
scannw_connection_setup(UINT8 profilenumber,stcCIFSConnect* cifs_conn)
{
	/* CIFS情報UTF8変換buffer */
	UINT8	cifs_transUTF[SCAN2FTP_MAXLEN_OF_ALL+1];
	UINT8	AuthenticationMethod = SCANNW_AUTH_METHOD_AUTO;
	
	/* CIFSホスト接続情報設定 */
	FUNC_GET_INDEXSTR(OBJ_SCAN2NW_SVRADR			,profilenumber	,cifs_conn->HostAddress				,CIFSCLIB_HOSTADDR_SIZE);	/* CIFSホスト名またはIPアドレス */
	
	memset(cifs_transUTF, NULL, sizeof(cifs_transUTF));
	FUNC_GET_INDEXSTR(OBJ_SCAN2NW_STDIR				,profilenumber	,cifs_transUTF						,CIFSCLIB_DIRECTORY_SIZE);	/* ファイル保存先フォルダ */
	ConvertUTF8( CONVERT_TO_UTF8, cifs_transUTF, cifs_conn->StoreDirectory, CIFSCLIB_DIRECTORY_SIZE+1);
	
	memset(cifs_transUTF, NULL, sizeof(cifs_transUTF));
	FUNC_GET_INDEXSTRLONG(OBJ_SCAN2NW_USER			,profilenumber	,cifs_transUTF						,CIFSCLIB_USERNAME_SIZE);	/* 認証のためのユーザー名 */
	ConvertUTF8( CONVERT_TO_UTF8, cifs_transUTF, cifs_conn->UserName, CIFSCLIB_USERNAME_SIZE+1);
	
	memset(cifs_transUTF, NULL, sizeof(cifs_transUTF));
	FUNC_GET_INDEXSTR(OBJ_SCAN2NW_PASSWD			,profilenumber, cifs_transUTF						,CIFSCLIB_PASSWORD_SIZE);	/* 認証のためのパスワード */
	ConvertUTF8( CONVERT_TO_UTF8, cifs_transUTF, cifs_conn->Password, CIFSCLIB_PASSWORD_SIZE+1);
	
	FUNC_GET_INDEXVAL(OBJ_SCAN2NW_AUTH_METHOD		,profilenumber	,&AuthenticationMethod				);							/* 認証方法 */
	cifs_conn->AuthenticationMethod = scan_convert_cifs_auth_method(AuthenticationMethod);
	
	FUNC_GET_INDEXSTR(OBJ_SCAN2NW_KERBEROS_SVRADR	,profilenumber	,cifs_conn->kerberosServerAddress	,CIFSCLIB_KERBADDR_SIZE);	/* ケルベロスサーバ名およびIPアドレス */
	
	DPRINTF(("[%s] HostAddress=%s, StoreDirectory=%s, UserName=%s, Password=%s, AuthenticationMethod=%d, kerberosServerAddress=%s\n",
			__FUNCTION__, cifs_conn->HostAddress, cifs_conn->StoreDirectory, cifs_conn->UserName, cifs_conn->Password, cifs_conn->AuthenticationMethod, cifs_conn->kerberosServerAddress));
	return;
}
#endif

#ifdef	USE_SCAN2NW
/****************************************************************************
* @par		Scan to NWの接続確認
* @param	UINT8 profilenumber(input) 接続先profile number
* @return	無し
*
* @par <外部仕様>
*		外部非公開
* @par <内部仕様>
*		接続情報の取得、接続確認、結果の設定を行う。
* @par <M票>
*		M-DCSL-867
*****************************************************************************/
STATIC	VOID
scannw_connection_check(UINT8 profilenumber)
{
	INT32	ret_value_cifs = CIFSCLIB_ERROR_SENDING;	/* 接続結果 */
	/* CIFS接続情報 */
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
	/* CIFS接続情報の取得 */
	scannw_connection_setup(profilenumber,&cifs_conn);
	
	/* CIFSホスト接続確認 */
	ret_value_cifs = cifsclib_IsConnect(&cifs_conn);
	
	/* 接続Check結果の登録 */
	switch(ret_value_cifs){
		case CIFSCLIB_SUCCESS:			/* 接続成功 */
			scan_check_setresult(SCAN_CHECK_OK);	/* 書込成功 */
			break;
		case CIFSCLIB_ERROR_AUTH:		/* 認証エラー */
			scan_check_setresult(SCAN_CHECK_ERR_AUTH);	/* 書込失敗:AuthenticationError */
			break;
		case CIFSCLIB_ERROR_TIMEOUT:	/* タイムアウト */
			scan_check_setresult(SCAN_CHECK_ERR_TOUT);	/* 書込失敗:ServerTimeout */
			break;
		case CIFSCLIB_ERROR_CLKNOREADY:	/* 時計未設定 */
			scan_check_setresult(SCAN_CHECK_ERR_WRDT);	/* 書込失敗:WrongDate&Time */
			break;
		case CIFSCLIB_ERROR_SENDING:	/* 送信エラー:認証、タイムアウト、引数エラー以外のエラー */
		case CIFSCLIB_ERROR_PARAMETER:	/* 引数エラー */
			scan_check_setresult(SCAN_CHECK_ERR_SEND);	/* 書込失敗:SendError */
			break;
		default:
			scan_check_setresult(SCAN_CHECK_ERR_SEND);	/* 書込失敗:SendError */
			DPRINTF(("[%s](%d) Scan to SettingCheck Err\n", __FUNCTION__, __LINE__));
			break;
	}
	return;
}
#endif

#ifdef	USE_SCAN2SFTP
/****************************************************************************
* @par		Scan to SFTP接続情報設定
* @param	UINT8 profilenumber(input) 接続先profilenumber
* @param	stcSFTPConnect* sftp_conn(input/output) SFTP接続情報保管構造体へのPointer
* @return	なし
*
* @par <外部仕様>
*		外部非公開
* @par <内部仕様>
*		Secure Scan to FTP接続情報を設定する。
* @par <M票>
*		
*****************************************************************************/
STATIC	void
scansftp_connection_setup(UINT8 profilenumber,stcSFTPConnect* sftp_conn)
{
	UINT8	sftp_port_num[5 + 1];		/* Port番号 */
	UINT8	l_PairKeyIdx;
	UINT8	l_PubKeyIdx;

	memset( sftp_port_num, NULL, sizeof(sftp_port_num) );
	/* SFTP Server Address */
	FUNC_GET_INDEXSTR( OBJ_SCAN2SFTP_SVRADR, profilenumber, sftp_conn->HostAddress, SCAN2SFTP_SVRADR_MAXLEN );
	/* Server Port Number */
	FUNC_GET_INDEXSTR( OBJ_SCAN2SFTP_PORT, profilenumber, sftp_port_num, sizeof(sftp_port_num) );
	sftp_conn->PortNumber = atoi( (const char *)sftp_port_num );
	/* User認証方式 */
	FUNC_GET_INDEXVAL( OBJ_SCAN2SFTP_AUTH_METHOD, profilenumber, &(sftp_conn->AuthMeth) );
	/* User Name */
	FUNC_GET_INDEXSTR( OBJ_SCAN2SFTP_USER, profilenumber, sftp_conn->UserName, SCAN2SFTP_USER_MAXLEN );
	/* Password */
	FUNC_GET_INDEXSTR( OBJ_SCAN2SFTP_PASSWD, profilenumber, sftp_conn->Password, SCAN2SFTP_PASSWD_MAXLEN );
	/* SFTPサーバ公開鍵 */
	FUNC_GET_INDEXVAL( OBJ_SCAN2SFTP_SERVER_PUBKEY_INDEX, profilenumber, &l_PairKeyIdx );
	sftp_conn->PubKeyIdx = (INT32)l_PairKeyIdx;
	/* MFC鍵ペアのID */
	FUNC_GET_INDEXVAL( OBJ_SCAN2SFTP_KEY_PAIR_INDEX, profilenumber, &l_PubKeyIdx );
	sftp_conn->PairKeyIdx = (INT32)l_PubKeyIdx;
	/* Store Directory */
	FUNC_GET_INDEXSTR( OBJ_SCAN2SFTP_STDIR, profilenumber, sftp_conn->StoreDirectory, SCAN2SFTP_STDIR_MAXLEN );
	return;
}

/****************************************************************************
* @par		Scan to SFTPの接続確認
* @param	UINT8 profilenumber(input) 接続先profile number
* @return	無し
*
* @par <外部仕様>
*		外部非公開
* @par <内部仕様>
*		接続情報の取得、接続確認、結果の設定を行う。
* @par <M票>
*		
*****************************************************************************/
STATIC	void
scansftp_connection_check(UINT8 profilenumber)
{
	INT32	ret_value_sftp = SFTPCLIB_ERROR_BUSY;	/* 接続結果 */

	/* SFTP接続情報 */
	stcSFTPConnect	sftp_conn;
	UINT8	buf_HostAddress[SCAN2SFTP_SVRADR_MAXLEN + 1];
	UINT8	buf_UserName[SCAN2SFTP_USER_MAXLEN + 1];
	UINT8	buf_Password[SCAN2SFTP_PASSWD_MAXLEN + 1];
	UINT8	buf_StoreDirectory[SCAN2SFTP_STDIR_MAXLEN + 1];

	/* 各Parameter初期化処理 */
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

	/* SFTP接続情報の取得 */
	scansftp_connection_setup( profilenumber, &sftp_conn );

	/* SFTPホスト接続確認 */
	ret_value_sftp = sftpclib_IsConnect( &sftp_conn );

	/* 接続Check結果登録 */
	switch( ret_value_sftp ){
		case	SFTPCLIB_ERR_NONE:			/* 接続成功 */
			scan_check_setresult( SCAN_CHECK_OK );
			break;
		case	SFTPCLIB_ERR_SSHINIT:		/* SSH初期化失敗 */
		case	SFTPCLIB_ERR_SUBSYS:		/* SSHサブシステムSFTP起動できない */
		case	SFTPCLIB_ERR_INIT:			/* SFTPの初期化失敗 */
		case	SFTPCLIB_ERR_DIR:			/* ディレクトリが存在していない */
		case	SFTPCLIB_ERR_UPLOAD:		/* SFTPデータアップロード失敗 */
		case	SFTPCLIB_ERR_QUIT:			/* SFTPサーバとの切断失敗 */
		case	SFTPCLIB_ERROR_BUSY:		/* BUSY ERROR */
		case	SFTPCLIB_ERROR_PARAMETER:	/* 引数エラー */
			scan_check_setresult( SCAN_CHECK_ERR_SEND );
			break;
		case	SFTPCLIB_ERR_TIMEOUT:		/* Time Out */
		case	SFTPCLIB_ERR_SSHDISCON:		/* SSHサーバに接続できない */
			scan_check_setresult( SCAN_CHECK_ERR_TOUT );
			break;
		case	SFTPCLIB_ERR_SSHVER:		/* SSHバージョン交換失敗 */
		case	SFTPCLIB_ERR_SSHKEX:		/* SSH鍵交換失敗 */
		case	SFTPCLIB_ERR_USRAUTH:		/* ユーザー認証失敗 */
		case	SFTPCLIB_ERR_SESSION:		/* SSHサーバとのSession作成失敗 */
			scan_check_setresult( SCAN_CHECK_ERR_AUTH );
			break;
		default:
			/* 結果設定：書き込み失敗（Send Error） */
			scan_check_setresult( SCAN_CHECK_ERR_SEND );
			break;
	}
	return;
}
#endif	/* USE_SCAN2SFTP */

/****************************************************************************
* @par		Scan to FTP接続Check開始要求
* @param	UINT8 profilenumber(input) 
* @return	TRUE：正常終了
*			FALSE：異常終了(拒否)
*
* @par <外部仕様>
*		Scan to FTPより接続Check開始要求を受け付ける。(状態によって受付を拒否する。)
*		事前Checkを開始する。
* @par <内部仕様>
*		内部関数(scan_check_start)をCallする。
* @par <M票>
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
* @par		Scan to NW接続Check開始要求
* @param	UINT8 profilenumber(input)
* @return	TRUE：正常終了
*			FALSE：異常終了(拒否)
*
* @par <外部仕様>
*		Scan to NWより接続Check開始要求を受け付ける。(状態によって受付を拒否する。)
*		事前Checkを開始する。
* @par <内部仕様>
*		内部関数(scan_check_start)をCallする。
* @par <M票>
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
* @par		Secure Scan to FTP接続Check開始要求
* @param	UINT8 profilenumber(input)
* @return	TRUE：正常終了
*			FALSE：異常終了(拒否)
*
* @par <外部仕様>
*		Secure Scan to FTPより接続Check開始要求を受け付ける。(状態によって受付を拒否する。)
*		事前Checkを開始する。
* @par <内部仕様>
*		内部関数(scan_check_start)をCallする。
* @par <M票>
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
* @par		Scan to FTP接続Check結果取得
* @param	無し
* @return	SCAN_CHECK_RESULT：Scan to FTP,NW設定Check処理結果保持変数
*
* @par <外部仕様>
*		Scan to FTP接続Check結果を取得する。
* @par <内部仕様>
*		内部関数(scan_check_getresult)をCallする。
*
* @par <M票>
*		M-DCSL-867
*****************************************************************************/
GLOBAL	SCAN_CHECK_RESULT
ScanFTP_Check_GetResult(void)
{
	return scan_check_getresult();
}
/****************************************************************************
* @par		Scan to NW接続Check結果取得
* @param	無し
* @return	SCAN_CHECK_RESULT：Scan to FTP,NW設定Check処理結果保持変数
*
* @par <外部仕様>
*		Scan to NW接続Check結果を取得する。
* @par <内部仕様>
*		内部関数(scan_check_getresult)をCallする。
*
* @par <M票>
*		M-DCSL-867
*****************************************************************************/
GLOBAL	SCAN_CHECK_RESULT
ScanNW_Check_GetResult(void)
{
	return scan_check_getresult();
}

#ifdef	USE_SCAN2SFTP
/****************************************************************************
* @par		Secure Scan to FTP接続Check結果取得
* @param	無し
* @return	SCAN_CHECK_RESULT：Scan to FTP,NW,SFTP設定Check処理結果保持変数
*
* @par <外部仕様>
*		Secure Scan to FTP接続Check結果を取得する。
* @par <内部仕様>
*		内部関数(scan_check_getresult)をCallする。
*
* @par <M票>
*		
*****************************************************************************/
GLOBAL SCAN_CHECK_RESULT
ScanSFTP_Check_GetResult(void)
{
	return scan_check_getresult();
}
#endif	/* USE_SCAN2SFTP */

/*****************************************************************************
* @par ScanToFTP/NW用の解像度を、色数と解像度に分ける
*
* @param	quality						(input)  色数＋読取解像度(SCAN2FTP_QUALITY)
* @param	color						(output) 色数(COLOR_MODE_XXXX)
* @param	resolution					(output) 読取解像度(RESOLUTION_XXXDPI)
* @return
*
* @par <外部仕様>
*
* @par <内部仕様>
*	本関数でColorAuto/GrayAutoの解像度を取得しようとした場合、
*	FTPC_DEFAULT_RESO(Default解像度)を返す
*	※本来は、scanmenu_EiScanGetResolutionにて解像度を取得する
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
* @par 読み取りページ数を保持する（白紙ページを含まない 読み取り完了ページ数）
*
* @param	iAction	(in)	ページ数の制御方法
* @param	ioPage	(out)	ページ数
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
* @par 白紙検出ページ数を保持する
*
* @param	iAction	(in)	白紙ページ数の制御方法
* @param	ioPage	(out)	白紙ページ数
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
* @par 現在の読取ページ総数と白紙検出ページ数を返す
*
* @param	page		(output)白紙を含む、読取完了した総ページ数
* @param	blankpage	(output)白紙検知したページ数
* @return
*
* @par <外部仕様>
*
* @par <内部仕様>
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
* @par スキャン完了後の解除シーケンス
* @param	event		(input) 完了画面が閉じられた要因
*							FTPC_UIEVT_OK	OKボタン押下
*							FTPC_UIEVT_STOP	STOPボタン押下
* @return
*
* @par <外部仕様>
*		完了画面が閉じられたときにコールされる
* @par <内部仕様>
*		FTPClientタスクに、Complete状態からReady状態へ復帰するためのメッセージを送信する
******************************************************************************/
GLOBAL VOID	ftpclient_closeCompleteStatus(INT32 event)
{
#ifdef FTPC_COMPLETE_SEQUENCE
	TASK_MSG_COM_T	com_msg;
	
	com_msg.from_task	= FTPC_APL_TASK;
	
	switch( event )
	{
	case FTPC_UIEVT_OK:		/* OKボタン押下 */
		com_msg.cmd_id	= CMD_CLOSE_COMPLETE_OK;
		break;
	case FTPC_UIEVT_STOP:	/* STOPボタン押下 */
		com_msg.cmd_id	= CMD_CLOSE_COMPLETE_ST;
		break;
	default:				/* その他の引数 */
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
* @par スキャン完了後の解除シーケンス
* @param	event		(input) 完了画面が閉じられた要因
*							CMD_CLOSE_COMPLETE_OK	OKボタン押下
*							CMD_CLOSE_COMPLETE_ST	STOPボタン押下
*			image_id	(input) 画データID
* @return
*
* @par <外部仕様>
*		アプリタスクの状態をCOMPLETE⇒READY状態にする(他の状態からはREADYにならない)
* @par <内部仕様>
*
******************************************************************************/
STATIC VOID	ftpclient_CompleteStatus_end(UINT16 event, INT32 image_id)
{
	UINT32 ScanPage;

	if( ftpscan_state == FTPC_COMPLETE ){
		/* データ送信へ移行するか判定 */
		ftpclient_ScanPage(FTPC_PGCNT_GET, &ScanPage);
		if( (ScanPage == 0 || ScanPage == FTPC_PGCNT_INVALID_VAL) || event == CMD_CLOSE_COMPLETE_ST ) {
			ImageDelete ( image_id );
			/* ステータスを待機状態 */
			ftpscan_state   = FTPC_START_WAIT;
#ifdef	USE_SEPARATE_UI
			cp_Sts_Apl_Entry( CP_STS_APL_FTPC, CP_STS_FTPC_READY );
#endif
		}
		else if( event == CMD_CLOSE_COMPLETE_OK ){
			/* 送信依頼          */
			ftpscan_state   = FTPC_SEND;
			transfer_process(image_id);
		}
	}
	return;
}

/*********************************************************************************************
* @par		Complete状態がタイムアウトしたときメッセージを送信のタイマーをセット
* @param	time(input)	タイムアウト時間(10ms単位)
* @retval	なし
* @par	<外部仕様>
*			time時間後に、Complete解除処理用メッセージを送信する
* @par	<内部仕様>
*			参考：panelTaskMain.c　set_flock_polling_time()				
*********************************************************************************************/
STATIC VOID
set_ftpc_CompleteStatus_end_time(UINT32 time)
{
	TASK_MSG_COM_T		   com_msg;

	DPRINTF(("set_ftpc_CompleteStatus_end_time(%d)\n",time));

	cancel_ftpc_CompleteStatus_end_time();
	/* タイムアウトを設定する */
	com_msg.from_task	= FTPC_APL_TASK;
	com_msg.cmd_id		= CMD_CLOSE_COMPLETE_OK; 
	timer_id			= FOS_SETTIMEMSG( time,qid_ftpclient,*((UINT32*)&com_msg) );

	return;
}


/*********************************************************************************************
* @par		タイムアウト用に登録したタイマーの解除を行う
* @param	なし
* @retval	なし
* @par	<外部仕様>
*			タイムアウトでComplete解除処理用メッセージ送信の解除を行う
* @par	<内部仕様>
*			参考：panelTaskMain.c　cancel_flock_polling_time()
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
