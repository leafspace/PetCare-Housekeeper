/************************************************************************/
/*																		*/
/*	Serio �t���[�����[�N �C���N���[�h�t�@�C��							*/
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
/*	ver 1.0.0 : 2010.03.12 :  �V�K�쐬	�|�{ 							*/
/*																		*/
/************************************************************************/
#ifndef _SERIO_H
#define _SERIO_H

#include	"serio_cmdparam.h"

/*****************************************************************************
 *  Constant value definition
 ****************************************************************************/
/*-- Return Value ----------------------------------------------------------*/
#define	SIO_RC_OK				(0)		/* ����I��							*/
#define	SIO_RC_NOTFIND			(-1)	/* �w�肳�ꂽID�ɑΉ�����C���X�^���X��������Ȃ�	*/
#define	SIO_RC_MEMFUL			(-2)	/* �������t��						*/
#define	SIO_RC_ILSTATUS			(-3)	/* �s���ȏ�Ԃł̎��s				*/
#define	SIO_RC_INITFAIL			(-4)	/* ���������s						*/
#define	SIO_RC_PARSEERROR		(-5)	/* Parse�G���[						*/
#define	SIO_RC_BUILDERROR		(-6)	/* Build�G���[						*/
#define	SIO_RC_PARAMERROR		(-7)	/* Parameter�G���[					*/

/*-- Application�I����ԂɊւ����` ---------------------------------------*/
#define	SERIOFW_APP_COMPLETED				(0)			/* ����I��												*/
#define	SERIOFW_APP_CANCELEDBY_USER			(0x0001)	/* User�ɂ��L�����Z���I��								*/
#define	SERIOFW_APP_ABORTEDBY_TIMEOUT		(0x0002)	/* ������ɂ��^�C���A�E�g�I��							*/
#define	SERIOFW_APP_ABORTEDBY_INTERNALERR	(0x0100)	/* ServiceError�ɂ�鋭���I���i�����G���[�j				*/
#define SERIOFW_APP_ABORTEDBY_PARSEERR		(0x0101)	/* ServiceError�ɂ�鋭���I���iParse�G���[�j			*/
#define	SERIOFW_APP_ABORTEDBY_LINKDOWN		(0x0200)	/* ServiceError�ɂ�鋭���I���iLinkDown�j				*/
#define	SERIOFW_APP_ABORTEDBY_CONNECTERR	(0x0201)	/* ServiceError�ɂ�鋭���I���iServer�ڑ��G���[�j		*/
#define	SERIOFW_APP_ABORTEDBY_COMTIMEOUT	(0x0202)	/* ServiceError�ɂ�鋭���I���i�ʐM�^�C���A�E�g�j		*/
#define	SERIOFW_APP_ABORTEDBY_PROXYAUTHERR	(0x0300)	/* ServiceError�ɂ�鋭���I���iProxy Server�̔F�؎��s�j	*/
#define	SERIOFW_APP_ABORTEDBY_CERTINVALID	(0x0400)	/* ServiceError�ɂ�鋭���I���i�ؖ����G���[�j			*/
#define	SERIOFW_APP_ABORTEDBY_CERTEXPIRED	(0x0401)	/* ServiceError�ɂ�鋭���I���i�ؖ����L�������G���[�j	*/
#define SERIOFW_APP_ABORTEDBY_ADMINPASSERR	(0x0500)	/* ServiceError�ɂ�鋭���I���i�Ǘ��҃p�X���[�h���ݒ�j	*/

