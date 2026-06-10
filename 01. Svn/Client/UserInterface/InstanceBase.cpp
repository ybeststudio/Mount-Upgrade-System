// In `GetMountLevelByVnum(DWORD dwMountVnum, bool IsNew)`, extend the switch statement with:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
		case 20149: // normal military horse (no guild)
		case 20150: // normal military horse (guild member)
		case 20151: // normal military horse (guild master)
#endif

// Before
	if ((dwHorseVnum >= 20101 && dwHorseVnum <= 20109) ||
		(dwHorseVnum == 20029 || dwHorseVnum == 20030))
		return 100.0f;


	float fRaceHeight = CRaceManager::Instance().GetRaceHeight(dwHorseVnum);
	if (fRaceHeight == 0.0f)
		return 100.0f;
	else
		return fRaceHeight;
}

CActorInstance& CInstanceBase::GetGraphicThingInstanceRef()
{

// After
	if ((dwHorseVnum >= 20101 && dwHorseVnum <= 20109) ||
		(dwHorseVnum == 20029 || dwHorseVnum == 20030))
		return 100.0f;

#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	if (dwHorseVnum >= 20149 && dwHorseVnum <= 20151)
		return 100.0f;
#endif

	float fRaceHeight = CRaceManager::Instance().GetRaceHeight(dwHorseVnum);
	if (fRaceHeight == 0.0f)
		return 100.0f;
	else
		return fRaceHeight;
}

CActorInstance& CInstanceBase::GetGraphicThingInstanceRef()
{
