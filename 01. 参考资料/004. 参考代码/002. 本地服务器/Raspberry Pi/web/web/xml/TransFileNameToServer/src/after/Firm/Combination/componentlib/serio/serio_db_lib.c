/************************************************************************/
/*																		*/
/*	Serio 内部データベース モジュール群									*/
/*																		*/
/************************************************************************/

/****** This file *******************************************************/
/*	file name = serio_db_lib.c											*/
/*	Tabstop   = 4														*/
/*																		*/
/*	Copyright: 2010 - 20xx brother Industries , Ltd.					*/
/*																		*/
/*	$Id$																*/
/*																		*/
/*	ver 1.0.0 : 2010.07.28 :  新規作成	榎本 							*/
/*																		*/
/************************************************************************/
/****** インクルード・ファイル ******************************************/
#include "spec.h"
#include "stdtype.h"
#include "fos.h"
#include "debug.h"
#include "common.h"

#include "serio.h"
#include "serio_mem.h"
#include "serio_object.h"

#include "serio_app_object.h"
#include "serio_cmd_object.h"
#include "serio_evt_object.h"
#include "serio_adapter_object.h"
#include "serio_connecter_object.h"
#include "serio_cmdprocess.h"


/*****************************************************************************
 * Local static variables.
 ****************************************************************************/
/*============================================================================
 *	Command Type correspond table
 *	＜注釈＞
 *	・コマンドタイプが追加された時はレコードを追加する
 *	・コマンドタイプ追加はserio_cmdparam.hのマクロ追加に連動する
 *==========================================================================*/
typedef struct CmdTypeCorrespond_s	CmdTypeCorrespond_t;
struct CmdTypeCorrespond_s
{
	INT32			CmdType;		/* コマンドタイプ		*/
	UINT16			CmdClass;		/* コマンドクラス		*/
	UINT16			Connector;		/* 対応するコネクタ		*/
	const UINT16	*DependCmd;		/* 依存関係のあるコマンド群（SERIOCMD_CLASS_SUBCMD時のみ） */
	UINT16			DependCmdNum;	/* 依存関係のあるコマンドの数（SERIOCMD_CLASS_SUBCMD時のみ） */
};

STATIC const UINT16	UploadStartDepends[] = {
	SERIO_CMDTYPE_SCANUPLOAD,
	SERIO_CMDTYPE_MEDIAUPLOAD,
};
#define	UPLOADSTART_DEPENDS_NUM		(sizeof(UploadStartDepends)/sizeof(UploadStartDepends[0]))

STATIC const UINT16	DownloadStartDepends[] = {
	SERIO_CMDTYPE_DOWNLOADPRINT2,
	SERIO_CMDTYPE_DOWNLOADMEDIA2,
};
#define	DOWNLOADSTART_DEPENDS_NUM		(sizeof(DownloadStartDepends)/sizeof(DownloadStartDepends[0]))


STATIC const UINT16	UploadContDepends[] = {
	SERIO_CMDTYPE_SCANUPLOAD,
	SERIO_CMDTYPE_MEDIAUPLOAD,
};
#define	UPLOADCONT_DEPENDS_NUM		(sizeof(UploadContDepends)/sizeof(UploadContDepends[0]))

STATIC const UINT16	DownloadContDepends[] = {
	SERIO_CMDTYPE_DOWNLOADPRINT2,
	SERIO_CMDTYPE_DOWNLOADMEDIA2,
};
#define	DOWNLOADCONT_DEPENDS_NUM		(sizeof(DownloadStartDepends)/sizeof(DownloadStartDepends[0]))

STATIC const UINT16	StartBrotherCPDepends[] = {
	SERIO_CMDTYPE_DOWNLOADPRINT2,
};
#define	STARTBROTHERCP_DEPENDS_NUM		(sizeof(StartBrotherCPDepends)/sizeof(StartBrotherCPDepends[0]))

STATIC const UINT16	IoJobContDepends[] = {	/* ！注意！　Continue指示対応IOJOBが増えたら追加すること */
	SERIO_CMDTYPE_COPY,
	SERIO_CMDTYPE_SENDFAX,
	SERIO_CMDTYPE_SCANTO,
	SERIO_CMDTYPE_DIRPRINT,
	SERIO_CMDTYPE_PRINTFAXMEM,
	SERIO_CMDTYPE_PRINTSPOOL,
	SERIO_CMDTYPE_PCPRINT,
};
#define	IOJOBCONT_DEPENDS_NUM		(sizeof(IoJobContDepends)/sizeof(IoJobContDepends[0]))