/*-- �{�̋@�\�Ƃ̃R�}���h�R�l�N�^�Ɋւ����` ------------------------------*/
/*==========================================================================*/
/*  �R�}���h����															*/
/*==========================================================================*/
enum {
	SERIOFW_CN_APLEXEC = 1,			/* �A�v���P�[�V�������s�ʒm�R�l�N�^ */
	SERIOFW_CN_USERLOCK,			/* ���[�ULock						*/
	SERIOFW_CN_USERUNLOCK,			/* ���[�UUnlock						*/
	SERIOFW_CN_UI,					/* UI����w��						*/
	SERIOFW_CN_COPY,				/* COPY���s							*/
	SERIOFW_CN_SENDFAX,				/* FAX���M							*/
	SERIOFW_CN_SCANSEND,			/* SCAN�]��							*/
	SERIOFW_CN_DIRECTPRINT,			/* USB�_�C���N�g�v�����g			*/
	SERIOFW_CN_PRNMEMFAX,			/* ��������MFAX�v�����g			*/
	SERIOFW_CN_PCPRINT,				/* PC�v�����g						*/
	SERIOFW_CN_PRNSPOOL,			/* Stored Job�v�����g				*/
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
	SERIOFW_CN_SESSIONCLOSE,		/* Session Close					*//* SerioFramework�ŕW���o�^ */
	SERIOFW_CN_WAIT,				/* Wait								*//* SerioFramework�ŕW���o�^ */
	SERIOFW_CN_GETLASTUSERLOG,		/* Log�擾							*/
	SERIOFW_CN_IOJOBCONT,			/* IOJOB Continue�w��(Sub)			*//* SerioFramework�ŕW���o�^ */
	SERIOFW_CN_NOTFY_DEVSTS,		/* Device Status Notify				*/

	SERIOFW_CN_MAX
};
/*==========================================================================*/
/*  �R�}���h����w���^�C�v													*/
/*==========================================================================*/
enum {
	SERIOFW_CN_CTRLTYPE_INIT =1,		/* �R�}���h�����̏�����				*/
	SERIOFW_CN_CTRLTYPE_EXEC,			/* �R�}���h�����̎��s				*/
	SERIOFW_CN_CTRLTYPE_RESTART,		/* �R�}���h������~������̍ĊJ		*/
	SERIOFW_CN_CTRLTYPE_CANCEL,			/* �R�}���h�����̃L�����Z��			*/
	SERIOFW_CN_CTRLTYPE_EXIT,			/* �R�}���h�����̏I��				*/
	
	SERIOFW_CN_CTRLTYPE_MAX
};
/*==========================================================================*/
/*  �R�}���h�N���X															*/
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
	SERIOCMD_CLASS_SUBCMD,				/* SubCommand Class					*//* �K���Ō�ɂ��邱��!! */
};

/*==========================================================================*/
/*  UI����w���^�C�v															*/
/*==========================================================================*/
//#define	SERIOFW_UIDEMTYPE_CURRENT			(0x0000)	/* �i�\��jCurrent��UI�w���^�C�v	*/
#define	SERIOFW_UIDEMTYPE_UISCRIPT_INPUT	(0x0001)	/* UI Script�ɂ��UI�w��(���͗v��)	*/
#define	SERIOFW_UIDEMTYPE_UISCRIPT_INFO		(0x0002)	/* UI Script�ɂ��UI�w��(��m)	*/
#define	SERIOFW_UIDEMTYPE_UISCRIPT_LINK		(0x0003)	/* UI Script�ɂ��UI�w��(LINK)	*/
#define	SERIOFW_UIDEMTYPE_UISCRIPT_NULL		(0x0004)	/* UI Script�ɂ��UI�w��(NULL)	*/
#define	SERIOFW_UIDEMTYPE_DEPBASE			(0x0010)	/* ��{�@�\�̕\���ɐ؂芷��	*/
#define	SERIOFW_UIDEMTYPE_PROCESSING 		(0x0011)	/* UI�w�����Ȃ����̕\��		*/
#define	SERIOFW_UIDEMTYPE_CANCELING			(0x0012)	/* CANCEL���\��				*/
#define	SERIOFW_UIDEMTYPE_ABORTING			(0x0013)	/* ABORT���\��				*/
#define	SERIOFW_UIDEMTYPE_PAUSED			(0x0014)	/* �ꎞ��~���\��			*/
#define	SERIOFW_UIDEMTYPE_CUSTOM			(0x0100)	/* ���[�U��`(���ʃo�C�g�����ʏ��Ƃ��Ďg�p�j*/

	
/*-- �{�̋@�\�����Event�ʒm�@�\�Ɋւ����` -------------------------------*/
/*==========================================================================*/
/*  Event ID																*/
/*==========================================================================*/
enum {
	SERIOFW_EVID_UIEND=1,				/* UI���͏I���ʒm					*/
	SERIOFW_EVID_JOBSTATUS,				/* �W���u�X�e�[�^�X�ʒm				*/
	SERIOFW_EVID_JOBPROGRESS,			/* �W���u�i���ʒm					*/
	SERIOFW_EVID_DEVSTATUS,				/* �f�o�C�X�X�e�[�^�X�ʒm			*/
	SERIOFW_EVID_LOCKRESULT,			/* LOCK/UNLOCK���ʒʒm				*/
	SERIOFW_EVID_DBREADRESULT,			/* DB Read���ʒʒm					*/
	SERIOFW_EVID_DBWRITERESULT,			/* DB Write���ʒʒm					*/
	SERIOFW_EVID_WAITJOBRESULT,			/* WaitJob���ʒʒm					*/
	SERIOFW_EVID_GETLOGRESULT,			/* GetLastUserLog���ʒʒm			*/

