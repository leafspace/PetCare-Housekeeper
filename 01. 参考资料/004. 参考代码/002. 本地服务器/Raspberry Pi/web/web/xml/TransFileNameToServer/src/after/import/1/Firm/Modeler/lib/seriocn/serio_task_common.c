/************************************************************************
 *
 *	serio_task_common.c - serio�Ŏg�p���鋤�ʊ֐��Q
 *
 *	Copyright: 2010-20XX brother Industries , Ltd.
 *
 *	$Id: //depot/Firm/Commonfile/Laser_origin2/task/serio/serio_task_common.c#2 $
 *	$DateTime: 2011/02/22 20:59:46 $
 *	$Change: 205973 $
 *	$Author: sugiyatk $
 *
 *	ver 1.0.0 : 2010.07.19 : ABS  : �V�K�쐬
 ************************************************************************/

/****** �C���N���[�h�E�t�@�C�� ******************************************/
#include "serio_task_common.h"
#include "componentlib/serio/serio.h"
#include "componentlib/serio/serio_log_rec.h"
#include "task/panel/panelTask.h"

#include "lib/cplib/cp_api_lib.h"

#include "task/cp/cp_task.h"

#include "serio_seriomfp_app.h"

#include "lib/funcset/funcset.h"
#include "lib/funcset/funcsetcntl.h"

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
STATIC	BOOL	CloudBSILockModeFlag = FALSE;
STATIC	BOOL	CloudBSIMsgSendtoPanelFlag = TRUE;
#endif	/* USE_CBSI */
GLOBAL UINT8	g_filename[ SCAN2FTP_FNAME_MAXSIZE ];
GLOBAL UINT8	g_filename_t[ SCAN2FTP_FNAME_MAXSIZE ];
/*********************************************************************************************/
/**
* @par		(serio)JOB�X�e�[�^�X�̒ʒm
* @param	Status(input) �W���u�X�e�[�^�X
* @retval	�Ȃ�
* @par	<�O���d�l>
*			Serio�^�X�N��JOB�X�e�[�^�X��ʒm����
* @par	<�����d�l>
* 			�����Ŏw�肳�ꂽJOB�X�e�[�^�X��Serio�^�X�N�֒ʒm����
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
	jobsts.Param.End.Reason = 0;		/* TODO:������ */
	/*jobsts.Param.End.Detail; */
	SerioFwEventNotify(SERIOFW_EVID_JOBSTATUS, &jobsts);
}

/*********************************************************************************************/
/**
* @par		(serio)JOB�X�e�[�^�X�̎��s�I����ʒm
* @param	Reason(input) �W���u�I���v��
* @retval	�Ȃ�
* @par	<�O���d�l>
*			Serio�^�X�N��JOB�X�e�[�^�X�̎��s�I����ʒm����
* @par	<�����d�l>
* 			�����Ŏw�肳�ꂽ�W���u�I���v����Serio�^�X�N�֒ʒm����
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
* @par		(serio)JOB�X�e�[�^�X�̎��s��~����ʒm
* @param	Reason(input)  �W���u��~�v��
* @param	SubCode(input) �T�u�R�[�h
* @retval	�Ȃ�
* @par	<�O���d�l>
*			Serio�^�X�N��JOB�X�e�[�^�X�̎��s��~����ʒm����
* @par	<�����d�l>
* 			�����Ŏw�肳�ꂽ�W���u��~�v���ƃT�u�R�[�h��Serio�^�X�N�֒ʒm����
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
* @par		(serio)UI���͏I���ʒm���s��
* @param	Reason(input)  �W���u��~�v��
* @param	SubCode(input) �T�u�R�[�h
* @retval	�Ȃ�
* @par	<�O���d�l>
*			Serio�^�X�N��UI���͏I����ʒm����
* @par	<�����d�l>
* 			���s��~���̂Ƃ��̂݁A���̊֐����Ă΂�� \n
* 			Serio�^�X�N��UI���͏I���ʒm���s���A�������ĊJ����
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
* @par		(serio)JOB�i����1����]���I����ʒm
* @param	�Ȃ�
* @retval	�Ȃ�
* @par	<�O���d�l>
*			Serio�^�X�N��JOB�i����1����]���I����ʒm����
* @par	<�����d�l>
* 			�����Ŏw�肳�ꂽ�ʐM�A�Ԃ�Serio�^�X�N�֒ʒm����
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
    
	memcpy(g_filename_t, g_filename, sizeof(g_filename));
	JobProg.Type  = SERIOFW_JOBPROG_TRANSEND;
	JobProg.Param.TransferEnd.FileNam = (MD_CHAR *)g_filename_t;				/* TODO : dummy�̂� */
	memset(g_filename, 0x00, sizeof(g_filename));
	SerioFwEventNotify(SERIOFW_EVID_JOBPROGRESS, &JobProg);
}



