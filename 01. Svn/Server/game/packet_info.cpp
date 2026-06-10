// Find this line:
Set(HEADER_CG_TREASURE_HUNT, sizeof(TPacketCGTreasureHunt), "TreasureHunt", true);

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	Set(HEADER_CG_MOUNT_UP_GRADE, sizeof(TPacketCGMountUpGrade), "MountUpGrade", true);
#endif
