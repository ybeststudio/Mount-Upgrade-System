// Find this line:
"`horse_skill_point` = %d, "

// Add after it:
#if defined(__MOUNT_UPGRADE__)
		"`mount_up_grade_exp` = %d, "
		"`mount_up_grade_fail` = %d, "
#endif

// Find this line:
pkTab->horse_skill_point

// Add after it:
#if defined(__MOUNT_UPGRADE__)
		, pkTab->mount_up_grade_exp
		, pkTab->mount_up_grade_fail
#endif

// Find this line:
"`horse_skill_point`"

// Add after it:
#if defined(__MOUNT_UPGRADE__)
			", `mount_up_grade_exp`"
			", `mount_up_grade_fail`"
#endif

// Before
	else
		memset(pkTab->quickslot, 0, sizeof(pkTab->quickslot));

	col++;

	str_to_number(pkTab->skill_group, row[col++]);
	str_to_number(pkTab->lAlignment, row[col++]);
#if defined(__CONQUEROR_LEVEL__)
	str_to_number(pkTab->conqueror_level, row[col++]);
	str_to_number(pkTab->conqueror_level_step, row[col++]);
	str_to_number(pkTab->sungma_str, row[col++]);
	str_to_number(pkTab->sungma_hp, row[col++]);
	str_to_number(pkTab->sungma_move, row[col++]);
	str_to_number(pkTab->sungma_immune, row[col++]);
	str_to_number(pkTab->conqueror_exp, row[col++]);
	str_to_number(pkTab->conqueror_point, row[col++]);
#endif
	str_to_number(pkTab->horse.bLevel, row[col++]);
	str_to_number(pkTab->horse.bRiding, row[col++]);
	str_to_number(pkTab->horse.sHealth, row[col++]);
	str_to_number(pkTab->horse.dwHorseHealthDropTime, row[col++]);
	str_to_number(pkTab->horse.sStamina, row[col++]);
	str_to_number(pkTab->logoff_interval, row[col++]);
	str_to_number(pkTab->horse_skill_point, row[col++]);

	// reset sub_skill_point
	{
#if defined(__REVERSED_FUNCTIONS__)
		/* 130 == SKILL_HORSE*/
		if (pkTab->skills[130].bMasterType != 0)
			pkTab->skills[130].bMasterType = SKILL_NORMAL;
#endif

		pkTab->skills[123].bLevel = 0; // SKILL_CREATE

		if (pkTab->level > 9)
		{
			int max_point = pkTab->level - 9;

			int skill_point =
				MIN(20, pkTab->skills[121].bLevel) + // SKILL_LEADERSHIP
				MIN(20, pkTab->skills[124].bLevel) + // SKILL_MINING
				MIN(10, pkTab->skills[131].bLevel) + // SKILL_HORSE_SUMMON
				MIN(20, pkTab->skills[141].bLevel) + // SKILL_ADD_HP HP
				MIN(20, pkTab->skills[142].bLevel) + // SKILL_RESIST_PENETRATE
#if defined(__PARTY_PROFICY__)
				MIN(20, pkTab->skills[133].bLevel) + // SKILL_ROLE_PROFICIENCY
#endif
#if defined(__PARTY_INSIGHT__)
				MIN(20, pkTab->skills[134].bLevel) + // SKILL_INSIGHT
#endif
				MIN(20, pkTab->skills[246].bLevel); // SKILL_HIT

			pkTab->sub_skill_point = max_point - skill_point;
		}
		else
			pkTab->sub_skill_point = 0;
	}

// After
	else
		memset(pkTab->quickslot, 0, sizeof(pkTab->quickslot));

	col++;

	str_to_number(pkTab->skill_group, row[col++]);
	str_to_number(pkTab->lAlignment, row[col++]);
#if defined(__CONQUEROR_LEVEL__)
	str_to_number(pkTab->conqueror_level, row[col++]);
	str_to_number(pkTab->conqueror_level_step, row[col++]);
	str_to_number(pkTab->sungma_str, row[col++]);
	str_to_number(pkTab->sungma_hp, row[col++]);
	str_to_number(pkTab->sungma_move, row[col++]);
	str_to_number(pkTab->sungma_immune, row[col++]);
	str_to_number(pkTab->conqueror_exp, row[col++]);
	str_to_number(pkTab->conqueror_point, row[col++]);
#endif
	str_to_number(pkTab->horse.bLevel, row[col++]);
	str_to_number(pkTab->horse.bRiding, row[col++]);
	str_to_number(pkTab->horse.sHealth, row[col++]);
	str_to_number(pkTab->horse.dwHorseHealthDropTime, row[col++]);
	str_to_number(pkTab->horse.sStamina, row[col++]);
	str_to_number(pkTab->logoff_interval, row[col++]);
	str_to_number(pkTab->horse_skill_point, row[col++]);
#if defined(__MOUNT_UPGRADE__)
	str_to_number(pkTab->mount_up_grade_exp, row[col++]);
	str_to_number(pkTab->mount_up_grade_fail, row[col++]);
#endif

	// reset sub_skill_point
	{
#if defined(__REVERSED_FUNCTIONS__)
		/* 130 == SKILL_HORSE*/
		if (pkTab->skills[130].bMasterType != 0)
			pkTab->skills[130].bMasterType = SKILL_NORMAL;
#endif

		pkTab->skills[123].bLevel = 0; // SKILL_CREATE

		if (pkTab->level > 9)
		{
			int max_point = pkTab->level - 9;

			int skill_point =
				MIN(20, pkTab->skills[121].bLevel) + // SKILL_LEADERSHIP
				MIN(20, pkTab->skills[124].bLevel) + // SKILL_MINING
				MIN(10, pkTab->skills[131].bLevel) + // SKILL_HORSE_SUMMON
				MIN(20, pkTab->skills[141].bLevel) + // SKILL_ADD_HP HP
				MIN(20, pkTab->skills[142].bLevel) + // SKILL_RESIST_PENETRATE
#if defined(__PARTY_PROFICY__)
				MIN(20, pkTab->skills[133].bLevel) + // SKILL_ROLE_PROFICIENCY
#endif
#if defined(__PARTY_INSIGHT__)
				MIN(20, pkTab->skills[134].bLevel) + // SKILL_INSIGHT
#endif
				MIN(20, pkTab->skills[246].bLevel); // SKILL_HIT

			pkTab->sub_skill_point = max_point - skill_point;
		}
		else
			pkTab->sub_skill_point = 0;
	}
