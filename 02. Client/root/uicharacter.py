# Add to the imports:
if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
	import mount


# Add anywhere inside the `CharacterWindow` class:
	if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
		MOUNT_UPGRADE_SKILL_SLOT_BASE = 2
		MOUNT_UPGRADE_SKILL_SLOT_ROW_COUNT = 4
		MOUNT_UPGRADE_SKILL_SLOT_COL_COUNT = 3
		MOUNT_UPGRADE_SKILL_VNUM_START = 285
		MOUNT_UPGRADE_SKILL_VNUM_END = 311


# Add anywhere in the CharacterWindow class:
	if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
		def __GetMountSkillLevel(self, skillVnum):
			slotIndex = player.GetSkillSlotIndex(skillVnum)
			if not slotIndex:
				if self.MOUNT_UPGRADE_SKILL_VNUM_START <= skillVnum <= self.MOUNT_UPGRADE_SKILL_VNUM_END:
					slotIndex = skillVnum
				else:
					return 0

			skillGrade = player.GetSkillGrade(slotIndex)
			skillLevel = player.GetSkillLevel(slotIndex)

			if skillGrade == 1:
				skillLevel += 19
			elif skillGrade == 2:
				skillLevel += 29
			elif skillGrade == 3:
				skillLevel = 40

			return skillLevel

		def __GetMountSkillGrade(self, skillVnum):
			slotIndex = player.GetSkillSlotIndex(skillVnum)
			if slotIndex:
				return player.GetSkillGrade(slotIndex)
			if self.MOUNT_UPGRADE_SKILL_VNUM_START <= skillVnum <= self.MOUNT_UPGRADE_SKILL_VNUM_END:
				return player.GetSkillGrade(skillVnum)
			return 0

		def __GetMountSkillRealSlotIndex(self, mountVisSlot):
			row = mountVisSlot % self.MOUNT_UPGRADE_SKILL_SLOT_ROW_COUNT
			col = mountVisSlot / self.MOUNT_UPGRADE_SKILL_SLOT_ROW_COUNT
			return self.MOUNT_UPGRADE_SKILL_SLOT_BASE + row * 2 + col * skill.SKILL_GRADE_STEP_COUNT

		def __IsMountSkillRealSlotIndex(self, realSkillSlot):
			if realSkillSlot < self.MOUNT_UPGRADE_SKILL_SLOT_BASE:
				return False
			if realSkillSlot % 2:
				return False
			col = realSkillSlot / skill.SKILL_GRADE_STEP_COUNT
			if col >= self.MOUNT_UPGRADE_SKILL_SLOT_COL_COUNT:
				return False
			row = (realSkillSlot % skill.SKILL_GRADE_STEP_COUNT) / 2
			return row < self.MOUNT_UPGRADE_SKILL_SLOT_ROW_COUNT

		def __GetMountUpgradeSkillAffectType(self, skillVnum):
			if skillVnum == 285:
				return chr.AFFECT_SKILL_MOUNT_UPGRADE_NIMBLE
			if skillVnum == 286:
				return chr.AFFECT_SKILL_MOUNT_UPGRADE_EXP
			if skillVnum == 287:
				return chr.AFFECT_SKILL_MOUNT_UPGRADE_SPEED
			if skillVnum == 288:
				return chr.AFFECT_SKILL_MOUNT_UPGRADE_GYENONGGONG
			if skillVnum == 289:
				return chr.AFFECT_SKILL_MOUNT_UPGRADE_INVINCIBLITIY
			if skillVnum == 290:
				return chr.AFFECT_SKILL_MOUNT_UPGRADE_KNOCKBACK
			if skillVnum == 301:
				return chr.AFFECT_SKILL_MOUNT_UPGRADE_EFFECT_UP
			if skillVnum == 307:
				return chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_STR
			if skillVnum == 308:
				return chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_HP
			if skillVnum == 309:
				return chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_MOVE
			if skillVnum == 310:
				return chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_IMMUNE
			if skillVnum == 311:
				return chr.AFFECT_SKILL_MOUNT_UPGRADE_HIT_PCT
			if (skillVnum >= 291 and skillVnum <= 300) or (skillVnum >= 302 and skillVnum <= 306):
				return chr.AFFECT_SKILL_MOUNT_UPGRADE_TO_STRONG
			return 0

		def __IsMountUpgradeSkillPassiveActive(self, skillVnum):
			affectType = self.__GetMountUpgradeSkillAffectType(skillVnum)
			if not affectType:
				return False
			return not player.EmptyAffectData(affectType)

		def __RefreshMountSkillSlotVisual(self, skillPage, realSlotIndex, skillVnum):
			skillPage.SetCoverButton(realSlotIndex)
			if self.__IsMountUpgradeSkillPassiveActive(skillVnum):
				skillPage.EnableCoverButton(realSlotIndex)
			else:
				skillPage.DisableCoverButton(realSlotIndex)


