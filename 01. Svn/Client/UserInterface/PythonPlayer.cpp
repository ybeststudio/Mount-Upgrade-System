// Before
	if (!GetSkillSlotIndex(dwSkillIndex, &dwSlotIndex))
	{
			return;
	}

// After
	if (!GetSkillSlotIndex(dwSkillIndex, &dwSlotIndex))
	{
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
		if (dwSkillIndex >= 285 && dwSkillIndex <= 311)
		{
			SetSkill(dwSkillIndex, dwSkillIndex);
			dwSlotIndex = dwSkillIndex;
		}
		else
#endif
			return;
	}
