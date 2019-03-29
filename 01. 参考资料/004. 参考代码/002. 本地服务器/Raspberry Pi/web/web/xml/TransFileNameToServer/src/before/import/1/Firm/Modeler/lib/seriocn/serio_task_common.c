/************************************************************************
 *
 *	serio_task_common.c - serioで使用する共通関数群
 *
 *	Copyright: 2010-20XX brother Industries , Ltd.
 *
 *	$Id: //depot/Firm/Commonfile/Laser_origin2/task/serio/serio_task_common.c#2 $
 *	$DateTime: 2011/02/22 20:59:46 $
 *	$Change: 205973 $
 *	$Author: sugiyatk $
 *
 *	ver 1.0.0 : 2010.07.19 : ABS  : 新規作成
 ************************************************************************/

/****** インクルード・ファイル ******************************************/
#include "serio_task_common.h"
#include "componentlib/serio/serio.h"
#include "componentlib/serio/serio_log_rec.h"
#include "task/panel/panelTask.h"

#include "lib/cplib/cp_api_lib.h"

#include "task/cp/cp_task.h"

#undef DEBUG_PRINT

#ifdef DEBUG_PRINT
//#define DPRINTF		EPRINTF
#define MPRINTF		EPRINTF
#else
#define	MPRINTF		DPRINTF
#endif

#ifdef USE_SERIO

/*****************************************************************************
 * Local static variables.
 ****************************************************************************/
STATIC	BOOL	JobRunning = FALSE;
STATIC	BOOL	StateError = FALSE;
#ifdef	USE_CBSI
STATIC	INT32	CloudBSIInfoFlag = FALSE;
#endif	/* USE_CBSI */

/*********************************************************************************************/
/**
* @par		(serio)JOBステータスの通知
* @param	Status(input) ジョブステータス
* @retval	なし
* @par	<外部仕様>
*			SerioタスクへJOBステータスを通知する
* @par	<内部仕様>
* 			引数で指定されたJOBステータスをSerioタスクへ通知する
*/
/*********************************************************************************************/
GLOBAL VOID
SendJobStatus(UINT16 Status, serio_job_category_t serio_job)
{
	SERIO_EVP_JOBSTS_T	jobsts;

#ifdef DEBUG_PRINT
		ID tid;
		if(get_tid(&tid) < 0)
		{
			tid = 0;
		}

    MPRINTF(("%s(%d)	SendJobStatus(TaskID=0x%02x)\n", __FILE__,(__LINE__),tid));
#endif

	memset(&jobsts, 0, sizeof(jobsts));

    switch (serio_job) {
	    case SERIO_JOB_COPY:
	        jobsts.JobID = SERIOFW_IOJOB_COPY;
	        break;
	    case SERIO_JOB_SCANSEND:
	        jobsts.JobID = SERIOFW_IOJOB_SCANSEND;
	        break;
	    case SERIO_JOB_SENDFAX:
	    case SERIO_JOB_SENDIFAX:
	        jobsts.JobID = SERIOFW_IOJOB_SENDFAX;
	        break;
	    case SERIO_JOB_PRINTMEMFAX:
	        jobsts.JobID = SERIOFW_IOJOB_PRNMEMFAX;
	        break;
	    case SERIO_JOB_PCPRINT:
	        jobsts.JobID = SERIOFW_IOJOB_PCPRINT;
	        break;
	    default:
	        return;
    }

	jobsts.Status = Status;
	jobsts.Param.End.Reason = 0;		/* TODO:見直し */
	/*jobsts.Param.End.Detail; */
	SerioFwEventNotify(SERIOFW_EVID_JOBSTATUS, &jobsts);
}

