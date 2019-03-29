/************************************************************************/
/*																		*/
/*	Serio �����f�[�^�x�[�X ���W���[���Q									*/
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
/*	ver 1.0.0 : 2010.07.28 :  �V�K�쐬	�|�{ 							*/
/*																		*/
/************************************************************************/
/****** �C���N���[�h�E�t�@�C�� ******************************************/
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
 *	�����߁�
 *	�E�R�}���h�^�C�v���ǉ����ꂽ���̓��R�[�h��ǉ�����
 *	�E�R�}���h�^�C�v�ǉ���serio_cmdparam.h�̃}�N���ǉ��ɘA������
 *==========================================================================*/
typedef struct CmdTypeCorrespond_s	CmdTypeCorrespond_t;
struct CmdTypeCorrespond_s
{
	INT32			CmdType;		/* �R�}���h�^�C�v		*/
	UINT16			CmdClass;		/* �R�}���h�N���X		*/
	UINT16			Connector;		/* �Ή�����R�l�N�^		*/
	const UINT16	*DependCmd;		/* �ˑ��֌W�̂���R�}���h�Q�iSERIOCMD_CLASS_SUBCMD���̂݁j */
	UINT16			DependCmdNum;	/* �ˑ��֌W�̂���R�}���h�̐��iSERIOCMD_CLASS_SUBCMD���̂݁j */
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

STATIC const UINT16	IoJobContDepends[] = {	/* �I���ӁI�@Continue�w���Ή�IOJOB����������ǉ����邱�� */
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
 *	�����߁�
 *	�EAck URL�̎�ʂ��ǉ����ꂽ���͓񎟌��ڂ̗v�f���𑝂₷
 *	�E�R�}���h�^�C�v���ǉ����ꂽ���͈ꎟ���̃��R�[�h��ǉ�����
 *	�E����2�����z��� ��(���̕���)�̏��Ԃ́Aenum "Ack Type"(serio_cmdparam.h) 
 *	  �̕��я��ɓ������Ă��Ȃ���΂Ȃ�Ȃ�
 *	�E��L�ǉ��͂Ƃ���serio_cmdparam.h�̃}�N���ǉ��ɘA������
 *	�EUI JOB�R�}���h��UI Script���Œ�`����邽�߂����ɂ͓o�^���Ȃ��i�ʏ����j
 ===========================================================================*/
STATIC const INT16	AckURLSelectTBl[][11] = {
	/*--- Cmd Type ------------+-- �R�}���h��MACK --------+-- JOB�J�nACK -------------+-- JOB�I��ACK -----------------*/
	/*-------------------------+-- 1Page����I��ACK -------+-- 1���Đ�ʐM�I��ACK -----+-- Upload File Info. ----------*/
	/*-------------------------+-- Upload File ------------+-- Download File ----------+---Download File Info. --------*/
	/*-------------------------+-- Job��ԒʒmACK ---------+---------------------------+-------------------------------*/
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
 *	�����߁�
 *	�EEvent�^�C�v����Ack�^�C�v��I�肷�邽�߂̃e�[�u��
 *	�EEvent�^�C�v���ǉ����ꂽ���̓��R�[�h��ǉ�����
 *	�EUI JOB�R�}���h��UI Script���Œ�`����邽�߂����ɂ͓o�^���Ȃ��i�ʏ����j
 ===========================================================================*/
typedef struct EvTypeToAckType_s	EvTypeToAckType_t;
struct EvTypeToAckType_s
{
	UINT16		EvtType;		/* Event�^�C�v			*/
	UINT16		AckType;		/* Ack�^�C�v			*/
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
 *	�����߁�
 *	�E�R�}���h�������s�������邩�ǂ������`�e�[�u��
 *	�E���ݎ��s���̃R�}���h�i�c�j�ɑ΂��A�ۗ����̃R�}���h�i���j���s�\��
 *	�@�ǂ�����\���B
 *	�ECommandClass���ǉ����ꂽ���̓��R�[�h��ǉ�����
 ===========================================================================*/
STATIC const UINT8	CmdExclusiveTbl[][8] = {
	/*-- NONE ----+--SYSJOB -+-- LOCK---+-- DBJOB --+-- UIJOB --+-- IOJOB --+-- WAIT ---+- NOTIFY -*/
	
	/* CLASS=NONE�i��O�j */
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
 * �R�}���h��ʁE�����Ɋւ��郉�C�u����
 ===========================================================================*/
/*****************************************************************************
 * �R�}���h�N���X�̎擾
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
 * �ˑ��R�}���h�̎擾
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
 * �w�肳�ꂽ�R�}���h�^�C�v���ˑ��R�}���h�Ƃ��Ď����𒲍�����
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
			/* �w�肳�ꂽ�R�}���h���ˑ�����R�}���h���Ȃ����T�� */
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
 * �R�}���h�R�l�N�^�̑I��
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
 * Ack URL�I���Ɋւ��郉�C�u����
 ===========================================================================*/
/*****************************************************************************
 * Ack URL��Request�f�[�^�̒�����I������
 ****************************************************************************/
UINT8 *
SelectCmdAckURL( INT32 CmdType, UINT16 AckType, void *ReqData )
{
	INT32	index;
	INT16	offset;
	UINT8	*pURL = (UINT8 *)NULL;
	
	/* Ack type�͈̔̓`�F�b�N */
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
 * �w�肳�ꂽAck URL������`�i�d�l�I�ɕK�v�Ƃ��Ȃ��j�ł��邩�𒲍�����
 ****************************************************************************/
BOOL
IsUndefCmdAckURL( INT32 CmdType, UINT16 AckType, void *ReqData )
{
	INT32	index;
	INT16	offset;
	
	/* Ack type�͈̔̓`�F�b�N */
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
 * EventType����Ή�����AckType��I������
 ****************************************************************************/
UINT16
SelectAckTypeFromEvtType( UINT16 EvtType )
{
	INT32	index;
	UINT16	AckType = 0;	/* 0:�I���Ȃ� */
	
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
 * Command�I���Ɋւ��郉�C�u����
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
    
    /* �^�[�Q�b�g��Sub�R�}���h�N���X�̎� */
	if( TargetCmdClass == SERIOCMD_CLASS_SUBCMD )
    {
    	/* �ˑ����Ă���R�}���h�����s�����𒲂ׁA���s���łȂ����NG */
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
    	/* �ˑ����Ă�����s���R�}���h�ƈˑ��֌W�������̑��̃R�}���h�����łɎ��s�������ׁA���݂����NG */
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
	
    /* �^�[�Q�b�g��Sub�R�}���h�N���X�ȊO�̎� */
	else
	{
	 	ExecCmd = (SerioCmdObj_t *)OBJ_FINDFIRST( ExecutingCmds, &node );
		while( ExecCmd )
    	{
			ExecCmdClass = GetSerioCmdClass( ExecCmd );
    		if( ExecCmdClass != SERIOCMD_CLASS_SUBCMD )	/* ���s���R�}���h=Sub�R�}���h�N���X�͏��O */
    		{
    			/* ���s�s�ȑg�ݍ��킹���P�ł��������NG */
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

