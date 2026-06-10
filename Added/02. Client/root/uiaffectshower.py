import ui
import localeInfo
import chr
import item
import app
import skill
import player
import uiToolTip
import math

if app.ENABLE_SET_ITEM:
	import uiToolTip

AFFECT_TOOLTIP_POS_X = 0
AFFECT_TOOLTIP_POS_Y = 40

# WEDDING
class LovePointImage(ui.ExpandedImageBox):

	FILE_PATH = "d:/ymir work/ui/pattern/LovePoint/"
	FILE_DICT = {
		0 : FILE_PATH + "01.dds",
		1 : FILE_PATH + "02.dds",
		2 : FILE_PATH + "02.dds",
		3 : FILE_PATH + "03.dds",
		4 : FILE_PATH + "04.dds",
		5 : FILE_PATH + "05.dds",
	}

	def __init__(self):
		ui.ExpandedImageBox.__init__(self)

		self.loverName = ""
		self.lovePoint = 0

		self.toolTip = uiToolTip.ToolTip(100)
		self.toolTip.HideToolTip()

	def __del__(self):
		ui.ExpandedImageBox.__del__(self)

	def SetLoverInfo(self, name, lovePoint):
		self.loverName = name
		self.lovePoint = lovePoint
		self.__Refresh()

	def OnUpdateLovePoint(self, lovePoint):
		self.lovePoint = lovePoint
		self.__Refresh()

	def __Refresh(self):
		self.lovePoint = max(0, self.lovePoint)
		self.lovePoint = min(100, self.lovePoint)

		if 0 == self.lovePoint:
			loveGrade = 0
		else:
			loveGrade = self.lovePoint / 25 + 1
		fileName = self.FILE_DICT.get(loveGrade, self.FILE_PATH+"00.dds")

		try:
			self.LoadImage(fileName)
		except:
			import dbg
			dbg.TraceError("LovePointImage.SetLoverInfo(lovePoint=%d) - LoadError %s" % (self.lovePoint, fileName))

		self.SetScale(0.7, 0.7)

		self.toolTip.ClearToolTip()
		self.toolTip.SetTitle(self.loverName)
		self.toolTip.AppendTextLine(localeInfo.AFF_LOVE_POINT % (self.lovePoint))
		self.toolTip.ResizeToolTip()

	def OnMouseOverIn(self):
		self.toolTip.ShowToolTip()

	def OnMouseOverOut(self):
		self.toolTip.HideToolTip()
# END_OF_WEDDING

if app.ENABLE_CONQUEROR_LEVEL:
	class SungMaAffectImage(ui.ExpandedImageBox):
		def __init__(self, str, hp, move, immune):
			ui.ExpandedImageBox.__init__(self)

			self.sungmaStr = str
			self.sungmaHp = hp
			self.sungmaMove = move
			self.sungmaImmune = immune

			self.toolTip = uiToolTip.ToolTip(100)
			self.toolTip.HideToolTip()

		def SetSungmaValue(self, str, hp, move, immune):
			self.sungmaStr = str
			self.sungmaHp = hp
			self.sungmaMove = move
			self.sungmaImmune = immune

			self.Refresh()

		def Refresh(self):
			fileName = "d:/ymir work/ui/skill/common/affect/sungma_buff.sub"
			try:
				self.LoadImage(fileName)
			except:
				import dbg
				dbg.TraceError("SungMaAffectImage.SetSungmaValue- LoadError %s" % (fileName))

			self.SetScale(0.7, 0.7)

			self.toolTip.ClearToolTip()

			self.toolTip.AppendSpace(5)
			self.toolTip.AutoAppendTextLine(localeInfo.TOOLTIP_AFFECT_SUNGMA_DESC, self.toolTip.TITLE_COLOR, False)
			self.toolTip.AutoAppendTextLine(localeInfo.TOOLTIP_AFFECT_SUNGMA_STR % (self.sungmaStr), self.toolTip.FONT_COLOR, False)
			self.toolTip.AutoAppendTextLine(localeInfo.TOOLTIP_AFFECT_SUNGMA_HP % (self.sungmaHp), self.toolTip.FONT_COLOR, False)
			self.toolTip.AutoAppendTextLine(localeInfo.TOOLTIP_AFFECT_SUNGMA_MOVE % (self.sungmaMove), self.toolTip.FONT_COLOR, False)
			self.toolTip.AutoAppendTextLine(localeInfo.TOOLTIP_AFFECT_SUNGMA_IMMUNE % (self.sungmaImmune), self.toolTip.FONT_COLOR, False)
			self.toolTip.ResizeToolTip()

		def OnMouseOverIn(self):
			self.toolTip.ShowToolTip()

		def OnMouseOverOut(self):
			self.toolTip.HideToolTip()

class HorseImage(ui.ExpandedImageBox):

	FILE_PATH = "d:/ymir work/ui/pattern/HorseState/"

	FILE_DICT = {
		00 : FILE_PATH + "00.dds",
		01 : FILE_PATH + "00.dds",
		02 : FILE_PATH + "00.dds",
		03 : FILE_PATH + "00.dds",
		10 : FILE_PATH + "10.dds",
		11 : FILE_PATH + "11.dds",
		12 : FILE_PATH + "12.dds",
		13 : FILE_PATH + "13.dds",
		20 : FILE_PATH + "20.dds",
		21 : FILE_PATH + "21.dds",
		22 : FILE_PATH + "22.dds",
		23 : FILE_PATH + "23.dds",
		30 : FILE_PATH + "30.dds",
		31 : FILE_PATH + "31.dds",
		32 : FILE_PATH + "32.dds",
		33 : FILE_PATH + "33.dds",
	}

	def __init__(self):
		ui.ExpandedImageBox.__init__(self)

		#self.textLineList = []
		self.toolTip = uiToolTip.ToolTip(100)
		self.toolTip.HideToolTip()

	def __del__(self):
		ui.ExpandedImageBox.__del__(self)

	def __GetHorseGrade(self, level):
		if 0 == level:
			return 0

		return (level-1)/10 + 1

	def SetState(self, level, health, battery):
		#self.textLineList=[]
		self.toolTip.ClearToolTip()

		if level>0:

			try:
				grade = self.__GetHorseGrade(level)
				self.__AppendText(localeInfo.LEVEL_LIST[grade])
			except IndexError:
				print "HorseImage.SetState(level=%d, health=%d, battery=%d) - Unknown Index" % (level, health, battery)
				return

			try:
				healthName=localeInfo.HEALTH_LIST[health]
				if len(healthName)>0:
					self.__AppendText(healthName)
			except IndexError:
				print "HorseImage.SetState(level=%d, health=%d, battery=%d) - Unknown Index" % (level, health, battery)
				return

			if health>0:
				if battery==0:
					self.__AppendText(localeInfo.NEEFD_REST)

			try:
				fileName=self.FILE_DICT[health*10+battery]
			except KeyError:
				print "HorseImage.SetState(level=%d, health=%d, battery=%d) - KeyError" % (level, health, battery)

			try:
				self.LoadImage(fileName)
			except:
				print "HorseImage.SetState(level=%d, health=%d, battery=%d) - LoadError %s" % (level, health, battery, fileName)

		self.SetScale(0.7, 0.7)

	def __AppendText(self, text):

		self.toolTip.AppendTextLine(text)
		self.toolTip.ResizeToolTip()

		#x=self.GetWidth()/2
		#textLine = ui.TextLine()
		#textLine.SetParent(self)
		#textLine.SetSize(0, 0)
		#textLine.SetOutline()
		#textLine.Hide()
		#textLine.SetPosition(x, 40+len(self.textLineList)*16)
		#textLine.SetText(text)
		#self.textLineList.append(textLine)

	def OnMouseOverIn(self):
		#for textLine in self.textLineList:
		#	textLine.Show()

		self.toolTip.ShowToolTip()

	def OnMouseOverOut(self):
		#for textLine in self.textLineList:
		#	textLine.Hide()

		self.toolTip.HideToolTip()


# AUTO_POTION
class AutoPotionImage(ui.ExpandedImageBox):

	FILE_PATH_HP = "d:/ymir work/ui/pattern/auto_hpgauge/"
	FILE_PATH_SP = "d:/ymir work/ui/pattern/auto_spgauge/"

	def __init__(self):
		ui.ExpandedImageBox.__init__(self)

		self.loverName = ""
		self.lovePoint = 0
		self.potionType = player.AUTO_POTION_TYPE_HP
		self.filePath = ""

		self.toolTip = uiToolTip.ToolTip(100)
		self.toolTip.HideToolTip()

	def __del__(self):
		ui.ExpandedImageBox.__del__(self)

	def SetPotionType(self, type):
		self.potionType = type

		if player.AUTO_POTION_TYPE_HP == type:
			self.filePath = self.FILE_PATH_HP
		elif player.AUTO_POTION_TYPE_SP == type:
			self.filePath = self.FILE_PATH_SP


	def OnUpdateAutoPotionImage(self):
		self.__Refresh()

	def __Refresh(self):
		print "__Refresh"

		isActivated, currentAmount, totalAmount, slotIndex = player.GetAutoPotionInfo(self.potionType)

		amountPercent = (float(currentAmount) / totalAmount) * 100.0
		grade = math.ceil(amountPercent / 20)

		if 5.0 > amountPercent:
			grade = 0

		if 80.0 < amountPercent:
			grade = 4
			if 90.0 < amountPercent:
				grade = 5

		fmt = self.filePath + "%.2d.dds"
		fileName = fmt % grade

		print self.potionType, amountPercent, fileName

		try:
			self.LoadImage(fileName)
		except:
			import dbg
			dbg.TraceError("AutoPotionImage.__Refresh(potionType=%d) - LoadError %s" % (self.potionType, fileName))

		self.SetScale(0.7, 0.7)

		self.toolTip.ClearToolTip()

		if player.AUTO_POTION_TYPE_HP == type:
			self.toolTip.SetTitle(localeInfo.TOOLTIP_AUTO_POTION_HP)
		else:
			self.toolTip.SetTitle(localeInfo.TOOLTIP_AUTO_POTION_SP)

		self.toolTip.AppendTextLine(localeInfo.TOOLTIP_AUTO_POTION_REST	% (amountPercent))
		self.toolTip.ResizeToolTip()

	def OnMouseOverIn(self):
		self.toolTip.ShowToolTip()

	def OnMouseOverOut(self):
		self.toolTip.HideToolTip()
# END_OF_AUTO_POTION

