#pragma once

#include "Locale_inc.h"

#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
#	include "GameType.h"

class CPythonMountUpGrade : public CSingleton<CPythonMountUpGrade>
{
	public:
		CPythonMountUpGrade();
		virtual ~CPythonMountUpGrade();

		void Reset();
		void Process(const uint8_t subheader, const uint8_t horse_level, const uint8_t is_fail, const uint32_t existing_exp);
		void Chat(const uint8_t type, const uint16_t value);

		void SetHandler(PyObject* poHandler);
		void ClearHandler();
		PyObject* GetHandler() const;

		void SetHorseLevel(const uint8_t level);
		uint8_t GetHorseLevel() const;

		void SetMountUpGradeFail(const uint8_t fail);
		uint8_t IsMountUpGradeFail() const;

		void SetMountUpGradeExistingExp(const uint32_t exp);
		uint32_t GetMountUpGradeExistingExp() const;

		uint32_t GetMountUpGradeNecessaryExp() const;
		uint32_t GetMountUpGradePrice() const;

		void SetSkillLevelUpVnum(const uint32_t vnum);
		uint32_t GetSkillLevelUpVnum() const;
		uint32_t ResolveSkillLevelUpVnum() const;

		bool IsUIOpen() const;

	private:
		uint8_t m_HorseLevel;
		uint8_t m_MountUpGradeFail;
		uint32_t m_MountUpGradeExistingExp;
		uint32_t m_SkillLevelUpVnum;
		bool m_bUIOpen;

	protected:
		PyObject* m_poHandler;

	public:
		enum EMountUpGradeChatType : uint8_t
		{
			CHAT_TYPE_BANN_WHILE_MOUNTING,
			CHAT_TYPE_LEVEL_UP_YANG_OR_FEED_NOT_ENOUGH,
			CHAT_TYPE_LEVEL_UP_GEM_NOT_ENOUGH,
			CHAT_TYPE_LEVEL_UP_PERCENT_SUCCESSFUL,
			CHAT_TYPE_LEVEL_UP_PERCENT_FAIL,
			CHAT_TYPE_EXP_HORSE_FEED,

			CHAT_TYPE_MAX
		};

		enum EMountUpGradeChatValue : uint8_t
		{
			CHAT_TYPE_NO_VALUE
		};

		enum EMountUpGradeReset : uint8_t
		{
			RESET
		};

		enum EMountUpGradeItem : uint32_t
		{
			HORSE_FEED_ITEM_ID = 50048,
			HORSE_FEED_EXP_COUNT = 1,
			HORSE_FEED_LEVEL_COUNT = 50
		};

		enum EMountUpGradeRetryGemCost : uint8_t
		{
			HORSE_LEVEL_DETERMINES_GEM_COST = 30,
			HORSE_LOWER_LEVEL_RETRY_GEM_COST = 50,
			HORSE_UPPER_LEVEL_RETRY_GEM_COST = 100,
		};

		enum EMountUpGradeFailType : uint8_t
		{
			MOUNT_UP_GRADE_FAIL_OFF,
			MOUNT_UP_GRADE_FAIL_ON
		};

		enum EMountUpGradeGCSubheaderType : uint8_t
		{
			SUBHEADER_GC_MOUNT_UP_GRADE_OPEN = 0,
			SUBHEADER_GC_MOUNT_UP_GRADE_GIVE_FEED = 1,
			SUBHEADER_GC_MOUNT_UP_GRADE_HORSE_LEVEL_UP = 2,
		};

		enum EMountUpGradeCGSubheaderType : uint8_t
		{
			SUBHEADER_CG_MOUNT_UP_GRADE_EXP = 0,
			SUBHEADER_CG_MOUNT_UP_GRADE_LEVEL_UP = 1,
			SUBHEADER_CG_MOUNT_UP_GRADE_CLOSE = 2,
			SUBHEADER_CG_MOUNT_UP_GRADE_SKILL_LEVEL_UP = 3,
		};

		enum EMountUpGradeLevelUpResult : uint8_t
		{
			MOUNT_UPGRADE_SYSTEM_LEVEL_UP_RESULT_PERCENT_SUCCESS = 0,
			MOUNT_UPGRADE_SYSTEM_LEVEL_UP_RESULT_PERCENT_FAIL = 1,
		};

		enum EMountUpGradeMisc : uint32_t
		{
			MOUNT_UPGRADE_SKILL_LEVELUP_COST_GEM = 1000,
			MOUNT_UPGRADE_SKILL_LEVELUP_COST_YANG = 20000000,
			MOUNT_UPGRADE_SYSTEM_HORSE_SKILL_LEVEL_MAX = 20,
			MOUNT_UPGRADE_SYSTEM_HORSE_MIN_LEVEL_CONQUEROR = 30,
			MOUNT_UPGRADE_SYSTEM_HORSE_LEVEL_MAX = 40,
		};
};

const uint32_t mount_up_grade_exp_table[HORSE_MAX_LEVEL + 1] =
{
	/* Done with Quest */
	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,	// 0 - 10
	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,	// 11 - 21

	/* Real Need Exp */
	12500,	12810,	13130,	13440,	14060,	14380,	14690,	15000,	18750,	37500,		// 22 - 31
	48800,	63490,	66930,	82620,	107510,	139890,	182030,	236860,	308200				// 32 - 40
};

const uint32_t mount_up_grade_price_table[HORSE_MAX_LEVEL + 1] =
{
	/* Done with Quest */
	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,	// 0 - 10
	0,		0,		0,		0,		0,		0,		0,		0,		0,		0,		0,	// 11 - 21

	/* Real Need Price */
	500000,		1000000,	1500000,	2000000,	2500000,	3000000,	3500000,	// 22 - 28
	4000000,	4500000,	5000000,	5000000,	5000000,	5000000,	5000000,	// 29 - 35
	5000000,	5000000,	5000000,	5000000,	5000000								// 36 - 40
};
#endif
