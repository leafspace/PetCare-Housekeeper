/************************************************************************/
/*																		*/
/*	Serio Command/Response パラメータ定義（外部公開用）					*/
/*																		*/
/************************************************************************/

/****** This file *******************************************************/
/*	file name = serio_cmdparam.h										*/
/*	Tabstop   = 4														*/
/*																		*/
/*	Copyright: 2010 - 20xx brother Industries , Ltd.					*/
/*																		*/
/*	$Id: $																*/
/*																		*/
/*	ver 1.0.0 : 2010.05.20 :  新規作成	榎本 							*/
/*																		*/
/************************************************************************/
#ifndef _SERIOCMDPARAM_H
#define _SERIOCMDPARAM_H

#include	"serio_parserbuilder_IF.h"

/*****************************************************************************
 *  Constant value 
 ****************************************************************************/
/*-- Command Type --*/
#define SERIO_CMDTYPE_USERLOCK       SOAP_UNION__ns__union_CommandType_ActivateLock
#define SERIO_CMDTYPE_USERUNLOCK     SOAP_UNION__ns__union_CommandType_DeactivateLock
#define SERIO_CMDTYPE_DISPFORM       SOAP_UNION__ns__union_CommandType_DisplayForm
#define SERIO_CMDTYPE_DISPINFO       SOAP_UNION__ns__union_CommandType_DisplayInfo
#define SERIO_CMDTYPE_READDB         SOAP_UNION__ns__union_CommandType_ReadDb
#define SERIO_CMDTYPE_READDBBUNDLE   SOAP_UNION__ns__union_CommandType_ReadDbBundle
#define SERIO_CMDTYPE_WRITEDB        SOAP_UNION__ns__union_CommandType_UpdateDb
#define SERIO_CMDTYPE_COPY           SOAP_UNION__ns__union_CommandType_IoCopy
#define SERIO_CMDTYPE_SENDFAX        SOAP_UNION__ns__union_CommandType_IoSendFax
#define SERIO_CMDTYPE_SCANTO         SOAP_UNION__ns__union_CommandType_IoScanAndSend
#define SERIO_CMDTYPE_DIRPRINT       SOAP_UNION__ns__union_CommandType_IoDirectPrint
#define SERIO_CMDTYPE_PRINTFAXMEM    SOAP_UNION__ns__union_CommandType_IoPrintFaxmem
#define SERIO_CMDTYPE_PRINTSPOOL     SOAP_UNION__ns__union_CommandType_IoPrintSpool
#define SERIO_CMDTYPE_PCPRINT        SOAP_UNION__ns__union_CommandType_IoPcPrint
#define SERIO_CMDTYPE_LASTJOBRSLTRQ  SOAP_UNION__ns__union_CommandType_LastJobResultReq
#define SERIO_CMDTYPE_NTFYDEVSTS     SOAP_UNION__ns__union_CommandType_NotifyDevStatus
#define SERIO_CMDTYPE_CLOSESESSION   SOAP_UNION__ns__union_CommandType_CloseSession
#define SERIO_CMDTYPE_WAIT           SOAP_UNION__ns__union_CommandType_Wait
#define SERIO_CMDTYPE_GETLASTUSERLOG SOAP_UNION__ns__union_CommandType_GetLastUserLog
#define SERIO_CMDTYPE_SCANUPLOAD     SOAP_UNION__ns__union_CommandType_IoScanAndUload
#define SERIO_CMDTYPE_MEDIAUPLOAD    SOAP_UNION__ns__union_CommandType_IoUloadFromMedia
#define SERIO_CMDTYPE_DOWNLOADPRINT  SOAP_UNION__ns__union_CommandType_IoDloadAndPrint
#define SERIO_CMDTYPE_DOWNLOADPRINT2 SOAP_UNION__ns__union_CommandType_IoDloadAndPrint2
#define SERIO_CMDTYPE_DOWNLOADMEDIA  SOAP_UNION__ns__union_CommandType_IoDloadToMedia
#define SERIO_CMDTYPE_DOWNLOADMEDIA2 SOAP_UNION__ns__union_CommandType_IoDloadToMedia2
#define SERIO_CMDTYPE_UPLOADSTART    SOAP_UNION__ns__union_CommandType_IoStartUload
#define SERIO_CMDTYPE_DWLOADSTART    SOAP_UNION__ns__union_CommandType_IoStartDload
#define SERIO_CMDTYPE_UPLOADCONT     SOAP_UNION__ns__union_CommandType_IoContinueUloading
#define SERIO_CMDTYPE_DWLOADCONT     SOAP_UNION__ns__union_CommandType_IoContinueDloading
#define SERIO_CMDTYPE_STARTBROTHERCP SOAP_UNION__ns__union_CommandType_IoStartBrotherCP
#define SERIO_CMDTYPE_MEDIASEND      SOAP_UNION__ns__union_CommandType_IoSendFromMedia
#define SERIO_CMDTYPE_IOJOBCONT      SOAP_UNION__ns__union_CommandType_IoContinue

/*-----------------------------------------------------------------------------
 * 注意!
 * コマンドタイプを追加する場合はGetSyncCmdEntryList()内のcase追加をする必要があります。
------------------------------------------------------------------------------*/

/*-- DB Write Type --*/
#define	SERIO_DBPUT_DATATYPE_PJL	SOAP_UNION__ns__union_UpdateDataType_Pjl
#define	SERIO_DBPUT_DATATYPE_MIB	SOAP_UNION__ns__union_UpdateDataType_Mib