STATIC const CmdTypeCorrespond_t	CmdTypeCorrespondTbl[] = {
    {SERIO_CMDTYPE_USERLOCK       ,SERIOCMD_CLASS_LOCK    ,SERIOFW_CN_USERLOCK        ,NULL                 ,0                         },
    {SERIO_CMDTYPE_USERUNLOCK     ,SERIOCMD_CLASS_LOCK    ,SERIOFW_CN_USERUNLOCK      ,NULL                 ,0                         },
    {SERIO_CMDTYPE_DISPFORM       ,SERIOCMD_CLASS_UIJOB   ,SERIOFW_CN_UI              ,NULL                 ,0                         },
    {SERIO_CMDTYPE_DISPINFO       ,SERIOCMD_CLASS_UIJOB   ,SERIOFW_CN_UI              ,NULL                 ,0                         },
    {SERIO_CMDTYPE_READDB         ,SERIOCMD_CLASS_DBJOB   ,SERIOFW_CN_DBREAD          ,NULL                 ,0                         },
    {SERIO_CMDTYPE_READDBBUNDLE   ,SERIOCMD_CLASS_DBJOB   ,SERIOFW_CN_DBREADBUNDLE    ,NULL                 ,0                         },
    {SERIO_CMDTYPE_WRITEDB        ,SERIOCMD_CLASS_DBJOB   ,SERIOFW_CN_DBWRITE         ,NULL                 ,0                         },
    {SERIO_CMDTYPE_COPY           ,SERIOCMD_CLASS_IOJOB   ,SERIOFW_CN_COPY            ,NULL                 ,0                         },
    {SERIO_CMDTYPE_SENDFAX        ,SERIOCMD_CLASS_IOJOB   ,SERIOFW_CN_SENDFAX         ,NULL                 ,0                         },
    {SERIO_CMDTYPE_SCANTO         ,SERIOCMD_CLASS_IOJOB   ,SERIOFW_CN_SCANSEND        ,NULL                 ,0                         },
    {SERIO_CMDTYPE_DIRPRINT       ,SERIOCMD_CLASS_IOJOB   ,SERIOFW_CN_DIRECTPRINT     ,NULL                 ,0                         },
    {SERIO_CMDTYPE_PRINTFAXMEM    ,SERIOCMD_CLASS_IOJOB   ,SERIOFW_CN_PRNMEMFAX       ,NULL                 ,0                         },
    {SERIO_CMDTYPE_PRINTSPOOL     ,SERIOCMD_CLASS_IOJOB   ,SERIOFW_CN_PRNSPOOL        ,NULL                 ,0                         },
    {SERIO_CMDTYPE_PCPRINT        ,SERIOCMD_CLASS_IOJOB   ,SERIOFW_CN_PCPRINT         ,NULL                 ,0                         },
    {SERIO_CMDTYPE_SCANUPLOAD     ,SERIOCMD_CLASS_IOJOB   ,SERIOFW_CN_SCANUPLOAD      ,NULL                 ,0                         },
    {SERIO_CMDTYPE_MEDIAUPLOAD    ,SERIOCMD_CLASS_IOJOB   ,SERIOFW_CN_MEDIAUPLOAD     ,NULL                 ,0                         },
    {SERIO_CMDTYPE_DOWNLOADPRINT  ,SERIOCMD_CLASS_IOJOB   ,SERIOFW_CN_DOWNLOADPRINT   ,NULL                 ,0                         },
    {SERIO_CMDTYPE_DOWNLOADPRINT2 ,SERIOCMD_CLASS_IOJOB   ,SERIOFW_CN_DOWNLOADPRINT2  ,NULL                 ,0                         },
    {SERIO_CMDTYPE_DOWNLOADMEDIA  ,SERIOCMD_CLASS_IOJOB   ,SERIOFW_CN_DOWNLOADMEDIA   ,NULL                 ,0                         },
    {SERIO_CMDTYPE_DOWNLOADMEDIA2 ,SERIOCMD_CLASS_IOJOB   ,SERIOFW_CN_DOWNLOADMEDIA2  ,NULL                 ,0                         },
    {SERIO_CMDTYPE_MEDIASEND      ,SERIOCMD_CLASS_IOJOB   ,SERIOFW_CN_SENDMEDIA       ,NULL                 ,0                         },
    {SERIO_CMDTYPE_UPLOADSTART    ,SERIOCMD_CLASS_SUBCMD  ,SERIOFW_CN_UPLOADSTART     ,UploadStartDepends   ,UPLOADSTART_DEPENDS_NUM   },
    {SERIO_CMDTYPE_DWLOADSTART    ,SERIOCMD_CLASS_SUBCMD  ,SERIOFW_CN_DOWNLOADSTART   ,DownloadStartDepends ,DOWNLOADSTART_DEPENDS_NUM },
    {SERIO_CMDTYPE_UPLOADCONT     ,SERIOCMD_CLASS_SUBCMD  ,SERIOFW_CN_UPLOADCONT      ,UploadContDepends    ,UPLOADCONT_DEPENDS_NUM    },
    {SERIO_CMDTYPE_DWLOADCONT     ,SERIOCMD_CLASS_SUBCMD  ,SERIOFW_CN_DOWNLOADCONT    ,DownloadContDepends  ,DOWNLOADCONT_DEPENDS_NUM  },
    {SERIO_CMDTYPE_STARTBROTHERCP ,SERIOCMD_CLASS_SUBCMD  ,SERIOFW_CN_STARTBROTHERCP  ,StartBrotherCPDepends,STARTBROTHERCP_DEPENDS_NUM},
    {SERIO_CMDTYPE_CLOSESESSION   ,SERIOCMD_CLASS_SYSJOB  ,SERIOFW_CN_SESSIONCLOSE    ,NULL                 ,0                         },
    {SERIO_CMDTYPE_WAIT           ,SERIOCMD_CLASS_WAIT    ,SERIOFW_CN_WAIT            ,NULL                 ,0                         },
    {SERIO_CMDTYPE_GETLASTUSERLOG ,SERIOCMD_CLASS_DBJOB   ,SERIOFW_CN_GETLASTUSERLOG  ,NULL                 ,0                         },
    {SERIO_CMDTYPE_IOJOBCONT      ,SERIOCMD_CLASS_SUBCMD  ,SERIOFW_CN_IOJOBCONT       ,IoJobContDepends     ,IOJOBCONT_DEPENDS_NUM     },
    {SERIO_CMDTYPE_NTFYDEVSTS     ,SERIOCMD_CLASS_NTFYSTS ,SERIOFW_CN_NOTFY_DEVSTS    ,NULL                 ,0                         },
};

