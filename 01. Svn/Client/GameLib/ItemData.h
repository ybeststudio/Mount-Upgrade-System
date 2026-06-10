// Find this line:
ITEM_TYPE_MAX_NUM,

// Add after it:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	enum ESkillBookSubTypes
	{
		SKILL_BOOK_NORMAL = 0,
		SKILL_BOOK_HORSE = 1,
	};
#endif
