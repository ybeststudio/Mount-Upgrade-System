// Add to includes:
#if defined(__MOUNT_UPGRADE__)
	#include "mount_up_grade.h"
#endif

// Add the following `CInputMain::MountUpGrade` function anywhere in this file:
#if defined(__MOUNT_UPGRADE__)
void CInputMain::MountUpGrade(LPCHARACTER ch, const char* c_pData)
{
	if (!ch)
	{
		sys_err("CInputMain::MountUpGrade - Unknown identifier)");
		return;
	}
	
	const auto* p = reinterpret_cast<const TPacketCGMountUpGrade*>(c_pData);
	
	if (!p) return;
	
	switch (p->iSubHeader)
	{
	case CMountUpGrade::EMountUpGradeCGSubheaderType::SUBHEADER_CG_MOUNT_UP_GRADE_EXP:
		CMountUpGrade::Instance().SetExp(ch);
		break;

	case CMountUpGrade::EMountUpGradeCGSubheaderType::SUBHEADER_CG_MOUNT_UP_GRADE_LEVEL_UP:
		CMountUpGrade::Instance().SetLevel(ch);
		break;

	case CMountUpGrade::EMountUpGradeCGSubheaderType::SUBHEADER_CG_MOUNT_UP_GRADE_CLOSE:
		break;

	case CMountUpGrade::EMountUpGradeCGSubheaderType::SUBHEADER_CG_MOUNT_UP_GRADE_SKILL_LEVEL_UP:
		CMountUpGrade::Instance().SetSkillLevelUp(ch, p->arg);
		break;

	default:
		sys_err("Unknown iSubHeader (Name: %s) - (iSubHeader: %d)", ch->GetName(), p->iSubHeader);
		return;
	}
}
#endif

// In `int CInputMain::Analyze(LPDESC d, BYTE bHeader, const char* c_pData)`, extend the switch statement with:
#if defined(__MOUNT_UPGRADE__)
		case HEADER_CG_MOUNT_UP_GRADE:
			MountUpGrade(ch, c_pData);
			break;
#endif