/*********************************************************************************************/
/**
* @par		(serio)JOB�i����1Page����I����ʒm
* @param	�Ȃ�
* @retval	�Ȃ�
* @par	<�O���d�l>
*			Serio�^�X�N��JOB�i����1Page����I����ʒm����
* @par	<�����d�l>
* 			�����Ŏw�肳�ꂽ�ʐM�A�Ԃ�Serio�^�X�N�֒ʒm����
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
//	JobProg.Param.PageEnd.dummy;				/* TODO : dummy�̂� */

	SerioFwEventNotify(SERIOFW_EVID_JOBPROGRESS, &JobProg);
}



/*********************************************************************************************/
/**
* @par		(serio)JOB���s�̊J�n
* @param	�Ȃ�
* @retval	�Ȃ�
* @par	<�O���d�l>
*			JOB���s�̊J�n�ݒ������
* @par	<�����d�l>
* 			�W���u���s���t���O��ON�ɂ���
*/
/*********************************************************************************************/
GLOBAL	VOID 
IoJobStart(VOID)
{
	JobRunning = TRUE;
}


/*********************************************************************************************/
/**
* @par		(serio)JOB���s�̏I��
* @param	�Ȃ�
* @retval	�Ȃ�
* @par	<�O���d�l>
*			JOB���s�̏I���ݒ������
* @par	<�����d�l>
* 			�W���u���s���t���O��OFF�ɂ���
*/
/*********************************************************************************************/
GLOBAL	VOID 
IoJobEnd(VOID)
{
	JobRunning = FALSE;
}

/*********************************************************************************************/
/**
* @par		(serio)JOB���s�����̃`�F�b�N
* @param	�Ȃ�
* @retval	�Ȃ�
* @par	<�O���d�l>
*			JOB���s�����̃`�F�b�N���s��
* @par	<�����d�l>
* 			�W���u���s���t���O��Ԃ�
*/
/*********************************************************************************************/
GLOBAL	BOOL 
IsIoJobRunning(VOID)
{
	return JobRunning;
}


/*********************************************************************************************/
/**
* @par		(serio)���u��ԃG���[���������̃`�F�b�N
* @param	�Ȃ�
* @retval	�Ȃ�
* @par	<�O���d�l>
*			���u��ԃG���[���������̃`�F�b�N���s��
* @par	<�����d�l>
* 			���u��ԃG���[�t���O��Ԃ�
*/
/*********************************************************************************************/
GLOBAL BOOL
IsStateError(VOID)
{
	return StateError;
}