	SERIOFW_EVID_MAX
};

/*==========================================================================*/
/*  UI���͏I���X�e�[�^�X													*/
/*==========================================================================*/
enum {
	SERIOFW_UIENDSTS_OK=1,				/* ����I���i���͊m��j				*/
	SERIOFW_UIENDSTS_SKIP,				/* ����I���i�X�L�b�v�j				*/
	SERIOFW_UIENDSTS_GO,				/* ����I���i�m�聕���s�J�n�j		*/
	SERIOFW_UIENDSTS_BACK,				/* �O��ʈړ����삪�Ȃ��ꂽ			*/
	SERIOFW_UIENDSTS_FORWARD,			/* ����ʈړ����삪�Ȃ��ꂽ			*/
	SERIOFW_UIENDSTS_PREV,				/* �O�̑I�����Ăяo�����Ȃ��ꂽ	*/
	SERIOFW_UIENDSTS_NEXT,				/* ���̑I�����Ăяo�����Ȃ��ꂽ	*/
	SERIOFW_UIENDSTS_CANCEL,			/* ���̓L�����Z��					*/
	SERIOFW_UIENDSTS_TIMEOUT,			/* TimeOut�ɂ����̓L�����Z��		*/
	SERIOFW_UIENDSTS_ABORT,				/* �������f�ɂ��I��				*/
	SERIOFW_UIENDSTS_CUSTOM1,			/* �J�X�^���{�^������				*/

	SERIO_UIENDSTS_MAX
};

/*==========================================================================*/
/*  �W���u����																*/
/*==========================================================================*/
enum {
	SERIOFW_IOJOB_COPY=1,				/* COPY���s							*/
	SERIOFW_IOJOB_SENDFAX,				/* FAX���M							*/
	SERIOFW_IOJOB_SCANSEND,				/* SCAN�]��							*/
	SERIOFW_IOJOB_DIRECTPRINT,			/* USB�_�C���N�g�v�����g			*/
	SERIOFW_IOJOB_PRNMEMFAX,			/* ��������MFAX�v�����g			*/
	SERIOFW_IOJOB_PCPRINT,				/* PC�v�����g						*/
	SERIOFW_IOJOB_PRNSPOOL,				/* Secure�v�����g					*/
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
/*  �W���u�X�e�[�^�X														*/
/*==========================================================================*/
enum {
	SERIOFW_JOBSTS_PROCESSING=1,		/* ���s��							*/
	SERIOFW_JOBSTS_PAUSED,				/* ���s��~��						*/
	SERIOFW_JOBSTS_CANCELING,			/* CANCEL��							*/
	SERIOFW_JOBSTS_END,					/* ���s�I��							*/

	SERIOFW_JOBSTS_MAX
};

/*==========================================================================*/
/*  �W���u��~�v��															*/
/*==========================================================================*/
enum {
	SERIOFW_JOBSTS_PAUSEDBY_ERROR=1,	/* �G���[�����ɂ��W���u��~�i��orCancel�҂��j*/
	SERIOFW_JOBSTS_PAUSEDBY_SCANNEXT,	/* ���y�[�W���e�Z�b�g���J�n�҂�		*/

	SERIOFW_IOJOB_PAUSEDBY_MAX
};

/*==========================================================================*/
/*  �W���u�L�����Z���v��														*/
/*==========================================================================*/
enum {
	SERIOFW_JOBSTS_CANCELBY_USER=1,		/* ���[�U�A�܂��́A�A�v���P�[�V��������̎w���ɂ��L�����Z��*/
	SERIOFW_JOBSTS_CANCELBY_ABORT,		/* �G���[���o���A�W���u���s���̓s���ɂ�鋭���I�ȃL�����Z��		*/

