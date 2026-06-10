// Find this line:
initLoading();

// Add after it:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	initMount();
#endif
