// Add to includes:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
#	include "PythonMountUpGrade.h"
#endif

// Before
	if ("Loading" != m_strPhase)
		m_phaseLeaveFunc.Run();

	Tracen("");
	Tracen("## Network - Loading Phase ##");
	Tracen("");

	m_strPhase = "Loading";

	m_dwChangingPhaseTime = ELTimer_GetMSec();
	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::LoadingPhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveLoadingPhase);

	CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
	rkPlayer.Clear();

	CFlyingManager::Instance().DeleteAllInstances();
	CEffectManager::Instance().DeleteAllInstances();

	CPythonApplication::Instance().SetDefaultCamera();
#if defined(ENABLE_EVENT_BANNER)
	CPythonInGameEventSystemManager::Instance().Clear();
#endif


	__DirectEnterMode_Initialize();
}

bool CPythonNetworkStream::RecvMainCharacter()
{

// After
	if ("Loading" != m_strPhase)
		m_phaseLeaveFunc.Run();

	Tracen("");
	Tracen("## Network - Loading Phase ##");
	Tracen("");

	m_strPhase = "Loading";

	m_dwChangingPhaseTime = ELTimer_GetMSec();
	m_phaseProcessFunc.Set(this, &CPythonNetworkStream::LoadingPhase);
	m_phaseLeaveFunc.Set(this, &CPythonNetworkStream::__LeaveLoadingPhase);

	CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
	rkPlayer.Clear();

	CFlyingManager::Instance().DeleteAllInstances();
	CEffectManager::Instance().DeleteAllInstances();

	CPythonApplication::Instance().SetDefaultCamera();
#if defined(ENABLE_EVENT_BANNER)
	CPythonInGameEventSystemManager::Instance().Clear();
#endif

#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	CPythonMountUpGrade::Instance().ClearHandler();
	CPythonMountUpGrade::Instance().Reset();
#endif

	__DirectEnterMode_Initialize();
}

bool CPythonNetworkStream::RecvMainCharacter()
{