if app.ENABLE_GROWTH_PET_SYSTEM:
	# GROWTH PET IMAGE
	class GrowthPetImage(ui.ExpandedImageBox):

		def __init__(self):
			ui.ExpandedImageBox.__init__(self)

			self.toolTipText = None
			self.description = None

		def __del__(self):
			ui.ExpandedImageBox.__del__(self)

		def SetToolTipText(self, text, x = 0, y = -19):

			if not self.toolTipText:
				textLine = ui.TextLine()
				textLine.SetParent(self)
				textLine.SetSize(0, 0)
				textLine.SetOutline()
				textLine.Hide()
				self.toolTipText = textLine

			self.toolTipText.SetText(text)
			w, h = self.toolTipText.GetTextSize()
			if localeInfo.IsARABIC():
				self.toolTipText.SetPosition(w+20, y)
			else:
				self.toolTipText.SetPosition(max(0, x + self.GetWidth()/2 - w/2), y)

		def SetDescription(self, description):
			self.description = description

		def OnMouseOverIn(self):
			if self.toolTipText:
				self.toolTipText.Show()

		def OnMouseOverOut(self):
			if self.toolTipText:
				self.toolTipText.Hide()

	# END OF GROWTH PET IMAGE

class AffectImage(ui.ExpandedImageBox):

	def __init__(self):
		ui.ExpandedImageBox.__init__(self)

		if app.ENABLE_AFFECT_RENEWAL:
			self.tool_tip = uiToolTip.ToolTip(100)
			self.tool_tip.HideToolTip()
			self.tool_tip_text = ""
		else:
			self.toolTipText = None

		self.isSkillAffect = True
		self.description = None
		self.endTime = 0
		self.affect = None
		self.isClocked = True

		if app.ENABLE_SET_ITEM:
			self.tooltipItem = uiToolTip.ItemToolTip()
			self.tooltipItem.Hide()

		if app.ENABLE_SET_ITEM or app.ENABLE_SNOWFLAKE_STICK_EVENT:
			self.affect_dict = {}

		if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
			self.mount_upgrade_multi_desc = {}

	if app.ENABLE_SET_ITEM or app.ENABLE_AFFECT_RENEWAL:
		def __del__(self):
			ui.ExpandedImageBox.__del__(self)
			if app.ENABLE_SET_ITEM:
				del self.tooltipItem

			if app.ENABLE_AFFECT_RENEWAL:
				del self.tool_tip

			if app.ENABLE_SET_ITEM or app.ENABLE_SNOWFLAKE_STICK_EVENT:
				self.affect_dict = {}

	if app.ENABLE_TREASURE_HUNT:
		def UpdateTreasureHuntRefineBuffDescription(self):
			if not self.description:
				return

			toolTip = localeInfo.TREASURE_HUNT_EVENT_BUFF_TITLE
			toolTip += "\\n"
			toolTip += self.description
			if self.endTime > 0:
				leftTime = localeInfo.SecondToDHM(self.endTime - app.GetGlobalTimeStamp())
				toolTip += " (%s : %s)" % (localeInfo.LEFT_TIME, leftTime)

			self.SetToolTipText(toolTip, AFFECT_TOOLTIP_POS_X, AFFECT_TOOLTIP_POS_Y, True, 15)

	def SetAffect(self, affect):
		self.affect = affect

	def GetAffect(self):
		return self.affect

	if app.ENABLE_AFFECT_RENEWAL:
		def SetToolTipText(self, text, x = 0, y = -19, adjust_line_height = False, line_height_distance = 20):
			self.tool_tip_text = text

	elif app.ENABLE_SET_ITEM:
		def SetToolTipText(self, text, x = 0, y = -19, adjust_line_height = False, line_height_distance = 20):
			if not self.toolTipText:
				textLine = ui.TextLine()
				textLine.SetParent(self)
				textLine.SetSize(0, 0)
				textLine.SetOutline()
				textLine.Hide()
				self.toolTipText = textLine

				if adjust_line_height:
					line_height = self.toolTipText.GetLineHeight()
					self.toolTipText.SetLineHeight(line_height + line_height_distance)

			self.toolTipText.SetText(text)
			w, h = self.toolTipText.GetTextSize()
			if localeInfo.IsARABIC():
				self.toolTipText.SetPosition(w + 20, y)
			else:
				self.toolTipText.SetPosition(max(0, x + self.GetWidth() / 2 - w / 2), y)
	else:
		def SetToolTipText(self, text, x = 0, y = -19):

			if not self.toolTipText:
				textLine = ui.TextLine()
				textLine.SetParent(self)
				textLine.SetSize(0, 0)
				textLine.SetOutline()
				textLine.Hide()
				self.toolTipText = textLine

			self.toolTipText.SetText(text)
			w, h = self.toolTipText.GetTextSize()
			if localeInfo.IsARABIC():
				self.toolTipText.SetPosition(w+20, y)
			else:
				self.toolTipText.SetPosition(max(0, x + self.GetWidth()/2 - w/2), y)

	def SetDescription(self, description):
		self.description = description

	def SetDuration(self, duration):
		self.endTime = 0
		if duration > 0:
			self.endTime = app.GetGlobalTimeStamp() + duration

	def UpdateAutoPotionDescription(self):

		potionType = 0
		if self.affect == chr.AFFECT_AUTO_HP_RECOVERY:
			potionType = player.AUTO_POTION_TYPE_HP
		else:
			potionType = player.AUTO_POTION_TYPE_SP

		isActivated, currentAmount, totalAmount, slotIndex = player.GetAutoPotionInfo(potionType)

		#print "UpdateAutoPotionDescription ", isActivated, currentAmount, totalAmount, slotIndex

		amountPercent = 0.0

		try:
			amountPercent = (float(currentAmount) / totalAmount) * 100.0
		except:
			amountPercent = 100.0

		self.SetToolTipText(self.description % amountPercent, AFFECT_TOOLTIP_POS_X, AFFECT_TOOLTIP_POS_Y)

	def SetClock(self, isClocked):
		self.isClocked = isClocked

	def UpdateDescription(self):
		if not self.isClocked:
			self.__UpdateDescription2()
			return

		if not self.description:
			return

		toolTip = self.description
		if self.endTime > 0:
			leftTime = localeInfo.SecondToDHM(self.endTime - app.GetGlobalTimeStamp())
			toolTip += " (%s : %s)" % (localeInfo.LEFT_TIME, leftTime)
		self.SetToolTipText(toolTip, AFFECT_TOOLTIP_POS_X, AFFECT_TOOLTIP_POS_Y)

	def __UpdateDescription2(self):
		if not self.description:
			return

		toolTip = self.description
		self.SetToolTipText(toolTip, AFFECT_TOOLTIP_POS_X, AFFECT_TOOLTIP_POS_Y)

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

	if app.ENABLE_SET_ITEM:
		def UpdateSetItemDescription(self, affect_type = chr.AFFECT_SET_ITEM):
			if not self.description:
				return

			if not self.tooltipItem:
				toolTip = self.description
			else:
				toolTip = self.description
				toolTip += "\\n"
				self.affect_dict = player.GetSetItemEffect(affect_type)

				if type(self.affect_dict) is dict:
					if len(self.affect_dict) > 0:
						for k in self.affect_dict.keys():
							toolTip += self.tooltipItem.GetAffectString(k, self.affect_dict[k])
							toolTip += "\\n"

			self.SetToolTipText(toolTip, AFFECT_TOOLTIP_POS_X, AFFECT_TOOLTIP_POS_Y, True, 15)

	if app.ENABLE_SNOWFLAKE_STICK_EVENT:
		def UpdateSnowflakeStickEventRankBuffDescription(self):
			return

		def UpdateSnowflakeStickEventSnowflakeBuffDescription(self, update_clock = False):
			if not self.description:
				return

			if not self.tooltipItem:
				toolTip = self.description
				if self.endTime > 0:
					leftTime = localeInfo.SecondToDHM(self.endTime - app.GetGlobalTimeStamp())
					toolTip += " (%s : %s)" % (localeInfo.LEFT_TIME, leftTime)
			else:
				toolTip = self.description
				if self.endTime > 0:
					leftTime = localeInfo.SecondToDHM(self.endTime - app.GetGlobalTimeStamp())
					toolTip += " (%s : %s)" % (localeInfo.LEFT_TIME, leftTime)

				toolTip += "\\n"
				toolTip += localeInfo.TOOLTIP_AFFECT_SNOWFLAKE_STICK_EVENT_SNOWFLAKE_BUFF_2
				toolTip += "\\n"

				if not update_clock:
					self.affect_dict = player.GetAffectData(self.affect)

				if type(self.affect_dict) is dict and self.affect_dict:
					if len(self.affect_dict) > 0:
						for k in self.affect_dict.keys():
							toolTip += self.tooltipItem.GetAffectString(k, self.affect_dict[k])
							toolTip += "\\n"

			self.SetToolTipText(toolTip, AFFECT_TOOLTIP_POS_X, AFFECT_TOOLTIP_POS_Y, True, 15)

	def SetSkillAffectFlag(self, flag):
		self.isSkillAffect = flag

	def IsSkillAffect(self):
		return self.isSkillAffect

	def OnMouseOverIn(self):
		if app.ENABLE_AFFECT_RENEWAL:
			if not self.tool_tip:
				return

			self.tool_tip.ClearToolTip()

			lines = self.tool_tip_text.split("\\n")
			if not lines:
				return

			for line in lines:
				self.tool_tip.AutoAppendTextLine(line)

			self.tool_tip.AlignHorizonalCenter()
			self.tool_tip.ShowToolTip()
		else:
			if self.toolTipText:
				self.toolTipText.Show()

	def OnMouseOverOut(self):
		if app.ENABLE_AFFECT_RENEWAL:
			if self.tool_tip:
				self.tool_tip.HideToolTip()
		else:
			if self.toolTipText:
				self.toolTipText.Hide()

