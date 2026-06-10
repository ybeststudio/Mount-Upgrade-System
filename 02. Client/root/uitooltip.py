# In `AddItemData`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
			if itemType == item.ITEM_TYPE_SKILLBOOK and item.SKILL_BOOK_HORSE == itemSubType:
				self.ClearToolTip()
				self.SetTitle(item.GetItemName())
				self.AppendDescription(item.GetItemDescription(), 26)
				self.AppendDescription(item.GetItemSummary(), 26, self.CONDITION_COLOR)
				if app.ENABLE_SOULBIND_SYSTEM:
					self.__AppendSealInformation(window_type, slotIndex)
				if osfInfo.SHOW_ITEM_VNUM_TOOTIP == True:
					if chr.IsGameMaster(player.GetMainCharacterIndex()):
						self.AppendSpace(5)
						self.AppendTextLine("|cffffcc00Vnum: ({})|r | |cFFFF7F50Type: ({})|r | |cffADFF2FSubtype: ({})|r".format(itemVnum, itemType, itemSubType))
						if metinSlot:
							self.AppendSpace(3)
							self.AppendTextLine("|cFFD2691ESocket: (|r|cffff00ff{}|r|cFFD2691E)|r".format(' - '.join([str(i) for i in metinSlot])))
				self.ShowToolTip()
				return


# In `SetSkillNew`, extend the if-statement with:
		elif player.SKILL_INDEX_RIDING == skillIndex:

			slotIndex = player.GetSkillSlotIndex(skillIndex)
			if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
				self.AppendSupportSkillDefaultData(skillIndex, skillGrade, skillLevel, player.HORSE_MAX_LEVEL)
			else:
				self.AppendSupportSkillDefaultData(skillIndex, skillGrade, skillLevel, 30)

		elif app.ENABLE_MOUNT_UPGRADE_SYSTEM and skill.SKILL_TYPE_MOUNT_UPGRADE == skill.GetSkillType(skillIndex):

			if self.SKILL_TOOL_TIP_WIDTH != self.toolTipWidth:
				self.toolTipWidth = self.SKILL_TOOL_TIP_WIDTH
				self.ResizeToolTip()

			import mount
			slotIndex = player.GetSkillSlotIndex(skillIndex)
			if slotIndex:
				skillLevel = player.GetSkillLevel(slotIndex)

			self.AppendDefaultData(skillIndex, skillGrade)
			self.AppendSkillConditionData(skillIndex)
			self.AppendMountUpgradeSkillDescription(skillIndex, skillLevel, self.ENABLE_COLOR)
