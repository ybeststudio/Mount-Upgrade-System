// Find this line:
Set(HEADER_GC_PRELOAD_ENTITIES, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCPreloadEntities), DYNAMIC_SIZE_PACKET));

// Add after it:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
		Set(HEADER_GC_MOUNT_UP_GRADE, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMountUpGrade), STATIC_SIZE_PACKET));
		Set(HEADER_GC_MOUNT_UP_GRADE_CHAT, CNetworkPacketHeaderMap::TPacketType(sizeof(TPacketGCMountUpGradeChat), STATIC_SIZE_PACKET));
#endif
