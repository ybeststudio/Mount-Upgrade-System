// Find this line:
ITEM_MAX_NUM,

// Add after it:
#if defined(__MOUNT_UPGRADE__)
enum ESkillBookSubTypes
{
	SKILL_BOOK_NORMAL = 0,
	SKILL_BOOK_HORSE = 1,
};
#endif
