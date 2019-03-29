/************************************************************************
 *
 * serio_connector_sendscan.c - IO JOB :: SCANコマンドコネクター
 *
 *	Copyright: 2010-20XX brother Industries , Ltd.
 *
 *	$Id: //depot/Firm/Commonfile/Laser_origin2/task/serio/serio_connector_sendscan.c#4 $
 *	$DateTime: 2011/04/01 13:10:09 $
 *	$Change: 210439 $
 *	$Author: muroika $
 *
 *	ver 1.0.0 : 2010.07.07 : ABS  : 新規作成
 ************************************************************************/
#if 1		/* 旧式のScanコネクタ BILStAr化が正式に対応されたらelse側の実装へ移行する */
#include "spec.h"
#include "stdtype.h"
#include "fos.h"
#include "debug.h"
#include "common.h"
#include "paradef.h"

#include "componentlib/keylib/aplid.h"
#include "lib/displib/displibapl.h"
#include "componentlib/keylib/keylib.h"
#include "serio_connector.h"
#include "serio_task_common.h"
#include "serio_seriomfp_app.h"
#include "serio_uicon_reqres_sub.h"
#include "serio_connector_debug.h"

#include "componentlib/hakkolib/hakkolib.h"
#include "subos/imagelib/image_lib.h"
#include "subos/sysmem/sysmemLib.h"
#include "componentlib/hakkolib/telcntl.h"
#include "componentlib/hakkolib/job.h"
#include "lib/funcset/funcsetcntl.h"
#include "lib/funcset/funcset.h"
#ifdef	USE_SEPARATE_UI
#include "lib/cplib/cp_api_lib.h"
#include "lib/cplib/cp_para_scan.h"
#include "lib/cplib/cp_sts_apl_sta.h"
#endif	/* USE_SEPARATE_UI */
#include "componentlib/state/statecntl.h"
#include "lib/cparam/cparam.h"
#include "lib/cparam/cspecEntry.h"
#include "scanning/scan/scanTask.h"
#include "lib/scanning_sub/doc_scan_area_spec.h"
#include "componentlib/resource/resource.h"
#include "task/ftpclient/ftpclient.h"
#include "task/mem_read/memReadTask.h"
#include "task/panel/soundlib.h"

#ifdef	USE_SCAN_AUTO_RESOLUTION
#include "task/pc_scanner/pcScanUif.h"
#include "modeltable/sysmemtable/sysmemModel.h"
#endif	/* USE_SCAN_AUTO_RESOLUTION */

#if (defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)) && defined(USE_SCAN)

#undef SEND_SCAN_DEBUG_PRINT

#define	EXTENSION_SIZE				3		/* 拡張子の文字数 */

enum
{
	SIMPLEX,
	DUPLEXLONG,
	DUPLEXSHORT
};

enum
{
	JOB_SCAN_TYPE_NONE,
	JOB_SCAN_TYPE_FTP,					/* ScanモードからのScan to FTP 		*/
	JOB_SCAN_TYPE_NETWORK,				/* ScanモードからのScan to Network 	*/
	JOB_SCAN_TYPE_EMAIL					/* ScanモードからのScan to E-Mail 	*/
};

#ifdef	USE_SEPARATE_UI
/* Scan2FTP/Scan2Network用の設定管理 */
typedef struct{
	UINT8	ftp_resolution;
	INT32	color_type;
	INT32	reso_type;
}scan_resolution_t;

STATIC const scan_resolution_t ScanResolution[] = {
	{FTP_BW100,		CPAPI_P_VAL_SCAN_COLOR_TYPE_BW,			CPAPI_P_VAL_SCAN_RESO_200X100},
	{FTP_BW200,		CPAPI_P_VAL_SCAN_COLOR_TYPE_BW,			CPAPI_P_VAL_SCAN_RESO_200    },
	{FTP_BW300,		CPAPI_P_VAL_SCAN_COLOR_TYPE_BW,			CPAPI_P_VAL_SCAN_RESO_300    },
	{FTP_Gray100,	CPAPI_P_VAL_SCAN_COLOR_TYPE_GRAY,		CPAPI_P_VAL_SCAN_RESO_100    },
	{FTP_Gray200,	CPAPI_P_VAL_SCAN_COLOR_TYPE_GRAY,		CPAPI_P_VAL_SCAN_RESO_200    },
	{FTP_Gray300,	CPAPI_P_VAL_SCAN_COLOR_TYPE_GRAY,		CPAPI_P_VAL_SCAN_RESO_300    },
#ifdef	USE_SCAN_AUTO_RESOLUTION
	{FTP_GrayAuto,	CPAPI_P_VAL_SCAN_COLOR_TYPE_GRAY,		CPAPI_P_VAL_SCAN_RESO_AUTO   },
#endif	/* USE_SCAN_AUTO_RESOLUTION */
	{FTP_Color100,	CPAPI_P_VAL_SCAN_COLOR_TYPE_COLOR,		CPAPI_P_VAL_SCAN_RESO_100    },
	{FTP_Color200,	CPAPI_P_VAL_SCAN_COLOR_TYPE_COLOR,		CPAPI_P_VAL_SCAN_RESO_200    },
	{FTP_Color300,	CPAPI_P_VAL_SCAN_COLOR_TYPE_COLOR,		CPAPI_P_VAL_SCAN_RESO_300    },
	{FTP_Color600,	CPAPI_P_VAL_SCAN_COLOR_TYPE_COLOR,		CPAPI_P_VAL_SCAN_RESO_600    },
#ifdef	USE_SCAN_AUTO_RESOLUTION
	{FTP_ColorAuto,	CPAPI_P_VAL_SCAN_COLOR_TYPE_COLOR,		CPAPI_P_VAL_SCAN_RESO_AUTO   },
#endif	/* USE_SCAN_AUTO_RESOLUTION */
};
#endif	/* USE_SEPARATE_UI */


STATIC INT32	ScanSendEMail(VOID *Param);
STATIC INT32	ScanSendFTP(VOID *Param);
STATIC INT32	ScanSendNetwork(VOID *Param);
STATIC 	void 	get_current_set(current_t *state);
STATIC 	UINT8 	get_scan_quality(enum ns__Selection3 quality);
#ifdef	USE_TMP_SCANSIZE
STATIC	UINT8	get_scan_doc_size(enum ns__ScanAndUloadScansize scan_doc_size);
#endif	/* USE_TMP_SCANSIZE */
STATIC	VOID 	get_filename(UINT8 *Filename, size_t size, UINT8 *InputFilename, SCAN2FTP_FILEFORMAT fileformat, UINT8 FileNameFixed);
STATIC	VOID 	get_extension_str(UINT8 *extension, SCAN2FTP_FILEFORMAT fileformat);
STATIC	SCAN2FTP_QUALITY	GetQuality(enum ns__ColorModeSelection ColorMode, enum ns__UloadResolution Resolution);
STATIC	SCAN2FTP_FILEFORMAT	GetFileformat(enum ns__FileFormatSelection *FileType);
STATIC  BOOL	IsColor(SCAN2FTP_QUALITY quality);
STATIC	BOOL	IsGray(SCAN2FTP_QUALITY quality);
STATIC	BOOL	IsMono(SCAN2FTP_QUALITY quality);
STATIC	BOOL 	serio_IsDischargeTray(VOID);
STATIC  INT8	GetJobScanType(VOID);
STATIC	INT32	GetTxProfileType(SERIO_CNP_IOJOB_SCANSEND_T *iojob);
STATIC  UINT16 getNumProfiles(SERIO_CNP_IOJOB_SCANSEND_T *iojob);
STATIC	INT32	GetsysmApliRsv(current_t *pCurrent);
STATIC	UINT8	GetFileformat_EMail(enum ns__FileFormatSelection FileType);
STATIC	UINT8	GetResolution_EMail(enum ns__ColorModeSelection ColorMode, enum ns__UloadResolution Resolution);
#ifdef	USE_TMP_SCANSIZE
STATIC	UINT8	GetScanSize_EMail(SERIO_CNP_IOJOB_SCANSEND_T *iojob);
#endif	/* USE_TMP_SCANSIZE */
STATIC	UINT8	GetCompRate_EMail(SERIO_CNP_IOJOB_SCANSEND_T *iojob);
STATIC	UINT8 	GetColorFlag(enum ns__ColorModeSelection ColorMode);
STATIC	UINT8 	GetGrayScale(enum ns__ColorModeSelection ColorMode);

STATIC 	INT32 	check_broad(VOID);
STATIC	VOID	DeleteEntry(INT32 entry_id);

STATIC MD_CHAR* strncpy_safe(MD_CHAR*, MD_CHAR*, UINT);

#ifdef SEND_SCAN_DEBUG_PRINT
STATIC 	VOID 	debug_print_email(current_t *current, INT32 rsv_id);
STATIC 	VOID 	debug_print_ftp(ACCESS_INFO *access_info);
STATIC 	VOID 	debug_print_network(CIFSACCESS_INFO *access_info);
#endif

#ifdef ENABLE_VIRTUAL_MFC /** ENABLE_VIRTUAL_MFC **/
STATIC INT32 Initialize__EvJobFinishT(SERIO_EV_JOB_FINISH_T* jobdone);
#endif /** ENABLE_VIRTUAL_MFC **/

#ifdef	DOUBLE_DEVICE_DUPLEX_SCAN
STATIC	INT32	GetsysmApliRsv_Color(current_t *pCurrent);
STATIC	INT32	GetsysmApliRsv_Gray(current_t *pCurrent);
STATIC	INT32	GetsysmApliRsv_Mono(current_t *pCurrent);
#endif	/* DOUBLE_DEVICE_DUPLEX_SCAN */

#ifdef	USE_SEPARATE_UI
STATIC VOID SetIoJobScanType_toCP( UINT8 ScanType );
STATIC VOID SetTmpdataIoJobCifsAccessinfo(CIFSACCESS_INFO *AccessInfo);
STATIC VOID SetTmpdataIoJobFtpAccessinfo(ACCESS_INFO *AccessInfo);
STATIC VOID SetTmpdataIoJobFtpCifsDuplexSetting( SCAN2FTP_DUALSCAN duplex );
STATIC VOID SetTmpdataIoJobFtpCifsResolutionSetting( SCAN2FTP_QUALITY reso );
STATIC VOID SetTmpdataIoJobFtpCifsFileType( SCAN2FTP_FILEFORMAT FileType );
STATIC VOID SetTmpdataIoJobFtpCifsFileName( UINT8* filename );
STATIC VOID SetTmpdataIoJobEmsResolutionSetting(enum ns__ColorModeSelection ColorMode, UINT8 reso);
#endif	/* USE_SEPARATE_UI */

CSubject_t *g_ScanSubject = NULL;

#ifdef	USE_SEPARATE_UI
STATIC	UINT8	svScanType;
#endif	/* USE_SEPARATE_UI */

/* パラメータ設定関連フラグ */
STATIC BOOL StrictParamFlag;
STATIC BOOL ParamChkOnlyFlag;
STATIC BOOL ScanParamErrFlag;

/* 通常のスキャンでの設定値 */
STATIC	CIFSACCESS_INFO		CifsAccessInfo_Scan;		/* Scan to Network */
STATIC	ACCESS_INFO			FtpAccessInfo_Scan;			/* Scan to FTP */
#ifdef USE_FAX /* DCLの場合、未使用 */
STATIC	UINT32				TelId_Scan;					/* Scan to E-Mail 発呼ID*/
#endif /* USE_FAX */
STATIC	current_t			Current_Scan;				/* Scan to E-Mail Scan設定値*/
STATIC  INT8				JobScanType = JOB_SCAN_TYPE_NONE;
STATIC	INT32				s_job_id = ERROR;			/* Scan to E-Mail Job番号 */
STATIC	INT32				s_rsv_id = ERROR;

extern INT32 getCloudBSIAppFlag(void);
/*********************************************************************************************/
/**
* @par		(serio)Job初期化処理
* @param	Param(input) Jobパラメータ
* @retval	OK	: 成功
* 			NG	: 失敗
* @par	<外部仕様>
*			Job初期化処理を行う
* @par	<内部仕様>
* 			使用するSubjectの設定をする
*/
/*********************************************************************************************/
GLOBAL INT32
IOJobConnector_SendScan_Init(VOID *Param)
{
	INT32 UnionType;
	
	if(Param == NULL)
	{
		EPRINTF(("[%s]Param == NULL\n", __FUNCTION__));
		return ERROR;
	}

	if(TRUE == getCloudBSIAppFlag())
	{
		SendJobStatus_End(SERIOFW_JOBSTS_END_COMPLETE, SERIO_EXTERR_COMPLETE, SERIO_JOB_SCANSEND);
		return ERROR;
	}

	UnionType = GetTxProfileType((SERIO_CNP_IOJOB_SCANSEND_T *)Param);
	if(UnionType == ERROR){
		return ERROR;
	}

	
	/* NextPage通知のObserver登録 */
	if(UnionType == SOAP_UNION_ns__TxProfile_Smtp){
		if(g_SubjectMem){
			g_ScanSubject = g_SubjectMem;
		}
	}else{
		if(g_SubjectFtp){
			g_ScanSubject = g_SubjectFtp;
		}
	}

	/* ここにStrictParamとParamCheckOnlyの状態を確認する処理を追加する。 */
	/* その２つのステータスの保持は、STATICなフラグをそれぞれ用意する。  */
	/* そのステータスが無い場合は、丸め優先、実行優先で良い。            */
	StrictParamFlag  = FALSE;
	ParamChkOnlyFlag = FALSE;
	ScanParamErrFlag = FALSE;

	return OK;
}


/*********************************************************************************************/
/**
* @par		(serio)Job実行処理
* @param	Param(input) Jobパラメータ
* @retval	OK	: 成功
* 			NG	: 失敗
* @par	<外部仕様>
*			Job実行処理を行う
* @par	<内部仕様>
* 			Scan to E-Mail/FTP/NetworkのJOB実行を行う
*/
/*********************************************************************************************/
GLOBAL INT32
IOJobConnector_SendScan_Exec(VOID *Param)
{
#if 0 /** (2011-01-19T16:54:41JST kazushige.muroi) **/
/*-------------------------------------------------------------
 * New
 *------------------------------------------------------------*/
	INT32 retval = ERROR;
	INT32 *retvals = NULL;
	UINT16 numProfiles;
	UINT16 i;
	
	numProfiles = getNumProfiles((SERIO_CNP_IOJOB_SCANSEND_T *)Param);
	if (numProfiles < 1){
		return retval;
	}
	retvals = (INT32*)SerioMalloc(sizeof(INT32) * numProfiles);
	
	for (i=0; i<numProfiles; ++i){
		switch(UnionType){
			case SOAP_UNION_ns__TxProfile_Smtp:
				retval = ScanSendEMail(Param);
				break;
			case SOAP_UNION_ns__TxProfile_Cifs:
				retval = ScanSendNetwork(Param);
				break;
			case SOAP_UNION_ns__TxProfile_Ftp:
				retval = ScanSendFTP(Param);
				break;
		}
		retvals[i] = retval;
	}

	
	SerioMfree(retvals);
	return retval;
#else  /** (2011-01-19T16:54:41JST kazushige.muroi) **/
/*-------------------------------------------------------------
 * Original
 *------------------------------------------------------------*/
    INT32 retval = ERROR;
 	INT32 UnionType;
	
	if(Param == NULL)
	{
		EPRINTF(("[%s]Param == NULL\n", __FUNCTION__));
		return ERROR;
	}

	UnionType = GetTxProfileType((SERIO_CNP_IOJOB_SCANSEND_T *)Param);
	
	/* StrictParamの設定を取得する。*/
	if(((SERIO_CNP_IOJOB_SCANSEND_T *)Param)->strictParam != NULL)
	{
		StrictParamFlag = *((SERIO_CNP_IOJOB_SCANSEND_T *)Param)->strictParam;
	}
	
	/* ParamChkOnlyの設定を取得する。*/
	if(((SERIO_CNP_IOJOB_SCANSEND_T *)Param)->paramCheckOnly != NULL)
	{
		ParamChkOnlyFlag = *((SERIO_CNP_IOJOB_SCANSEND_T *)Param)->paramCheckOnly;
	}
	
	switch(UnionType){
		case SOAP_UNION_ns__TxProfile_Smtp:
#ifdef	USE_SEPARATE_UI
			svScanType = CPAPI_SCAN_EMAIL_SERVER;
			SetIoJobScanType_toCP( CPAPI_SCAN_EMAIL_SERVER );
#endif	/* USE_SEPARATE_UI */
			retval = ScanSendEMail(Param);
			break;
  		case SOAP_UNION_ns__TxProfile_Cifs:
#ifdef	USE_SEPARATE_UI
			svScanType = CPAPI_SCAN_NETWORK;
			SetIoJobScanType_toCP( CPAPI_SCAN_NETWORK );
#endif	/* USE_SEPARATE_UI */
			retval = ScanSendNetwork(Param);
			break;
		case SOAP_UNION_ns__TxProfile_Ftp:
#ifdef	USE_SEPARATE_UI
			svScanType = CPAPI_SCAN_FTP;
			SetIoJobScanType_toCP( CPAPI_SCAN_FTP );
#endif	/* USE_SEPARATE_UI */
			retval = ScanSendFTP(Param);
			break;
	}
	
	return retval;
#endif /** (2011-01-19T16:54:41JST kazushige.muroi) **/

}


/*********************************************************************************************/
/**
* @par		(serio)Scanの再開処理
* @param	Param(input) Restartパラメータ
* @retval	OK	: 成功
* 			NG	: 失敗
* @par	<外部仕様>
*			Scanの再開処理を行う
* @par	<内部仕様>
* 			Subjectに登録されていれるObserverに再開の通知を行う
*/
/*********************************************************************************************/
GLOBAL INT32
IOJobConnector_SendScan_Restart(SERIO_CN_RESTART_PARAM_T *Param)
{
	if(Param == NULL)
	{
		EPRINTF(("[%s]Param == NULL\n", __FUNCTION__));
		return ERROR;
	}

	if(Param->Reason == SERIOFW_JOBSTS_PAUSEDBY_SCANNEXT){
		g_ScanSubject->vptr_Notify(g_ScanSubject, (VOID *)Param);
	}else{
		if(get_nowkeypanel() != APL_SERIO){
			keygetpanel(APL_SERIO, LINE_TYPE0);
		}
	}
	return OK;
}


/*********************************************************************************************/
/**
* @par		(serio)Scanのキャンセル処理
* @param	Param(input) Restartパラメータ
* @retval	OK	: 成功
* 			NG	: 失敗
* @par	<外部仕様>
*			Scanのキャンセル処理を行う<お任せ表示の際には通らない>
* @par	<内部仕様>
* 			
*/
/*********************************************************************************************/
GLOBAL INT32
IOJobConnector_SendScan_Cancel(VOID *Param)
{
#ifndef	USE_SEPARATE_UI
	if(IsIoJobRunning() == TRUE){
		/* JOB実行中なら通知を行う */
		g_SubjectJobStop->vptr_Notify(g_SubjectJobStop, NULL);
		return OK;
	}
#else	/* USE_SEPARATE_UI */
	UINT32			qid_ftpclient;
	ftpclt_msg		msg_param;
	INT32			mem_qid;
	memread_msg_t	mem_msg;
	
	switch(svScanType){
		case CPAPI_SCAN_EMAIL_SERVER:
			DPRINTF(("SCAN STOP by Scan2E-Mail Server\n"));
			mem_qid                   = FOS_MSGGETID(MEM_READ_MSG_NAME);
			mem_msg.com_msg.cmd_id    = MEMR_CMD_STOP;
			mem_msg.com_msg.from_task = PANEL_BASE_TASK;
			FOS_MSGSEND( mem_qid, (UINT8*)&mem_msg, sizeof(memread_msg_t) );
			break;
  		case CPAPI_SCAN_NETWORK:
		case CPAPI_SCAN_FTP:
			DPRINTF(("SCAN STOP by Scan2FTP or Scan2Network\n"));
			qid_ftpclient       = FOS_MSGGETID( FTPC_MSG_NAME );
			msg_param.cmd_id    = (UINT16)CMD_SCANSTOP;
			msg_param.from_task = (UINT16)PANEL_BASE_TASK;
			FOS_MSGSEND( qid_ftpclient, (UINT8 *)&msg_param, sizeof(ftpclt_msg) );
			break;
		default:
			DPRINTF(("SCAN STOP by Unknown ScanType\n"));
			break;
	}

#endif	/* USE_SEPARATE_UI */

	return OK;
}


/*********************************************************************************************/
/**
* @par		(serio)Scanの終了処理
* @param	Param(input) Restartパラメータ
* @retval	OK	: 成功
* 			NG	: 失敗
* @par	<外部仕様>
*			Scanの終了処理を行う
* @par	<内部仕様>
* 			
*/
/*********************************************************************************************/
GLOBAL INT32
IOJobConnector_SendScan_Exit(VOID *Param)
{
	return OK;
}

/**
@fn STATIC MD_CHAR* strncpy_safe(MD_CHAR* str_to, MD_CHAR* str_from, UINT size)
@brief 簡易データチェックを設けた、安全なstrncpy
@param[in] str_to:  
@param[in] str_from:  
@param[in] size: size
# (yas/expand-link "param")
@return (MD_CHAR*)str_to
@author muroika
*/
STATIC MD_CHAR* strncpy_safe(MD_CHAR* str_to, MD_CHAR* str_from, UINT size)
{
    if (str_from){
        strncpy(str_to, str_from, size);
    }
    else{
        strcpy(str_to, "");
    }
    return str_to;
}