/*===========================================================================
 * Ack URL refernce table
 *	＜注釈＞
 *	・Ack URLの種別が追加された時は二次元目の要素数を増やす
 *	・コマンドタイプが追加された時は一次元のレコードを追加する
 *	・この2次元配列の 列(横の並び)の順番は、enum "Ack Type"(serio_cmdparam.h) 
 *	  の並び順に同期していなければならない
 *	・上記追加はともにserio_cmdparam.hのマクロ追加に連動する
 *	・UI JOBコマンドはUI Script内で定義されるためここには登録しない（個別処理）
 ===========================================================================*/
STATIC const INT16	AckURLSelectTBl[][11] = {
	/*--- Cmd Type ------------+-- コマンド受信ACK --------+-- JOB開始ACK -------------+-- JOB終了ACK -----------------*/
	/*-------------------------+-- 1Page印刷終了ACK -------+-- 1あて先通信終了ACK -----+-- Upload File Info. ----------*/
	/*-------------------------+-- Upload File ------------+-- Download File ----------+---Download File Info. --------*/
	/*-------------------------+-- Job状態通知ACK ---------+---------------------------+-------------------------------*/
	{SERIO_CMDTYPE_USERLOCK,	ERROR,						ERROR,						USERLOCK_JOBEND_URL,		
								ERROR,						ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								ERROR},
	
	{SERIO_CMDTYPE_USERUNLOCK,	ERROR,						ERROR,						USERUNLOCK_JOBEND_URL,
								ERROR,						ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								ERROR},
	
	{SERIO_CMDTYPE_READDB,		ERROR,						ERROR,						DBJOB_READ_JOBEND_URL,
								ERROR,						ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								ERROR},

	{SERIO_CMDTYPE_READDBBUNDLE,ERROR,						ERROR,						DBJOB_READBUNDLE_JOBEND_URL,
								ERROR,						ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								ERROR},
	
	{SERIO_CMDTYPE_WRITEDB,		ERROR,						ERROR,						DBJOB_WRITE_JOBEND_URL,
								ERROR,						ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								ERROR},
	
	{SERIO_CMDTYPE_COPY,		IOJOB_COPY_CMDRECV_URL,		IOJOB_COPY_JOBSTART_URL,	IOJOB_COPY_JOBEND_URL,
								IOJOB_COPY_PRINTPAGE_URL,	ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								IOJOB_COPY_JOBSTATUS_URL},
	
	{SERIO_CMDTYPE_SENDFAX,		IOJOB_SENDFAX_CMDRECV_URL,	IOJOB_SENDFAX_JOBSTART_URL,	IOJOB_SENDFAX_JOBEND_URL,
								ERROR,						IOJOB_SENDFAX_DATACOMM_URL,	ERROR,
								ERROR,						ERROR,						ERROR,
								IOJOB_SENDFAX_JOBSTATUS_URL},
	
	{SERIO_CMDTYPE_SCANTO,		IOJOB_SCANTO_CMDRECV_URL,	IOJOB_SCANTO_JOBSTART_URL,	IOJOB_SCANTO_JOBEND_URL,
								ERROR,						IOJOB_SCANTO_DATACOMM_URL,	ERROR,
								ERROR,						ERROR,						ERROR,
								IOJOB_SCANTO_JOBSTATUS_URL},
	
	{SERIO_CMDTYPE_DIRPRINT,	IOJOB_DIRPRN_CMDRECV_URL,	IOJOB_DIRPRN_JOBSTART_URL,	IOJOB_DIRPRN_JOBEND_URL,
								IOJOB_DIRPRN_PRINTPAGE_URL,	ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								IOJOB_DIRPRN_JOBSTATUS_URL},
	
	{SERIO_CMDTYPE_PRINTFAXMEM,	IOJOB_PRNMFAX_CMDRECV_URL,	IOJOB_PRNMFAX_JOBSTART_URL,	IOJOB_PRNMFAX_JOBEND_URL,
								IOJOB_PRNMFAX_PRINTPAGE_URL,ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								IOJOB_PRNMFAX_JOBSTATUS_URL},
	
	{SERIO_CMDTYPE_PRINTSPOOL,	IOJOB_SPLPRN_CMDRECV_URL,	IOJOB_SPLPRN_JOBSTART_URL,	IOJOB_SPLPRN_JOBEND_URL,
								IOJOB_SPLPRN_PRINTPAGE_URL,	ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								IOJOB_SPLPRN_JOBSTATUS_URL},
	
	{SERIO_CMDTYPE_PCPRINT,		IOJOB_PCPRN_CMDRECV_URL,	IOJOB_PCPRN_JOBSTART_URL,	IOJOB_PCPRN_JOBEND_URL,
								IOJOB_PCPRN_PRINTPAGE_URL,	ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								IOJOB_PCPRN_JOBSTATUS_URL},
	
	{SERIO_CMDTYPE_SCANUPLOAD,	IOJOB_UPSCAN_CMDRECV_URL,	IOJOB_UPSCAN_JOBSTART_URL,	IOJOB_UPSCAN_JOBEND_URL,
								ERROR,						ERROR,						IOJOB_UPSCAN_UPFILEINFO_URL,
								IOJOB_UPSCAN_UPFILEEND_URL,	ERROR,						ERROR,
								ERROR},
	
	{SERIO_CMDTYPE_MEDIAUPLOAD,	IOJOB_UPMEDIA_CMDRECV_URL,	IOJOB_UPMEDIA_JOBSTART_URL,	IOJOB_UPMEDIA_JOBEND_URL,
								ERROR,						ERROR,						IOJOB_UPMEDIA_UPFILEINFO_URL,
								IOJOB_UPMEDIA_UPFILEEND_URL,ERROR,						ERROR,
								ERROR},
	
	{SERIO_CMDTYPE_DOWNLOADPRINT,IOJOB_DWNPRN_CMDRECV_URL,	IOJOB_DWNPRN_JOBSTART_URL,	IOJOB_DWNPRN_JOBEND_URL,
								IOJOB_DWNPRN_PRINTPAGE_URL,	ERROR,						ERROR,
								ERROR,						IOJOB_DWNPRN_DWNFILEEND_URL,ERROR,
								ERROR},
	
	{SERIO_CMDTYPE_DOWNLOADPRINT2,IOJOB_DWNPRN2_CMDRECV_URL,IOJOB_DWNPRN2_JOBSTART_URL,	IOJOB_DWNPRN2_JOBEND_URL,
								IOJOB_DWNPRN2_PRINTPAGE_URL,ERROR,						ERROR,
								ERROR,						IOJOB_DWNPRN2_DWNFILEEND_URL,IOJOB_DWNPRN2_DWFILEINFO_URL,
								ERROR},
	
	{SERIO_CMDTYPE_DOWNLOADMEDIA,IOJOB_DWNMEIDA_CMDRECV_URL,IOJOB_DWNMEIDA_JOBSTART_URL,IOJOB_DWNMEIDA_JOBEND_URL,
								ERROR,						ERROR,						ERROR,
								ERROR,						IOJOB_DWNMEIDA_DWNFILEEND_URL,ERROR,
								ERROR},
	
	{SERIO_CMDTYPE_DOWNLOADMEDIA2,IOJOB_DWNMEIDA2_CMDRECV_URL,IOJOB_DWNMEIDA2_JOBSTART_URL,IOJOB_DWNMEIDA2_JOBEND_URL,
								ERROR,						ERROR,						ERROR,
								ERROR,						IOJOB_DWNMEIDA2_DWNFILEEND_URL,IOJOB_DWNMEIDA2_DWFILEINFO_URL,
								ERROR},
	
	{SERIO_CMDTYPE_MEDIASEND,   IOJOB_SENDMEDIA_CMDRECV_URL,IOJOB_SENDMEDIA_JOBSTART_URL,IOJOB_SENDMEDIA_JOBEND_URL,
                                ERROR,                      IOJOB_SENDMEDIA_DATACOMM_URL,ERROR,
								ERROR,                      ERROR,						ERROR,
								ERROR},

	{SERIO_CMDTYPE_UPLOADSTART,	ERROR,						ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								ERROR,                      ERROR,						ERROR,
								ERROR},

	{SERIO_CMDTYPE_DWLOADSTART,	ERROR,						ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								ERROR,                      ERROR,						ERROR,
								ERROR},
	
	{SERIO_CMDTYPE_UPLOADCONT,	ERROR,						ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								ERROR,                      ERROR,						ERROR,
								ERROR},

	{SERIO_CMDTYPE_DWLOADCONT,	ERROR,						ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								ERROR},
	
	{SERIO_CMDTYPE_STARTBROTHERCP,ERROR,					ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								ERROR},
	
	{SERIO_CMDTYPE_LASTJOBRSLTRQ,ERROR,						ERROR,						LASTJOB_RSLT_REQ_JOBEND_URL,
								ERROR,						ERROR,						ERROR,
								ERROR,                      ERROR,						ERROR,
								ERROR},
	
	{SERIOFW_CN_SESSIONCLOSE,	ERROR,						ERROR,						CLOSE_SESS_REQ_JOBEND_URL,
								ERROR,						ERROR,						ERROR,
								ERROR,                      ERROR,						ERROR,
								ERROR},
	
	{SERIO_CMDTYPE_WAIT,		ERROR,						ERROR,						WAIT_JOBEND_URL,
								ERROR,						ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								ERROR},

	{SERIO_CMDTYPE_GETLASTUSERLOG,ERROR,					ERROR,						GETLOG_JOBEND_URL,
								ERROR,						ERROR,						ERROR,
								ERROR,                      ERROR,						ERROR,
								ERROR},

	{SERIO_CMDTYPE_IOJOBCONT,	ERROR,						ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								ERROR,                      ERROR,						ERROR,
								ERROR},
	
	{SERIO_CMDTYPE_NTFYDEVSTS,	NTFY_DEVSTS_CMDRECV_URL,	NTFY_DEVSTS_JOBSTART_URL,	NTFY_DEVSTS_JOBEND_URL,
								ERROR,						ERROR,						ERROR,
								ERROR,						ERROR,						ERROR,
								NTFY_DEVSTS_STATUS_URL},
};