# In `__RefreshSkillPage`, extend the if-statement with:
			elif app.ENABLE_MOUNT_UPGRADE_SYSTEM and skill.SKILL_TYPE_MOUNT_UPGRADE == skillType:
				realSlotIndex = self.__GetETCSkillRealSlotIndex(slotIndex)
				mountSkillLevel = self.__GetMountSkillLevel(skillIndex)
				skillPage.SetSkillSlot(realSlotIndex, skillIndex, mountSkillLevel)
				skillPage.SetSlotCount(realSlotIndex, mountSkillLevel)
				self.__RefreshMountSkillSlotVisual(skillPage, realSlotIndex, skillIndex)


# In `__RefreshSkillPage`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM and "ACTIVE" == name and self.PAGE_HORSE == self.curSelectedSkillGroup:
			self.mountSkillVisDict = {}
			for i in xrange(self.PAGE_SLOT_COUNT):
				skillPage.ClearSlot(self.__GetMountSkillRealSlotIndex(i))

			mountVisSlot = 0
			for vnum in xrange(self.MOUNT_UPGRADE_SKILL_VNUM_START, self.MOUNT_UPGRADE_SKILL_VNUM_END + 1):
				skillLevel = self.__GetMountSkillLevel(vnum)
				if skillLevel <= 0:
					continue

				player.SetSkill(vnum, vnum)

				realSlotIndex = self.__GetMountSkillRealSlotIndex(mountVisSlot)
				skillPage.SetSkillSlot(realSlotIndex, vnum, skillLevel)
				skillPage.SetSlotCount(realSlotIndex, skillLevel)
				self.__RefreshMountSkillSlotVisual(skillPage, realSlotIndex, vnum)
				self.mountSkillVisDict[realSlotIndex] = vnum
				mountVisSlot += 1
				if mountVisSlot >= self.PAGE_SLOT_COUNT:
					break

			skillPage.RefreshSlot()


# In `__RefreshSkillPlusButton`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM and name == "HORSE":
			mountVisSlot = 0
			for vnum in xrange(self.MOUNT_UPGRADE_SKILL_VNUM_START, self.MOUNT_UPGRADE_SKILL_VNUM_END + 1):
				skillLevel = self.__GetMountSkillLevel(vnum)
				if skillLevel <= 0 or skillLevel >= mount.MOUNT_UPGRADE_SYSTEM_HORSE_SKILL_LEVEL_MAX:
					continue

				slotWindow.ShowSlotButton(self.__GetMountSkillRealSlotIndex(mountVisSlot))
				mountVisSlot += 1
				if mountVisSlot >= self.PAGE_SLOT_COUNT:
					break


# In `__RefreshSkillPlusButton`, extend the if-statement with:
				if app.ENABLE_MOUNT_UPGRADE_SYSTEM and skill.SKILL_TYPE_MOUNT_UPGRADE == skill.GetSkillType(skillIndex):
					continue


# In `OnPressedSlotButton`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM and hasattr(self, "mountSkillVisDict"):
			mountSkillVnum = self.mountSkillVisDict.get(slotNumber, 0)
			if mountSkillVnum:
				skillIndex = mountSkillVnum
				srcSlotIndex = mountSkillVnum

		if app.ENABLE_MOUNT_UPGRADE_SYSTEM and skillIndex and skill.SKILL_TYPE_MOUNT_UPGRADE == skill.GetSkillType(skillIndex):
			if self.interface:
				self.interface.ClickMountSkillLevelUpButton(skillIndex)
			return


# In `__SetSkillSlotData`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
			for vnum in xrange(self.MOUNT_UPGRADE_SKILL_VNUM_START, self.MOUNT_UPGRADE_SKILL_VNUM_END + 1):
				if self.__GetMountSkillLevel(vnum) > 0:
					player.SetSkill(vnum, vnum)


# In `__RealSkillSlotToSourceSlot`, extend the if-statement with:
		if self.PAGE_HORSE == self.curSelectedSkillGroup:
			if app.ENABLE_MOUNT_UPGRADE_SYSTEM and hasattr(self, "mountSkillVisDict"):
				mountSkillVnum = self.mountSkillVisDict.get(realSkillSlot, 0)
				if mountSkillVnum:
					return mountSkillVnum
				if self.__IsMountSkillRealSlotIndex(realSkillSlot):
					return realSkillSlot
			return realSkillSlot + self.ACTIVE_PAGE_SLOT_COUNT