/*********************************************************************************************/
/**
* @par		(serio)Scan to E-Mailを行う
* @param	Param(input) JOBパラメータ
* @retval	OK	: 成功
* 			NG	: 失敗
* @par	<外部仕様>
*			Scan to E-Mailを行う
* @par	<内部仕様>
* 			Temporaryの設定を行い、Scan To E-Mailを開始する
*/
/*********************************************************************************************/
INT32	ScanSendEMail(VOID *Param)
{
	SERIO_CNP_IOJOB_SCANSEND_T *iojob;
	struct 	ns__SmtpProperty 	*pSmtp;
	INT32			entry_id;		/* 発呼登録ID */
	current_t		current;
	INT32			rsv_id;
	INT32			img_id;
	INT32			job_id;
	UINT32			mem_qid;
#ifdef USE_FAX /* DCLの場合未使用 */
	INT16 			type;
	INT32			telid;
#endif /* USE_FAX */
	INT32			i;
	enum ns__ColorModeSelection ColorMode;
	/* enum ns__Selection3 		Resolution; */
	enum ns__UloadResolution 		Resolution;
	UINT8			uJobScanType;
	BOOL			EntryError;

#ifdef USE_FAX
	type = JOB_TX;
#endif /* USE_FAX */

	mem_qid = FOS_MSGGETID( MEM_READ_MSG_NAME );

#ifdef	USE_INTERNET_FAX
	if (fstget_data(FSW_NET_SCAN2EMAILSVR_FUNC) != FSW_NET_SCAN2EMAILSVR_FUNC_ON) {
		SendJobStatus_End(SERIOFW_JOBSTS_END_DEVERR, SERIO_EXTERR_INABILITY, SERIO_JOB_SCANSEND);
		buzzer_refusal();
		return OK;
	}
#endif	/* USE_INTERNET_FAX */

	if (resourceforceget(RES_SCAN_VIDEO, mem_qid) == RES_NG) {
		SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
		return OK;
	}

	if (ADFresReserve(FOR_FAX, ADF_RSERVE_REQ) == RESERVE_NG) {
		resourcefree(RES_SCAN_VIDEO);
		SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
		return OK;
	}

	iojob = (SERIO_CNP_IOJOB_SCANSEND_T *)Param;
#if 0 /**  (2010-08-06T11:50:25+09:00 kazushige.muroi) **/
	/*------------------------------------------------------------------------
	 * old
	 *----------------------------------------------------------------------*/
	if(iojob->TxProfiles != NULL){
		pSmtp = &iojob->TxProfiles->__TxProfiles->TxProfile.Smtp;
	}else{
		pSmtp = NULL;
	}
#else  /**  (2010-08-06T11:50:25+09:00 kazushige.muroi) **/
	pSmtp = &iojob->TxProfiles.__TxProfiles[0].TxProfile.Smtp;
#endif /**  (2010-08-06T11:50:25+09:00 kazushige.muroi) **/

	/* テーブルを初期化する */
	memset(&current,  0x00, sizeof(current));
	
	uJobScanType = GetJobScanType();
	if(uJobScanType == JOB_SCAN_TYPE_EMAIL){
		/* ScanモードからScan to E-Mailを行ったとき */
		memcpy(&current,  &Current_Scan, sizeof(current));
	}else if(uJobScanType == JOB_SCAN_TYPE_NONE){
		/* Serioアプリ実行でE-MailのJOB実行通知が来たとき */

		if(pSmtp == NULL){
			/* アプリ実行でSmtpの設定が存在しないときはエラー */
			SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
			return OK;
		}
		current.serio_job = SERIO_JOB_SCANSEND;
		get_current_set(&current);
#ifdef	FB_SCAN_TYPE
		/* ADFに原稿があればADFを使用する */
		if ( MacStateRef(STID_MAN_SOURCE_IN) == MAC_ON ) {
			current.scan_src = ADF_SCAN;
		}else{
			current.scan_src = FB_SCAN;
		}
#endif	/* FB_SCAN_TYPE */
		ColorMode = ns__ColorModeSelection__Color;
		current.color_flag = GetColorFlag(ColorMode);
		current.gray_scale = GetGrayScale(ColorMode);
		Resolution = ns__UloadResolution__Normal;
		current.resolution[0] = GetResolution_EMail(ColorMode, Resolution);
		current.file_format = GetFileformat_EMail(ns__FileFormatSelection__PDF);
		current.gndcolor_removal = SCAN_GCOL_REMOVAL_OFF;
		current.gndcolor_level = SCAN_GCOL_LEVEL_0;
		if((iojob->DuplexScanEnable == NULL) || (iojob->ShortEdgeBinding == NULL)){
			fstset_data( FUNC_TMP_DUALSCANSETTING, FUNC_DUALSCAN_SIMPLEX );
		}
	}else{
		/* Email以外のスキャンを行ったのに、EmailのJOB実行通知が来たときはエラー */
		resourcefree(RES_SCAN_VIDEO);
		ADFresReserve(FOR_FAX, ADF_RSERVE_CANCEL);
		SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
		return OK;
	}


	if(pSmtp != NULL){
		/* アプリ起動でのScanの設定 */
		if ((entry_id = telEntryOpen(TELCTR_MAIL)) < 0) {
			resourcefree(RES_SCAN_VIDEO);
			ADFresReserve(FOR_FAX, ADF_RSERVE_CANCEL);
			SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
			return OK;
		}

		EntryError = FALSE;
		for(i = 0; i < pSmtp->__sizeDestination; i++){
			if (telEntryReserve(TEL_TYPE_MAIL, entry_id) < 0) {
				EntryError = TRUE;
				break;
			}

#ifdef USE_FAX /* telEntryReserve()がUSE_FAX無効の場合、必ずErrorとなりunreachableとなるため */
			/* 同報キー入力をチェックする */
			if(check_broad() != OK){
				EntryError = TRUE;
				break;
			}

#ifdef	USE_DIALICV_CHECK
			telEntryTenkey(entry_id, pSmtp->Destination[i], NULL);
#else
			telEntryTenkey(entry_id, pSmtp->Destination[i]);
#endif
			telEntryIndex(BROAD, entry_id);
#endif /* USE_FAX */
		}

		if(EntryError == TRUE){
			resourcefree(RES_SCAN_VIDEO);
			ADFresReserve(FOR_FAX, ADF_RSERVE_CANCEL);
			DeleteEntry(entry_id);
			SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
			return OK;
		}
	}


	if ((img_id = ImageEntry(IMAGE_FILE, IMAGE_USE_SND_FAX)) == ERROR) {
		resourcefree(RES_SCAN_VIDEO);
		ADFresReserve(FOR_FAX, ADF_RSERVE_CANCEL);
		DeleteEntry(entry_id);
		SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
		return OK;
	}

	if((iojob->ColorMode != NULL) || (iojob->Resolution != NULL)){
		if(iojob->ColorMode == NULL){
			ColorMode = ns__ColorModeSelection__Color;
		}else{
			ColorMode = *iojob->ColorMode;
		}
		if(iojob->Resolution == NULL){
			Resolution = ns__UloadResolution__Normal;
		}else{
			Resolution = *iojob->Resolution;
		}
		current.resolution[0] = GetResolution_EMail(ColorMode, Resolution);
		current.color_flag = GetColorFlag(ColorMode);
		current.gray_scale = GetGrayScale(ColorMode);
	}


	if(iojob->FileType != NULL){
		current.file_format = GetFileformat_EMail(*iojob->FileType);
	}

	/* 禁則処理 */
	if((ColorMode == ns__ColorModeSelection__Mono) && ((current.file_format == FILE_FORMAT_JPEG) || (current.file_format == FILE_FORMAT_XPS) )){
		if((StrictParamFlag == TRUE) || (ParamChkOnlyFlag == TRUE))
		{
			/* パラメータエラー */
			ScanParamErrFlag = TRUE;
		}
		else
		{
			/* Gray設定に丸める */
			ColorMode          = ns__ColorModeSelection__Gray;
			current.color_flag = COLORFAX_OFF;
			current.gray_scale = USW_SCAN_MULTI;
			current.resolution[0] = GetResolution_EMail(ColorMode, Resolution);
		}
	}
	if((current.file_format == FILE_FORMAT_TIFF) && ((ColorMode == ns__ColorModeSelection__Gray) || (ColorMode == ns__ColorModeSelection__Color))){
		if((StrictParamFlag == TRUE) || (ParamChkOnlyFlag == TRUE))
		{
			/* パラメータエラー */
			ScanParamErrFlag = TRUE;
		}
		else
		{
			/* Mono設定に丸める */
			ColorMode          = ns__ColorModeSelection__Mono;
			current.color_flag = COLORFAX_OFF;
			current.gray_scale = USW_SCAN_2LEVEL;
			current.resolution[0] = GetResolution_EMail(ColorMode, Resolution);
		}
	}


#ifdef		USE_DUPLEX_SCAN
	if( fstget_data(FSW_DUPLEX_SCAN_FUNC ) == FSW_DUPLEX_SCAN_FUNC_ON )
	{
		if((iojob->DuplexScanEnable != NULL) || (iojob->ShortEdgeBinding != NULL)){
			if((iojob->DuplexScanEnable == NULL) || (*iojob->DuplexScanEnable == xsd__boolean__false_)){
				fstset_data( FUNC_TMP_DUALSCANSETTING, FUNC_DUALSCAN_SIMPLEX );
			}else if(*iojob->DuplexScanEnable == xsd__boolean__true_){
				if((iojob->ShortEdgeBinding == NULL) || (*iojob->ShortEdgeBinding == xsd__boolean__false_)){
					fstset_data( FUNC_TMP_DUALSCANSETTING, FUNC_DUALSCAN_DXLONG );
				}else{
					fstset_data( FUNC_TMP_DUALSCANSETTING, FUNC_DUALSCAN_DXSHORT );
				}

			}
		}
	}else{
		fstset_data( FUNC_TMP_DUALSCANSETTING, FUNC_DUALSCAN_SIMPLEX );
	}
#else
	fstset_data( FUNC_TMP_DUALSCANSETTING, FUNC_DUALSCAN_SIMPLEX );
#endif	/* USE_DUPLEX_SCAN */
	
#ifdef	FB_SCAN_TYPE
	if(iojob->ScanTray != NULL){
		if(*iojob->ScanTray == ns__ScanTraySelection__ADF){
			current.scan_src = ADF_SCAN;
		}else{
			current.scan_src = FB_SCAN;
		}
	}
	/* 禁則処理 */
	if((MacStateRef(STID_MAN_SOURCE_IN) == MAC_OFF) || (current.scan_src == FB_SCAN)){
		/* ADFに原稿がない、またはFB読み取り指定のときは強制的に、FBでSIMPLEXに設定を変更する */
		current.scan_src = FB_SCAN;
		fstset_data( FUNC_TMP_DUALSCANSETTING, FUNC_DUALSCAN_SIMPLEX );
	}
#endif	/* FB_SCAN_TYPE */

#ifdef	USE_NETWORK_SCANNER
	/* ScanToEmailではネットワークスキャナーモードはONとする */
	current.net_scan = NET_SCANNER_ON;
#endif	/* USE_NETWORK_SCANNER */

#ifdef	USE_TMP_SCANSIZE
	/* IOJOBのDocSizeより、スキャンサイズを求める */
	current.scan_size = GetScanSize_EMail(iojob);
#endif	/* USE_TMP_SCANSIZE */
	/* IOJOBのJpgQualityより、圧縮率を求める */
	current.comp_rate = GetCompRate_EMail(iojob);

	/* システム領域にエントリーする前に、パラメータ厳密設定におけるパラメータ異常の場合は、処理を中断しSerioFWにエラーを通知する */
	if(ScanParamErrFlag == TRUE)
	{
		ImageDelete(img_id);
		resourcefree(RES_SCAN_VIDEO);
		ADFresReserve(FOR_FAX, ADF_RSERVE_CANCEL);
		DeleteEntry(entry_id);
		/* Scan中断通知をSerioFWに返す */
		SendJobStatus_End(SERIOFW_JOBSTS_END_CANCEL, SERIO_EXTERR_INABILITY, SERIO_JOB_SCANSEND);
		return OK;
	}

	rsv_id = GetsysmApliRsv(&current);
	if (rsv_id == ERROR){
		ImageDelete(img_id);
		resourcefree(RES_SCAN_VIDEO);
		ADFresReserve(FOR_FAX, ADF_RSERVE_CANCEL);
		DeleteEntry(entry_id);
		SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
		return OK;
	}

	ImageDelete(img_id);
	switch(current.file_format){
		case FILE_FORMAT_TIFF:
			img_id = ImageEntry(IMAGE_FILE, IMAGE_USE_READ_FAX);
			break;
		case FILE_FORMAT_JPEG:
			img_id = ImageEntry(IMAGE_FILE, IMAGE_USE_JPEG);
			break;
		case FILE_FORMAT_XPS:
			img_id = ImageEntry(IMAGE_FILE, IMAGE_USE_XPS);
			break;
		case FILE_FORMAT_PDF:
			img_id = ImageEntry(IMAGE_FILE, IMAGE_USE_PDF);
			break;
		case FILE_FORMAT_SPDF:
			img_id = ImageEntry(IMAGE_FILE, IMAGE_USE_SPDF);
			break;
		case FILE_FORMAT_PDFA:
			img_id = ImageEntry(IMAGE_FILE, IMAGE_USE_PDFA);
			break;
		case FILE_FORMAT_SIPDF:
			img_id = ImageEntry(IMAGE_FILE, IMAGE_USE_SIGNEDPDF);
			break;									
		default:
			img_id = ImageEntry(IMAGE_FILE, IMAGE_USE_PDF);
			break;
	}
	if (img_id == ERROR) {
		sysmApliFree(rsv_id);
		resourcefree(RES_SCAN_VIDEO);
		ADFresReserve(FOR_FAX, ADF_RSERVE_CANCEL);
		DeleteEntry(entry_id);
		SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
		return OK;
	}

#ifdef USE_FAX /* USE_FAXが無効の場合、telidにセットした値が使用されないため */
	if(pSmtp != NULL){
		telid = telEntryIndex(START, entry_id);
	}else{
		telid = TelId_Scan;
	}
#endif /* USE_FAX */
	
	if(ParamChkOnlyFlag != TRUE)
	{
#ifdef USE_FAX /* USE_FAXが無効の場合、telidにセットした値が使用されないため */
		if ((job_id = jobEntry(type, img_id, telid, JOBTIME_FOREVER, &current, NULL)) == ERROR) {
			resourcefree(RES_SCAN_VIDEO);
			ADFresReserve(FOR_FAX, ADF_RSERVE_CANCEL);
			ImageDelete( img_id );
			if (sysmApliFree(rsv_id) == ERROR) {
				EPRINTF(("sysmApliFree_k(%d) Error\n",rsv_id));
			}
			if(pSmtp != NULL){
				telDelDialNum( telid | LOWER16 );
			}
			SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
			return OK;
		}
#else
		/* USE_FAXが無効の場合、jobEntry()、telDelDialNum()はErrorを必ずErrorを返し */
		/* 処理を実行しないためtelidが使用されずDCPモデルではエラーとなる */
		/* 暫定でError処理のみ実行 */
		resourcefree(RES_SCAN_VIDEO);
		if (sysmApliFree(rsv_id) == ERROR) {
			EPRINTF(("sysmApliFree_k(%d) Error\n",rsv_id));
		}
		ADFresReserve(FOR_FAX, ADF_RSERVE_CANCEL);
		ImageDelete( img_id );
		SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
#endif /* USE_FAX */
		/* StaticのJob番号に、取得したJob番号を設定する */
		s_job_id = job_id;
		s_rsv_id = rsv_id;
		
#ifdef SEND_SCAN_DEBUG_PRINT
		debug_print_email(&current, rsv_id);
#endif

#ifdef	USE_SEPARATE_UI
#if 1	/* BILStArの状態によっては、else側を使う可能性があるため残しておく */
		/* CPタスクへSCANアプリ起動要求を出す */
		SendMsgToCp(CPAPI_MSGCMDID_SERIO_START_SCAN);
#else	/* if 1 */
		/* アプリ状態を登録 */
		cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_START);
		cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_EXEC);
#endif	/* if 1 */
#endif	/* USE_SEPARATE_UI */

#ifdef ENABLE_VIRTUAL_MFC
		{
			SERIO_EVP_JOBSTS_T  jobsts;
			jobsts.JobID = SERIOFW_IOJOB_SCANSEND;
			jobsts.Status = SERIOFW_JOBSTS_END;
			jobsts.Param.End.Reason = SERIOFW_JOBSTS_END_COMPLETE;
	        Initialize__EvJobFinishT(&jobsts.Param.End.Detail);
			SerioFwEventNotify(SERIOFW_EVID_JOBSTATUS, &jobsts);
		}
#endif
	}
	else
	{
		resourcefree(RES_SCAN_VIDEO);
		ADFresReserve(FOR_FAX, ADF_RSERVE_CANCEL);
		ImageDelete( img_id );
		if (sysmApliFree(rsv_id) == ERROR) {
			EPRINTF(("sysmApliFree_k(%d) Error\n",rsv_id));
		}
		if(pSmtp != NULL){
			telDelDialNum( telid | LOWER16 );
		}
		/* Paramチェックのみの場合は、ここでIO Job完了通知をする */
		SendJobStatus_End( SERIOFW_JOBSTS_END_COMPLETE, SERIO_EXTERR_COMPLETE, SERIO_JOB_SCANSEND );
	}
	return 0;
}

/*********************************************************************************************/
/**
* @par		(serio)Scan to FTPを行う
* @param	Param(input) JOBパラメータ
* @retval	OK	: 成功
* 			NG	: 失敗
* @par	<外部仕様>
*			Scan to FTPを行う
* @par	<内部仕様>
* 			Temporaryの設定を行い、Scan To FTPを開始する
*/
/*********************************************************************************************/
INT32	ScanSendFTP(VOID *Param)
{
    SERIO_CNP_IOJOB_SCANSEND_T  *iojob;
    ACCESS_INFO                  access_info;
    struct  ns__FtpParams       *pFtp;
    enum ns__ColorModeSelection  ColorMode;
    enum ns__UloadResolution          Resolution;
    UINT8           JobScanType;
    UINT8           filename[SCAN2FTP_FNAME_MAXLEN  + 1];
    UINT8           len;


	DPRINTF(("ScanSendFTP()\n"));
	memset(&access_info, 0, sizeof(access_info));

	iojob = (SERIO_CNP_IOJOB_SCANSEND_T *)Param;

#if 0 /**  (2010-08-06T12:10:00+09:00 kazushige.muroi) **/
	/*------------------------------------------------------------------------
	 * old
	 *----------------------------------------------------------------------*/
	if(getNumProfiles((SERIO_CNP_IOJOB_SCANSEND_T*)Param) > 0){
		pFtp = iojob->TxProfiles->__TxProfiles->TxProfile.Ftp.FtpParams;
	}else{
		pFtp = NULL;;
	}
#else  /**  (2010-08-06T12:10:00+09:00 kazushige.muroi) **/
	if (getNumProfiles((SERIO_CNP_IOJOB_SCANSEND_T*)Param) > 0 &&
		iojob->TxProfiles.__TxProfiles[0].__union == SOAP_UNION_ns__TxProfile_Ftp){
		pFtp = iojob->TxProfiles.__TxProfiles[0].TxProfile.Ftp.FtpParams;
		DPRINTF(("ScanSendFTP:set profile\n"));
	}
#endif /**  (2010-08-06T12:10:00+09:00 kazushige.muroi) **/



	if ( serio_IsDischargeTray() == TRUE ) {
		SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
		DPRINTF(("ScanSendFTP:serio_IsDischargeTray\n"));
		return OK;
	}

	/* 初期値設定 */
	JobScanType = GetJobScanType();
	if(JobScanType == JOB_SCAN_TYPE_FTP){
		/* ScanモードからScan to FTPを行ったとき */
		DPRINTF(("ScanSendFTP:JOB_SCAN_TYPE_FTP\n"));
		memcpy(&access_info, &FtpAccessInfo_Scan, sizeof(access_info));
	}else if(JobScanType == JOB_SCAN_TYPE_NONE){
		/* Serioアプリ実行でFTPのJOB実行通知が来たとき */

		DPRINTF(("ScanSendFTP:JOB_SCAN_TYPE_NONE\n"));
		if(pFtp == NULL){
			/* アプリ実行でFTPの設定が存在しないときはエラー */
			SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
			EPRINTF(("ScanSendFTP:profile NULL\n"));
			return OK;
		}

		memset(access_info.servername, 0, sizeof(access_info.servername));
		memset(access_info.serveraddress, 0, sizeof(access_info.serveraddress));
		memset(access_info.storedir, 0, sizeof(access_info.storedir));
		memset(access_info.username, 0, sizeof(access_info.username));
		memset(access_info.password, 0, sizeof(access_info.password));
		memset(access_info.spdfpass, 0, sizeof(access_info.spdfpass));
		memset(access_info.filename, 0, sizeof(access_info.filename));
		access_info.ispassive = 0;
		access_info.portnum = 21;
		access_info.quality = GetQuality(ns__ColorModeSelection__Color, ns__UloadResolution__Normal);
		access_info.fileformat = FTP_PDF;
		access_info.dualscan = FTP_SIMPLEX;
		access_info.filenametype = FTP_FNAMETYPE9_MANU;						/* アプリ起動はMANUAL固定 */
		access_info.scan_quality = P_SCAN_QUAL_NORMAL;
		access_info.scan_multifeed_detect = SCAN_MULTIFEED_DETECT_OFF;
		access_info.scan_quality_gray = SCAN_QUALITY_NORMAL;
		access_info.gndcolor_removal = SCAN_GCOL_REMOVAL_OFF;
		access_info.gndcolor_level = SCAN_GCOL_LEVEL_0;
		
		if(NULL != iojob->FileNameFixed){
			access_info.FileNameFixed = *(iojob->FileNameFixed);
		} else {
			access_info.FileNameFixed = FALSE;
		}
#ifdef	USE_TMP_SCANSIZE
		access_info.scan_doc_size = SCANSIZE_A4;
#endif	/* USE_TMP_SCANSIZE */
#ifdef	FB_SCAN_TYPE
		/* ADFに原稿があればADFを使用する */
		if ( MacStateRef(STID_MAN_SOURCE_IN) == MAC_ON ) {
			access_info.scan_src = SCAN_SRC_ADF;
		}else{
			access_info.scan_src = SCAN_SRC_FB;
		}
#endif	/* FB_SCAN_TYPE */
	}else{
		/* FTP以外のスキャンを行ったのに、FTPのJOB実行通知が来たときはエラー */
		EPRINTF(("ScanSendFTP:JOB_SCAN_TYPE ERROR\n"));
		SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
		return OK;
	}



	if(pFtp != NULL){
		DPRINTF(("ScanSendFTP:Set TxProfile\n"));
		/* TxProfileの設定 */
		strncpy((MD_CHAR *)access_info.servername,    pFtp->Host,     sizeof(access_info.servername) - 1);
		strncpy((MD_CHAR *)access_info.serveraddress, pFtp->Host,     sizeof(access_info.serveraddress) - 1);
		strncpy((MD_CHAR *)access_info.storedir,      pFtp->StoreDir, sizeof(access_info.storedir) - 1);
		strncpy((MD_CHAR *)access_info.username,      pFtp->User,     sizeof(access_info.username) - 1);
		strncpy((MD_CHAR *)access_info.password,      pFtp->Password, sizeof(access_info.password) - 1);
		strcpy((MD_CHAR *)access_info.spdfpass, "");

		if(pFtp->PassiveMode == xsd__boolean__false_){
			access_info.ispassive = 1;		/* Off */
		}else{
			access_info.ispassive = 0;		/* On */
		}
		access_info.portnum = atoi(pFtp->PortNum);
	}



	if(iojob->ColorMode != NULL || iojob->Resolution != NULL){
		if(iojob->ColorMode == NULL){
			ColorMode = ns__ColorModeSelection__Color;
		}else{
			ColorMode = *iojob->ColorMode;
		}
		if(iojob->Resolution == NULL){
			Resolution = ns__UloadResolution__Normal;
		}else{
			Resolution = *iojob->Resolution;
		}
		access_info.quality = GetQuality(ColorMode, Resolution);
	}



	if(iojob->FileType != NULL){
		access_info.fileformat = GetFileformat(iojob->FileType);
	}
	
	/* 禁則処理 */
	if(IsMono(access_info.quality) && ((access_info.fileformat == FTP_JPEG) || (access_info.fileformat == FTP_XPS) )){
		if((StrictParamFlag == TRUE) || (ParamChkOnlyFlag == TRUE))
		{
			/* パラメータエラー */
			ScanParamErrFlag = TRUE;
		}
		else
		{
			/* Scan解像度設定を取り直す */
			ColorMode = ns__ColorModeSelection__Gray;
			access_info.quality = GetQuality(ColorMode, Resolution);
		}
	}
	if((access_info.fileformat == FTP_Tiff) && ((IsGray(access_info.quality) == TRUE || IsColor(access_info.quality) == TRUE))){
		if((StrictParamFlag == TRUE) || (ParamChkOnlyFlag == TRUE))
		{
			/* パラメータエラー */
			ScanParamErrFlag = TRUE;
		}
		else
		{
			/* Scan解像度設定を取り直す */
			ColorMode = ns__ColorModeSelection__Mono;
			access_info.quality = GetQuality(ColorMode, Resolution);
		}
	}


	if(pFtp != NULL){
		get_filename(access_info.filename, sizeof(access_info.filename), (UINT8 *)pFtp->FileName, access_info.fileformat, access_info.FileNameFixed);
		access_info.filenametype = FTP_FNAMETYPE9_MANU;
	}else if(pFtp == NULL && iojob->FileType != NULL){
		/* Scanで設定したファイルタイプと、IOJOBで設定されたファイルタイプが異なるときがあるので、*/
		/* ファイル名の後ろの部分(スキャンカウントと拡張子)を再設定する。 */

		/* スキャンカウントと拡張子の文字列長さ(_XXXXXX.pdf)*/
		len = (1 + 6 + 1 + 3);
		if(strlen((MD_CHAR *)access_info.filename) >= len){
			/* Scanで設定したスキャンカウントと拡張子を除いたファイル名を取得 */
			memset(filename, 0, sizeof(filename));
			strncpy((MD_CHAR *)filename, (MD_CHAR *)access_info.filename, strlen((MD_CHAR *)access_info.filename) - len);
			get_filename(access_info.filename, sizeof(access_info.filename), filename, access_info.fileformat, access_info.FileNameFixed);
		}
	}
	

#ifdef USE_DUPLEX_SCAN
	if(iojob->DuplexScanEnable != NULL || iojob->ShortEdgeBinding != NULL){
		if(iojob->DuplexScanEnable == NULL || *iojob->DuplexScanEnable == xsd__boolean__false_){
			access_info.dualscan = FTP_SIMPLEX;
		}else if(*iojob->DuplexScanEnable == xsd__boolean__true_){
			if(iojob->ShortEdgeBinding == NULL || *iojob->ShortEdgeBinding == xsd__boolean__false_){
				access_info.dualscan = FTP_DUPLEXLONG;
			}else{
				access_info.dualscan = FTP_DUPLEXSHORT;
			}
		}
	}
#endif


#ifdef	FB_SCAN_TYPE
	if(iojob->ScanTray != NULL){
		if(*iojob->ScanTray == ns__ScanTraySelection__FB){
			access_info.scan_src = SCAN_SRC_FB;
		}else{
			access_info.scan_src = SCAN_SRC_ADF;
		}
	}


	/* 禁則処理 */
	if((MacStateRef(STID_MAN_SOURCE_IN) == MAC_OFF) || (access_info.scan_src == SCAN_SRC_FB)){
		/* ADFに原稿がない、またはFB読み取り指定のときは強制的に、FBでSIMPLEXに設定を変更する */
		access_info.scan_src = SCAN_SRC_FB;
		access_info.dualscan = FTP_SIMPLEX;
	}
#endif	/* FB_SCAN_TYPE */
	
#ifdef	USE_TMP_SCANSIZE
	/* Scan DocSizeを取得する */
	if(iojob->DocSize != NULL){
		access_info.scan_doc_size = get_scan_doc_size(*iojob->DocSize);
	}
#endif	/* USE_TMP_SCANSIZE */

	/* 電文から指示された画質から読み取りタスクが使用する画質を取得する */
	if(iojob->JpgQuality != NULL){
		access_info.scan_quality = get_scan_quality(*iojob->JpgQuality);
	}
	else {	/* タグ無しの場合はMidを設定する */
		access_info.scan_quality = P_SCAN_QUAL_MID;
	}
	
	/* Scan開始前に厳密設定において、パラメータ間違いがあった場合はキャンセル終了する */
	if(ScanParamErrFlag == TRUE)
	{
		/* Scan中断通知をSerioFWに返す */
		SendJobStatus_End(SERIOFW_JOBSTS_END_CANCEL, SERIO_EXTERR_INABILITY, SERIO_JOB_SCANSEND);
		return OK;
	}
	
	if(ParamChkOnlyFlag != TRUE)
	{
#ifdef USE_SCAN2FTP
		ftpclient_setaccessinfo( &access_info );
#endif /* USE_SCAN2FTP */
#ifdef SEND_SCAN_DEBUG_PRINT
		debug_print_ftp(&access_info);
#endif

#ifdef	USE_SEPARATE_UI
		/* ここでTMP領域へ現在の設定値をセットする */
		SetTmpdataIoJobFtpAccessinfo(&access_info);
#if 1	/* BILStArの状態によっては、else側を使う可能性があるため残しておく */
		/* CPタスクへSCANアプリ起動要求を出す */
		SendMsgToCp(CPAPI_MSGCMDID_SERIO_START_SCAN);
#else	/* if 1 */
		/* アプリ状態を登録 */
		cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_START);
		cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_EXEC);
#endif	/* if 1 */
#endif	/* USE_SEPARATE_UI */
		
#ifdef ENABLE_VIRTUAL_MFC
		{
			SERIO_EVP_JOBSTS_T  jobsts;

			jobsts.JobID = SERIOFW_IOJOB_SCANSEND;
			jobsts.Status = SERIOFW_JOBSTS_END;
			jobsts.Param.End.Reason = SERIOFW_JOBSTS_END_COMPLETE;
	        Initialize__EvJobFinishT(&jobsts.Param.End.Detail);
			SerioFwEventNotify(SERIOFW_EVID_JOBSTATUS, &jobsts);
		}
#endif
	}
	else
	{
		/* Paramチェックのみの場合は、ここでIO Job完了通知をする */
		SendJobStatus_End( SERIOFW_JOBSTS_END_COMPLETE, SERIO_EXTERR_COMPLETE, SERIO_JOB_SCANSEND );
	}
	
    return OK;
}

#ifdef ENABLE_VIRTUAL_MFC /** ENABLE_VIRTUAL_MFC **/
/*------------------------------------------------------------------------
 * Initialize__EvJobFinishT
 *    SERIO_EV_JOB_FINISH_T構造体を初期化する。
 *----------------------------------------------------------------------*/
STATIC INT32
Initialize__EvJobFinishT(SERIO_EV_JOB_FINISH_T* jobdone)
{
    if (!jobdone) {
        return NG;
    }
    jobdone->ErrorDetail    = NULL;
    jobdone->ServerErrorMsg = NULL;
    jobdone->ExecutableErr  = NULL;
    jobdone->JobName        = NULL;
    jobdone->UserId         = NULL;
    jobdone->Timestamp      = NULL;
    jobdone->NumPages       = NULL;
    jobdone->NumColorPages  = NULL;
    jobdone->NumImages      = NULL;
    jobdone->MeanCoverage   = NULL;
    return OK;
}

#endif /** ENABLE_VIRTUAL_MFC **/