/*===========================================================================
 * EventType -> Ack Type refernce table
 *	＜注釈＞
 *	・EventタイプからAckタイプを選定するためのテーブル
 *	・Eventタイプが追加された時はレコードを追加する
 *	・UI JOBコマンドはUI Script内で定義されるためここには登録しない（個別処理）
 ===========================================================================*/
typedef struct EvTypeToAckType_s	EvTypeToAckType_t;
struct EvTypeToAckType_s
{
	UINT16		EvtType;		/* Eventタイプ			*/
	UINT16		AckType;		/* Ackタイプ			*/
};
STATIC const EvTypeToAckType_t	EvTypeToAckTypeTbl[] = {
	{SERIO_EVTYPE_LAUNCHREQ,		SERIO_ACKTYPE_JOBEND},
	{SERIO_EVTYPE_LOCKRSLT,			SERIO_ACKTYPE_JOBEND},
	{SERIO_EVTYPE_DBREAD,			SERIO_ACKTYPE_JOBEND},
	{SERIO_EVTYPE_DBWRITE,			SERIO_ACKTYPE_JOBEND},
	{SERIO_EVTYPE_JOBRECEIVED,		SERIO_ACKTYPE_CMDRECV},
	{SERIO_EVTYPE_JOBSTARTED,		SERIO_ACKTYPE_JOBSTART},
	{SERIO_EVTYPE_PAGEEND,			SERIO_ACKTYPE_PRINTPAGE},
	{SERIO_EVTYPE_TRANSEND,			SERIO_ACKTYPE_DATACOMM},	
	{SERIO_EVTYPE_UPLOADFILEINFO,	SERIO_ACKTYPE_UPLOADFILEINFO},
    {SERIO_EVTYPE_DOWNLOADFILEINFO, SERIO_ACKTYPE_DOWNLOADFILEINFO},
	{SERIO_EVTYPE_UPLOADFILE,		SERIO_ACKTYPE_UPLOADFILE},
	{SERIO_EVTYPE_DOWNLOADFILE,		SERIO_ACKTYPE_DOWNLOADFILE},
	{SERIO_EVTYPE_JOBFINISHED,		SERIO_ACKTYPE_JOBEND},
	{SERIO_EVTYPE_SESSIONCLOSED,	SERIO_ACKTYPE_JOBEND},
	{SERIO_EVTYPE_WAITTOUT,			SERIO_ACKTYPE_JOBEND},
	{SERIO_EVTYPE_LASTUSERLOG,		SERIO_ACKTYPE_JOBEND},
	{SERIO_EVTYPE_DEVSTSCHANGED,	SERIO_ACKTYPE_JOBSTATUS},
	{SERIO_EVTYPE_NTFTABORTED,		SERIO_ACKTYPE_JOBEND},
};