/*-- UI Control Type --*/
#define	SERIO_UICTRL_INFO			SOAP_UNION__ns2__union_ControlType_InfoControl
#define	SERIO_UICTRL_FORM			SOAP_UNION__ns2__union_ControlType_FormControl

/*-- UI Screen Type --*/
#define	SERIO_UISCREEN_IO			SOAP_UNION__ns2__union_ScreenType_IoScreen
#define	SERIO_UISCREEN_LINK			SOAP_UNION__ns2__union_ScreenType_LinkScreen
#define	SERIO_UISCREEN_NULL			SOAP_UNION__ns2__union_ScreenType_NullScreen

/*-- UI Form Control Type --*/
#define	SERIO_FORMCTRL_TEXTAREA		SOAP_UNION__ns2__union_DispControlType_Textarea
#define	SERIO_FORMCTRL_SELECTION	SOAP_UNION__ns2__union_DispControlType_Selection

/*-- UI Info Control InfoType --*/
#define	SERIO_INFOCTRL_MESSAGE		SOAP_UNION__ns2__union_InfoType_Message
#define	SERIO_INFOCTRL_IMAGE		SOAP_UNION__ns2__union_InfoType_Img

/*-- UI Link Control Type --*/
#define	SERIO_UILINKTYPE_LINK		SOAP_UNION__ns2__union_LinkScreenControlType_LinkControl
#define	SERIO_UILINKTYPE_EITHEROR	SOAP_UNION__ns2__union_LinkScreenControlType_EitherOrControl

/*-- UI Operation Type --*/
#define	SERIO_UIOPETYPE_SUBMIT		ns2__OpType__Submit
#define	SERIO_UIOPETYPE_SKIP		ns2__OpType__Skip
#define	SERIO_UIOPETYPE_SKIPGO		ns2__OpType__SkipGo
#define	SERIO_UIOPETYPE_BACK		ns2__OpType__Back
#define	SERIO_UIOPETYPE_CUSTOM		ns2__OpType__Custom
#define	SERIO_UIOPETYPE_PREV		ns2__OpType__Prev
#define	SERIO_UIOPETYPE_NEXT		ns2__OpType__Next

/*-- UI Screen Type --*/
#define SERIO_UIIOTYPE_MESSAGE		SOAP_UNION__ns2__union_IoType_Message
#define SERIO_UIIOTYPE_IMAGE		SOAP_UNION__ns2__union_IoType_Img
#define SERIO_UIIOTYPE_TEXTAREA		SOAP_UNION__ns2__union_IoType_TextArea
#define SERIO_UIIOTYPE_NUMERIC		SOAP_UNION__ns2__union_IoType_NumericalArea
#define SERIO_UIIOTYPE_DEVPARAM		SOAP_UNION__ns2__union_IoType_DevParamArea
#define SERIO_UIIOTYPE_SELECT		SOAP_UNION__ns2__union_IoType_Selection
#define SERIO_UIIOTYPE_FILSEL		SOAP_UNION__ns2__union_IoType_FileSelection
#define SERIO_UIIOTYPE_IDSCANNER	SOAP_UNION__ns2__union_IoType_IdScanner
#define SERIO_UIIOTYPE_SHORTCUT		SOAP_UNION__ns2__union_IoType_Shortcut
#define SERIO_UIIOTYPE__ANY SOAP_UNION__ns2__union_IoType___any

/*-- UI Input Letter Type --*/
#define SERIO_LETTERTYPE_ALPHUPR	ns2__LetterType__UpperCase
#define SERIO_LETTERTYPE_ALPHLWR	ns2__LetterType__LowerCase
#define SERIO_LETTERTYPE_NUMERIC	ns2__LetterType__Numeric
#define SERIO_LETTERTYPE_SIGN		ns2__LetterType__Glyph
#define SERIO_LETTERTYPE_ENVDEP		ns2__LetterType__EnvDependent
#define SERIO_LETTERTYPE_NAMEINPUT	ns2__LetterType__NameInputSet