/*********************************************************************************************/
/**
* @par		(serio)Scan to Networkを行う
* @param	Param(input) JOBパラメータ
* @retval	OK	: 成功
* 			NG	: 失敗
* @par	<外部仕様>
*			Scan to Networkを行う
* @par	<内部仕様>
* 			Temporaryの設定を行い、Scan To Networkを開始する
*/
/*********************************************************************************************/
INT32	ScanSendNetwork(VOID *Param)
{
	SERIO_CNP_IOJOB_SCANSEND_T *iojob;
	CIFSACCESS_INFO				access_info;				/* 10あるうちの、一つに選択されたFTP登録情報 */
	struct 	ns__CifsParams 	*pCifs;
	enum ns__ColorModeSelection ColorMode;
	enum ns__UloadResolution 		Resolution;
	UINT8			filename[SCAN2FTP_FNAME_MAXLEN  + 1];
	UINT8			len;


	memset(&access_info, 0, sizeof(access_info));

	iojob = (SERIO_CNP_IOJOB_SCANSEND_T *)Param;
#if 0 /**  (2010-08-06T12:09:11+09:00 kazushige.muroi) **/
	/*------------------------------------------------------------------------
	 * old
	 *----------------------------------------------------------------------*/
	if(iojob->TxProfiles != NULL){
		pCifs = iojob->TxProfiles->__TxProfiles->TxProfile.Cifs.CifsParams;
	}else{
		pCifs = NULL;
	}
#else  /**  (2010-08-06T12:09:11+09:00 kazushige.muroi) **/
    if (getNumProfiles((SERIO_CNP_IOJOB_SCANSEND_T*)Param) > 0) {
        pCifs = iojob->TxProfiles.__TxProfiles->TxProfile.Cifs.CifsParams;
    }
#endif /**  (2010-08-06T12:09:11+09:00 kazushige.muroi) **/

	DPRINTF(("ScanSendNetwork()\n"));

	if ( serio_IsDischargeTray() == TRUE ) {
		EPRINTF(("serio_IsDischargeTray()\n"));
		SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
		return OK;
	}

	/* 初期値設定 */
	if(JobScanType == JOB_SCAN_TYPE_NETWORK){
		/* ScanモードからScan to Networkを行ったとき */
		DPRINTF(("ScanSendNetwork:JOB_SCAN_TYPE_NETWORK\n"));
		memcpy(&access_info, &CifsAccessInfo_Scan, sizeof(access_info));
	}else if(JobScanType == JOB_SCAN_TYPE_NONE){
		DPRINTF(("ScanSendNetwork:JOB_SCAN_TYPE_NONE\n"));
		/* Serioアプリ実行でNetworkのJOB実行通知が来たとき */
		if(pCifs == NULL){
			/* アプリ実行でFTPの設定が存在しないときはエラー */
			EPRINTF(("pCifs == NULL\n"));
			SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
			return OK;
		}
		memset(access_info.serveraddress, 0, sizeof(access_info.serveraddress));
		memset(access_info.KerberosServerAddress, 0, sizeof(access_info.KerberosServerAddress));
		memset(access_info.storedir, 0, sizeof(access_info.storedir));
		memset(access_info.username, 0, sizeof(access_info.username));
		memset(access_info.password, 0, sizeof(access_info.password));
		memset(access_info.spdfpass, 0, sizeof(access_info.spdfpass));
		memset(access_info.filename, 0, sizeof(access_info.filename));
		access_info.filenametype = FTP_FNAMETYPE9_MANU;						/* アプリ起動はMANUAL固定 */
		access_info.quality = GetQuality(ns__ColorModeSelection__Color, ns__UloadResolution__Normal);
		access_info.fileformat = FTP_PDF;
		access_info.dualscan = FTP_SIMPLEX;
		access_info.AuthenticationMethod = CIFSCLIB_AUTHMETH_AUTO;
		access_info.scan_quality = P_SCAN_QUAL_NORMAL;
		access_info.scan_multifeed_detect = SCAN_MULTIFEED_DETECT_OFF;
		access_info.scan_quality_gray = SCAN_QUALITY_NORMAL;
		access_info.gndcolor_removal = SCAN_GCOL_REMOVAL_OFF;
		access_info.gndcolor_level = SCAN_GCOL_LEVEL_0;
		
		if(NULL != iojob->FileNameFixed){
			access_info.FileNameFixed = *(iojob->FileNameFixed);
		} else {
			access_info.FileNameFixed = FALSE;
		}
#ifdef	USE_TMP_SCANSIZE
		access_info.scan_doc_size = SCANSIZE_A4;
#endif	/* USE_TMP_SCANSIZE */
#ifdef	FB_SCAN_TYPE
		/* ADFに原稿があればADFを使用する */
		if ( MacStateRef(STID_MAN_SOURCE_IN) == MAC_ON ) {
			access_info.scan_src = SCAN_SRC_ADF;
		}else{
			access_info.scan_src = SCAN_SRC_FB;
		}
#endif	/* FB_SCAN_TYPE */
	}else{
		EPRINTF(("ScanSendNetwork:JOB_SCAN_TYPE_ERROR\n"));
		/* Network以外のスキャンを行ったのに、NetworkのJOB実行通知が来たときはエラー */
		SendJobStatus_End(SERIOFW_JOBSTS_END_SYSBUSY, SERIO_EXTERR_SYSTEMBUSY, SERIO_JOB_SCANSEND);
		return OK;
	}


	if(pCifs != NULL){
		DPRINTF(("ScanSendNetwork:pCifs != NULL\n"));
		strncpy_safe((MD_CHAR *)access_info.serveraddress,         pCifs->Host,           sizeof(access_info.serveraddress) - 1);
		strncpy_safe((MD_CHAR *)access_info.KerberosServerAddress, pCifs->KerberosServer, sizeof(access_info.KerberosServerAddress) - 1);
		strncpy_safe((MD_CHAR *)access_info.storedir,              pCifs->StoreDir,       sizeof(access_info.storedir) - 1);
		strncpy_safe((MD_CHAR *)access_info.username,              pCifs->User,           sizeof(access_info.username) - 1);
		strncpy_safe((MD_CHAR *)access_info.password,              pCifs->Password,       sizeof(access_info.password) - 1);
		strcpy((MD_CHAR *)access_info.spdfpass, "");

		if(pCifs->AuthMethod == ns__CifsParams_AuthMethod__Auto){
			DPRINTF(("ScanSendNetwork:CIFSCLIB_AUTHMETH_AUTO\n"));
			access_info.AuthenticationMethod = CIFSCLIB_AUTHMETH_AUTO;
		}else if(pCifs->AuthMethod == ns__CifsParams_AuthMethod__Kerberos){
			DPRINTF(("ScanSendNetwork:CIFSCLIB_AUTHMETH_KERBEROS\n"));
			access_info.AuthenticationMethod = CIFSCLIB_AUTHMETH_KERBEROS;
		}else if(pCifs->AuthMethod == ns__CifsParams_AuthMethod__NTLMv2){
			DPRINTF(("ScanSendNetwork:CIFSCLIB_AUTHMETH_NTLMV2\n"));
			access_info.AuthenticationMethod = CIFSCLIB_AUTHMETH_NTLMV2;
		}
	}

	if(iojob->ColorMode != NULL || iojob->Resolution != NULL){
		if(iojob->ColorMode == NULL){
			ColorMode = ns__ColorModeSelection__Color;
		}else{
			ColorMode = *iojob->ColorMode;
		}
		if(iojob->Resolution == NULL){
			Resolution = ns__UloadResolution__Normal;
		}else{
			Resolution = *iojob->Resolution;
		}
		access_info.quality = GetQuality(ColorMode, Resolution);
	}

	if(iojob->FileType != NULL){
		access_info.fileformat = GetFileformat(iojob->FileType);
	}

	/* 禁則処理 */
	if(IsMono(access_info.quality) && (access_info.fileformat == FTP_JPEG || access_info.fileformat == FTP_XPS )){
		if((StrictParamFlag == TRUE) || (ParamChkOnlyFlag == TRUE))
		{
			/* パラメータエラー */
			ScanParamErrFlag = TRUE;
		}
		else
		{
			/* Scan解像度設定を取り直す */
			ColorMode = ns__ColorModeSelection__Gray;
			access_info.quality = GetQuality(ColorMode, Resolution);
		}
	}
	if(access_info.fileformat == FTP_Tiff && (IsGray(access_info.quality) == TRUE || IsColor(access_info.quality) == TRUE)){
		if((StrictParamFlag == TRUE) || (ParamChkOnlyFlag == TRUE))
		{
			/* パラメータエラー */
			ScanParamErrFlag = TRUE;
		}
		else
		{
			/* Scan解像度設定を取り直す */
			ColorMode = ns__ColorModeSelection__Mono;
			access_info.quality = GetQuality(ColorMode, Resolution);
		}
	}

	if(pCifs != NULL){	
		get_filename(access_info.filename,  sizeof(access_info.filename), (UINT8 *)pCifs->FileName, access_info.fileformat, access_info.FileNameFixed);
		access_info.filenametype = FTP_FNAMETYPE9_MANU;
	}else if(pCifs == NULL && iojob->FileType != NULL){
		/* Scanで設定したファイルタイプと、IOJOBで設定されたファイルタイプが異なるときがあるので、*/
		/* ファイル名の後ろの部分(スキャンカウントと拡張子)を再設定する。 */
		
		/* スキャンカウントと拡張子の文字列長さ(_XXXXXX.pdf)*/
		len = (1 + 6 + 1 + 3);
		if(strlen((MD_CHAR *)access_info.filename) >= len){
			/* Scanで設定したスキャンカウントと拡張子を除いたファイル名を取得 */
			memset(filename, 0, sizeof(filename));
			strncpy((MD_CHAR *)filename, (MD_CHAR *)access_info.filename, strlen((MD_CHAR *)access_info.filename) - len);
			get_filename(access_info.filename, sizeof(access_info.filename), filename, access_info.fileformat, access_info.FileNameFixed);
		}
	}

#ifdef USE_DUPLEX_SCAN
    
	if(iojob->DuplexScanEnable != NULL || iojob->ShortEdgeBinding != NULL){
		if(iojob->DuplexScanEnable == NULL || *iojob->DuplexScanEnable == xsd__boolean__false_){
			access_info.dualscan = FTP_SIMPLEX;
		}else if(*iojob->DuplexScanEnable == xsd__boolean__true_){
			if(iojob->ShortEdgeBinding == NULL || *iojob->ShortEdgeBinding == xsd__boolean__false_){
				access_info.dualscan = FTP_DUPLEXLONG;
			}else{
				access_info.dualscan = FTP_DUPLEXSHORT;
			}
		}
	}
#endif

#ifdef FB_SCAN_TYPE
	if(iojob->ScanTray != NULL){
		if(*iojob->ScanTray == ns__ScanTraySelection__FB){
			access_info.scan_src = SCAN_SRC_FB;
		}else{
			access_info.scan_src = SCAN_SRC_ADF;
		}
	}

	/* 禁則処理 */
	if(MacStateRef(STID_MAN_SOURCE_IN) == MAC_OFF || access_info.scan_src == SCAN_SRC_FB){
		/* ADFに原稿がない、またはFB読み取り指定のときは強制的に、FBでSIMPLEXに設定を変更する */
		access_info.scan_src = SCAN_SRC_FB;
		access_info.dualscan = FTP_SIMPLEX;
	}
#endif	/* FB_SCAN_TYPE */

#ifdef	USE_TMP_SCANSIZE
	/* Scan DocSizeを取得する */
	if(iojob->DocSize != NULL){
		access_info.scan_doc_size = get_scan_doc_size(*iojob->DocSize);
	}
#endif	/* USE_TMP_SCANSIZE */

	/* 電文から指示された画質から読み取りタスクが使用する画質を取得する */
	if(iojob->JpgQuality != NULL){
		access_info.scan_quality = get_scan_quality(*iojob->JpgQuality);
	}
	else {	/* タグ無しの場合はMidを設定する */
		access_info.scan_quality = P_SCAN_QUAL_MID;
	}
	
	/* Scan開始前に厳密設定において、パラメータ間違いがあった場合はキャンセル終了する */
	if(ScanParamErrFlag == TRUE)
	{
		/* Scan中断通知をSerioFWに返す */
		SendJobStatus_End(SERIOFW_JOBSTS_END_CANCEL, SERIO_EXTERR_INABILITY, SERIO_JOB_SCANSEND);
		return OK;
	}
	
	if(ParamChkOnlyFlag != TRUE)
	{
#ifdef	USE_SCAN2NW
		cifsclient_setaccessinfo( &access_info );
#endif /* USE_SCAN2NW */
#ifdef SEND_SCAN_DEBUG_PRINT
		debug_print_network(&access_info);
#endif

#ifdef	USE_SEPARATE_UI
		/* ここでTMP領域へ現在の設定値をセットする */
		SetTmpdataIoJobCifsAccessinfo(&access_info);
#if 1	/* BILStArの状態によっては、else側を使う可能性があるため残しておく */
		/* CPタスクへSCANアプリ起動要求を出す */
		SendMsgToCp(CPAPI_MSGCMDID_SERIO_START_SCAN);
#else	/* if 1 */
		/* アプリ状態を登録 */
		cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_START);
		cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_EXEC);
#endif	/* if 1 */
#endif	/* USE_SEPARATE_UI */
		
#ifdef ENABLE_VIRTUAL_MFC
		{
			SERIO_EVP_JOBSTS_T  jobsts;
			jobsts.JobID = SERIOFW_IOJOB_SCANSEND;
			jobsts.Status = SERIOFW_JOBSTS_END;
			jobsts.Param.End.Reason = SERIOFW_JOBSTS_END_COMPLETE;
	        Initialize__EvJobFinishT(&jobsts.Param.End.Detail);
			SerioFwEventNotify(SERIOFW_EVID_JOBSTATUS, &jobsts);
		}
#endif
	}
	else
	{
		/* Paramチェックのみの場合は、ここでIO Job完了通知をする */
		SendJobStatus_End( SERIOFW_JOBSTS_END_COMPLETE, SERIO_EXTERR_COMPLETE, SERIO_JOB_SCANSEND );
	}
	
	return OK;
}




/*********************************************************************************************/
/**
* @par		(serio)Scan to E-MailのJOB情報の設定を行う
* @param	Param(input) JOB情報
* @retval	なし
* @par	<外部仕様>
*			Scan to E-MailのJOB情報の設定を行う
* @par	<内部仕様>
* 			現在の設定でJOB情報を設定する
*/
/*********************************************************************************************/
STATIC void get_current_set(current_t *state)
{
	/* 原稿／メモリ */
//	state->docmem = chk_docmem();
	state->docmem = CRNT_DM_MEMIN;
	/* コントラスト */
	state->contrast = fstget_data(FUNC_CONTRAST);
	/* 解像度 */
#ifdef	USE_MULTI_RESOLUTION
	fstget_data_str(FUNC_RESOLUTION, (MAX_SCAN_MAULT + 3) / 4,
													state->resolution);
#else
	state->resolution[0] = fstget_data(FUNC_RESOLUTION);
#endif
	/* ポーリング 被呼／発呼 */
	if (fstget_data(FUNC_POLLING_RCV) == POLLING_RCV_ON) {
		state->polling = CRNT_PL_POLLRX;
		state->docmem = CRNT_DM_NA;
		if (cspecGetFunc(POLL_SECURE_FUNC) == TRUE) {
			fstget_data_str(FUNC_SEPOLL_PASSWD_RX, 2, state->password_id);
		} else {
			memset(state->password_id, 0xAA, 2);
		}
	}
	else if (fstget_data(FUNC_POLLED_TX) == POLLED_TX_ON) {
		state->polling = CRNT_PL_POLLTX;
		if (cspecGetFunc(POLL_SECURE_FUNC) == TRUE) {
			fstget_data_str(FUNC_SEPOLL_PASSWD_TX, 2, state->password_id);
		} else {
			memset(state->password_id, 0xAA, 2);
		}
	}
	else {
		state->polling = CRNT_PL_NA;
		memset(state->password_id, 0xAA, 2);
	}
#if defined(COLOR_FAX_FUNC)||defined(COLOR_NETWORK_SCANNER)
	/* カラーＦＡＸ送信 */
	state->color_flag = fstget_data(FUNC_COLORFAX);
#else
	/* カラー無しモデルでのモノクロPDF用 */
	state->color_flag = COLORFAX_OFF;
#endif
	/* Gray Scale */
	state->gray_scale = fstget_data(FUNC_SCAN_GRAYSCALE);
	/* パスワード送信 */
	state->password = CRNT_PW_OFF;
	/* カバーページ */
#ifdef COVERPAGE_FUNC
	state->coverpage = fstget_data(FUNC_CVPAGE_MODE);
#if defined( COLOR_FAX_FUNC) && defined(USE_LASER_PRINT)	/*M-LCF-983/1076 Dialing/NoCoverPageスクロール表示対応*/
	if((state->color_flag == COLORFAX_ON) && (state->coverpage != CVPAGE_MODE_OFF)){
		state->coverpage = CVPAGE_MODE_OFF;
		state->cvpg_cancel_flag = CVPAGE_CANCEL_ON;
	}
	else{
		state->cvpg_cancel_flag = CVPAGE_CANCEL_OFF;
	}
#endif
	state->cvpg_total_page = fstget_data(FUNC_TOTAL_PAGE);
#endif /* COVERPAGE_FUNC */
	/* 会話予約 */
	state->call_reserve = fstget_data(FUNC_CALL_RESERVE);
	/* 海外通信モード */
	state->overseas = fstget_data(FUNC_OVERSEA_MODE);
	/* インタラプトモード */
	if (MacStateRef(STID_INTERRUPT) == MAC_OFF) {
		state->interrupt = INTERRUPT_OFF;
	}
	else {
		state->interrupt = INTERRUPT_ON;
	}
#ifdef	USE_NETWORK_SCANNER
	/* ネットワークスキャナーモードの設定 */
	state->net_scan = fstget_data(FUNC_NETWORK_SCANNER);
	/* ファイルフォーマット */
#ifdef	COLOR_NETWORK_SCANNER
	if(state->color_flag == COLORFAX_ON){
		state->file_format = fstget_data(FUNC_LAN_SCAN_FORMAT);
	}
	else if(state->gray_scale == USW_SCAN_MULTI){
		state->file_format = fstget_data(FUNC_LAN_SCAN_FORMAT_GRAY);
	}
	else{
		state->file_format = fstget_data(FUNC_LAN_SCAN_FORMAT_BW);
	}
#else
	state->file_format = fstget_data(FUNC_LAN_SCAN_FORMAT_BW);
#endif
#endif
#ifdef	USE_FAXTRN_FUNC
	/* FAX転送モードを設定する */
	state->fax_trans_mode = fstget_data(FUNC_TMP_TRANSFER);
#endif

	/* カレント設定をデフォルトに戻す */
	fstreset_sendset();

	return;
}



/******************************************************************************
 * @per     STATIC UINT8 get_scan_quality(SCAN2FTP_QUALITY quality)
 * @param   [I/-] quality        : 画質
 * @retval  電文で指定された画質
 * @per     <外部仕様>
 *          電文で指定された画質を取得する。
 *          <内部仕様>
 *          引数の電文で指定された画質を取得し、
 *          読み取りタスクが使用する画質へ変換する
******************************************************************************/
STATIC UINT8
get_scan_quality(enum ns__Selection3 quality)
{
	UINT8					ret;							/* 画質 */

	switch(quality){
		case ns__Selection3__Low:		/* 低画質 */
			ret = P_SCAN_QUAL_NORMAL;
			break;
		case ns__Selection3__Normal:	/* 中画質 */
		default:
			ret = P_SCAN_QUAL_MID;
			break;
		case ns__Selection3__High:		/* 高画質 */
			ret = P_SCAN_QUAL_HIGH;
			break;
	}

	return ret;
}

#ifdef	USE_TMP_SCANSIZE
/******************************************************************************
 * @per     get_scan_doc_size(enum ns__ScanAndUloadScansize scan_doc_size)
 * @param   [I/-] scan_doc_size        : Scan Document Size
 * @retval  電文で指定されたScan Document Size
 * @per     <外部仕様>
 *          電文で指定されたScan Document Sizeを取得する。
 *          <内部仕様>
 *          電文で指定されたScan Document SizeをScan用情報に変換する。
******************************************************************************/
STATIC	UINT8
get_scan_doc_size(enum ns__ScanAndUloadScansize scan_doc_size)
{
	
	UINT8					ret;							/* Scan Document Size */
	
	switch(scan_doc_size){
		case ns__ScanAndUloadScansize__Letter:
			ret = SCANSIZE_LETTER;
			break;
		case ns__ScanAndUloadScansize__Legal:
			ret = SCANSIZE_LEGAL;
			break;
		case ns__ScanAndUloadScansize__A4:
		default:
			ret = SCANSIZE_A4;
			break;
	}
	
	return ret;
}
#endif	/* USE_TMP_SCANSIZE */

/*********************************************************************************************/
/**
* @par		(serio)FTP用の解像度の取得
* @param	ColorMode(input) カラー設定
* @param	Resolution(input) 解像度
* @retval	OK	: 成功
* 			NG	: 失敗
* @par	<外部仕様>
*			FTP用の解像度の取得
* @par	<内部仕様>
* 			カラー設置と解像度からFTP用の解像度を取得する
*			サーバの設定はModel毎には用意出来ないことが考えられるため、本体側で値を補正する。
*			※モデル毎にメンテの必要あり<Modelによって対応外の設定があり、Modelで判別できるCSWもないため>
*/
/*********************************************************************************************/
STATIC	SCAN2FTP_QUALITY
GetQuality(enum ns__ColorModeSelection ColorMode, enum ns__UloadResolution Resolution)
{
	SCAN2FTP_QUALITY	ret;

	ret = FTP_BW100;
	if(ColorMode == ns__ColorModeSelection__Mono){
		/* Mono設定 */
		switch(Resolution)
		{
			case ns__UloadResolution__100:
				if(StrictParamFlag == TRUE)
				{
					ScanParamErrFlag = TRUE;
				}
			case ns__UloadResolution__Low:
			case ns__UloadResolution__200x100:
				ret = FTP_BW100;
				break;
			case ns__UloadResolution__Auto:
				if(StrictParamFlag == TRUE)
				{
					ScanParamErrFlag = TRUE;
				}
			case ns__UloadResolution__Normal:
			case ns__UloadResolution__200:
				ret = FTP_BW200;
				break;
			case ns__UloadResolution__400:
			case ns__UloadResolution__600:
				if(StrictParamFlag == TRUE)
				{
					ScanParamErrFlag = TRUE;
				}
			case ns__UloadResolution__High:
			case ns__UloadResolution__300:
				ret = FTP_BW300;
				break;
		}
	}else if(ColorMode == ns__ColorModeSelection__Gray){
		/* Gray設定 */
		switch(Resolution)
		{
			case ns__UloadResolution__200x100:
				if(StrictParamFlag == TRUE)
				{
					ScanParamErrFlag = TRUE;
				}
			case ns__UloadResolution__Low:
			case ns__UloadResolution__100:
				ret = FTP_Gray100;
				break;
			case ns__UloadResolution__Normal:
			case ns__UloadResolution__200:
				ret = FTP_Gray200;
				break;
			case ns__UloadResolution__400:
			case ns__UloadResolution__600:
				if(StrictParamFlag == TRUE)
				{
					ScanParamErrFlag = TRUE;
				}
			case ns__UloadResolution__High:
			case ns__UloadResolution__300:
				ret = FTP_Gray300;
				break;
			case ns__UloadResolution__Auto:
#ifdef	USE_SCAN_AUTO_RESOLUTION
				ret = FTP_GrayAuto;
#else	/* USE_SCAN_AUTO_RESOLUTION */
				ret = FTP_Gray300;
#endif	/* USE_SCAN_AUTO_RESOLUTION */
				break;
		}
	}else if(ColorMode == ns__ColorModeSelection__Color){
		/* Color設定 */
		switch(Resolution)
		{
			case ns__UloadResolution__200x100:
				if(StrictParamFlag == TRUE)
				{
					ScanParamErrFlag = TRUE;
				}
			case ns__UloadResolution__Low:
			case ns__UloadResolution__100:
				ret = FTP_Color100;
				break;
			case ns__UloadResolution__200:
				ret = FTP_Color200;
				break;
			case ns__UloadResolution__400:
				if(StrictParamFlag == TRUE)
				{
					ScanParamErrFlag = TRUE;
				}
			case ns__UloadResolution__Normal:
			case ns__UloadResolution__300:
				ret = FTP_Color300;
				break;
			case ns__UloadResolution__High:
			case ns__UloadResolution__600:
				ret = FTP_Color600;
				break;
			case ns__UloadResolution__Auto:
#ifdef	USE_SCAN_AUTO_RESOLUTION
				ret = FTP_ColorAuto;
#else	/* USE_SCAN_AUTO_RESOLUTION */
				ret = FTP_Color300;
#endif	/* USE_SCAN_AUTO_RESOLUTION */
				break;
		}
	}

	return ret;
}

/*********************************************************************************************/
/**
* @par		(serio)FTP用のファイルフォーマットの取得
* @param	FileType(input) ファイルフォーマット
* @retval	OK	: 成功
* 			NG	: 失敗
* @par	<外部仕様>
*			FTP用のファイルフォーマットの取得
* @par	<内部仕様>
* 			JOB情報のファイルフォーマットから、FTP用のファイルフォーマットへ変換し取得する
*/
/*********************************************************************************************/
STATIC	SCAN2FTP_FILEFORMAT
GetFileformat(enum ns__FileFormatSelection *FileType)
{
	SCAN2FTP_FILEFORMAT		ret;

	ret = FTP_PDF;
	if(FileType == NULL){
		ret = FTP_PDF;
	}else if(*FileType == ns__FileFormatSelection__TIFF){
		ret = FTP_Tiff;
	}else if(*FileType == ns__FileFormatSelection__JPEG){
		ret = FTP_JPEG;
	}else if(*FileType == ns__FileFormatSelection__PDF){
		ret = FTP_PDF;
	}else if(*FileType == ns__FileFormatSelection__XPS){
		ret = FTP_XPS;
	}else if(*FileType == ns__FileFormatSelection__PDFA){
		ret = FTP_PDFA;	
	}else if(*FileType == ns__FileFormatSelection__SPDF){
		ret = FTP_SPDF;	
	}else if(*FileType == ns__FileFormatSelection__SIPDF){
		ret = FTP_SIPDF;	
	}	

	return ret;
}


/*********************************************************************************************/
/**
* @par		(serio)E-Mail用のファイルフォーマットの取得
* @param	FileType(input) ファイルフォーマット
* @retval	OK	: 成功
* 			NG	: 失敗
* @par	<外部仕様>
*			E-Mail用のファイルフォーマットの取得
* @par	<内部仕様>
* 			JOB情報のファイルフォーマットから、E-Mail用のファイルフォーマットへ変換し取得する
*/
/*********************************************************************************************/
STATIC	UINT8
GetFileformat_EMail(enum ns__FileFormatSelection FileType)
{
	UINT8	ret;

	ret = FILE_FORMAT_PDF;
	if(FileType == ns__FileFormatSelection__TIFF){
		ret = FILE_FORMAT_TIFF;
	}else if(FileType == ns__FileFormatSelection__JPEG){
		ret = FILE_FORMAT_JPEG;
	}else if(FileType == ns__FileFormatSelection__PDF){
		ret = FILE_FORMAT_PDF;
	}else if(FileType == ns__FileFormatSelection__XPS){
		ret = FILE_FORMAT_XPS;
	 }else if(FileType == ns__FileFormatSelection__SPDF){
	  ret = FILE_FORMAT_SPDF;
	 }else if(FileType == ns__FileFormatSelection__SIPDF){
	  ret = FILE_FORMAT_SIPDF;
	 }else if(FileType == ns__FileFormatSelection__PDFA){
	  ret = FILE_FORMAT_PDFA;
	 }

	return ret;
}	