/*===========================================================================
 * Command Exclusive table
 *	＜注釈＞
 *	・コマンド同時実行を許可するかどうかを定義テーブル
 *	・現在実行中のコマンド（縦）に対し、保留中のコマンド（横）実行可能か
 *	　どうかを表す。
 *	・CommandClassが追加された時はレコードを追加する
 ===========================================================================*/
STATIC const UINT8	CmdExclusiveTbl[][8] = {
	/*-- NONE ----+--SYSJOB -+-- LOCK---+-- DBJOB --+-- UIJOB --+-- IOJOB --+-- WAIT ---+- NOTIFY -*/
	
	/* CLASS=NONE（例外） */
	{	'X',		'X',		'X',		'X',		'X',		'X',		'X',		'X'	},
	
	/* SYSJOB */
	{	'X',		'X',		'X',		'X',		'X',		'X',		'X',		'X'	},

	/* LOCK */
	{	'X',		'X',		'X',		'O',		'O',		'X',		'X',		'O'	},

	/* DBJOB */
	{	'X',		'X',		'O',		'X',		'O',		'O',		'O',		'O'	},

	/* UIJOB */
	{	'X',		'X',		'O',		'O',		'X',		'O',		'O',		'O'	},

	/* IOJOB */
	{	'X',		'X',		'X',		'O',		'O',		'X',		'X',		'O'	},

	/* WAIT */
	{	'X',		'X',		'X',		'O',		'O',		'X',		'X',		'O'	},

	/* NOTIFY */
	{	'X',		'O',		'O',		'O',		'O',		'O',		'O',		'X'	},
};
 
