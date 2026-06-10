// Find this line:
ch->SetSkillLevel(dwVnum, byLev);

// Add after it:
#if defined(__MOUNT_UPGRADE__)
		if (dwVnum >= 285 && dwVnum <= 311)
			ch->ComputePoints();
#endif