/*********************************************************************************************/
/**
* @par		(serio)E-Mail用の解像度の取得
* @param	ColorMode(input) カラー設定
* @param	Resolution(input) 解像度
* @retval	OK	: 成功
* 			NG	: 失敗
* @par	<外部仕様>
*			E-Mail用の解像度の取得
* @par	<内部仕様>
* 			カラー設置と解像度からFTP用の解像度を取得する
*			外部Serverには本体設定は意識させないため、この関数の中で設定を丸める。
*/
/*********************************************************************************************/
STATIC	UINT8
GetResolution_EMail(enum ns__ColorModeSelection ColorMode, enum ns__UloadResolution Resolution)
{
	UINT8	reso;

	reso = RESOLUTION_STD;
	if(ColorMode == ns__ColorModeSelection__Mono){
		/* Mono設定 */
		switch(Resolution)
		{
			case ns__UloadResolution__100:
				if(StrictParamFlag == TRUE)
				{
					ScanParamErrFlag = TRUE;
				}
			case ns__UloadResolution__Low:
			case ns__UloadResolution__200x100:
				reso = RESOLUTION_STD;
				break;
			case ns__UloadResolution__Auto:
			case ns__UloadResolution__Normal:
			case ns__UloadResolution__200:
				reso = RESOLUTION_FINE;
				break;
			case ns__UloadResolution__400:
			case ns__UloadResolution__600:
				if(StrictParamFlag == TRUE)
				{
					ScanParamErrFlag = TRUE;
				}
			case ns__UloadResolution__High:
			case ns__UloadResolution__300:
				reso = RESOLUTION_SFINE;
				break;
			default:
				break;
		}
	}else if(ColorMode == ns__ColorModeSelection__Gray){
		/* Gray設定 */
		switch(Resolution)
		{
			case ns__UloadResolution__200x100:
				if(StrictParamFlag == TRUE)
				{
					ScanParamErrFlag = TRUE;
				}
			case ns__UloadResolution__Low:
			case ns__UloadResolution__100:
				reso = RESOLUTION_STD;
				break;
			case ns__UloadResolution__Normal:
			case ns__UloadResolution__200:
				reso = RESOLUTION_FINE;
				break;
			case ns__UloadResolution__400:
			case ns__UloadResolution__600:
				if(StrictParamFlag == TRUE)
				{
					ScanParamErrFlag = TRUE;
				}
			case ns__UloadResolution__High:
			case ns__UloadResolution__300:
				reso = RESOLUTION_SFINE;
				break;
			case ns__UloadResolution__Auto:
#ifdef	USE_SCAN_AUTO_RESOLUTION
				reso = RESOLUTION_GRAUTO;
#else	/* USE_SCAN_AUTO_RESOLUTION */
				reso = RESOLUTION_SFINE;
#endif	/* USE_SCAN_AUTO_RESOLUTION */
				break;
			default:
				break;
		}
	}else if(ColorMode == ns__ColorModeSelection__Color){
		/* Color設定 */
		switch(Resolution)
		{
			case ns__UloadResolution__200x100:
				if(StrictParamFlag == TRUE)
				{
					ScanParamErrFlag = TRUE;
				}
			case ns__UloadResolution__Low:
			case ns__UloadResolution__100:
				reso = RESOLUTION_STD;
				break;
			case ns__UloadResolution__200:
				reso = RESOLUTION_FINE;
				break;
			case ns__UloadResolution__400:
				if(StrictParamFlag == TRUE)
				{
					ScanParamErrFlag = TRUE;
				}
			case ns__UloadResolution__Normal:
			case ns__UloadResolution__300:
				reso = RESOLUTION_SFINE;
				break;
			case ns__UloadResolution__High:
			case ns__UloadResolution__600:
				reso = RESOLUTION_PHOTO;
				break;
			case ns__UloadResolution__Auto:
#ifdef	USE_SCAN_AUTO_RESOLUTION
				reso = RESOLUTION_CLAUTO;
#else	/* USE_SCAN_AUTO_RESOLUTION */
				reso = RESOLUTION_SFINE;
#endif	/* USE_SCAN_AUTO_RESOLUTION */
				break;
			default:
				reso = RESOLUTION_STD;
				break;
		}
	}
	
#ifdef USE_SEPARATE_UI
	/* ここでScan2E-Mail Server用の設定TMP領域保存を行う */
	/* 丸められたResolution設定を元に、UI画面表示用のTMP設定をする */
	SetTmpdataIoJobEmsResolutionSetting( ColorMode, reso );
#endif	/* USE_SEPARATE_UI */

	return reso;

}

#ifdef	USE_TMP_SCANSIZE
/*********************************************************************************************/
/**
* @par		(serio)E-Mail用のスキャンサイズの取得
* @param	iojob(input)Scan実行のIOJOB
* @retval	スキャンサイズ(SCANSIZE_A4 / SCANSIZE_LETTER / SCANSIZE_LEGAL / SCANSIZE_OTHER)
* @par	<外部仕様>
*			E-Mail用のスキャンサイズの取得
* @par	<内部仕様>
* 			Scan実行のIOJOBのDocSizeより、E-Mail用のスキャンサイズを取得する。
*			Scan実行のIOJOBにDocSizeが設定されていない場合は、
*			FUNC_TMP_SEMAIL_SCANSIZE の設定値を取得する。
*/
/*********************************************************************************************/
STATIC	UINT8
GetScanSize_EMail(SERIO_CNP_IOJOB_SCANSEND_T *iojob)
{
	UINT8 ret;
	
	if( (iojob != NULL) && (iojob->DocSize != NULL) )
	{
		/* IOJOBのDocSizeより、スキャンサイズを求める */
		switch(*(iojob->DocSize))
		{
			case ns__ScanAndUloadScansize__A4:
				ret = SCANSIZE_A4;
				break;
			case ns__ScanAndUloadScansize__Letter:
				ret = SCANSIZE_LETTER;
				break;
			case ns__ScanAndUloadScansize__Legal:
				ret = SCANSIZE_LEGAL;
				break;
			default:
				ret = SCANSIZE_OTHER;
				break;
		}
	}
	else
	{
		/* IOJOBに未設定の場合は、設定値を取得する */
		ret = fstget_data(FUNC_TMP_SEMAIL_SCANSIZE);
	}

	return ret;
}
#endif /* USE_TMP_SCANSIZE */

/*********************************************************************************************/
/**
* @par		(serio)E-Mail用の圧縮率の取得
* @param	iojob(input)Scan実行のIOJOB
* @retval	圧縮率 (FUNC_COMPRES_RATE_LOW / FUNC_COMPRES_RATE_MID / FUNC_COMPRES_RATE_HI)
* @par	<外部仕様>
*			E-Mail用の圧縮率の取得
* @par	<内部仕様>
* 			・Scan実行のIOJOBのJpgQuality(画質)が設定されている場合は、
*			  JpgQualityより、E-Mail用の圧縮率を取得する。
*			    高画質 -> 低圧縮
*			    中画質 -> 中圧縮
*			    低画質 -> 高圧縮
*			・Scan実行のIOJOBにJpgQuality(画質)が設定されていない場合は、
*			  FUNC_COMPRES_RATE_MID を返す
*/
/*********************************************************************************************/
STATIC	UINT8
GetCompRate_EMail(SERIO_CNP_IOJOB_SCANSEND_T *iojob)
{
	UINT8 ret;

	ret = FUNC_COMPRES_RATE_MID;	/* IO JOB未設定時の値 */
	
	if( iojob->JpgQuality != NULL )
	{
		/* IOJOBのJpgQuality(画質)より、圧縮率を求める */
		switch(*(iojob->JpgQuality))
		{
			case ns__Selection3__High:
				/* 高画質 -> 低圧縮 */
				ret = FUNC_COMPRES_RATE_LOW;
				break;
			case ns__Selection3__Normal:
			default:
				/* 中画質 -> 中圧縮 */
				ret = FUNC_COMPRES_RATE_MID;
				break;
			case ns__Selection3__Low:
				/* 低画質 -> 高圧縮 */
				ret = FUNC_COMPRES_RATE_HI;
				break;
		}
	}

	return ret;
}

/*********************************************************************************************/
/**
* @par		(serio)指定された解像度がカラーかの判断
* @param	Resolution(input) 解像度
* @retval	TRUE  : カラー
* 			FALSE : カラーではない
* @par	<外部仕様>
*			指定された解像度がカラーかの判断
* @par	<内部仕様>
* 			解像度からカラーかそうでないかの判断を行う
*			※モデル毎にメンテの必要あり<Modelによって定義が無い設定があり、Modelで判別できるCSWもないため>
*/
/*********************************************************************************************/
STATIC  BOOL
IsColor(SCAN2FTP_QUALITY quality)
{
	BOOL 	ret;

	ret = FALSE;
	switch(quality){
		case FTP_Color100:
		case FTP_Color200:
		case FTP_Color300:
		case FTP_Color400:
		case FTP_Color600:
#ifdef	USE_SCAN_AUTO_RESOLUTION
		case FTP_ColorAuto:
#endif	/* USE_SCAN_AUTO_RESOLUTION */
			ret = TRUE;
			break;
		default:
			ret = FALSE;
			break;
	}

	return ret;
}

/*********************************************************************************************/
/**
* @par		(serio)指定された解像度がグレーかの判断
* @param	Resolution(input) 解像度
* @retval	TRUE  : グレー
* 			FALSE : グレーではない
* @par	<外部仕様>
*			指定された解像度がグレーかの判断
* @par	<内部仕様>
* 			解像度からグレーかそうでないかの判断を行う
*			※モデル毎にメンテの必要あり<Modelによって定義が無い設定があり、Modelで判別できるCSWもないため>
*/
/*********************************************************************************************/
STATIC	BOOL
IsGray(SCAN2FTP_QUALITY quality)
{
	BOOL 	ret;

	switch(quality){
		case FTP_Gray100:
		case FTP_Gray200:
		case FTP_Gray300:
		case FTP_Gray400:
		case FTP_Gray600:
#ifdef	USE_SCAN_AUTO_RESOLUTION
		case FTP_GrayAuto:
#endif	/* USE_SCAN_AUTO_RESOLUTION */
			ret = TRUE;
			break;
		default:
			ret = FALSE;
			break;
	}

	return ret;
}

/*********************************************************************************************/
/**
* @par		(serio)指定された解像度がモノクロかの判断
* @param	Resolution(input) 解像度
* @retval	TRUE  : モノクロ
* 			FALSE : モノクロではない
* @par	<外部仕様>
*			指定された解像度がモノクロかの判断
* @par	<内部仕様>
* 			解像度からモノクロかそうでないかの判断を行う
*			※モデル毎にメンテの必要あり<Modelによって定義が無い設定があり、Modelで判別できるCSWもないため>
*/
/*********************************************************************************************/
STATIC	BOOL
IsMono(SCAN2FTP_QUALITY quality)
{
	BOOL 	ret;

	switch(quality){
		case FTP_BW200:
		case FTP_BW100:
		case FTP_BW300:
			ret = TRUE;
			break;
		default:
			ret = FALSE;
			break;
	}

	return ret;
}

/*********************************************************************************************/
/**
* @par		(serio)アプリケーション予約を行う
* @param	Resolution(input) 解像度
* @retval	pCurrent : ジョブ情報
* @par	<外部仕様>
*			アプリケーション予約を行う
* @par	<内部仕様>
* 			ジョブ情報、両面情報からアプリケーション予約を行う
*/
/*********************************************************************************************/
STATIC	INT32
GetsysmApliRsv(current_t *pCurrent)
{
	INT32			rsv_id;

#ifdef	COLOR_NETWORK_SCANNER
	if (pCurrent->color_flag == COLORFAX_ON)
	{
#ifdef	DOUBLE_DEVICE_DUPLEX_SCAN
		/* Colorの場合のアプリケーション予約を行う */
		rsv_id = GetsysmApliRsv_Color(pCurrent);
#else	/* DOUBLE_DEVICE_DUPLEX_SCAN */
#ifdef	USE_DUPLEX_SCAN
		if((fstget_data(FSW_DUPLEX_SCAN_FUNC) == FSW_DUPLEX_SCAN_FUNC_ON)
				&& (fstget_data(FUNC_TMP_DUALSCANSETTING) == FUNC_DUALSCAN_DXLONG))
		{
			/* カラー両面読取反転 */
			if(pCurrent->comp_rate == FUNC_COMPRES_RATE_LOW){
				DPRINTF(("format:JPEG file size:LARGE\n"));
				DPRINTF(("reso:%d\n",pCurrent->resolution[0]));
				switch(pCurrent->resolution[0])
				{
					case	SCAN2_RESO_CL600DPI:
						if(rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_LARGE) != ERROR){
							DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_LARGE) = %d\n", rsv_id));
							break;
						}
						DPRINTF(("Large mem cannot get\n"));
					case	SCAN2_RESO_CL300DPI:
						if(rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_MID) != ERROR){
							DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_MID) = %d\n", rsv_id));
							break;
						}
						DPRINTF(("Middle mem cannot get\n"));
					default:
						rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE);
						DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE) = %d\n", rsv_id));
						break;
				}
			}else{
				rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE);
				DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE) = %d\n", rsv_id));
			}
		}
		else
		{
			rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR);
			DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR) = %d\n", rsv_id));
		}
#else
		rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR);
		DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR) = %d\n", rsv_id));
#endif
#endif	/* DOUBLE_DEVICE_DUPLEX_SCAN */
	}
	else if (pCurrent->gray_scale == USW_SCAN_MULTI)
	{
#ifdef	DOUBLE_DEVICE_DUPLEX_SCAN
		/* Grayの場合のアプリケーション予約を行う */
		rsv_id = GetsysmApliRsv_Gray(pCurrent);
#else	/* DOUBLE_DEVICE_DUPLEX_SCAN */
#ifdef	USE_DUPLEX_SCAN
		if((fstget_data(FSW_DUPLEX_SCAN_FUNC) == FSW_DUPLEX_SCAN_FUNC_ON)
				&& (fstget_data(FUNC_TMP_DUALSCANSETTING) == FUNC_DUALSCAN_DXLONG))
		{
			/* Gray両面読取反転 */
			if(pCurrent->comp_rate == FUNC_COMPRES_RATE_LOW){
				DPRINTF(("GRAY format:JPEG file size:LARGE\n"));
				DPRINTF(("reso:%d\n",pCurrent->resolution[0]));
				switch(pCurrent->resolution[0])
				{
					case	RESOLUTION_SFINE:
						if(rsv_id = sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE_MID) != ERROR){
							DPRINTF(("sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE_MID) = %d\n", rsv_id));
							break;
						}
						DPRINTF(("Middle mem cannot get\n"));
					default:
						rsv_id = sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE);
						DPRINTF(("sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE) = %d\n", rsv_id));
						break;
				}
			}else{
				rsv_id = sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE);
				DPRINTF(("sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE) = %d\n", rsv_id));
			}
		}
		else
		{
			rsv_id = sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY);
			DPRINTF(("sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY) = %d\n", rsv_id));
		}
#else
		rsv_id = sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY);
		DPRINTF(("sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY) = %d\n", rsv_id));
#endif
#endif	/* DOUBLE_DEVICE_DUPLEX_SCAN */
	}
	else
#endif
	{
#ifdef	DOUBLE_DEVICE_DUPLEX_SCAN
		/* Monoの場合のアプリケーション予約を行う */
		rsv_id = GetsysmApliRsv_Mono(pCurrent);
#else	/* DOUBLE_DEVICE_DUPLEX_SCAN */
#ifdef	USE_DUPLEX_SCAN
		if((fstget_data(FSW_DUPLEX_SCAN_FUNC) == FSW_DUPLEX_SCAN_FUNC_ON)
				&& (fstget_data(FUNC_TMP_DUALSCANSETTING) == FUNC_DUALSCAN_DXLONG))
		{
			/* モノ両面読取反転 */
			rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_MONO_DUPLEX_ROTATE);
			DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_MONO_DUPLEX_ROTATE) = %d\n", rsv_id));
		}
		else
		{
			rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_MONO);
			DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_MONO) = %d\n", rsv_id));
		}
#else
		rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_MONO);
		DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_MONO) = %d\n", rsv_id));
#endif
#endif	/* DOUBLE_DEVICE_DUPLEX_SCAN */

	}

	return rsv_id;
}

#ifdef	DOUBLE_DEVICE_DUPLEX_SCAN
/*********************************************************************************************/
/**
* @par		(serio)ColorModeがColorの場合のアプリケーション予約を行う
* @param	pCurrent(input) job設定情報
* @retval	rsv_id : 予約番号
*					 ERROR - 入力情報orメモリフルor同時動作管理部で予約拒否
* @par	<外部仕様>
*			ColorModeがColorの場合を前提とした、アプリケーション予約を行う
* @par	<内部仕様>
* 			ジョブ情報、両面情報からアプリケーション予約を行う
*
* ※ base : cp_fax_lib.c memr_current_set()
*/
/*********************************************************************************************/
STATIC	INT32
GetsysmApliRsv_Color(current_t *pCurrent)
{
	INT32			rsv_id;
#ifdef	USE_SCAN_AUTO_RESOLUTION
	UINT16			auto_resolution;
#endif 	/* USE_SCAN_AUTO_RESOLUTION */
	rsv_id = ERROR;

	if(pCurrent == NULL) {
		EPRINTF(("GetsysmApliRsv_Color() Error : pCurrent = NULL\n"));
		return ERROR;
	}
	if(pCurrent->color_flag != COLORFAX_ON) {
		/* Colorでなければエラーリターンする */
		EPRINTF(("GetsysmApliRsv_Color() Error : not Color\n"));
		return ERROR;
	}

	if((fstget_data(FSW_DUPLEX_SCAN_FUNC) == FSW_DUPLEX_SCAN_FUNC_ON)
		&& ((fstget_data(FUNC_TMP_DUALSCANSETTING) == FUNC_DUALSCAN_DXLONG)
			|| (fstget_data(FUNC_TMP_DUALSCANSETTING) == FUNC_DUALSCAN_DXSHORT)))
	{
		/* カラー両面読取反転 */
		if(pCurrent->comp_rate == FUNC_COMPRES_RATE_LOW)
		{
			DPRINTF(("Color format:JPEG file size:LARGE\n"));
			DPRINTF(("reso:%d\n", pCurrent->resolution[0]));
			switch(pCurrent->resolution[0])
			{
#ifdef	USE_SCAN_AUTO_RESOLUTION
				case	RESOLUTION_CLAUTO:
					/* 読取自動解像度設定の場合、解像度を取得する */
					if(scanmenu_EiScanGetResolution(SCAN_APP_EMAIL, pCurrent->resolution[0],
					                                pCurrent->file_format, P_SCAN_QUAL_HIGH, &auto_resolution) == OK)
					{
						switch(auto_resolution){
							case	QUALITY_COLOR_600DPI:
								if((rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_LARGE)) != ERROR){
									DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_LARGE) = %d\n", rsv_id));
									break;
								}
							case	QUALITY_COLOR_400DPI:
							case	QUALITY_COLOR_300DPI:
								if((rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_MID)) != ERROR){
									DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_MID) = %d\n", rsv_id));
									break;
								}
							default:
								rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE);
								DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE) = %d\n", rsv_id));
								break;
						}
					}
					break;
#endif	/* USE_SCAN_AUTO_RESOLUTION */	
				case	RESOLUTION_PHOTO:
					if((rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_LARGE)) != ERROR){
						DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_LARGE) = %d\n", rsv_id));
						break;
					}
					DPRINTF(("Large mem cannot get\n"));
				case	RESOLUTION_SFINE:
					if((rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_MID)) != ERROR){
						DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_MID) = %d\n", rsv_id));
						break;
					}
					DPRINTF(("Middle mem cannot get\n"));
				case	RESOLUTION_FINE:
				case	RESOLUTION_STD:
				default:
					rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE);
					DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE) = %d\n", rsv_id));
					break;
			}
		}
		else if(pCurrent->comp_rate == FUNC_COMPRES_RATE_MID)
		{
			DPRINTF(("Color format:JPEG file size:MID\n"));
			DPRINTF(("reso:%d\n",pCurrent->resolution[0]));
			switch(pCurrent->resolution[0])
			{
#ifdef	USE_SCAN_AUTO_RESOLUTION
				case	RESOLUTION_CLAUTO:
					/* 読取自動解像度設定の場合、解像度を取得する */
					if(scanmenu_EiScanGetResolution(SCAN_APP_EMAIL, pCurrent->resolution[0],
					                                pCurrent->file_format, P_SCAN_QUAL_MID, &auto_resolution) == OK)
					{
						switch(auto_resolution){
							case	QUALITY_COLOR_600DPI:
								if((rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_MID)) != ERROR){
									DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_MID) = %d\n", rsv_id));
									break;
								}
							case	QUALITY_COLOR_400DPI:
							case	QUALITY_COLOR_300DPI:
							default:
								rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE);
								DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE) = %d\n", rsv_id));
								break;
						}
					}
					DPRINTF(("auto resolution setting cannot get\n"));
					break;
#endif	/* USE_SCAN_AUTO_RESOLUTION */	
				case	RESOLUTION_PHOTO:
					if((rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_MID)) != ERROR){
						DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE_MID) = %d\n", rsv_id));
						break;
					}
					DPRINTF(("Large mem cannot get\n"));
				case	RESOLUTION_SFINE:
				case	RESOLUTION_FINE:
				case	RESOLUTION_STD:
				default:
					rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE);
					DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE) = %d\n", rsv_id));
					break;
			}
		}
		else{
			rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE);
			DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_COLOR_DUPLEX_ROTATE) = %d\n", rsv_id));
		}
	}
	else
	{
		rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_COLOR);
		DPRINTF(("else sysmApliRsv(SYSM_AP_FAX_READ_COLOR) = %d\n", rsv_id));
	}

	return rsv_id;
}

/*********************************************************************************************/
/**
* @par		(serio)ColorModeがGrayの場合のアプリケーション予約を行う
* @param	pCurrent(input) job設定情報
* @retval	rsv_id : 予約番号
*					 ERROR - 入力情報orメモリフルor同時動作管理部で予約拒否
* @par	<外部仕様>
*			ColorModeがGrayの場合を前提とした、アプリケーション予約を行う
* @par	<内部仕様>
* 			ジョブ情報、両面情報からアプリケーション予約を行う
* ※ base : cp_fax_lib.c memr_current_set()
*/
/*********************************************************************************************/
STATIC	INT32
GetsysmApliRsv_Gray(current_t *pCurrent)
{
	INT32			rsv_id;
#ifdef	USE_SCAN_AUTO_RESOLUTION
	UINT16			auto_resolution;
#endif 	/* USE_SCAN_AUTO_RESOLUTION */
	rsv_id = ERROR;

	if(pCurrent == NULL) {
		EPRINTF(("GetsysmApliRsv_Gray() Error : pCurrent = NULL\n"));
		return ERROR;
	}
	if( (pCurrent->color_flag != COLORFAX_OFF) ||
	    (pCurrent->gray_scale != USW_SCAN_MULTI) )
	{
		/* Grayでなければエラーリターンする */
		EPRINTF(("GetsysmApliRsv_Gray() Error : not Gray\n"));
		return ERROR;
	}

	if((fstget_data(FSW_DUPLEX_SCAN_FUNC) == FSW_DUPLEX_SCAN_FUNC_ON)
		&& ((fstget_data(FUNC_TMP_DUALSCANSETTING) == FUNC_DUALSCAN_DXLONG)
			|| (fstget_data(FUNC_TMP_DUALSCANSETTING) == FUNC_DUALSCAN_DXSHORT)))
	{
		/* カラー両面読取反転 */
		if(pCurrent->comp_rate == FUNC_COMPRES_RATE_LOW)
		{
			DPRINTF(("GRAY format:JPEG file size:LARGE\n"));
			DPRINTF(("reso:%d\n",pCurrent->resolution[0]));
			switch(pCurrent->resolution[0])
			{
#ifdef	USE_SCAN_AUTO_RESOLUTION
			case	RESOLUTION_GRAUTO:
				/* 読取自動解像度設定の場合、解像度を取得する */
				if(scanmenu_EiScanGetResolution(SCAN_APP_EMAIL, pCurrent->resolution[0],
				                                pCurrent->file_format, P_SCAN_QUAL_HIGH, &auto_resolution) == OK)
				{
					switch(auto_resolution){
						case	QUALITY_GRAY_600DPI:
							if((rsv_id = sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE_LARGE)) != ERROR){
								DPRINTF(("sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE_LARGE) = %d\n", rsv_id));
								break;
							}
						case	QUALITY_GRAY_400DPI:
						case	QUALITY_GRAY_300DPI:
							if((rsv_id = sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE_MID)) != ERROR){
								DPRINTF(("sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE_MID) = %d\n", rsv_id));
								break;
							}
						default:
							rsv_id = sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE);
							DPRINTF(("sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE) = %d\n", rsv_id));
							break;
					}
				}
				break;
#endif	/* USE_SCAN_AUTO_RESOLUTION */	
			case	RESOLUTION_SFINE:
				if((rsv_id = sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE_MID)) != ERROR){
					DPRINTF(("sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE_MID) = %d\n", rsv_id));
					break;
				}
				DPRINTF(("Middle mem cannot get\n"));
			case	RESOLUTION_FINE:
			case	RESOLUTION_STD:
			default:
				rsv_id = sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE);
				DPRINTF(("sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE) = %d\n", rsv_id));
				break;
			}
		}else{
			rsv_id = sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE);
			DPRINTF(("sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY_DUPLEX_ROTATE) = %d\n", rsv_id));
		}
	}
	else
	{
		rsv_id = sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY);
		DPRINTF(("else sysmApliRsv(SYSM_AP_SCANTO_READ_GRAY) = %d\n", rsv_id));
	}

	return rsv_id;
}

/*********************************************************************************************/
/**
* @par		(serio)ColorModeがMonoの場合のアプリケーション予約を行う
* @param	pCurrent(input) job設定情報
* @retval	rsv_id : 予約番号
*					 ERROR - 入力情報orメモリフルor同時動作管理部で予約拒否
* @par	<外部仕様>
*			ColorModeがGrayの場合を前提とした、アプリケーション予約を行う
* @par	<内部仕様>
* 			ジョブ情報、両面情報からアプリケーション予約を行う
* ※ base : cp_fax_lib.c memr_current_set()
*/
/*********************************************************************************************/
STATIC	INT32
GetsysmApliRsv_Mono(current_t *pCurrent)
{
	INT32			rsv_id;

	if(pCurrent == NULL) {
		EPRINTF(("GetsysmApliRsv_Mono() Error : pCurrent = NULL\n"));
		return ERROR;
	}
	if( (pCurrent->color_flag != COLORFAX_OFF) ||
	    (pCurrent->gray_scale != USW_SCAN_2LEVEL) )
	{
		/* Monoでなければエラーリターンする */
		EPRINTF(("GetsysmApliRsv_Mono() Error : not Mono\n"));
		return ERROR;
	}

	if((fstget_data(FSW_DUPLEX_SCAN_FUNC) == FSW_DUPLEX_SCAN_FUNC_ON)
		&& ((fstget_data(FUNC_TMP_DUALSCANSETTING) == FUNC_DUALSCAN_DXLONG)
		 || (fstget_data(FUNC_TMP_DUALSCANSETTING) == FUNC_DUALSCAN_DXSHORT)))
	{
		/* モノ両面読取反転 */
		rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_MONO_DUPLEX_ROTATE);
		DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_MONO_DUPLEX_ROTATE) = %d\n", rsv_id));
	}
	else
	{
		rsv_id = sysmApliRsv(SYSM_AP_FAX_READ_MONO);
		DPRINTF(("sysmApliRsv(SYSM_AP_FAX_READ_MONO) = %d\n", rsv_id));
	}

	return rsv_id;
}
#endif	/* DOUBLE_DEVICE_DUPLEX_SCAN */

