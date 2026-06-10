// Find this line:
PyModule_AddIntConstant(poModule, "ITEM_TYPE_SKILLBOOK", CItemData::ITEM_TYPE_SKILLBOOK);

// Add after it:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	PyModule_AddIntConstant(poModule, "SKILL_BOOK_NORMAL", CItemData::SKILL_BOOK_NORMAL);
	PyModule_AddIntConstant(poModule, "SKILL_BOOK_HORSE", CItemData::SKILL_BOOK_HORSE);
#endif

// Find this line:
PyModule_AddIntConstant(poModule, "ITEM_VNUM_GEM_STONE", ITEM_VNUM_GEM_STONE);

// Add after it:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	PyModule_AddIntConstant(poModule, "ITEM_VNUM_MOUNT_UPGRADE_SYSTEM_FEED", 50048);
#endif