	SERIOFW_JOBSTS_CANCELBY_MAX
};
	
/*==========================================================================*/
/*  �W���u�I���X�e�[�^�X													*/
/*==========================================================================*/
enum {
	SERIOFW_JOBSTS_END_COMPLETE=1,		/* ����I�� */
	SERIOFW_JOBSTS_END_CANCEL,			/* �L�����Z���I�� */
	SERIOFW_JOBSTS_END_SYSBUSY,			/* ���\�[�X�s���A�V�X�e���s���ɂ����s�s�I��	*/
	SERIOFW_JOBSTS_END_DEVERR,			/* �n�[�h�A���J�v���ɂ��f�o�C�X��Q�ɂ����s�s�I��	*/

	SERIOFW_JOBSTS_END_MAX
};

/*==========================================================================*/
/*  UserOption																*/
/*==========================================================================*/
enum {
	SERIOFW_JOBSTS_OPTION_YES=1,		/* Yes */
	SERIOFW_JOBSTS_OPTION_NO,			/* No */
	SERIOFW_JOBSTS_OPTION_RESTART,		/* �ĊJ */

	SERIOFW_JOBSTS_OPTION_MAX
};

/*==========================================================================*/
/*  �W���u�i���^�C�v														*/
/*==========================================================================*/
#define	SERIOFW_JOBPROG_PRINTPAGEEND	SERIO_EVTYPE_PAGEEND          /* 1Page����I��					*/
#define	SERIOFW_JOBPROG_TRANSEND		SERIO_EVTYPE_TRANSEND         /* 1����]���I��					*/
#define	SERIOFW_JOBPROG_UPLOADFINFO		SERIO_EVTYPE_UPLOADFILEINFO   /* Upload File���ʒm			*/
#define	SERIOFW_JOBPROG_DOWNLOADFINFO	SERIO_EVTYPE_DOWNLOADFILEINFO /* Download File���ʒm			*/
#define	SERIOFW_JOBPROG_UPLOADFILE		SERIO_EVTYPE_UPLOADFILE       /* �PFile UPLOAD�I��			*/
#define	SERIOFW_JOBPROG_DOWNLOADFILE	SERIO_EVTYPE_DOWNLOADFILE     /* �PFile DOWNLOAD�I��			*/

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
/*-- �O���Ƃ�I/F�Ɋւ����` ------------------------------*/
/*==========================================================================*/
/*  Adapter�o�^���															*/
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
	SerioEvAdptrCB_t		EvCbFunc;  		/* Event�R�[���o�b�N�֐�			*/
	SerioCmdEmptyAdptrCB_t	CmdEmpCbFunc;  	/* CmdEmpty�R�[���o�b�N�֐�			*/
	SerioAppCancelAdptrCB_t	AppCancelCbFunc;  /* App Cancel�R�[���o�b�N�֐�		*/
	SerioSessionTimeOutCB_t	SessToutCbFunc;  /* SessionTimeOut�R�[���o�b�N�֐�	*/
	SERIO_CMD_PARSE_BUILD_T	CmdParsBldr;	/* �R�}���hParser/Builder�֐�		*/
};

/*==========================================================================*/
/*  Application�o�^���														*/
/*==========================================================================*/
typedef struct SERIO_APP_ENTRY_S SERIO_APP_ENTRY_T;
struct SERIO_APP_ENTRY_S
{
	INT32					AppCode;  	/* Application���ʃR�[�h			*/
};


/*-- �{�̋@�\�Ƃ�I/F�Ɋւ����` ------------------------------*/
/*==========================================================================*/
/*  Connecter																*/
/*==========================================================================*/
/*  �R�l�N�^�o�^���														*/
typedef struct SERIO_CNNCTR_ENTRY_S SERIO_CNNCTR_ENTRY_T;
struct SERIO_CNNCTR_ENTRY_S
{
	SerioFwCmdCnFunc_t		Connector;  /* �R�l�N�^�R�[���o�b�N�֐�			*/
};