/*********************************************************************************************/
/**
* @par		(serio)JOBステータスの実行終了を通知
* @param	Reason(input) ジョブ終了要因
* @retval	なし
* @par	<外部仕様>
*			SerioタスクへJOBステータスの実行終了を通知する
* @par	<内部仕様>
* 			引数で指定されたジョブ終了要因をSerioタスクへ通知する
*/
/*********************************************************************************************/
GLOBAL VOID
SendJobStatus_End(INT32 Reason, INT32 ErrorInfo, serio_job_category_t serio_job)
{
	SERIO_EVP_JOBSTS_T			jobsts;
#ifdef DEBUG_PRINT
		ID tid;
		if(get_tid(&tid) < 0)
		{
			tid = 0;
		}

    MPRINTF(("%s(%d)	SendJobStatus_End(TaskID=0x%02x)\n", __FILE__,(__LINE__),tid));
#endif

    /* initialize */
    memset(&jobsts, 0, sizeof(jobsts));

    switch (serio_job) {
	    case SERIO_JOB_COPY:
	        jobsts.JobID = SERIOFW_IOJOB_COPY;
	        break;
	    case SERIO_JOB_SCANSEND:
	        jobsts.JobID = SERIOFW_IOJOB_SCANSEND;
	        break;
	    case SERIO_JOB_SENDFAX:
	    case SERIO_JOB_SENDIFAX:
	        jobsts.JobID = SERIOFW_IOJOB_SENDFAX;
	        break;
	    case SERIO_JOB_PRINTMEMFAX:
	        jobsts.JobID = SERIOFW_IOJOB_PRNMEMFAX;
	        break;
	    case SERIO_JOB_PCPRINT:
	        jobsts.JobID = SERIOFW_IOJOB_PCPRINT;
	        break;
	    default:
	        return;
    }

	jobsts.Status = SERIOFW_JOBSTS_END;
	jobsts.Param.End.Reason = Reason;
    jobsts.Param.End.Detail.ErrorInfo = ErrorInfo;
    
#ifdef USE_SERIO_LOG
	set_iojob_end_bsilog(serio_job, Reason);
#endif	/* USE_SERIO_LOG */

	SerioFwEventNotify(SERIOFW_EVID_JOBSTATUS, &jobsts);
}

/*********************************************************************************************/
/**
* @par		(serio)JOBステータスの実行停止中を通知
* @param	Reason(input)  ジョブ停止要因
* @param	SubCode(input) サブコード
* @retval	なし
* @par	<外部仕様>
*			SerioタスクへJOBステータスの実行停止中を通知する
* @par	<内部仕様>
* 			引数で指定されたジョブ停止要因とサブコードをSerioタスクへ通知する
*/
/*********************************************************************************************/
GLOBAL VOID
SendJobStatus_Paused(UINT16 Reason, UINT16 SubCode, serio_job_category_t serio_job)
{
	SERIO_EVP_JOBSTS_T			jobsts;

#ifdef DEBUG_PRINT
		ID tid;
		if(get_tid(&tid) < 0)
		{
			tid = 0;
		}

    MPRINTF(("%s(%d)	SendJobStatus_Paused(TaskID=0x%02x)\n", __FILE__,(__LINE__),tid));
#endif

    memset(&jobsts, 0, sizeof(jobsts));
    switch (serio_job) {
	    case SERIO_JOB_COPY:
		    jobsts.JobID = SERIOFW_IOJOB_COPY;
		    break;
	    case SERIO_JOB_SCANSEND:
	        jobsts.JobID = SERIOFW_IOJOB_SCANSEND;
	        break;
	    case SERIO_JOB_SENDFAX:
	    case SERIO_JOB_SENDIFAX:
	        jobsts.JobID = SERIOFW_IOJOB_SENDFAX;
	        break;
	    case SERIO_JOB_PRINTMEMFAX:
	        jobsts.JobID = SERIOFW_IOJOB_PRNMEMFAX;
	        break;
	    case SERIO_JOB_PCPRINT:
	        jobsts.JobID = SERIOFW_IOJOB_PCPRINT;
	        break;
	    default:
	        return;
    }

	jobsts.Status = SERIOFW_JOBSTS_PAUSED;
	jobsts.Param.Paused.Reason  =  Reason;
	jobsts.Param.Paused.SubCode =  SubCode;
	SerioFwEventNotify(SERIOFW_EVID_JOBSTATUS, &jobsts);
}


/*********************************************************************************************/
/**
* @par		(serio)UI入力終了通知を行う
* @param	Reason(input)  ジョブ停止要因
* @param	SubCode(input) サブコード
* @retval	なし
* @par	<外部仕様>
*			SerioタスクへUI入力終了を通知する
* @par	<内部仕様>
* 			実行停止中のときのみ、この関数が呼ばれる \n
* 			SerioタスクへUI入力終了通知を行い、処理を再開する
*/
/*********************************************************************************************/
GLOBAL VOID
SendUiEnd_Restart(UINT16 Reason, UINT16 SubCode)
{
	SERIO_EVP_JOBSTS_T			jobsts;

#ifdef DEBUG_PRINT
		ID tid;
		if(get_tid(&tid) < 0)
		{
			tid = 0;
		}

    MPRINTF(("%s(%d)	SendUiEnd_Restart(TaskID=0x%02x)\n", __FILE__,(__LINE__),tid));
#endif
    memset(&jobsts, 0, sizeof(SERIO_EVP_JOBSTS_T));
           
	jobsts.Status = SERIOFW_JOBSTS_PROCESSING;
	SerioFwEventNotify(SERIOFW_EVID_JOBSTATUS, &jobsts);
}


