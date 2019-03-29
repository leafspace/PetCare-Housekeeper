/************************************************************************/
/*																		*/
/*	Serio フレームワーク インクルードファイル							*/
/*																		*/
/************************************************************************/

/****** This file *******************************************************/
/*	file name = serio.h													*/
/*	Tabstop   = 4														*/
/*																		*/
/*	Copyright: 2010 - 20xx brother Industries , Ltd.					*/
/*																		*/
/*	$Id: $																*/
/*																		*/
/*	ver 1.0.0 : 2010.03.12 :  新規作成	榎本 							*/
/*																		*/
/************************************************************************/
#ifndef _SERIO_H
#define _SERIO_H

#include	"serio_cmdparam.h"

/*****************************************************************************
 *  Constant value definition
 ****************************************************************************/
/*-- Return Value ----------------------------------------------------------*/
#define	SIO_RC_OK				(0)		/* 正常終了							*/
#define	SIO_RC_NOTFIND			(-1)	/* 指定されたIDに対応するインスタンスが見つからない	*/
#define	SIO_RC_MEMFUL			(-2)	/* メモリフル						*/
#define	SIO_RC_ILSTATUS			(-3)	/* 不正な状態での実行				*/
#define	SIO_RC_INITFAIL			(-4)	/* 初期化失敗						*/
#define	SIO_RC_PARSEERROR		(-5)	/* Parseエラー						*/
#define	SIO_RC_BUILDERROR		(-6)	/* Buildエラー						*/
#define	SIO_RC_PARAMERROR		(-7)	/* Parameterエラー					*/

/*-- Application終了状態に関する定義 ---------------------------------------*/
#define	SERIOFW_APP_COMPLETED				(0)			/* 正常終了												*/
#define	SERIOFW_APP_CANCELEDBY_USER			(0x0001)	/* Userによるキャンセル終了								*/
#define	SERIOFW_APP_ABORTEDBY_TIMEOUT		(0x0002)	/* 無操作によるタイムアウト終了							*/
#define	SERIOFW_APP_ABORTEDBY_INTERNALERR	(0x0100)	/* ServiceErrorによる強制終了（内部エラー）				*/
#define SERIOFW_APP_ABORTEDBY_PARSEERR		(0x0101)	/* ServiceErrorによる強制終了（Parseエラー）			*/
#define	SERIOFW_APP_ABORTEDBY_LINKDOWN		(0x0200)	/* ServiceErrorによる強制終了（LinkDown）				*/
#define	SERIOFW_APP_ABORTEDBY_CONNECTERR	(0x0201)	/* ServiceErrorによる強制終了（Server接続エラー）		*/
#define	SERIOFW_APP_ABORTEDBY_COMTIMEOUT	(0x0202)	/* ServiceErrorによる強制終了（通信タイムアウト）		*/
#define	SERIOFW_APP_ABORTEDBY_PROXYAUTHERR	(0x0300)	/* ServiceErrorによる強制終了（Proxy Serverの認証失敗）	*/
#define	SERIOFW_APP_ABORTEDBY_CERTINVALID	(0x0400)	/* ServiceErrorによる強制終了（証明書エラー）			*/
#define	SERIOFW_APP_ABORTEDBY_CERTEXPIRED	(0x0401)	/* ServiceErrorによる強制終了（証明書有効期限エラー）	*/
#define SERIOFW_APP_ABORTEDBY_ADMINPASSERR	(0x0500)	/* ServiceErrorによる強制終了（管理者パスワード未設定）	*/

/*-- 本体機能とのコマンドコネクタに関する定義 ------------------------------*/
/*==========================================================================*/
/*  コマンド識別															*/
/*==========================================================================*/
enum {
	SERIOFW_CN_APLEXEC = 1,			/* アプリケーション実行通知コネクタ */
	SERIOFW_CN_USERLOCK,			/* ユーザLock						*/
	SERIOFW_CN_USERUNLOCK,			/* ユーザUnlock						*/
	SERIOFW_CN_UI,					/* UI制御指示						*/
	SERIOFW_CN_COPY,				/* COPY実行							*/
	SERIOFW_CN_SENDFAX,				/* FAX送信							*/
	SERIOFW_CN_SCANSEND,			/* SCAN転送							*/
	SERIOFW_CN_DIRECTPRINT,			/* USBダイレクトプリント			*/
	SERIOFW_CN_PRNMEMFAX,			/* メモリ受信FAXプリント			*/
	SERIOFW_CN_PCPRINT,				/* PCプリント						*/
	SERIOFW_CN_PRNSPOOL,			/* Stored Jobプリント				*/
	SERIOFW_CN_DBREAD,				/* DB Read							*/
	SERIOFW_CN_DBWRITE,				/* DB Write							*/
	SERIOFW_CN_DBREADBUNDLE,		/* DB Read Bundle                   */
	SERIOFW_CN_MEDIAUPLOAD,			/* Media Read + Upload				*/
	SERIOFW_CN_SCANUPLOAD,			/* Scan + Upload					*/
	SERIOFW_CN_DOWNLOADPRINT,		/* Download + Print					*/
	SERIOFW_CN_DOWNLOADPRINT2,		/* Download + Print	2				*/
	SERIOFW_CN_DOWNLOADMEDIA,		/* Download + Media					*/
	SERIOFW_CN_DOWNLOADMEDIA2,		/* Download + Media	2				*/
    SERIOFW_CN_SENDMEDIA,       	/* Send from Medium                 */
	SERIOFW_CN_UPLOADSTART,			/* Upload Start(Sub)				*/
	SERIOFW_CN_DOWNLOADSTART,		/* Download Start(Sub)				*/
	SERIOFW_CN_UPLOADCONT,			/* Upload Continue(Sub)				*/
	SERIOFW_CN_DOWNLOADCONT,		/* Download Continue(Sub)			*/
	SERIOFW_CN_STARTBROTHERCP,		/* Start BrotherCP(Sub)				*/
	SERIOFW_CN_SESSIONCLOSE,		/* Session Close					*//* SerioFrameworkで標準登録 */
	SERIOFW_CN_WAIT,				/* Wait								*//* SerioFrameworkで標準登録 */
	SERIOFW_CN_GETLASTUSERLOG,		/* Log取得							*/
	SERIOFW_CN_IOJOBCONT,			/* IOJOB Continue指示(Sub)			*//* SerioFrameworkで標準登録 */
	SERIOFW_CN_NOTFY_DEVSTS,		/* Device Status Notify				*/