STATIC	UINT8 
GetColorFlag(enum ns__ColorModeSelection ColorMode)
{
	UINT8		color_flag;

	if(ColorMode == ns__ColorModeSelection__Color){
		color_flag = COLORFAX_ON;
	}else{
		color_flag = COLORFAX_OFF;
	}

	return color_flag;
}

UINT8 GetGrayScale(enum ns__ColorModeSelection ColorMode)
{
	UINT8	gray_scale;

	if(ColorMode == ns__ColorModeSelection__Color || ColorMode == ns__ColorModeSelection__Gray){
		gray_scale = USW_SCAN_MULTI;
	}else{
		gray_scale = USW_SCAN_2LEVEL;
	}

	return gray_scale;
}



/*********************************************************************************************/
/**
* @par		(serio)ファイルフォーマットから拡張子を取得する
* @param	extension(output) 拡張子文字列
* @param	fileformat(input) ファイルフォーマット
* @retval	なし
* @par	<外部仕様>
*			ファイルフォーマットから拡張子を取得する
* @par	<内部仕様>
* 			ファイルフォーマットから拡張子を取得する
*/
/*********************************************************************************************/
STATIC	VOID
get_filename(UINT8 *Filename, size_t size, UINT8 *InputFilename, SCAN2FTP_FILEFORMAT fileformat, UINT8 FileNameFixed)
{
	UINT32			scan_count;											/* Scan to FTPのファイル名に利用する連番 */
	UINT8			extension[ EXTENSION_SIZE + 1 ];
	UINT32			StrLen;


	/* ファイルフォーマットの文字列を取得する */
	get_extension_str(extension, fileformat);
	
	if ( FALSE ==  FileNameFixed)
	{
		/* ADFとFBの読取連続番号を取得する */
		scan_counter(&scan_count);

		/* ファイル名に追加する"_" + 読み取り連増番号 + "." + 拡張子の長さ */
		StrLen = 1 + 6 + 1 + strlen((MD_CHAR *)extension);

		memset(Filename, 0, size);

		strncpy((MD_CHAR *)Filename, (MD_CHAR *)InputFilename, (size - StrLen - 1));
		Filename += strlen((MD_CHAR *)Filename);

		sprintf( (MD_CHAR *)Filename, "_%06d.%s", scan_count, extension );
	}
	else
	{
		sprintf( (MD_CHAR *)Filename, "%s.%s", InputFilename, extension );
	}
	
}


/*********************************************************************************************/
/**
* @par		(serio)ファイルフォーマットから拡張子を取得する
* @param	extension(output) 拡張子文字列
* @param	fileformat(input) ファイルフォーマット
* @retval	なし
* @par	<外部仕様>
*			ファイルフォーマットから拡張子を取得する
* @par	<内部仕様>
* 			ファイルフォーマットから拡張子を取得する
*/
/*********************************************************************************************/
STATIC	VOID
get_extension_str(UINT8 *extension, SCAN2FTP_FILEFORMAT fileformat)
{
	switch(fileformat){
	case FTP_PDFA:
	case FTP_SIPDF:		
	case FTP_PDF:
	case FTP_SPDF:
		strcpy( (MD_CHAR *)extension, "pdf" );
		break;
	case FTP_JPEG:
		strcpy( (MD_CHAR *)extension, "jpg" );
		break;
	case FTP_Tiff:
		strcpy( (MD_CHAR *)extension, "tif" );
		break;
	case FTP_XPS:
		strcpy( (MD_CHAR *)extension, "xps" );
		break;
	}
}


/*********************************************************************************************/
/**
* @par		(serio)読み取り拒否を確認
* @param	extension(output) 拡張子文字列
* @param	fileformat(input) ファイルフォーマット
* @retval	TRUE  : 読み取り拒否
* 			FALSE : 読み取り可能
* @par	<外部仕様>
*			読み取り拒否を確認する
* @par	<内部仕様>
* 			排紙トレイの用紙有無、ADFカバーOPENを確認する
*/
/*********************************************************************************************/
STATIC	BOOL
serio_IsDischargeTray( VOID )
{
	BOOL	blReturn = FALSE;

#ifdef	USE_ADFHAISHI_SENSOR
	if ( MacStateRef(STID_ADFSCN_DISABLE) == MAC_ON ) {
		blReturn = TRUE;
	}
#endif	/* USE_ADFHAISHI_SENSOR */

#ifdef	USE_ADFCOVER_SENSOR
	/* ADFカバーOPENを確認する */
	if ( ( MacStateRef(STID_ADFCVR_OPN_STOPWAIT) == MAC_ON )
	||   ( MacStateRef(STID_ADFCVR_OPN) == MAC_ON ) ){
		blReturn = TRUE;
	}
#endif	/* USE_ADFCOVER_SENSOR */

	return ( blReturn );
}

/****************************************************************************/
/*	check_broad : 同報キー入力をチェックする								*/
/*																			*/
/*	入力 : 無し																*/
/*	出力 : DS_XXXX															*/
/*																			*/
/****************************************************************************/
STATIC INT32
check_broad(VOID)
{
	/* INTERRUPTモードならダイヤル入力拒否 */
	if (MacStateRef(STID_INTERRUPT) == MAC_ON) {
		DPRINTF(("DS_NG:interrupt ON\n"));
		return ERROR;
	}
#ifdef	USE_FAXTRN_FUNC
	/* FAX転送/リスト転送設定有りなら入力拒否 */
	if (fstget_data(FUNC_TMP_TRANSFER) == FAX_TRANSFER_ON
			|| fstget_data(FUNC_TMP_TRANSFER) == REPORT_TRANSFER_ON) {
		return ERROR;
	}
#endif
	/* ポーリング発呼設定有りなら原稿は見ない */
//	if (fstget_data(FUNC_POLLING_RCV) == POLLING_RCV_OFF) {
//		/* 送信可能原稿をチェックする */
//		if (doc_state == DOC_STATE_NG) {
//			DPRINTF(("DS_NG:doc NG\n"));
//			return ERROR;
//		}
//	}
	/* 発呼本数FULLをチェックする */
	if ( jobGetNumber() >= (MEM_JOB_MAX) ) {
		DPRINTF(("DS_NG:mem_job NG\n"));
		return ERROR;
	}
	if (fstget_data(FUNC_TIMER_TX) != TIMER_TX_OFF) {
		/* タイマ同報送信がすでにあるか調べる */
		if (jobChkKind(JOB_TMMLTTX) == TRUE) {
			return ERROR;
		}
		/* タイマ順次ポーリングがすでにあるか調べる */
		if (jobChkKind(JOB_TMMLPOLLRX) == TRUE) {
			return ERROR;
		}
	}
#ifdef USE_FAX2NET
	/* FAX2NET Web Retrieval は入力拒否 */
	if (fstget_data(FUNC_F2N_WEBRET) == F2N_WEBRET_ON) {
		return ERROR;
	}
#endif
#ifdef	COLOR_FAX_FUNC
#ifdef USE_FAX2NET
	/* カラーのFAX2NET同報送信は許可する */
	if (fstget_data(FUNC_F2N_FAXTOMAIL) != F2N_FAXTOMAIL_ON
		&& fstget_data(FUNC_F2N_FAXOVERIP) != F2N_FAXOVERIP_ON) {
#endif
#if	defined(USE_OPT_LAN)&&defined(COLOR_NETWORK_SCANNER)
		/* カラーE-Mailの同報は許可する */
		if(fstget_data(FUNC_NETWORK_SCANNER) == NET_SCANNER_OFF){
#endif	/* USE_OPT_LAN  && COLOR_NETWORK_SCANNER*/
		if (fstget_data(FUNC_COLORFAX)==COLORFAX_ON) {
			/* カラー送信をチェックする */
			DPRINTF(("DS_NG:Resolution=Color NG\n"));
			return ERROR;
		}
#if	defined(USE_OPT_LAN)&&defined(COLOR_NETWORK_SCANNER)
		}
#endif	/* USE_OPT_LAN  && COLOR_NETWORK_SCANNER*/
#ifdef USE_FAX2NET
	}
#endif
#else
#ifdef	USE_OPT_LAN
#ifdef	COLOR_NETWORK_SCANNER
#endif
#endif
#endif

	return OK;
}

#ifdef	USE_SEPARATE_UI
/*********************************************************************************************/
/**
* @par		(serio)Scan種別をTMP領域にセットする。<Scan共通>
* @param	ScanType(input) TMPに設定するScan種別情報
* @retval	なし
* @par	<外部仕様>
*			Scan種別をTMP領域にセットする。
* @par	<内部仕様>
* 			CP側仕様変更に対応。Scan種別を『TMP_SCAN_KIND』にCP経由で設定する。
*/
/*********************************************************************************************/
STATIC VOID 
SetIoJobScanType_toCP( UINT8 ScanType )
{
	ARG_PARA		param_inf;
	
	param_inf.kind = CPAPI_P_SCAN_KIND;
	param_inf.datatype = CPAPI_VAL;
	param_inf.str = NULL;
	param_inf.val = ScanType;
	param_inf.len = 0;
	
	cpApiCall(CPAPI_KIND_SET, CPAPI_SETID_PARA_CMN, (CPAPI_ARG_TBL *)&param_inf);
}

/*********************************************************************************************/
/**
* @par		(serio)ScanResolution設定をTMP領域にセットする。<Scan2E-Mail Server対応>
* @param	ScanType(input) TMPに設定するScan解像度及び指定Color情報
* @retval	なし
* @par	<外部仕様>
*			Scan種別をTMP領域にセットする。
* @par	<内部仕様>
* 			CP側仕様変更に対応。ScanResolution設定をTMP領域に設定する。
*			CP関数の仕様上、一旦Resolution情報を色情報と解像度情報に分割して設定する。
*/
/*********************************************************************************************/
STATIC VOID 
SetTmpdataIoJobEmsResolutionSetting(enum ns__ColorModeSelection ColorMode, UINT8 reso)
{
	INT32	color = CPAPI_P_VAL_SCAN_COLOR_TYPE_DEFAULT;
	INT32	resolution = CPAPI_P_VAL_SCAN_RESO_DEFAULT;
	
	if(ColorMode == ns__ColorModeSelection__Mono)
	{
		color = CPAPI_P_VAL_SCAN_COLOR_TYPE_BW;
		
		switch( reso )
		{
			case RESOLUTION_STD:
				resolution = CPAPI_P_VAL_SCAN_RESO_200X100;
				break;
			case RESOLUTION_FINE:
				resolution = CPAPI_P_VAL_SCAN_RESO_200;
				break;
			case RESOLUTION_SFINE:
				resolution = CPAPI_P_VAL_SCAN_RESO_300;
				break;
			default:
				break;
		}
	}
	else if(ColorMode == ns__ColorModeSelection__Gray)
	{
		color = CPAPI_P_VAL_SCAN_COLOR_TYPE_GRAY;
		
		switch( reso )
		{
			case RESOLUTION_STD:
				resolution = CPAPI_P_VAL_SCAN_RESO_100;
				break;
			case RESOLUTION_FINE:
				resolution = CPAPI_P_VAL_SCAN_RESO_200;
				break;
			case RESOLUTION_SFINE:
				resolution = CPAPI_P_VAL_SCAN_RESO_300;
				break;
#ifdef	USE_SCAN_AUTO_RESOLUTION
			case RESOLUTION_GRAUTO:
				resolution = CPAPI_P_VAL_SCAN_RESO_AUTO;
				break;
#endif	/* USE_SCAN_AUTO_RESOLUTION */
			default:
				break;
		}
	}
	else if(ColorMode == ns__ColorModeSelection__Color)
	{
		color = CPAPI_P_VAL_SCAN_COLOR_TYPE_COLOR;
		
		switch( reso )
		{
			case RESOLUTION_STD:
				resolution = CPAPI_P_VAL_SCAN_RESO_100;
				break;
			case RESOLUTION_FINE:
				resolution = CPAPI_P_VAL_SCAN_RESO_200;
				break;
			case RESOLUTION_SFINE:
				resolution = CPAPI_P_VAL_SCAN_RESO_300;
				break;
			case RESOLUTION_PHOTO:
				resolution = CPAPI_P_VAL_SCAN_RESO_600;
				break;
#ifdef	USE_SCAN_AUTO_RESOLUTION
			case RESOLUTION_CLAUTO:
				resolution = CPAPI_P_VAL_SCAN_RESO_AUTO;
				break;
#endif	/* USE_SCAN_AUTO_RESOLUTION */
			default:
				break;
		}
	}
	
	cp_SetTmpScanColorSetting( color, NULL, 0 );
	cp_SetTmpScanReso( resolution, NULL, 0 );
}

/*********************************************************************************************/
/**
* @par		(serio)Scan種別をTMP領域にセットする。<Scan2FTP限定>
* @param	ScanType(input) TMPに設定するScan種別情報
* @retval	なし
* @par	<外部仕様>
*			Scan種別をTMP領域にセットする。
* @par	<内部仕様>
* 			CP側仕様変更に対応。Panel側に伝えるべき情報ををTMP領域に設定する。
*/
/*********************************************************************************************/
STATIC VOID 
SetTmpdataIoJobFtpAccessinfo(ACCESS_INFO *AccessInfo)
{
	
	/* Resolution */
	SetTmpdataIoJobFtpCifsResolutionSetting( AccessInfo->quality );
	
	/* Duplex */
	SetTmpdataIoJobFtpCifsDuplexSetting( AccessInfo->dualscan );
	
	/* File Type */
	SetTmpdataIoJobFtpCifsFileType( AccessInfo->fileformat );
	
	/* File名 */
	SetTmpdataIoJobFtpCifsFileName( AccessInfo->filename );
	
}

/*********************************************************************************************/
/**
* @par		(serio)Scan種別をTMP領域にセットする。<Scan2NW限定>
* @param	ScanType(input) TMPに設定するScan種別情報
* @retval	なし
* @par	<外部仕様>
*			Scan種別をTMP領域にセットする。
* @par	<内部仕様>
* 			CP側仕様変更に対応。Panel側に伝えるべき情報ををTMP領域に設定する。
*/
/*********************************************************************************************/
STATIC VOID 
SetTmpdataIoJobCifsAccessinfo(CIFSACCESS_INFO *AccessInfo)
{
	
	/* Resolution */
	SetTmpdataIoJobFtpCifsResolutionSetting( AccessInfo->quality );
	
	/* Duplex */
	SetTmpdataIoJobFtpCifsDuplexSetting( AccessInfo->dualscan );
	
	/* File Type */
	SetTmpdataIoJobFtpCifsFileType( AccessInfo->fileformat );
	
	/* File名 */
	SetTmpdataIoJobFtpCifsFileName( AccessInfo->filename );
	
}

/*********************************************************************************************/
/**
* @par		(serio)Scan種別をTMP領域にセットする。<Scan2FTP/NW対応>
* @param	ScanType(input) TMPに設定するScan種別情報
* @retval	なし
* @par	<外部仕様>
*			Scan種別をTMP領域にセットする。
* @par	<内部仕様>
* 			CP側仕様変更に対応。Scan種別を『TMP_SCAN_KIND』にCP経由で設定する。
*/
/*********************************************************************************************/
STATIC VOID 
SetTmpdataIoJobFtpCifsDuplexSetting( SCAN2FTP_DUALSCAN duplex )
{
	INT32	duplex_mode;
	
	switch(duplex){
		case FTP_DUPLEXLONG:
			duplex_mode = CPAPI_P_VAL_SCAN_DUP_SCAN_LONG_EDGE;
			break;
		case FTP_DUPLEXSHORT:
			duplex_mode = CPAPI_P_VAL_SCAN_DUP_SCAN_SHORT_EDGE;
			break;
		case FTP_SIMPLEX:
			duplex_mode = CPAPI_P_VAL_SCAN_DUP_SCAN_OFF;
			break;
	}
	
	cp_SetTmpScanDupScan( duplex_mode, NULL, NULL );
}

/*********************************************************************************************/
/**
* @par		(serio)ScanResolution設定をTMP領域にセットする。<Scan2FTP/NW対応>
* @param	ScanType(input) TMPに設定するScan種別情報
* @retval	なし
* @par	<外部仕様>
*			Scan種別をTMP領域にセットする。
* @par	<内部仕様>
* 			CP側仕様変更に対応。ScanResolution設定をTMP領域に設定する。
*			CP関数の仕様上、一旦Resolution情報を色情報と解像度情報に分割して設定する。
*/
/*********************************************************************************************/
STATIC VOID 
SetTmpdataIoJobFtpCifsResolutionSetting( SCAN2FTP_QUALITY reso )
{
	UINT8	i;
	INT32	color = CPAPI_P_VAL_SCAN_COLOR_TYPE_DEFAULT;
	INT32	resolution = CPAPI_P_VAL_SCAN_RESO_DEFAULT;;
	
	for( i = 0; i < (sizeof(ScanResolution)/sizeof(ScanResolution[0])); i++ )
	{
		if( reso == ScanResolution[i].ftp_resolution )
		{
			color      = ScanResolution[i].color_type;
			resolution = ScanResolution[i].reso_type;
		}
	}
	
	cp_SetTmpScanColorSetting( color, NULL, 0 );
	cp_SetTmpScanReso( resolution, NULL, 0 );
}

/*********************************************************************************************/
/**
* @par		(serio)Scan FileタイプをTMP領域にセットする。<Scan2FTP/NW対応>
* @param	FileType(input) TMPに設定するScan Fileタイプ情報
* @retval	なし
* @par	<外部仕様>
*			Scan FileタイプをTMP領域にセットする。
* @par	<内部仕様>
* 			CP側仕様変更に対応。Scan FileタイプをTMP領域に設定する。
*/
/*********************************************************************************************/
STATIC VOID 
SetTmpdataIoJobFtpCifsFileType( SCAN2FTP_FILEFORMAT FileType )
{
	INT32 file_type;
	
	switch(FileType){
		case FTP_PDF:
			file_type = CPAPI_P_VAL_FILE_FORMAT_PDF;
			break;
		case FTP_Tiff:
			file_type = CPAPI_P_VAL_FILE_FORMAT_TIFF;
			break;
		case FTP_JPEG:
			file_type = CPAPI_P_VAL_FILE_FORMAT_JPEG;
			break;
		case FTP_XPS:
			file_type = CPAPI_P_VAL_FILE_FORMAT_XPS;
			break;
		case FTP_PDFA:
			file_type = CPAPI_P_VAL_FILE_FORMAT_PDFA;
			break;	
		case FTP_SPDF:
			file_type = CPAPI_P_VAL_FILE_FORMAT_SPDF;
			break;	
		case FTP_SIPDF:
			file_type = CPAPI_P_VAL_FILE_FORMAT_SIPDF;
			break;					
	}
	
	cp_SetTmpScanFiletype( file_type, 0, 0 );

}

/*********************************************************************************************/
/**
* @par		(serio)Scan File名をTMP領域にセットする。<Scan2FTP/NW対応>
* @param	FileType(input) TMPに設定するScan File名
* @retval	なし
* @par	<外部仕様>
*			Scan File名をTMP領域にセットする。
* @par	<内部仕様>
* 			CP側仕様変更に対応。Scan File名をTMP領域に設定する。
*/
/*********************************************************************************************/
STATIC VOID 
SetTmpdataIoJobFtpCifsFileName( UINT8* filename )
{
	INT32 mode = 0;
	UINT8 len  = 0;
	
	cp_SetTmpScanFileName(mode, filename, len);
	
}

#endif	/* USE_SEPARATE_UI */

/*********************************************************************************************/
/**
* @par		(serio)Scan to FTPの登録情報の設定
* @param	AccessInfo(input) FTP登録情報
* @retval	なし
* @par	<外部仕様>
*			Scan to FTPの登録情報を設定する
* @par	<内部仕様>
* 			通常のScan(FTP)実行のタイプの設定と、FTP登録情報の設定を行う
*/
/*********************************************************************************************/
VOID
SetIoJobFtpAccessinfo(ACCESS_INFO *AccessInfo)
{
	JobScanType = JOB_SCAN_TYPE_FTP;
	memcpy(&FtpAccessInfo_Scan, AccessInfo, sizeof(FtpAccessInfo_Scan));
}


/*********************************************************************************************/
/**
* @par		(serio)Scan to Networkの登録情報の設定
* @param	AccessInfo(input) Network登録情報
* @retval	なし
* @par	<外部仕様>
*			Scan to Networkの登録情報を設定する
* @par	<内部仕様>
* 			通常のScan(Network)実行のタイプの設定と、Network登録情報の設定を行う
*/
/*********************************************************************************************/
VOID
SetIoJobCifsAccessinfo(CIFSACCESS_INFO *AccessInfo)
{
	JobScanType = JOB_SCAN_TYPE_NETWORK;
	memcpy(&CifsAccessInfo_Scan, AccessInfo, sizeof(CifsAccessInfo_Scan));
}


/*********************************************************************************************/
/**
* @par		(serio)Scan to E-Mailの登録情報の設定
* @param	telid(input) 発呼電話番号ID
* @param	current(input) job設定情報
* @retval	なし
* @par	<外部仕様>
*			Scan to E-Mailの登録情報を設定する
* @par	<内部仕様>
* 			通常のScan(E-Mail)実行のタイプの設定と、発呼電話番号IDとJOB設定情報の設定を行う
*/
/*********************************************************************************************/
VOID
SetIoJobEMail(UINT32 telid, current_t *current)
{
	JobScanType = JOB_SCAN_TYPE_EMAIL;
#ifdef USE_FAX /* DCL対応 */
	TelId_Scan = telid;
#endif /* USE_FAX */
	memcpy(&Current_Scan, current, sizeof(Current_Scan));
}



/*********************************************************************************************/
/**
* @par		(serio)スキャン実行タイプの取得
* @param	なし
* @retval	スキャン実行タイプ
* @par	<外部仕様>
*			スキャン実行タイプの取得する
* @par	<内部仕様>
*			スキャン実行タイプの取得する
*/
/*********************************************************************************************/
STATIC  INT8
GetJobScanType()
{
	return JobScanType;
}


/*********************************************************************************************/
/**
* @par		(serio)スキャンモードからのスキャン実行時の設定をクリアする
* @param	なし
* @retval	なし
* @par	<外部仕様>
*			スキャンモードからのスキャン実行時の設定をクリアする
* @par	<内部仕様>
*			スキャンモードからのスキャン実行時の設定をクリアする
*/
/*********************************************************************************************/
GLOBAL VOID
ClearScanJobSetting()
{
	switch(JobScanType){
	case JOB_SCAN_TYPE_NONE:
		/* 何もしない */
		break;
	case JOB_SCAN_TYPE_FTP:
		memset(&FtpAccessInfo_Scan, 0, sizeof(FtpAccessInfo_Scan));
		break;
	case JOB_SCAN_TYPE_NETWORK:
		memset(&CifsAccessInfo_Scan, 0, sizeof(CifsAccessInfo_Scan));
		break;
	case JOB_SCAN_TYPE_EMAIL:
#ifdef USE_FAX /* DCL対応 */
		telDelDialNum(TelId_Scan);		/* メールアドレス登録情報削除 */
#endif /* USE_FAX */
		memset(&Current_Scan, 0, sizeof(Current_Scan));
		break;
	}

	JobScanType = JOB_SCAN_TYPE_NONE;
}


/*********************************************************************************************/
/**
* @par		(serio)Scan実行タイプを取得
* @param	iojob(input)Scan実行のIOJOB
* @retval	ERROR		: 失敗
* 			ERROR以外	: 実行タイプ
* @par	<外部仕様>
*			Scanの実行タイプを取得する
* @par	<内部仕様>
* 			TxProfilesが存在すればそのタイプを返す、存在しなければScan実行時設定したタイプで \n
* 			判断し、結果を返す
*/
/*********************************************************************************************/
STATIC	INT32
GetTxProfileType(SERIO_CNP_IOJOB_SCANSEND_T *iojob)
{
	INT32	ret = ERROR;

#if 0 /**  (2010-08-06T12:03:57+09:00 kazushige.muroi) **/
	/*------------------------------------------------------------------------
	 * old
	 *----------------------------------------------------------------------*/
	if(iojob->TxProfiles != NULL){
		/* TxProfilesが存在すればそのままそのタイプを返す */
		ret = iojob->TxProfiles->__TxProfiles->__union;
	}else{
		/* Job実行時はTxProfilesが存在しないときがあるので、 */
		/* Scanモードで実行した時に設定するタイプで判断する */
		switch(GetJobScanType()){
			case JOB_SCAN_TYPE_FTP: 
				ret = SOAP_UNION_ns__TxProfile_Ftp;
				break;
			case JOB_SCAN_TYPE_NETWORK:
				ret = SOAP_UNION_ns__TxProfile_Cifs;
				break;
			case JOB_SCAN_TYPE_EMAIL:
				ret = SOAP_UNION_ns__TxProfile_Smtp;
				break;
			case JOB_SCAN_TYPE_NONE:	/* breakなし */
			default:
				ret = ERROR;
				break;
		}
	}
#else  /**  (2010-08-06T12:03:57+09:00 kazushige.muroi) **/
	if (iojob->TxProfiles.__TxProfiles != NULL){
		ret = iojob->TxProfiles.__TxProfiles[0].__union;
	}
#endif /**  (2010-08-06T12:03:57+09:00 kazushige.muroi) **/

	return ret;
}

STATIC UINT16
getNumProfiles(SERIO_CNP_IOJOB_SCANSEND_T *iojob)
{
	return (UINT16)iojob->TxProfiles.__size__TxProfiles;
}

STATIC	VOID
DeleteEntry(INT32 entry_id)
{
	/* 登録電話番号を削除する */
	if (entry_id != 0) {
		if (telBroadBack(entry_id) != BACK_ALL) {
			telBroadBack(entry_id);
		}
	}
}

/*********************************************************************************************/
/**
* @par		(serio)メモリ読込時のジョブ番号問い合わせ
* @param	なし
* @retval	ジョブ番号
* @par	<外部仕様>
*			メモリ読込時のジョブ番号を返す
* @par	<内部仕様>
*			メモリ読込時のジョブ番号を返す
*/
/*********************************************************************************************/
GLOBAL INT32
GetSerioMemIoJobNo(VOID)
{
	return s_job_id;
}