/*****************************************************************************
 * Local function prototypes.
 ****************************************************************************/

/*===========================================================================
 * コマンド種別・属性に関するライブラリ
 ===========================================================================*/
/*****************************************************************************
 * コマンドクラスの取得
 ****************************************************************************/
UINT16
GetSerioCmdClass( SerioCmdObj_t *Cmd )
{
	INT32	index;
	UINT16	Class = SERIOCMD_CLASS_IOJOB;
	
	for( index=0; index<sizeof(CmdTypeCorrespondTbl)/sizeof(CmdTypeCorrespondTbl[0]); index++ )
	{
		if( Cmd->CmdType == CmdTypeCorrespondTbl[index].CmdType )
		{
			Class = CmdTypeCorrespondTbl[index].CmdClass;
			break;
		}
	}
	
	return Class;
}

/*****************************************************************************
 * 依存コマンドの取得
 ****************************************************************************/
UINT16
GetSerioDependCmd( SerioCmdObj_t *Cmd, const UINT16 *DependCmdList[] )
{
	INT32	index;
	
	for( index=0; index<sizeof(CmdTypeCorrespondTbl)/sizeof(CmdTypeCorrespondTbl[0]); index++ )
	{
		if( Cmd->CmdType == CmdTypeCorrespondTbl[index].CmdType )
		{
			*DependCmdList = CmdTypeCorrespondTbl[index].DependCmd;
			return CmdTypeCorrespondTbl[index].DependCmdNum;
		}
	}
	
	return 0;
}