/*  �A�v���P�[�V�������s�ʒm�R�l�N�^�p�����[�^								*/
typedef struct SERIO_CNP_APLSTART_S SERIO_CNP_APLSTART_T;
struct SERIO_CNP_APLSTART_S
{
	INT32					AplCode;	/* �A�v���P�[�V�����̋N������Code	*/
	INT32					AppID;		/* �A�v���P�[�V����ID				*/
};
	
/*  �W���u��~������ĊJ����w���p�����[�^									*/
typedef struct SERIO_CNP_RESTART_PARAM_S SERIO_CN_RESTART_PARAM_T;
struct SERIO_CNP_RESTART_PARAM_S
{
	UINT16					Reason;		/* ��~�v��							*/
	UINT16					SubCode;	/* �T�u�R�[�h						*/
	void					*Custom;	/* Pause�����p�����[�^				*/
};

/*  UI����w���p�����[�^													*/
typedef struct SERIO_CNP_UICOMAND_S SERIO_CNP_UICOMAND_T;
struct SERIO_CNP_UICOMAND_S
{
	UINT16					UIType;		/* UI�^�C�v							*/
	void					*Param;		/* UI�^�C�v���Ƃɒ�`�����p�����[�^*/
};

/*--- UI�^�C�v���Ƃ̃p�����[�^ ----------------------------------------------*/
/*	��UI Script�iSERIOFW_UIDEMTYPE_UISCRIPT_INPUT/INFO�j�p�����[�^			
SERIO_UISCREEN_T��serio_cmpparam.h,SerioUiScriptStub.h�ɂĒ�`				*/

/*	��Cancel���\���iSERIOFW_UIDEMTYPE_CANCELING�j�p�����[�^					*/
typedef struct SERIO_CNP_UICOMPRM_CANCELING_S SERIO_CNP_UICOMPRM_CANCELING_T;
struct SERIO_CNP_UICOMPRM_CANCELING_S
{
	INT32					Reason;		/* ��~�v��							*/
};

/*	��Abort���\���iSERIOFW_UIDEMTYPE_ABORTING�j�p�����[�^					*/
typedef struct SERIO_CNP_UICOMPRM_ABORTING_S SERIO_CNP_UICOMPRM_ABORTING_T;
struct SERIO_CNP_UICOMPRM_ABORTING_S
{
	INT32					Reason;		/* ��~�v��							*/
};

/*	���ꎞ��~���\���iSERIOFW_UIDEMTYPE_PAUSE�j�p�����[�^					*/
typedef struct SERIO_CNP_UICOMPRM_PAUSED_S SERIO_CNP_UICOMPRM_PAUSED_T;
struct SERIO_CNP_UICOMPRM_PAUSED_S
{
	UINT16					Reason;		/* ��~�v��							*/
	UINT16					SubCode;	/* ��~�v���T�u�R�[�h				*/
};
/*--------------------------------------------------------------------------*/

/* LOCK�R�}���h����w���p�����[�^											
SERIO_CNP_LOCKUSER_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`			*/

/* UNLOCK�R�}���h����w���p�����[�^											
SERIO_CNP_UNLOCKUSER_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`			*/

/* DB Read�R�}���h����w���p�����[�^										
SERIO_CNP_DBJOB_READ_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`			*/

/* DB Write�R�}���h����w���p�����[�^										
SERIO_CNP_DBJOB_WRITE_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`		*/

/* IOJOB:Copy�R�}���h����w���p�����[�^										
SERIO_CNP_IOJOB_COPY_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`			*/

/* IOJOB:Send Fax�R�}���h����w���p�����[�^									
SERIO_CNP_IOJOB_SENDFAX_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`		*/

/* IOJOB:Scan���M�R�}���h����w���p�����[�^									
SERIO_CNP_IOJOB_SCANSEND_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`		*/

/* IOJOB:��������MFAX�v�����g�R�}���h����w���p�����[�^					
SERIO_CNP_IOJOB_PRNMEMFAX_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`	*/

/* IOJOB:DirectPrint�R�}���h����w���p�����[�^								
SERIO_CNP_IOJOB_DIRECTPRN_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`	*/

/* IOJOB:Stored�W���u�v�����g�R�}���h����w���p�����[�^						
SERIO_CNP_IOJOB_PRNSPOOL_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`		*/

/* IOJOB:PC�v�����g�R�}���h����w���p�����[�^								
SERIO_CNP_IOJOB_PCPRN_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`		*/

