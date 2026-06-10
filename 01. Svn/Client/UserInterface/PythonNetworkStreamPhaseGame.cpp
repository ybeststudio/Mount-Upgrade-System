// Add to includes:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	#include "PythonMountUpGrade.h"
#endif

// In `void CPythonNetworkStream::GamePhase()`, extend the switch statement with:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
			case HEADER_GC_MOUNT_UP_GRADE:
				ret = RecvMountUpGrade();
				break;

			case HEADER_GC_MOUNT_UP_GRADE_CHAT:
				ret = RecvMountUpGradeChat();
				break;
#endif

// Before
	for (int i = 0; i < SKILL_MAX_NUM; ++i)
	{
		const TPlayerSkill& rPlayerSkill = packet.skills[i];


		if (i >= 112 && i <= 115 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(7, i);

		if (i >= 116 && i <= 119 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(8, i);

#if defined(ENABLE_678TH_SKILL)
		if (i >= 221 && i <= 229 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(7, i);

		if (i >= 236 && i <= 244 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(8, i);
#endif

		rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
	}

// After
	for (int i = 0; i < SKILL_MAX_NUM; ++i)
	{
		const TPlayerSkill& rPlayerSkill = packet.skills[i];

#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
		if (i >= 285 && i <= 311 && rPlayerSkill.bLevel > 0)
			rkPlayer.SetSkill(i, i);
#endif

		if (i >= 112 && i <= 115 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(7, i);

		if (i >= 116 && i <= 119 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(8, i);

#if defined(ENABLE_678TH_SKILL)
		if (i >= 221 && i <= 229 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(7, i);

		if (i >= 236 && i <= 244 && rPlayerSkill.bLevel)
			rkPlayer.SetSkill(8, i);
#endif

		rkPlayer.SetSkillLevel_(i, rPlayerSkill.bMasterType, rPlayerSkill.bLevel);
	}

// Add the following `CPythonNetworkStream::MountUpGrade` function anywhere in this file:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
bool CPythonNetworkStream::MountUpGrade(const uint8_t iSubHeader, const uint32_t arg)
{
	if (!__CanActMainInstance())
		return true;

	TPacketCGMountUpGrade p =
	{
		static_cast<uint8_t>(HEADER_CG_MOUNT_UP_GRADE),
		static_cast<uint8_t>(iSubHeader),
		static_cast<uint32_t>(arg),
	};

	if (!Send(sizeof(p), &p))
	{
		Tracen("CPythonNetworkStream::MountUpGrade Error");
		return false;
	}

	return SendSequence();
}

bool CPythonNetworkStream::RecvMountUpGrade()
{
	TPacketGCMountUpGrade p;
	if (!Recv(sizeof(p), &p))
	{
		Tracen("RecvOpenMountUpGrade Error");
		return false;
	}

	if (!CPythonMountUpGrade::Instance().GetHandler())
	{
		if (m_apoPhaseWnd[PHASE_WINDOW_GAME])
			PyCallClassMemberFunc(m_apoPhaseWnd[PHASE_WINDOW_GAME], "RegisterMountUpgradeHandler", Py_BuildValue("()"));
	}

	CPythonMountUpGrade::Instance().Process(p.subheader, p.horse_level, p.is_fail, p.existing_exp);

	return true;
}

bool CPythonNetworkStream::RecvMountUpGradeChat()
{
	TPacketGCMountUpGradeChat p;
	if (!Recv(sizeof(p), &p))
	{
		Tracen("RecvMountUpGradeChat Error");
		return false;
	}

	CPythonMountUpGrade::Instance().Chat(p.type, p.value);

	return true;
}
#endif