/*-- Event Type ---*/
#define	SERIO_EVTYPE_LAUNCHREQ        SOAP_UNION__ns__union_EventType_LaunchReqReceived
#define	SERIO_EVTYPE_LOCKRSLT         SOAP_UNION__ns__union_EventType_LockDone
#define	SERIO_EVTYPE_USERINPUT        SOAP_UNION__ns__union_EventType_UserInput
#define	SERIO_EVTYPE_DBREAD           SOAP_UNION__ns__union_EventType_DbReadDone
#define	SERIO_EVTYPE_DBWRITE          SOAP_UNION__ns__union_EventType_DbUpdateDone
#define	SERIO_EVTYPE_JOBRECEIVED      SOAP_UNION__ns__union_EventType_JobReceived
#define	SERIO_EVTYPE_JOBSTARTED       SOAP_UNION__ns__union_EventType_JobStarted
#define	SERIO_EVTYPE_PAGEEND          SOAP_UNION__ns__union_EventType_JobOnePagePrinted
#define	SERIO_EVTYPE_TRANSEND         SOAP_UNION__ns__union_EventType_JobDataCommDone
#define	SERIO_EVTYPE_TRANSFILENAME    SOAP_UNION__ns__union_EventType_TransFileName
#define	SERIO_EVTYPE_UPLOADFILEINFO   SOAP_UNION__ns__union_EventType_UlImgIdentified
#define	SERIO_EVTYPE_DOWNLOADFILEINFO SOAP_UNION__ns__union_EventType_DlImgIdentified
#define	SERIO_EVTYPE_UPLOADFILE       SOAP_UNION__ns__union_EventType_JobOneFileUloaded
#define	SERIO_EVTYPE_DOWNLOADFILE     SOAP_UNION__ns__union_EventType_JobOneFileDloaded
#define	SERIO_EVTYPE_DEVSTSCHANGED    SOAP_UNION__ns__union_EventType_DeviceStatusChanged
#define	SERIO_EVTYPE_JOBSTSCHANGED    SOAP_UNION__ns__union_EventType_JobStatusChanged
#define	SERIO_EVTYPE_JOBFINISHED      SOAP_UNION__ns__union_EventType_JobDone
#define	SERIO_EVTYPE_NTFTABORTED      SOAP_UNION__ns__union_EventType_NotificationAborted
#define	SERIO_EVTYPE_SESSIONCLOSED    SOAP_UNION__ns__union_EventType_SessionClosed
#define	SERIO_EVTYPE_WAITTOUT         SOAP_UNION__ns__union_EventType_WaitTimeOut
#define SERIO_EVTYPE_LASTUSERLOG      SOAP_UNION__ns__union_EventType_LastUserLog

/*-- External Notify ErrorCode ---*/
#define	SERIO_EXTERR_COMPLETE		(1)			/* 正常終了 */
#define	SERIO_EXTERR_ILLEGALCMD		(10)		/* Command異常 */
#define	SERIO_EXTERR_SYSTEMERR		(20)		/* 複合機System異常 */
#define	SERIO_EXTERR_ABORTBYUSER	(30)		/* 利用者強制終了 */
#define	SERIO_EXTERR_LOCKED			(40)		/* Lock設定済み */
#define SERIO_EXTERR_TRMNTDBYSVR    (50)        /* 外部サーバによる強制終了 */
#define	SERIO_EXTERR_NOTFIND		(70)		/* 対象識別情報異常 */
#define	SERIO_EXTERR_ILLEAGAPARAM	(80)		/* 値形式異常 */
#define	SERIO_EXTERR_NOTPERMISSION	(90)		/* Access権異常 */
#define	SERIO_EXTERR_FUNCEXTLOCK	(100)		/* 機能利用拒絶（外部） */
#define	SERIO_EXTERR_FUNCINTLOCK	(110)		/* 機能利用拒絶（内部） */
#define	SERIO_EXTERR_LIMEXTEXCEED	(120)		/* 利用上限到達（外部） */
#define	SERIO_EXTERR_LIMINTEXCEED	(130)		/* 利用上限到達（内部） */
#define	SERIO_EXTERR_SYSTEMBUSY		(140)		/* ビジー異常 */
#define	SERIO_EXTERR_INABILITY		(150)		/* 機能実行能力異常 */
#define	SERIO_EXTERR_COMMFAIL		(160)		/* 通信異常 */
#define	SERIO_EXTERR_EVENTTOUT		(170)		/* Event応答Time-out */
#define	SERIO_EXTERR_SEESIONTOUT	(180)		/* Session Time-out */
#define	SERIO_EXTERR_UITOUT			(200)		/* UI Time-out */
#define	SERIO_EXTERR_OTHER			(990)		/* その他異常 */

/*-- Command DeviceStatus Type --*/
#define SERIO_STATUSTYPE_PRTALERT	ns__StatusType__prtAlert

/*-- Ack Type ------------------------------------------------------------
 *  このenumの並び順は、AckUrlSelectTBl[][]の列(横の並び)の順番に
 *  同期していなければならない
 *----------------------------------------------------------------------*/
enum {
		SERIO_ACKTYPE_CMDRECV=1,				/* コマンド受信ACK		*/
		SERIO_ACKTYPE_JOBSTART,					/* JOB開始ACK			*/
		SERIO_ACKTYPE_JOBEND,					/* JOB終了ACK			*/
		SERIO_ACKTYPE_PRINTPAGE,				/* 1Page印刷終了ACK		*/
		SERIO_ACKTYPE_DATACOMM,					/* 1あて先通信終了ACK	*/
		SERIO_ACKTYPE_UPLOADFILEINFO,			/* UpLoadFile情報通知	*/
		SERIO_ACKTYPE_UPLOADFILE,				/* １File UpLoad通知	*/
		SERIO_ACKTYPE_DOWNLOADFILE,				/* １File DownLoad通知	*/
		SERIO_ACKTYPE_DOWNLOADFILEINFO,			/* DownLoadFile情報通知	*/
		SERIO_ACKTYPE_JOBSTATUS,				/* Job状態通知ACK	*/
		SERIO_ACKTYPE_FILENAME,		
		SERIO_ACKTYPE_MAX
};

