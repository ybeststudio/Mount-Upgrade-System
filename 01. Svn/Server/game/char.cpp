// Find this line:
m_chRider = NULL;

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	m_mount_up_grade_exp = 0;
	m_mount_up_grade_fail = 0;
#endif

// In `void CHARACTER::Initialize()`, find this block:
m_bIsLoadedAffect = false;

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	m_bRefreshMountUpgradeSkillAffect = false;
#endif

// Find this line:
tab.skill_group = m_points.bSkillGroup;

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	tab.mount_up_grade_exp = GetMountUpGradeExp();
	tab.mount_up_grade_fail = IsMountUpGradeFail();
#endif

// Find this line:
SetGem(t->gem);

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	SetMountUpGradeExp(t->mount_up_grade_exp);
	SetMountUpGradeFail(t->mount_up_grade_fail);
#endif

// In `void CHARACTER::SetProto(const CMob* pkMob)`, extend the related check like this:
#if defined(__MOUNT_UPGRADE__)
		|| GetRaceNum() == 20149
		|| GetRaceNum() == 20150
		|| GetRaceNum() == 20151
#endif

// Before
			if (GetHorseIQ() > GetPoint(POINT_IQ))
				PointChange(POINT_IQ, GetHorseIQ() - GetPoint(POINT_IQ));

		}
	}

	ComputeBattlePoints();

	if (iMaxHP != GetMaxHP())
	{

// After
			if (GetHorseIQ() > GetPoint(POINT_IQ))
				PointChange(POINT_IQ, GetHorseIQ() - GetPoint(POINT_IQ));

#if defined(__MOUNT_UPGRADE__)
			if (GetHorseSungMaST() > GetPoint(POINT_SUNGMA_STR))
				PointChange(POINT_SUNGMA_STR, GetHorseSungMaST() - GetPoint(POINT_SUNGMA_STR));

			if (GetHorseSungMaDX() > GetPoint(POINT_SUNGMA_HP))
				PointChange(POINT_SUNGMA_HP, GetHorseSungMaDX() - GetPoint(POINT_SUNGMA_HP));

			if (GetHorseSungMaHT() > GetPoint(POINT_SUNGMA_MOVE))
				PointChange(POINT_SUNGMA_MOVE, GetHorseSungMaHT() - GetPoint(POINT_SUNGMA_MOVE));

			if (GetHorseSungMaIQ() > GetPoint(POINT_SUNGMA_IMMUNE))
				PointChange(POINT_SUNGMA_IMMUNE, GetHorseSungMaIQ() - GetPoint(POINT_SUNGMA_IMMUNE));
#endif
		}
	}

	ComputeBattlePoints();

	if (iMaxHP != GetMaxHP())
	{

// Find this line:
ComputeSkillPoints();

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	if (IsPC())
	{
		NormalizeMountUpgradeSkills();
		m_bRefreshMountUpgradeSkillAffect = true;
		RefreshMountUpgradeSkillAffect();
		m_bRefreshMountUpgradeSkillAffect = false;
	}
#endif

// In `void CHARACTER::SetTarget(LPCHARACTER pkChrTarget)`, extend the related check like this:
#if defined(__MOUNT_UPGRADE__)
				|| m_pkChrTarget->GetRaceNum() == 20149
				|| m_pkChrTarget->GetRaceNum() == 20150
				|| m_pkChrTarget->GetRaceNum() == 20151
#endif
