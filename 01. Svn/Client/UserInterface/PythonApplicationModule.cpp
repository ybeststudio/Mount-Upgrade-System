// Find this line:
PyModule_AddIntConstant(poModule, "ENABLE_LOADING_PERFORMANCE", 0);

// Add after it:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	PyModule_AddIntConstant(poModule, "ENABLE_MOUNT_UPGRADE_SYSTEM", 1);
#else
	PyModule_AddIntConstant(poModule, "ENABLE_MOUNT_UPGRADE_SYSTEM", 0);
#endif