/* Ack URL Offset */
#define	USERLOCK_JOBEND_URL			((INT32)(&(((struct ns__ComActivateLock *)0)->JobFinAckUrl)))
#define	USERUNLOCK_JOBEND_URL		((INT32)(&(((struct ns__ComDeactivateLock *)0)->JobFinAckUrl)))
#define	DBJOB_READ_JOBEND_URL		((INT32)(&(((struct ns__ComReadDb *)0)->JobFinAckUrl)))
#define	DBJOB_READBUNDLE_JOBEND_URL	((INT32)(&(((struct ns__ComReadDbBundle *)0)->JobFinAckUrl)))
#define	DBJOB_WRITE_JOBEND_URL		((INT32)(&(((struct ns__ComUpdateDb *)0)->JobFinAckUrl)))
#define	IOJOB_COPY_CMDRECV_URL		((INT32)(&(((struct ns__ComIoCopy *)0)->CmdRecvAckUrl)))
#define	IOJOB_COPY_JOBSTART_URL		((INT32)(&(((struct ns__ComIoCopy *)0)->JobStartAckUrl)))
#define	IOJOB_COPY_JOBEND_URL		((INT32)(&(((struct ns__ComIoCopy *)0)->JobFinAckUrl)))
#define	IOJOB_COPY_PRINTPAGE_URL	((INT32)(&(((struct ns__ComIoCopy *)0)->PagePrintAckUrl)))
#define	IOJOB_COPY_JOBSTATUS_URL	((INT32)(&(((struct ns__ComIoCopy *)0)->JobStatusAckUrl)))
#define	IOJOB_SENDFAX_CMDRECV_URL	((INT32)(&(((struct ns__ComIoSendFax *)0)->CmdRecvAckUrl)))
#define	IOJOB_SENDFAX_JOBSTART_URL	((INT32)(&(((struct ns__ComIoSendFax *)0)->JobStartAckUrl)))
#define	IOJOB_SENDFAX_JOBEND_URL	((INT32)(&(((struct ns__ComIoSendFax *)0)->JobFinAckUrl)))
#define	IOJOB_SENDFAX_DATACOMM_URL	((INT32)(&(((struct ns__ComIoSendFax *)0)->IoDataCommAckUrl)))
#define	IOJOB_SENDFAX_JOBSTATUS_URL	((INT32)(&(((struct ns__ComIoSendFax *)0)->JobStatusAckUrl)))
#define	IOJOB_SCANTO_CMDRECV_URL	((INT32)(&(((struct ns__ComIoScanAndSend *)0)->CmdRecvAckUrl)))
#define	IOJOB_SCANTO_JOBSTART_URL	((INT32)(&(((struct ns__ComIoScanAndSend *)0)->JobStartAckUrl)))
#define	IOJOB_SCANTO_FILENAME_URL	((INT32)(&(((struct ns__ComIoScanAndSend *)0)->FileNameAckUrl)))
#define	IOJOB_SCANTO_JOBEND_URL		((INT32)(&(((struct ns__ComIoScanAndSend *)0)->JobFinAckUrl)))
#define	IOJOB_SCANTO_DATACOMM_URL	((INT32)(&(((struct ns__ComIoScanAndSend *)0)->IoDataCommAckUrl)))
#define	IOJOB_SCANTO_JOBSTATUS_URL	((INT32)(&(((struct ns__ComIoScanAndSend *)0)->JobStatusAckUrl)))
#define	IOJOB_DIRPRN_CMDRECV_URL	((INT32)(&(((struct ns__ComIoDirectPrint *)0)->CmdRecvAckUrl)))
#define	IOJOB_DIRPRN_JOBSTART_URL	((INT32)(&(((struct ns__ComIoDirectPrint *)0)->JobStartAckUrl)))
#define	IOJOB_DIRPRN_JOBEND_URL		((INT32)(&(((struct ns__ComIoDirectPrint *)0)->JobFinAckUrl)))
#define	IOJOB_DIRPRN_PRINTPAGE_URL	((INT32)(&(((struct ns__ComIoDirectPrint *)0)->PagePrintAckUrl)))
#define	IOJOB_DIRPRN_JOBSTATUS_URL	((INT32)(&(((struct ns__ComIoDirectPrint *)0)->JobStatusAckUrl)))
#define	IOJOB_PRNMFAX_CMDRECV_URL	((INT32)(&(((struct ns__ComIoPrintFaxmem *)0)->CmdRecvAckUrl)))
#define	IOJOB_PRNMFAX_JOBSTART_URL	((INT32)(&(((struct ns__ComIoPrintFaxmem *)0)->JobStartAckUrl)))
#define	IOJOB_PRNMFAX_JOBEND_URL	((INT32)(&(((struct ns__ComIoPrintFaxmem *)0)->JobFinAckUrl)))
#define	IOJOB_PRNMFAX_PRINTPAGE_URL	((INT32)(&(((struct ns__ComIoPrintFaxmem *)0)->PagePrintAckUrl)))
#define	IOJOB_PRNMFAX_JOBSTATUS_URL	((INT32)(&(((struct ns__ComIoPrintFaxmem *)0)->JobStatusAckUrl)))
#define	IOJOB_SPLPRN_CMDRECV_URL	((INT32)(&(((struct ns__ComIoPrintSpool *)0)->CmdRecvAckUrl)))
#define	IOJOB_SPLPRN_JOBSTART_URL	((INT32)(&(((struct ns__ComIoPrintSpool *)0)->JobStartAckUrl)))
#define	IOJOB_SPLPRN_JOBEND_URL		((INT32)(&(((struct ns__ComIoPrintSpool *)0)->JobFinAckUrl)))
#define	IOJOB_SPLPRN_PRINTPAGE_URL	((INT32)(&(((struct ns__ComIoPrintSpool *)0)->PagePrintAckUrl)))
#define	IOJOB_SPLPRN_JOBSTATUS_URL	((INT32)(&(((struct ns__ComIoPrintSpool *)0)->JobStatusAckUrl)))
#define	IOJOB_PCPRN_CMDRECV_URL		((INT32)(&(((struct ns__ComIoPcPrint *)0)->CmdRecvAckUrl)))
#define	IOJOB_PCPRN_JOBSTART_URL	((INT32)(&(((struct ns__ComIoPcPrint *)0)->JobStartAckUrl)))
#define	IOJOB_PCPRN_JOBEND_URL		((INT32)(&(((struct ns__ComIoPcPrint *)0)->JobFinAckUrl)))
#define	IOJOB_PCPRN_PRINTPAGE_URL	((INT32)(&(((struct ns__ComIoPcPrint *)0)->PagePrintAckUrl)))
#define	IOJOB_PCPRN_JOBSTATUS_URL	((INT32)(&(((struct ns__ComIoPcPrint *)0)->JobStatusAckUrl)))
#define	IOJOB_UPSCAN_CMDRECV_URL	((INT32)(&(((struct ns__ComIoScanAndUload *)0)->CmdRecvAckUrl)))
#define	IOJOB_UPSCAN_JOBSTART_URL	((INT32)(&(((struct ns__ComIoScanAndUload *)0)->JobStartAckUrl)))
#define	IOJOB_UPSCAN_JOBEND_URL		((INT32)(&(((struct ns__ComIoScanAndUload *)0)->JobFinAckUrl)))
#define	IOJOB_UPSCAN_UPFILEINFO_URL	((INT32)(&(((struct ns__ComIoScanAndUload *)0)->FileInfoAckUrl)))
#define	IOJOB_UPSCAN_UPFILEEND_URL	((INT32)(&(((struct ns__ComIoScanAndUload *)0)->EachUloadAckUrl)))
#define	IOJOB_UPMEDIA_CMDRECV_URL	((INT32)(&(((struct ns__ComIoUloadFromMedia *)0)->CmdRecvAckUrl)))
#define	IOJOB_UPMEDIA_JOBSTART_URL	((INT32)(&(((struct ns__ComIoUloadFromMedia *)0)->JobStartAckUrl)))
#define	IOJOB_UPMEDIA_JOBEND_URL	((INT32)(&(((struct ns__ComIoUloadFromMedia *)0)->JobFinAckUrl)))
#define	IOJOB_UPMEDIA_UPFILEINFO_URL ((INT32)(&(((struct ns__ComIoUloadFromMedia *)0)->FileInfoAckUrl)))
#define	IOJOB_UPMEDIA_UPFILEEND_URL	((INT32)(&(((struct ns__ComIoUloadFromMedia *)0)->EachUloadAckUrl)))
#define	IOJOB_DWNMEIDA_CMDRECV_URL	((INT32)(&(((struct ns__ComIoDloadToMedia *)0)->CmdRecvAckUrl)))
#define	IOJOB_DWNMEIDA_JOBSTART_URL	((INT32)(&(((struct ns__ComIoDloadToMedia *)0)->JobStartAckUrl)))
#define	IOJOB_DWNMEIDA_JOBEND_URL	((INT32)(&(((struct ns__ComIoDloadToMedia *)0)->JobFinAckUrl)))
#define	IOJOB_DWNMEIDA_DWNFILEEND_URL ((INT32)(&(((struct ns__ComIoDloadToMedia *)0)->EachDloadAckUrl)))
#define	IOJOB_DWNMEIDA2_CMDRECV_URL    ((INT32)(&(((struct ns__ComIoDloadToMedia2 *)0)->CmdRecvAckUrl)))
#define	IOJOB_DWNMEIDA2_JOBSTART_URL   ((INT32)(&(((struct ns__ComIoDloadToMedia2 *)0)->JobStartAckUrl)))
#define	IOJOB_DWNMEIDA2_JOBEND_URL     ((INT32)(&(((struct ns__ComIoDloadToMedia2 *)0)->JobFinAckUrl)))
#define	IOJOB_DWNMEIDA2_DWNFILEEND_URL ((INT32)(&(((struct ns__ComIoDloadToMedia2 *)0)->EachDloadAckUrl)))
#define IOJOB_DWNMEIDA2_DWFILEINFO_URL ((INT32)(&(((struct ns__ComIoDloadToMedia2 *)0)->DlFileInfoAckUrl)))
#define IOJOB_SENDMEDIA_CMDRECV_URL  ((INT32)(&(((struct ns__ComIoSendFromMedia *)0)->CmdRecvAckUrl)))
#define IOJOB_SENDMEDIA_JOBSTART_URL ((INT32)(&(((struct ns__ComIoSendFromMedia *)0)->JobStartAckUrl)))
#define IOJOB_SENDMEDIA_DATACOMM_URL ((INT32)(&(((struct ns__ComIoSendFromMedia *)0)->IoDataCommAckUrl)))
#define IOJOB_SENDMEDIA_JOBEND_URL   ((INT32)(&(((struct ns__ComIoSendFromMedia *)0)->JobFinAckUrl)))
#define	IOJOB_DWNPRN_CMDRECV_URL	((INT32)(&(((struct ns__ComIoDloadAndPrint *)0)->CmdRecvAckUrl)))
#define	IOJOB_DWNPRN_JOBSTART_URL	((INT32)(&(((struct ns__ComIoDloadAndPrint *)0)->JobStartAckUrl)))
#define	IOJOB_DWNPRN_JOBEND_URL		((INT32)(&(((struct ns__ComIoDloadAndPrint *)0)->JobFinAckUrl)))
#define	IOJOB_DWNPRN_PRINTPAGE_URL	((INT32)(&(((struct ns__ComIoDloadAndPrint *)0)->PagePrintAckUrl)))
#define	IOJOB_DWNPRN_DWNFILEEND_URL	((INT32)(&(((struct ns__ComIoDloadAndPrint *)0)->EachDloadAckUrl)))
#define	IOJOB_DWNPRN2_CMDRECV_URL    ((INT32)(&(((struct ns__ComIoDloadAndPrint2 *)0)->CmdRecvAckUrl)))
#define	IOJOB_DWNPRN2_JOBSTART_URL   ((INT32)(&(((struct ns__ComIoDloadAndPrint2 *)0)->JobStartAckUrl)))
#define	IOJOB_DWNPRN2_JOBEND_URL	 ((INT32)(&(((struct ns__ComIoDloadAndPrint2 *)0)->JobFinAckUrl)))
#define	IOJOB_DWNPRN2_PRINTPAGE_URL  ((INT32)(&(((struct ns__ComIoDloadAndPrint2 *)0)->PagePrintAckUrl)))
#define	IOJOB_DWNPRN2_DWNFILEEND_URL ((INT32)(&(((struct ns__ComIoDloadAndPrint2 *)0)->EachDloadAckUrl)))
#define IOJOB_DWNPRN2_DWFILEINFO_URL ((INT32)(&(((struct ns__ComIoDloadAndPrint2 *)0)->DlFileInfoAckUrl)))
#define	LASTJOB_RSLT_REQ_JOBEND_URL	((INT32)(&(((struct ns__ComLastJobResultReq *)0)->JobFinAckUrl)))
#define	NTFY_DEVSTS_CMDRECV_URL    ((INT32)(&(((struct ns__ComNotifyDevStatus *)0)->CmdRecvAckUrl)))
#define	NTFY_DEVSTS_JOBSTART_URL   ((INT32)(&(((struct ns__ComNotifyDevStatus *)0)->JobStartAckUrl)))
#define	NTFY_DEVSTS_JOBEND_URL	   ((INT32)(&(((struct ns__ComNotifyDevStatus *)0)->JobFinAckUrl)))
#define NTFY_DEVSTS_STATUS_URL     ((INT32)(&(((struct ns__ComNotifyDevStatus *)0)->JobStatusAckUrl)))
#define	CLOSE_SESS_REQ_JOBEND_URL	((INT32)(&(((struct ns__ComCloseSession *)0)->JobFinAckUrl)))
#define	WAIT_JOBEND_URL				((INT32)(&(((struct ns__ComWait *)0)->JobFinAckUrl)))
#define	GETLOG_JOBEND_URL			((INT32)(&(((struct ns__ComGetLastUserLog *)0)->JobFinAckUrl)))