/*********************************************************************************************/
/**
* @par		(serio)���u��ԃG���[�t���O��ݒ肷��
* @param	Error : ���u��ԃG���[�t���O(TRUE:���u��ԃG���[�����AFALSE:���u��ԃG���[�Ȃ�)
* @retval	�Ȃ�
* @par	<�O���d�l>
*			���u��ԃG���[�t���O��ݒ肷��
* @par	<�����d�l>
* 			���u��ԃG���[�t���O��ݒ肷��
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
* @par		�P�[�u���̐ڑ��̏�Ԃ��`�F�b�N����
* @param	�Ȃ�
* @retval	TRUE	�ڑ�����
* @retval	FASE	�ڑ��Ȃ�
* @par	<�O���d�l>
* 			USB�AFTP�AEMS�AFTP�̐ڑ���Ԃ��`�F�b�N����
* @par	<�����d�l>
* 			USB�AFTP�AEMS�AFTP�̐ڑ���Ԃ��`�F�b�N����
*/
/*********************************************************************************************/
GLOBAL	BOOL
check_scanmode_cable_serio(VOID)
{
	/* uiframe��UifNline_com.h��panelTask.h�𓯎��Ɏg�p����ƁA */
	/* KEY_AVAILABLE�����łɒ�`����Ă���x�����������邽��    */
	/* �x����������邽�߂� panelTask.h�̊֐����ĂԂ����̏���   */
	return check_scanmode_cable();
}
#endif	/* USE_SCAN */

#ifdef USE_SERIO_LOG
/*********************************************************************************************/
/**
* @par		BSI����User�ɂ��IoJob�̗��p���ʂ�Log�ɕۑ�����
* @param	serio_job(in)  �FIoJob��ʏ��
* @param	Reason(in)     �F���p���ʏ��
* @retval	�Ȃ�
* @par	<�O���d�l>
* 			BSI����User�ɂ��IoJob�̗��p���ʂ�Log�ɕۑ�����
* @par	<�����d�l>
* 			BSI����User�ɂ��IoJob�̗��p���ʂ�E2PROM�ɕۑ�����
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
		/* Scan�̎��s�I������Job�I����Log���c�� */
		SerioLog_Rec_End(LOG_DATA_FUNC_SCAN, err_reason);
	}
	
}
#endif	/* USE_SERIO_LOG */

/**
**************************************************************************
* @par (GLOBAL) Serio�ŋ��L����CpTask�ւ̃��b�Z�[�W�ʒm
* @param  ���b�Z�[�WFrom Task
*         ���b�Z�[�W�R�}���hID
* @return 
* @retval 
* @par <�O���d�l>
*      
* @par <�����d�l>
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
	
	/* �ʒm���e�̐ݒ� */
	com_buff.cmd_id	= cmd_id;
	com_buff.from_task= from_task;
	send_buff = (UINT8 *)&com_buff;
	/* �ʒm�T�C�Y�̐ݒ� */
	size = sizeof(TASK_MSG_COM_T);
	
	/* CpTask�̃��b�Z�[�W�L���[ID�擾 */
	cp_qid = FOS_MSGGETID(CP_MSG_NAME);
	
	/* Task�փ��b�Z�[�W���M */
	FOS_MSGSEND(cp_qid, send_buff, size);
	
	return;
}

#ifdef	USE_CBSI
/*********************************************************************************************/
/**
* @par		(CloudBSI)CloudBSI���ݒ�󋵊m�F
* @param	�Ȃ�
* @retval	CloudBSIInfoFlag�FCloudBSI�ݒ��
* @par	<�O���d�l>
*			CloudBSI�̏��Server����S�Đݒ肳�ꂽ�����m�F����B
* @par	<�����d�l>
* 			CloudBSI�̏��Server����S�Đݒ肳�ꂽ�����m�F����B
*/
/*********************************************************************************************/
GLOBAL INT32 Is_CloudBSIInfoGet( VOID )
{
	return CloudBSIInfoFlag;
}