/* IOJOB:Media Read + Upload�R�}���h����w���p�����[�^						
SERIO_CNP_IOJOB_MEDIAUPLOAD_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`	*/

/* IOJOB:Scan + Upload�R�}���h����w���p�����[�^							
SERIO_CNP_IOJOB_SCANUPLOAD_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`	*/

/* IOJOB:Downpload + Print�R�}���h����w���p�����[�^						
SERIO_CNP_IOJOB_DOWNLOADPRINT_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`*/

/* IOJOB:Downpload + Media�R�}���h����w���p�����[�^						
SERIO_CNP_IOJOB_DOWNLOADMEDIA_T��serio_cmpparam.h,SerioCommandStub.h�ɂĒ�`*/



/*==========================================================================*/
/*  Event Parameter															*/
/*==========================================================================*/

/*--------------------------------------------------------------------------*/
/*	UI�I���C�x���g�p�����[�^												*/
/*--------------------------------------------------------------------------*/
typedef struct SERIO_EVP_UIEND_S SERIO_EVP_UIEND_T;
struct SERIO_EVP_UIEND_S
{
	UINT16					UIType;		/* UI�^�C�v							*/
	UINT16					Result;		/* UI�������s����					*/
	UINT16					Status;		/* UI���͏I���X�e�[�^�X				*/
	void					*InputResult;/* UI���͌���						*/
	INT32					InputResultSize;/* UI���͌��ʂ̃f�[�^�T�C�Y		*/
};

/*-----UI�I���C�x���g�ڍ׃p�����[�^ ----------------------------------------*/

/*	��SERIOFW_UIDEMTYPE_LINK�p�����[�^					*/
typedef struct SERIO_EVP_UIEND_LINK_S SERIO_EVP_UIEND_LINK_T;
struct SERIO_EVP_UIEND_LINK_S
{
	char					*Href;		/* �I�����ꂽ�����N�̎Q�Ɛ�			*/
};

/*--------------------------------------------------------------------------*/
/*	�W���u�X�e�[�^�X�C�x���g�p�����[�^										*/
/*--------------------------------------------------------------------------*/

/* ���W���u�X�e�[�^�X��Detail�̏ڍ׃p�����[�^								*/
typedef struct SERIO_JOBSTS_PARAM_DETAIL_S SERIO_JOBSTS_PARAM_DETAIL_T;
struct SERIO_JOBSTS_PARAM_DETAIL_S
{
	UINT32						ReasonDetail;	/* �ω��v���ڍ�				*/
	UINT32						UstatusCode;	/* UstatusCode				*/
	SERIO_USEROPTIONS_T			UserOption;		/* ����d�l					*/
	UINT8*						Description;	/* ����						*/
};

/* ���W���u�X�e�[�^�X=PROCESSING���̏ڍ׃p�����[�^							*/
typedef struct SERIO_JOBSTS_PROCESSING_S SERIO_JOBSTS_PROCESSING_T;
struct SERIO_JOBSTS_PROCESSING_S
{
	UINT16						dummy;		/* dummy						*/
	SERIO_JOBSTS_PARAM_DETAIL_T	Detail;		/* �W���u��ԍX�V�ڍ׏��		*/
};
	
/* ���W���u�X�e�[�^�X=PAUSED���̏ڍ׃p�����[�^								*/
typedef struct SERIO_JOBSTS_PAUSED_S SERIO_JOBSTS_PAUSED_T;
struct SERIO_JOBSTS_PAUSED_S
{
	UINT16						Reason;		/* �W���u��~�v��				*/
	UINT16						SubCode;	/* ��~�v���T�u�R�[�h			*/
	SERIO_JOBSTS_PARAM_DETAIL_T	Detail;		/* �W���u��ԍX�V�ڍ׏��		*/
};
	
/* ���W���u�X�e�[�^�X=CANCELING���̏ڍ׃p�����[�^							*/
typedef struct SERIO_JOBSTS_CANCELING_S SERIO_JOBSTS_CANCELING_T;
struct SERIO_JOBSTS_CANCELING_S
{
	UINT16						Reason;		/* �W���u�L�����Z���v��			*/
	SERIO_JOBSTS_PARAM_DETAIL_T	Detail;		/* �W���u��ԍX�V�ڍ׏��		*/
};