/*****************************************************************************
 *  Type definition
 ****************************************************************************/
/*-- Job Command Parameter --*/
typedef	struct ns__ComActivateLock		SERIO_CNP_LOCKUSER_T;		/* Lock				*/
typedef	struct ns__ComDeactivateLock	SERIO_CNP_UNLOCKUSER_T;		/* UnLock			*/
typedef	struct ns__ComReadDb			SERIO_CNP_DBJOB_READ_T;		/* DB JOB:DB Read	*/
typedef struct ns__ComReadDbBundle      SERIO_CNP_DBJOB_READBUNDLE_T; /* DB Job:DB Read Bundle */
typedef	struct ns__ComUpdateDb			SERIO_CNP_DBJOB_WRITE_T;	/* DB JOB:DB Write	*/
typedef	struct ns__ComIoCopy			SERIO_CNP_IOJOB_COPY_T;		/* IO JOB:Copy		*/
typedef	struct ns__ComIoSendFax			SERIO_CNP_IOJOB_SENDFAX_T;	/* IO JOB:Send FAX	*/
typedef	struct ns__ComIoScanAndSend		SERIO_CNP_IOJOB_SCANSEND_T;	/* IO JOB:Scan		*/
typedef	struct ns__ComIoDirectPrint		SERIO_CNP_IOJOB_DIRECTPRN_T;/* IO JOB:Scan		*/
typedef	struct ns__ComIoPrintFaxmem		SERIO_CNP_IOJOB_PRNMEMFAX_T;/* IO JOB:PrintFAX	*/
typedef	struct ns__ComIoPrintSpool		SERIO_CNP_IOJOB_PRNSPOOL_T;	/* IO JOB:Spool Print*/
typedef	struct ns__ComIoPcPrint			SERIO_CNP_IOJOB_PCPRN_T;	/* IO JOB:PC Print	*/
typedef	struct ns__ComWait				SERIO_CNP_WAITJOB_T;		/* WAIT JOB			*/
typedef	struct ns__ComIoContinue		SERIO_CNP_IOJOB_CONTINUE_T;	/* IO JOB:Continue	*/
typedef	struct ns__ComNotifyDevStatus	SERIO_CNP_NTFY_DEVSTS_T;	/* Notify DevStatus JOB	*/

