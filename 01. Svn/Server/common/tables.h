// Find this line:
BYTE inven_stage;

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	uint32_t mount_up_grade_exp;
	uint8_t mount_up_grade_fail;
#endif
