// Find this line:
extern int test_server;

// Add after it:
#if defined(__MOUNT_UPGRADE__)
namespace
{
	bool IsMountUpgradeSkillVnum(DWORD dwVnum)
	{
		return dwVnum >= 285 && dwVnum <= 311;
	}

	int GetEffectiveMountHorseLevel(const CHARACTER* ch)
	{
		return MAX(ch->GetHorseLevel(), ch->GetSkillLevel(SKILL_HORSE));
	}

	bool IsMountUpgradeSkill(DWORD dwVnum, const CSkillProto* pkSkill)
	{
		if (pkSkill && pkSkill->dwType == SKILL_BOOK_TYPE_MOUNT_UPGRADE)
			return true;

		return IsMountUpgradeSkillVnum(dwVnum);
	}
}
#endif

// Find this line:
m_pSkillLevels[dwVnum].bLevel = MIN(SKILL_MAX_LEVEL, bLev);

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	if (IsMountUpgradeSkillVnum(dwVnum))
	{
		m_pSkillLevels[dwVnum].bLevel = MIN(20, (int)bLev);
		m_pSkillLevels[dwVnum].bMasterType = SKILL_NORMAL;
		return;
	}
#endif

// In `bool CHARACTER::IsLearnableSkill(DWORD dwSkillVnum)`, find this block:
if (pkSkill->dwType == SKILL_BOOK_TYPE_HORSE)
	{
		if (dwSkillVnum == SKILL_HORSE_WILDATTACK_RANGE && GetJob() != JOB_ASSASSIN)
			return false;

		return true;
	}

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	if (IsMountUpgradeSkill(dwSkillVnum, pkSkill))
	{
		if (GetSkillLevel(dwSkillVnum) >= 1)
			return false;

		const int iHorseLevel = GetEffectiveMountHorseLevel(this);
		if (iHorseLevel < 21)
			return false;

		int iLearnedCount = 0;
		for (DWORD vnum = 285; vnum <= 311; ++vnum)
		{
			if (GetSkillLevel(vnum) > 0)
				++iLearnedCount;
		}

		const int iMaxSlots = (iHorseLevel >= 31) ? 2 : 1;
		if (iLearnedCount >= iMaxSlots)
			return false;

		return true;
	}
#endif

// Before
	if (pkSk == nullptr)
	{
		return false;
	}

// After
	if (pkSk == nullptr)
	{
#if defined(__MOUNT_UPGRADE__)
		if (IsMountUpgradeSkillVnum(dwSkillVnum))
		{
			sys_err("LearnSkillByBook: mount upgrade skill %u missing in skill_proto", dwSkillVnum);
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot train this skill."));
		}
#endif
		return false;
	}

// Before
	if (!IsLearnableSkill(dwSkillVnum))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot train this skill."));
		return false;
	}

// After
	if (!IsLearnableSkill(dwSkillVnum))
	{
#if defined(__MOUNT_UPGRADE__)
		if (IsMountUpgradeSkill(dwSkillVnum, pkSk))
		{
			const int iHorseLevel = GetEffectiveMountHorseLevel(this);
			if (iHorseLevel < 21)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot train this skill."));
				sys_log(0, "LearnSkillByBook: mount skill %u blocked, horse level %d < 21", dwSkillVnum, iHorseLevel);
				return false;
			}

			if (GetSkillLevel(dwSkillVnum) >= 1)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot train this skill."));
				return false;
			}

			int iLearnedCount = 0;
			for (DWORD vnum = 285; vnum <= 311; ++vnum)
			{
				if (GetSkillLevel(vnum) > 0)
					++iLearnedCount;
			}

			const int iMaxSlots = (iHorseLevel >= 31) ? 2 : 1;
			if (iLearnedCount >= iMaxSlots)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot train this skill."));
				sys_log(0, "LearnSkillByBook: mount skill %u blocked, learned %d/%d slots", dwSkillVnum, iLearnedCount, iMaxSlots);
				return false;
			}

			if (pkSk->dwType != SKILL_BOOK_TYPE_MOUNT_UPGRADE)
				sys_err("LearnSkillByBook: mount skill %u has wrong proto type %u (expected %u)", dwSkillVnum, pkSk->dwType, SKILL_BOOK_TYPE_MOUNT_UPGRADE);
		}