typedef	struct ns__LockInfo				SERIO_LOCK_INFO_T;			/* LockコマンドのLock情報	*/

/*-- Job Response/Event Parameter --*/
typedef	struct ns__EvLaunchReqReceived	SERIO_EV_LAUNCH_REQ_T;		/* App起動通知		*/
typedef	struct ns__EvLockDone			SERIO_EV_LOCK_RSLT_T;		/* LOCK/UNLOCK 結果通知	*/
typedef	struct ns__EvUserInput			SERIO_EV_USERINPUT_T;		/* ユーザ入力結果	*/
typedef	struct ns__EvDbReadDone			SERIO_EV_DBREAD_T;			/* DB Read 結果通知	*/
typedef	struct ns__EvDbUpdateDone		SERIO_EV_DBWRITE_T;			/* DB Write 結果通知*/
typedef	struct ns__EvIojobStatusUpdated	SERIO_EV_JOBSTS_UPDATE_T;	/* ジョブ状態更新通知	*/
typedef	struct ns__EvDeviceStatusChanged SERIO_EV_DEVSTS_CHANGE_T;	/* 装置状態更新通知	*/
typedef	struct ns__EvJobStatusChanged	SERIO_EV_JOBSTS_CHANGE_T;	/* ジョブ状態更新通知	*/
typedef	struct ns__EvJobDone			SERIO_EV_JOB_FINISH_T;		/* ジョブ結果通知	*/
typedef	struct ns__EvLaunchReqReceived	SERIO_EV_JOB_LAUNCH_T;		/* ジョブ起動通知	*/
typedef	struct ns__EvJobReceived		SERIO_EV_JOB_RECEIVED_T;	/* ジョブ受信通知	*/
typedef	struct ns__EvJobStarted			SERIO_EV_JOB_STARTED_T;		/* ジョブ開始通知	*/
typedef	struct ns__EvJobOnePagePrinted	SERIO_EV_PAGEEND_T;			/* １ページ完了通知	*/
typedef	struct ns__EvJobDataCommDone	SERIO_EV_TRANSEND_T;		/* １通信完了通知	*/
typedef	struct ns__EvTransFileName		SERIO_EV_TRANSFILENAME_T;	
typedef	struct ns__EvUlImgIdentified	SERIO_EV_UPLOADFILEINFO_T;	/* Uploadファイル情報通知	*/
typedef	struct ns__DloadFile			SERIO_EV_DLOADFILE_T;		/* Downloadファイル情報		*/
typedef	struct ns__DloadFileList		SERIO_EV_DLOADFILELIST_T;	/* Downloadファイルリスト情報 */
typedef	struct ns__EvDlImgIdentified	SERIO_EV_DOWNLOADFILEINFO_T;/* Downloadファイル情報通知	*/
typedef	struct ns__EvJobOneFileUloaded	SERIO_EV_UPLOADFILE_T;		/* １ファイルUpload完了通知	*/
typedef	struct ns__EvJobOneFileDloaded	SERIO_EV_DOWNLOADFILE_T;	/* １ファイルDownload完了通知	*/
typedef	struct ns__EvNotificationAborted SERIO_EV_NTFT_ABOTED_T;	/* Notification終了通知	*/
typedef	struct ns__EvSessionClosed		SERIO_EV_SESSIONCLOSE_T;	/* Sessionクローズ通知	*/
typedef	struct ns__EvWaitTimeOut		SERIO_EV_WAITTOUT_T;		/* Waitタイムアウト通知	*/
typedef	struct ns__EvLastUserLog		SERIO_EV_LASTUSERLOG_T;		/* LastUserLog 通知	*/
typedef	struct ns__Value				SERIO_VALUE_T;				/* Keyに対応づけられたデータ */
typedef	struct ns__KeyValueData			SERIO_KEYVALUE_DATA_T;		/* Keyに対応づけられたデータ */
typedef	struct ns__KeyValueDataArray	SERIO_KEYVALUE_DATA_ARRAY_T;/* Keyに対応づけられたデータ */