/*********************************************************************************************/
/**
* @par		(serio)JOB進捗の1宛先転送終了を通知
* @param	なし
* @retval	なし
* @par	<外部仕様>
*			SerioタスクへJOB進捗の1宛先転送終了を通知する
* @par	<内部仕様>
* 			引数で指定された通信連番をSerioタスクへ通知する
*/
/*********************************************************************************************/
GLOBAL VOID
SendJobProgress_TransEnd(serio_job_category_t serio_job)
{
	SERIO_EVP_JOBPROGRESS_T			JobProg;

#ifdef DEBUG_PRINT
		ID tid;
		if(get_tid(&tid) < 0)
		{
			tid = 0;
		}

    MPRINTF(("%s(%d)	SendJobProgress_TransEnd(TaskID=0x%02x)\n", __FILE__,(__LINE__),tid));
#endif

    switch (serio_job) {
	    case SERIO_JOB_COPY:
		    JobProg.JobID = SERIOFW_IOJOB_COPY;
		    break;
	    case SERIO_JOB_SCANSEND:
	        JobProg.JobID = SERIOFW_IOJOB_SCANSEND;
	        break;
	    case SERIO_JOB_SENDFAX:
	    case SERIO_JOB_SENDIFAX:
	        JobProg.JobID = SERIOFW_IOJOB_SENDFAX;
	        break;
	    case SERIO_JOB_PRINTMEMFAX:
	        JobProg.JobID = SERIOFW_IOJOB_PRNMEMFAX;
	        break;
	    case SERIO_JOB_PCPRINT:
	        JobProg.JobID = SERIOFW_IOJOB_PCPRINT;
	        break;
	    default:
	        return;
    }
	JobProg.Type  = SERIOFW_JOBPROG_TRANSEND;
//	JobProg.Param.TransferEnd.dummy;				/* TODO : dummyのみ */

	SerioFwEventNotify(SERIOFW_EVID_JOBPROGRESS, &JobProg);
}

/*********************************************************************************************/
/**
* @par		(serio)JOB進捗のFileNameを通知
* @param	なし
* @retval	なし
* @par	<外部仕様>
*			SerioタスクへJOB進捗のFileNameを通知する
* @par	<内部仕様>
* 			引数で指定された通信連番をSerioタスクへ通知する
*/
/*********************************************************************************************/
GLOBAL	VOID 
SendJobProgress_TransFileName(serio_job_category_t serio_job, UINT8 *filename)
{
	SERIO_EVP_JOBPROGRESS_T			JobProg;

#ifdef DEBUG_PRINT
		ID tid;
		if(get_tid(&tid) < 0)
		{
			tid = 0;
		}

    MPRINTF(("%s(%d)	SendJobProgress_TransFileName(TaskID=0x%02x)		FileName = %s\n", __FILE__,(__LINE__),tid, filename));
#endif

    switch (serio_job) {
	    case SERIO_JOB_SCANSEND:
	        JobProg.JobID = SERIOFW_IOJOB_SCANSEND;
	        break;
	    default:
	        return;
    }
	JobProg.Type  = SERIOFW_JOBPROG_TRANSFILENAME;
	JobProg.Param.TransferFileName.FileNam = (char *)filename;

	SerioFwEventNotify(SERIOFW_EVID_JOBPROGRESS, &JobProg);
}

