import ui
import app
import chat
import mount
import player
import localeInfo
import uiScriptLocale
import uiCommon
import uiToolTip
import wndMgr

class MountUpgradeSystemWindow(ui.ScriptWindow):
	def __init__(self):
		ui.ScriptWindow.__init__(self)
		self.__is_loaded = False
		self.__is_lock = False
		self.__interface = None
		self.__popup_feed_window = None
		self.__popup_level_up_window = None
		self.__popup_skill_level_up_window = None
		self.__result_popup_window = None
		self.__skill_levelup_skill_vnum = 0
		self.__toolTip = uiToolTip.ToolTip()
		self.__toolTip.ClearToolTip()
		self.__LoadWindow()

	def __ClearMountHandler(self):
		try:
			if mount.GetMountHandler() is self:
				mount.SetMountHandler(None)
		except:
			pass

	def __del__(self):
		self.__ClearMountHandler()
		ui.ScriptWindow.__del__(self)

	def BindInterfaceClass(self, interface):
		self.__interface = interface

	def Destroy(self):
		self.__ClearMountHandler()
		self.ClearDictionary()
		self.__is_loaded = False
		self.Hide()

	def __LoadWindow(self):
		try:
			pyScrLoader = ui.PythonScriptLoader()
			pyScrLoader.LoadScriptFile(self, "uiscript/mountupgradesystemwindow.py")
		except:
			import exception
			exception.Abort("MountUpgradeSystemWindow.__LoadWindow")

		self.__BindObject()
		self.__BindEvent()
		self.__is_loaded = True
		mount.SetMountHandler(self)

	def __BindObject(self):
		self.board = self.GetChild("board")
		self.__cur_level_text_object = self.GetChild("cur_level_text")
		self.__next_level_text_object = self.GetChild("next_level_text")
		self.__level_up_arrow_image = self.GetChild("level_up_arrow")
		self.__exp_gauge_bar = self.GetChild("exp_gauge_bar")
		self.__exp_gauge_text_object = self.GetChild("exp_gauge_text")
		self.__give_feed_button = self.GetChild("feed_button")
		self.__level_up_button = self.GetChild("level_up_button")
		self.__cost_gold_text_object = self.GetChild("yang_text")
		self.__retry_cost_gem_text_object = self.GetChild("gem_text")
		self.yang_icon = self.GetChild("yang_icon")
		self.gem_icon = self.GetChild("gem_icon")

	def __BindEvent(self):
		self.board.SetCloseEvent(ui.__mem_func__(self.Close))
		self.__give_feed_button.SetEvent(ui.__mem_func__(self.__OnClickGiveFeedButton))
		self.__level_up_button.SetEvent(ui.__mem_func__(self.__OnClickLevelUpButton))

		if self.yang_icon:
			self.yang_icon.SetEvent(ui.__mem_func__(self.__OverInYangToolTip), "mouse_over_in")
			self.yang_icon.SetEvent(ui.__mem_func__(self.__OverOutToolTip), "mouse_over_out")
		if self.gem_icon:
			self.gem_icon.SetEvent(ui.__mem_func__(self.__OverInGemToolTip), "mouse_over_in")
			self.gem_icon.SetEvent(ui.__mem_func__(self.__OverOutToolTip), "mouse_over_out")

	def __GetHorseMaxLevel(self):
		try:
			return player.HORSE_MAX_LEVEL
		except:
			return mount.MOUNT_UPGRADE_SYSTEM_HORSE_LEVEL_MAX

	def __EnsureMountHandler(self):
		try:
			mount.SetMountHandler(self)
		except:
			pass

	def MountUpgradeSystemProcess(self, type, data=None):
		self.__EnsureMountHandler()

		if type == mount.MOUNT_UPGRADE_SYSTEM_GC_SUBHEADER_OPEN:
			self.__Open()
		else:
			self.__is_lock = False
			self.__GiveFeedButtonEnable()
			self.__LevelUpButtonEnable()
			self.Refresh()

			if type == mount.MOUNT_UPGRADE_SYSTEM_GC_SUBHEADER_HORSE_LEVEL_UP and data is not None:
				is_failed = 0
				cur_horse_level = 0
				if isinstance(data, tuple) and len(data) >= 2:
					cur_horse_level = data[0]
					is_failed = data[1]
				if is_failed:
					self.__EnableResultPopUpWindow(localeInfo.MOUNT_UPGRADE_SYSTEM_LEVEL_UP_RESULT_PERCENT_FAIL)
				else:
					self.__EnableResultPopUpWindow(
						localeInfo.MOUNT_UPGRADE_SYSTEM_LEVEL_UP_RESULT_PERCENT_SUCCESS % cur_horse_level)

	def MountUpgradeSystemChat(self, chat_type, value):
		self.__EnsureMountHandler()
		if chat_type == 0:
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.MOUNT_UPGRADE_SYSTEM_BANN_WHILE_MOUNTING)
			self.__UnlockUI()
		elif chat_type == 1:
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.MOUNT_UPGRADE_SYSTEM_LEVEL_UP_RESULT_YANG_OR_FEED_NOT_ENOUGH)
			self.__UnlockUI()
		elif chat_type == 2:
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.MOUNT_UPGRADE_SYSTEM_LEVEL_UP_RESULT_GEM_NOT_ENOUGH)
			self.__UnlockUI()
		elif chat_type == 3:
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.MOUNT_UPGRADE_SYSTEM_LEVEL_UP_RESULT_PERCENT_SUCCESS % value)
		elif chat_type == 4:
			chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.MOUNT_UPGRADE_SYSTEM_LEVEL_UP_RESULT_PERCENT_FAIL)
		elif chat_type == 5:
			if hasattr(localeInfo, "MOUNT_UPGRADE_SYSTEM_EXP_MOUNT_FEED"):
				chat.AppendChat(chat.CHAT_TYPE_INFO, localeInfo.MOUNT_UPGRADE_SYSTEM_EXP_MOUNT_FEED % value)

	def __UnlockUI(self):
		self.__is_lock = False
		self.__GiveFeedButtonEnable()
		self.__LevelUpButtonEnable()

	def Open(self):
		self.__Open()

	def Hide(self):
		ui.ScriptWindow.Hide(self)

	def __Open(self):
		if self.__is_lock:
			return

		self.__EnsureMountHandler()
		self.SetCenterPosition()
		self.Refresh()
		self.__GiveFeedButtonEnable()
		self.__LevelUpButtonEnable()
		self.Show()

	def Refresh(self):
		if not self.__is_loaded:
			return

		level = mount.GetHorseLevel()
		max_level = self.__GetHorseMaxLevel()
		curValue = mount.GetMountExistingExp()
		maxValue = mount.GetMountNecessaryExp()
		maxExp = maxValue > 0 and curValue >= maxValue

		self.__cur_level_text_object.SetText(str(level))

		if level >= max_level:
			self.__next_level_text_object.Hide()
		else:
			self.__next_level_text_object.SetText(str(level + 1))
			self.__next_level_text_object.Show()

		yangCost = mount.GetMountUpGradePrice()
		if level < max_level and maxExp:
			self.__cost_gold_text_object.Show()
			self.__cost_gold_text_object.SetText(localeInfo.NumberToMoneyString(yangCost))
		else:
			self.__cost_gold_text_object.Hide()

		if mount.IsMountUpGradeFail() > mount.MOUNT_UP_GRADE_FAIL_OFF and maxExp:
			if level < mount.HORSE_LEVEL_DETERMINES_GEM_COST:
				gemCost = mount.HORSE_LOWER_LEVEL_RETRY_GEM_COST
			else:
				gemCost = mount.HORSE_UPPER_LEVEL_RETRY_GEM_COST
			self.__retry_cost_gem_text_object.SetText(str(gemCost))
			self.__retry_cost_gem_text_object.Show()
		else:
			self.__retry_cost_gem_text_object.Hide()

		level_up = level < max_level and maxExp
		if level_up:
			self.__level_up_button.Show()
			self.__give_feed_button.Hide()
			self.__level_up_arrow_image.Show()
		else:
			self.__level_up_button.Hide()
			self.__give_feed_button.Show()
			self.__level_up_arrow_image.Hide()

		if maxExp:
			curValue = maxValue

		if maxValue > 0:
			self.__exp_gauge_bar.SetPercentage(curValue, maxValue)
		else:
			self.__exp_gauge_bar.SetPercentage(0, 1)

		if level >= max_level:
			self.__exp_gauge_text_object.SetText("MAX")
		else:
			self.__exp_gauge_text_object.SetText("%d/%d" % (curValue, maxValue))

	def __IsConquerorLevelRequired(self):
		if not app.ENABLE_CONQUEROR_LEVEL:
			return False
		level = mount.GetHorseLevel()
		if level < mount.MOUNT_UPGRADE_SYSTEM_HORSE_MIN_LEVEL_CONQUEROR - 1:
			return False
		return player.GetStatus(player.POINT_CONQUEROR_LEVEL) < 1

	def __GiveFeedButtonEnable(self):
		if self.__give_feed_button:
			self.__give_feed_button.Enable()

	def __GiveFeedButtonDisable(self):
		if self.__give_feed_button:
			self.__give_feed_button.Disable()

	def __LevelUpButtonEnable(self):
		if self.__level_up_button:
			self.__level_up_button.Enable()

	def __LevelUpButtonDisable(self):
		if self.__level_up_button:
			self.__level_up_button.Disable()

	def __OnClickGiveFeedButton(self):
		if self.__is_lock:
			return

		level = mount.GetHorseLevel()
		max_level = self.__GetHorseMaxLevel()

		if level >= max_level:
			self.__EnableResultPopUpWindow(uiScriptLocale.MOUNT_UPGRADE_SYSTEM_DISABLE_GIVE_FEED_BUTTON_POPUP_MESSAGE)
			return

		if player.GetItemCountByVnum(mount.HORSE_FEED_ITEM_ID) < mount.HORSE_FEED_EXP_COUNT:
			self.__EnableResultPopUpWindow(uiScriptLocale.MOUNT_UPGRADE_SYSTEM_GIVE_FEED_NOT_ENOUGH)
			return

		curValue = mount.GetMountExistingExp()
		maxValue = mount.GetMountNecessaryExp()
		if maxValue > 0 and curValue >= maxValue:
			self.__EnableResultPopUpWindow(uiScriptLocale.MOUNT_UPGRADE_SYSTEM_DISABLE_GIVE_FEED_BUTTON_POPUP_MESSAGE)
			return

		if self.__popup_feed_window:
			return

		popup = uiCommon.QuestionDialog()
		popup.SetText(uiScriptLocale.MOUNT_UPGRADE_SYSTEM_GIVE_FEED_BUTTON_POPUP_MESSAGE)
		popup.SetAcceptEvent(ui.__mem_func__(self.__SendMountUpgradeGiveFeed))
		popup.SetCancelEvent(ui.__mem_func__(self.__ClosePopupFeedWindow))
		popup.Open()
		self.__popup_feed_window = popup

	def __OnClickLevelUpButton(self):
		if self.__is_lock:
			return

		if self.__IsConquerorLevelRequired():
			self.__EnableResultPopUpWindow(localeInfo.MOUNT_UPGRADE_SYSTEM_CONQUEROR_LEVEL_MSG)
			return

		level = mount.GetHorseLevel()
		max_level = self.__GetHorseMaxLevel()

		if level >= max_level:
			self.__EnableResultPopUpWindow(uiScriptLocale.MOUNT_UPGRADE_SYSTEM_DISABLE_GIVE_FEED_BUTTON_POPUP_MESSAGE)
			return

		if player.GetItemCountByVnum(mount.HORSE_FEED_ITEM_ID) < mount.HORSE_FEED_LEVEL_COUNT:
			self.__EnableResultPopUpWindow(uiScriptLocale.MOUNT_UPGRADE_SYSTEM_GIVE_FEED_NOT_ENOUGH)
			return

		curValue = mount.GetMountExistingExp()
		maxValue = mount.GetMountNecessaryExp()
		if curValue < maxValue:
			return

		yangCost = mount.GetMountUpGradePrice()
		if level < mount.HORSE_LEVEL_DETERMINES_GEM_COST:
			gemCost = mount.HORSE_LOWER_LEVEL_RETRY_GEM_COST
		else:
			gemCost = mount.HORSE_UPPER_LEVEL_RETRY_GEM_COST

		if self.__popup_level_up_window:
			return

		popup = uiCommon.MountUpGradeDialog()
		popup.SetText1(localeInfo.MOUNT_UPGRADE_SYSTEM_LEVEL_UP_POPUP_MSG_LEVEL % (level + 1))

		fType = mount.IsMountUpGradeFail()
		if fType > mount.MOUNT_UP_GRADE_FAIL_OFF:
			popup.SetText2(localeInfo.MOUNT_UPGRADE_SYSTEM_LEVEL_UP_POPUP_RETRY_COST % (
				mount.HORSE_FEED_LEVEL_COUNT, gemCost, yangCost))
		else:
			popup.SetText2(localeInfo.MOUNT_UPGRADE_SYSTEM_LEVEL_UP_POPUP_COST % (
				mount.HORSE_FEED_LEVEL_COUNT, yangCost))

		popup.SetAcceptEvent(ui.__mem_func__(self.__SendMountUpgradeLevelUp))
		popup.SetCancelEvent(ui.__mem_func__(self.__ClosePopupLevelUpWindow))
		popup.Open()
		self.__popup_level_up_window = popup

	def __SendMountUpgradeGiveFeed(self):
		self.__ClosePopupFeedWindow()
		self.__is_lock = True
		self.__GiveFeedButtonDisable()
		self.__LevelUpButtonDisable()
		mount.SendMountUpgradeSystemGiveFeed()

	def __SendMountUpgradeLevelUp(self):
		self.__ClosePopupLevelUpWindow()
		self.__is_lock = True
		self.__GiveFeedButtonDisable()
		self.__LevelUpButtonDisable()
		mount.SendMountUpgradeSystemLevelUp()

	def __SendSkillLevelUp(self):
		self.__ClosePopupSkillLevelUpWindow()
		mount.SendMountUpgradeSystemSkillLevelUp()

	def ClickMountSkillLevelUpButton(self, skill_vnum):
		if not skill_vnum:
			return

		self.__skill_levelup_skill_vnum = skill_vnum

		if self.__popup_skill_level_up_window:
			return

		popup = uiCommon.QuestionDialog()
		popup.SetText(localeInfo.TOOLTIP_MOUNT_UPGRADE_SKILL_LEVEL_UP % (
			mount.MOUNT_UPGRADE_SKILL_LEVELUP_COST_GEM, mount.MOUNT_UPGRADE_SKILL_LEVELUP_COST_YANG))
		popup.SetAcceptEvent(ui.__mem_func__(self.__SendSkillLevelUp))
		popup.SetCancelEvent(ui.__mem_func__(self.__ClosePopupSkillLevelUpWindow))
		popup.Open()
		self.__popup_skill_level_up_window = popup

	def __ClosePopupFeedWindow(self):
		if self.__popup_feed_window:
			self.__popup_feed_window.Close()
			self.__popup_feed_window = None

	def __ClosePopupLevelUpWindow(self):
		if self.__popup_level_up_window:
			self.__popup_level_up_window.Close()
			self.__popup_level_up_window = None

	def __ClosePopupSkillLevelUpWindow(self):
		if self.__popup_skill_level_up_window:
			self.__popup_skill_level_up_window.Close()
			self.__popup_skill_level_up_window = None

	def __EnableResultPopUpWindow(self, message):
		if self.__result_popup_window:
			self.__result_popup_window.Close()
			self.__result_popup_window = None

		popup = uiCommon.PopupDialog()
		popup.SetText(message)
		popup.Open()
		self.__result_popup_window = popup

	def __DisableResultPopUpWindow(self):
		if self.__result_popup_window:
			self.__result_popup_window.Close()
			self.__result_popup_window = None

	def __OverInYangToolTip(self):
		arglen = len(localeInfo.CHEQUE_SYSTEM_UNIT_YANG)
		pos_x, pos_y = wndMgr.GetMousePosition()
		self.__toolTip.ClearToolTip()
		self.__toolTip.SetThinBoardSize(5 * arglen)
		self.__toolTip.SetToolTipPosition(pos_x + 5, pos_y - 10)
		self.__toolTip.AppendTextLine(localeInfo.CHEQUE_SYSTEM_UNIT_YANG, 0xFFFFFF00)
		self.__toolTip.Show()

	def __OverInGemToolTip(self):
		arglen = len(localeInfo.GEM_SYSTEM_NAME)
		pos_x, pos_y = wndMgr.GetMousePosition()
		self.__toolTip.ClearToolTip()
		self.__toolTip.SetThinBoardSize(5 * arglen)
		self.__toolTip.SetToolTipPosition(pos_x + 5, pos_y - 10)
		self.__toolTip.AppendTextLine(localeInfo.GEM_SYSTEM_NAME, 0xFFFFFF00)
		self.__toolTip.Show()

	def __OverOutToolTip(self):
		self.__toolTip.Hide()

	def Close(self):
		self.__is_lock = False
		self.__ClosePopupFeedWindow()
		self.__ClosePopupLevelUpWindow()
		self.__ClosePopupSkillLevelUpWindow()
		self.__DisableResultPopUpWindow()
		mount.SendMountUpgradeSystemClose()
		self.Hide()

	def OnPressEscapeKey(self):
		self.Close()
		return True

	def OnGameOver(self):
		self.Close()

	def OpenCharacterWindowWithState(self, state, skill_group):
		if self.__interface and hasattr(self.__interface, "OpenCharacterWindowWithState"):
			self.__interface.OpenCharacterWindowWithState(state, skill_group)

	def OnUpdate(self):
		pass
