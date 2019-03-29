/************************************************************************
 *
 *	serio_task_common.h - serioで使用する共通関数群
 *
 *	Copyright: 2010-20XX brother Industries , Ltd.
 *
 *	$Id: //depot/Firm/Commonfile/Laser_origin2/task/serio/serio_task_common.h#2 $
 *	$DateTime: 2011/02/22 20:59:46 $
 *	$Change: 205973 $
 *	$Author: sugiyatk $
 *
 *	ver 1.0.0 : 2010.07.19 : ABS  : 新規作成
 ************************************************************************/

#ifndef SERIO_TASK_COMMON_H
#define SERIO_TASK_COMMON_H

/****** インクルード・ファイル ******************************************/
#include "spec.h"
#include "stdtype.h"
#include "fos.h"
#include "debug.h"
#include "common.h"

#ifdef USE_SERIO

#include "componentlib/hakkolib/job.h"

#define WITH_SOAPDEFS_H
#include "componentlib/serio/serio.h"
#include "componentlib/serio/serio_net_bridge.h"

/*****************************************************************************
 * Local function prototypes.
 ****************************************************************************/
GLOBAL	VOID SendJobStatus(UINT16 Status, serio_job_category_t serio_job);
GLOBAL	VOID SendJobStatus_End(INT32 Reason, INT32 ErrorInfo, serio_job_category_t serio_job);
GLOBAL	VOID SendJobStatus_Paused(UINT16 Reason, UINT16 SubCode, serio_job_category_t serio_job);
GLOBAL	VOID SendUiEnd_Restart(UINT16 Reason, UINT16 SubCode);
GLOBAL	VOID SendJobProgress_TransEnd(serio_job_category_t serio_job);
GLOBAL	VOID SendJobProgress_PrintPageEnd(serio_job_category_t serio_job);
GLOBAL	VOID SendJobProgress_TransFileName(serio_job_category_t serio_job, UINT8 *filename);

GLOBAL	VOID IoJobStart(VOID);
GLOBAL	VOID IoJobEnd(VOID);
GLOBAL	BOOL IsIoJobRunning(VOID);
GLOBAL  BOOL IsStateError(VOID);
GLOBAL  VOID SetStateError(BOOL Error);
GLOBAL  VOID Serio_SendMsgToCp(UINT16 from_task, UINT16 cmd_id);

GLOBAL	BOOL Is_CloudBSIInfoGet( VOID );
GLOBAL	VOID Set_CloudBSIInfoFlag( BOOL flag );

#ifdef	USE_SCAN
GLOBAL	BOOL check_scanmode_cable_serio(VOID);
#endif	/* USE_SCAN */

GLOBAL	VOID set_iojob_end_bsilog(  serio_job_category_t serio_job, INT32 Reason );

#endif /* USE_SERIO */
#endif
