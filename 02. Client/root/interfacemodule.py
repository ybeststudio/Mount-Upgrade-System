# Add to the imports:
if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
	import mount
	import uimountupgradesystem


# In `__init__`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
			self.wndMountUpgradeSystem = None


# In `__MakeWindows`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
			self.wndMountUpgradeSystem = uimountupgradesystem.MountUpgradeSystemWindow()
			self.wndMountUpgradeSystem.BindInterfaceClass(self)


# In `Close`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
			if self.wndMountUpgradeSystem:
				self.wndMountUpgradeSystem.Destroy()
				del self.wndMountUpgradeSystem


# In `HideAllWindows`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
			if self.wndMountUpgradeSystem:
				self.wndMountUpgradeSystem.Hide()


# In `__HideWindows`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM and self.wndMountUpgradeSystem:
			hideWindows += self.wndMountUpgradeSystem,


# Add anywhere in the Interface class:
	if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
		def RegisterMountUpgradeHandler(self):
			if self.wndMountUpgradeSystem:
				mount.SetMountHandler(self.wndMountUpgradeSystem)

		def ToggleMountUpgradeSystemWindow(self):
			if not player.IsObserverMode():
				if self.wndMountUpgradeSystem and self.wndMountUpgradeSystem.IsShow():
					self.wndMountUpgradeSystem.Close()

		def ClickMountSkillLevelUpButton(self, skill_vnum):
			if self.wndMountUpgradeSystem:
				self.wndMountUpgradeSystem.ClickMountSkillLevelUpButton(skill_vnum)
