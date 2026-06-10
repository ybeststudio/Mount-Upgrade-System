// Find this line:
PyObject* GetPhaseWindow(BYTE bPhaseWnd) const { return m_apoPhaseWnd[bPhaseWnd]; }

// Add after it:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
public:
	bool MountUpGrade(const uint8_t iSubHeader, const uint32_t arg = 0);
	bool RecvMountUpGrade();
	bool RecvMountUpGradeChat();
#endif