typedef struct ns__DbReadResultList		SERIO_DBREADRESULTLIST_T;
typedef struct ns__DbReadResult			SERIO_DBREADRESULT_T;
typedef struct ns__DbUpdateResultList	SERIO_DBUPDATERESULTLIST_T;
typedef struct ns__DbUpdateResult		SERIO_DBUPDATERESULT_T;
typedef struct ns__ValueType			SERIO_DBJOB_VALUETYPE_T;
typedef struct ns__MibDataType			SERIO_DBJOB_MIBDATATYPE_T;
typedef struct ns__JobLogs				SERIO_JOBLOGS_T;
typedef struct ns__JobLog				SERIO_JOBLOG_T;
typedef struct ns__LogItem				SERIO_LOGITEM_T;
typedef struct ns__Status				SERIO_STATUS_T;
typedef struct ns__Statuses				SERIO_STATUSES_T;
typedef struct ns__StsItem				SERIO_STSITEM_T;
typedef struct ns__UserOptions			SERIO_USEROPTIONS_T;
typedef struct ns__EventUserId          SERIO_EVENTUSERID_T;

/*-- UIJOB Command Parameter --*/
typedef	struct ns2__UiScreen			SERIO_UISCREEN_T;			/* UI Screen		*/
typedef	struct ns2__IoScreen			SERIO_UISCREEN_IO_T;		/* IO Screen		*/
typedef	struct ns2__LinkScreen			SERIO_UISCREEN_LINK_T;		/* Link Screen		*/
typedef	struct ns2__IoObject			SERIO_IOOBJECT_T;			/* IO Object		*/
typedef	struct ns2__InfoControl			SERIO_UIOBJ_INFOCTRL_T;		/* Info Control		*/
typedef	struct ns2__FormControl			SERIO_UIOBJ_FORMCTRL_T;		/* Form Control		*/
typedef	struct ns2__Textarea			SERIO_UIOBJ_TEXTAREA_T;		/* Text Area		*/
typedef	struct ns2__Slection			SERIO_UIOBJ_SELECT_T;		/* Select Control	*/
typedef	struct ns2__OperationList		SERIO_UIOPERATION_LIST_T;	/* Operation List	*/
typedef	struct ns2__LinkControl			SERIO_UILINK_LINK_T;		/* Link Control		*/
typedef	struct ns2__EitherOrControl		SERIO_UILINK_EITHEROR_T;	/* Either Or Control*/
typedef	struct ns2__Label				SERIO_IOOBJ_LABELIMG_T;		/* Label Image		*/
typedef	struct ns2__TextArea			SERIO_IOOBJ_TEXT_T;			/* Text Area		*/
typedef	struct ns2__NumericalArea		SERIO_IOOBJ_NUMERIC_T;		/* Numeric Area		*/
typedef	struct ns2__DevParamArea		SERIO_IOOBJ_DEVPARAM_T;		/* Device Parameter Area		*/
typedef	struct ns2__Selection			SERIO_IOOBJ_SELECT_T;		/* Select			*/
typedef	struct ns2__FileSelection		SERIO_IOOBJ_FILSEL_T;		/* File Select		*/
typedef	struct ns2__Operation			SERIO_UISCREEN_OPE_T;		/* Operation		*/
typedef struct ns2__LettertypeList      SERIO_UI_LETTERTYPELIST_T;  /* Lettertype */
typedef struct ns2__ItemDetailList      SERIO_UI_ITEM_DETAILLIST_T; /* Select > Item > Details */
typedef struct ns2__Detail              SERIO_UI_ITEM_DETAIL_T;     /* Select > Item > Details > Detail */
typedef struct ns2__IdScanner           SERIO_IOOBJ_IDSCAN_T;       /* ID Scanner */
typedef struct ns2__Shortcut            SERIO_IOOBJ_SHORTCUT_T;     /* Shortcut */

