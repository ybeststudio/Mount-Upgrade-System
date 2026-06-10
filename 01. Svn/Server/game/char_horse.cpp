// Before
		else if (GetHorseStamina() <= 0)
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("Your Horse's endurance is too low."));

		return false;
	}

	HorseSummon(false);

	MountVnum(dwMountVnum);

#if defined(ENABLE_PASSIVE_SYSTEM)
	RemoveAffect(AFFECT_PASSIVE_RELIC_DISMOUNT_SPEED);
#endif


	if (test_server)
		sys_log(0, "Ride Horse : %s ", GetName());

	return true;
}

bool CHARACTER::StopRiding()
{

// After
		else if (GetHorseStamina() <= 0)
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("Your Horse's endurance is too low."));

		return false;
	}

	HorseSummon(false);

	MountVnum(dwMountVnum);

#if defined(ENABLE_PASSIVE_SYSTEM)
	RemoveAffect(AFFECT_PASSIVE_RELIC_DISMOUNT_SPEED);
#endif

#if defined(__MOUNT_UPGRADE__)
	RefreshMountUpgradeSkillAffect();
#endif

	if (test_server)
		sys_log(0, "Ride Horse : %s ", GetName());

	return true;
}

bool CHARACTER::StopRiding()
{

// Before
	if (CHorseRider::StopRiding())
	{
		quest::CQuestManager::instance().Unmount(GetPlayerID());

		if (!IsDead() && !IsStun())
		{
			DWORD dwOldVnum = GetMountVnum();
			MountVnum(0);

			HorseSummon(true, false, dwOldVnum);
		}
		else
		{
			m_dwMountVnum = 0;

			ComputePoints();
			UpdatePacket();
		}

		PointChange(POINT_ST, 0);
		PointChange(POINT_DX, 0);
		PointChange(POINT_HT, 0);
		PointChange(POINT_IQ, 0);
		return true;
	}

// After
	if (CHorseRider::StopRiding())
	{
		quest::CQuestManager::instance().Unmount(GetPlayerID());

		if (!IsDead() && !IsStun())
		{
			DWORD dwOldVnum = GetMountVnum();
			MountVnum(0);

			HorseSummon(true, false, dwOldVnum);
		}
		else
		{
			m_dwMountVnum = 0;

			ComputePoints();
			UpdatePacket();
		}

		PointChange(POINT_ST, 0);
		PointChange(POINT_DX, 0);
		PointChange(POINT_HT, 0);
		PointChange(POINT_IQ, 0);
#if defined(__MOUNT_UPGRADE__)
		PointChange(POINT_SUNGMA_STR, 0);
		PointChange(POINT_SUNGMA_HP, 0);
		PointChange(POINT_SUNGMA_MOVE, 0);
		PointChange(POINT_SUNGMA_IMMUNE, 0);
		RefreshMountUpgradeSkillAffect();
#endif
		return true;
	}

// Before
	if (IsRiding())
	{
			return true;
		else
			return false;

		if (GetMountVnum())
		{
			if (GetMountVnum() >= 20209 && GetMountVnum() <= 20212)
				return true;

			if (CMobVnumHelper::IsRamadanBlackHorse(GetMountVnum()))
				return true;
		}
		else
			return false;
	}

// After
	if (IsRiding())
	{
#if defined(__MOUNT_UPGRADE__)
		if (GetHorseGrade() >= 3)
#else
		if (GetHorseGrade() == 3)
#endif
			return true;
		else
			return false;

		if (GetMountVnum())
		{
			if (GetMountVnum() >= 20209 && GetMountVnum() <= 20212)
				return true;

			if (CMobVnumHelper::IsRamadanBlackHorse(GetMountVnum()))
				return true;
		}
		else
			return false;
	}