	SERIOFW_CN_MAX
};
/*==========================================================================*/
/*  コマンド制御指示タイプ													*/
/*==========================================================================*/
enum {
	SERIOFW_CN_CTRLTYPE_INIT =1,		/* コマンド処理の初期化				*/
	SERIOFW_CN_CTRLTYPE_EXEC,			/* コマンド処理の実行				*/
	SERIOFW_CN_CTRLTYPE_RESTART,		/* コマンド処理停止中からの再開		*/
	SERIOFW_CN_CTRLTYPE_CANCEL,			/* コマンド処理のキャンセル			*/
	SERIOFW_CN_CTRLTYPE_EXIT,			/* コマンド処理の終了				*/
	
	SERIOFW_CN_CTRLTYPE_MAX
};
/*==========================================================================*/
/*  コマンドクラス															*/
/*==========================================================================*/
enum {
	SERIOCMD_CLASS_NONE=0,				/* Unknown							*/
	SERIOCMD_CLASS_SYSJOB,				/* SYS Job							*/
	SERIOCMD_CLASS_LOCK,				/* LOCK/UNLOCK Job					*/
	SERIOCMD_CLASS_DBJOB,				/* DB Job							*/
	SERIOCMD_CLASS_UIJOB,				/* UI Job							*/
	SERIOCMD_CLASS_IOJOB,				/* IO Job							*/
	SERIOCMD_CLASS_WAIT,				/* WAIT Job							*/
	SERIOCMD_CLASS_NTFYSTS,				/* Notify Status					*/
	SERIOCMD_CLASS_SUBCMD,				/* SubCommand Class					*//* 必ず最後にすること!! */
};

/*==========================================================================*/
/*  UI制御指示タイプ															*/
/*==========================================================================*/
//#define	SERIOFW_UIDEMTYPE_CURRENT			(0x0000)	/* （予約）CurrentのUI指示タイプ	*/
#define	SERIOFW_UIDEMTYPE_UISCRIPT_INPUT	(0x0001)	/* UI ScriptによるUI指示(入力要求)	*/
#define	SERIOFW_UIDEMTYPE_UISCRIPT_INFO		(0x0002)	/* UI ScriptによるUI指示(報知)	*/
#define	SERIOFW_UIDEMTYPE_UISCRIPT_LINK		(0x0003)	/* UI ScriptによるUI指示(LINK)	*/
#define	SERIOFW_UIDEMTYPE_UISCRIPT_NULL		(0x0004)	/* UI ScriptによるUI指示(NULL)	*/
#define	SERIOFW_UIDEMTYPE_DEPBASE			(0x0010)	/* 基本機能の表示に切り換え	*/
#define	SERIOFW_UIDEMTYPE_PROCESSING 		(0x0011)	/* UI指示がない時の表示		*/
#define	SERIOFW_UIDEMTYPE_CANCELING			(0x0012)	/* CANCEL中表示				*/
#define	SERIOFW_UIDEMTYPE_ABORTING			(0x0013)	/* ABORT中表示				*/
#define	SERIOFW_UIDEMTYPE_PAUSED			(0x0014)	/* 一時停止中表示			*/
#define	SERIOFW_UIDEMTYPE_CUSTOM			(0x0100)	/* ユーザ定義(下位バイトを識別情報として使用）*/

	
/*-- 本体機能からのEvent通知機能に関する定義 -------------------------------*/
/*==========================================================================*/
/*  Event ID																*/
/*==========================================================================*/
enum {
	SERIOFW_EVID_UIEND=1,				/* UI入力終了通知					*/
	SERIOFW_EVID_JOBSTATUS,				/* ジョブステータス通知				*/
	SERIOFW_EVID_JOBPROGRESS,			/* ジョブ進捗通知					*/
	SERIOFW_EVID_DEVSTATUS,				/* デバイスステータス通知			*/
	SERIOFW_EVID_LOCKRESULT,			/* LOCK/UNLOCK結果通知				*/
	SERIOFW_EVID_DBREADRESULT,			/* DB Read結果通知					*/
	SERIOFW_EVID_DBWRITERESULT,			/* DB Write結果通知					*/
	SERIOFW_EVID_WAITJOBRESULT,			/* WaitJob結果通知					*/
	SERIOFW_EVID_GETLOGRESULT,			/* GetLastUserLog結果通知			*/

