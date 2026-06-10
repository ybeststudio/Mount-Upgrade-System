// Find this line:
CHARACTER_PATH_MAX_NUM = 64,

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	SKILL_MAX_NUM = 320,
#else
	SKILL_MAX_NUM = 255,
#endif

// Find this line:
SKILL_MAX_LEVEL = 40,

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	HORSE_MAX_LEVEL = 40,
#endif
