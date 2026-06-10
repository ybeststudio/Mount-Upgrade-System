# Add to the imports:
if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
	import skill


# In `__UseItem`, extend the if-statement with:
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
			itemType = item.GetItemType()
			itemSubType = item.GetItemSubType()
			skillVnum = item.GetValue(0)
			isMountSkillBook = item.ITEM_TYPE_SKILLBOOK == itemType and (
				item.SKILL_BOOK_HORSE == itemSubType or (skillVnum >= 285 and skillVnum <= 311))
			if isMountSkillBook:
				if skillVnum:
					skillName = skill.GetSkillName(skillVnum, 0)
					self.questionDialog = uiCommon.QuestionDialog()
					self.questionDialog.SetText(localeInfo.TOOLTIP_MOUNT_UPGRADE_SKILLBOOK_USE % skillName)
					self.questionDialog.SetAcceptEvent(ui.__mem_func__(self.__UseItemQuestionDialog_OnAccept))
					self.questionDialog.SetCancelEvent(ui.__mem_func__(self.__UseItemQuestionDialog_OnCancel))
					self.questionDialog.Open()
					self.questionDialog.slotIndex = slotIndex
					self.questionDialog.slotWindow = slotWindow
					constInfo.SET_ITEM_QUESTION_DIALOG_STATUS(1)
					return