	SERIOFW_EVID_MAX
};

/*==========================================================================*/
/*  UI入力終了ステータス													*/
/*==========================================================================*/
enum {
	SERIOFW_UIENDSTS_OK=1,				/* 正常終了（入力確定）				*/
	SERIOFW_UIENDSTS_SKIP,				/* 正常終了（スキップ）				*/
	SERIOFW_UIENDSTS_GO,				/* 正常終了（確定＆実行開始）		*/
	SERIOFW_UIENDSTS_BACK,				/* 前画面移動操作がなされた			*/
	SERIOFW_UIENDSTS_FORWARD,			/* 次画面移動操作がなされた			*/
	SERIOFW_UIENDSTS_PREV,				/* 前の選択候補呼び出しがなされた	*/
	SERIOFW_UIENDSTS_NEXT,				/* 次の選択候補呼び出しがなされた	*/
	SERIOFW_UIENDSTS_CANCEL,			/* 入力キャンセル					*/
	SERIOFW_UIENDSTS_TIMEOUT,			/* TimeOutによる入力キャンセル		*/
	SERIOFW_UIENDSTS_ABORT,				/* 強制中断による終了				*/
	SERIOFW_UIENDSTS_CUSTOM1,			/* カスタムボタン操作				*/

	SERIO_UIENDSTS_MAX
};

/*==========================================================================*/
/*  ジョブ識別																*/
/*==========================================================================*/
enum {
	SERIOFW_IOJOB_COPY=1,				/* COPY実行							*/
	SERIOFW_IOJOB_SENDFAX,				/* FAX送信							*/
	SERIOFW_IOJOB_SCANSEND,				/* SCAN転送							*/
	SERIOFW_IOJOB_DIRECTPRINT,			/* USBダイレクトプリント			*/
	SERIOFW_IOJOB_PRNMEMFAX,			/* メモリ受信FAXプリント			*/
	SERIOFW_IOJOB_PCPRINT,				/* PCプリント						*/
	SERIOFW_IOJOB_PRNSPOOL,				/* Secureプリント					*/
	SERIOFW_IOJOB_MEDIAUPLOAD,			/* Media Read + Upload				*/
	SERIOFW_IOJOB_SCANUPLOAD,			/* Scan + Upload					*/
	SERIOFW_IOJOB_DOWNLOADPRINT,		/* Download + Print					*/
	SERIOFW_IOJOB_DOWNLOADMEDIA,		/* Download + Media					*/
	SERIOFW_IOJOB_DOWNLOADPRINT2,		/* Download + Print2					*/
	SERIOFW_IOJOB_DOWNLOADMEDIA2,		/* Download + Media2					*/
    SERIOFW_IOJOB_SENDMEDIA,            /* Send files in a medium           */
	SERIOFW_IOJOB_MAX
};

/*==========================================================================*/
/*  ジョブステータス														*/
/*==========================================================================*/
enum {
	SERIOFW_JOBSTS_PROCESSING=1,		/* 実行中							*/
	SERIOFW_JOBSTS_PAUSED,				/* 実行停止中						*/
	SERIOFW_JOBSTS_CANCELING,			/* CANCEL中							*/
	SERIOFW_JOBSTS_END,					/* 実行終了							*/

	SERIOFW_JOBSTS_MAX
};

/*==========================================================================*/
/*  ジョブ停止要因															*/
/*==========================================================================*/
enum {
	SERIOFW_JOBSTS_PAUSEDBY_ERROR=1,	/* エラー発生によるジョブ停止（回復orCancel待ち）*/
	SERIOFW_JOBSTS_PAUSEDBY_SCANNEXT,	/* 次ページ原稿セット＆開始待ち		*/

	SERIOFW_IOJOB_PAUSEDBY_MAX
};

/*==========================================================================*/
/*  ジョブキャンセル要因														*/
/*==========================================================================*/
enum {
	SERIOFW_JOBSTS_CANCELBY_USER=1,		/* ユーザ、または、アプリケーションからの指示によるキャンセル*/
	SERIOFW_JOBSTS_CANCELBY_ABORT,		/* エラー検出等、ジョブ実行側の都合による強制的なキャンセル		*/

	SERIOFW_JOBSTS_CANCELBY_MAX
};
	
/*==========================================================================*/
/*  ジョブ終了ステータス													*/
/*==========================================================================*/
enum {
	SERIOFW_JOBSTS_END_COMPLETE=1,		/* 正常終了 */
	SERIOFW_JOBSTS_END_CANCEL,			/* キャンセル終了 */
	SERIOFW_JOBSTS_END_SYSBUSY,			/* リソース不足、システム都合による実行不可終了	*/
	SERIOFW_JOBSTS_END_DEVERR,			/* ハード、メカ要因によるデバイス障害による実行不可終了	*/

	SERIOFW_JOBSTS_END_MAX
};

