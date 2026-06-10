// Find this line:
"UNIQUE_BUNDLE",

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	static string arSub17[] = {
		"0",
		"SKILL_BOOK_HORSE",
	};
#endif

// Find this line:
arSub16,	// ITEM_UNIQUE

// Add after it:
#if defined(__MOUNT_UPGRADE__)
		arSub17,	// ITEM_SKILLBOOK
#else
		0,			// ITEM_SKILLBOOK
#endif

// Find this line:
sizeof(arSub16) / sizeof(arSub16[0]),	// ITEM_UNIQUE

// Add after it:
#if defined(__MOUNT_UPGRADE__)
		sizeof(arSub17) / sizeof(arSub17[0]),	// ITEM_SKILLBOOK
#else
		0,										// ITEM_SKILLBOOK
#endif