class AffectShower(ui.Window):

	MALL_DESC_IDX_START = 1000
	IMAGE_STEP = 25
	AFFECT_MAX_NUM = 32

	INFINITE_AFFECT_DURATION = 0x1FFFFFFF

	if app.ENABLE_AFFECT_RENEWAL:
		BASE_POS_X = 10
		BASE_POS_Y = 10

	if app.ENABLE_WOLFMAN_CHARACTER:
		AFFECT_DATA_DICT =	{
				chr.AFFECT_POISON : (localeInfo.SKILL_TOXICDIE, "d:/ymir work/ui/skill/common/affect/poison.sub"),
				chr.AFFECT_BLEEDING : (localeInfo.SKILL_BLEEDING, "d:/ymir work/ui/skill/common/affect/poison.sub"),
				chr.AFFECT_SLOW : (localeInfo.SKILL_SLOW, "d:/ymir work/ui/skill/common/affect/slow.sub"),
				chr.AFFECT_STUN : (localeInfo.SKILL_STUN, "d:/ymir work/ui/skill/common/affect/stun.sub"),

				chr.AFFECT_ATT_SPEED_POTION : (localeInfo.SKILL_INC_ATKSPD, "d:/ymir work/ui/skill/common/affect/Increase_Attack_Speed.sub"),
				chr.AFFECT_MOV_SPEED_POTION : (localeInfo.SKILL_INC_MOVSPD, "d:/ymir work/ui/skill/common/affect/Increase_Move_Speed.sub"),
				chr.AFFECT_FISH_MIND : (localeInfo.SKILL_FISHMIND, "d:/ymir work/ui/skill/common/affect/fishmind.sub"),

				chr.AFFECT_JEONGWI : (localeInfo.SKILL_JEONGWI, "d:/ymir work/ui/skill/warrior/jeongwi_03.sub",),
				chr.AFFECT_GEOMGYEONG : (localeInfo.SKILL_GEOMGYEONG, "d:/ymir work/ui/skill/warrior/geomgyeong_03.sub",),
				chr.AFFECT_CHEONGEUN : (localeInfo.SKILL_CHEONGEUN, "d:/ymir work/ui/skill/warrior/cheongeun_03.sub",),
				chr.AFFECT_GYEONGGONG : (localeInfo.SKILL_GYEONGGONG, "d:/ymir work/ui/skill/assassin/gyeonggong_03.sub",),
				chr.AFFECT_EUNHYEONG : (localeInfo.SKILL_EUNHYEONG, "d:/ymir work/ui/skill/assassin/eunhyeong_03.sub",),
				chr.AFFECT_GWIGEOM : (localeInfo.SKILL_GWIGEOM, "d:/ymir work/ui/skill/sura/gwigeom_03.sub",),
				chr.AFFECT_GONGPO : (localeInfo.SKILL_GONGPO, "d:/ymir work/ui/skill/sura/gongpo_03.sub",),
				chr.AFFECT_JUMAGAP : (localeInfo.SKILL_JUMAGAP, "d:/ymir work/ui/skill/sura/jumagap_03.sub"),
				chr.AFFECT_HOSIN : (localeInfo.SKILL_HOSIN, "d:/ymir work/ui/skill/shaman/hosin_03.sub",),
				chr.AFFECT_BOHO : (localeInfo.SKILL_BOHO, "d:/ymir work/ui/skill/shaman/boho_03.sub",),
				chr.AFFECT_KWAESOK : (localeInfo.SKILL_KWAESOK, "d:/ymir work/ui/skill/shaman/kwaesok_03.sub",),
				chr.AFFECT_HEUKSIN : (localeInfo.SKILL_HEUKSIN, "d:/ymir work/ui/skill/sura/heuksin_03.sub",),
				chr.AFFECT_MUYEONG : (localeInfo.SKILL_MUYEONG, "d:/ymir work/ui/skill/sura/muyeong_03.sub",),
				chr.AFFECT_GICHEON : (localeInfo.SKILL_GICHEON, "d:/ymir work/ui/skill/shaman/gicheon_03.sub",),
				chr.AFFECT_JEUNGRYEOK : (localeInfo.SKILL_JEUNGRYEOK, "d:/ymir work/ui/skill/shaman/jeungryeok_03.sub",),
				chr.AFFECT_PABEOP : (localeInfo.SKILL_PABEOP, "d:/ymir work/ui/skill/sura/pabeop_03.sub",),
				chr.AFFECT_FALLEN_CHEONGEUN : (localeInfo.SKILL_CHEONGEUN, "d:/ymir work/ui/skill/warrior/cheongeun_03.sub",),
				28 : (localeInfo.SKILL_FIRE, "d:/ymir work/ui/skill/sura/hwayeom_03.sub",),
				chr.AFFECT_CHINA_FIREWORK : (localeInfo.SKILL_POWERFUL_STRIKE, "d:/ymir work/ui/skill/common/affect/powerfulstrike.sub",),
				chr.AFFECT_RED_POSSESSION : (localeInfo.SKILL_GWIGEOM, "d:/ymir work/ui/skill/wolfman/red_possession_03.sub",),
				chr.AFFECT_BLUE_POSSESSION : (localeInfo.SKILL_CHEONGEUN, "d:/ymir work/ui/skill/wolfman/blue_possession_03.sub",),

				#64 - END
				chr.AFFECT_EXP_BONUS : (localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/exp_bonus.sub",),

				chr.AFFECT_ITEM_BONUS : (localeInfo.TOOLTIP_MALL_ITEMBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/item_bonus.sub",),
				chr.AFFECT_SAFEBOX : (localeInfo.TOOLTIP_MALL_SAFEBOX, "d:/ymir work/ui/skill/common/affect/safebox.sub",),
				chr.AFFECT_AUTOLOOT : (localeInfo.TOOLTIP_MALL_AUTOLOOT, "d:/ymir work/ui/skill/common/affect/autoloot.sub",),
				chr.AFFECT_FISH_MIND : (localeInfo.TOOLTIP_MALL_FISH_MIND, "d:/ymir work/ui/skill/common/affect/fishmind.sub",),
				chr.AFFECT_MARRIAGE_FAST : (localeInfo.TOOLTIP_MALL_MARRIAGE_FAST, "d:/ymir work/ui/skill/common/affect/marriage_fast.sub",),
				chr.AFFECT_GOLD_BONUS : (localeInfo.TOOLTIP_MALL_GOLDBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/gold_bonus.sub",),

				chr.AFFECT_NO_DEATH_PENALTY : (localeInfo.TOOLTIP_APPLY_NO_DEATH_PENALTY, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),
				chr.AFFECT_SKILL_BOOK_BONUS : (localeInfo.TOOLTIP_APPLY_SKILL_BOOK_BONUS, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),
				chr.AFFECT_SKILL_BOOK_NO_DELAY : (localeInfo.TOOLTIP_APPLY_SKILL_BOOK_NO_DELAY, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),

				# hp, sp
				chr.AFFECT_AUTO_HP_RECOVERY : (localeInfo.TOOLTIP_AUTO_POTION_REST, "d:/ymir work/ui/pattern/auto_hpgauge/05.dds"),
				chr.AFFECT_AUTO_SP_RECOVERY : (localeInfo.TOOLTIP_AUTO_POTION_REST, "d:/ymir work/ui/pattern/auto_spgauge/05.dds"),
				#chr.AFFECT_AUTO_HP_RECOVERY : (localeInfo.TOOLTIP_AUTO_POTION_REST, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),
				#chr.AFFECT_AUTO_SP_RECOVERY : (localeInfo.TOOLTIP_AUTO_POTION_REST, "d:/ymir work/ui/skill/common/affect/gold_bonus.sub"),

				MALL_DESC_IDX_START+player.POINT_MALL_ATTBONUS : (localeInfo.TOOLTIP_MALL_ATTBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/att_bonus.sub",),
				MALL_DESC_IDX_START+player.POINT_MALL_DEFBONUS : (localeInfo.TOOLTIP_MALL_DEFBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/def_bonus.sub",),
				MALL_DESC_IDX_START+player.POINT_MALL_EXPBONUS : (localeInfo.TOOLTIP_MALL_EXPBONUS, "d:/ymir work/ui/skill/common/affect/exp_bonus.sub",),
				MALL_DESC_IDX_START+player.POINT_MALL_ITEMBONUS : (localeInfo.TOOLTIP_MALL_ITEMBONUS, "d:/ymir work/ui/skill/common/affect/item_bonus.sub",),
				MALL_DESC_IDX_START+player.POINT_MALL_GOLDBONUS : (localeInfo.TOOLTIP_MALL_GOLDBONUS, "d:/ymir work/ui/skill/common/affect/gold_bonus.sub",),
				MALL_DESC_IDX_START+player.POINT_CRITICAL_PCT : (localeInfo.TOOLTIP_APPLY_CRITICAL_PCT,"d:/ymir work/ui/skill/common/affect/critical.sub"),
				MALL_DESC_IDX_START+player.POINT_PENETRATE_PCT : (localeInfo.TOOLTIP_APPLY_PENETRATE_PCT, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),
				MALL_DESC_IDX_START+player.POINT_MAX_HP_PCT : (localeInfo.TOOLTIP_MAX_HP_PCT, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),
				MALL_DESC_IDX_START+player.POINT_MAX_SP_PCT : (localeInfo.TOOLTIP_MAX_SP_PCT, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),	

				MALL_DESC_IDX_START+player.POINT_PC_BANG_EXP_BONUS : (localeInfo.TOOLTIP_MALL_EXPBONUS_P_STATIC, "d:/ymir work/ui/skill/common/affect/EXP_Bonus_p_on.sub",),
				MALL_DESC_IDX_START+player.POINT_PC_BANG_DROP_BONUS: (localeInfo.TOOLTIP_MALL_ITEMBONUS_P_STATIC, "d:/ymir work/ui/skill/common/affect/Item_Bonus_p_on.sub",),
				MALL_DESC_IDX_START+player.POINT_MELEE_MAGIC_ATT_BONUS_PER : (localeInfo.TOOLTIP_MALL_ATTBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/att_bonus.sub",),
		}
	else:
		AFFECT_DATA_DICT =	{
				chr.AFFECT_POISON : (localeInfo.SKILL_TOXICDIE, "d:/ymir work/ui/skill/common/affect/poison.sub"),
				chr.AFFECT_SLOW : (localeInfo.SKILL_SLOW, "d:/ymir work/ui/skill/common/affect/slow.sub"),
				chr.AFFECT_STUN : (localeInfo.SKILL_STUN, "d:/ymir work/ui/skill/common/affect/stun.sub"),

				chr.AFFECT_ATT_SPEED_POTION : (localeInfo.SKILL_INC_ATKSPD, "d:/ymir work/ui/skill/common/affect/Increase_Attack_Speed.sub"),
				chr.AFFECT_MOV_SPEED_POTION : (localeInfo.SKILL_INC_MOVSPD, "d:/ymir work/ui/skill/common/affect/Increase_Move_Speed.sub"),
				chr.AFFECT_FISH_MIND : (localeInfo.SKILL_FISHMIND, "d:/ymir work/ui/skill/common/affect/fishmind.sub"),

				chr.AFFECT_JEONGWI : (localeInfo.SKILL_JEONGWI, "d:/ymir work/ui/skill/warrior/jeongwi_03.sub",),
				chr.AFFECT_GEOMGYEONG : (localeInfo.SKILL_GEOMGYEONG, "d:/ymir work/ui/skill/warrior/geomgyeong_03.sub",),
				chr.AFFECT_CHEONGEUN : (localeInfo.SKILL_CHEONGEUN, "d:/ymir work/ui/skill/warrior/cheongeun_03.sub",),
				chr.AFFECT_GYEONGGONG : (localeInfo.SKILL_GYEONGGONG, "d:/ymir work/ui/skill/assassin/gyeonggong_03.sub",),
				chr.AFFECT_EUNHYEONG : (localeInfo.SKILL_EUNHYEONG, "d:/ymir work/ui/skill/assassin/eunhyeong_03.sub",),
				chr.AFFECT_GWIGEOM : (localeInfo.SKILL_GWIGEOM, "d:/ymir work/ui/skill/sura/gwigeom_03.sub",),
				chr.AFFECT_GONGPO : (localeInfo.SKILL_GONGPO, "d:/ymir work/ui/skill/sura/gongpo_03.sub",),
				chr.AFFECT_JUMAGAP : (localeInfo.SKILL_JUMAGAP, "d:/ymir work/ui/skill/sura/jumagap_03.sub"),
				chr.AFFECT_HOSIN : (localeInfo.SKILL_HOSIN, "d:/ymir work/ui/skill/shaman/hosin_03.sub",),
				chr.AFFECT_BOHO : (localeInfo.SKILL_BOHO, "d:/ymir work/ui/skill/shaman/boho_03.sub",),
				chr.AFFECT_KWAESOK : (localeInfo.SKILL_KWAESOK, "d:/ymir work/ui/skill/shaman/kwaesok_03.sub",),
				chr.AFFECT_HEUKSIN : (localeInfo.SKILL_HEUKSIN, "d:/ymir work/ui/skill/sura/heuksin_03.sub",),
				chr.AFFECT_MUYEONG : (localeInfo.SKILL_MUYEONG, "d:/ymir work/ui/skill/sura/muyeong_03.sub",),
				chr.AFFECT_GICHEON : (localeInfo.SKILL_GICHEON, "d:/ymir work/ui/skill/shaman/gicheon_03.sub",),
				chr.AFFECT_JEUNGRYEOK : (localeInfo.SKILL_JEUNGRYEOK, "d:/ymir work/ui/skill/shaman/jeungryeok_03.sub",),
				chr.AFFECT_PABEOP : (localeInfo.SKILL_PABEOP, "d:/ymir work/ui/skill/sura/pabeop_03.sub",),
				chr.AFFECT_FALLEN_CHEONGEUN : (localeInfo.SKILL_CHEONGEUN, "d:/ymir work/ui/skill/warrior/cheongeun_03.sub",),
				28 : (localeInfo.SKILL_FIRE, "d:/ymir work/ui/skill/sura/hwayeom_03.sub",),
				chr.AFFECT_CHINA_FIREWORK : (localeInfo.SKILL_POWERFUL_STRIKE, "d:/ymir work/ui/skill/common/affect/powerfulstrike.sub",),

				#64 - END
				chr.AFFECT_EXP_BONUS : (localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/exp_bonus.sub",),

				chr.AFFECT_ITEM_BONUS : (localeInfo.TOOLTIP_MALL_ITEMBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/item_bonus.sub",),
				chr.AFFECT_SAFEBOX : (localeInfo.TOOLTIP_MALL_SAFEBOX, "d:/ymir work/ui/skill/common/affect/safebox.sub",),
				chr.AFFECT_AUTOLOOT : (localeInfo.TOOLTIP_MALL_AUTOLOOT, "d:/ymir work/ui/skill/common/affect/autoloot.sub",),
				chr.AFFECT_FISH_MIND : (localeInfo.TOOLTIP_MALL_FISH_MIND, "d:/ymir work/ui/skill/common/affect/fishmind.sub",),
				chr.AFFECT_MARRIAGE_FAST : (localeInfo.TOOLTIP_MALL_MARRIAGE_FAST, "d:/ymir work/ui/skill/common/affect/marriage_fast.sub",),
				chr.AFFECT_GOLD_BONUS : (localeInfo.TOOLTIP_MALL_GOLDBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/gold_bonus.sub",),

				chr.AFFECT_NO_DEATH_PENALTY : (localeInfo.TOOLTIP_APPLY_NO_DEATH_PENALTY, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),
				chr.AFFECT_SKILL_BOOK_BONUS : (localeInfo.TOOLTIP_APPLY_SKILL_BOOK_BONUS, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),
				chr.AFFECT_SKILL_BOOK_NO_DELAY : (localeInfo.TOOLTIP_APPLY_SKILL_BOOK_NO_DELAY, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),

				# ÀÚµ¿¹°¾à hp, sp
				chr.AFFECT_AUTO_HP_RECOVERY : (localeInfo.TOOLTIP_AUTO_POTION_REST, "d:/ymir work/ui/pattern/auto_hpgauge/05.dds"),			
				chr.AFFECT_AUTO_SP_RECOVERY : (localeInfo.TOOLTIP_AUTO_POTION_REST, "d:/ymir work/ui/pattern/auto_spgauge/05.dds"),
				#chr.AFFECT_AUTO_HP_RECOVERY : (localeInfo.TOOLTIP_AUTO_POTION_REST, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),			
				#chr.AFFECT_AUTO_SP_RECOVERY : (localeInfo.TOOLTIP_AUTO_POTION_REST, "d:/ymir work/ui/skill/common/affect/gold_bonus.sub"),			

				MALL_DESC_IDX_START+player.POINT_MALL_ATTBONUS : (localeInfo.TOOLTIP_MALL_ATTBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/att_bonus.sub",),
				MALL_DESC_IDX_START+player.POINT_MALL_DEFBONUS : (localeInfo.TOOLTIP_MALL_DEFBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/def_bonus.sub",),
				MALL_DESC_IDX_START+player.POINT_MALL_EXPBONUS : (localeInfo.TOOLTIP_MALL_EXPBONUS, "d:/ymir work/ui/skill/common/affect/exp_bonus.sub",),
				MALL_DESC_IDX_START+player.POINT_MALL_ITEMBONUS : (localeInfo.TOOLTIP_MALL_ITEMBONUS, "d:/ymir work/ui/skill/common/affect/item_bonus.sub",),
				MALL_DESC_IDX_START+player.POINT_MALL_GOLDBONUS : (localeInfo.TOOLTIP_MALL_GOLDBONUS, "d:/ymir work/ui/skill/common/affect/gold_bonus.sub",),
				MALL_DESC_IDX_START+player.POINT_CRITICAL_PCT : (localeInfo.TOOLTIP_APPLY_CRITICAL_PCT,"d:/ymir work/ui/skill/common/affect/critical.sub"),
				MALL_DESC_IDX_START+player.POINT_PENETRATE_PCT : (localeInfo.TOOLTIP_APPLY_PENETRATE_PCT, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),
				MALL_DESC_IDX_START+player.POINT_MAX_HP_PCT : (localeInfo.TOOLTIP_MAX_HP_PCT, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),
				MALL_DESC_IDX_START+player.POINT_MAX_SP_PCT : (localeInfo.TOOLTIP_MAX_SP_PCT, "d:/ymir work/ui/skill/common/affect/gold_premium.sub"),	

				MALL_DESC_IDX_START+player.POINT_PC_BANG_EXP_BONUS : (localeInfo.TOOLTIP_MALL_EXPBONUS_P_STATIC, "d:/ymir work/ui/skill/common/affect/EXP_Bonus_p_on.sub",),
				MALL_DESC_IDX_START+player.POINT_PC_BANG_DROP_BONUS: (localeInfo.TOOLTIP_MALL_ITEMBONUS_P_STATIC, "d:/ymir work/ui/skill/common/affect/Item_Bonus_p_on.sub",),
				MALL_DESC_IDX_START+player.POINT_MELEE_MAGIC_ATT_BONUS_PER : (localeInfo.TOOLTIP_MALL_ATTBONUS_STATIC, "d:/ymir work/ui/skill/common/affect/att_bonus.sub",),
		}


		if app.ENABLE_DRAGON_SOUL_SYSTEM:
			AFFECT_DATA_DICT[chr.AFFECT_DRAGON_SOUL_DECK_0] = (localeInfo.TOOLTIP_DRAGON_SOUL_DECK1, "d:/ymir work/ui/dragonsoul/buff_ds_sky1.tga")
			AFFECT_DATA_DICT[chr.AFFECT_DRAGON_SOUL_DECK_1] = (localeInfo.TOOLTIP_DRAGON_SOUL_DECK2, "d:/ymir work/ui/dragonsoul/buff_ds_land1.tga")

	if app.ENABLE_AUTO_SYSTEM:
		AFFECT_DATA_DICT[chr.AFFECT_AUTO_USE] = (localeInfo.TOOLTIP_AUTO_SYSTEM_PRIMIUM, "d:/ymir work/ui/skill/common/affect/auto_premium.sub")

	if app.ENABLE_SET_ITEM:
		AFFECT_DATA_DICT[chr.AFFECT_SET_ITEM] = (localeInfo.TOOLTIP_SET_ITEM, "d:/ymir work/ui/skill/common/affect/set_bonus.sub")
		AFFECT_DATA_DICT[chr.AFFECT_SET_ITEM_SET_VALUE_1] = (localeInfo.TOOLTIP_AFFECT_SET_ITEM_1, "d:/ymir work/ui/skill/common/affect/set_bonus.sub")
		AFFECT_DATA_DICT[chr.AFFECT_SET_ITEM_SET_VALUE_2] = (localeInfo.TOOLTIP_AFFECT_SET_ITEM_2, "d:/ymir work/ui/skill/common/affect/set_bonus.sub")
		AFFECT_DATA_DICT[chr.AFFECT_SET_ITEM_SET_VALUE_3] = (localeInfo.TOOLTIP_AFFECT_SET_ITEM_3, "d:/ymir work/ui/skill/common/affect/set_bonus.sub")
		AFFECT_DATA_DICT[chr.AFFECT_SET_ITEM_SET_VALUE_4] = (localeInfo.TOOLTIP_AFFECT_SET_ITEM_4, "d:/ymir work/ui/skill/common/affect/set_bonus.sub")
		AFFECT_DATA_DICT[chr.AFFECT_SET_ITEM_SET_VALUE_5] = (localeInfo.TOOLTIP_AFFECT_SET_ITEM_5, "d:/ymir work/ui/skill/common/affect/set_bonus.sub")

	if app.ENABLE_MONSTER_CARD:
		AFFECT_DATA_DICT[chr.AFFECT_POLYMORPH] = (localeInfo.TOOLTIP_AFFECT_POLYMORPH, "d:/ymir work/ui/game/monster_card/poly_affect.sub")

	if app.ENABLE_ELEMENT_ADD:
		AFFECT_DATA_DICT[chr.AFFECT_MOUNT_FALL] = (localeInfo.TOOLTIP_AFFECT_MOUNT_FALL, "d:/ymir work/ui/skill/common/affect/mount_fall.sub")

	if app.ENABLE_DS_SET:
		AFFECT_DATA_DICT[chr.AFFECT_DS_SET] = (localeInfo.TOOLTIP_DS_SET, "d:/ymir work/ui/skill/common/affect/ds_set_bonus.sub")

	AFFECT_DATA_DICT[chr.AFFECT_RESEARCHER_ELIXIR] = (localeInfo.TOOLTIP_AFFECT_RESEARCHER_ELIXIR, "d:/ymir work/ui/skill/common/affect/researcher_elixir.sub")
	AFFECT_DATA_DICT[chr.AFFECT_SAFE_BOX_BUFF] = (localeInfo.TOOLTIP_AFFECT_SAFEBOX_BUFF_01, "d:/ymir work/ui/skill/common/affect/safebox_buff.sub")

	#if app.ENABLE_FLOWER_EVENT:
	#	AFFECT_DATA_DICT[chr.AFFECT_FLOWER_EVENT]	= ("",	"d:/ymir work/ui/skill/common/affect/flower_event.sub")

	if app.ENABLE_FISHING_GAME:
		AFFECT_DATA_DICT[chr.AFFECT_FISHING_GOLD_TUNA] = (localeInfo.TOOLTIP_AFFECT_FISHING_GOLD_TUNA, "d:/ymir work/ui/skill/common/affect/fishing_gold_tuna_buff.sub")
		AFFECT_DATA_DICT[chr.AFFECT_FISHING_MOVE_SPEED_DOWN] = (localeInfo.TOOLTIP_AFFECT_FISHING_MOVE_SPEED_DOWN, "d:/ymir work/ui/skill/common/affect/fishing_move_speed_down.sub")

	if app.ENABLE_LOOTING_SYSTEM and app.ENABLE_PREMIUM_LOOTING:
		AFFECT_DATA_DICT[chr.AFFECT_LOOTING_SYSTEM] = (localeInfo.TOOLTIP_AFFECT_LOOTING_SYSTEM, "d:/ymir work/ui/skill/common/affect/looting_system.sub")

	if app.ENABLE_CONQUEROR_LEVEL:
		AFFECT_DATA_DICT[chr.AFFECT_SKILL_9_CHEONUN] = (localeInfo.TOOLTIP_AFFECT_CHEONUN, "d:/ymir work/ui/skill/shaman/cheonun_03.sub")
		AFFECT_DATA_DICT[chr.AFFECT_SUNGMA_BONUS] = (localeInfo.TOOLTIP_SUNGMA_ATTR_PRIMIUM, "d:/ymir work/ui/skill/common/affect/sungma_premium_buff.sub")

	if app.ENABLE_YUTNORI_EVENT_FLAG_RENEWAL:
		AFFECT_DATA_DICT[chr.AFFECT_HALLOWEEN_EVENT] = (localeInfo.TOOLTIP_HALLOWEEN_EVENT_BUFF, "d:/ymir work/ui/skill/common/affect/halloween_bonus.sub")

	if app.ENABLE_SNOWFLAKE_STICK_EVENT:
		AFFECT_DATA_DICT[chr.AFFECT_SNOWFLAKE_STICK_EVENT_RANK_BUFF] = (localeInfo.TOOLTIP_AFFECT_SNOWFLAKE_STICK_EVENT_RANK_BUFF, "d:/ymir work/ui/skill/common/affect/snowflake_stick_event_rank_buff.sub")
		AFFECT_DATA_DICT[chr.AFFECT_SNOWFLAKE_STICK_EVENT_SNOWFLAKE_BUFF] = (localeInfo.TOOLTIP_AFFECT_SNOWFLAKE_STICK_EVENT_SNOWFLAKE_BUFF_1, "d:/ymir work/ui/skill/common/affect/snowflake_stick_event_snowflake_buff.sub")

	if app.ENABLE_TREASURE_HUNT:
		TREASURE_HUNT_REFINE_BUFF_IMAGE = "d:/ymir work/ui/skill/common/affect/treasure_hunt_refine_buff.sub"
		AFFECT_DATA_DICT[chr.AFFECT_INCREASE_REFINE_PCT] = (localeInfo.TOOLTIP_AFFECT_INCREASE_REFINE_PCT, TREASURE_HUNT_REFINE_BUFF_IMAGE)
		AFFECT_DATA_DICT[chr.AFFECT_REFINE_FREE_MATERIAL] = (localeInfo.TOOLTIP_AFFECT_REFINE_FREE_MATERIAL, TREASURE_HUNT_REFINE_BUFF_IMAGE)

	if app.ENABLE_ELEMENTAL_DUNGEON:
		AFFECT_DATA_DICT[chr.AFFECT_CURSE_OF_ELEMENTAL] = (localeInfo.TOOLTIP_CURSE_OF_ELEMENTAL, "d:/ymir work/ui/skill/common/affect/curse_of_elemental.sub")
		AFFECT_DATA_DICT[chr.AFFECT_PROTECTION_OF_ELEMENTAL] = (localeInfo.TOOLTIP_PROTECTION_OF_ELEMENTAL, "d:/ymir work/ui/skill/common/affect/protection_of_elemental.sub")
		AFFECT_DATA_DICT[chr.AFFECT_POTION_OF_ELEMENTAL] = (localeInfo.TOOLTIP_POTION_OF_ELEMENTAL, "d:/ymir work/ui/skill/common/affect/potion_of_elemental.sub")

	if app.ENABLE_GUILD_DRAGONLAIR_SYSTEM:
		AFFECT_DATA_DICT[chr.AFFECT_RED_DRAGONLAIR_BUFF] = (localeInfo.TOOLTIP_AFFECT_RED_DRAGONLAIR_BUFF, "d:/ymir work/ui/skill/common/affect/def_grade_down.sub")

	if app.ENABLE_SUMMER_EVENT_ROULETTE:
		AFFECT_DATA_DICT[chr.AFFECT_LATE_SUMMER_EVENT_BUFF] = (localeInfo.TOOLTIP_AFFECT_LATE_SUMMER_EVENT_BUFF, "d:/ymir work/ui/skill/common/affect/late_summber_buff_affect.sub")
		AFFECT_DATA_DICT[chr.AFFECT_LATE_SUMMER_EVENT_PRIMIUM_BUFF] = (localeInfo.TOOLTIP_AFFECT_LATE_SUMMER_EVENT_PRIMIUM_BUFF, "d:/ymir work/ui/skill/common/affect/late_summber_primium_buff_affect.sub")

	if app.ENABLE_DEFENSE_WAVE:
		AFFECT_DATA_DICT[chr.AFFECT_DEFENSEWAVE_LASER] = (localeInfo.TOOLTIP_AFFECT_DEFENSEWAVE_LASER, "d:/ymir work/ui/skill/common/affect/laser.sub")

	_OFFICIAL_AFFECT_ICON = "d:/ymir work/ui/skill/common/affect/gold_premium.sub"
	_OFFICIAL_DEBUFF_ICON = "d:/ymir work/ui/skill/common/affect/def_grade_down.sub"
	_OFFICIAL_OFFICIAL_AFFECT_ENTRIES = (
		(chr.AFFECT_PVP_EXPBONUS, localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_PVP_ENTER, localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_PREMIUM_PRIVATE_SHOP, localeInfo.TOOLTIP_AFFECT_PREMIUM_PRIVATE_SHOP, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_MALL_EXPBONUS, localeInfo.TOOLTIP_MALL_EXPBONUS, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_EXP_BONUS_EVENT, localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_ATT_SPEED_SLOW, localeInfo.TOOLTIP_AFFECT_ATT_GRADE_DOWN, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_ATT_GRADE_DOWN, localeInfo.TOOLTIP_AFFECT_ATT_GRADE_DOWN, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_DEF_GRADE_DOWN, localeInfo.TOOLTIP_AFFECT_DEF_GRADE_DOWN, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_CRITICAL_PCT_DOWN, localeInfo.TOOLTIP_AFFECT_CRITICAL_GRADE_DOWN, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_NO_RECOVERY, localeInfo.TOOLTIP_AFFECT_RCAST_SPEED, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_REDUCE_CAST_SPEED, localeInfo.TOOLTIP_AFFECT_RCAST_SPEED, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_CZ_UNLIMIT_ENTER, localeInfo.TOOLTIP_AFFECT_CZ_UNLIMIT_ENTER, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_NO_DEATH_PENALTY_PLUS, localeInfo.TOOLTIP_APPLY_NO_DEATH_PENALTY, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_MISTS_ISLAND_BUFF, localeInfo.TOOLTIP_AFFECT_MI_BUFF, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_SEASON_RANKING_BUFF, localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_SEASON_PVE_BUFF, localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_BATTLE_FIELD, localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_BATTLE_POTION, localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_BATTLE_ROYALE_SLOW, localeInfo.SKILL_SLOW, "d:/ymir work/ui/skill/common/affect/slow.sub"),
		(chr.AFFECT_BATTLE_ROYALE_MOVE_SPEED, localeInfo.SKILL_INC_MOVSPD, "d:/ymir work/ui/skill/common/affect/Increase_Move_Speed.sub"),
		(chr.AFFECT_BATTLE_ROYALE_INFINITE_STAMINA, localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_PASSIVE_JOB_DECK, localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_WORLD_BOSS_REWARD, localeInfo.TOOLTIP_AFFECT_WORLD_BOSS_REWARD, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_WORLD_BOSS_DAILY_REWARD, localeInfo.TOOLTIP_AFFECT_WORLD_BOSS_DAILY_REWARD, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_SUNGMAHEE_TOWER_BUFF, localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_SUNGMAHEE_TOWER_DEBUFF, localeInfo.TOOLTIP_AFFECT_SUNGMAHEE_TOWER_CURSE, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_SUNGMAHEE_TOWER_CURSE, localeInfo.TOOLTIP_AFFECT_SUNGMAHEE_TOWER_CURSE, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_GARRISON_BUFF, localeInfo.TOOLTIP_AFFECT_GARRISON_BUFF, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_GARRISON_DEBUFF, localeInfo.TOOLTIP_AFFECT_GARRISON_DEBUFF, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_OTHER_WORLD_DEAD_SOUL, localeInfo.TOOLTIP_AFFECT_OTHER_WORLD_DEAD_SOUL, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_OTHER_WORLD_GOBLIN_PROTECTION, localeInfo.TOOLTIP_AFFECT_OTHER_WORLD_GOBLIN_PROTECTION, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_OTHER_WORLD_YEOMWANG_CURSE, localeInfo.TOOLTIP_AFFECT_OTHER_WORLD_YEOMWANG_CURSE, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_OTHER_WORLD_NORTH_REAPER_ROOM_DEBUFF, localeInfo.TOOLTIP_AFFECT_OTHER_WORLD_YEOMWANG_CURSE, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_WHITE_DRAGON_CAVE_CURSE, localeInfo.TOOLTIP_AFFECT_SUNGMAHEE_TOWER_CURSE, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_SECRET_DUNGEON_BUFF_A, localeInfo.TOOLTIP_AFFECT_SECRET_DUNGEON_BUFF_A, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_SECRET_DUNGEON_BUFF_B, localeInfo.TOOLTIP_AFFECT_SECRET_DUNGEON_BUFF_B, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_SECRET_DUNGEON_BUFF_C, localeInfo.TOOLTIP_AFFECT_SECRET_DUNGEON_BUFF_C, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_SECRET_DUNGEON_BUFF_POINT_A, localeInfo.TOOLTIP_AFFECT_SECRET_DUNGEON_BUFF_POINT_A, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_SECRET_DUNGEON_BUFF_POINT_B, localeInfo.TOOLTIP_AFFECT_SECRET_DUNGEON_BUFF_POINT_B, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_SECRET_DUNGEON_BUFF_POINT_C, localeInfo.TOOLTIP_AFFECT_SECRET_DUNGEON_BUFF_POINT_C, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_FLOWER_OF_GALE, localeInfo.TOOLTIP_AFFECT_FLOWER_OF_DESTRUCTION, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_FLOWER_OF_DESTRUCTION, localeInfo.TOOLTIP_AFFECT_FLOWER_OF_DESTRUCTION, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_GUILD_BATTLE_PERSONAL_BUFF, localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_NO_FALLEN, localeInfo.TOOLTIP_AFFECT_NO_FALLEN, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_AIR_BORNE, localeInfo.TOOLTIP_AFFECT_AIR_BORNE, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_FREEZING_DEBUFF, localeInfo.TOOLTIP_AFFECT_FREEZING, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_GUILD_WHITE_DRAGON_ENHANCE_SCALES, localeInfo.TOOLTIP_AFFECT_GUILD_WHITE_DRAGON_ENHANCE_SCALES, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_POLYMORPH_DEBUFF, localeInfo.TOOLTIP_AFFECT_POLYMORPH_DEBUFF, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_GUILD_WHITE_DRAGON_BREATH_DEBUFF, localeInfo.TOOLTIP_AFFECT_GUILD_WHITE_DRAGON_BREATH_DEBUFF, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_GUILD_WHITE_DRAGON_RAGE_DEBUFF, localeInfo.TOOLTIP_AFFECT_GUILD_WHITE_DRAGON_LAIR_RAGE_DEBUFF, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_GUILD_WHITE_DRAGON_POTION_BUFF, localeInfo.TOOLTIP_AFFECT_GUILD_WHITE_DRAGON_LAIR_BUFF_POTION, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_PREMIUM_MEMBERSHIP, localeInfo.TOOLTIP_AFFECT_PREMIUM_MEMBERSHIP_BUFF, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_GUILD_SPECIAL_BUILDING_BUFF, localeInfo.TOOLTIP_AFFECT_GUILD_SPECIAL_BUILDING_BUFF, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_GREEDY_GATE_BUFF, localeInfo.TOOLTIP_AFFECT_GREEDY_GATE_BUFF, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_MIND_CONTROL, localeInfo.TOOLTIP_AFFECT_MIND_CONTROL, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_HEAVY_BLEEDING, localeInfo.TOOLTIP_AFFECT_HEAVY_BLEEDING, "d:/ymir work/ui/skill/common/affect/poison.sub"),
		(chr.AFFECT_CURSE_OF_DARKNESS, localeInfo.TOOLTIP_AFFECT_CURSE_OF_DARKNESS, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_CURSE_OF_DARKNESS_IBLIS, localeInfo.TOOLTIP_AFFECT_CURSE_OF_DARKNESS, _OFFICIAL_DEBUFF_ICON),
		(chr.AFFECT_INVINCIBILITY_DECCAN, localeInfo.TOOLTIP_AFFECT_INVINCIBILITY_DECCAN, _OFFICIAL_AFFECT_ICON),
		(chr.AFFECT_SKILL_MOUNT_UPGRADE_NIMBLE, localeInfo.TOOLTIP_AFFECT_MOUNT_UPGRADE_SKILL_NIMBLE, "d:/ymir work/ui/skill/common/mount_upgrade/mount_upgrade_skill_nimble.sub"),
		(chr.AFFECT_SKILL_MOUNT_UPGRADE_EXP, localeInfo.TOOLTIP_AFFECT_MOUNT_UPGRADE_SKILL_EXP, "d:/ymir work/ui/skill/common/mount_upgrade/mount_upgrade_skill_exp.sub"),
		(chr.AFFECT_SKILL_MOUNT_UPGRADE_SPEED, localeInfo.TOOLTIP_AFFECT_MOUNT_UPGRADE_SKILL_ATTACK_SPEED, "d:/ymir work/ui/skill/common/mount_upgrade/mount_upgrade_skill_speed.sub"),
		(chr.AFFECT_SKILL_MOUNT_UPGRADE_GYENONGGONG, localeInfo.TOOLTIP_AFFECT_MOUNT_UPGRADE_SKILL_GYEONGGONG, "d:/ymir work/ui/skill/common/mount_upgrade/mount_upgrade_skill_gyeonggong.sub"),
		(chr.AFFECT_SKILL_MOUNT_UPGRADE_INVINCIBLITIY, localeInfo.TOOLTIP_AFFECT_MOUNT_UPGRADE_SKILL_INVINCIBILITY, "d:/ymir work/ui/skill/common/mount_upgrade/mount_upgrade_skill_invisibility.sub"),
		(chr.AFFECT_SKILL_MOUNT_UPGRADE_KNOCKBACK, localeInfo.TOOLTIP_AFFECT_MOUNT_UPGRADE_SKILL_KNOCKBACK, "d:/ymir work/ui/skill/common/mount_upgrade/mount_upgrade_skill_knockback.sub"),
		(chr.AFFECT_SKILL_MOUNT_UPGRADE_TO_STRONG, localeInfo.TOOLTIP_AFFECT_MOUNT_UPGRADE_SKILL_TO_STRONG, "d:/ymir work/ui/skill/common/mount_upgrade/mount_upgrade_skill_random_strong.sub"),
		(chr.AFFECT_SKILL_MOUNT_UPGRADE_EFFECT_UP, localeInfo.TOOLTIP_AFFECT_MOUNT_UPGRADE_SKILL_EFFECT_UP, "d:/ymir work/ui/skill/common/mount_upgrade/mount_upgrade_skill_effect_up.sub"),
		(chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_STR, localeInfo.TOOLTIP_AFFECT_MOUNT_UPGRADE_SKILL_SUNGMA_STR, "d:/ymir work/ui/skill/common/mount_upgrade/mount_upgrade_skill_sungma_str.sub"),
		(chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_HP, localeInfo.TOOLTIP_AFFECT_MOUNT_UPGRADE_SKILL_SUNGMA_HP, "d:/ymir work/ui/skill/common/mount_upgrade/mount_upgrade_skill_sungma_hp.sub"),
		(chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_MOVE, localeInfo.TOOLTIP_AFFECT_MOUNT_UPGRADE_SKILL_SUNGMA_MOVE, "d:/ymir work/ui/skill/common/mount_upgrade/mount_upgrade_skill_sungma_move.sub"),
		(chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_IMMUNE, localeInfo.TOOLTIP_AFFECT_MOUNT_UPGRADE_SKILL_SUNGMA_IMMUNE, "d:/ymir work/ui/skill/common/mount_upgrade/mount_upgrade_skill_sungma_immune.sub"),
		(chr.AFFECT_SKILL_MOUNT_UPGRADE_HIT_PCT, localeInfo.TOOLTIP_AFFECT_MOUNT_UPGRADE_SKILL_HIT_PCT, "d:/ymir work/ui/skill/common/mount_upgrade/mount_upgrade_skill_hit_pct.sub"),
		(chr.AFFECT_MERCENARY_MISSION_MAX_ACTIVE_INCREASE, localeInfo.TOOLTIP_MALL_EXPBONUS_STATIC, _OFFICIAL_AFFECT_ICON),
	)
	def __IsOfficialAffectEnabled(_affectType):
		if _affectType in (chr.AFFECT_RED_DRAGONLAIR_STONE, chr.AFFECT_RED_DRAGONLAIR_BUFF,
			chr.AFFECT_WHITE_DRAGON_CAVE_CURSE, chr.AFFECT_FREEZING_DEBUFF,
			chr.AFFECT_GUILD_WHITE_DRAGON_ENHANCE_SCALES, chr.AFFECT_GUILD_WHITE_DRAGON_BREATH_DEBUFF,
			chr.AFFECT_GUILD_WHITE_DRAGON_RAGE_DEBUFF, chr.AFFECT_GUILD_WHITE_DRAGON_POTION_BUFF):
			return app.ENABLE_GUILD_DRAGONLAIR_SYSTEM
		if _affectType in (chr.AFFECT_LATE_SUMMER_EVENT_BUFF, chr.AFFECT_LATE_SUMMER_EVENT_PRIMIUM_BUFF):
			return app.ENABLE_SUMMER_EVENT_ROULETTE
		if _affectType == chr.AFFECT_PASSIVE_JOB_DECK:
			return app.ENABLE_PASSIVE_SYSTEM
		if _affectType in (chr.AFFECT_SUNGMAHEE_TOWER_BUFF, chr.AFFECT_SUNGMAHEE_TOWER_DEBUFF,
			chr.AFFECT_SUNGMAHEE_TOWER_CURSE, chr.AFFECT_GARRISON_BUFF, chr.AFFECT_GARRISON_DEBUFF,
			chr.AFFECT_GREEDY_GATE_BUFF):
			return app.ENABLE_SUNGMAHEE_GATE
		if _affectType in (chr.AFFECT_FLOWER_OF_GALE, chr.AFFECT_FLOWER_OF_DESTRUCTION):
			return app.ENABLE_FLOWER_EVENT
		if _affectType in (chr.AFFECT_INCREASE_REFINE_PCT, chr.AFFECT_REFINE_FREE_MATERIAL):
			return app.ENABLE_TREASURE_HUNT
		if _affectType in (chr.AFFECT_SKILL_MOUNT_UPGRADE_NIMBLE, chr.AFFECT_SKILL_MOUNT_UPGRADE_EXP,
			chr.AFFECT_SKILL_MOUNT_UPGRADE_SPEED, chr.AFFECT_SKILL_MOUNT_UPGRADE_GYENONGGONG,
			chr.AFFECT_SKILL_MOUNT_UPGRADE_INVINCIBLITIY, chr.AFFECT_SKILL_MOUNT_UPGRADE_KNOCKBACK,
			chr.AFFECT_SKILL_MOUNT_UPGRADE_TO_STRONG, chr.AFFECT_SKILL_MOUNT_UPGRADE_EFFECT_UP,
			chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_STR, chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_HP,
			chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_MOVE, chr.AFFECT_SKILL_MOUNT_UPGRADE_SUNGMA_IMMUNE,
			chr.AFFECT_SKILL_MOUNT_UPGRADE_HIT_PCT):
			return app.ENABLE_MOUNT_UPGRADE_SYSTEM
		if _affectType in (chr.AFFECT_EXP_BONUS_EVENT, chr.AFFECT_ATT_SPEED_SLOW, chr.AFFECT_PEPSI_EVENT,
			chr.AFFECT_SUMMER_EVENT, chr.AFFECT_BATTLE_FIELD, chr.AFFECT_BATTLE_POTION,
			chr.AFFECT_LUCKEY_EVENT_BUFF, chr.AFFECT_NO_RECOVERY, chr.AFFECT_REDUCE_CAST_SPEED,
			chr.AFFECT_ATT_GRADE_DOWN, chr.AFFECT_DEF_GRADE_DOWN, chr.AFFECT_CRITICAL_PCT_DOWN,
			chr.AFFECT_CZ_UNLIMIT_ENTER, chr.AFFECT_NO_DEATH_PENALTY_PLUS, chr.AFFECT_MALL_EXPBONUS,
			chr.AFFECT_MISTS_ISLAND_BUFF, chr.AFFECT_SEASON_RANKING_BUFF, chr.AFFECT_SEASON_PVE_BUFF,
			chr.AFFECT_BATTLE_ROYALE_SLOW, chr.AFFECT_BATTLE_ROYALE_MOVE_SPEED,
			chr.AFFECT_BATTLE_ROYALE_INFINITE_STAMINA, chr.AFFECT_WORLD_BOSS_REWARD,
			chr.AFFECT_WORLD_BOSS_DAILY_REWARD):
			return app.ENABLE_INGAME_EVENT_MANAGER
		if _affectType in (chr.AFFECT_SECRET_DUNGEON_BUFF_POINT_C, chr.AFFECT_SECRET_DUNGEON_BUFF_C,
			chr.AFFECT_SECRET_DUNGEON_BUFF_B, chr.AFFECT_SECRET_DUNGEON_BUFF_A,
			chr.AFFECT_SECRET_DUNGEON_BUFF_POINT_B, chr.AFFECT_SECRET_DUNGEON_BUFF_POINT_A):
			return app.ENABLE_LABYRINTH_DUNGEON
		if _affectType in (chr.AFFECT_OTHER_WORLD_DEAD_SOUL, chr.AFFECT_OTHER_WORLD_GOBLIN_PROTECTION,
			chr.AFFECT_OTHER_WORLD_YEOMWANG_CURSE, chr.AFFECT_OTHER_WORLD_NORTH_REAPER_ROOM_DEBUFF,
			chr.AFFECT_MIND_CONTROL, chr.AFFECT_HEAVY_BLEEDING, chr.AFFECT_CURSE_OF_DARKNESS,
			chr.AFFECT_INVINCIBILITY_DECCAN, chr.AFFECT_CURSE_OF_DARKNESS_IBLIS):
			return app.ENABLE_DAWNMIST_DUNGEON
		return True

	for _affectType, _tooltip, _icon in _OFFICIAL_OFFICIAL_AFFECT_ENTRIES:
		if not __IsOfficialAffectEnabled(_affectType):
			continue
		if _affectType not in AFFECT_DATA_DICT:
			AFFECT_DATA_DICT[_affectType] = (_tooltip, _icon)

	def __init__(self):
		ui.Window.__init__(self)

		self.serverPlayTime = 0
		self.clientPlayTime = 0

		self.lastUpdateTime = 0
		self.affectImageDict = {}
		self.horseImage = None
		self.lovePointImage = None
		if app.ENABLE_CONQUEROR_LEVEL:
			self.sungmaImage = None
		self.autoPotionImageHP = AutoPotionImage()
		self.autoPotionImageSP = AutoPotionImage()
		self.SetPosition(10, 10)
		self.Show()

		if app.ENABLE_GROWTH_PET_SYSTEM:
			self.petSkillaffectImageDict = {}

		if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
			self.__mount_upgrade_passive_skill = {}

		if app.ENABLE_SET_ITEM:
			self.SetItemAffectCheck()
		
	def __del__(self):
		ui.Window.__del__(self)

	if app.ENABLE_SET_ITEM:
		def SetItemAffectCheck(self):
			#set affect off
			if player.EmptySetItemEffect(chr.AFFECT_SET_ITEM):
				self.BINARY_NEW_RemoveAffect(chr.AFFECT_SET_ITEM, 0)
			#set affect on
			else:
				self.BINARY_NEW_RemoveAffect(chr.AFFECT_SET_ITEM, 0)
				self.BINARY_NEW_AddAffect(chr.AFFECT_SET_ITEM, 0, 0, self.INFINITE_AFFECT_DURATION)

			for affect in xrange(chr.AFFECT_SET_ITEM_SET_VALUE_1, chr.AFFECT_SET_ITEM_SET_VALUE_5 + 1):
				if player.EmptySetItemEffect(affect):
					self.BINARY_NEW_RemoveAffect(affect, 0)
				else:
					self.BINARY_NEW_RemoveAffect(affect, 0)
					self.BINARY_NEW_AddAffect(affect, 0, 0, self.INFINITE_AFFECT_DURATION)

	def ClearAllAffects(self):
		self.horseImage = None
		self.lovePointImage = None
		if app.ENABLE_CONQUEROR_LEVEL:
			self.sungmaImage = None
		self.affectImageDict = {}

		if app.ENABLE_GROWTH_PET_SYSTEM:
			self.petSkillaffectImageDict = {}

		if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
			self.__mount_upgrade_passive_skill = {}

		self.__ArrangeImageList()

	def ClearAffects(self):
		self.living_affectImageDict = {}
		for key, image in self.affectImageDict.items():
			if not image.IsSkillAffect():
				self.living_affectImageDict[key] = image
		self.affectImageDict = self.living_affectImageDict
		self.__ArrangeImageList()

	def BINARY_NEW_AddAffect(self, type, pointIdx, value, duration):

		print "BINARY_NEW_AddAffect", type, pointIdx, value, duration

		# if type < 500:
			# return

		if type < 500:
			pass_affect_list = []
			pass_affect_list.append(chr.AFFECT_POLYMORPH)
			if app.ENABLE_CONQUEROR_LEVEL:
				pass_affect_list.append(chr.AFFECT_SKILL_9_CHEONUN)

			if not type in pass_affect_list:
				return

		if type == chr.AFFECT_MALL:
			affect = self.MALL_DESC_IDX_START + pointIdx
		else:
			affect = type

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

		if not self.AFFECT_DATA_DICT.has_key(affect):
			return

		if app.ENABLE_AUTO_SYSTEM:
			if type == chr.AFFECT_AUTO_USE:
				import chrmgr
				if not chrmgr.GetAutoOnOff():
					return

		## The blessing of the dragon god, the lesson of the good man sets the Duration to 0.
		if affect == chr.AFFECT_NO_DEATH_PENALTY or\
		   affect == chr.AFFECT_SKILL_BOOK_BONUS or\
		   affect == chr.AFFECT_AUTO_SP_RECOVERY or\
		   affect == chr.AFFECT_AUTO_HP_RECOVERY or\
		   affect == chr.AFFECT_SKILL_BOOK_NO_DELAY:
			duration = 0

		affectData = self.AFFECT_DATA_DICT[affect]
		description = affectData[0]
		filename = affectData[1]

		if pointIdx == player.POINT_MALL_ITEMBONUS or\
		   pointIdx == player.POINT_MALL_GOLDBONUS:
			value = 1 + float(value) / 100.0

		if affect == chr.AFFECT_SAFE_BOX_BUFF:
			if pointIdx == player.POINT_MALL_EXPBONUS:
				description = localeInfo.TOOLTIP_AFFECT_SAFEBOX_BUFF_01

			elif pointIdx == player.POINT_MALL_DEFBONUS:
				description = localeInfo.TOOLTIP_AFFECT_SAFEBOX_BUFF_02

			elif pointIdx == player.POINT_MAX_HP_PCT:
				description = localeInfo.TOOLTIP_AFFECT_SAFEBOX_BUFF_03

			elif pointIdx == player.POINT_ATTBONUS_MONSTER:
				description = localeInfo.TOOLTIP_AFFECT_SAFEBOX_BUFF_04

			else:
				description = affectData[0]

		if affect != chr.AFFECT_AUTO_SP_RECOVERY and affect != chr.AFFECT_AUTO_HP_RECOVERY:
			description = description(float(value))
		
		# if affect != chr.AFFECT_AUTO_SP_RECOVERY and affect != chr.AFFECT_AUTO_HP_RECOVERY:
			# if app.ENABLE_FLOWER_EVENT:
				# if affect == chr.AFFECT_FLOWER_EVENT:
					# description = uiToolTip.ItemToolTip().GetAffectString(item.GetPointApply(pointIdx), float(value))
					# toolTipString.append(self.itemToolTip.GetAffectString(type, value))
				# elif value != 0:
					# description = description(float(value))
			# else:
				# description = description(float(value))

		try:
			print "Add affect %s" % affect
			image = AffectImage()
			image.SetParent(self)
			image.LoadImage(filename)
			image.SetDescription(description)
			image.SetDuration(duration)
			image.SetAffect(affect)
			if app.ENABLE_SET_ITEM and affect == chr.AFFECT_SET_ITEM or affect >= chr.AFFECT_SET_ITEM_SET_VALUE_1 and affect <= chr.AFFECT_SET_ITEM_SET_VALUE_5:
				image.SetClock(False)
				image.UpdateSetItemDescription(affect)
			elif app.ENABLE_SNOWFLAKE_STICK_EVENT and affect == chr.AFFECT_SNOWFLAKE_STICK_EVENT_SNOWFLAKE_BUFF:
				image.SetClock(False)
				image.UpdateSnowflakeStickEventSnowflakeBuffDescription()
			elif app.ENABLE_TREASURE_HUNT and affect in (chr.AFFECT_INCREASE_REFINE_PCT, chr.AFFECT_REFINE_FREE_MATERIAL):
				image.SetClock(True)
				image.UpdateTreasureHuntRefineBuffDescription()
			elif app.ENABLE_MOUNT_UPGRADE_SYSTEM and self.__IsMountUpgradePassiveAffect(affect):
				image.SetClock(False)
				if pointIdx:
					image.AddMultiLineMountUpgradeDescription(pointIdx, description)
				image.UpdateDescriptionMountUpgradeSkill()

			elif affect == chr.AFFECT_EXP_BONUS_EURO_FREE or\
				affect == chr.AFFECT_EXP_BONUS_EURO_FREE_UNDER_15 or\
				self.INFINITE_AFFECT_DURATION < duration:
				image.SetClock(False)
				image.UpdateDescription()
			elif affect == chr.AFFECT_AUTO_SP_RECOVERY or affect == chr.AFFECT_AUTO_HP_RECOVERY:
				image.UpdateAutoPotionDescription()
			else:
				image.UpdateDescription()

			if affect == chr.AFFECT_DRAGON_SOUL_DECK_0 or affect == chr.AFFECT_DRAGON_SOUL_DECK_1:
				image.SetScale(1, 1)
			else:
				image.SetScale(0.7, 0.7)
			image.SetSkillAffectFlag(False)
			image.Show()
			self.affectImageDict[affect] = image
			self.__ArrangeImageList()
		except Exception, e:
			print "except Aff affect ", e
			pass

	def BINARY_NEW_UpdateAffect(self, type, point_type, value, duration):
		if type == chr.AFFECT_MALL:
			affect = self.MALL_DESC_IDX_START + point_type
		else:
			affect = type

		if affect in self.affectImageDict:
			image = self.affectImageDict[affect]
			image.SetDuration(duration)

			# Eğer AffectImage sınıfında value tutuyorsa
			if hasattr(image, "UpdateValue"):
				image.UpdateValue(value)

			image.UpdateDescription()
			image.Show()  # Görünümü güncelle
			self.__ArrangeImageList()
		else:
			self.BINARY_NEW_AddAffect(type, point_type, value, duration)

	def BINARY_NEW_RemoveAffect(self, type, pointIdx):
		if type == chr.AFFECT_MALL:
			affect = self.MALL_DESC_IDX_START + pointIdx
		else:
			affect = type

		#print "Remove Affect %s %s" % ( type , pointIdx )
		if app.ENABLE_MOUNT_UPGRADE_SYSTEM and self.__IsMountUpgradePassiveAffect(affect):
			self.BINARY_NEW_Remove_MountUpgrade_Passive_Skill_Affect(affect, pointIdx)
			return

		self.__RemoveAffect(affect)
		self.__ArrangeImageList()

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

	def SetAffect(self, affect):
		self.__AppendAffect(affect)
		self.__ArrangeImageList()

	def ResetAffect(self, affect):
		self.__RemoveAffect(affect)
		self.__ArrangeImageList()

	def SetLoverInfo(self, name, lovePoint):
		image = LovePointImage()
		image.SetParent(self)
		image.SetLoverInfo(name, lovePoint)
		self.lovePointImage = image
		self.__ArrangeImageList()

	def ShowLoverState(self):
		if self.lovePointImage:
			self.lovePointImage.Show()
			self.__ArrangeImageList()

	def HideLoverState(self):
		if self.lovePointImage:
			self.lovePointImage.Hide()
			self.__ArrangeImageList()

	def ClearLoverState(self):
		self.lovePointImage = None
		self.__ArrangeImageList()

	def OnUpdateLovePoint(self, lovePoint):
		if self.lovePointImage:
			self.lovePointImage.OnUpdateLovePoint(lovePoint)

	def SetHorseState(self, level, health, battery):
		if level==0:
			self.horseImage=None
		else:
			image = HorseImage()
			image.SetParent(self)
			image.SetState(level, health, battery)
			image.Show()

			self.horseImage=image
			self.__ArrangeImageList()

	def SetPlayTime(self, playTime):
		self.serverPlayTime = playTime
		self.clientPlayTime = app.GetTime()

		if localeInfo.IsVIETNAM():
			image = PlayTimeImage()
			image.SetParent(self)
			image.SetPlayTime(playTime)
			image.Show()

			self.playTimeImage=image
			self.__ArrangeImageList()

	if app.ENABLE_CONQUEROR_LEVEL:
		def SetSungMaAffectImage(self, str, hp, move, immune):
			image = SungMaAffectImage(str, hp, move, immune)
			image.SetParent(self)
			image.SetSungmaValue(str, hp, move, immune)
			image.Show()

			self.sungmaImage = image
			self.__ArrangeImageList()

	def __AppendAffect(self, affect):

		if self.affectImageDict.has_key(affect):
			return

		try:
			affectData = self.AFFECT_DATA_DICT[affect]
		except KeyError:
			return

		name = affectData[0]
		filename = affectData[1]

		skillIndex = player.AffectIndexToSkillIndex(affect)
		if 0 != skillIndex:
			name = skill.GetSkillName(skillIndex)

		image = AffectImage()
		image.SetParent(self)
		image.SetSkillAffectFlag(True)

		try:
			image.LoadImage(filename)
		except:
			pass

		image.SetToolTipText(name, AFFECT_TOOLTIP_POS_X, AFFECT_TOOLTIP_POS_Y)
		image.SetScale(0.7, 0.7)
		image.Show()
		self.affectImageDict[affect] = image

	def __RemoveAffect(self, affect):
		"""
		if affect == chr.AFFECT_AUTO_SP_RECOVERY:
			self.autoPotionImageSP.Hide()

		if affect == chr.AFFECT_AUTO_HP_RECOVERY:
			self.autoPotionImageHP.Hide()
		"""

		if not self.affectImageDict.has_key(affect):
			#print "__RemoveAffect %s ( No Affect )" % affect
			return

		#print "__RemoveAffect %s ( Affect )" % affect
		del self.affectImageDict[affect]

		self.__ArrangeImageList()

	def __ArrangeImageList(self):

		width = len(self.affectImageDict) * self.IMAGE_STEP
		if self.lovePointImage:
			width+=self.IMAGE_STEP
		if self.horseImage:
			width+=self.IMAGE_STEP
		if app.ENABLE_CONQUEROR_LEVEL:
			if self.sungmaImage:
				width += self.IMAGE_STEP

		if app.ENABLE_GROWTH_PET_SYSTEM:
			width += len(self.petSkillaffectImageDict) * self.IMAGE_STEP

		self.SetSize(width, 26)

		xPos = 0

		if app.ENABLE_GROWTH_PET_SYSTEM:
			tempDict = {}
			for value in self.petSkillaffectImageDict.values():
				tempDict[value[0]] = value[1]

			for value in range(1,4):
				if tempDict.has_key(value):
					tempDict[value].SetPosition(xPos, 0)
					xPos += self.IMAGE_STEP

		if self.lovePointImage:
			if self.lovePointImage.IsShow():
				self.lovePointImage.SetPosition(xPos, 0)
				xPos += self.IMAGE_STEP

		if self.horseImage:
			self.horseImage.SetPosition(xPos, 0)
			xPos += self.IMAGE_STEP

		if app.ENABLE_CONQUEROR_LEVEL:
			if self.sungmaImage:
				self.sungmaImage.SetPosition(xPos, 0)
				xPos += self.IMAGE_STEP

		for image in self.affectImageDict.values():
			image.SetPosition(xPos, 0)
			xPos += self.IMAGE_STEP

	def OnUpdate(self):
		try:
			if app.GetGlobalTimeStamp() > self.lastUpdateTime:
			#if app.GetGlobalTime() - self.lastUpdateTime > 500:
			#if 0 < app.GetGlobalTime():
				#self.lastUpdateTime = app.GetGlobalTime()
				self.lastUpdateTime = app.GetGlobalTimeStamp()

				for image in self.affectImageDict.values():
					if image.GetAffect() == chr.AFFECT_AUTO_HP_RECOVERY or image.GetAffect() == chr.AFFECT_AUTO_SP_RECOVERY:
						image.UpdateAutoPotionDescription()
						continue

					if app.ENABLE_SET_ITEM:
						if image.GetAffect() == chr.AFFECT_SET_ITEM or image.GetAffect() >= chr.AFFECT_SET_ITEM_SET_VALUE_1 and image.GetAffect() <= chr.AFFECT_SET_ITEM_SET_VALUE_5:
							continue

					if app.ENABLE_SNOWFLAKE_STICK_EVENT:
						if image.GetAffect() == chr.AFFECT_SNOWFLAKE_STICK_EVENT_SNOWFLAKE_BUFF:
							image.UpdateSnowflakeStickEventSnowflakeBuffDescription(True)
							continue

					if app.ENABLE_TREASURE_HUNT:
						if image.GetAffect() in (chr.AFFECT_INCREASE_REFINE_PCT, chr.AFFECT_REFINE_FREE_MATERIAL):
							image.UpdateTreasureHuntRefineBuffDescription()
							continue

					if not image.IsSkillAffect():
						image.UpdateDescription()
		except Exception, e:
			print "AffectShower::OnUpdate error : ", e

	if app.ENABLE_GROWTH_PET_SYSTEM:
		def SetPetSkillAffect(self, index, affect):

			if self.__AppendPetSkillAffect(index, affect):
				self.__ArrangeImageList()

	if app.ENABLE_GROWTH_PET_SYSTEM:
		def ClearPetSkillAffect(self):
			self.petSkillaffectImageDict.clear()
			self.__ArrangeImageList()

	if app.ENABLE_GROWTH_PET_SYSTEM:
		def __AppendPetSkillAffect(self, index, affect):

			if self.petSkillaffectImageDict.has_key(affect):
				return False

			filename = skill.GetPetSkillIconPath(affect)
			if "" == filename:
				return False

			( pet_skill_name, pet_skill_desc, pet_skill_use_type, pet_skill_cool_time ) = skill.GetPetSkillInfo(affect)

			image = GrowthPetImage()
			image.SetParent(self)

			try:
				image.LoadImage(filename)
			except:
				print "LoadImage Except nn"
				return False

			image.SetToolTipText(pet_skill_name, 0, 40)

			image.SetDescription(pet_skill_desc)

			image.SetScale(0.7, 0.7)

			image.Show()

			self.petSkillaffectImageDict[affect] = [index, image]

			return True