/*==========================================================================*/
/*  UserOption																*/
/*==========================================================================*/
enum {
	SERIOFW_JOBSTS_OPTION_YES=1,		/* Yes */
	SERIOFW_JOBSTS_OPTION_NO,			/* No */
	SERIOFW_JOBSTS_OPTION_RESTART,		/* 再開 */

	SERIOFW_JOBSTS_OPTION_MAX
};

/*==========================================================================*/
/*  ジョブ進捗タイプ														*/
/*==========================================================================*/
#define	SERIOFW_JOBPROG_PRINTPAGEEND	SERIO_EVTYPE_PAGEEND          /* 1Page印刷終了					*/
#define	SERIOFW_JOBPROG_TRANSEND		SERIO_EVTYPE_TRANSEND         /* 1宛先転送終了					*/
#define	SERIOFW_JOBPROG_UPLOADFINFO		SERIO_EVTYPE_UPLOADFILEINFO   /* Upload File情報通知			*/
#define	SERIOFW_JOBPROG_DOWNLOADFINFO	SERIO_EVTYPE_DOWNLOADFILEINFO /* Download File情報通知			*/
#define	SERIOFW_JOBPROG_UPLOADFILE		SERIO_EVTYPE_UPLOADFILE       /* １File UPLOAD終了			*/
#define	SERIOFW_JOBPROG_DOWNLOADFILE	SERIO_EVTYPE_DOWNLOADFILE     /* １File DOWNLOAD終了			*/

/*****************************************************************************
 *  Type definition
 ****************************************************************************/
/*-- Adapter Callback Function Type --*/
typedef struct SERIO_EV_ADPTR_PARAM_S SERIO_EV_ADPTR_PARAM_T;
struct SERIO_EV_ADPTR_PARAM_S
{
	INT32					AppID;
	INT32					CmdID;
	INT32					EventID;
	UINT16					EventType;
	const char				*EventData;
	const char				*Dest;
	void					*CmdOpt;
};
typedef INT32 (*SerioEvAdptrCB_t)( SERIO_EV_ADPTR_PARAM_T *param );

typedef struct SERIO_CMDEMPTY_ADPTR_PARAM_S SERIO_CMDEMPTY_ADPTR_PARAM_T;
struct SERIO_CMDEMPTY_ADPTR_PARAM_S
{
	INT32					AppID;
};
typedef struct SERIO_SESSTOUT_ADPTR_PARAM_S SERIO_SESSTOUT_ADPTR_PARAM_T;
struct SERIO_SESSTOUT_ADPTR_PARAM_S
{
	INT32					AppID;
};
typedef INT32 (*SerioCmdEmptyAdptrCB_t)( SERIO_CMDEMPTY_ADPTR_PARAM_T *param );
typedef INT32 (*SerioSessionTimeOutCB_t)( SERIO_SESSTOUT_ADPTR_PARAM_T *param );
typedef INT32 (*SerioAppCancelAdptrCB_t)( INT32 AppID );

/*-- Command Parser/Builder Function Type --*/
typedef SerioHandle (*SerioOpenHandleFunc_t)( int in_type );
typedef int (*SerioParserFunc_t)( SerioHandle in_handle, char *in_str, SerioParser **out_parse );
typedef int (*SerioBuilderFunc_t)( SerioHandle in_handle, SerioBuilder *in_build, char **out_str );
typedef void (*SerioCloseHandleFunc_t)( SerioHandle in_handle );

/*-- Connecter Callback Function Type --*/
typedef INT32 (*SerioFwCmdCnFunc_t)( UINT16 CtrlType, void *Param );

/*****************************************************************************
 *  data structure definition.
 ****************************************************************************/
/*-- 外部とのI/Fに関する定義 ------------------------------*/
/*==========================================================================*/
/*  Adapter登録情報															*/
/*==========================================================================*/
typedef struct SERIO_CMD_PARSE_BUILD_S SERIO_CMD_PARSE_BUILD_T;
struct SERIO_CMD_PARSE_BUILD_S
{
	SerioOpenHandleFunc_t	Open;
	SerioParserFunc_t		Parse;
	SerioBuilderFunc_t		Build;
	SerioCloseHandleFunc_t	Close;
};

typedef struct SERIO_ADPTR_ENTRY_S SERIO_ADPTR_ENTRY_T;
struct SERIO_ADPTR_ENTRY_S
{
	SerioEvAdptrCB_t		EvCbFunc;  		/* Eventコールバック関数			*/
	SerioCmdEmptyAdptrCB_t	CmdEmpCbFunc;  	/* CmdEmptyコールバック関数			*/
	SerioAppCancelAdptrCB_t	AppCancelCbFunc;  /* App Cancelコールバック関数		*/
	SerioSessionTimeOutCB_t	SessToutCbFunc;  /* SessionTimeOutコールバック関数	*/
	SERIO_CMD_PARSE_BUILD_T	CmdParsBldr;	/* コマンドParser/Builder関数		*/
};

/*==========================================================================*/
/*  Application登録情報														*/
/*==========================================================================*/
typedef struct SERIO_APP_ENTRY_S SERIO_APP_ENTRY_T;
struct SERIO_APP_ENTRY_S
{
	INT32					AppCode;  	/* Application識別コード			*/
};