/*********************************************************************************************/
/**
* @par		(serio)JOB進捗の1Page印刷終了を通知
* @param	なし
* @retval	なし
* @par	<外部仕様>
*			SerioタスクへJOB進捗の1Page印刷終了を通知する
* @par	<内部仕様>
* 			引数で指定された通信連番をSerioタスクへ通知する
*/
/*********************************************************************************************/
GLOBAL VOID
SendJobProgress_PrintPageEnd(serio_job_category_t serio_job)
{
	SERIO_EVP_JOBPROGRESS_T			JobProg;

#ifdef DEBUG_PRINT
		ID tid;
		if(get_tid(&tid) < 0)
		{
			tid = 0;
		}

    MPRINTF(("%s(%d)	SendJobProgress_PrintPageEnd(TaskID=0x%02x)\n", __FILE__,(__LINE__),tid));
#endif

    switch (serio_job) {
	    case SERIO_JOB_COPY:
		    JobProg.JobID = SERIOFW_IOJOB_COPY;
		    break;
	    case SERIO_JOB_SCANSEND:
	        JobProg.JobID = SERIOFW_IOJOB_SCANSEND;
	        break;
	    case SERIO_JOB_SENDFAX:
	    case SERIO_JOB_SENDIFAX:
	        JobProg.JobID = SERIOFW_IOJOB_SENDFAX;
	        break;
	    case SERIO_JOB_PRINTMEMFAX:
	        JobProg.JobID = SERIOFW_IOJOB_PRNMEMFAX;
	        break;
	    case SERIO_JOB_PCPRINT:
	        JobProg.JobID = SERIOFW_IOJOB_PCPRINT;
	        break;
	   default:
	        return;
    }
	JobProg.Type  = SERIOFW_JOBPROG_PRINTPAGEEND;
//	JobProg.Param.PageEnd.dummy;				/* TODO : dummyのみ */

	SerioFwEventNotify(SERIOFW_EVID_JOBPROGRESS, &JobProg);
}



/*********************************************************************************************/
/**
* @par		(serio)JOB実行の開始
* @param	なし
* @retval	なし
* @par	<外部仕様>
*			JOB実行の開始設定をする
* @par	<内部仕様>
* 			ジョブ実行中フラグをONにする
*/
/*********************************************************************************************/
GLOBAL	VOID 
IoJobStart(VOID)
{
	JobRunning = TRUE;
}


/*********************************************************************************************/
/**
* @par		(serio)JOB実行の終了
* @param	なし
* @retval	なし
* @par	<外部仕様>
*			JOB実行の終了設定をする
* @par	<内部仕様>
* 			ジョブ実行中フラグをOFFにする
*/
/*********************************************************************************************/
GLOBAL	VOID 
IoJobEnd(VOID)
{
	JobRunning = FALSE;
}

/*********************************************************************************************/
/**
* @par		(serio)JOB実行中かのチェック
* @param	なし
* @retval	なし
* @par	<外部仕様>
*			JOB実行中かのチェックを行う
* @par	<内部仕様>
* 			ジョブ実行中フラグを返す
*/
/*********************************************************************************************/
GLOBAL	BOOL 
IsIoJobRunning(VOID)
{
	return JobRunning;
}


/*********************************************************************************************/
/**
* @par		(serio)装置状態エラー発生中かのチェック
* @param	なし
* @retval	なし
* @par	<外部仕様>
*			装置状態エラー発生中かのチェックを行う
* @par	<内部仕様>
* 			装置状態エラーフラグを返す
*/
/*********************************************************************************************/
GLOBAL BOOL
IsStateError(VOID)
{
	return StateError;
}

/*********************************************************************************************/
/**
* @par		(serio)装置状態エラーフラグを設定する
* @param	Error : 装置状態エラーフラグ(TRUE:装置状態エラー発生、FALSE:装置状態エラーなし)
* @retval	なし
* @par	<外部仕様>
*			装置状態エラーフラグを設定する
* @par	<内部仕様>
* 			装置状態エラーフラグを設定する
*/
/*********************************************************************************************/
GLOBAL VOID
SetStateError(BOOL Error)
{
	StateError = Error;
}

#ifdef	USE_SCAN
/*********************************************************************************************/
/**
* @par		ケーブルの接続の状態をチェックする
* @param	なし
* @retval	TRUE	接続あり
* @retval	FASE	接続なし
* @par	<外部仕様>
* 			USB、FTP、EMS、FTPの接続状態をチェックする
* @par	<内部仕様>
* 			USB、FTP、EMS、FTPの接続状態をチェックする
*/
/*********************************************************************************************/
GLOBAL	BOOL
check_scanmode_cable_serio(VOID)
{
	/* uiframeのUifNline_com.hとpanelTask.hを同時に使用すると、 */
	/* KEY_AVAILABLEがすでに定義されている警告が発生するため    */
	/* 警告を回避するために panelTask.hの関数を呼ぶだけの処理   */
	return check_scanmode_cable();
}
#endif	/* USE_SCAN */

