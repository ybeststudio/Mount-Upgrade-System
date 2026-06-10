# In `__init__`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
			self.interface.RegisterMountUpgradeHandler()

# In `BINARY_NEW_AddAffect`, extend the if-statement with:
		elif app.ENABLE_MOUNT_UPGRADE_SYSTEM and chr.AFFECT_SKILL_MOUNT_UPGRADE_NIMBLE <= type <= chr.AFFECT_SKILL_MOUNT_UPGRADE_HIT_PCT:
			self.RefreshSkill()


# Add anywhere in the GameWindow class:
		elif app.ENABLE_MOUNT_UPGRADE_SYSTEM and chr.AFFECT_SKILL_MOUNT_UPGRADE_NIMBLE <= type <= chr.AFFECT_SKILL_MOUNT_UPGRADE_HIT_PCT:
			self.RefreshSkill()

	if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
		def BINARY_NEW_Remove_MountUpgrade_Passive_Skill_Affect(self, affect, point_idx):
			if self.affectShower:
				self.affectShower.BINARY_NEW_Remove_MountUpgrade_Passive_Skill_Affect(affect, point_idx)
			self.RefreshSkill()


# Add anywhere in the GameWindow class:
	if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
		def OpenMountUpGrade(self):
			pass

		def RegisterMountUpgradeHandler(self):
			if self.interface:
				self.interface.RegisterMountUpgradeHandler()