/*****************************************************************************
 * 指定されたコマンドタイプを依存コマンドとして持つかを調査する
 ****************************************************************************/
BOOL
IsDependOnCmd( SerioCmdObj_t *Cmd, UINT16 DependCmdType )
{
	INT32	index;
	INT32	ptr;
	
	for( index=0; index<sizeof(CmdTypeCorrespondTbl)/sizeof(CmdTypeCorrespondTbl[0]); index++ )
	{
		if( Cmd->CmdType == CmdTypeCorrespondTbl[index].CmdType )
		{
			/* 指定されたコマンドが依存するコマンドがないか探す */
			for( ptr=0; ptr < CmdTypeCorrespondTbl[index].DependCmdNum; ptr++ )
			{
				if( CmdTypeCorrespondTbl[index].DependCmd[ptr] == DependCmdType )
				{
					return TRUE;
				}
			}
		}
	}
	
	return FALSE;
}
/*****************************************************************************
 * コマンドコネクタの選択
 ****************************************************************************/
UINT16
SelectCmdConnectorID( SerioCmdObj_t *Cmd )
{
	INT32	index;
	UINT16	Connector = (UINT16)ERROR;
	
	for( index=0; index<sizeof(CmdTypeCorrespondTbl)/sizeof(CmdTypeCorrespondTbl[0]); index++ )
	{
		if( Cmd->CmdType == CmdTypeCorrespondTbl[index].CmdType )
		{
			Connector = CmdTypeCorrespondTbl[index].Connector;
			break;
		}
	}
	
	return Connector;
}

/*===========================================================================
 * Ack URL選択に関するライブラリ
 ===========================================================================*/
/*****************************************************************************
 * Ack URLをRequestデータの中から選択する
 ****************************************************************************/
UINT8 *
SelectCmdAckURL( INT32 CmdType, UINT16 AckType, void *ReqData )
{
	INT32	index;
	INT16	offset;
	UINT8	*pURL = (UINT8 *)NULL;
	
	/* Ack typeの範囲チェック */
	if( (AckType < 1 ) || (AckType >= SERIO_ACKTYPE_MAX) )
	{
		return (UINT8 *)NULL;
	}
	
	for( index=0; index<sizeof(AckURLSelectTBl)/sizeof(AckURLSelectTBl[0]); index++ )
	{
		if( AckURLSelectTBl[index][0] == (UINT16)CmdType )
		{
			offset = AckURLSelectTBl[index][AckType];
			if( offset != ERROR )
			{
				pURL = *((UINT8 **)((UINT8 *)ReqData + offset ));
				if( ! pURL )
				{
					SFPRINTF(5,("SIO:SelectCmdAckURL Empty-URL(CmdType:%d AckType:%d)\n", CmdType,AckType));
				}
			}
			break;
		}
	}
	
	return pURL;
}