#endif
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot train this skill."));
		return false;
	}

// Before
	if (FN_should_check_exp(this)
		)
	{
		need_exp = 20000;

#if defined(__CONQUEROR_LEVEL__)
		if (bConquerorSkill && GetConquerorExp() < need_exp)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("? ? ?   ?."));
			return false;
		}
		else
		{
			if (GetExp() < need_exp)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("? ? ?   ?."));
				return false;
			}
		}
#else
		if (GetExp() < need_exp)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot read this due to your lack of experience."));
			return false;
		}
#endif
	}

// After
	if (FN_should_check_exp(this)
#if defined(__MOUNT_UPGRADE__)
		&& !IsMountUpgradeSkill(dwSkillVnum, pkSk)
#endif
		)
	{
		need_exp = 20000;

#if defined(__CONQUEROR_LEVEL__)
		if (bConquerorSkill && GetConquerorExp() < need_exp)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("? ? ?   ?."));
			return false;
		}
		else
		{
			if (GetExp() < need_exp)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("? ? ?   ?."));
				return false;
			}
		}
#else
		if (GetExp() < need_exp)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot read this due to your lack of experience."));
			return false;
		}
#endif
	}

// Before
				if (pkSk->dwType != SKILL_BOOK_TYPE_PASSIVE
#if defined(__CONQUEROR_LEVEL__)
					&& !bConquerorSkill
#endif
					)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("This skill's level is not high enough to be trained with a Book."));
					return false;
				}

// After
				if (pkSk->dwType != SKILL_BOOK_TYPE_PASSIVE
#if defined(__MOUNT_UPGRADE__)
					&& pkSk->dwType != SKILL_BOOK_TYPE_MOUNT_UPGRADE
#endif
#if defined(__CONQUEROR_LEVEL__)
					&& !bConquerorSkill
#endif
					)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("This skill's level is not high enough to be trained with a Book."));
					return false;
				}

// In `bool CHARACTER::LearnSkillByBook(DWORD dwSkillVnum, BYTE bProb)`, find this block:
BYTE bLastLevel = GetSkillLevel(dwSkillVnum);

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	if (IsMountUpgradeSkill(dwSkillVnum, pkSk) && bLastLevel == 0 && bProb == 0)
	{
		SkillLevelUp(dwSkillVnum, SKILL_UP_BY_BOOK);
		if (GetSkillLevel(dwSkillVnum) > bLastLevel)
		{
			RefreshMountUpgradeSkillAffect();
			return true;
		}
		return false;
	}
#endif

// Before
	if (pkSk->dwType != SKILL_BOOK_TYPE_SUPPORT && pkSk->dwType != SKILL_BOOK_TYPE_PASSIVE
		)
	{
		switch (GetSkillMasterType(pkSk->dwVnum))
		{
			case SKILL_GRAND_MASTER:
				if (bMethod != SKILL_UP_BY_QUEST)
					return;
				break;

			case SKILL_PERFECT_MASTER:
				return;
		}
	}

// After
	if (pkSk->dwType != SKILL_BOOK_TYPE_SUPPORT && pkSk->dwType != SKILL_BOOK_TYPE_PASSIVE
#if defined(__MOUNT_UPGRADE__)
		&& !IsMountUpgradeSkill(dwVnum, pkSk)
#endif
		)
	{
		switch (GetSkillMasterType(pkSk->dwVnum))
		{
			case SKILL_GRAND_MASTER:
				if (bMethod != SKILL_UP_BY_QUEST)
					return;
				break;

			case SKILL_PERFECT_MASTER:
				return;
		}
	}