/*-- 本体機能とのI/Fに関する定義 ------------------------------*/
/*==========================================================================*/
/*  Connecter																*/
/*==========================================================================*/
/*  コネクタ登録情報														*/
typedef struct SERIO_CNNCTR_ENTRY_S SERIO_CNNCTR_ENTRY_T;
struct SERIO_CNNCTR_ENTRY_S
{
	SerioFwCmdCnFunc_t		Connector;  /* コネクタコールバック関数			*/
};

/*  アプリケーション実行通知コネクタパラメータ								*/
typedef struct SERIO_CNP_APLSTART_S SERIO_CNP_APLSTART_T;
struct SERIO_CNP_APLSTART_S
{
	INT32					AplCode;	/* アプリケーションの起動事象Code	*/
	INT32					AppID;		/* アプリケーションID				*/
};
	
/*  ジョブ停止中から再開制御指示パラメータ									*/
typedef struct SERIO_CNP_RESTART_PARAM_S SERIO_CN_RESTART_PARAM_T;
struct SERIO_CNP_RESTART_PARAM_S
{
	UINT16					Reason;		/* 停止要因							*/
	UINT16					SubCode;	/* サブコード						*/
	void					*Custom;	/* Pause解除パラメータ				*/
};

/*  UI制御指示パラメータ													*/
typedef struct SERIO_CNP_UICOMAND_S SERIO_CNP_UICOMAND_T;
struct SERIO_CNP_UICOMAND_S
{
	UINT16					UIType;		/* UIタイプ							*/
	void					*Param;		/* UIタイプごとに定義されるパラメータ*/
};

/*--- UIタイプごとのパラメータ ----------------------------------------------*/
/*	■UI Script（SERIOFW_UIDEMTYPE_UISCRIPT_INPUT/INFO）パラメータ			
SERIO_UISCREEN_Tはserio_cmpparam.h,SerioUiScriptStub.hにて定義				*/

/*	■Cancel中表示（SERIOFW_UIDEMTYPE_CANCELING）パラメータ					*/
typedef struct SERIO_CNP_UICOMPRM_CANCELING_S SERIO_CNP_UICOMPRM_CANCELING_T;
struct SERIO_CNP_UICOMPRM_CANCELING_S
{
	INT32					Reason;		/* 停止要因							*/
};

/*	■Abort中表示（SERIOFW_UIDEMTYPE_ABORTING）パラメータ					*/
typedef struct SERIO_CNP_UICOMPRM_ABORTING_S SERIO_CNP_UICOMPRM_ABORTING_T;
struct SERIO_CNP_UICOMPRM_ABORTING_S
{
	INT32					Reason;		/* 停止要因							*/
};

/*	■一時停止中表示（SERIOFW_UIDEMTYPE_PAUSE）パラメータ					*/
typedef struct SERIO_CNP_UICOMPRM_PAUSED_S SERIO_CNP_UICOMPRM_PAUSED_T;
struct SERIO_CNP_UICOMPRM_PAUSED_S
{
	UINT16					Reason;		/* 停止要因							*/
	UINT16					SubCode;	/* 停止要因サブコード				*/
};
/*--------------------------------------------------------------------------*/

/* LOCKコマンド制御指示パラメータ											
SERIO_CNP_LOCKUSER_Tはserio_cmpparam.h,SerioCommandStub.hにて定義			*/

/* UNLOCKコマンド制御指示パラメータ											
SERIO_CNP_UNLOCKUSER_Tはserio_cmpparam.h,SerioCommandStub.hにて定義			*/

/* DB Readコマンド制御指示パラメータ										
SERIO_CNP_DBJOB_READ_Tはserio_cmpparam.h,SerioCommandStub.hにて定義			*/

/* DB Writeコマンド制御指示パラメータ										
SERIO_CNP_DBJOB_WRITE_Tはserio_cmpparam.h,SerioCommandStub.hにて定義		*/

/* IOJOB:Copyコマンド制御指示パラメータ										
SERIO_CNP_IOJOB_COPY_Tはserio_cmpparam.h,SerioCommandStub.hにて定義			*/

/* IOJOB:Send Faxコマンド制御指示パラメータ									
SERIO_CNP_IOJOB_SENDFAX_Tはserio_cmpparam.h,SerioCommandStub.hにて定義		*/

/* IOJOB:Scan送信コマンド制御指示パラメータ									
SERIO_CNP_IOJOB_SCANSEND_Tはserio_cmpparam.h,SerioCommandStub.hにて定義		*/

/* IOJOB:メモリ受信FAXプリントコマンド制御指示パラメータ					
SERIO_CNP_IOJOB_PRNMEMFAX_Tはserio_cmpparam.h,SerioCommandStub.hにて定義	*/

/* IOJOB:DirectPrintコマンド制御指示パラメータ								
SERIO_CNP_IOJOB_DIRECTPRN_Tはserio_cmpparam.h,SerioCommandStub.hにて定義	*/

/* IOJOB:Storedジョブプリントコマンド制御指示パラメータ						
SERIO_CNP_IOJOB_PRNSPOOL_Tはserio_cmpparam.h,SerioCommandStub.hにて定義		*/