#ifdef USE_SERIO_LOG
/*********************************************************************************************/
/**
* @par		BSI制限UserによるIoJobの利用結果をLogに保存する
* @param	serio_job(in)  ：IoJob種別情報
* @param	Reason(in)     ：利用結果情報
* @retval	なし
* @par	<外部仕様>
* 			BSI制限UserによるIoJobの利用結果をLogに保存する
* @par	<内部仕様>
* 			BSI制限UserによるIoJobの利用結果をE2PROMに保存する
*/
/*********************************************************************************************/
GLOBAL	VOID	
set_iojob_end_bsilog(  serio_job_category_t serio_job, INT32 Reason )
{
	UINT8 err_reason = LOG_DATA_RESULT_UNKNOWN_ERROR;
	
	MPRINTF(("[%s]ErrorInfo=%d\n", __FUNCTION__, Reason));
	
	switch(Reason){
		case SERIOFW_JOBSTS_END_COMPLETE:
			err_reason = LOG_DATA_RESULT_OK;
			break;
		case SERIOFW_JOBSTS_END_SYSBUSY:
			err_reason = LOG_DATA_RESULT_ERROR;
			break;
		case SERIOFW_JOBSTS_END_CANCEL:
			err_reason = LOG_DATA_RESULT_CANCEL;
			break;
		default:
			break;
	}
	
	if(serio_job == SERIO_JOB_SCANSEND)
	{
		/* Scanの実行終了時はJob終了のLogを残す */
		SerioLog_Rec_End(LOG_DATA_FUNC_SCAN, err_reason);
	}
	
}
#endif	/* USE_SERIO_LOG */

/**
**************************************************************************
* @par (GLOBAL) Serioで共有するCpTaskへのメッセージ通知
* @param  メッセージFrom Task
*         メッセージコマンドID
* @return 
* @retval 
* @par <外部仕様>
*      
* @par <内部仕様>
*      
**************************************************************************
*/
GLOBAL VOID 
Serio_SendMsgToCp(UINT16 from_task, UINT16 cmd_id)
{
	UINT8					*send_buff;
	TASK_MSG_COM_T			com_buff;
	INT32					size;
	INT32					cp_qid;
	
	DPRINTF(("SerioLib:Serio_SendMsgToCp() CALLED.\n"));
	
	/* 通知内容の設定 */
	com_buff.cmd_id	= cmd_id;
	com_buff.from_task= from_task;
	send_buff = (UINT8 *)&com_buff;
	/* 通知サイズの設定 */
	size = sizeof(TASK_MSG_COM_T);
	
	/* CpTaskのメッセージキューID取得 */
	cp_qid = FOS_MSGGETID(CP_MSG_NAME);
	
	/* Taskへメッセージ送信 */
	FOS_MSGSEND(cp_qid, send_buff, size);
	
	return;
}

#ifdef	USE_CBSI
/*********************************************************************************************/
/**
* @par		(CloudBSI)CloudBSI情報設定状況確認
* @param	なし
* @retval	CloudBSIInfoFlag：CloudBSI設定状況
* @par	<外部仕様>
*			CloudBSIの情報がServerから全て設定されたかを確認する。
* @par	<内部仕様>
* 			CloudBSIの情報がServerから全て設定されたかを確認する。
*/
/*********************************************************************************************/
GLOBAL BOOL Is_CloudBSIInfoGet( VOID )
{
	return CloudBSIInfoFlag;
}

/*********************************************************************************************/
/**
* @par		(CloudBSI)CloudBSI情報設定状況セット
* @param	Status(input) ジョブステータス
* @retval	なし
* @par	<外部仕様>
*			CloudBSIの情報が設定中か設定済かの状況をフラグにセットする。
* @par	<内部仕様>
* 			取得済みを設定された場合、Panelに対して取得済みを通知する。
*/
/*********************************************************************************************/
GLOBAL VOID Set_CloudBSIInfoFlag( BOOL flag )
{
	CloudBSIInfoFlag = flag;
	
#ifdef	USE_SEPARATE_UI
	if(CloudBSIInfoFlag == TRUE)
	{
		Serio_SendMsgToCp( SERIO_LIBRARY, CPAPI_CBSI_INFO_GET_STATUS_FIN );
	}
#endif	/* USE_SEPARATE_UI */
	
}

#endif	/* USE_CBSI */

#endif /* USE_SERIO */