/* ���W���u�X�e�[�^�X=END���̏ڍ׃p�����[�^									*/
/****************************************************************************/
/* ���⑫��																	*/
/* SERIO_EV_JOB_FINISH_T�̒�`�́Aserio_cmpparam.h,SerioCommandStub.h��		*/
/* ����܂�																	*/
/****************************************************************************/
typedef struct SERIO_JOBSTS_END_S SERIO_JOBSTS_END_T;
struct SERIO_JOBSTS_END_S
{
	UINT16						Reason;		/* �W���u�I���v��				*/
	SERIO_EV_JOB_FINISH_T		Detail;		/* �W���u���s���ʏڍ׏��			*/
};


/* �W���u�X�e�[�^�X�ڍ׃p�����[�^(Union) 									*/
typedef union SERIO_EVP_JOBSTS_PARAM_U SERIO_EVP_JOBSTS_PARAM_U;
union SERIO_EVP_JOBSTS_PARAM_U
{
	SERIO_JOBSTS_PROCESSING_T	Processing;	/* Processing���̃p�����[�^		*/
	SERIO_JOBSTS_PAUSED_T		Paused;		/* Paused���̃p�����[�^			*/
	SERIO_JOBSTS_CANCELING_T	Canceling;	/* Canceling���̃p�����[�^		*/
	SERIO_JOBSTS_END_T			End;		/* End���̃p�����[�^			*/
};


/*-----�W���u�X�e�[�^�X�p�����[�^ ------------------------------------------*/
typedef struct SERIO_EVP_JOBSTS_S SERIO_EVP_JOBSTS_T;
struct SERIO_EVP_JOBSTS_S
{
	UINT16						JobID;		/* �W���u����						*/
	UINT16						Status;		/* �W���u�X�e�[�^�X					*/
	SERIO_EVP_JOBSTS_PARAM_U	Param;		/* �W���u�X�e�[�^�X�ڍ׃p�����[�^	*/
};


/*--------------------------------------------------------------------------*/
/*  �W���u�i���C�x���g�p�����[�^											*/
/*--------------------------------------------------------------------------*/
/* �W���u�i���ڍ׃p�����[�^(Union)											*/
/****************************************************************************/
/* ���⑫��																	*/
/* Union�����o�̍\���̂̒�`�́Aserio_cmpparam.h,SerioCommandStub.h��		*/
/* ����܂�																	*/
/****************************************************************************/
typedef union SERIO_EVP_JOBPROG_PARAM_U SERIO_EVP_JOBPROG_PARAM_U;
union SERIO_EVP_JOBPROG_PARAM_U
{
	SERIO_EV_PAGEEND_T			PageEnd;		/* 1Page����I��			*/
	SERIO_EV_TRANSEND_T			TransferEnd;	/* 1����ʐM�I��			*/
	SERIO_EV_UPLOADFILEINFO_T	UpldFileInfo;	/* Upload�t�@�C�����ʒm	*/
	SERIO_EV_DOWNLOADFILEINFO_T	DwldFileInfo;	/* Download�t�@�C�����ʒm	*/
	SERIO_EV_UPLOADFILE_T		FileUploadEnd;	/* 1�t�@�C��Upload�����ʒm	*/
	SERIO_EV_DOWNLOADFILE_T		FileDownloadEnd;/* 1�t�@�C��Download�����ʒm	*/
};
	
/*----- �W���u�i���p�����[�^ -----------------------------------------------*/
typedef struct SERIO_EVP_JOBPROGRESS_S SERIO_EVP_JOBPROGRESS_T;
struct SERIO_EVP_JOBPROGRESS_S
{
	UINT16						JobID;	/* �W���u����						*/
	UINT16						Type;	/* �i�����^�C�v					*/
	SERIO_EVP_JOBPROG_PARAM_U	Param;	/* �W���u�i���ڍ׃p�����[�^			*/
};