/* IOJOB:PCプリントコマンド制御指示パラメータ								
SERIO_CNP_IOJOB_PCPRN_Tはserio_cmpparam.h,SerioCommandStub.hにて定義		*/

/* IOJOB:Media Read + Uploadコマンド制御指示パラメータ						
SERIO_CNP_IOJOB_MEDIAUPLOAD_Tはserio_cmpparam.h,SerioCommandStub.hにて定義	*/

/* IOJOB:Scan + Uploadコマンド制御指示パラメータ							
SERIO_CNP_IOJOB_SCANUPLOAD_Tはserio_cmpparam.h,SerioCommandStub.hにて定義	*/

/* IOJOB:Downpload + Printコマンド制御指示パラメータ						
SERIO_CNP_IOJOB_DOWNLOADPRINT_Tはserio_cmpparam.h,SerioCommandStub.hにて定義*/

/* IOJOB:Downpload + Mediaコマンド制御指示パラメータ						
SERIO_CNP_IOJOB_DOWNLOADMEDIA_Tはserio_cmpparam.h,SerioCommandStub.hにて定義*/



/*==========================================================================*/
/*  Event Parameter															*/
/*==========================================================================*/

/*--------------------------------------------------------------------------*/
/*	UI終了イベントパラメータ												*/
/*--------------------------------------------------------------------------*/
typedef struct SERIO_EVP_UIEND_S SERIO_EVP_UIEND_T;
struct SERIO_EVP_UIEND_S
{
	UINT16					UIType;		/* UIタイプ							*/
	UINT16					Result;		/* UI処理実行結果					*/
	UINT16					Status;		/* UI入力終了ステータス				*/
	void					*InputResult;/* UI入力結果						*/
	INT32					InputResultSize;/* UI入力結果のデータサイズ		*/
};

/*-----UI終了イベント詳細パラメータ ----------------------------------------*/

/*	■SERIOFW_UIDEMTYPE_LINKパラメータ					*/
typedef struct SERIO_EVP_UIEND_LINK_S SERIO_EVP_UIEND_LINK_T;
struct SERIO_EVP_UIEND_LINK_S
{
	char					*Href;		/* 選択されたリンクの参照先			*/
};

/*--------------------------------------------------------------------------*/
/*	ジョブステータスイベントパラメータ										*/
/*--------------------------------------------------------------------------*/

/* ■ジョブステータスのDetailの詳細パラメータ								*/
typedef struct SERIO_JOBSTS_PARAM_DETAIL_S SERIO_JOBSTS_PARAM_DETAIL_T;
struct SERIO_JOBSTS_PARAM_DETAIL_S
{
	UINT32						ReasonDetail;	/* 変化要因詳細				*/
	UINT32						UstatusCode;	/* UstatusCode				*/
	SERIO_USEROPTIONS_T			UserOption;		/* 操作仕様					*/
	UINT8*						Description;	/* 説明						*/
};

/* ■ジョブステータス=PROCESSING時の詳細パラメータ							*/
typedef struct SERIO_JOBSTS_PROCESSING_S SERIO_JOBSTS_PROCESSING_T;
struct SERIO_JOBSTS_PROCESSING_S
{
	UINT16						dummy;		/* dummy						*/
	SERIO_JOBSTS_PARAM_DETAIL_T	Detail;		/* ジョブ状態更新詳細情報		*/
};
	
/* ■ジョブステータス=PAUSED時の詳細パラメータ								*/
typedef struct SERIO_JOBSTS_PAUSED_S SERIO_JOBSTS_PAUSED_T;
struct SERIO_JOBSTS_PAUSED_S
{
	UINT16						Reason;		/* ジョブ停止要因				*/
	UINT16						SubCode;	/* 停止要因サブコード			*/
	SERIO_JOBSTS_PARAM_DETAIL_T	Detail;		/* ジョブ状態更新詳細情報		*/
};
	
/* ■ジョブステータス=CANCELING時の詳細パラメータ							*/
typedef struct SERIO_JOBSTS_CANCELING_S SERIO_JOBSTS_CANCELING_T;
struct SERIO_JOBSTS_CANCELING_S
{
	UINT16						Reason;		/* ジョブキャンセル要因			*/
	SERIO_JOBSTS_PARAM_DETAIL_T	Detail;		/* ジョブ状態更新詳細情報		*/
};

/* ■ジョブステータス=END時の詳細パラメータ									*/
/****************************************************************************/
/* ＜補足＞																	*/
/* SERIO_EV_JOB_FINISH_Tの定義は、serio_cmpparam.h,SerioCommandStub.hに		*/
/* あります																	*/
/****************************************************************************/
typedef struct SERIO_JOBSTS_END_S SERIO_JOBSTS_END_T;
struct SERIO_JOBSTS_END_S
{
	UINT16						Reason;		/* ジョブ終了要因				*/
	SERIO_EV_JOB_FINISH_T		Detail;		/* ジョブ実行結果詳細情報			*/
};


/* ジョブステータス詳細パラメータ(Union) 									*/
typedef union SERIO_EVP_JOBSTS_PARAM_U SERIO_EVP_JOBSTS_PARAM_U;
union SERIO_EVP_JOBSTS_PARAM_U
{
	SERIO_JOBSTS_PROCESSING_T	Processing;	/* Processing時のパラメータ		*/
	SERIO_JOBSTS_PAUSED_T		Paused;		/* Paused時のパラメータ			*/
	SERIO_JOBSTS_CANCELING_T	Canceling;	/* Canceling時のパラメータ		*/
	SERIO_JOBSTS_END_T			End;		/* End時のパラメータ			*/
};