#ifdef SEND_SCAN_DEBUG_PRINT
STATIC 	VOID 
debug_print_email(current_t *current, INT32 rsv_id)
{
	UINT8 docmem_list[][20]      = {"CRNT_DM_NA", "CRNT_DM_DOCUMENT", "CRNT_DM_MEMIN", "CRNT_DM_MEMIN_DEL", "CRNT_DM_MEM", "CRNT_DM_MEM_DEL"};
	UINT8 resolution_list[][20]  = {"RESOLUTION_STD", "RESOLUTION_FINE", "RESOLUTION_SFINE", "RESOLUTION_PHOTO", "RESOLUTION_COLOR"};
	UINT8 resolution_list2[][20]  = {"SCAN2_RESO_CL100DPI", "SCAN2_RESO_CL200DPI", "SCAN2_RESO_CL300DPI", "SCAN2_RESO_CL600DPI"};
	UINT8 gray_scale_list[][20]  = {"USW_SCAN_2LEVEL",  "USW_SCAN_MULTI"};
	UINT8 file_format_list[][20] = {"FILE_FORMAT_PDF", "FILE_FORMAT_JPEG", "FILE_FORMAT_TIFF", "FILE_FORMAT_XPS", "FILE_FORMAT_SPDF"};
	UINT8 color_flag_list[][20]  = {"COLORFAX_OFF", "COLORFAX_ON"};
#ifdef FB_SCAN_TYPE
	UINT8 scan_src_list[][20]    = {"ADF_SCAN", "FB_SCAN"};
#endif /* FB_SCAN_TYPE */
	UINT8 duplex_list[][30]      = { "FUNC_DUALSCAN_SIMPLEX", "FUNC_DUALSCAN_DXLONG", "FUNC_DUALSCAN_DXSHORT"};

	EPRINTF(("docmem = %s(%d)\n", docmem_list[current->docmem], current->docmem));
	if(current->color_flag == COLORFAX_OFF){
		EPRINTF(("resolution[0] = %s(%d)\n", resolution_list[current->resolution[0]], current->resolution[0]));
	}else{
		EPRINTF(("resolution[0] = %s(%d)\n", resolution_list2[current->resolution[0]], current->resolution[0]));
	}

	EPRINTF(("gray_scale = %s(%d)\n", gray_scale_list[current->gray_scale], current->gray_scale));
	EPRINTF(("color_flag = %s(%d)\n", color_flag_list[current->color_flag], current->color_flag));
	EPRINTF(("file_format = %s(%d)\n", file_format_list[current->file_format], current->file_format));
#ifdef FB_SCAN_TYPE
	EPRINTF(("scan_src = %s(%d)\n", scan_src_list[current->scan_src], current->scan_src));
#endif /* FB_SCAN_TYPE */
	EPRINTF(("duplex_list = %s(%d)\n", duplex_list[fstget_data(FUNC_TMP_DUALSCANSETTING)], fstget_data(FUNC_TMP_DUALSCANSETTING)));
	EPRINTF(("apli = %d\n", sysmGetRsvApli(rsv_id)));
}

STATIC 	VOID
debug_print_ftp(ACCESS_INFO *access_info)
{
	UINT8 quality_list[][20]      = {"FTP_Color100", "FTP_Color200", "FTP_Color300", "FTP_Color600", "FTP_Gray100", "FTP_Gray200", "FTP_Gray300", "FTP_BW200", "FTP_BW100"};
	UINT8 fileformat_list[][20]   = {"FTP_PDF", "FTP_SPDF", "FTP_JPEG", "FTP_Tiff", "FTP_XPS"};
	UINT8 dualscan_list[][20]     = {"FTP_SIMPLEX", "FTP_DUPLEXLONG", "FTP_DUPLEXSHORT"};
	UINT8 filenametype_list[][25] = {"FTP_FNAMETYPE0_NODE", "FTP_FNAMETYPE1_ESTM", "FTP_FNAMETYPE2_REPO", "FTP_FNAMETYPE3_ORDE", "FTP_FNAMETYPE4_CONT", "FTP_FNAMETYPE5_CHCK", "FTP_FNAMETYPE6_RCPT", "FTP_FNAMETYPE7_OPT1", "FTP_FNAMETYPE8_OPT2", "FTP_FNAMETYPE9_MANU"};
	UINT8 ispassive_list[][10]    = {"Passive", "Active"};
	UINT8 scan_quality_list[][20] = {"P_SCAN_QUAL_NORMAL", "P_SCAN_QUAL_MID", "P_SCAN_QUAL_HIGH", "P_SCAN_QUAL_SHIGH"};
#ifdef FB_SCAN_TYPE
	UINT8 scan_src_list[][20]     = {"ERROR","SCAN_SRC_ADF", "SCAN_SRC_FB"};
#endif /* FB_SCAN_TYPE */

	EPRINTF(("quality = %s(%d)\n", quality_list[access_info->quality - FTP_Color100], access_info->quality));
	EPRINTF(("fileformat = %s(%d)\n", fileformat_list[access_info->fileformat - FTP_PDF], access_info->fileformat));
	EPRINTF(("dualscan = %s(%d)\n", dualscan_list[access_info->dualscan - FTP_SIMPLEX], access_info->dualscan));
	EPRINTF(("filenametype = %s(%d)\n", filenametype_list[access_info->filenametype - FTP_FNAMETYPE0_NODE], access_info->filenametype));
	EPRINTF(("servername = %s\n", access_info->servername));
	EPRINTF(("serveraddress = %s\n", access_info->serveraddress));
	EPRINTF(("username = %s\n", access_info->username));
	EPRINTF(("password = %s\n", access_info->password));
	EPRINTF(("storedir = %s\n", access_info->storedir));
	EPRINTF(("filename = %s\n", access_info->filename));
	EPRINTF(("spdfpass = %s\n", access_info->spdfpass));
	EPRINTF(("ispassive = %s(%d)\n", ispassive_list[access_info->ispassive], access_info->ispassive));
	EPRINTF(("portnum = %d\n", access_info->portnum));
	EPRINTF(("scan_quality = %s(%d)\n", scan_quality_list[access_info->scan_quality], access_info->scan_quality));
#ifdef FB_SCAN_TYPE
	EPRINTF(("scan_src = %s(%d)\n", scan_src_list[access_info->scan_src], access_info->scan_src));
#endif /* FB_SCAN_TYPE */
}

STATIC 	VOID
debug_print_network(CIFSACCESS_INFO *access_info)
{
	UINT8 quality_list[][20]              = {"FTP_Color100", "FTP_Color200", "FTP_Color300", "FTP_Color600", "FTP_Gray100", "FTP_Gray200", "FTP_Gray300", "FTP_BW200", "FTP_BW100"};
	UINT8 fileformat_list[][20]           = {"FTP_PDF", "FTP_SPDF", "FTP_JPEG", "FTP_Tiff", "FTP_XPS"};
	UINT8 dualscan_list[][20]             = {"FTP_SIMPLEX", "FTP_DUPLEXLONG", "FTP_DUPLEXSHORT"};
	UINT8 filenametype_list[][25]         = {"FTP_FNAMETYPE0_NODE", "FTP_FNAMETYPE1_ESTM", "FTP_FNAMETYPE2_REPO", "FTP_FNAMETYPE3_ORDE", "FTP_FNAMETYPE4_CONT", "FTP_FNAMETYPE5_CHCK", "FTP_FNAMETYPE6_RCPT", "FTP_FNAMETYPE7_OPT1", "FTP_FNAMETYPE8_OPT2", "FTP_FNAMETYPE9_MANU"};
	UINT8 AuthenticationMethod_list[][30] = {"ERROR", "CIFSCLIB_AUTHMETH_AUTO", "CIFSCLIB_AUTHMETH_KERBEROS", "CIFSCLIB_AUTHMETH_NTLMV2"};
	UINT8 scan_quality_list[][20]         = {"P_SCAN_QUAL_NORMAL", "P_SCAN_QUAL_MID", "P_SCAN_QUAL_HIGH", "P_SCAN_QUAL_SHIGH"};
#ifdef FB_SCAN_TYPE
	UINT8 scan_src_list[][20]             = {"ERROR","SCAN_SRC_ADF", "SCAN_SRC_FB"};
#endif /* FB_SCAN_TYPE */

	EPRINTF(("quality = %s(%d)\n", quality_list[access_info->quality - FTP_Color100], access_info->quality));
	EPRINTF(("fileformat = %s(%d)\n", fileformat_list[access_info->fileformat - FTP_PDF], access_info->fileformat));
	EPRINTF(("dualscan = %s(%d)\n", dualscan_list[access_info->dualscan - FTP_SIMPLEX], access_info->dualscan));
	EPRINTF(("filenametype = %s(%d)\n", filenametype_list[access_info->filenametype - FTP_FNAMETYPE0_NODE], access_info->filenametype));
	EPRINTF(("serveraddress = %s\n", access_info->serveraddress));
	EPRINTF(("username = %s\n", access_info->username));
	EPRINTF(("password = %s\n", access_info->password));
	EPRINTF(("storedir = %s\n", access_info->storedir));
	EPRINTF(("filename = %s\n", access_info->filename));
	EPRINTF(("AuthenticationMethod = %s(%d)\n", AuthenticationMethod_list[access_info->AuthenticationMethod], access_info->AuthenticationMethod));
	EPRINTF(("KerberosServerAddress = %s\n", access_info->KerberosServerAddress));
	EPRINTF(("spdfpass = %s\n", access_info->spdfpass));
	EPRINTF(("scan_quality = %s(%d)\n", scan_quality_list[access_info->scan_quality], access_info->scan_quality));
#ifdef FB_SCAN_TYPE
	EPRINTF(("scan_src = %s(%d)\n", scan_src_list[access_info->scan_src], access_info->scan_src));
#endif /* FB_SCAN_TYPE */
}
#endif


#endif /* (USE_BSI || defined(USE_PHOENIX_SERIO_MOBILE_IF)) && USE_SCAN */

/*****************************************************************************/
/**
 * @par    Serio IO Job Scan Connector 実行開始
 * @note    M-BC2-3956 BC2-FB専用の回避策
**/
GLOBAL VOID
Seriocn_Memread_Scan_Start( VOID )
{
#if (defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)) && defined(USE_SCAN)
	UINT32			mem_qid;
	memread_msg_t	mem_msg;
	INT16 			type;
	UINT32			timer_sec;
	
	mem_qid = FOS_MSGGETID( MEM_READ_MSG_NAME );
	
	type = JOB_TX;
	timer_sec = JOBTIME_NOW;
	
	mem_msg.com_msg.from_task = PANEL_BASE_TASK;
#ifdef USE_SEPARATE_UI
	mem_msg.com_msg.cmd_id = MEMR_CMD_START;
	mem_msg.job_msg.job_type = type;
#else	/* USE_SEPARATE_UI */
	mem_msg.com_msg.cmd_id = type;
#endif	/* USE_SEPARATE_UI */
	mem_msg.job_msg.job_id = s_job_id;
	mem_msg.job_msg.rsv_id = s_rsv_id;
	mem_msg.job_msg.dialtime = timer_sec;
	FOS_MSGSEND(mem_qid, (UINT8*)&mem_msg, sizeof(memread_msg_t));
#endif	/* (defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)) && defined(USE_SCAN) */
	return;
}

/*****************************************************************************/
/**
 * @par    Serio IO Job Scan Connector 実行開始
 * @note    M-BC2-3956 BC2-FB専用の回避策
**/
GLOBAL VOID 
Seriocn_Ftpclient_Scan_Start( VOID )
{
#if (defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)) && defined(USE_SCAN)
   UINT32                       qid_ftpclient;
    ftpclt_msg                   msg_param;
	
	/*  */
	memset( &msg_param, NULL, sizeof(ftpclt_msg) );
	
	qid_ftpclient		= FOS_MSGGETID( FTPC_MSG_NAME );
	msg_param.from_task	= (UINT16)PANEL_BASE_TASK;
	msg_param.cmd_id	= (UINT16)CMD_SCANSTART;

	FOS_MSGSEND( qid_ftpclient, (UINT8 *)&msg_param, sizeof(ftpclt_msg) );
#endif	/* (defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)) && defined(USE_SCAN) */
	return;
}

#else	/* if 0 */
/****** インクルード・ファイル ******************************************/
#include "spec.h"
#include "stdtype.h"
#include "fos.h"
#include "debug.h"
#include "common.h"

#undef	INK_MODEL_ONLY			/* BC2でビルドを通すため仮に設ける。 正式な担当Gで対策をお願いします */

#if (defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)) && defined(USE_SCAN)

#include "componentlib/serio/serio.h"
#include "phx_serio_ioscansend_connector.h"
#include "componentlib/report/reptlib.h"
#include "componentlib/hakkolib/job.h"
#include "lib/funcset/funcset.h"
#include "lib/cplib/cp_api_lib.h"
#include "lib/cplib/cp_sts_apl_sta.h"
#include "task/ftpclient/ftpclient.h"
#include "scanning/scan/scanTask.h"
#include "serio_task_common.h"
#include "serio_connector_debug.h"

/*--------------- 定義値群 ---------------*/
typedef enum {
    SERIO_MEM_PARA_INIT = 1,
    SERIO_MEM_PARA_SET,
    SERIO_MEM_PARA_GET,
    SERIO_MEM_PARA_DELETE,
    SERIO_MEM_PARA_MAX,
}SERIO_MEM_PARAM_t;

#define	IOSCAN_PARAM_ERROR		0xff

/*--------------- テーブル群 ---------------*/
typedef struct {
	enum ns__UloadResolution	Resolution;
	UINT8						mn_reso_param;
	UINT8						gr_reso_param;
	UINT8						cl_reso_param;
}SCAN_RESO_STRICTPARAM_t;

/* 下記Scan方式によって設定に差異が生まれる場合を想定し、現状は設定内容が同じであるが分けておく。 */
/* Scan2FTP or Scan2Networkの際の補正なしParam設定テーブル */
STATIC const SCAN_RESO_STRICTPARAM_t	Ftpclient_Strict_Resolution[] = {
	{ns__UloadResolution__Low		,CPAPI_P_VAL_SCAN_RESO_200X100	,CPAPI_P_VAL_SCAN_RESO_100	,CPAPI_P_VAL_SCAN_RESO_100 },
	{ns__UloadResolution__100		,IOSCAN_PARAM_ERROR				,CPAPI_P_VAL_SCAN_RESO_100	,CPAPI_P_VAL_SCAN_RESO_100 },
	{ns__UloadResolution__200x100	,CPAPI_P_VAL_SCAN_RESO_200X100	,IOSCAN_PARAM_ERROR			,IOSCAN_PARAM_ERROR		   },
	{ns__UloadResolution__150		,IOSCAN_PARAM_ERROR				,IOSCAN_PARAM_ERROR			,IOSCAN_PARAM_ERROR		   },
	{ns__UloadResolution__200		,CPAPI_P_VAL_SCAN_RESO_200		,CPAPI_P_VAL_SCAN_RESO_200	,CPAPI_P_VAL_SCAN_RESO_200 },
	{ns__UloadResolution__Normal	,CPAPI_P_VAL_SCAN_RESO_200		,CPAPI_P_VAL_SCAN_RESO_200	,CPAPI_P_VAL_SCAN_RESO_300 },
	{ns__UloadResolution__300		,CPAPI_P_VAL_SCAN_RESO_300		,CPAPI_P_VAL_SCAN_RESO_300	,CPAPI_P_VAL_SCAN_RESO_300 },
	{ns__UloadResolution__400		,IOSCAN_PARAM_ERROR				,IOSCAN_PARAM_ERROR			,IOSCAN_PARAM_ERROR		   },
	{ns__UloadResolution__High		,CPAPI_P_VAL_SCAN_RESO_300		,CPAPI_P_VAL_SCAN_RESO_300	,CPAPI_P_VAL_SCAN_RESO_600 },
	{ns__UloadResolution__600		,IOSCAN_PARAM_ERROR				,IOSCAN_PARAM_ERROR			,CPAPI_P_VAL_SCAN_RESO_600 },
#ifdef  USE_SCAN_AUTO_RESOLUTION
	{ns__UloadResolution__Auto		,IOSCAN_PARAM_ERROR				,CPAPI_P_VAL_SCAN_RESO_AUTO	,CPAPI_P_VAL_SCAN_RESO_AUTO},
#else   /* USE_SCAN_AUTO_RESOLUTION */
	{ns__UloadResolution__Auto		,IOSCAN_PARAM_ERROR				,IOSCAN_PARAM_ERROR			,IOSCAN_PARAM_ERROR		   },
#endif  /* USE_SCAN_AUTO_RESOLUTION */
};

/* Scan2FTP or Scan2Networkの際の補正ありParam設定テーブル */
STATIC const SCAN_RESO_STRICTPARAM_t	Ftpclient_not_Strict_Resolution[] = {
	{ns__UloadResolution__Low		,CPAPI_P_VAL_SCAN_RESO_200X100	,CPAPI_P_VAL_SCAN_RESO_100	,CPAPI_P_VAL_SCAN_RESO_100 },
	{ns__UloadResolution__100		,CPAPI_P_VAL_SCAN_RESO_200X100	,CPAPI_P_VAL_SCAN_RESO_100	,CPAPI_P_VAL_SCAN_RESO_100 },
	{ns__UloadResolution__200x100	,CPAPI_P_VAL_SCAN_RESO_200X100	,CPAPI_P_VAL_SCAN_RESO_100	,CPAPI_P_VAL_SCAN_RESO_100 },
	{ns__UloadResolution__150		,CPAPI_P_VAL_SCAN_RESO_200X100	,CPAPI_P_VAL_SCAN_RESO_100	,CPAPI_P_VAL_SCAN_RESO_100 },
	{ns__UloadResolution__200		,CPAPI_P_VAL_SCAN_RESO_200		,CPAPI_P_VAL_SCAN_RESO_200	,CPAPI_P_VAL_SCAN_RESO_200 },
	{ns__UloadResolution__Normal	,CPAPI_P_VAL_SCAN_RESO_200		,CPAPI_P_VAL_SCAN_RESO_200	,CPAPI_P_VAL_SCAN_RESO_300 },
	{ns__UloadResolution__300		,CPAPI_P_VAL_SCAN_RESO_300		,CPAPI_P_VAL_SCAN_RESO_300	,CPAPI_P_VAL_SCAN_RESO_300 },
	{ns__UloadResolution__400		,CPAPI_P_VAL_SCAN_RESO_300		,CPAPI_P_VAL_SCAN_RESO_300	,CPAPI_P_VAL_SCAN_RESO_300 },
	{ns__UloadResolution__High		,CPAPI_P_VAL_SCAN_RESO_300		,CPAPI_P_VAL_SCAN_RESO_300	,CPAPI_P_VAL_SCAN_RESO_600 },
	{ns__UloadResolution__600		,CPAPI_P_VAL_SCAN_RESO_300		,CPAPI_P_VAL_SCAN_RESO_300	,CPAPI_P_VAL_SCAN_RESO_600 },
#ifdef  USE_SCAN_AUTO_RESOLUTION
	{ns__UloadResolution__Auto		,CPAPI_P_VAL_SCAN_RESO_200		,CPAPI_P_VAL_SCAN_RESO_AUTO	,CPAPI_P_VAL_SCAN_RESO_AUTO},
#else   /* USE_SCAN_AUTO_RESOLUTION */
	{ns__UloadResolution__Auto		,CPAPI_P_VAL_SCAN_RESO_200		,CPAPI_P_VAL_SCAN_RESO_200	,CPAPI_P_VAL_SCAN_RESO_300 },
#endif  /* USE_SCAN_AUTO_RESOLUTION */
};

/* Scan2E-mail Serverの際の補正なしParam設定テーブル */
STATIC const SCAN_RESO_STRICTPARAM_t	Memread_Strict_Resolution[] = {
	{ns__UloadResolution__Low		,CPAPI_P_VAL_SCAN_RESO_200X100	,CPAPI_P_VAL_SCAN_RESO_100	,CPAPI_P_VAL_SCAN_RESO_100 },
	{ns__UloadResolution__100		,IOSCAN_PARAM_ERROR				,CPAPI_P_VAL_SCAN_RESO_100	,CPAPI_P_VAL_SCAN_RESO_100 },
	{ns__UloadResolution__200x100	,CPAPI_P_VAL_SCAN_RESO_200X100	,IOSCAN_PARAM_ERROR			,IOSCAN_PARAM_ERROR		   },
	{ns__UloadResolution__150		,IOSCAN_PARAM_ERROR				,IOSCAN_PARAM_ERROR			,IOSCAN_PARAM_ERROR		   },
	{ns__UloadResolution__200		,CPAPI_P_VAL_SCAN_RESO_200		,CPAPI_P_VAL_SCAN_RESO_200	,CPAPI_P_VAL_SCAN_RESO_200 },
	{ns__UloadResolution__Normal	,CPAPI_P_VAL_SCAN_RESO_200		,CPAPI_P_VAL_SCAN_RESO_200	,CPAPI_P_VAL_SCAN_RESO_300 },
	{ns__UloadResolution__300		,CPAPI_P_VAL_SCAN_RESO_300		,CPAPI_P_VAL_SCAN_RESO_300	,CPAPI_P_VAL_SCAN_RESO_300 },
	{ns__UloadResolution__400		,IOSCAN_PARAM_ERROR				,IOSCAN_PARAM_ERROR			,IOSCAN_PARAM_ERROR		   },
	{ns__UloadResolution__High		,CPAPI_P_VAL_SCAN_RESO_300		,CPAPI_P_VAL_SCAN_RESO_300	,CPAPI_P_VAL_SCAN_RESO_600 },
	{ns__UloadResolution__600		,IOSCAN_PARAM_ERROR				,IOSCAN_PARAM_ERROR			,CPAPI_P_VAL_SCAN_RESO_600 },
#ifdef  USE_SCAN_AUTO_RESOLUTION
	{ns__UloadResolution__Auto		,IOSCAN_PARAM_ERROR				,CPAPI_P_VAL_SCAN_RESO_AUTO	,CPAPI_P_VAL_SCAN_RESO_AUTO},
#else   /* USE_SCAN_AUTO_RESOLUTION */
	{ns__UloadResolution__Auto		,IOSCAN_PARAM_ERROR				,IOSCAN_PARAM_ERROR			,IOSCAN_PARAM_ERROR		   },
#endif  /* USE_SCAN_AUTO_RESOLUTION */
};

/* Scan2E-mail Serverの際の補正ありParam設定テーブル */
STATIC const SCAN_RESO_STRICTPARAM_t	Memread_not_Strict_Resolution[] = {
	{ns__UloadResolution__Low		,CPAPI_P_VAL_SCAN_RESO_200X100	,CPAPI_P_VAL_SCAN_RESO_100	,CPAPI_P_VAL_SCAN_RESO_100 },
	{ns__UloadResolution__100		,CPAPI_P_VAL_SCAN_RESO_200X100	,CPAPI_P_VAL_SCAN_RESO_100	,CPAPI_P_VAL_SCAN_RESO_100 },
	{ns__UloadResolution__200x100	,CPAPI_P_VAL_SCAN_RESO_200X100	,CPAPI_P_VAL_SCAN_RESO_100	,CPAPI_P_VAL_SCAN_RESO_100 },
	{ns__UloadResolution__150		,CPAPI_P_VAL_SCAN_RESO_200X100	,CPAPI_P_VAL_SCAN_RESO_100	,CPAPI_P_VAL_SCAN_RESO_100 },
	{ns__UloadResolution__200		,CPAPI_P_VAL_SCAN_RESO_200		,CPAPI_P_VAL_SCAN_RESO_200	,CPAPI_P_VAL_SCAN_RESO_200 },
	{ns__UloadResolution__Normal	,CPAPI_P_VAL_SCAN_RESO_200		,CPAPI_P_VAL_SCAN_RESO_200	,CPAPI_P_VAL_SCAN_RESO_300 },
	{ns__UloadResolution__300		,CPAPI_P_VAL_SCAN_RESO_300		,CPAPI_P_VAL_SCAN_RESO_300	,CPAPI_P_VAL_SCAN_RESO_300 },
	{ns__UloadResolution__400		,CPAPI_P_VAL_SCAN_RESO_300		,CPAPI_P_VAL_SCAN_RESO_300	,CPAPI_P_VAL_SCAN_RESO_300 },
	{ns__UloadResolution__High		,CPAPI_P_VAL_SCAN_RESO_300		,CPAPI_P_VAL_SCAN_RESO_300	,CPAPI_P_VAL_SCAN_RESO_600 },
	{ns__UloadResolution__600		,CPAPI_P_VAL_SCAN_RESO_300		,CPAPI_P_VAL_SCAN_RESO_300	,CPAPI_P_VAL_SCAN_RESO_600 },
#ifdef  USE_SCAN_AUTO_RESOLUTION
	{ns__UloadResolution__Auto		,CPAPI_P_VAL_SCAN_RESO_200		,CPAPI_P_VAL_SCAN_RESO_AUTO	,CPAPI_P_VAL_SCAN_RESO_AUTO},
#else   /* USE_SCAN_AUTO_RESOLUTION */
	{ns__UloadResolution__Auto		,CPAPI_P_VAL_SCAN_RESO_200		,CPAPI_P_VAL_SCAN_RESO_200	,CPAPI_P_VAL_SCAN_RESO_300 },
#endif  /* USE_SCAN_AUTO_RESOLUTION */
};

/*--------------- STATIC変数宣言群 ---------------*/
STATIC BOOL StrictParamFlag;
STATIC BOOL ParamChkOnlyFlag;
STATIC BOOL ScanParamErrFlag;

/*--------------- STATIC関数宣言群 ---------------*/
STATIC INT8  serio_scan_connector_mem_param(SERIO_MEM_PARAM_t cmd, void* param);

STATIC INT32 serio_scan_cnvScanTray(enum ns__ScanTraySelection inParam);
STATIC PHXSERIO_PROFILETYPE_T GetProfileType(INT32 ProfileUnionType);
STATIC UINT8 GetResolution_EMail(enum ns__ColorModeSelection* ColorMode, enum ns__UloadResolution* Resolution);
STATIC UINT8 GetFileformat_EMail(enum ns__FileFormatSelection* FileType);
STATIC UINT8 GetColorFlag(enum ns__ColorModeSelection* ColorMode);
STATIC UINT8 GetGrayScale(enum ns__ColorModeSelection* ColorMode);
STATIC UINT8 GetAuthMethod(enum ns__CifsParams_AuthMethod AuthMethod);
STATIC UINT8 GetColorMode(enum ns__ColorModeSelection* in_pColorMode);
STATIC INT32 GetQuality(enum ns__ColorModeSelection* ColorMode, enum ns__UloadResolution* Resolution);
STATIC UINT8 GetDualscan(enum xsd__boolean* DuplexScanEnable, enum xsd__boolean* ShortEdgeBinding);
STATIC UINT8 get_scan_doc_size(enum ns__ScanAndUloadScansize* scan_doc_size);
STATIC UINT8 get_scan_quality(enum ns__Selection3* compress_rate);
STATIC INT32 GetTxProfileType(SERIO_CNP_IOJOB_SCANSEND_T *iojob);
STATIC UINT8 GetFileformat_SNW(enum ns__FileFormatSelection *FileType);
STATIC UINT8 GetFileformat_SFTP(enum ns__FileFormatSelection *FileType);
STATIC INT8  ChkProhibitionSettings(IOScansend_exec_param_t* ioscansend_param);


