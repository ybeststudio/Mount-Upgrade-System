// Find this line:
bool m_bIsLoadedAffect;

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	bool m_bRefreshMountUpgradeSkillAffect;
#endif

// Find this line:
void ComputePassiveSkill(DWORD dwVnum);

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	void NormalizeMountUpgradeSkills();
	void RefreshMountUpgradeSkillAffect();
#endif

// Find this line:
LPDEFENSE_WAVE m_pDefenseWave;

// Add after it:
#if defined(__MOUNT_UPGRADE__)
public:
	void SetMountUpGradeExp(uint32_t exp) { m_mount_up_grade_exp = exp; }
	uint32_t GetMountUpGradeExp() const { return m_mount_up_grade_exp; }

	void SetMountUpGradeFail(uint8_t fail) { m_mount_up_grade_fail = fail; }
	uint8_t IsMountUpGradeFail() const { return m_mount_up_grade_fail; }

protected:
	uint32_t m_mount_up_grade_exp;
	uint8_t m_mount_up_grade_fail;
#endif
