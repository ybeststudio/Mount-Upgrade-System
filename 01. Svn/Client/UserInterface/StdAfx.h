// Find this line:
void initLoading();

// Add after it:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
void initMount();
#endif