/*****************************************************************************
 *  Macro
 ****************************************************************************/
#define	GetUiScreenType(X)				(((SERIO_UISCREEN_T *)(X))->__union_ScreenType)
#define	GetUiIoObjects(X)				(((SERIO_UISCREEN_T *)(X))->union_ScreenType.IoScreen.IoObject)
#define	GetUiIoObjectNum(X)				(((SERIO_UISCREEN_T *)(X))->union_ScreenType.IoScreen.__sizeIoObject)
#define	GetUiLinkType(X)				(((SERIO_UISCREEN_T *)(X))->union_ScreenType.LinkScreen.__union_LinkScreenControlType)
#define	GetUiLinkControl(X)				(&(((SERIO_UISCREEN_T *)(X))->union_ScreenType.LinkScreen.union_LinkScreenControlType.LinkControl))
#define	GetUiEitherOrControl(X)			(&(((SERIO_UISCREEN_T *)(X))->union_ScreenType.LinkScreen.union_LinkScreenControlType.EitherOrControl))
#define	GetUiScreenTitle(X)				(((SERIO_UISCREEN_T *)(X))->Title)
#define	GetUiLinkTitle(X)				(((SERIO_UISCREEN_T *)(X))->Title)
#define	GetUiLinkDescription(X)			(((SERIO_UISCREEN_T *)(X))->union_ScreenType.LinkScreen.Description)
#define	IsUiScreenOpeList(X)			(((SERIO_UISCREEN_T *)(X))->Operations)
#define	GetUiScreenOpeList(X)			(((SERIO_UISCREEN_T *)(X))->Operations->Op)
#define	GetUiScreenOpeNum(X)			(((SERIO_UISCREEN_T *)(X))->Operations->__sizeOp)
#define	GetUiTypeMessage(X)				(&(((SERIO_IOOBJECT_T *)(X))->union_IoType.Message))
#define	GetUiTypeImage(X)				(&(((SERIO_IOOBJECT_T *)(X))->union_IoType.Img))
#define	GetUiTypeInputText(X)			(&(((SERIO_IOOBJECT_T *)(X))->union_IoType.TextArea))
#define	GetUiTypeInputDevParam(X)		(&(((SERIO_IOOBJECT_T *)(X))->union_IoType.DevParamArea))
#define	GetUiTypeIdScanner(X)			(&(((SERIO_IOOBJECT_T *)(X))->union_IoType.IdScanner))
#define	GetUiTypeInputNum(X)			(&(((SERIO_IOOBJECT_T *)(X))->union_IoType.NumericalArea))
#define	GetUiTypeSelect(X)				(&(((SERIO_IOOBJECT_T *)(X))->union_IoType.Selection))
#define	GetUiTypeFileSelect(X)			(&(((SERIO_IOOBJECT_T *)(X))->union_IoType.FileSelection))
#define	GetUiTypeShortcut(X)			(&(((SERIO_IOOBJECT_T *)(X))->union_IoType.Shortcut))
#define GetUiType(X)                    (((SERIO_IOOBJECT_T *)(X))->__union_IoType)

#endif	/* _SERIOCMDPARAM_H */