/*-----ジョブステータスパラメータ ------------------------------------------*/
typedef struct SERIO_EVP_JOBSTS_S SERIO_EVP_JOBSTS_T;
struct SERIO_EVP_JOBSTS_S
{
	UINT16						JobID;		/* ジョブ識別						*/
	UINT16						Status;		/* ジョブステータス					*/
	SERIO_EVP_JOBSTS_PARAM_U	Param;		/* ジョブステータス詳細パラメータ	*/
};


/*--------------------------------------------------------------------------*/
/*  ジョブ進捗イベントパラメータ											*/
/*--------------------------------------------------------------------------*/
/* ジョブ進捗詳細パラメータ(Union)											*/
/****************************************************************************/
/* ＜補足＞																	*/
/* Unionメンバの構造体の定義は、serio_cmpparam.h,SerioCommandStub.hに		*/
/* あります																	*/
/****************************************************************************/
typedef union SERIO_EVP_JOBPROG_PARAM_U SERIO_EVP_JOBPROG_PARAM_U;
union SERIO_EVP_JOBPROG_PARAM_U
{
	SERIO_EV_PAGEEND_T			PageEnd;		/* 1Page印刷終了			*/
	SERIO_EV_TRANSEND_T			TransferEnd;	/* 1宛先通信終了			*/
	SERIO_EV_UPLOADFILEINFO_T	UpldFileInfo;	/* Uploadファイル情報通知	*/
	SERIO_EV_DOWNLOADFILEINFO_T	DwldFileInfo;	/* Downloadファイル情報通知	*/
	SERIO_EV_UPLOADFILE_T		FileUploadEnd;	/* 1ファイルUpload完了通知	*/
	SERIO_EV_DOWNLOADFILE_T		FileDownloadEnd;/* 1ファイルDownload完了通知	*/
};
	
/*----- ジョブ進捗パラメータ -----------------------------------------------*/
typedef struct SERIO_EVP_JOBPROGRESS_S SERIO_EVP_JOBPROGRESS_T;
struct SERIO_EVP_JOBPROGRESS_S
{
	UINT16						JobID;	/* ジョブ識別						*/
	UINT16						Type;	/* 進捗情報タイプ					*/
	SERIO_EVP_JOBPROG_PARAM_U	Param;	/* ジョブ進捗詳細パラメータ			*/
};


/*--------------------------------------------------------------------------*/
/*  デバイスステータスパラメータ											*/
/*--------------------------------------------------------------------------*/
typedef union SERIO_EVP_DEVSTS_PARAM_U SERIO_EVP_DEVSTS_PARAM_U;
union SERIO_EVP_DEVSTS_PARAM_U
{
	SERIO_EV_DEVSTS_CHANGE_T	Notify;			/* DeviceStatus状態変化通知	*/
	SERIO_EV_NTFT_ABOTED_T		Abort;			/* 完了通知	*/
};
/****************************************************************************/
/* ＜補足＞																	*/
/* SERIO_EV_DEVSTS_CHANGE_Tの定義は、serio_cmpparam.hに						*/
/* あります																	*/
/****************************************************************************/
typedef struct SERIO_EVP_DEVSTS_S SERIO_EVP_DEVSTS_T;
struct SERIO_EVP_DEVSTS_S
{
	UINT16						Type;		/*  */
	SERIO_EVP_DEVSTS_PARAM_U	Param;		/* 実行結果						*/
};


/*--------------------------------------------------------------------------*/
/*  LOCK/UNLOCK結果イベントパラメータ										*/
/*--------------------------------------------------------------------------*/
/****************************************************************************/
/* ＜補足＞																	*/
/* SERIO_EV_LOCK_RSLT_Tの定義は、serio_cmpparam.h,SerioCommandStub.hに		*/
/* あります																	*/
/****************************************************************************/
typedef struct SERIO_EVP_LOCKRESULT_S SERIO_EVP_LOCKRESULT_T;
struct SERIO_EVP_LOCKRESULT_S
{
	SERIO_EV_LOCK_RSLT_T	Result;	/* 	実行結果					*/
};



/*--------------------------------------------------------------------------*/
/*  DB Read結果通知イベントパラメータ										*/
/*--------------------------------------------------------------------------*/
/****************************************************************************/
/* ＜補足＞																	*/
/* SERIO_EV_DBREAD_Tの定義は、serio_cmpparam.h,SerioCommandStub.hに			*/
/* あります																	*/
/****************************************************************************/
typedef struct SERIO_EVP_DBREADRESULT_S SERIO_EVP_DBREADRESULT_T;
struct SERIO_EVP_DBREADRESULT_S
{
	SERIO_EV_DBREAD_T		Result;	/* 	実行結果					*/
};