/*--------------------------------------------------------------------------*/
/*  �f�o�C�X�X�e�[�^�X�p�����[�^											*/
/*--------------------------------------------------------------------------*/
typedef union SERIO_EVP_DEVSTS_PARAM_U SERIO_EVP_DEVSTS_PARAM_U;
union SERIO_EVP_DEVSTS_PARAM_U
{
	SERIO_EV_DEVSTS_CHANGE_T	Notify;			/* DeviceStatus��ԕω��ʒm	*/
	SERIO_EV_NTFT_ABOTED_T		Abort;			/* �����ʒm	*/
};
/****************************************************************************/
/* ���⑫��																	*/
/* SERIO_EV_DEVSTS_CHANGE_T�̒�`�́Aserio_cmpparam.h��						*/
/* ����܂�																	*/
/****************************************************************************/
typedef struct SERIO_EVP_DEVSTS_S SERIO_EVP_DEVSTS_T;
struct SERIO_EVP_DEVSTS_S
{
	UINT16						Type;		/*  */
	SERIO_EVP_DEVSTS_PARAM_U	Param;		/* ���s����						*/
};


/*--------------------------------------------------------------------------*/
/*  LOCK/UNLOCK���ʃC�x���g�p�����[�^										*/
/*--------------------------------------------------------------------------*/
/****************************************************************************/
/* ���⑫��																	*/
/* SERIO_EV_LOCK_RSLT_T�̒�`�́Aserio_cmpparam.h,SerioCommandStub.h��		*/
/* ����܂�																	*/
/****************************************************************************/
typedef struct SERIO_EVP_LOCKRESULT_S SERIO_EVP_LOCKRESULT_T;
struct SERIO_EVP_LOCKRESULT_S
{
	SERIO_EV_LOCK_RSLT_T	Result;	/* 	���s����					*/
};



/*--------------------------------------------------------------------------*/
/*  DB Read���ʒʒm�C�x���g�p�����[�^										*/
/*--------------------------------------------------------------------------*/
/****************************************************************************/
/* ���⑫��																	*/
/* SERIO_EV_DBREAD_T�̒�`�́Aserio_cmpparam.h,SerioCommandStub.h��			*/
/* ����܂�																	*/
/****************************************************************************/
typedef struct SERIO_EVP_DBREADRESULT_S SERIO_EVP_DBREADRESULT_T;
struct SERIO_EVP_DBREADRESULT_S
{
	SERIO_EV_DBREAD_T		Result;	/* 	���s����					*/
};



/*--------------------------------------------------------------------------*/
/*  DB Write���ʒʒm�C�x���g�p�����[�^										*/
/*--------------------------------------------------------------------------*/
/****************************************************************************/
/* ���⑫��																	*/
/* SERIO_EV_DBWRITE_T�̒�`�́Aserio_cmpparam.h,SerioCommandStub.h��		*/
/* ����܂�																	*/
/****************************************************************************/
typedef struct SERIO_EVP_DBWRITERESULT_S SERIO_EVP_DBWRITERESULT_T;
struct SERIO_EVP_DBWRITERESULT_S
{
	SERIO_EV_DBWRITE_T		Result;	/* 	���s����					*/
};


/*--------------------------------------------------------------------------*/
/*  WaitJbb���ʒʒm�C�x���g�p�����[�^										*/
/*--------------------------------------------------------------------------*/
/****************************************************************************/
/* ���⑫��																	*/
/* SERIO_EV_WAITTOUT_T�̒�`�́Aserio_cmpparam.h,SerioCommandStub.h��		*/
/* ����܂�																	*/
/****************************************************************************/
typedef struct SERIO_EVP_WAITJOBRESULT_S SERIO_EVP_WAITJOBRESULT_T;
struct SERIO_EVP_WAITJOBRESULT_S
{
	SERIO_EV_WAITTOUT_T		Result;	/* 	���s����					*/
};


/*--------------------------------------------------------------------------*/
/*  GetLastUserLog���ʒʒm�C�x���g�p�����[�^										*/
/*--------------------------------------------------------------------------*/
/****************************************************************************/
/* ���⑫��																	*/
/* SERIO_EV_LASTUSERLOG_T�̒�`�́Aserio_cmpparam.h,SerioCommandStub.h��	*/
/* ����܂�																	*/
/****************************************************************************/
typedef struct SERIO_EVP_LASTUSERLOGRESULT_S SERIO_EVP_LASTUSERLOGRESULT_T;
struct SERIO_EVP_LASTUSERLOGRESULT_S
{
	SERIO_EV_LASTUSERLOG_T	Result;	/* 	���s����					*/
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