/*****************************************************************************/
/**
 * @par    Serio IO Job Scan Connector 初期化
 * @param   Param   [IN]    起動引数
 * @retval  実行結果
**/
GLOBAL INT32
IOJobConnector_SendScan_Init(VOID *Param)
{
    INT32 UnionType;
    if(Param == NULL) {
        return ERROR;
    }
    UnionType = GetTxProfileType((SERIO_CNP_IOJOB_SCANSEND_T *)Param);
    if(UnionType == ERROR){
        return ERROR;
    }
    
    /* ここにStrictParamとParamCheckOnlyの状態を確認する処理を追加する。 */
    /* その２つのステータスの保持は、STATICなフラグをそれぞれ用意する。  */
    /* そのステータスが無い場合は、丸め優先、実行優先で良い。            */
    StrictParamFlag  = FALSE;
    ParamChkOnlyFlag = FALSE;
    ScanParamErrFlag = FALSE;
    
    serio_scan_connector_mem_param(SERIO_MEM_PARA_INIT, NULL);

    return OK;
}

/*****************************************************************************/
/**
 * @par    Serio IO Job Scan Connector 実行
 * @param   Param   [IN]    起動引数
 * @retval  実行結果
**/
GLOBAL INT32
IOJobConnector_SendScan_Exec(VOID *Param)
{
    serio_scan_connector_mem_param(SERIO_MEM_PARA_SET, Param);

	if(ParamChkOnlyFlag == FALSE)
	{
		cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_START);
		cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_EXEC);
	}
    return OK;
}

/*****************************************************************************/
/**
 * @par    Serio IO Job Scan Connector 再開
 * @param   Param   [IN]    起動引数（未使用）
 * @retval  実行結果
**/
GLOBAL INT32
IOJobConnector_SendScan_Restart(SERIO_CN_RESTART_PARAM_T *Param)
{
    if(Param != NULL) {
        switch(Param->Reason) {
        case SERIOFW_JOBSTS_PAUSEDBY_ERROR:
            cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_ERR_RESTART);
            cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_EXEC);
            break;

        case SERIOFW_JOBSTS_PAUSEDBY_SCANNEXT:
            if(Param->SubCode == 1) {
                cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_JOB_RESTART);
                cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_EXEC);
            } else if(Param->SubCode == 2) {
                cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_JOB_ENDPAGE);
                cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_EXEC);
            }
            break;
        default:
            break;
        }
    }
    return OK;
}

/*****************************************************************************/
/**
 * @par    Serio IO Job Scan Connector 停止
 * @retval  実行結果
**/
GLOBAL INT32
IOJobConnector_SendScan_Cancel(VOID *Param)
{
    cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_IO_SCAN_CANCEL);
    return OK;
}

/*****************************************************************************/
/**
 * @par    Serio IO Job Scan Connector 終了
 * @retval  実行結果
**/
GLOBAL INT32
IOJobConnector_SendScan_Exit(VOID *Param)
{
    serio_scan_connector_mem_param(SERIO_MEM_PARA_DELETE, NULL);
    cp_Sts_Apl_Entry(CP_STS_APL_PHX, CP_STS_PHX_APP_START_FOR_LOGIN);
    return OK;
}

/*****************************************************************************/
/**
 * @par    Serio IO Job Scan Connector 終了
 * @retval  実行結果
**/
GLOBAL INT8 
Serio_IoScan_Get_exec_param( IOScansend_exec_param_t *op_exec_param )
{
    INT8                        ret;
    if(op_exec_param == NULL) {
        ret = ERROR;
    } else {
        serio_scan_connector_mem_param(SERIO_MEM_PARA_GET, op_exec_param);
        ret = OK;
    }
    return ret;
}

/*****************************************************************************/
/**
 * @par    Serio IO Job Scan の起動引数の保持
 * @param   cmd     [IN]        処理方法(初期化、設定、取得、削除）
 * @param   Param   [IN/OUT]    起動引数
 * @retval  OK  成功
 * @retval  NG  失敗
**/
STATIC INT8 
serio_scan_connector_mem_param(SERIO_MEM_PARAM_t cmd, void* param)
{
    STATIC IOScansend_exec_param_t  ioscansend_param;
    IOScansend_exec_param_t*        pexec_param;
    struct ns__FtpParams*           pFtp;
    struct ns__CifsParams*          pCifs;
    struct ns__SmtpProperty*        pSmtp;
    SERIO_CNP_IOJOB_SCANSEND_T*     serioPara;
    INT8                            ret = OK;

    switch(cmd) {
    case SERIO_MEM_PARA_INIT:
    case SERIO_MEM_PARA_DELETE:
        memset(&ioscansend_param, 0, sizeof(ioscansend_param));
        break;

    case SERIO_MEM_PARA_SET:
        serioPara = (SERIO_CNP_IOJOB_SCANSEND_T*)param;
        
        /* StrictParamの設定を取得する。*/
        if(serioPara->strictParam != NULL)
        {
			StrictParamFlag = *serioPara->strictParam;
		}
		
		/* ParamChkOnlyの設定を取得する。*/
		if(serioPara->paramCheckOnly != NULL)
		{
			ParamChkOnlyFlag = *serioPara->paramCheckOnly;
		}
        
        if(serioPara->ScanTray != NULL) {
            ioscansend_param.ScanTray = serio_scan_cnvScanTray(*serioPara->ScanTray);
        }

        ioscansend_param.ProfileType      = GetProfileType(( serioPara->TxProfiles.__TxProfiles[0].__union ));
        switch(ioscansend_param.ProfileType) {
        case PHXSERIO_PROFILE_TYPE_SMTP      :
            pSmtp = &serioPara->TxProfiles.__TxProfiles->TxProfile.Smtp;
            strncpy((char*)ioscansend_param.TxProfile.Smtp.destination, *pSmtp->Destination, sizeof(ioscansend_param.TxProfile.Smtp.destination));
            ioscansend_param.TxProfile.Smtp.color_flag    = GetColorFlag(serioPara->ColorMode);
            ioscansend_param.TxProfile.Smtp.resolution    = GetResolution_EMail(serioPara->ColorMode, serioPara->Resolution);
            ioscansend_param.TxProfile.Smtp.gray_scale    = GetGrayScale(serioPara->ColorMode);
            ioscansend_param.TxProfile.Smtp.file_format   = GetFileformat_EMail(serioPara->FileType);
            ioscansend_param.TxProfile.Smtp.scan_size     = get_scan_doc_size(serioPara->DocSize);;
            ioscansend_param.TxProfile.Smtp.duplex_scan   = GetDualscan(serioPara->DuplexScanEnable, serioPara->ShortEdgeBinding);;
            ioscansend_param.TxProfile.Smtp.file_size  = get_scan_quality(serioPara->JpgQuality);

            break;

        case PHXSERIO_PROFILE_TYPE_NETWORK      :
            pCifs = serioPara->TxProfiles.__TxProfiles->TxProfile.Cifs.CifsParams;
            if(pCifs != NULL) {
                strncpy((MD_CHAR*)ioscansend_param.TxProfile.Cifs.Host,           pCifs->Host,           sizeof(ioscansend_param.TxProfile.Cifs.Host          ) - 1);
                strncpy((MD_CHAR*)ioscansend_param.TxProfile.Cifs.StoreDir,       pCifs->StoreDir,       sizeof(ioscansend_param.TxProfile.Cifs.StoreDir      ) - 1);
                strncpy((MD_CHAR*)ioscansend_param.TxProfile.Cifs.FileName,       pCifs->FileName,       sizeof(ioscansend_param.TxProfile.Cifs.FileName      ) - 1);
                strncpy((MD_CHAR*)ioscansend_param.TxProfile.Cifs.User,           pCifs->User,           sizeof(ioscansend_param.TxProfile.Cifs.User          ) - 1);
                strncpy((MD_CHAR*)ioscansend_param.TxProfile.Cifs.Password,       pCifs->Password,       sizeof(ioscansend_param.TxProfile.Cifs.Password      ) - 1);
                strncpy((MD_CHAR*)ioscansend_param.TxProfile.Cifs.KerberosServer, pCifs->KerberosServer, sizeof(ioscansend_param.TxProfile.Cifs.KerberosServer) - 1);
                ioscansend_param.TxProfile.Cifs.AuthMethod    = GetAuthMethod(pCifs->AuthMethod);

                ioscansend_param.TxProfile.Cifs.color_mode    = GetColorMode(serioPara->ColorMode);
                ioscansend_param.TxProfile.Cifs.quality       = GetQuality(serioPara->ColorMode, serioPara->Resolution);
                ioscansend_param.TxProfile.Cifs.fileformat    = GetFileformat_SNW(serioPara->FileType);
                ioscansend_param.TxProfile.Cifs.dualscan      = GetDualscan(serioPara->DuplexScanEnable, serioPara->ShortEdgeBinding);
                ioscansend_param.TxProfile.Cifs.scan_doc_size = get_scan_doc_size(serioPara->DocSize);
                ioscansend_param.TxProfile.Cifs.file_size  = get_scan_quality(serioPara->JpgQuality);
            }
            break;

        case PHXSERIO_PROFILE_TYPE_FTP       :
            pFtp = serioPara->TxProfiles.__TxProfiles->TxProfile.Ftp.FtpParams;
            if(pFtp != NULL) {
                strncpy((MD_CHAR*)ioscansend_param.TxProfile.Ftp.FileName,       pFtp->FileName,       sizeof(ioscansend_param.TxProfile.Ftp.FileName      ) - 1);
                strncpy((MD_CHAR*)ioscansend_param.TxProfile.Ftp.Host,           pFtp->Host,           sizeof(ioscansend_param.TxProfile.Ftp.Host          ) - 1);
                strncpy((MD_CHAR*)ioscansend_param.TxProfile.Ftp.User,           pFtp->User,           sizeof(ioscansend_param.TxProfile.Ftp.User          ) - 1);
                strncpy((MD_CHAR*)ioscansend_param.TxProfile.Ftp.Password,       pFtp->Password,       sizeof(ioscansend_param.TxProfile.Ftp.Password      ) - 1);
                strncpy((MD_CHAR*)ioscansend_param.TxProfile.Ftp.StoreDir,       pFtp->StoreDir,       sizeof(ioscansend_param.TxProfile.Ftp.StoreDir      ) - 1);

                if(pFtp->PassiveMode == xsd__boolean__true_){
                    ioscansend_param.TxProfile.Ftp.isPassive = 1; /* Off */
                }else {
                    ioscansend_param.TxProfile.Ftp.isPassive = 0; /* On */
                }
                ioscansend_param.TxProfile.Ftp.PortNum       = atoi(pFtp->PortNum);
                ioscansend_param.TxProfile.Ftp.color_mode    = GetColorMode(serioPara->ColorMode);
                ioscansend_param.TxProfile.Ftp.quality       = GetQuality(serioPara->ColorMode, serioPara->Resolution);
                ioscansend_param.TxProfile.Ftp.fileformat    = GetFileformat_SFTP(serioPara->FileType);
                ioscansend_param.TxProfile.Ftp.dualscan      = GetDualscan(serioPara->DuplexScanEnable, serioPara->ShortEdgeBinding);
                ioscansend_param.TxProfile.Ftp.scan_doc_size = get_scan_doc_size(serioPara->DocSize);
                ioscansend_param.TxProfile.Ftp.file_size  = get_scan_quality(serioPara->JpgQuality);
            }
            break;
        case SOAP_UNION_ns__TxProfile_Scan2file :
        case SOAP_UNION_ns__TxProfile_Scan2email:
        case SOAP_UNION_ns__TxProfile_Scan2ocr  :
        case SOAP_UNION_ns__TxProfile___any     :
        default:
            /* phx serio unsupported profile ... */
            break;
        }
		
		/* Scanパラメータ設定において、対応外の設定値が設定されたかを確認する */
		if(ScanParamErrFlag == TRUE)
		{
			/* Scan中断通知をSerioFWに返す */
			SendJobStatus_End(SERIOFW_JOBSTS_END_CANCEL, SERIO_EXTERR_INABILITY, SERIO_JOB_SCANSEND);
		}
		
		/* ここで禁則処理を確認する。パラメータの補正あり、なしに関わらずチェックをし、禁則処理に当たったらエラーを返す。 */
		if(ChkProhibitionSettings(&ioscansend_param) == NG)
		{
			/* Scan中断通知をSerioFWに返す */
			SendJobStatus_End(SERIOFW_JOBSTS_END_CANCEL, SERIO_EXTERR_INABILITY, SERIO_JOB_SCANSEND);
		}
		
        break;

    case SERIO_MEM_PARA_GET:
        if (param == NULL) {
            ret = NG;
        } else {
            pexec_param = (IOScansend_exec_param_t*)param;
            *pexec_param = ioscansend_param;
        }
        break;
    default:
        break;
    }
    return ret;
}

/*****************************************************************************/
/**
 * @par     Serio/本体 ScanTray Parameter 変換
 * @param   cmd     [IN]        Serio Parameter
 * @retval  本体 Parameter
**/
STATIC INT32 
serio_scan_cnvScanTray(enum ns__ScanTraySelection inParam)
{
    INT32 ret = FB_SCAN;
    switch(inParam){
    case ns__ScanTraySelection__ADF: ret = ADF_SCAN; break;
    case ns__ScanTraySelection__FB : ret = FB_SCAN;  break;
    default                        :                 break;
    }
    return ret;
}

/*****************************************************************************/
/**
 * @par     Serio/本体 Scan ProfileType Parameter 変換
 * @param   cmd     [IN]        Serio Parameter
 * @retval  本体(phoenix) Parameter
**/
STATIC PHXSERIO_PROFILETYPE_T 
GetProfileType(INT32 ProfileUnionType)
{
    PHXSERIO_PROFILETYPE_T ret = PHXSERIO_PROFILE_UNKNOWN;
    switch(ProfileUnionType){
    case SOAP_UNION_ns__TxProfile_Smtp      : ret = PHXSERIO_PROFILE_TYPE_SMTP;    break;
    case SOAP_UNION_ns__TxProfile_Cifs      : ret = PHXSERIO_PROFILE_TYPE_NETWORK; break;
    case SOAP_UNION_ns__TxProfile_Ftp       : ret = PHXSERIO_PROFILE_TYPE_FTP;     break;
    case SOAP_UNION_ns__TxProfile_Scan2file :                                        break;
    case SOAP_UNION_ns__TxProfile_Scan2email:                                        break;
    case SOAP_UNION_ns__TxProfile_Scan2ocr  :                                        break;
    case SOAP_UNION_ns__TxProfile___any     :                                        break;
    default                                 :                                        break;
    }
    return ret;
}

/*****************************************************************************/
/**
 * @par     Serio/本体 E-Mail用Resolution Parameter 変換
 * @param   cmd     [IN]        Serio Parameter
 * @retval  本体 Parameter
**/
STATIC UINT8 
GetResolution_EMail(enum ns__ColorModeSelection* in_pColorMode, enum ns__UloadResolution* in_pResolution)
{
#ifdef INK_MODEL_ONLY
    UINT8   ret = RESOLUTION_STD;
    enum ns__ColorModeSelection ColorMode;
    enum ns__UloadResolution    Resolution;
    if(in_pColorMode == NULL || in_pResolution == NULL) {
        return ret;
    }
    ColorMode   = *in_pColorMode;
    Resolution  = *in_pResolution;

    ret = RESOLUTION_STD;
    if((ColorMode == ns__ColorModeSelection__Mono) ||
       (ColorMode == ns__ColorModeSelection__Gray)){
        /* Gray設定 */
        /* Mono設定 */
        switch(Resolution) {
        case ns__UloadResolution__Low     : ret = RESOLUTION_STD;      break;
        case ns__UloadResolution__100     : ret = RESOLUTION_STD;      break;
        case ns__UloadResolution__200x100 : ret = RESOLUTION_STD;      break;
        case ns__UloadResolution__Normal  : ret = RESOLUTION_FINE;     break;
        case ns__UloadResolution__200     : ret = RESOLUTION_FINE;     break;
        case ns__UloadResolution__High    : ret = RESOLUTION_SFINE;    break;
        case ns__UloadResolution__300     : ret = RESOLUTION_SFINE;    break;
        case ns__UloadResolution__400     : ret = RESOLUTION_SFINE;    break;
        case ns__UloadResolution__600     : ret = RESOLUTION_SFINE;    break;
        case ns__UloadResolution__Auto    : ret = RESOLUTION_FINE;     break;
        default                           : ret = RESOLUTION_FINE;     break;
        }
    }else if(ColorMode == ns__ColorModeSelection__Color){
        /* Color設定 */
        switch(Resolution) {
        case ns__UloadResolution__Low     : ret = SCAN2_RESO_CL100DPI; break;
        case ns__UloadResolution__100     : ret = SCAN2_RESO_CL100DPI; break;
        case ns__UloadResolution__200x100 : ret = SCAN2_RESO_CL100DPI; break;
        case ns__UloadResolution__200     : ret = SCAN2_RESO_CL200DPI; break;
        case ns__UloadResolution__Normal  : ret = SCAN2_RESO_CL300DPI; break;
        case ns__UloadResolution__300     : ret = SCAN2_RESO_CL300DPI; break;
        case ns__UloadResolution__400     : ret = SCAN2_RESO_CL300DPI; break;
        case ns__UloadResolution__High    : ret = SCAN2_RESO_CL600DPI; break;
        case ns__UloadResolution__600     : ret = SCAN2_RESO_CL600DPI; break;
#ifdef  USE_SCAN_AUTO_RESOLUTION
        case ns__UloadResolution__Auto    : ret = SCAN2_RESO_CLAUTO;   break;
#else   /* USE_SCAN_AUTO_RESOLUTION */
        case ns__UloadResolution__Auto    : ret = SCAN2_RESO_CL300DPI; break;
#endif  /* USE_SCAN_AUTO_RESOLUTION */
        default                           : ret = SCAN2_RESO_CL100DPI; break;
        }
    }
    return ret;
#else
    UINT8   i;
    UINT8   ret = CPAPI_P_VAL_SCAN_RESO_200;	/* ns__UloadResolution__Normalに相当する設定 */
    enum ns__ColorModeSelection ColorMode;
    enum ns__UloadResolution    Resolution;
	
	if(StrictParamFlag == FALSE)
	{
		ret = IOSCAN_PARAM_ERROR;
    }
    
    if(in_pColorMode == NULL || in_pResolution == NULL) {
        return ret;
    }
    ColorMode   = *in_pColorMode;
    Resolution  = *in_pResolution;

	if(StrictParamFlag == FALSE)
	{
		/* 丸め処理あり */
		for( i = 0; i < sizeof(Memread_not_Strict_Resolution)/sizeof(Memread_not_Strict_Resolution[0]); i++ )
		{
			if( Resolution == Memread_not_Strict_Resolution[i].Resolution )
			{
				if(ColorMode == ns__ColorModeSelection__Mono)
				{
					/* Mono設定 */
					ret = Memread_not_Strict_Resolution[i].mn_reso_param;
				}
				else if(ColorMode == ns__ColorModeSelection__Gray)
				{
					/* Gray設定 */
					ret = Memread_not_Strict_Resolution[i].gr_reso_param;
				}
				else if(ColorMode == ns__ColorModeSelection__Color)
				{
					/* Color設定 */
					ret = Memread_not_Strict_Resolution[i].cl_reso_param;
				}
			}
		}
	}
	else
	{
		/* 丸め処理なし */
		for( i = 0; i < sizeof(Memread_Strict_Resolution)/sizeof(Memread_Strict_Resolution[0]); i++ )
		{
			if( Resolution == Memread_Strict_Resolution[i].Resolution )
			{
				if(ColorMode == ns__ColorModeSelection__Mono)
				{
					/* Mono設定 */
					ret = Memread_Strict_Resolution[i].mn_reso_param;
				}
				else if(ColorMode == ns__ColorModeSelection__Gray)
				{
					/* Gray設定 */
					ret = Memread_Strict_Resolution[i].gr_reso_param;
				}
				else if(ColorMode == ns__ColorModeSelection__Color)
				{
					/* Color設定 */
					ret = Memread_Strict_Resolution[i].cl_reso_param;
				}
			}
		}
	}
	
	if(ret == IOSCAN_PARAM_ERROR)
	{
		ScanParamErrFlag = TRUE;
	}

    return ret;
#endif
}

/*****************************************************************************/
/**
 * @par     Serio/本体 E-Mail用File format Parameter 変換
 * @param   cmd     [IN]        Serio Parameter
 * @retval  本体 Parameter
**/
STATIC  UINT8   
GetFileformat_EMail(enum ns__FileFormatSelection* FileType)
{
    UINT8   ret = SCAN2NW_FILETYPE_PDF;
    
	if(StrictParamFlag == TRUE)
	{
		ret = IOSCAN_PARAM_ERROR;
	}
	
    if( FileType != NULL) {
        switch(*FileType) {
        case ns__FileFormatSelection__TIFF :  ret = CPAPI_P_VAL_FILE_FORMAT_TIFF; break;
        case ns__FileFormatSelection__JPEG :  ret = CPAPI_P_VAL_FILE_FORMAT_JPEG; break;
        case ns__FileFormatSelection__PDF  :  ret = CPAPI_P_VAL_FILE_FORMAT_PDF;  break;
        case ns__FileFormatSelection__XPS  :  ret = CPAPI_P_VAL_FILE_FORMAT_XPS;  break;
        default                            :                                      break;
        }
    }
    
	if(ret == IOSCAN_PARAM_ERROR)
	{
		ScanParamErrFlag = TRUE;
	}
	
    return ret;
}

/*****************************************************************************/
/**
 * @par     Serio/本体 Color Mode Parameter 変換
 * @param   cmd     [IN]        Serio Parameter
 * @retval  本体 Parameter
**/
STATIC UINT8 
GetGrayScale(enum ns__ColorModeSelection* ColorMode)
{
	UINT8	gray_scale;

	if(*ColorMode == ns__ColorModeSelection__Color || *ColorMode == ns__ColorModeSelection__Gray){
		gray_scale = USW_SCAN_MULTI;
	}else{
		gray_scale = USW_SCAN_2LEVEL;
	}

	return gray_scale;
}

/*****************************************************************************/
/**
 * @par     Serio/本体 Color Mode Parameter 変換
 * @param   cmd     [IN]        Serio Parameter
 * @retval  本体 Parameter
**/
STATIC  UINT8 
GetColorFlag(enum ns__ColorModeSelection* in_pColorMode)
{
    UINT8       ret = CPAPI_P_VAL_SCAN_COLOR_TYPE_COLOR;
    enum ns__ColorModeSelection ColorMode;

    if(in_pColorMode == NULL){
        return ret;
    }
    
	if(StrictParamFlag == TRUE)
	{
		ret = IOSCAN_PARAM_ERROR;
	}

    ColorMode = *in_pColorMode;
    
    /* カラー設定値 */
    switch(ColorMode){
    case ns__ColorModeSelection__Color: ret = CPAPI_P_VAL_SCAN_COLOR_TYPE_COLOR;  break;
    case ns__ColorModeSelection__Mono : ret = CPAPI_P_VAL_SCAN_COLOR_TYPE_BW;     break;
    case ns__ColorModeSelection__Gray : ret = CPAPI_P_VAL_SCAN_COLOR_TYPE_GRAY;   break;
    case ns__ColorModeSeledtion__Auto : ret = CPAPI_P_VAL_SCAN_COLOR_TYPE_AUTO;   break;
    default                           :                                           break;
    }
    
	if(ret == IOSCAN_PARAM_ERROR)
	{
		ScanParamErrFlag = TRUE;
	}
	
    return ret;
}

/*****************************************************************************/
/**
 * @par     Serio/本体 認証方式 Parameter 変換
 * @param   cmd     [IN]        Serio Parameter
 * @retval  本体 Parameter
**/
STATIC UINT8 
GetAuthMethod(enum ns__CifsParams_AuthMethod AuthMethod)
{
    UINT8 ret = CIFSCLIB_AUTHMETH_AUTO;
    switch(AuthMethod) {
    case ns__CifsParams_AuthMethod__Auto     : ret = CIFSCLIB_AUTHMETH_AUTO;     break;
    case ns__CifsParams_AuthMethod__Kerberos : ret = CIFSCLIB_AUTHMETH_KERBEROS; break;
    case ns__CifsParams_AuthMethod__NTLMv2   : ret = CIFSCLIB_AUTHMETH_NTLMV2;   break;
    }
    return ret;
}