/*****************************************************************************
 * 指定されたAck URLが未定義（仕様的に必要としない）であるかを調査する
 ****************************************************************************/
BOOL
IsUndefCmdAckURL( INT32 CmdType, UINT16 AckType, void *ReqData )
{
	INT32	index;
	INT16	offset;
	
	/* Ack typeの範囲チェック */
	if( (AckType < 1 ) || (AckType >= SERIO_ACKTYPE_MAX) )
	{
		return FALSE;
	}
	
	for( index=0; index<sizeof(AckURLSelectTBl)/sizeof(AckURLSelectTBl[0]); index++ )
	{
		if( AckURLSelectTBl[index][0] == (UINT16)CmdType )
		{
			offset = AckURLSelectTBl[index][AckType];
			if( offset == ERROR )
			{
				return TRUE;
			}
		}
	}
	
	return FALSE;
}

/*****************************************************************************
 * EventTypeから対応するAckTypeを選択する
 ****************************************************************************/
UINT16
SelectAckTypeFromEvtType( UINT16 EvtType )
{
	INT32	index;
	UINT16	AckType = 0;	/* 0:選択なし */
	
	for( index=0; index<sizeof(EvTypeToAckTypeTbl)/sizeof(EvTypeToAckTypeTbl[0]); index++ )
	{
		if( EvtType == EvTypeToAckTypeTbl[index].EvtType )
		{
			AckType  = EvTypeToAckTypeTbl[index].AckType;
			break;
		}
	}
	
	return AckType;
}

/*===========================================================================
 * Command選択に関するライブラリ
 ===========================================================================*/
BOOL
IsSerioCmdExecutable( ListCtrlNode_t *ExecutingCmds, SerioCmdObj_t *TargetCmd )
{
	UINT16			ExecCmdClass;
	UINT16			ExecCmdType;
	UINT16			TargetCmdClass;
	SerioCmdObj_t	*ExecCmd;
	ListCtrlNode_t	*node;

	TargetCmdClass = GetSerioCmdClass( TargetCmd );
    
    /* ターゲットがSubコマンドクラスの時 */
	if( TargetCmdClass == SERIOCMD_CLASS_SUBCMD )
    {
    	/* 依存しているコマンドが実行中かを調べ、実行中でなければNG */
	 	ExecCmd = (SerioCmdObj_t *)OBJ_FINDFIRST( ExecutingCmds, &node );
		while( ExecCmd )
    	{
    		ExecCmdType = GetSerioCmdType( ExecCmd );
    		if( IsDependOnCmd( TargetCmd, ExecCmdType ) )
    		{
    			break;
    		}
			ExecCmd = (SerioCmdObj_t *)OBJ_FINDNEXT( &node );
		}
    	if( ! ExecCmd )
    	{
    		return FALSE;
    	}
    	/* 依存している実行中コマンドと依存関係を持つその他のコマンドがすでに実行中か調べ、存在すればNG */
	 	ExecCmd = (SerioCmdObj_t *)OBJ_FINDFIRST( ExecutingCmds, &node );
		while( ExecCmd )
    	{

    		if( GetSerioCmdClass( ExecCmd ) == SERIOCMD_CLASS_SUBCMD )
    		{
    			if( IsDependOnCmd( ExecCmd, ExecCmdType ) )
    			{
					return FALSE;
    			}
    		}
			ExecCmd = (SerioCmdObj_t *)OBJ_FINDNEXT( &node );
    	}
		return TRUE;
    }
	
    /* ターゲットがSubコマンドクラス以外の時 */
	else
	{
	 	ExecCmd = (SerioCmdObj_t *)OBJ_FINDFIRST( ExecutingCmds, &node );
		while( ExecCmd )
    	{
			ExecCmdClass = GetSerioCmdClass( ExecCmd );
    		if( ExecCmdClass != SERIOCMD_CLASS_SUBCMD )	/* 実行中コマンド=Subコマンドクラスは除外 */
    		{
    			/* 実行不可な組み合わせが１つでも見つかればNG */
				if( CmdExclusiveTbl[ExecCmdClass][TargetCmdClass] == 'X' )
				{
					return FALSE;
				}
    		}
			ExecCmd = (SerioCmdObj_t *)OBJ_FINDNEXT( &node );
    	}
		return TRUE;
	}
}

