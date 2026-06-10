// Find this line:
void FlowerEvent(LPCHARACTER pChar, const char* pszData);

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	void MountUpGrade(LPCHARACTER ch, const char* c_pData);
#endif
