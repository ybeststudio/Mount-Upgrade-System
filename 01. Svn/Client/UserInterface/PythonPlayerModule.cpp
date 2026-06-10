// Find this line:
PyModule_AddIntConstant(poModule, "COMB_WND_SLOT_MAX", ECombSlotType::COMB_WND_SLOT_MAX);

// Add after it:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	PyModule_AddIntConstant(poModule, "HORSE_MAX_LEVEL", HORSE_MAX_LEVEL);
#endif
