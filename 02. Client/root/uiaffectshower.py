# In `__init__`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
			self.mount_upgrade_multi_desc = {}


# Add anywhere in the AffectImage class:
	if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
		def GetMountUpgradeMultiDescriptionCount(self):
			return len(self.mount_upgrade_multi_desc)

		def AddMultiLineMountUpgradeDescription(self, point_idx, description):
			self.mount_upgrade_multi_desc[point_idx] = description

		def RemoveMountUpgradeMultiDescription(self, point_idx):
			if self.mount_upgrade_multi_desc.has_key(point_idx):
				del self.mount_upgrade_multi_desc[point_idx]

		def UpdateDescriptionMountUpgradeSkill(self):
			if not self.description:
				return

			toolTip = self.description
			if self.mount_upgrade_multi_desc:
				toolTip += "\\n"
				for point_idx in sorted(self.mount_upgrade_multi_desc.keys()):
					toolTip += self.mount_upgrade_multi_desc[point_idx]
					toolTip += "\\n"

			if self.endTime > 0:
				leftTime = localeInfo.SecondToDHM(self.endTime - app.GetGlobalTimeStamp())
				toolTip += " (%s : %s)" % (localeInfo.LEFT_TIME, leftTime)

			self.SetToolTipText(toolTip, AFFECT_TOOLTIP_POS_X, AFFECT_TOOLTIP_POS_Y, True, 15)


# In `__IsOfficialAffectEnabled`, extend the if-statement with:
		if _affectType in (chr.AFFECT_SKILL_MOUNT_UPGRADE_NIMBLE, chr.AFFECT_SKILL_MOUNT_UPGRADE_EXP,
			chr.AFFECT_SKILL_MOUNT_UPGRADE_SPEED, chr.AFFECT_SKILL_MOUNT_UPGRADE_GYENONGGONG,
			chr.AFFECT_SKILL_MOUNT_UPGRADE_INVINCIBLITIY, chr.AFFECT_SKILL_MOUNT_UPGRADE_KNOCKBACK,
			chr.AFFECT_SKILL_MOUNT_UPGRADE_TO_STRONG, chr.AFFECT_SKILL_MOUNT_UPGRADE_EFFECT_UP,
			chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_STR, chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_HP,
			chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_MOVE, chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_IMMUNE,
			chr.AFFECT_SKILL_MOUNT_UPGRADE_HIT_PCT):
			return app.ENABLE_MOUNT_UPGRADE_SYSTEM

# In `__init__`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
			self.__mount_upgrade_passive_skill = {}


# In `ClearAllAffects`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
			self.__mount_upgrade_passive_skill = {}


# In `BINARY_NEW_AddAffect`, extend the if-statement with:
		if self.affectImageDict.has_key(affect):
			if app.ENABLE_SET_ITEM and affect == chr.AFFECT_SET_ITEM or affect >= chr.AFFECT_SET_ITEM_SET_VALUE_1 and affect <= chr.AFFECT_SET_ITEM_SET_VALUE_5:
				self.affectImageDict[affect].UpdateSetItemDescription(affect)
			if app.ENABLE_SNOWFLAKE_STICK_EVENT and affect == chr.AFFECT_SNOWFLAKE_STICK_EVENT_SNOWFLAKE_BUFF:
				self.affectImageDict[affect].UpdateSnowflakeStickEventSnowflakeBuffDescription()
			if app.ENABLE_TREASURE_HUNT and affect in (chr.AFFECT_INCREASE_REFINE_PCT, chr.AFFECT_REFINE_FREE_MATERIAL):
				self.affectImageDict[affect].UpdateTreasureHuntRefineBuffDescription()
			if app.ENABLE_MOUNT_UPGRADE_SYSTEM and self.__IsMountUpgradePassiveAffect(affect) and pointIdx:
				if self.AFFECT_DATA_DICT.has_key(affect):
					lineDesc = self.AFFECT_DATA_DICT[affect][0]
					lineDesc = lineDesc(float(value))
					self.affectImageDict[affect].AddMultiLineMountUpgradeDescription(pointIdx, lineDesc)
					self.affectImageDict[affect].UpdateDescriptionMountUpgradeSkill()
			return


# In `BINARY_NEW_AddAffect`, extend the elif-statement with:
			elif app.ENABLE_MOUNT_UPGRADE_SYSTEM and self.__IsMountUpgradePassiveAffect(affect):
				image.SetClock(False)
				if pointIdx:
					image.AddMultiLineMountUpgradeDescription(pointIdx, description)
				image.UpdateDescriptionMountUpgradeSkill()


# In `BINARY_NEW_RemoveAffect`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM and self.__IsMountUpgradePassiveAffect(affect):
			self.BINARY_NEW_Remove_MountUpgrade_Passive_Skill_Affect(affect, pointIdx)
			return


# Add anywhere in the AffectShower class:
	if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
		def __IsMountUpgradePassiveAffect(self, affect):
			return affect in (
				chr.AFFECT_SKILL_MOUNT_UPGRADE_NIMBLE,
				chr.AFFECT_SKILL_MOUNT_UPGRADE_EXP,
				chr.AFFECT_SKILL_MOUNT_UPGRADE_SPEED,
				chr.AFFECT_SKILL_MOUNT_UPGRADE_GYENONGGONG,
				chr.AFFECT_SKILL_MOUNT_UPGRADE_INVINCIBLITIY,
				chr.AFFECT_SKILL_MOUNT_UPGRADE_KNOCKBACK,
				chr.AFFECT_SKILL_MOUNT_UPGRADE_TO_STRONG,
				chr.AFFECT_SKILL_MOUNT_UPGRADE_EFFECT_UP,
				chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_STR,
				chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_HP,
				chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_MOVE,
				chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_IMMUNE,
				chr.AFFECT_SKILL_MOUNT_UPGRADE_HIT_PCT,
			)

		def BINARY_NEW_Remove_MountUpgrade_Passive_Skill_Affect(self, affect, point_idx):
			if not self.affectImageDict.has_key(affect):
				return

			image = self.affectImageDict[affect]
			image.RemoveMountUpgradeMultiDescription(point_idx)
			if image.GetMountUpgradeMultiDescriptionCount() <= 0:
				self.__RemoveAffect(affect)
			else:
				image.UpdateDescriptionMountUpgradeSkill()
			self.__ArrangeImageList()
