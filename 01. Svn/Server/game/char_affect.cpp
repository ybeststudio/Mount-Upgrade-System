// Before
	if (AFFECT_REVIVE_INVISIBLE != pkAff->dwType
#if defined(__SET_ITEM__)
		&& (!IS_NO_CLEAR_SET_ITEM(pkAff->dwType))
#endif
		&& IsPC())
		ComputePoints();
	else
		UpdatePacket();

#if defined(__LOOT_FILTER_SYSTEM__) && defined(__PREMIUM_LOOT_FILTER__)
	if (pkAff->dwType == AFFECT_LOOTING_SYSTEM)
		ClearLootFilter();
#endif

	CheckMaximumPoints();

	if (test_server)
		sys_log(0, "AFFECT_REMOVE: %s (flag %u apply: %u)", GetName(), pkAff->dwFlag, pkAff->wApplyOn);

	if (IsPC())
	{
		SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->wApplyOn);
	}

// After
	if (AFFECT_REVIVE_INVISIBLE != pkAff->dwType
#if defined(__SET_ITEM__)
		&& (!IS_NO_CLEAR_SET_ITEM(pkAff->dwType))
#endif
#if defined(__MOUNT_UPGRADE__)
		&& !m_bRefreshMountUpgradeSkillAffect
#endif
		&& IsPC())
		ComputePoints();
	else
		UpdatePacket();

#if defined(__LOOT_FILTER_SYSTEM__) && defined(__PREMIUM_LOOT_FILTER__)
	if (pkAff->dwType == AFFECT_LOOTING_SYSTEM)
		ClearLootFilter();
#endif

	CheckMaximumPoints();

	if (test_server)
		sys_log(0, "AFFECT_REMOVE: %s (flag %u apply: %u)", GetName(), pkAff->dwFlag, pkAff->wApplyOn);

	if (IsPC())
	{
		SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->wApplyOn);
	}
