// Add the following declaration/member block related section:
#if !defined(__MOUNT_UPGRADE__)
const int HORSE_MAX_LEVEL = 30;
#endif

// Find this line:
int iIQ;

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	int iSungMaST;
	int iSungMaDX;
	int iSungMaHT;
	int iSungMaIQ;
#endif

// Find this line:
int GetHorseIQ() { return c_aHorseStat[GetHorseLevel()].iIQ; }

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	int GetHorseSungMaST() { return c_aHorseStat[GetHorseLevel()].iSungMaST; }
	int GetHorseSungMaDX() { return c_aHorseStat[GetHorseLevel()].iSungMaDX; }
	int GetHorseSungMaHT() { return c_aHorseStat[GetHorseLevel()].iSungMaHT; }
	int GetHorseSungMaIQ() { return c_aHorseStat[GetHorseLevel()].iSungMaIQ; }
#endif
