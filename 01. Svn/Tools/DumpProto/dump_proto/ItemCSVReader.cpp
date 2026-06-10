// Add near the other feature flags at the top of this file:
#define ENABLE_MOUNT_UPGRADE_SYSTEM

// Find this line:
"UNIQUE_BUNDLE",

// Add after it:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	string arSub17[] = {
		"0",
		"SKILL_BOOK_HORSE",
	};
#endif

// Find this line:
arSub16,	// ITEM_UNIQUE

// Add after it:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
		arSub17,	// ITEM_SKILLBOOK
#else
		0,			// ITEM_SKILLBOOK
#endif

// Find this line:
sizeof(arSub16) / sizeof(arSub16[0]),	// ITEM_UNIQUE

// Add after it:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
		sizeof(arSub17) / sizeof(arSub17[0]),	// ITEM_SKILLBOOK
#else
		0,										// ITEM_SKILLBOOK
#endif