/*--------------------------------------------------------------------------*/
/*  DB Write結果通知イベントパラメータ										*/
/*--------------------------------------------------------------------------*/
/****************************************************************************/
/* ＜補足＞																	*/
/* SERIO_EV_DBWRITE_Tの定義は、serio_cmpparam.h,SerioCommandStub.hに		*/
/* あります																	*/
/****************************************************************************/
typedef struct SERIO_EVP_DBWRITERESULT_S SERIO_EVP_DBWRITERESULT_T;
struct SERIO_EVP_DBWRITERESULT_S
{
	SERIO_EV_DBWRITE_T		Result;	/* 	実行結果					*/
};


/*--------------------------------------------------------------------------*/
/*  WaitJbb結果通知イベントパラメータ										*/
/*--------------------------------------------------------------------------*/
/****************************************************************************/
/* ＜補足＞																	*/
/* SERIO_EV_WAITTOUT_Tの定義は、serio_cmpparam.h,SerioCommandStub.hに		*/
/* あります																	*/
/****************************************************************************/
typedef struct SERIO_EVP_WAITJOBRESULT_S SERIO_EVP_WAITJOBRESULT_T;
struct SERIO_EVP_WAITJOBRESULT_S
{
	SERIO_EV_WAITTOUT_T		Result;	/* 	実行結果					*/
};


/*--------------------------------------------------------------------------*/
/*  GetLastUserLog結果通知イベントパラメータ										*/
/*--------------------------------------------------------------------------*/
/****************************************************************************/
/* ＜補足＞																	*/
/* SERIO_EV_LASTUSERLOG_Tの定義は、serio_cmpparam.h,SerioCommandStub.hに	*/
/* あります																	*/
/****************************************************************************/
typedef struct SERIO_EVP_LASTUSERLOGRESULT_S SERIO_EVP_LASTUSERLOGRESULT_T;
struct SERIO_EVP_LASTUSERLOGRESULT_S
{
	SERIO_EV_LASTUSERLOG_T	Result;	/* 	実行結果					*/
};


/*****************************************************************************
 *  prototype definition.
 ****************************************************************************/
extern INT32 SerioFrameworkInit( void );

extern INT32 SerioFwCmdCnnctrRegist( UINT16 Identify, SERIO_CNNCTR_ENTRY_T *Entry );
extern INT32 SerioFwCustomCmdCnnctrRegist( INT32 AppID, UINT16 Identify, SERIO_CNNCTR_ENTRY_T *Entry );
extern INT32 SerioFwEventNotify( UINT16 EventID, void *Param );
extern INT32 SerioFwAdptrCreate( SERIO_ADPTR_ENTRY_T *Entry );
extern INT32 SerioFwAppCreate( INT32 AdaptID, SERIO_APP_ENTRY_T *Entry );
extern INT32 SerioFwAppOpen( INT32 AppID );
extern INT32 SerioFwAppCancel( INT32 AppID, INT32 Reason );
extern INT32 SerioFwAppAbort( INT32 AppID, INT32 Reason );
extern INT32 SerioFwAppAbortConfirm( INT32 AppID );
extern INT32 SerioFwCmdAdd( INT32 AppID, UINT8 *CmdData, UINT8 *Opt, INT32 OptSize );
extern INT32 SerioFwCmdCancel( INT32 CmdID );
extern INT32 SerioFwCmdEnd( INT32 CmdID, const char *EvData, const char *Dest, SerioHandle BuildHandle );
extern INT32 SerioFwAppClose( INT32 AppID );
extern INT32 SerioFwAppTerrm( INT32 AppID );
extern INT32 SerioFwEvtCreate( INT32 CmdID, const char *EvData, const char *Dest, SerioHandle BuildHandle );
extern INT32 SerioFwEvtSendEnd( INT32 EvtID );
extern INT32 SerioFwEvtCancel( INT32 EvtID );

extern INT32 SerioFwCallCnnctr( INT32 CmdID, UINT16 CtrlType, void *Param );
extern INT32 SerioFwCallAppLinkCnnctr( INT32 AppID, UINT16 CnID, UINT16 CtrlType, void *Param );

extern INT32 SerioFwGetCmdReqData( INT32 CmdID, void **Param );
extern INT32 SerioFwGetLinkConnctr( INT32 CmdID );
extern INT32 SerioFwGetAppList( INT32 AppIDList[], INT32 max_num );
extern INT32 SerioFwGetCmdList( INT32 AppID, UINT16 CmdType, INT32 CmdIDList[], INT32 max_num );
extern INT32 SerioFwGetCmdClassList( INT32 AppID, UINT16 CmdClass, INT32 CmdIDList[], INT32 max_num );
extern INT32 SerioFwGetExecCmdList( INT32 AppID, UINT16 CmdType, INT32 CmdIDList[], INT32 max_num );
extern INT32 SerioFwGetExecCmdClassList( INT32 AppID, UINT16 CmdClass, INT32 CmdIDList[], INT32 max_num );
extern INT32 SerioFwGetAdptrEntry( INT32 AppID, SERIO_ADPTR_ENTRY_T *Entry );
extern INT32 SerioFwGetAppEntry( INT32 AppID, SERIO_APP_ENTRY_T *Entry );
extern BOOL  SerioFwIsNonRsltCmd( INT32 CmdID );
extern BOOL  SerioFwIsCmdCanceling( INT32 CmdID );

#endif	/* _SERIO_H */