/*********************************************************************************************/
/**
* @par		(CloudBSI)CloudBSI���ݒ�󋵃Z�b�g
* @param	flag(input) CloudBSI���ݒ��
* @retval	�Ȃ�
* @par	<�O���d�l>
*			CloudBSI�̏�񂪐ݒ蒆���ݒ�ς��̏󋵂��t���O�ɃZ�b�g����B
* @par	<�����d�l>
* 			CloudBSI�̏�񂪐ݒ蒆���ݒ�ς��̏󋵂��t���O�ɃZ�b�g����B
*/
/*********************************************************************************************/
GLOBAL VOID Set_CloudBSIInfoFlag( INT32 flag )
{
	CloudBSIInfoFlag = flag;

#ifdef	USE_SEPARATE_UI
	if(CloudBSIInfoFlag == TRUE)
	{
		if( CloudBSIMsgSendtoPanelFlag == TRUE )
		{
			/* �ݒ肪���������ꍇ�́A�Q�Ɨp�̃o�b�t�@�փR�s�[���� */
			seriomfp_cloudbsi_appinfo_copy_to_tempbuff();
			Serio_SendMsgToCp( SERIO_LIBRARY, CPAPI_CBSI_INFO_GET_STATUS_FIN );
			Set_CloudBSIMsgSendtoPanelFlag( FALSE );
		}
        /*�T�[�o��ݒ肷�郁�j���[���ڂ�CloudBSI FunSW�� */
		Set_CloudBSIFunSwitchID();
	}
#endif	/* USE_SEPARATE_UI */
}

/*********************************************************************************************/
/**
* @par		(CloudBSI)CloudBSI��Lock��Ԃ̖{�ݒ�
* @param	flag(input) LockMode�̗L�������ݒ�
* @retval	�Ȃ�
* @par	<�O���d�l>
*			CloudBSI��Lock��Ԃ�ݒ肷��B
* @par	<�����d�l>
* 			CloudBSI��Lock��Ԃ������t���O��ݒ肷��B
*/
/*********************************************************************************************/
GLOBAL VOID
Set_CloudBSILockMode( BOOL flag )
{
	CloudBSILockModeFlag = flag;

	if(flag == TRUE)
	{
		/* ����N�����̂��߂�Lock_ui�L�����L������ */
		sete2p_data(E2P_CBSI_LOCK_UI_ADR, TRUE);
	}
	else
	{
		/* ����N�����̂��߂�Lock_ui�������L������ */
		sete2p_data(E2P_CBSI_LOCK_UI_ADR, FALSE);
	}
}

/*********************************************************************************************/
/**
* @par		(CloudBSI)CloudBSI��Lock��Ԏ擾
* @param	�Ȃ�
* @retval	LockMode�̗L������
* @par	<�O���d�l>
*			CloudBSI��Lock��Ԃ��擾����B
* @par	<�����d�l>
* 			CloudBSI��Lock��Ԃ������t���O�̏�Ԃ��擾����B
*/
/*********************************************************************************************/
GLOBAL BOOL
Get_IsCloudBSILockMode( VOID )
{
	return CloudBSILockModeFlag;
}

/*********************************************************************************************/
/**
* @par		(CloudBSI)���擾��Ԃ���̏��ݒ肪���邩�A��������ݒ肷��B
* @param	flag(input) ���擾��Ԃ���̏��ݒ�L��
* @retval	�Ȃ�
* @par	<�O���d�l>
*			���擾��Ԃ���̏��ݒ�L����ݒ肷��B
* @par	<�����d�l>
* 			���擾��Ԃ���̏��ݒ�L���������t���O�̏�Ԃ�ݒ肷��B�B
*/
/*********************************************************************************************/
GLOBAL VOID
Set_CloudBSIMsgSendtoPanelFlag( BOOL flag )
{
	CloudBSIMsgSendtoPanelFlag = flag;

}
#endif	/* USE_CBSI */

#ifdef USE_BSI
GLOBAL VOID 
SerioUIJobCompulsiveCancel( VOID )
{
	serio_app_type_t app_type;
	
	app_type = get_app_trigger_type();
	
	if( (app_type == SERIO_APP_TYPE_BSI) || (app_type == SERIO_APP_TYPE_CLOUDBSI) )
	{
		SerioUIJobCompulsiveNotify();
	}
}

#endif	/* USE_CBSI */

#endif /* USE_SERIO */