// Before
		if (pkSk->dwType != SKILL_BOOK_TYPE_SUPPORT && pkSk->dwType != SKILL_BOOK_TYPE_PASSIVE
			)
			if (GetSkillMasterType(pkSk->dwVnum) != SKILL_MASTER
#if defined(__CONQUEROR_LEVEL__)
				&& !IsConquerorSkill(pkSk->dwVnum)
#endif
				)
				return;
	}

	if (GetLevel() < pkSk->bLevelLimit)
		return;

	if (pkSk->preSkillVnum)
	{

// After
		if (pkSk->dwType != SKILL_BOOK_TYPE_SUPPORT && pkSk->dwType != SKILL_BOOK_TYPE_PASSIVE
#if defined(__MOUNT_UPGRADE__)
			&& pkSk->dwType != SKILL_BOOK_TYPE_MOUNT_UPGRADE
#endif
			)
			if (GetSkillMasterType(pkSk->dwVnum) != SKILL_MASTER
#if defined(__CONQUEROR_LEVEL__)
				&& !IsConquerorSkill(pkSk->dwVnum)
#endif
				)
				return;
	}

	if (GetLevel() < pkSk->bLevelLimit)
		return;

	if (pkSk->preSkillVnum)
	{

// In `void CHARACTER::SkillLevelUp(DWORD dwVnum, BYTE bMethod)`, extend the switch statement with:
#if defined(__MOUNT_UPGRADE__)
			case SKILL_BOOK_TYPE_MOUNT_UPGRADE:
				idx = POINT_NONE;
				break;
#endif

// Add the following `CHARACTER::NormalizeMountUpgradeSkills` function anywhere in this file:
#if defined(__MOUNT_UPGRADE__)
namespace
{
	DWORD GetMountUpgradeSkillAffect(DWORD dwVnum)
	{
		switch (dwVnum)
		{
			case 285: return AFFECT_SKILL_MOUNT_UPGRADE_NIMBLE;
			case 286: return AFFECT_SKILL_MOUNT_UPGRADE_EXP;
			case 287: return AFFECT_SKILL_MOUNT_UPGRADE_SPEED;
			case 288: return AFFECT_SKILL_MOUNT_UPGRADE_GYENONGGONG;
			case 289: return AFFECT_SKILL_MOUNT_UPGRADE_INVINCIBLITIY;
			case 290: return AFFECT_SKILL_MOUNT_UPGRADE_KNOCKBACK;
			case 301: return AFFECT_SKILL_MOUNT_UPGRADE_EFFECT_UP;
			case 307: return AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_STR;
			case 308: return AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_HP;
			case 309: return AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_MOVE;
			case 310: return AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_IMMUNE;
			case 311: return AFFECT_SKILL_MOUNT_UPGRADE_HIT_PCT;
			default:
				if ((dwVnum >= 291 && dwVnum <= 300) || (dwVnum >= 302 && dwVnum <= 306))
					return AFFECT_SKILL_MOUNT_UPGRADE_TO_STRONG;
				return 0;
		}
	}

	POINT_TYPE GetMountUpgradeSkillApplyPoint(const CSkillProto* pkSk)
	{
		if (!pkSk)
			return POINT_NONE;

		if (pkSk->wPointOn != POINT_NONE && pkSk->wPointOn < POINT_MAX_NUM)
			return pkSk->wPointOn;

		if (pkSk->wPointOn3 != POINT_NONE && pkSk->wPointOn3 < POINT_MAX_NUM)
			return pkSk->wPointOn3;

		if (pkSk->wPointOn2 != POINT_NONE && pkSk->wPointOn2 < POINT_MAX_NUM)
			return pkSk->wPointOn2;

		return POINT_NONE;
	}

	bool IsMountUpgradeSkillAffect(DWORD dwType)
	{
		return dwType >= AFFECT_SKILL_MOUNT_UPGRADE_NIMBLE && dwType <= AFFECT_SKILL_MOUNT_UPGRADE_HIT_PCT;
	}
}

#if defined(__MOUNT_UPGRADE__)
void CHARACTER::NormalizeMountUpgradeSkills()
{
	if (!m_pSkillLevels)
		return;

	for (DWORD dwVnum = 285; dwVnum <= 311; ++dwVnum)
	{
		TPlayerSkill& sk = m_pSkillLevels[dwVnum];
		if (sk.bLevel == 0)
			continue;

		if (sk.bMasterType == SKILL_MASTER && sk.bLevel < 20)
			sk.bLevel = MIN(20, (int)sk.bLevel + 19);
		else if (sk.bMasterType == SKILL_GRAND_MASTER && sk.bLevel < 20)
			sk.bLevel = MIN(20, (int)sk.bLevel + 29);

		if (sk.bLevel > 20)
			sk.bLevel = 20;

		sk.bMasterType = SKILL_NORMAL;
	}
}
#endif

void CHARACTER::RefreshMountUpgradeSkillAffect()
{
	if (!IsPC())
		return;

	for (AffectContainerList::iterator it = m_list_pkAffect.begin(); it != m_list_pkAffect.end();)
	{
		CAffect* pkAff = *it;
		if (!pkAff || !IsMountUpgradeSkillAffect(pkAff->dwType))
		{
			++it;
			continue;
		}

		if (GetDesc())
			SendAffectRemovePacket(GetDesc(), GetPlayerID(), pkAff->dwType, pkAff->wApplyOn);

		it = m_list_pkAffect.erase(it);
		CAffect::Release(pkAff);
	}

	if (GetEffectiveMountHorseLevel(this) < 21)
	{
		if (!m_bRefreshMountUpgradeSkillAffect)
			ComputePoints();
		return;
	}

	const double dHorseLv = static_cast<double>(GetEffectiveMountHorseLevel(this));

	for (DWORD dwVnum = 285; dwVnum <= 311; ++dwVnum)
	{
		const BYTE bSkillLevel = GetSkillLevel(dwVnum);
		if (bSkillLevel == 0)
			continue;

		if (dwVnum == 288)
		{
			if (IsRiding())
				continue;
		}
		else if (!IsRiding())
			continue;

		const DWORD dwAffect = GetMountUpgradeSkillAffect(dwVnum);
		if (dwAffect == 0)
			continue;

		CSkillProto* pkSk = CSkillManager::instance().Get(dwVnum);
		if (!pkSk)
			continue;

		const POINT_TYPE wApplyOn = GetMountUpgradeSkillApplyPoint(pkSk);
		if (wApplyOn >= POINT_MAX_NUM)
			continue;

		pkSk->SetPointVar("skilllv", bSkillLevel);
		pkSk->SetPointVar("horselv", dHorseLv);
		pkSk->SetPointVar("horse_level", dHorseLv);
		pkSk->SetPointVar("k", dwVnum == 301 ? 100.0 : static_cast<double>(bSkillLevel));

		int iValue = static_cast<int>(pkSk->kPointPoly.Eval());
		if (iValue == 0 && dwVnum == 290)
			iValue = 1;
		else if (iValue == 0 && dwVnum != 289)
			continue;

		CAffect* pkAff = CAffect::Acquire();
		m_list_pkAffect.push_back(pkAff);

		pkAff->dwType = dwAffect;
		pkAff->wApplyOn = wApplyOn;
		pkAff->lApplyValue = iValue;
		pkAff->dwFlag = AFF_NONE;
		pkAff->lDuration = INFINITE_AFFECT_DURATION;
		pkAff->lSPCost = 0;
#if defined(__AFFECT_RENEWAL__)
		pkAff->bRealTime = false;
		pkAff->bUpdate = false;
#endif
#if defined(__9TH_SKILL__)
		pkAff->lValue = 0;
#endif

		if (GetDesc())
			SendAffectAddPacket(GetDesc(), pkAff);
	}

	if (!m_bRefreshMountUpgradeSkillAffect)
		ComputePoints();
}
#endif

// In `GetMountLevelByVnum(DWORD dwMountVnum, bool IsNew)`, extend the switch statement with:
#if defined(__MOUNT_UPGRADE__)
		case 20149: // normal military horse (no guild)
		case 20150: // normal military horse (guild member)
		case 20151: // normal military horse (guild master)
#endif