/*****************************************************************************/
/**
 * @par     Serio/（表示用） 色 Parameter 変換
 * @param   cmd     [IN]        Serio Parameter
 * @retval  本体 Parameter
 * @note    表示用のTemp領域へ設定する Scan Color
 *          PanelではFileTypeによって、Colorの初期値を決めているため、
 *          本体設定用のマクロ名が無い
 *          CPAPI_P_VAL_SCAN_COLOR_TYPE_COLOR	0x00   カラー設定：カラー
 *          CPAPI_P_VAL_SCAN_COLOR_TYPE_BW		0x01   カラー設定：モノクロ
 *          CPAPI_P_VAL_SCAN_COLOR_TYPE_GRAY	0x02   カラー設定：グレイ
 *          CPAPI_P_VAL_SCAN_COLOR_TYPE_AUTO	0x03   カラー設定：オート
 *          CPAPI_P_VAL_SCAN_COLOR_TYPE_DEFAULT	0x00   デフォルトカラー設定：カラー
**/
STATIC UINT8 
GetColorMode(enum ns__ColorModeSelection* in_pColorMode)
{
    UINT8           ret = CPAPI_P_VAL_SCAN_COLOR_TYPE_COLOR; /* Color */
    enum ns__ColorModeSelection ColorMode;
    
    if(in_pColorMode == NULL) {
        return ret;
    }
    
	if(StrictParamFlag == TRUE)
	{
		ret = IOSCAN_PARAM_ERROR;
	}

    ColorMode   = *in_pColorMode;

    
    /* カラー設定値 */
    switch(ColorMode){
    case ns__ColorModeSelection__Color: ret = CPAPI_P_VAL_SCAN_COLOR_TYPE_COLOR;  break;
    case ns__ColorModeSelection__Mono : ret = CPAPI_P_VAL_SCAN_COLOR_TYPE_BW;     break;
    case ns__ColorModeSelection__Gray : ret = CPAPI_P_VAL_SCAN_COLOR_TYPE_GRAY;   break;
    case ns__ColorModeSeledtion__Auto : ret = CPAPI_P_VAL_SCAN_COLOR_TYPE_AUTO;   break;
    default                           :                                           break;
    }
	
	if(ret == IOSCAN_PARAM_ERROR)
	{
		ScanParamErrFlag = TRUE;
	}
	
    return ret;
}
/*****************************************************************************/
/**
 * @par     Serio/本体 Quality（色 + 解像度） Parameter 変換
 * @param   cmd     [IN]        Serio Parameter
 * @retval  本体 Parameter
**/
STATIC INT32 
GetQuality(enum ns__ColorModeSelection* in_pColorMode, enum ns__UloadResolution* in_pResolution)
{
#ifdef INK_MODEL_ONLY
    INT32           ret = SCAN2_RESO_BW100DPI;
    enum ns__ColorModeSelection ColorMode;
    enum ns__UloadResolution    Resolution;

    if(in_pColorMode == NULL || in_pResolution == NULL) {
        return ret;
    }
    ColorMode   = *in_pColorMode;
    Resolution  = *in_pResolution;

    ret = SCAN2_RESO_BW100DPI;
    if( (ColorMode == ns__ColorModeSelection__Mono) ||
        (ColorMode == ns__ColorModeSelection__Gray) ){
        switch(Resolution) {
        case ns__UloadResolution__Low:     ret = SCAN2_RESO_BW100DPI; break;
        case ns__UloadResolution__100:     ret = SCAN2_RESO_BW100DPI; break;
        case ns__UloadResolution__200x100: ret = SCAN2_RESO_BW100DPI; break;
        case ns__UloadResolution__Normal:  ret = SCAN2_RESO_BW200DPI; break;
        case ns__UloadResolution__200:     ret = SCAN2_RESO_BW200DPI; break;
        case ns__UloadResolution__Auto:    ret = SCAN2_RESO_BW200DPI; break;
        case ns__UloadResolution__High:    ret = SCAN2_RESO_BW300DPI; break;
        case ns__UloadResolution__300:     ret = SCAN2_RESO_BW300DPI; break;
        case ns__UloadResolution__400:     ret = SCAN2_RESO_BW300DPI; break;
        case ns__UloadResolution__600:     ret = SCAN2_RESO_BW300DPI; break;
        default:                           ret = SCAN2_RESO_BW100DPI; break;
        }
    } else if(ColorMode == ns__ColorModeSelection__Color){
        switch(Resolution) {
        case ns__UloadResolution__Low:     ret = SCAN2_RESO_CL100DPI; break;
        case ns__UloadResolution__100:     ret = SCAN2_RESO_CL100DPI; break;
        case ns__UloadResolution__200x100: ret = SCAN2_RESO_CL100DPI; break;
        case ns__UloadResolution__200:     ret = SCAN2_RESO_CL200DPI; break;
        case ns__UloadResolution__Normal:  ret = SCAN2_RESO_CL300DPI; break;
        case ns__UloadResolution__300:     ret = SCAN2_RESO_CL300DPI; break;
        case ns__UloadResolution__400:     ret = SCAN2_RESO_CL300DPI; break;
        case ns__UloadResolution__High:    ret = SCAN2_RESO_CL600DPI; break;
        case ns__UloadResolution__600:     ret = SCAN2_RESO_CL600DPI; break;
        case ns__UloadResolution__Auto:    ret = SCAN2_RESO_CL300DPI; break;
        default:                           ret = SCAN2_RESO_CL300DPI; break;
        }
    }

    return ret;
#else
    UINT8           i;
    INT32           ret = CPAPI_P_VAL_SCAN_RESO_200;	/* ns__UloadResolution__Normalに相当する設定 */
    enum ns__ColorModeSelection ColorMode;
    enum ns__UloadResolution    Resolution;
    
    if(in_pColorMode == NULL || in_pResolution == NULL) {
        return ret;
    }
    
	if(StrictParamFlag == TRUE)
	{
		ret = IOSCAN_PARAM_ERROR;
    }
    
    ColorMode   = *in_pColorMode;
    Resolution  = *in_pResolution;

	if(StrictParamFlag == FALSE)
	{
		/* 丸め処理あり */
		for( i = 0; i < sizeof(Ftpclient_not_Strict_Resolution)/sizeof(Ftpclient_not_Strict_Resolution[0]); i++ )
		{
			if( Resolution == Ftpclient_not_Strict_Resolution[i].Resolution )
			{
				if(ColorMode == ns__ColorModeSelection__Mono)
				{
					/* Mono設定 */
					ret = Ftpclient_not_Strict_Resolution[i].mn_reso_param;
				}
				else if(ColorMode == ns__ColorModeSelection__Gray)
				{
					/* Gray設定 */
					ret = Ftpclient_not_Strict_Resolution[i].gr_reso_param;
				}
				else if(ColorMode == ns__ColorModeSelection__Color)
				{
					/* Color設定 */
					ret = Ftpclient_not_Strict_Resolution[i].cl_reso_param;
				}
			}
		}
	}
	else
	{
		/* 丸め処理なし */
		for( i = 0; i < sizeof(Ftpclient_Strict_Resolution)/sizeof(Ftpclient_Strict_Resolution[0]); i++ )
		{
			if( Resolution == Ftpclient_Strict_Resolution[i].Resolution )
			{
				if(ColorMode == ns__ColorModeSelection__Mono)
				{
					/* Mono設定 */
					ret = Ftpclient_Strict_Resolution[i].mn_reso_param;
				}
				else if(ColorMode == ns__ColorModeSelection__Gray)
				{
					/* Gray設定 */
					ret = Ftpclient_Strict_Resolution[i].gr_reso_param;
				}
				else if(ColorMode == ns__ColorModeSelection__Color)
				{
					/* Color設定 */
					ret = Ftpclient_Strict_Resolution[i].cl_reso_param;
				}
			}
		}
	}
	
	if(ret == IOSCAN_PARAM_ERROR)
	{
		ScanParamErrFlag = TRUE;
	}
	
    return ret;
#endif
}

/*****************************************************************************/
/**
 * @par     Serio/本体 両面Scan Parameter 変換
 * @param   cmd     [IN]        Serio Parameter
 * @retval  本体 Parameter
 * @note    表示用のTemp領域へ設定する Scan Color
 *          PanelではFileTypeによって、Colorの初期値を決めているため、
 *          本体設定用のマクロ名が無い
 *          CPAPI_P_VAL_SCAN_DUP_SCAN_OFF			0x00   FUNC_DUALSCAN_SIMPLEX :両面スキャン OFF
 *          CPAPI_P_VAL_SCAN_DUP_SCAN_LONG_EDGE		0x01   FUNC_DUALSCAN_DXLONG  :両面スキャン 両面
 *          CPAPI_P_VAL_SCAN_DUP_SCAN_SHORT_EDGE	0x02   FUNC_DUALSCAN_DXSHORT :両面スキャン 片面
**/
STATIC UINT8 
GetDualscan(enum xsd__boolean* DuplexScanEnable, enum xsd__boolean* ShortEdgeBinding)
{
#ifdef INK_MODEL_ONLY
    UINT8 ret = TMP_DXSSCAN_FUNCTION_OFF;

    if( (DuplexScanEnable == NULL) || (*DuplexScanEnable == xsd__boolean__false_) ) {
        ret = TMP_DXSSCAN_FUNCTION_OFF;
    } else if(*DuplexScanEnable == xsd__boolean__true_){
        if( (ShortEdgeBinding == NULL) || (*ShortEdgeBinding == xsd__boolean__false_) ) {
            ret = TMP_DXSSCAN_FUNCTION_2SIDE_LONG;
        } else if(*ShortEdgeBinding == xsd__boolean__true_){
            ret = TMP_DXSSCAN_FUNCTION_2SIDE_SHORT;
        }
    }
    return ret;
#else
    UINT8 ret = CPAPI_P_VAL_SCAN_DUP_SCAN_OFF;

	if(StrictParamFlag == TRUE)
	{
		ret = IOSCAN_PARAM_ERROR;
	}
	
    if( (DuplexScanEnable == NULL) || (*DuplexScanEnable == xsd__boolean__false_) ) {
        ret = CPAPI_P_VAL_SCAN_DUP_SCAN_OFF;
    } else if(*DuplexScanEnable == xsd__boolean__true_){
        if( (ShortEdgeBinding == NULL) || (*ShortEdgeBinding == xsd__boolean__false_) ) {
            ret = CPAPI_P_VAL_SCAN_DUP_SCAN_LONG_EDGE;
        } else if(*ShortEdgeBinding == xsd__boolean__true_){
            ret = CPAPI_P_VAL_SCAN_DUP_SCAN_SHORT_EDGE;
        }
    }
	
	if(ret == IOSCAN_PARAM_ERROR)
	{
		ScanParamErrFlag = TRUE;
	}
	
    return ret;
#endif
}

/*****************************************************************************/
/**
 * @par     Serio/本体 Scan Document SizeParameter 変換
 * @param   cmd     [IN]        Serio Parameter
 * @retval  本体 Parameter
**/
STATIC  UINT8 
get_scan_doc_size(enum ns__ScanAndUloadScansize* in_pscan_doc_size)
{
#ifdef INK_MODEL_ONLY /* TODO レーザ対応 */
    UINT8 ret = USW_SCANTOFTP_SCANSIZE_A4;
    if(in_pscan_doc_size != NULL) {
        switch(*in_pscan_doc_size){
        case ns__ScanAndUloadScansize__Letter       : ret = USW_SCANTOFTP_SCANSIZE_LETTER; break;
        case ns__ScanAndUloadScansize__Legal        : ret = USW_SCANTOFTP_SCANSIZE_LEGAL;  break;
        case ns__ScanAndUloadScansize__A4           :                                      break;
        case ns__ScanAndUloadScansize__A5           :                                      break;
        case ns__ScanAndUloadScansize__A6           :                                      break;
        case ns__ScanAndUloadScansize__B5           :                                      break;
        case ns__ScanAndUloadScansize__B6           :                                      break;
        case ns__ScanAndUloadScansize__BusinessCard :                                      break;
        default                                     :                                      break;
        }
    }
    return ret;
#else
    UINT8 ret = CPAPI_P_VAL_GLASS_SCAN_SIZE_A4;
    
	if(StrictParamFlag == TRUE)
	{
		ret = IOSCAN_PARAM_ERROR;
	}
	
    if(in_pscan_doc_size != NULL) {
        switch(*in_pscan_doc_size){
        case ns__ScanAndUloadScansize__Letter       : ret = CPAPI_P_VAL_GLASS_SCAN_SIZE_LETTER; break;
        case ns__ScanAndUloadScansize__Legal        : ret = CPAPI_P_VAL_GLASS_SCAN_SIZE_LEGAL;  break;
        case ns__ScanAndUloadScansize__A4           : ret = CPAPI_P_VAL_GLASS_SCAN_SIZE_A4;     break;
        case ns__ScanAndUloadScansize__A5           :                                           break;
        case ns__ScanAndUloadScansize__A6           :                                           break;
        case ns__ScanAndUloadScansize__B5           :                                           break;
        case ns__ScanAndUloadScansize__B6           :                                           break;
        case ns__ScanAndUloadScansize__BusinessCard :                                           break;
        default                                     :                                           break;
        }
    }
	
	if(ret == IOSCAN_PARAM_ERROR)
	{
		ScanParamErrFlag = TRUE;
	}
	
    return ret;
#endif
}

/*****************************************************************************/
/**
 * @par     Serio/本体 圧縮サイズ SizeParameter 変換
 * @param   cmd     [IN]        Serio Parameter
 * @retval  本体 Parameter
**/
STATIC UINT8 
get_scan_quality(enum ns__Selection3* compress_rate)
{
    UINT8   ret = CPAPI_P_VAL_FILESIZE_MIDDLE;                          /* 画質 */
    if(compress_rate != NULL){
        switch(*compress_rate){
        case ns__Selection3__Low   : ret = CPAPI_P_VAL_FILESIZE_LARGE;  break;
        case ns__Selection3__Normal: ret = CPAPI_P_VAL_FILESIZE_MIDDLE; break;
        case ns__Selection3__High  : ret = CPAPI_P_VAL_FILESIZE_SMALL;  break;
        default                    :                                    break;
        }
    }
    return ret;
}

/*****************************************************************************/
/**
 * @par     Serio Scan ProfileType 取得
 * @param   cmd     [IN]        Serio Parameter
 * @retval  Profile Union Type
**/
STATIC  INT32 
GetTxProfileType(SERIO_CNP_IOJOB_SCANSEND_T *iojob)
{
    INT32   ret = ERROR;
    if (iojob->TxProfiles.__TxProfiles != NULL){
        ret = iojob->TxProfiles.__TxProfiles[0].__union;
    }
    return ret;
}

/*****************************************************************************/
/**
 * @par     Serio/本体 File format
 * @param   cmd     [IN]        Serio Parameter
 * @retval  本体 Parameter
**/
STATIC  UINT8 
GetFileformat_SNW(enum ns__FileFormatSelection *FileType)
{
    UINT8       ret = CPAPI_P_VAL_SNW_FILETYPE_PDF;
    
	if(StrictParamFlag == TRUE)
	{
		ret = IOSCAN_PARAM_ERROR;
	}
	
	if(FileType != NULL){
		switch(*FileType){
		case ns__FileFormatSelection__TIFF : ret = CPAPI_P_VAL_SNW_FILETYPE_TIFF; break;
		case ns__FileFormatSelection__JPEG : ret = CPAPI_P_VAL_SNW_FILETYPE_JPEG; break;
		case ns__FileFormatSelection__PDF  : ret = CPAPI_P_VAL_SNW_FILETYPE_PDF;  break;
		case ns__FileFormatSelection__XPS  : ret = CPAPI_P_VAL_SNW_FILETYPE_XPS;  break;
		default                            :                                      break;
		}
	}
	
	if(ret == IOSCAN_PARAM_ERROR)
	{
		ScanParamErrFlag = TRUE;
	}
	
    return ret;
}

/*****************************************************************************/
/**
 * @par     Serio/本体 File format
 * @param   cmd     [IN]        Serio Parameter
 * @retval  本体 Parameter
**/
STATIC  UINT8 
GetFileformat_SFTP(enum ns__FileFormatSelection *FileType)
{
    UINT8       ret = CPAPI_P_VAL_SFTP_FILETYPE_PDF;
    
	if(StrictParamFlag == TRUE)
	{
		ret = IOSCAN_PARAM_ERROR;
	}
	
	if(FileType != NULL){
		switch(*FileType){
		case ns__FileFormatSelection__TIFF : ret = CPAPI_P_VAL_SFTP_FILETYPE_TIFF; break;
		case ns__FileFormatSelection__JPEG : ret = CPAPI_P_VAL_SFTP_FILETYPE_JPEG; break;
		case ns__FileFormatSelection__PDF  : ret = CPAPI_P_VAL_SFTP_FILETYPE_PDF;  break;
		case ns__FileFormatSelection__XPS  : ret = CPAPI_P_VAL_SFTP_FILETYPE_XPS;  break;
		default                            :                                       break;
		}
	}
	
	if(ret == IOSCAN_PARAM_ERROR)
	{
		ScanParamErrFlag = TRUE;
	}
	
    return ret;
}

/*****************************************************************************/
/**
 * @par     Serio/本体 SCAN実行における禁則処理に該当しているかの確認
 * @param   cmd     [IN]        ioscansend_param：電文から指示されたパラメータ
 * @retval  禁則処理に (OK：該当した、NG：該当しない)
**/
STATIC INT8  
ChkProhibitionSettings(IOScansend_exec_param_t* ioscansend_param)
{
	INT8	ret = OK;
	UINT8	ColorMode;
	UINT8	FileFormat;
	
	
	switch(ioscansend_param->ProfileType)
	{
		case PHXSERIO_PROFILE_TYPE_SMTP   :
			ColorMode  = ioscansend_param->TxProfile.Smtp.color_flag;
			FileFormat = ioscansend_param->TxProfile.Smtp.file_format;
			break;
		case PHXSERIO_PROFILE_TYPE_NETWORK:
			ColorMode  = ioscansend_param->TxProfile.Cifs.color_mode;
			FileFormat = ioscansend_param->TxProfile.Cifs.fileformat;
			break;
		case PHXSERIO_PROFILE_TYPE_FTP    :
			ColorMode  = ioscansend_param->TxProfile.Ftp.color_mode;
			FileFormat = ioscansend_param->TxProfile.Ftp.fileformat;
			break;
	}
	
	if(StrictParamFlag == FALSE)
	{
		/* 禁則処理を回避する丸め処理 */
		if(((ColorMode != CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_FILE_FORMAT_TIFF  ))
		|| ((ColorMode != CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_SNW_FILETYPE_TIFF ))
		|| ((ColorMode != CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_SFTP_FILETYPE_TIFF)))
		{
			ioscansend_param->TxProfile.Ftp.color_mode = CPAPI_P_VAL_SCAN_COLOR_TYPE_BW;
		}
		
		if(((ColorMode == CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_FILE_FORMAT_JPEG  ))
		|| ((ColorMode == CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_FILE_FORMAT_PDF   ))
		|| ((ColorMode == CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_SNW_FILETYPE_JPEG ))
		|| ((ColorMode == CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_SNW_FILETYPE_PDF  ))
		|| ((ColorMode == CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_SFTP_FILETYPE_JPEG))
		|| ((ColorMode == CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_SFTP_FILETYPE_PDF )))
		{
			ioscansend_param->TxProfile.Ftp.color_mode = CPAPI_P_VAL_SCAN_COLOR_TYPE_GRAY;
		}
	}
	else
	{
		/* 禁則処理チェック */
		if(((ColorMode != CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_FILE_FORMAT_TIFF  ))
		|| ((ColorMode == CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_FILE_FORMAT_JPEG  ))
		|| ((ColorMode == CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_FILE_FORMAT_PDF   ))
		|| ((ColorMode != CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_SNW_FILETYPE_TIFF ))
		|| ((ColorMode == CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_SNW_FILETYPE_JPEG ))
		|| ((ColorMode == CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_SNW_FILETYPE_PDF  ))
		|| ((ColorMode != CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_SFTP_FILETYPE_TIFF))
		|| ((ColorMode == CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_SFTP_FILETYPE_JPEG))
		|| ((ColorMode == CPAPI_P_VAL_SCAN_COLOR_TYPE_BW) && (FileFormat == CPAPI_P_VAL_SFTP_FILETYPE_PDF )))
		{
			ret = NG;
		}
	}
	
	return ret;
}


#ifdef SERIOCN_PRINT_DEBUG
/**
@fn GLOBAL VOID IOJobConnector_ScanSend_PrintParam(VOID *Param)
@brief 電文から受け取ったパラメータをデバッグプリントする
@param[in] Param: (SERIO_CNP_IOJOB_SCANSEND_T*) parsed xml struct.
@return VOID
*/

GLOBAL VOID IOJobConnector_ScanSend_PrintParam(VOID *Param)
{
    SERIO_CNP_IOJOB_SCANSEND_T *obj;
    struct ns__TxProfiles *profiles;
    UINT32 j, i;

    obj = (SERIO_CNP_IOJOB_SCANSEND_T *)Param;
    SERIOCN_PARAM_PRINT(("<< Parameters of ScanSend  begin>>"));
    
    /* UserId */
    if (obj->UserId != NULL){
        SERIOCN_PARAM_PRINT(("UserId: %s", obj->UserId));
    }

    /* ScanTray */
    if (obj->ScanTray != NULL){
        SERIOCN_PARAM_PRINT(("ScanTray: %s", enum_ScanTraySelection[*obj->ScanTray]));
    }

    /* ColorMode */
    if (obj->ColorMode != NULL){
        SERIOCN_PARAM_PRINT(("ColorMode: %s", enum_ColorModeSelection[*obj->ColorMode]));
    }
    
    /* Resolution */
    if (obj->Resolution != NULL){
        SERIOCN_PARAM_PRINT(("Resolution: %s", enum_UloadResolutionSelection[*obj->Resolution]));
    }
    
    /* DocSize */
    if (obj->DocSize != NULL){
        SERIOCN_PARAM_PRINT(("DocSize: %s", enum_UloadScanSizeSelection[*obj->DocSize]));
    }
    
    /* Density */
    if (obj->Density != NULL){
        SERIOCN_PARAM_PRINT(("Density: %s", enum_Selection5[*obj->Density]));
    }

    /* Brightness */
    if (obj->Brightness != NULL){
        SERIOCN_PARAM_PRINT(("Brightness: %s", enum_Selection5[*obj->Brightness]));
    }
    
    /* JpgQuality */
    if (obj->JpgQuality != NULL){
        SERIOCN_PARAM_PRINT(("JpgQuality: %s", enum_Selection3[*obj->JpgQuality]));
    }

    /* FileType */
    if (obj->FileType != NULL){
        SERIOCN_PARAM_PRINT(("FileType: %s", enum_FileFormatSelection[*obj->FileType]));
    }

    /* DuplexScanEnable */
    if (obj->DuplexScanEnable != NULL){
        SERIOCN_PARAM_PRINT(("DuplexScanEnable: %s", enum_boolean[*obj->DuplexScanEnable]));
    }

    /* ShortEdgeBinding */
    if (obj->ShortEdgeBinding != NULL){
        SERIOCN_PARAM_PRINT(("ShortEdgeBinding: %s", enum_boolean[*obj->ShortEdgeBinding]));
    }

    /* CmdRecvAckUrl */
    if (obj->CmdRecvAckUrl != NULL){
        SERIOCN_PARAM_PRINT(("CmdRecvAckUrl: %s", obj->CmdRecvAckUrl));
    }

    /* JobStartAckUrl */
    if (obj->JobStartAckUrl != NULL){
        SERIOCN_PARAM_PRINT(("JobStartAckUrl: %s", obj->JobStartAckUrl));
    }

    /* IoDataCommAckUrl */
    if (obj->IoDataCommAckUrl != NULL){
        SERIOCN_PARAM_PRINT(("IoDataCommAckUrl: %s", obj->IoDataCommAckUrl));
    }
    
    /* JobFinAckUrl */
    if (obj->JobFinAckUrl != NULL){
        SERIOCN_PARAM_PRINT(("JobFinAckUrl: %s", obj->JobFinAckUrl));
    }

    /* TxProfiles */
    profiles = &obj->TxProfiles;
    SERIOCN_PARAM_PRINT(("TxProfiles: "));
    for (j=0; j<profiles->__size__TxProfiles; ++j){
        switch (profiles->__TxProfiles[j].__union){
            case SOAP_UNION_ns__TxProfile_Smtp:
            {
                struct ns__SmtpProperty *smtpObj = &profiles->__TxProfiles[j].TxProfile.Smtp;
                SERIOCN_PARAM_PRINT(("  SMTP:"));
                for (i=0; i<smtpObj->__sizeDestination; ++i){
                    SERIOCN_PARAM_PRINT(("    Destination(%d): %s", i, smtpObj->Destination[i]));
                }
                if (smtpObj->Subject != NULL){
                    SERIOCN_PARAM_PRINT(("    Subject: %s", smtpObj->Subject));
                }
                if (smtpObj->MsgBody != NULL){
                    SERIOCN_PARAM_PRINT(("    MsgBody: %s", smtpObj->MsgBody));
                }
                break;
            }
            case SOAP_UNION_ns__TxProfile_Cifs:
                {
                    struct ns__CifsProperty *cifsObj = &profiles->__TxProfiles[j].TxProfile.Cifs;

                    SERIOCN_PARAM_PRINT(("  CIFS:"));
                    if (cifsObj->CifsParams != NULL){
                        if (cifsObj->CifsParams->Host != NULL){
                            SERIOCN_PARAM_PRINT(("    Host: %s", cifsObj->CifsParams->Host));
                        }
                        if (cifsObj->CifsParams->StoreDir != NULL){
                            SERIOCN_PARAM_PRINT(("    StoreDir: %s", cifsObj->CifsParams->StoreDir));
                        }
                        if (cifsObj->CifsParams->FileName != NULL){
                            SERIOCN_PARAM_PRINT(("    FileName: %s", cifsObj->CifsParams->FileName));
                        }
                        SERIOCN_PARAM_PRINT(("    AuthMethod: %s", enum_CifsAuthMethod[cifsObj->CifsParams->AuthMethod]));
                        if (cifsObj->CifsParams->User != NULL){
                            SERIOCN_PARAM_PRINT(("    User: %s", cifsObj->CifsParams->User));
                        }
                        if (cifsObj->CifsParams->Password != NULL){
                            SERIOCN_PARAM_PRINT(("    Password: %s", cifsObj->CifsParams->Password));
                        }
                        if (cifsObj->CifsParams->KerberosServer != NULL){
                            SERIOCN_PARAM_PRINT(("    KerberosServer: %s", cifsObj->CifsParams->KerberosServer));
                        }
                    }
    
                    break;
                }
            case SOAP_UNION_ns__TxProfile_Ftp:
            {
                struct ns__FtpProperty *ftpObj = &profiles->__TxProfiles[j].TxProfile.Ftp;
                SERIOCN_PARAM_PRINT(("  FTP:"));
                if (ftpObj->FtpParams != NULL){
                    if (ftpObj->FtpParams->FileName != NULL){
                        SERIOCN_PARAM_PRINT(("    FileName: %s", ftpObj->FtpParams->FileName));
                    }
                    if (ftpObj->FtpParams->Host != NULL){
                        SERIOCN_PARAM_PRINT(("    Host: %s", ftpObj->FtpParams->Host));
                    }
                    if (ftpObj->FtpParams->User != NULL){
                        SERIOCN_PARAM_PRINT(("    User: %s", ftpObj->FtpParams->User));
                    }
                    if (ftpObj->FtpParams->Password != NULL){
                        SERIOCN_PARAM_PRINT(("    Password: %s", ftpObj->FtpParams->Password));
                    }
                    if (ftpObj->FtpParams->StoreDir != NULL){
                        SERIOCN_PARAM_PRINT(("    StoreDir: %s", ftpObj->FtpParams->StoreDir));
                    }
                    SERIOCN_PARAM_PRINT(("    PassiveMode: %s", enum_boolean[ftpObj->FtpParams->PassiveMode]));
                    if (ftpObj->FtpParams->PortNum != NULL){
                        SERIOCN_PARAM_PRINT(("    PortNum: %s", ftpObj->FtpParams->PortNum));
                    }
                }
                break;
            }
            case SOAP_UNION_ns__TxProfile_Scan2file:
            {
                struct ns__Scan2fileProperty *s2fObj = &profiles->__TxProfiles[j].TxProfile.Scan2file;
                SERIOCN_PARAM_PRINT(("  Scan2File:"));
                if (s2fObj->Destination){
                    SERIOCN_PARAM_PRINT(("    Destination: %s", s2fObj->Destination));
                }
                break;
            }
            case SOAP_UNION_ns__TxProfile_Scan2email:
            {
                struct ns__Scan2emailProperty *s2eObj = &profiles->__TxProfiles[j].TxProfile.Scan2email;
                SERIOCN_PARAM_PRINT(("  Scan2Email:"));
                if (s2eObj->Destination){
                    SERIOCN_PARAM_PRINT(("    Destination: %s", s2eObj->Destination));
                }
                break;
            }
            case SOAP_UNION_ns__TxProfile_Scan2ocr:
            {
                struct ns__Scan2ocrProperty *s2oObj = &profiles->__TxProfiles[j].TxProfile.Scan2ocr;
                SERIOCN_PARAM_PRINT(("  Scan2OCR:"));
                if (s2oObj->Destination){
                    SERIOCN_PARAM_PRINT(("    Destination: %s", s2oObj->Destination));
                }
                break;
            }
    
        } /* switch */
    }

    SERIOCN_PARAM_PRINT(("<< Parameters of ScanSend  end>>"));
}


#endif /** SERIOCN_PRINT_DEBUG **/

#endif /* (defined(USE_BSI) || defined(USE_PHOENIX_SERIO_MOBILE_IF)) && defined(USE_SCAN) */
#endif	/* if 0 */

