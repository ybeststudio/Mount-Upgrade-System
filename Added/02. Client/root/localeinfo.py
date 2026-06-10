import app
import constInfo
import background

MAP_TRENT02 = "MAP_TRENT02"
MAP_WL = "MAP_WL"
MAP_NUSLUCK = "MAP_NUSLUCK"
MAP_TREE2 = "MAP_TREE2"

BLEND_POTION_NO_TIME = "BLEND_POTION_NO_TIME"
BLEND_POTION_NO_INFO = "BLEND_POTION_NO_INFO"

APP_TITLE = "Mythos2 # Rise Beyond Myth! | www.beststudio.dev"

GUILD_HEADQUARTER = "Main Building"
GUILD_FACILITY = "Facility"
GUILD_OBJECT = "Object"
GUILD_MEMBER_COUNT_INFINITY = "INFINITY"

LOGIN_FAILURE_WEB_BLOCK = "BLOCK_LOGIN(WEB)"
LOGIN_FAILURE_BLOCK_LOGIN = "BLOCK_LOGIN"
CHANNEL_NOTIFY_FULL = "CHANNEL_NOTIFY_FULL"

GUILD_BUILDING_LIST_TXT = app.GetLocalePath() + "/GuildBuildingList.txt"

GUILD_MARK_MIN_LEVEL = "3"
GUILD_MARK_NOT_ENOUGH_LEVEL = "Available from Guild Level 3 or higher."

ERROR_MARK_UPLOAD_NEED_RECONNECT = "UploadMark: Reconnect to game"
ERROR_MARK_CHECK_NEED_RECONNECT = "CheckMark: Reconnect to game"

VIRTUAL_KEY_ALPHABET_LOWERS  = r"[1234567890]/qwertyuiop\=asdfghjkl;`'zxcvbnm.,"
VIRTUAL_KEY_ALPHABET_UPPERS  = r'{1234567890}?QWERTYUIOP|+ASDFGHJKL:~"ZXCVBNM<>'
VIRTUAL_KEY_SYMBOLS    = '!@#$%^&*()_+|{}:"<>?~'
VIRTUAL_KEY_NUMBERS    = "1234567890-=\[];',./`"
VIRTUAL_KEY_SYMBOLS_BR    = '!@#$%^&*()_+|{}:"<>?~áàãâéèêíìóòôõúùç'

__IS_ENGLISH	= "ENGLISH" == app.GetLocaleServiceName()
__IS_HONGKONG	= "HONGKONG" == app.GetLocaleServiceName()
__IS_NEWCIBN	= "locale/newcibn" == app.GetLocalePath()
__IS_EUROPE		= "EUROPE" == app.GetLocaleServiceName()
__IS_CANADA		= "locale/ca" == app.GetLocalePath()
__IS_BRAZIL		= "locale/br" == app.GetLocalePath()
__IS_SINGAPORE	= "locale/sg" == app.GetLocalePath()
__IS_VIETNAM	= "locale/vn" == app.GetLocalePath()
__IS_ARABIC		= "locale/ae" == app.GetLocalePath()
__IS_CIBN10		= "locale/cibn10" == app.GetLocalePath()
__IS_WE_KOREA	= "locale/we_korea" == app.GetLocalePath()
__IS_TAIWAN		= "locale/taiwan" == app.GetLocalePath()
__IS_JAPAN		= "locale/japan" == app.GetLocalePath()
LOGIN_FAILURE_WRONG_SOCIALID = "ASDF"
LOGIN_FAILURE_SHUTDOWN_TIME = "ASDF"

if __IS_CANADA:
	__IS_EUROPE = True

def IsYMIR():
	return "locale/ymir" == app.GetLocalePath()

def IsJAPAN():
	return "locale/japan" == app.GetLocalePath()

def IsENGLISH():
	global __IS_ENGLISH
	return __IS_ENGLISH

def IsHONGKONG():
	global __IS_HONGKONG
	return __IS_HONGKONG

def IsTAIWAN():
	return "locale/taiwan" == app.GetLocalePath()

def IsNEWCIBN():
	return "locale/newcibn" == app.GetLocalePath()

def IsCIBN10():
	global __IS_CIBN10
	return __IS_CIBN10

def IsEUROPE():
	global __IS_EUROPE
	return __IS_EUROPE

def IsCANADA():
	global __IS_CANADA
	return __IS_CANADA

def IsBRAZIL():
	global __IS_BRAZIL
	return __IS_BRAZIL

def IsVIETNAM():
	global __IS_VIETNAM
	return __IS_VIETNAM

def IsSINGAPORE():
	global __IS_SINGAPORE
	return __IS_SINGAPORE

def IsARABIC():
	global __IS_ARABIC
	return __IS_ARABIC

def IsWE_KOREA():
	return "locale/we_korea" == app.GetLocalePath()

# SUPPORT_NEW_KOREA_SERVER
def LoadLocaleData():
	if IsYMIR():
		import net
		SERVER = "Anka2"
		if SERVER == net.GetServerInfo()[:len(SERVER)]:
			app.SetCHEONMA(0)
			app.LoadLocaleData("locale/we_korea")
			constInfo.ADD_DEF_BONUS_ENABLE = 0
		else:
			app.SetCHEONMA(1)
			app.LoadLocaleData("locale/ymir")
			constInfo.ADD_DEF_BONUS_ENABLE = 1
	else:
		app.LoadLocaleData(app.GetLocalePath())

def IsCHEONMA():
	return IsYMIR()

# END_OF_SUPPORT_NEW_KOREA_SERVER

def mapping(**kwargs): return kwargs

def SNA(text):
	def f(x):
		return text
	return f

def SA(text):
	def f(x):
		return text % x
	return f

def SAA(text):
	def f(x1, x2):
		return text % (x1, x2)
	return f

def SAAAA(text):
	def f(x1, x2, x3, x4):
		return text % (x1, x2, x3, x4)
	return f

def SAN(text):
	def f(x1, x2):
		return text % x1
	return f

if app.ENABLE_LOCALE_CLIENT:
	alsoExportToCharset = "windows-1250"
	localeDict = {}

	def LoadLocaleFile(srcFileName):
		funcDict = { "SA" : SA, "SNA" : SNA, "SAA" : SAA, "SAN" : SAN, "SAAAA" : SAAAA, }

		lineIndex = 1

		try:
			lines = open(srcFileName, "r").readlines()
		except IOError:
			import dbg
			dbg.LogBox("LoadLocaleError(%(srcFileName)s)" % locals())
			app.Abort()

		global localeDict

		for line in lines:
			try:
				tokens = line[:-1].split("\t")
				if len(tokens) == 2:
					localeDict[tokens[0]] = tokens[1]
				elif len(tokens) >= 3:
					type = tokens[2].strip()
					if type:
						localeDict[tokens[0]] = funcDict[type](tokens[1])
					else:
						localeDict[tokens[0]] = tokens[1]
				else:
					raise RuntimeError, "Unknown TokenSize"

				lineIndex += 1

			except:
				import dbg
				dbg.LogBox("%s: line(%d): %s" % (srcFileName, lineIndex, line), "Error")
				raise

			globals().update(localeDict)

	def ReloadLocaleFile():
		global localeDict
		localeDict.clear()

		global GUILD_BUILDING_LIST_TXT
		GUILD_BUILDING_LIST_TXT = app.GetLocalePath() + "/GuildBuildingList.txt"

		global __IS_ENGLISH
		global __IS_HONGKONG
		global __IS_NEWCIBN
		global __IS_EUROPE
		global __IS_CANADA
		global __IS_BRAZIL
		global __IS_SINGAPORE
		global __IS_VIETNAM
		global __IS_ARABIC
		global __IS_CIBN10
		global __IS_WE_KOREA
		global __IS_TAIWAN
		global __IS_JAPAN

		__IS_ENGLISH = "ENGLISH" == app.GetLocaleServiceName()
		__IS_HONGKONG = "HONGKONG" == app.GetLocaleServiceName()
		__IS_NEWCIBN = "locale/newcibn" == app.GetLocalePath()
		__IS_EUROPE = "EUROPE" == app.GetLocaleServiceName()
		__IS_CANADA = "locale/ca" == app.GetLocalePath()
		__IS_BRAZIL = "locale/br" == app.GetLocalePath()
		__IS_SINGAPORE = "locale/sg" == app.GetLocalePath()
		__IS_VIETNAM = "locale/vn" == app.GetLocalePath()
		__IS_ARABIC = "locale/ae" == app.GetLocalePath()
		__IS_CIBN10 = "locale/cibn10" == app.GetLocalePath()
		__IS_WE_KOREA = "locale/we_korea" == app.GetLocalePath()
		__IS_TAIWAN = "locale/taiwan" == app.GetLocalePath()
		__IS_JAPAN = "locale/japan" == app.GetLocalePath()

		global LOCALE_FILE_NAME
		global NEW_LOCALE_FILE_NAME
		LOCALE_FILE_NAME = "%s/locale_game.txt" % app.GetLocalePath()
		NEW_LOCALE_FILE_NAME = "%s/new_locale_game.txt" % app.GetLocalePath()

		LoadLocaleFile(LOCALE_FILE_NAME)
		LoadLocaleFile(NEW_LOCALE_FILE_NAME)

		## NOTE : Reset all global variables after reloading the locale file
		## in order to change the strings properly.

		## All other global variables that are defined in other modules can be moved
		## into their class constructor which will reset them once they're loaded.

		global OPTION_PVPMODE_MESSAGE_DICT
		OPTION_PVPMODE_MESSAGE_DICT = {
			0 : PVP_MODE_NORMAL,
			1 : PVP_MODE_REVENGE,
			2 : PVP_MODE_KILL,
			3 : PVP_MODE_PROTECT,
			4 : PVP_MODE_GUILD,
		}

		global GUILDWAR_NORMAL_DESCLIST
		global GUILDWAR_WARP_DESCLIST
		global GUILDWAR_CTF_DESCLIST
		GUILDWAR_NORMAL_DESCLIST = [GUILD_WAR_USE_NORMAL_MAP, GUILD_WAR_LIMIT_30MIN, GUILD_WAR_WIN_CHECK_SCORE]
		GUILDWAR_WARP_DESCLIST = [GUILD_WAR_USE_BATTLE_MAP, GUILD_WAR_WIN_WIPE_OUT_GUILD, GUILD_WAR_REWARD_POTION]
		GUILDWAR_CTF_DESCLIST = [GUILD_WAR_USE_BATTLE_MAP, GUILD_WAR_WIN_TAKE_AWAY_FLAG1, GUILD_WAR_WIN_TAKE_AWAY_FLAG2, GUILD_WAR_REWARD_POTION]

		global MINIMAP_ZONE_NAME_DICT
		MINIMAP_ZONE_NAME_DICT = {
			"metin2_map_n_flame_01" : MAP_N_FLAME_01,
			"metin2_map_smhdungeon_01" : MAP_SUNGMAHEE_TOWER_WAIT,
			"metin2_map_secretdungeon_01" : MAP_SECRET_DUNGEON,
			"map_c2" : MAP_C2,
			"metin2_map_smhdungeon_02" : MAP_SUNGMAHEE_TOWER_WAIT,
			"metin2_map_dawnmistwood" : MAP_DAWNMISTWOOD,
			"metin2_map_b1" : MAP_B1,
			"metin2_map_b3" : MAP_B3,
			"metin2_map_boss_awaken_skipia" : MAP_BOSS_AWAKEN_SKIPIA,
			"metin2_map_moonlight_boss" : MAP_MOONLIGHT_VALLEY_BOOS_ROOM,
			"metin2_map_boss_awaken_flame" : MAP_BOSS_AWAKEN_FLAME,
			"metin2_map_spiderdungeon" : MAP_SPIDERDUNGEON,
			"metin2_guild_village_02" : GUILD_VILLAGE_02,
			"metin2_guild_village_03" : GUILD_VILLAGE_03,
			"metin2_guild_village_01" : GUILD_VILLAGE_01,
			"map_n_threeway" : MAP_N_THREEWAY,
			"map_b2" : MAP_B2,
			"metin2_map_whitedragoncave_boss" : MAP_WHITE_DRAGON_CAVE_BOSS,
			"metin2_map_smhgate_a1" : MAP_SUNGMAHEE_GATE_A1,
			"metin2_map_smhgate_devils" : MAP_SUNGMAHEE_GATE_DEVILS,
			"metin2_map_guild_whitedragon_boss_pass" : MAP_INDEX_GUILD_WHITE_DRAGON_PASS,
			"metin2_map_guild_summon" : MAP_GUILD_SUMMON,
			"metin2_map_greedy_cave" : MAP_INDEX_GREEDY_CAVE,
			"metin2_map_greedy_room" : MAP_INDEX_GREEDY_ROOM,
			"metin2_map_battlefied" : MAP_BATTLE_FIELD,
			"metin2_map_labyrinth" : MAP_BOSS_LABYRINTH,
			"metin2_map_monkeydungeon_03" : MAP_MONKEY_DUNGEON3,
			"metin2_map_monkeydungeon" : MAP_MONKEY_DUNGEON,
			"metin2_map_n_flame_dragon_pass" : MAP_N_FLAME_DRAGON,
			"metin2_map_skipia_dungeon_boss" : MAP_SKIPIA_DUNGEON_BOSS,
			"metin2_map_boss_crack_flame" : MAP_BOSS_CRACK_FLAME,
			"metin2_map_Mt_Thunder" : MAP_MT_THUNDER,
			"metin2_map_sungzi_snow_pass02" : MAP_SUNGZI_SNOW_PASS02,
			"metin2_map_sungzi_snow_pass03" : MAP_SUNGZI_SNOW_PASS03,
			"metin2_map_sungzi_snow_pass01" : MAP_SUNGZI_SNOW_PASS01,
			"metin2_map_deviltower1" : MAP_DEVILTOWER1,
			"metin2_map_eastplain_mystery" : MAP_WEAKENDED_MYSTERY_DUNGEON,
			"metin2_map_sungzi_desert_01" : MAP_SUNGZI_DESERT_01,
			"metin2_map_smhgate_snow" : MAP_SUNGMAHEE_GATE_SNOW,
			"metin2_map_empirecastle" : MAP_EMPIRECASTLE,
			"metin2_map_BayBlackSand" : MAP_BAYBLACKSAND,
			"metin2_map_smhgate_flame" : MAP_SUNGMAHEE_GATE_FLAME,
			"metin2_map_devilsCatacomb" : MAP_DEVILCATACOMB,
			"metin2_map_n_desert_01" : MAP_N_DESERT_01,
			"metin2_map_snake_temple_01" : MAP_SNAKE_TEMPLE_1,
			"metin2_map_snake_temple_02" : MAP_SNAKE_TEMPLE_2,
			"metin2_map_n_flame_dragon" : MAP_N_FLAME_DRAGON,
			"metin2_map_anglar_dungeon_01" : MAP_FARMING_DUNGEON,
			"metin2_12zi_stage" : MAP_CZ_DUNGEON,
			"metin2_map_guild_battle_base" : MAP_GUILD_BATTLE_BASE,
			"metin2_map_smhgate_c1" : MAP_SUNGMAHEE_GATE_C1,
			"metin2_map_t2" : MAP_T2,
			"metin2_map_t3" : MAP_T3,
			"metin2_map_t1" : MAP_T1,
			"metin2_map_guild_whitedragon_boss" : MAP_INDEX_GUILD_WHITE_DRAGON,
			"metin2_map_t4" : MAP_T4,
			"metin2_map_WL_01" : MAP_WL_01,
			"metin2_map_boss_crack_dawnmist" : MAP_BOSS_CRACK_DAWNMIST,
			"metin2_map_mists_of_island" : MAP_MISTS_OF_ISLAND,
			"metin2_map_miniboss_01" : MAP_MINIBOSS_01,
			"metin2_map_miniboss_02" : MAP_MINIBOSS_02,
			"metin2_map_sungzi_desert_hill_02" : MAP_SUNGZI_DESERT_HILL_02,
			"metin2_map_sungzi_desert_hill_03" : MAP_SUNGZI_DESERT_HILL_03,
			"metin2_map_n_snow_dungeon_01" : MAP_N_SNOW_DUNGEON_01,
			"metin2_map_sungzi_desert_hill_01" : MAP_SUNGZI_DESERT_HILL_01,
			"metin2_map_trent" : MAP_TRENT,
			"metin2_map_guild_01" : MAP_GUILD_01,
			"metin2_map_guild_02" : MAP_GUILD_02,
			"metin2_map_guild_03" : MAP_GUILD_03,
			"metin2_map_pvp_arena" : MAP_PVP_ARENA,
			"metin2_guild_war4" : GUILD_WAR4,
			"metin2_guild_war1" : GUILD_WAR1,
			"metin2_guild_war3" : GUILD_WAR3,
			"metin2_guild_war2" : GUILD_WAR2,
			"metin2_map_n_flame_dungeon_01" : MAP_N_FLAME_DUNGEON_01,
			"metin2_map_c1" : MAP_C1,
			"metin2_map_oxevent" : MAP_OXEVENT,
			"metin2_map_otherworld_01" : MAP_OTHER_WORLD_01,
			"metin2_map_Mt_Th_dungeon_01" : MAP_MT_TH_DUNGEON_01,
			"metin2_map_monkeydungeon_02" : MAP_MONKEY_DUNGEON2,
			"metin2_map_otherworld_02" : MAP_OTHER_WORLD_02,
			"metin2_map_otherworld_03" : MAP_OTHER_WORLD_03,
			"metin2_map_otherworld_04" : MAP_OTHER_WORLD_04,
			"metin2_map_sungzi_snow" : MAP_SUNGZI_SNOW,
			"metin2_map_sungzi" : MAP_SUNGZI,
			"metin2_map_smhgate_dawnmist" : MAP_SUNGMAHEE_GATE_DAWNMIST,
			"metin2_map_defensewave_port" : MAP_DEFENSE_WAVE_PORT,
			"metin2_map_boss_awaken_dawnmist" : MAP_BOSS_AWAKEN_DAWNMIST,
			"metin2_map_smhgate_desert" : MAP_SUNGMAHEE_GATE_DESERT,
			"metin2_map_guild_summon_nw" : MAP_GUILD_SUMMON,
			"metin2_map_a1" : MAP_A1,
			"metin2_map_a3" : MAP_A3,
			"metin2_map_milgyo" : MAP_MILGYO,
			"metin2_map_duel" : MAP_DUEL,
			"metin2_map_monkey_dungeon_11" : MAP_MONKEY_DUNGEON_11,
			"metin2_map_smhgate_b1" : MAP_SUNGMAHEE_GATE_B1,
			"metin2_map_monkey_dungeon_13" : MAP_MONKEY_DUNGEON_13,
			"metin2_map_monkey_dungeon_12" : MAP_MONKEY_DUNGEON_12,
			"metin2_map_spiderdungeon_02" : MAP_SPIDERDUNGEON_02,
			"metin2_map_spiderdungeon_03" : MAP_SPIDERDUNGEON_03,
			"metin2_map_boss_awaken_snow" : MAP_BOSS_AWAKEN_SNOW,
			"gm_guild_build" : MAP_PATHWAY_ICE_02,
			"metin2_map_privateshop" : MAP_PRIVATE_SHOP_MAP,
			"metin2_map_battlearena01" : MAP_BATTLEARENA_01,
			"metin2_map_nusluck01" : MAP_NUSLUCK01,
			"metin2_map_battlearena02" : MAP_BATTLEARENA_02,
			"metin2_map_battlearena03" : MAP_BATTLEARENA_03,
			"metin2_map_dawnmist_dungeon_01" : MAP_DAWNMIST_DUNGEON_01,
			"map_n_snowm_01" : MAP_N_SNOWM_01,
			"metin2_map_sungzi_flame_hill_03" : MAP_SUNGZI_FLAME_HILL_03,
			"metin2_map_sungzi_flame_hill_02" : MAP_SUNGZI_FLAME_HILL_02,
			"metin2_map_sungzi_flame_hill_01" : MAP_SUNGZI_FLAME_HILL_01,
			"metin2_map_smhgate_threeway" : MAP_SUNGMAHEE_GATE_THREEWAY,
			"metin2_map_maze_dungeon3" : MAP_MAZE_DUNGEON_03,
			"metin2_map_maze_dungeon2" : MAP_MAZE_DUNGEON_02,
			"metin2_map_maze_dungeon1" : MAP_MAZE_DUNGEON_01,
			"metin2_map_defensewave" : MAP_DEFENSE_WAVE,
			"metin2_map_eastplain_01" : MAP_EASTPLAIN_01,
			"metin2_map_eastplain_02" : MAP_EASTPLAIN_02,
			"metin2_map_eastplain_03" : MAP_EASTPLAIN_03,
			"metin2_map_whitedragoncave_02" : MAP_WHITE_DRAGON_CAVE_02,
			"metin2_map_whitedragoncave_01" : MAP_WHITE_DRAGON_CAVE_01,
			"metin2_map_icecrystalcave" : MAP_ICE_CRYSTALS_CAVE,
			"metin2_map_skipia_dungeon_02" : MAP_SKIPIA_DUNGEON_02,
			"metin2_map_skipia_dungeon_01" : MAP_SKIPIA_DUNGEON_01,
			"metin2_map_treasure_hunt" : MAP_TREASURE_ISLAND,
			"metin2_map_golden_land" : MAP_GOLDEN_LAND,
			"metin2_map_golden_land_stage" : MAP_GOLDEN_LAND_STAGE,
			"metin2_map_moonlight" : MAP_MOONLIGHT_VALLEY,
			"metin2_map_guild_battle" : MAP_GUILD_BATTLE,
			"metin2_map_c3" : MAP_C3,
			"metin2_map_elemental_04" : MAP_ELEMENTAL_DUNGEON,
			"metin2_map_elemental_02" : MAP_ELEMENTAL_DUNGEON_FIRE,
			"metin2_map_elemental_03" : MAP_ELEMENTAL_DUNGEON_ELECTRICITY,
			"metin2_map_snakevalley" : MAP_SNAKE_VALLEY,
			"metin2_map_elemental_01" : MAP_ELEMENTAL_DUNGEON_DARK,
			"metin2_map_wedding_01" : MAP_WEDDING_01,
			"metin2_map_boss_crack_snow" : MAP_BOSS_CRACK_SNOW,
			"metin2_map_trent02" : MAP_TRENT02,
			"metin2_map_empirewar01" : MAP_EMPIREWAR01,
			"metin2_map_boss_crack_skipia" : MAP_BOSS_CRACK_SKIPIA,
			"metin2_map_empirewar03" : MAP_EMPIREWAR03,
			"metin2_map_empirewar02" : MAP_EMPIREWAR02,
			"metin2_map_battleroyale" : MAP_BATTLE_ROYALE,
			"metin2_map_whitdragonvalley" : MAP_WHITE_DRAGON_VALLEY,
			"Metin2_map_CapeDragonHead" : MAP_CAPEDRAGONHEAD,
		}

		global JOBINFO_TITLE
		JOBINFO_TITLE = [
			[ JOB_WARRIOR0, JOB_WARRIOR1, JOB_WARRIOR2, ],
			[ JOB_ASSASSIN0, JOB_ASSASSIN1, JOB_ASSASSIN2, ],
			[ JOB_SURA0, JOB_SURA1, JOB_SURA2, ],
			[ JOB_SHAMAN0, JOB_SHAMAN1, JOB_SHAMAN2, ],
			[ JOB_WOLFMAN0, JOB_WOLFMAN1, JOB_WOLFMAN1, ],
		]

		if app.WJ_SHOW_PARTY_ON_MINIMAP:
			global MINIMAP_ZONE_NAME_DICT_BY_IDX
			MINIMAP_ZONE_NAME_DICT_BY_IDX = {
				0 : "",
				1 : MAP_A1,
				3 : MAP_A3,
				4 : MAP_GUILD_01,
				5 : MAP_MONKEY_DUNGEON_11,
				6 : GUILD_VILLAGE_01,
				21 : MAP_B1,
				23 : MAP_B3,
				24 : MAP_GUILD_02,
				25 : MAP_MONKEY_DUNGEON_12,
				26 : GUILD_VILLAGE_02,
				41 : MAP_C1,
				43 : MAP_C3,
				44 : MAP_GUILD_03,
				45 : MAP_MONKEY_DUNGEON_13,
				46 : GUILD_VILLAGE_03,
				61 : MAP_N_SNOWM_01,
				62 : MAP_N_FLAME_01,
				63 : MAP_N_DESERT_01,
				64 : MAP_N_THREEWAY,
				65 : MAP_MILGYO,
				66 : MAP_DEVILTOWER1,
				67 : MAP_TRENT,
				68 : MAP_TRENT02,
				69 : MAP_WL_01,
				70 : MAP_NUSLUCK01,
				71 : MAP_SPIDERDUNGEON_02,
				72 : MAP_SKIPIA_DUNGEON_01,
				73 : MAP_SKIPIA_DUNGEON_02,
				74 : MAP_N_SNOWM_02,
				75 : MAP_N_FLAME_02,
				76 : MAP_N_DESERT_02,
				77 : MAP_A2_1,
				78 : MAP_MILGYO_A,
				79 : MAP_TRENT_A,
				80 : MAP_TRENT02_A,
				81 : MAP_WEDDING_01,
				91 : MAP_E,
				92 : MAP_E,
				93 : MAP_E,
				103 : MAP_T1,
				104 : MAP_SPIDERDUNGEON,
				105 : MAP_T2,
				107 : MAP_MONKEY_DUNGEON,
				108 : MAP_MONKEY_DUNGEON2,
				109 : MAP_MONKEY_DUNGEON3,
				110 : MAP_T3,
				111 : MAP_T4,
				112 : MAP_DUEL,
				113 : MAP_OXEVENT,
				114 : MAP_SUNGZI,
				118 : MAP_SUNGZI_FLAME_HILL_01,
				119 : MAP_SUNGZI_FLAME_HILL_02,
				120 : MAP_SUNGZI_FLAME_HILL_03,
				121 : MAP_SUNGZI_SNOW,
				122 : MAP_SUNGZI_SNOW_PASS01,
				123 : MAP_SUNGZI_SNOW_PASS02,
				124 : MAP_SUNGZI_SNOW_PASS03,
				125 : MAP_SUNGZI_DESERT_01,
				126 : MAP_SUNGZI_DESERT_HILL_01,
				127 : MAP_SUNGZI_DESERT_HILL_02,
				128 : MAP_SUNGZI_DESERT_HILL_03,
				130 : GUILD_WAR1,
				131 : GUILD_WAR2,
				132 : GUILD_WAR3,
				133 : GUILD_WAR4,
				180 : METIN_TEST,
				181 : MAP_EMPIREWAR01,
				182 : MAP_EMPIREWAR02,
				183 : MAP_EMPIREWAR03,
				184 : MAP_SKIPIA_DUNGEON_011,
				185 : MAP_SKIPIA_DUNGEON_021,
				186 : MAP_SKIPIA_DUNGEON_012,
				187 : MAP_SKIPIA_DUNGEON_022,
				188 : MAP_SKIPIA_DUNGEON_013,
				189 : MAP_SKIPIA_DUNGEON_023,
				193 : MAP_SPIDERDUNGEON_02_1,
				194 : MAP_HOLYPLACE_FLAME,
				195 : MAP_PATHWAY_FLAME_01,
				196 : MAP_PATHWAY_FLAME_02,
				197 : MAP_PATHWAY_FLAME_03,
				198 : MAP_HOLYPLACE_ICE,
				199 : MAP_PATHWAY_ICE_01,
				200 : MAP_PATHWAY_ICE_02,
				201 : MAP_PATHWAY_ICE_03,
				202 : MAP_HOLYPLACE_DESERT,
				203 : MAP_PATHWAY_DESERT_01,
				204 : MAP_PATHWAY_DESERT_02,
				205 : MAP_PATHWAY_DESERT_03,
				208 : MAP_SKIPIA_DUNGEON_BOSS,
				209 : MAP_SKIPIA_DUNGEON_BOSS_1,
				210 : MAP_SKIPIA_DUNGEON_BOSS_2,
				211 : MAP_SKIPIA_DUNGEON_BOSS_3,
				216 : MAP_DEVILCATACOMB,
				217 : MAP_SPIDERDUNGEON_03,
				301 : MAP_CAPEDRAGONHEAD,
				302 : MAP_DAWNMISTWOOD,
				303 : MAP_BAYBLACKSAND,
				304 : MAP_MT_THUNDER,
				351 : MAP_N_FLAME_DUNGEON_01,
				352 : MAP_N_SNOW_DUNGEON_01,
				353 : MAP_DAWNMIST_DUNGEON_01,
				354 : MAP_MT_TH_DUNGEON_01,
				355 : MAP_CZ_DUNGEON,
				356 : MAP_N_FLAME_DRAGON,
				357 : MAP_BATTLE_FIELD,
				358 : MAP_DEFENSE_WAVE,
				359 : MAP_DEFENSE_WAVE_PORT,
				360 : MAP_MISTS_OF_ISLAND,
				361 : MAP_MINIBOSS_01,
				362 : MAP_MINIBOSS_02,
				363 : MAP_BOSS_LABYRINTH,
				364 : MAP_BOSS_CRACK_SKIPIA,
				365 : MAP_BOSS_CRACK_FLAME,
				366 : MAP_BOSS_CRACK_SNOW,
				367 : MAP_BOSS_CRACK_DAWNMIST,
				368 : MAP_BOSS_AWAKEN_SKIPIA,
				369 : MAP_BOSS_AWAKEN_FLAME,
				370 : MAP_BOSS_AWAKEN_SNOW,
				371 : MAP_BOSS_AWAKEN_DAWNMIST,
				372 : MAP_GUILD_PVE,
				373 : MAP_EASTPLAIN_01,
				374 : MAP_EMPIRECASTLE,
				375 : MAP_BATTLE_ROYALE,
				376 : MAP_EASTPLAIN_02,
				377 : MAP_EASTPLAIN_03,
				378 : MAP_ELEMENTAL_DUNGEON_DARK,
				379 : MAP_ELEMENTAL_DUNGEON_FIRE,
				380 : MAP_ELEMENTAL_DUNGEON_ELECTRICITY,
				381 : MAP_ELEMENTAL_DUNGEON,
				382 : MAP_MAZE_DUNGEON_01,
				383 : MAP_MAZE_DUNGEON_02,
				384 : MAP_MAZE_DUNGEON_03,
				385 : MAP_SNAKE_VALLEY,
				386 : MAP_SUNGMAHEE_TOWER_WAIT,
				387 : MAP_SUNGMAHEE_TOWER_DUNGEON,
				388 : MAP_ICE_CRYSTALS_CAVE,
				389 : MAP_WHITE_DRAGON_VALLEY,
				390 : MAP_SNAKE_TEMPLE_1,
				391 : MAP_SNAKE_TEMPLE_2,
				392 : MAP_PRIVATE_SHOP_MAP,
				393 : MAP_WHITE_DRAGON_CAVE_01,
				394 : MAP_WHITE_DRAGON_CAVE_02,
				395 : MAP_WHITE_DRAGON_CAVE_BOSS,
				396 : MAP_FARMING_DUNGEON,
				399 : MAP_SECRET_DUNGEON,
				400 : MAP_OTHER_WORLD_02,
				401 : MAP_OTHER_WORLD_01,
				402 : MAP_OTHER_WORLD_03,
				403 : MAP_OTHER_WORLD_04,
				404 : MAP_SUNGMAHEE_GATE_A1,
				405 : MAP_SUNGMAHEE_GATE_B1,
				406 : MAP_SUNGMAHEE_GATE_C1,
				407 : MAP_SUNGMAHEE_GATE_THREEWAY,
				408 : MAP_SUNGMAHEE_GATE_DESERT,
				409 : MAP_SUNGMAHEE_GATE_FLAME,
				410 : MAP_SUNGMAHEE_GATE_SNOW,
				411 : MAP_SUNGMAHEE_GATE_DEVILS,
				412 : MAP_SUNGMAHEE_GATE_DAWNMIST,
				413 : MAP_GUILD_BATTLE_BASE,
				414 : MAP_GUILD_BATTLE,
				415 : MAP_WEAKENDED_MYSTERY_DUNGEON,
				419 : MAP_TREASURE_ISLAND,
				420 : MAP_INDEX_GUILD_WHITE_DRAGON_PASS,
				421 : MAP_MOONLIGHT_VALLEY,
				422 : MAP_MOONLIGHT_VALLEY_BOOS_ROOM,
				423 : MAP_INDEX_GREEDY_CAVE,
				424 : MAP_INDEX_GREEDY_ROOM,
				426 : MAP_GUILD_SUMMON,
				427 : MAP_GUILD_SUMMON,
				428 : MAP_N_FLAME_DRAGON,
				429 : MAP_INDEX_GUILD_WHITE_DRAGON,
			}
			#if background.IsMapInfoByMapName("metin2_map_dawnmist_dungeon_01"):
			MINIMAP_ZONE_NAME_DICT_BY_IDX[353] = MAP_DAWNMIST_DUNGEON_01
			#if background.IsMapInfoByMapName("metin2_map_Mt_Th_dungeon_01"):
			MINIMAP_ZONE_NAME_DICT_BY_IDX[354] = MAP_MT_TH_DUNGEON_01
			#if background.IsMapInfoByMapName("metin2_map_n_flame_dragon"):
			MINIMAP_ZONE_NAME_DICT_BY_IDX[356] = MAP_N_FLAME_DRAGON
			#if background.IsMapInfoByMapName("metin2_map_battlefied"):
			MINIMAP_ZONE_NAME_DICT_BY_IDX[357] = MAP_BATTLE_FIELD
			#if background.IsMapInfoByMapName("metin2_12zi_stage"):
			MINIMAP_ZONE_NAME_DICT_BY_IDX[355] = MAP_CZ_DUNGEON

		global WHISPER_ERROR
		WHISPER_ERROR = {
			1 : CANNOT_WHISPER_NOT_LOGON,
			2 : CANNOT_WHISPER_DEST_REFUSE,
			3 : CANNOT_WHISPER_SELF_REFUSE,
		}

		global NOTIFY_MESSAGE
		NOTIFY_MESSAGE = {
			"CANNOT_EQUIP_SHOP" : CANNOT_EQUIP_IN_SHOP,
			"CANNOT_EQUIP_EXCHANGE" : CANNOT_EQUIP_IN_EXCHANGE,
		}

		global ATTACK_ERROR_TAIL_DICT
		ATTACK_ERROR_TAIL_DICT = {
			"IN_SAFE" : CANNOT_ATTACK_SELF_IN_SAFE,
			"DEST_IN_SAFE" : CANNOT_ATTACK_DEST_IN_SAFE,
		}

		global SHOT_ERROR_TAIL_DICT
		SHOT_ERROR_TAIL_DICT = {
			"EMPTY_ARROW" : CANNOT_SHOOT_EMPTY_ARROW,
			"IN_SAFE" : CANNOT_SHOOT_SELF_IN_SAFE,
			"DEST_IN_SAFE" : CANNOT_SHOOT_DEST_IN_SAFE,
		}

		global USE_SKILL_ERROR_TAIL_DICT
		USE_SKILL_ERROR_TAIL_DICT = {
			"IN_SAFE" : CANNOT_SKILL_SELF_IN_SAFE,
			"NEED_TARGET" : CANNOT_SKILL_NEED_TARGET,
			"NEED_EMPTY_BOTTLE" : CANNOT_SKILL_NEED_EMPTY_BOTTLE,
			"NEED_POISON_BOTTLE" : CANNOT_SKILL_NEED_POISON_BOTTLE,
			"REMOVE_FISHING_ROD" : CANNOT_SKILL_REMOVE_FISHING_ROD,
			"NOT_YET_LEARN" : CANNOT_SKILL_NOT_YET_LEARN,
			"NOT_MATCHABLE_WEAPON" : CANNOT_SKILL_NOT_MATCHABLE_WEAPON,
			"WAIT_COOLTIME" : CANNOT_SKILL_WAIT_COOLTIME,
			"NOT_ENOUGH_HP" : CANNOT_SKILL_NOT_ENOUGH_HP,
			"NOT_ENOUGH_SP" : CANNOT_SKILL_NOT_ENOUGH_SP,
			"CANNOT_USE_SELF" : CANNOT_SKILL_USE_SELF,
			"ONLY_FOR_ALLIANCE" : CANNOT_SKILL_ONLY_FOR_ALLIANCE,
			"CANNOT_ATTACK_ENEMY_IN_SAFE_AREA" : CANNOT_SKILL_DEST_IN_SAFE,
			"CANNOT_APPROACH" : CANNOT_SKILL_APPROACH,
			"CANNOT_ATTACK" : CANNOT_SKILL_ATTACK,
			"ONLY_FOR_CORPSE" : CANNOT_SKILL_ONLY_FOR_CORPSE,
			"EQUIP_FISHING_ROD" : CANNOT_SKILL_EQUIP_FISHING_ROD,
			"NOT_HORSE_SKILL" : CANNOT_SKILL_NOT_HORSE_SKILL,
			"HAVE_TO_RIDE" : CANNOT_SKILL_HAVE_TO_RIDE,
		}

		global LEVEL_LIST
		LEVEL_LIST = ["", HORSE_LEVEL1, HORSE_LEVEL2, HORSE_LEVEL3]

		global HEALTH_LIST
		HEALTH_LIST = [
			HORSE_HEALTH0,
			HORSE_HEALTH1,
			HORSE_HEALTH2,
			HORSE_HEALTH3,
		]

		global USE_SKILL_ERROR_CHAT_DICT
		USE_SKILL_ERROR_CHAT_DICT = {
			"NEED_EMPTY_BOTTLE" : SKILL_NEED_EMPTY_BOTTLE,
			"NEED_POISON_BOTTLE" : SKILL_NEED_POISON_BOTTLE,
			"ONLY_FOR_GUILD_WAR" : SKILL_ONLY_FOR_GUILD_WAR,
		}

		global SHOP_ERROR_DICT
		SHOP_ERROR_DICT = {
			"NOT_ENOUGH_MONEY" : SHOP_NOT_ENOUGH_MONEY,
			"SOLDOUT" : SHOP_SOLDOUT,
			"INVENTORY_FULL" : SHOP_INVENTORY_FULL,
			"INVALID_POS" : SHOP_INVALID_POS,
			"NOT_ENOUGH_MONEY_EX" : SHOP_NOT_ENOUGH_MONEY_EX,
		}
		if app.ENABLE_SHOPEX_RENEWAL:
			SHOP_ERROR_DICT.update({"NOT_ENOUGH_ITEM": SHOP_NOT_ENOUGH_ITEM,})

		global STAT_MINUS_DESCRIPTION
		STAT_MINUS_DESCRIPTION = {
			"HTH-" : STAT_MINUS_CON,
			"INT-" : STAT_MINUS_INT,
			"STR-" : STAT_MINUS_STR,
			"DEX-" : STAT_MINUS_DEX,
		}

		global MODE_NAME_LIST
		global TITLE_NAME_LIST
		MODE_NAME_LIST = (PVP_OPTION_NORMAL, PVP_OPTION_REVENGE, PVP_OPTION_KILL, PVP_OPTION_PROTECT,)
		TITLE_NAME_LIST = (PVP_LEVEL0, PVP_LEVEL1, PVP_LEVEL2, PVP_LEVEL3, PVP_LEVEL4, PVP_LEVEL5, PVP_LEVEL6, PVP_LEVEL7, PVP_LEVEL8,)

		if app.ENABLE_GUILD_LEADER_GRADE_NAME:
			global GUILD_LEADER_GRADE_NAME_LIST
			GUILD_LEADER_GRADE_NAME_LIST = (GUILD_LEADER_GRADE0, GUILD_LEADER_GRADE1)

else:
	def LoadLocaleFile(srcFileName, localeDict):
		funcDict = { "SA" : SA, "SNA" : SNA, "SAA" : SAA, "SAN" : SAN, "SAAAA" : SAAAA, }

		lineIndex = 1

		try:
			lines = open(srcFileName, "r").readlines()
		except IOError:
			import dbg
			dbg.LogBox("LoadLocaleError(%(srcFileName)s)" % locals())
			app.Abort()

		for line in lines:
			try:
				tokens = line[:-1].split("\t")
				if len(tokens) == 2:
					localeDict[tokens[0]] = tokens[1]
				elif len(tokens) >= 3:
					type = tokens[2].strip()
					if type:
						localeDict[tokens[0]] = funcDict[type](tokens[1])
					else:
						localeDict[tokens[0]] = tokens[1]
				else:
					raise RuntimeError, "Unknown TokenSize"

				lineIndex += 1

			except:
				import dbg
				dbg.LogBox("%s: line(%d): %s" % (srcFileName, lineIndex, line), "Error")
				raise

all = ["locale", "error"]

if IsEUROPE() and IsBRAZIL():
	FN_GM_MARK = "locale/common/effect/gm.mse"
	LOCALE_FILE_NAME = "%s/locale_game.txt" % app.GetLocalePath()
	NEW_LOCALE_FILE_NAME = "%s/new_locale_game.txt" % app.GetLocalePath()

	constInfo.IN_GAME_SHOP_ENABLE = 0
elif IsSINGAPORE():
	FN_GM_MARK = "locale/common/effect/gm.mse"
	LOCALE_FILE_NAME = "%s/locale_game.txt" % app.GetLocalePath()
	NEW_LOCALE_FILE_NAME = "%s/new_locale_game.txt" % app.GetLocalePath()

	constInfo.IN_GAME_SHOP_ENABLE = 0
elif IsNEWCIBN():
	APP_TITLE = "Anka2"
	FN_GM_MARK = "locale/common/effect/gm.mse"
	LOCALE_FILE_NAME = "%s/locale_game.txt" % app.GetLocalePath()
	NEW_LOCALE_FILE_NAME = "%s/new_locale_game.txt" % app.GetLocalePath()

	constInfo.IN_GAME_SHOP_ENABLE = 1
elif IsTAIWAN():
	APP_TITLE = "Anka2"
	FN_GM_MARK = "locale/common/effect/gm.mse"
	LOCALE_FILE_NAME = "%s/locale_game.txt" % app.GetLocalePath()
	NEW_LOCALE_FILE_NAME = "%s/new_locale_game.txt" % app.GetLocalePath()

	constInfo.IN_GAME_SHOP_ENABLE = 1
else:
	FN_GM_MARK = "locale/common/effect/gm.mse"
	LOCALE_FILE_NAME = "%s/locale_game.txt" % app.GetLocalePath()
	NEW_LOCALE_FILE_NAME = "%s/new_locale_game.txt" % app.GetLocalePath()

	constInfo.IN_GAME_SHOP_ENABLE = 1

if app.ENABLE_LOCALE_CLIENT:
	LoadLocaleFile(LOCALE_FILE_NAME)
	LoadLocaleFile(NEW_LOCALE_FILE_NAME)
else:
	LoadLocaleFile(LOCALE_FILE_NAME, locals())
	LoadLocaleFile(NEW_LOCALE_FILE_NAME, locals())

dictSingleWord = {
	"m":1, "n":1, "r":1, "M":1, "N":1, "R":1, "l":1, "L":1, "1":1, "3":1, "6":1, "7":1, "8":1, "0":1,
}

dictDoubleWord = {
	"?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1,
	"?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1,
	"?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1,
	"?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1,
	"?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1,
	"?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1,
	"?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1,
	"?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1,
	"?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1,
	"?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1,
	"?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1, "?":1,
}

locale = mapping(
)

def GetAuxiliaryWordType(text):
	textLength = len(text)

	if textLength > 1:
		singleWord = text[-1]

		if (singleWord >= '0' and singleWord <= '9') or\
			(singleWord >= 'a' and singleWord <= 'z') or\
			(singleWord >= 'A' and singleWord <= 'Z'):
			if not dictSingleWord.has_key(singleWord):
				return 1

		elif dictDoubleWord.has_key(text[-2:]):
			return 1

	return 0

def CutMoneyString(sourceText, startIndex, endIndex, insertingText, backText):
	sourceLength = len(sourceText)

	if sourceLength < startIndex:
		return backText

	text = sourceText[max(0, sourceLength-endIndex):sourceLength-startIndex]

	if not text:
		return backText

	if int(text) <= 0:
		return backText

	text = str(int(text))

	if backText:
		backText = " " + backText

	return text + insertingText + backText

if app.ENABLE_GEM_SYSTEM:
	def SecondToHMGolbal(time):
		if time < 60:
			return "00:01"

		minute = int((time / 60) % 60)
		hour = int((time / 60) / 60)

		text = ""

		if hour > 0:
			if hour >= 10:
				text += str(hour) + ":"
			else:
				text += "0" + str(hour) + ":"
		else:
			text += "00:"

		if minute > 0:
			if minute >= 10:
				text += str(minute)
			else:
				text += "0" + str(minute)

		return text

def SecondToDHMS(time):
	if time < 60:
		if IsARABIC():
			return "%.2f %s" % (time, SECOND)
		else:
			return "%.2f %s" % (time, SECOND)

	second = int(time % 60)
	minute = int((time / 60) % 60)
	hour = int((time / 60) / 60) % 24
	day = int(int((time / 60) / 60) / 24)

	text = ""

	if day > 0:
		text += str(day) + DAY
		text += " "

	if hour > 0:
		text += str(hour) + HOUR
		text += " "

	if minute > 0:
		text += str(minute) + MINUTE

	if second > 0:
		text += str(second) + SECOND

	return text

def SecondToDHM(time):
	if time < 60:
		if IsARABIC():
			return "%.2f %s" % (time, SECOND)
		else:
			return "0" + MINUTE

	second = int(time % 60)
	minute = int((time / 60) % 60)
	hour = int((time / 60) / 60) % 24
	day = int(int((time / 60) / 60) / 24)

	text = ""

	if day > 0:
		text += str(day) + DAY
		text += " "

	if hour > 0:
		text += str(hour) + HOUR
		text += " "

	if minute > 0:
		text += str(minute) + MINUTE

	return text

def SecondToHM(time):
	if time < 60:
		if IsARABIC():
			return "%.2f %s" % (time, SECOND)
		else:
			return "0" + MINUTE

	second = int(time % 60)
	minute = int((time / 60) % 60)
	hour = int((time / 60) / 60)

	text = ""

	if hour > 0:
		text += str(hour) + HOUR
		if hour > 0:
			text += " "

	if minute > 0:
		text += str(minute) + MINUTE

	return text

if app.ENABLE_GROWTH_PET_SYSTEM:
	def SecondToDay(time):
		if time < 60:
			return "1" + DAY

		second = int(time % 60)
		minute = int((time / 60) % 60)
		hour = int((time / 60) / 60) % 24
		day = int(int((time / 60) / 60) / 24)

		if day < 1:
			day = 1

		if day > 9999:
			day = 9999

		text = str(day) + DAY
		return text

if app.ENABLE_GROWTH_PET_SYSTEM:
	def SecondToDayNumber(time):
		if time < 60:
			return 1

		second = int(time % 60)
		minute = int((time / 60) % 60)
		hour = int((time / 60) / 60) % 24
		day = int(int((time / 60) / 60) / 24)

		if day < 1:
			day = 1

		if day > 9999:
			day = 9999

		return day

if app.ENABLE_GROWTH_PET_SYSTEM:
	def SecondToH(time):
		hour = int((time / 60) / 60)

		text = ""
		hour = max( 0, hour )
		text += str(hour)

		return text

#if app.ENABLE_BATTLE_FIELD:
	def SecondToColonTypeHM(time):
		if time < 60:
			return "00:00"

		minute = int((time / 60) % 60)
		hour = int((time / 60) / 60)

		return "%02d:%02d" % (hour, minute)

#if app.ENABLE_GUILD_DRAGONLAIR_PARTY_SYSTEM or app.ENABLE_12ZI:
	def SecondToColonTypeHMS(time):
		if time < 60:
			return "00:00:%02d" % (time)

		second = int(time % 60)
		minute = int((time / 60) % 60)
		hour = int((time / 60) / 60)

		return "%02d:%02d:%02d" % (hour, minute, second)

	def SecondToColonTypeMS(time):

		second = int(time % 60)
		minute = int((time / 60) % 60)

		return "%02d:%02d" % (minute, second)

def GetAlignmentTitleName(alignment):
	if alignment >= 12000:
		return TITLE_NAME_LIST[0]
	elif alignment >= 8000:
		return TITLE_NAME_LIST[1]
	elif alignment >= 4000:
		return TITLE_NAME_LIST[2]
	elif alignment >= 1000:
		return TITLE_NAME_LIST[3]
	elif alignment >= 0:
		return TITLE_NAME_LIST[4]
	elif alignment > -4000:
		return TITLE_NAME_LIST[5]
	elif alignment > -8000:
		return TITLE_NAME_LIST[6]
	elif alignment > -12000:
		return TITLE_NAME_LIST[7]

	return TITLE_NAME_LIST[8]

OPTION_PVPMODE_MESSAGE_DICT = {
	0 : PVP_MODE_NORMAL,
	1 : PVP_MODE_REVENGE,
	2 : PVP_MODE_KILL,
	3 : PVP_MODE_PROTECT,
	4 : PVP_MODE_GUILD,
}

error = mapping(
	CREATE_WINDOW = GAME_INIT_ERROR_MAIN_WINDOW,
	CREATE_CURSOR = GAME_INIT_ERROR_CURSOR,
	CREATE_NETWORK = GAME_INIT_ERROR_NETWORK,
	CREATE_ITEM_PROTO = GAME_INIT_ERROR_ITEM_PROTO,
	CREATE_MOB_PROTO = GAME_INIT_ERROR_MOB_PROTO,
	CREATE_NO_DIRECTX = GAME_INIT_ERROR_DIRECTX,
	CREATE_DEVICE = GAME_INIT_ERROR_GRAPHICS_NOT_EXIST,
	CREATE_NO_APPROPRIATE_DEVICE = GAME_INIT_ERROR_GRAPHICS_BAD_PERFORMANCE,
	CREATE_FORMAT = GAME_INIT_ERROR_GRAPHICS_NOT_SUPPORT_32BIT,
	NO_ERROR = ""
)

GUILDWAR_NORMAL_DESCLIST = [GUILD_WAR_USE_NORMAL_MAP, GUILD_WAR_LIMIT_30MIN, GUILD_WAR_WIN_CHECK_SCORE]
GUILDWAR_WARP_DESCLIST = [GUILD_WAR_USE_BATTLE_MAP, GUILD_WAR_WIN_WIPE_OUT_GUILD, GUILD_WAR_REWARD_POTION]
GUILDWAR_CTF_DESCLIST = [GUILD_WAR_USE_BATTLE_MAP, GUILD_WAR_WIN_TAKE_AWAY_FLAG1, GUILD_WAR_WIN_TAKE_AWAY_FLAG2, GUILD_WAR_REWARD_POTION]

MINIMAP_ZONE_NAME_DICT = {
	"metin2_map_n_flame_01" : MAP_N_FLAME_01,
	"metin2_map_smhdungeon_01" : MAP_SUNGMAHEE_TOWER_WAIT,
	"metin2_map_secretdungeon_01" : MAP_SECRET_DUNGEON,
	"map_c2" : MAP_C2,
	"metin2_map_smhdungeon_02" : MAP_SUNGMAHEE_TOWER_WAIT,
	"metin2_map_dawnmistwood" : MAP_DAWNMISTWOOD,
	"metin2_map_b1" : MAP_B1,
	"metin2_map_b3" : MAP_B3,
	"metin2_map_boss_awaken_skipia" : MAP_BOSS_AWAKEN_SKIPIA,
	"metin2_map_moonlight_boss" : MAP_MOONLIGHT_VALLEY_BOOS_ROOM,
	"metin2_map_boss_awaken_flame" : MAP_BOSS_AWAKEN_FLAME,
	"metin2_map_spiderdungeon" : MAP_SPIDERDUNGEON,
	"metin2_guild_village_02" : GUILD_VILLAGE_02,
	"metin2_guild_village_03" : GUILD_VILLAGE_03,
	"metin2_guild_village_01" : GUILD_VILLAGE_01,
	"map_n_threeway" : MAP_N_THREEWAY,
	"map_b2" : MAP_B2,
	"metin2_map_whitedragoncave_boss" : MAP_WHITE_DRAGON_CAVE_BOSS,
	"metin2_map_smhgate_a1" : MAP_SUNGMAHEE_GATE_A1,
	"metin2_map_smhgate_devils" : MAP_SUNGMAHEE_GATE_DEVILS,
	"metin2_map_guild_whitedragon_boss_pass" : MAP_INDEX_GUILD_WHITE_DRAGON_PASS,
	"metin2_map_guild_summon" : MAP_GUILD_SUMMON,
	"metin2_map_greedy_cave" : MAP_INDEX_GREEDY_CAVE,
	"metin2_map_greedy_room" : MAP_INDEX_GREEDY_ROOM,
	"metin2_map_battlefied" : MAP_BATTLE_FIELD,
	"metin2_map_labyrinth" : MAP_BOSS_LABYRINTH,
	"metin2_map_monkeydungeon_03" : MAP_MONKEY_DUNGEON3,
	"metin2_map_monkeydungeon" : MAP_MONKEY_DUNGEON,
	"metin2_map_n_flame_dragon_pass" : MAP_N_FLAME_DRAGON,
	"metin2_map_skipia_dungeon_boss" : MAP_SKIPIA_DUNGEON_BOSS,
	"metin2_map_boss_crack_flame" : MAP_BOSS_CRACK_FLAME,
	"metin2_map_Mt_Thunder" : MAP_MT_THUNDER,
	"metin2_map_sungzi_snow_pass02" : MAP_SUNGZI_SNOW_PASS02,
	"metin2_map_sungzi_snow_pass03" : MAP_SUNGZI_SNOW_PASS03,
	"metin2_map_sungzi_snow_pass01" : MAP_SUNGZI_SNOW_PASS01,
	"metin2_map_deviltower1" : MAP_DEVILTOWER1,
	"metin2_map_eastplain_mystery" : MAP_WEAKENDED_MYSTERY_DUNGEON,
	"metin2_map_sungzi_desert_01" : MAP_SUNGZI_DESERT_01,
	"metin2_map_smhgate_snow" : MAP_SUNGMAHEE_GATE_SNOW,
	"metin2_map_empirecastle" : MAP_EMPIRECASTLE,
	"metin2_map_BayBlackSand" : MAP_BAYBLACKSAND,
	"metin2_map_smhgate_flame" : MAP_SUNGMAHEE_GATE_FLAME,
	"metin2_map_devilsCatacomb" : MAP_DEVILCATACOMB,
	"metin2_map_n_desert_01" : MAP_N_DESERT_01,
	"metin2_map_snake_temple_01" : MAP_SNAKE_TEMPLE_1,
	"metin2_map_snake_temple_02" : MAP_SNAKE_TEMPLE_2,
	"metin2_map_n_flame_dragon" : MAP_N_FLAME_DRAGON,
	"metin2_map_anglar_dungeon_01" : MAP_FARMING_DUNGEON,
	"metin2_12zi_stage" : MAP_CZ_DUNGEON,
	"metin2_map_guild_battle_base" : MAP_GUILD_BATTLE_BASE,
	"metin2_map_smhgate_c1" : MAP_SUNGMAHEE_GATE_C1,
	"metin2_map_t2" : MAP_T2,
	"metin2_map_t3" : MAP_T3,
	"metin2_map_t1" : MAP_T1,
	"metin2_map_guild_whitedragon_boss" : MAP_INDEX_GUILD_WHITE_DRAGON,
	"metin2_map_t4" : MAP_T4,
	"metin2_map_WL_01" : MAP_WL_01,
	"metin2_map_boss_crack_dawnmist" : MAP_BOSS_CRACK_DAWNMIST,
	"metin2_map_mists_of_island" : MAP_MISTS_OF_ISLAND,
	"metin2_map_miniboss_01" : MAP_MINIBOSS_01,
	"metin2_map_miniboss_02" : MAP_MINIBOSS_02,
	"metin2_map_sungzi_desert_hill_02" : MAP_SUNGZI_DESERT_HILL_02,
	"metin2_map_sungzi_desert_hill_03" : MAP_SUNGZI_DESERT_HILL_03,
	"metin2_map_n_snow_dungeon_01" : MAP_N_SNOW_DUNGEON_01,
	"metin2_map_sungzi_desert_hill_01" : MAP_SUNGZI_DESERT_HILL_01,
	"metin2_map_trent" : MAP_TRENT,
	"metin2_map_guild_01" : MAP_GUILD_01,
	"metin2_map_guild_02" : MAP_GUILD_02,
	"metin2_map_guild_03" : MAP_GUILD_03,
	"metin2_map_pvp_arena" : MAP_PVP_ARENA,
	"metin2_guild_war4" : GUILD_WAR4,
	"metin2_guild_war1" : GUILD_WAR1,
	"metin2_guild_war3" : GUILD_WAR3,
	"metin2_guild_war2" : GUILD_WAR2,
	"metin2_map_n_flame_dungeon_01" : MAP_N_FLAME_DUNGEON_01,
	"metin2_map_c1" : MAP_C1,
	"metin2_map_oxevent" : MAP_OXEVENT,
	"metin2_map_otherworld_01" : MAP_OTHER_WORLD_01,
	"metin2_map_Mt_Th_dungeon_01" : MAP_MT_TH_DUNGEON_01,
	"metin2_map_monkeydungeon_02" : MAP_MONKEY_DUNGEON2,
	"metin2_map_otherworld_02" : MAP_OTHER_WORLD_02,
	"metin2_map_otherworld_03" : MAP_OTHER_WORLD_03,
	"metin2_map_otherworld_04" : MAP_OTHER_WORLD_04,
	"metin2_map_sungzi_snow" : MAP_SUNGZI_SNOW,
	"metin2_map_sungzi" : MAP_SUNGZI,
	"metin2_map_smhgate_dawnmist" : MAP_SUNGMAHEE_GATE_DAWNMIST,
	"metin2_map_defensewave_port" : MAP_DEFENSE_WAVE_PORT,
	"metin2_map_boss_awaken_dawnmist" : MAP_BOSS_AWAKEN_DAWNMIST,
	"metin2_map_smhgate_desert" : MAP_SUNGMAHEE_GATE_DESERT,
	"metin2_map_guild_summon_nw" : MAP_GUILD_SUMMON,
	"metin2_map_a1" : MAP_A1,
	"metin2_map_a3" : MAP_A3,
	"metin2_map_milgyo" : MAP_MILGYO,
	"metin2_map_duel" : MAP_DUEL,
	"metin2_map_monkey_dungeon_11" : MAP_MONKEY_DUNGEON_11,
	"metin2_map_smhgate_b1" : MAP_SUNGMAHEE_GATE_B1,
	"metin2_map_monkey_dungeon_13" : MAP_MONKEY_DUNGEON_13,
	"metin2_map_monkey_dungeon_12" : MAP_MONKEY_DUNGEON_12,
	"metin2_map_spiderdungeon_02" : MAP_SPIDERDUNGEON_02,
	"metin2_map_spiderdungeon_03" : MAP_SPIDERDUNGEON_03,
	"metin2_map_boss_awaken_snow" : MAP_BOSS_AWAKEN_SNOW,
	"gm_guild_build" : MAP_PATHWAY_ICE_02,
	"metin2_map_privateshop" : MAP_PRIVATE_SHOP_MAP,
	"metin2_map_battlearena01" : MAP_BATTLEARENA_01,
	"metin2_map_nusluck01" : MAP_NUSLUCK01,
	"metin2_map_battlearena02" : MAP_BATTLEARENA_02,
	"metin2_map_battlearena03" : MAP_BATTLEARENA_03,
	"metin2_map_dawnmist_dungeon_01" : MAP_DAWNMIST_DUNGEON_01,
	"map_n_snowm_01" : MAP_N_SNOWM_01,
	"metin2_map_sungzi_flame_hill_03" : MAP_SUNGZI_FLAME_HILL_03,
	"metin2_map_sungzi_flame_hill_02" : MAP_SUNGZI_FLAME_HILL_02,
	"metin2_map_sungzi_flame_hill_01" : MAP_SUNGZI_FLAME_HILL_01,
	"metin2_map_smhgate_threeway" : MAP_SUNGMAHEE_GATE_THREEWAY,
	"metin2_map_maze_dungeon3" : MAP_MAZE_DUNGEON_03,
	"metin2_map_maze_dungeon2" : MAP_MAZE_DUNGEON_02,
	"metin2_map_maze_dungeon1" : MAP_MAZE_DUNGEON_01,
	"metin2_map_defensewave" : MAP_DEFENSE_WAVE,
	"metin2_map_eastplain_01" : MAP_EASTPLAIN_01,
	"metin2_map_eastplain_02" : MAP_EASTPLAIN_02,
	"metin2_map_eastplain_03" : MAP_EASTPLAIN_03,
	"metin2_map_whitedragoncave_02" : MAP_WHITE_DRAGON_CAVE_02,
	"metin2_map_whitedragoncave_01" : MAP_WHITE_DRAGON_CAVE_01,
	"metin2_map_icecrystalcave" : MAP_ICE_CRYSTALS_CAVE,
	"metin2_map_skipia_dungeon_02" : MAP_SKIPIA_DUNGEON_02,
	"metin2_map_skipia_dungeon_01" : MAP_SKIPIA_DUNGEON_01,
	"metin2_map_treasure_hunt" : MAP_TREASURE_ISLAND,
	"metin2_map_golden_land" : MAP_GOLDEN_LAND,
	"metin2_map_golden_land_stage" : MAP_GOLDEN_LAND_STAGE,
	"metin2_map_moonlight" : MAP_MOONLIGHT_VALLEY,
	"metin2_map_guild_battle" : MAP_GUILD_BATTLE,
	"metin2_map_c3" : MAP_C3,
	"metin2_map_elemental_04" : MAP_ELEMENTAL_DUNGEON,
	"metin2_map_elemental_02" : MAP_ELEMENTAL_DUNGEON_FIRE,
	"metin2_map_elemental_03" : MAP_ELEMENTAL_DUNGEON_ELECTRICITY,
	"metin2_map_snakevalley" : MAP_SNAKE_VALLEY,
	"metin2_map_elemental_01" : MAP_ELEMENTAL_DUNGEON_DARK,
	"metin2_map_wedding_01" : MAP_WEDDING_01,
	"metin2_map_boss_crack_snow" : MAP_BOSS_CRACK_SNOW,
	"metin2_map_trent02" : MAP_TRENT02,
	"metin2_map_empirewar01" : MAP_EMPIREWAR01,
	"metin2_map_boss_crack_skipia" : MAP_BOSS_CRACK_SKIPIA,
	"metin2_map_empirewar03" : MAP_EMPIREWAR03,
	"metin2_map_empirewar02" : MAP_EMPIREWAR02,
	"metin2_map_battleroyale" : MAP_BATTLE_ROYALE,
	"metin2_map_whitdragonvalley" : MAP_WHITE_DRAGON_VALLEY,
	"Metin2_map_CapeDragonHead" : MAP_CAPEDRAGONHEAD,
}

if app.WJ_SHOW_PARTY_ON_MINIMAP or app.ENABLE_MONSTER_CARD or app.ENABLE_PARTY_MATCH:
	MINIMAP_ZONE_NAME_DICT_BY_IDX = {
		0 : "",
		1 : MAP_A1,
		3 : MAP_A3,
		4 : MAP_GUILD_01,
		5 : MAP_MONKEY_DUNGEON_11,
		6 : GUILD_VILLAGE_01,
		21 : MAP_B1,
		23 : MAP_B3,
		24 : MAP_GUILD_02,
		25 : MAP_MONKEY_DUNGEON_12,
		26 : GUILD_VILLAGE_02,
		41 : MAP_C1,
		43 : MAP_C3,
		44 : MAP_GUILD_03,
		45 : MAP_MONKEY_DUNGEON_13,
		46 : GUILD_VILLAGE_03,
		61 : MAP_N_SNOWM_01,
		62 : MAP_N_FLAME_01,
		63 : MAP_N_DESERT_01,
		64 : MAP_N_THREEWAY,
		65 : MAP_MILGYO,
		66 : MAP_DEVILTOWER1,
		67 : MAP_TRENT,
		68 : MAP_TRENT02,
		69 : MAP_WL_01,
		70 : MAP_NUSLUCK01,
		71 : MAP_SPIDERDUNGEON_02,
		72 : MAP_SKIPIA_DUNGEON_01,
		73 : MAP_SKIPIA_DUNGEON_02,
		74 : MAP_N_SNOWM_02,
		75 : MAP_N_FLAME_02,
		76 : MAP_N_DESERT_02,
		77 : MAP_A2_1,
		78 : MAP_MILGYO_A,
		79 : MAP_TRENT_A,
		80 : MAP_TRENT02_A,
		81 : MAP_WEDDING_01,
		91 : MAP_E,
		92 : MAP_E,
		93 : MAP_E,
		103 : MAP_T1,
		104 : MAP_SPIDERDUNGEON,
		105 : MAP_T2,
		107 : MAP_MONKEY_DUNGEON,
		108 : MAP_MONKEY_DUNGEON2,
		109 : MAP_MONKEY_DUNGEON3,
		110 : MAP_T3,
		111 : MAP_T4,
		112 : MAP_DUEL,
		113 : MAP_OXEVENT,
		114 : MAP_SUNGZI,
		118 : MAP_SUNGZI_FLAME_HILL_01,
		119 : MAP_SUNGZI_FLAME_HILL_02,
		120 : MAP_SUNGZI_FLAME_HILL_03,
		121 : MAP_SUNGZI_SNOW,
		122 : MAP_SUNGZI_SNOW_PASS01,
		123 : MAP_SUNGZI_SNOW_PASS02,
		124 : MAP_SUNGZI_SNOW_PASS03,
		125 : MAP_SUNGZI_DESERT_01,
		126 : MAP_SUNGZI_DESERT_HILL_01,
		127 : MAP_SUNGZI_DESERT_HILL_02,
		128 : MAP_SUNGZI_DESERT_HILL_03,
		130 : GUILD_WAR1,
		131 : GUILD_WAR2,
		132 : GUILD_WAR3,
		133 : GUILD_WAR4,
		180 : METIN_TEST,
		181 : MAP_EMPIREWAR01,
		182 : MAP_EMPIREWAR02,
		183 : MAP_EMPIREWAR03,
		184 : MAP_SKIPIA_DUNGEON_011,
		185 : MAP_SKIPIA_DUNGEON_021,
		186 : MAP_SKIPIA_DUNGEON_012,
		187 : MAP_SKIPIA_DUNGEON_022,
		188 : MAP_SKIPIA_DUNGEON_013,
		189 : MAP_SKIPIA_DUNGEON_023,
		193 : MAP_SPIDERDUNGEON_02_1,
		194 : MAP_HOLYPLACE_FLAME,
		195 : MAP_PATHWAY_FLAME_01,
		196 : MAP_PATHWAY_FLAME_02,
		197 : MAP_PATHWAY_FLAME_03,
		198 : MAP_HOLYPLACE_ICE,
		199 : MAP_PATHWAY_ICE_01,
		200 : MAP_PATHWAY_ICE_02,
		201 : MAP_PATHWAY_ICE_03,
		202 : MAP_HOLYPLACE_DESERT,
		203 : MAP_PATHWAY_DESERT_01,
		204 : MAP_PATHWAY_DESERT_02,
		205 : MAP_PATHWAY_DESERT_03,
		208 : MAP_SKIPIA_DUNGEON_BOSS,
		209 : MAP_SKIPIA_DUNGEON_BOSS_1,
		210 : MAP_SKIPIA_DUNGEON_BOSS_2,
		211 : MAP_SKIPIA_DUNGEON_BOSS_3,
		216 : MAP_DEVILCATACOMB,
		217 : MAP_SPIDERDUNGEON_03,
		301 : MAP_CAPEDRAGONHEAD,
		302 : MAP_DAWNMISTWOOD,
		303 : MAP_BAYBLACKSAND,
		304 : MAP_MT_THUNDER,
		351 : MAP_N_FLAME_DUNGEON_01,
		352 : MAP_N_SNOW_DUNGEON_01,
		353 : MAP_DAWNMIST_DUNGEON_01,
		354 : MAP_MT_TH_DUNGEON_01,
		355 : MAP_CZ_DUNGEON,
		356 : MAP_N_FLAME_DRAGON,
		357 : MAP_BATTLE_FIELD,
		358 : MAP_DEFENSE_WAVE,
		359 : MAP_DEFENSE_WAVE_PORT,
		360 : MAP_MISTS_OF_ISLAND,
		361 : MAP_MINIBOSS_01,
		362 : MAP_MINIBOSS_02,
		363 : MAP_BOSS_LABYRINTH,
		364 : MAP_BOSS_CRACK_SKIPIA,
		365 : MAP_BOSS_CRACK_FLAME,
		366 : MAP_BOSS_CRACK_SNOW,
		367 : MAP_BOSS_CRACK_DAWNMIST,
		368 : MAP_BOSS_AWAKEN_SKIPIA,
		369 : MAP_BOSS_AWAKEN_FLAME,
		370 : MAP_BOSS_AWAKEN_SNOW,
		371 : MAP_BOSS_AWAKEN_DAWNMIST,
		372 : MAP_GUILD_PVE,
		373 : MAP_EASTPLAIN_01,
		374 : MAP_EMPIRECASTLE,
		375 : MAP_BATTLE_ROYALE,
		376 : MAP_EASTPLAIN_02,
		377 : MAP_EASTPLAIN_03,
		378 : MAP_ELEMENTAL_DUNGEON_DARK,
		379 : MAP_ELEMENTAL_DUNGEON_FIRE,
		380 : MAP_ELEMENTAL_DUNGEON_ELECTRICITY,
		381 : MAP_ELEMENTAL_DUNGEON,
		382 : MAP_MAZE_DUNGEON_01,
		383 : MAP_MAZE_DUNGEON_02,
		384 : MAP_MAZE_DUNGEON_03,
		385 : MAP_SNAKE_VALLEY,
		386 : MAP_SUNGMAHEE_TOWER_WAIT,
		387 : MAP_SUNGMAHEE_TOWER_DUNGEON,
		388 : MAP_ICE_CRYSTALS_CAVE,
		389 : MAP_WHITE_DRAGON_VALLEY,
		390 : MAP_SNAKE_TEMPLE_1,
		391 : MAP_SNAKE_TEMPLE_2,
		392 : MAP_PRIVATE_SHOP_MAP,
		393 : MAP_WHITE_DRAGON_CAVE_01,
		394 : MAP_WHITE_DRAGON_CAVE_02,
		395 : MAP_WHITE_DRAGON_CAVE_BOSS,
		396 : MAP_FARMING_DUNGEON,
		399 : MAP_SECRET_DUNGEON,
		400 : MAP_OTHER_WORLD_02,
		401 : MAP_OTHER_WORLD_01,
		402 : MAP_OTHER_WORLD_03,
		403 : MAP_OTHER_WORLD_04,
		404 : MAP_SUNGMAHEE_GATE_A1,
		405 : MAP_SUNGMAHEE_GATE_B1,
		406 : MAP_SUNGMAHEE_GATE_C1,
		407 : MAP_SUNGMAHEE_GATE_THREEWAY,
		408 : MAP_SUNGMAHEE_GATE_DESERT,
		409 : MAP_SUNGMAHEE_GATE_FLAME,
		410 : MAP_SUNGMAHEE_GATE_SNOW,
		411 : MAP_SUNGMAHEE_GATE_DEVILS,
		412 : MAP_SUNGMAHEE_GATE_DAWNMIST,
		413 : MAP_GUILD_BATTLE_BASE,
		414 : MAP_GUILD_BATTLE,
		415 : MAP_WEAKENDED_MYSTERY_DUNGEON,
		417 : MAP_GOLDEN_LAND,
		418 : MAP_GOLDEN_LAND_STAGE,
		419 : MAP_TREASURE_ISLAND,
		420 : MAP_INDEX_GUILD_WHITE_DRAGON_PASS,
		421 : MAP_MOONLIGHT_VALLEY,
		422 : MAP_MOONLIGHT_VALLEY_BOOS_ROOM,
		423 : MAP_INDEX_GREEDY_CAVE,
		424 : MAP_INDEX_GREEDY_ROOM,
		426 : MAP_GUILD_SUMMON,
		427 : MAP_GUILD_SUMMON,
		428 : MAP_N_FLAME_DRAGON,
		429 : MAP_INDEX_GUILD_WHITE_DRAGON
	}
	#if background.IsMapInfoByMapName("metin2_map_dawnmist_dungeon_01"):
	#	MINIMAP_ZONE_NAME_DICT_BY_IDX[353] = MAP_DAWNMIST_DUNGEON_01
	#if background.IsMapInfoByMapName("metin2_map_Mt_Th_dungeon_01"):
	#	MINIMAP_ZONE_NAME_DICT_BY_IDX[354] = MAP_MT_TH_DUNGEON_01
	#if background.IsMapInfoByMapName("metin2_map_n_flame_dragon"):
	#	MINIMAP_ZONE_NAME_DICT_BY_IDX[356] = MAP_N_FLAME_DRAGON
	#if background.IsMapInfoByMapName("metin2_map_battlefied"):
	#	MINIMAP_ZONE_NAME_DICT_BY_IDX[357] = MAP_BATTLE_FIELD
	#if background.IsMapInfoByMapName("metin2_12zi_stage"):
	#	MINIMAP_ZONE_NAME_DICT_BY_IDX[355] = MAP_CZ_DUNGEON

# JOB_TITLE
if app.ENABLE_WOLFMAN_CHARACTER:
	JOBINFO_TITLE = [
		[JOB_WARRIOR0, JOB_WARRIOR1, JOB_WARRIOR2,],
		[JOB_ASSASSIN0, JOB_ASSASSIN1, JOB_ASSASSIN2,],
		[JOB_SURA0, JOB_SURA1, JOB_SURA2,],
		[JOB_SHAMAN0, JOB_SHAMAN1, JOB_SHAMAN2,],
		[JOB_WOLFMAN0, JOB_WOLFMAN1, JOB_WOLFMAN1,],
	]
else:
	JOBINFO_TITLE = [
		[JOB_WARRIOR0, JOB_WARRIOR1, JOB_WARRIOR2,],
		[JOB_ASSASSIN0, JOB_ASSASSIN1, JOB_ASSASSIN2,],
		[JOB_SURA0, JOB_SURA1, JOB_SURA2,],
		[JOB_SHAMAN0, JOB_SHAMAN1, JOB_SHAMAN2,],
	]


JOBINFO_DATA_LIST = [
	[
		["Å¸°í³­ ¿ë¸Í°ú ±ÁÈ÷Áö ¾Ê´Â ¹«»çÀÇ",
		"±â°³¸¦ »ç¶÷µéÀº ÀÏÄÃ¾î [¿ëÀÚ]¶ó°í",
		"ºÎ¸¥´Ù. ¾î¶°ÇÑ À§±â¿¡¼­µµ ±×µéÀº ",
		"µÚ·Î ¹°·¯¼­Áö ¾ÊÀ¸¸ç, ´ÙÄ¡°í ¿òÁ÷",
		"ÀÌ±â Èûµç µ¿·á¸¦ À§ÇØ ´Ü½ÅÀ¸·Î",
		"Àûµé°ú ¸¶ÁÖ ½Î¿ì±âµµ ÇÑ´Ù. ÀÌµéÀº",
		"Àß ´Ü·ÃµÈ ±ÙÀ°°ú Èû, °­·ÂÇÑ °ø°İ·Â",
		"À¸·Î ÀüÀå ÃÖ¼±µÎ¿¡¼­ °ø°İÁøÀ¸·Î",
		"È°¾àÇÑ´Ù.                      ",],
		["°¡Àå ÀÏ¹İÀûÀÎ °ø°İÇü ¹«»ç·Î, ",
		"ÀûÁ¢Àü¿¡ µû¸¥ Á÷Á¢ °ø°İÀ¸·Î ÀüÀå",
		"¿¡¼­ È°¾àÇÑ´Ù. ±ºÁ÷ Æ¯¼º»ó ±Ù·ÂÀ»",
		"¸ŞÀÎÀ¸·Î ½ºÅİ Æ÷ÀÎÆ®¸¦ ÅõÀÚÇÏµÇ, ",
		"ÀûÁ¢Àü¿¡ µû¸¥ »ı¸í·Â / ¹æ¾î·Â",
		"È®º¸¸¦ À§ÇØ Ã¼·ÂÀ» ¿Ã¸°´Ù. ¶ÇÇÑ",
		"°ø°İÀÇ Á¤È®¼ºÀ» ³ôÀÌ±â À§ÇØ ¹ÎÃ¸",
		"¿¡µµ Æ÷ÀÎÆ®¸¦ ÅõÀÚÇÒ ÇÊ¿ä°¡ ÀÖ´Ù.",],
		["»ó´ç ¼öÁØÀÇ Á¤½Å·ÂÀ» ÀÌ¿ëÇÏ´Â",
		"Áß/±Ù°Å¸® Á¢ÀüÇü ¹«»ç·Î, °¢ ±â¼ú",
		"ÇÏ³ªÇÏ³ªÀÇ ³ôÀº °ø°İ·ÂÀ¸·Î ÀüÀå¿¡¼­",
		"È°¾àÇÑ´Ù. ±ºÁ÷ Æ¯¼º»ó ±Ù·ÂÀ» ¸ŞÀÎ",
		"À¸·Î ½ºÅÈ Æ÷ÀÎÆ®¸¦ ÅõÀÚÇÏµÇ, ",
		"Áß/±Ù°Å¸® °ø°İÀÇ Á¤È®¼º°ú ¸íÁß·üÀ»",
		"À§ÇØ ¹ÎÃ¸À» ¿Ã¸°´Ù. ¶ÇÇÑ Á¢Àü ½Ã ",
		"Àû °ø°İ¿¡ µû¸¥ »ı¸í·Â / ¹æ¾î·Â",
		"È®º¸¸¦ À§ÇØ Ã¼·Â¿¡µµ Æ÷ÀÎÆ®¸¦",
		"ÅõÀÚÇÒ ÇÊ¿ä°¡ ÀÖ´Ù.        ",],
	],
	[
		["ÀÚ°´Àº ¾î¶°ÇÑ »óÈ²¿¡¼­µµ ÀÚ½ÅÀÇ",
		"¸öÀ» ¼û±â°í Àº¹ĞÇÑ ¾îµÒÀÇ ÀÓ¹«¸¦",
		"¼öÇàÇÏ¸é¼­ ÀüÀåÀÇ ÈÄÀ§¸¦ Áö¿øÇÏ´Â", 
		"ÀÚµéÀÌ´Ù. ÀÌµéÀº ¾ÆÁÖ ºü¸£°í ½Å¼Ó",
		"ÇÏ¸ç, ºñÇÒ µ¥ ¾øÀÌ °ú°¨ÇÏ°í ÀıÁ¦µÈ",
		"Çàµ¿À¸·Î ÀûÀÇ ±Ş¼Ò¿¡ Ä¡¸íÅ¸¸¦ ³¯¸®",
		"µÇ, ÀüÀå¿¡¼± ÀûÁøÀ» ÇâÇØ ¹«¼öÇÑ",
		"È­»ìÀ» ³»»ÕÀ¸¸ç ÀÚ½ÅÀÇ ¿ë¸ÍÀ»",
		"¼±º¸ÀÎ´Ù.                   "],
		["µÎ¼Õ ´Ü°ËÀ» ÁÖ¹«±â·Î ´Ù·ç¸ç, ½Å¼Ó",
		"ÇÏ°Ô Ä¡°í ºüÁö´Â ÀÚ°´ Æ¯À¯ÀÇ ¿òÁ÷ÀÓ",
		"À¸·Î ÀüÀå¿¡¼­ È°¾àÇÑ´Ù. ±ºÁ÷ Æ¯¼º»ó",
		"¹ÎÃ¸À» ¸ŞÀÎÀ¸·Î ½ºÅİ Æ÷ÀÎÆ®¸¦ ÅõÀÚ",
		"ÇÏµÇ, ±Ù·ÂÀ» ¿Ã·Á °ø°İ·ÂÀ» ³ôÀÎ´Ù.",
		"¶ÇÇÑ ±ÙÁ¢Àü¿¡ µû¸¥ »ı¸í·Â/¹æ¾î·Â ",
		"»ó½ÂÀ» À§ÇØ Ã¼·Â¿¡µµ Æ÷ÀÎÆ®¸¦",
		"ÅõÀÚÇÒ ÇÊ¿ä°¡ ÀÖ´Ù.          ",],
		["È°À» ÁÖ¹«±â·Î ´Ù·ç¸ç, ±ä ½Ã¾ß¿Í",
		"»çÁ¤°Å¸®¿¡ µû¸¥ ¿ø°Å¸® °ø°İÀ¸·Î",
		"ÀüÀå¿¡¼­ È°¾àÇÑ´Ù. ±ºÁ÷ Æ¯¼º»ó",
		"°ø°İ ¼º°ø·üÀÇ Áõ°¡¸¦ À§ÇØ ¹ÎÃ¸À»",
		"¸ŞÀÎÀ¸·Î ¿Ã·Á¾ß ÇÏ¸ç, ¿ø°Å¸®",
		"°ø°İÀÇ µ¥¹ÌÁö Áõ°¡¸¦ À§ÇØ ±Ù·ÂÀ»",
		"¿Ã¸± ÇÊ¿ä°¡ ÀÖ´Ù. ¶ÇÇÑ Àûµé¿¡°Ô",
		"Æ÷À§µÇ¾úÀ» ½Ã, Àû °ø°İ¿¡ ¹öÆ¼±â",
		"À§ÇÑ »ı¸í·Â/¹æ¾î·Â »ó½ÂÀ» À§ÇØ",
		"Ã¼·Â¿¡µµ Æ÷ÀÎÆ®¸¦ ÅõÀÚÇÒ ÇÊ¿ä°¡",
		"ÀÖ´Ù.                        ", ],
	],
	[
		["¼ö¶ó´Â [µ¶Àº µ¶À¸·Î]ÀÇ ¼Ó¼ºÀ¸·Î",
		"Ã¢¼³µÈ Æ¯¼ö ¼Ó¼ºÀÇ ±ºÁ÷ÀÌ´Ù. ",
		"±×µéÀº ÀüÀå¿¡¼­ ÀûµéÀÇ »ç±â¸¦ ÀúÇÏ",
		"½ÃÅ°°í, ¾Ç¸¶ÀÇ ÈûÀ» ½ÇÀº ¸¶ÅºÀ¸·Î",
		"ÀûÀÇ ¿µÈ¥°ú À°½ÅÀ» Áş¹¶°µ´Ù. ¶§·Î",
		"ÀÌµéÀº ÀÚ½ÅÀÇ °Ë°ú °©¿Ê¿¡ ¾îµÒÀÇ",
		"ÈûÀ» ½Ç¾î, ÀüÀå¿¡¼­ ¹«»ç ¸øÁö ¾ÊÀº",
		"°ø°İ·ÂÀ» ¹ßÈÖÇÏ±âµµ ÇÏ´Âµ¥, ÀûµéÀ»",
		"Á×¿©´ë´Â±× ¸ğ½ÀÀÌ ¿ö³«¿¡ ²ûÂïÇØ",
		"»ç¶÷µéÀº ¼ö¶ó¸¦ ÀÏÄÃ¾î [¸¶½Å]ÀÌ¶ó",
		"ºÎ¸£±â¸¦ ÁÖÀú ¾É´Â´Ù."],
		["È¯¹«±ºÀÇ ¼ö¶ó´Â ¾Ç¸¶ÀÇ ¾¾¿¡¼­",
		"¾ò¾îÁö´Â ¸¶·ÂÀ» ¹«±â³ª ¹æ¾î±¸¿¡",
		"½Ç¾î ¹«»ç ¸øÁö ¾ÊÀº ÀüÅõ·ÂÀ¸·Î",
		"ÀüÀå¿¡¼­ È°¾àÇÑ´Ù. ±ºÁ÷ Æ¯¼º»ó",
		"Áö´ÉÀÌ ³ô¾ÆÁú¼ö·Ï Âø¿ë Àåºñ¿¡", 
		"½Ç¸®´Â ¸¶·ÂÀÇ À§·ÂÀÌ Áõ´ëµÇ¹Ç·Î,",
		"Áö´É°ú ±Ù·ÂÀ» ¸ŞÀÎÀ¸·Î ½ºÅÈ",
		"Æ÷ÀÎÆ®¸¦ ÅõÀÚÇÏµÇ, Á¢Àü¿¡ µû¸¥",
		"»ı¸í·Â/¹æ¾î·Â È®º¸¸¦ À§ÇØ Ã¼·ÂÀ»",
		"¿Ã¸°´Ù. ¶ÇÇÑ °ø°İÀÇ Á¤È®¼º°ú",
		"È¸ÇÇ¸¦ À§ÇØ¼­ ¹ÎÃ¸¿¡µµ Æ÷ÀÎÆ®¸¦",
		"ÅõÀÚÇÒ ÇÊ¿ä°¡ ÀÖ´Ù.           ",],
		["Èæ¸¶±ºÀÇ ¼ö¶óµéÀº °¢Á¾ ¾îµÒÀÇ",
		"ÁÖ¹®°ú ¾Ç¸¶ÀÇ ¸¶¹ıÀ¸·Î ÀüÀå¿¡¼­",
		"È°¾àÇÑ´Ù. ±ºÁ÷ Æ¯¼º»ó ¸¶¹ı °ø°İÀÌ",
		"ÁÖÀÌ¹Ç·Î Áö´ÉÀ» ¸ŞÀÎÀ¸·Î ½ºÅİ",
		"Æ÷ÀÎÆ®¸¦ ÅõÀÚÇÏµÇ, ¿ø°Å¸® ¸¶¹ı",
		"°ø°İÀÇ Á¤È®¼ºÀ» À§ÇØ ¹ÎÃ¸À» ¿Ã¸°´Ù.",
		"¶ÇÇÑ Æ÷À§ µÇ¾úÀ»½Ã, Àû °ø°İ¿¡ µû¸¥",
		"»ı¸í·Â / ¹æ¾î·Â È®º¸¸¦ À§ÇØ Ã¼·Â¿¡µµ",
		"Æ÷ÀÎÆ®¸¦ ÅõÀÚÇÒ ÇÊ¿ä°¡ ÀÖ´Ù.    ",],
	],
	[
		["¹«´çÀº ¿ë½Å°ú ÀÚ¿¬, µÎ °í´ëÀÇ",
		"ÈûÀ» ´Ù·ê ¼ö ÀÖ´Â À¯ÀÏÇÑ Á÷Á¾ÀÌ´Ù.",
		"±×µéÀº ÈÄ¹æ¿¡¼­ ¾Æ±ºÀ» º¸Á¶ÇÏ°í",
		"´ÙÄ£ µ¿·áÀÇ ºÎ»óÀ» È¸º¹ ½ÃÅ°¸ç",
		"¶³¾îÁø »ç±â¸¦ »ó½Â½ÃÅ²´Ù. ±×µéÀº",
		"¾Æ±ºÀÇ ¼ö¸é°ú ÈŞ½ÄÀ» ¹æÇØÇÏ´Â ÀÚ¸¦ ",
		"Àı´ë ¿ë¼­ÇÏÁö ¾ÊÀ¸¸ç, ±×·± ÀÚµé",
		"¿¡°Ô´Â ÇÑ Á¡ ÁÖÀú ¾øÀÌ ÁÖ¹®À»",
		"ÅÍÆ®·Á ±× ºñ°ÌÇÔÀ» ¾öÈ÷ Â¡°èÇÑ´Ù.",],
		["Ãµ·æ±ºÀÇ ¹«´çµéÀº °¢Á¾ ºÎÀû¼ú°ú",
		"º¸Á¶ÁÖ¹®¿¡ ´ÉÇÏ¸ç, ÀûÀÇ Á÷ / °£Á¢",
		"°ø°İÀ¸·ÎºÎÅÍ ¾Æ±ºÀ» ÁöÅ²´Ù. ±ºÁ÷",
		"Æ¯¼º»ó ¸¶¹ı ´É·ÂÀÌ ÁÖÀÌ¹Ç·Î Áö´ÉÀ»",
		"¸ŞÀÎÀ¸·Î ½ºÅİ Æ÷ÀÎÆ®¸¦ ÅõÀÚÇÏµÇ,",
		"Æ÷À§µÇ¾úÀ» ½Ã, Àû °ø°İ¿¡ µû¸¥",
		"»ı¸í·Â / ¹æ¾î·Â È®º¸¸¦ À§ÇØ Ã¼·ÂÀ»",
		"¿Ã¸°´Ù. ¶ÇÇÑ ¿ø°Å¸® ¸¶¹ı °ø°İÀÇ",
		"Á¤È®¼ºÀ» À§¿¡ ¹ÎÃ¸¿¡µµ Æ÷ÀÎÆ®¸¦",
		"ÅõÀÚÇÒ ÇÊ¿ä°¡ ÀÖ´Ù.           ",],
		["±¤·Ú±ºÀÇ ¹«´çµéÀº ÀÚ¿¬ÀÇ ÈûÀ»",
		"ºô·Á ¾Æ±ºÀ» È¸º¹ÇÏ°í, ³ú½ÅÀÇ ",
		"ÈûÀ¸·Î ¹ĞÁıÇÑ Àûµé¿¡°Ô Å« Ãæ°İÀ»",
		"ÀÔÈú ¼ö ÀÖ´Â ÀÌµéÀÌ´Ù. ±ºÁ÷ÀÇ",
		"Æ¯¼º»ó ¸¶¹ı ´É·ÂÀÌ ÁÖÀÌ¹Ç·Î Áö´ÉÀ»",
		"¸ŞÀÎÀ¸·Î ½ºÅİ Æ÷ÀÎÆ®¸¦ ÅõÀÚÇÏµÇ,",
		"Æ÷À§µÇ¾úÀ»½Ã, Àû °ø°İ¿¡ µû¸¥",
		"»ı¸í·Â / ¹æ¾î·Â È®º¸¸¦ À§ÇØ Ã¼·ÂÀ»",
		"¿Ã¸°´Ù. ¶ÇÇÑ ¿ø°Å¸® ¸¶¹ı °ø°İÀÇ",
		"Á¤È®¼ºÀ» À§¿¡ ¹ÎÃ¸¿¡µµ Æ÷ÀÎÆ®¸¦",
		"ÅõÀÚÇÒ ÇÊ¿ä°¡ ÀÖ´Ù.             "],
	],
	[
		["¼öÀÎ",],
		["¼öÀÎÀº [ºùÀÇ¿Í Äè]ÀÇ ¼Ó¼ºÀ¸·Î",
		"Ã¢¼³ÀÌ µÈ Æ¯¼ö ±ºÁ÷ÀÌ´Ù.",
		"±×µéÀº ÀüÀå¿¡¼­ ¼±ºÀ¿¡ ¼­¸ç, ¸¶¹°µéÀÇ",
		"»ç±â¸¦ ÀúÇÏ½ÃÅ°°í À°½ÅÀ» Âõ¾î ¹ß±ä´Ù.",
		"¶§·Î´Â ÀÚ½ÅµéÀÇ ¸ö¿¡ ´Á´ëÀÇ ¿µÈ¥À» ÀÌ¾î",
		"¹Ş¾Æ, ÀüÀå¿¡¼­ ¹«»ç¸¦ ÃÊ¿ùÇÏ´Â ¼ø°£ÀûÀÎ",
		"°ø°İ·ÂÀ» ¹ßÃëÇÏ±âµµ ÇÏ´Âµ¥, ÀûµéÀº ±×",
		"¸ğ½ÀÀÌ Èí»ç [´ë¶û]À» ´à¾Æ [»ç½Å]ÀÌ¶ó",
		"ºÎ¸£±âµµ ÇÑ´Ù."],
		["ÇØ´ç¾øÀ½"],
	],
]


WHISPER_ERROR = {
	1 : CANNOT_WHISPER_NOT_LOGON,
	2 : CANNOT_WHISPER_DEST_REFUSE,
	3 : CANNOT_WHISPER_SELF_REFUSE,
}

NOTIFY_MESSAGE = {
	"CANNOT_EQUIP_SHOP" : CANNOT_EQUIP_IN_SHOP,
	"CANNOT_EQUIP_EXCHANGE" : CANNOT_EQUIP_IN_EXCHANGE,
}

ATTACK_ERROR_TAIL_DICT = {
	"IN_SAFE" : CANNOT_ATTACK_SELF_IN_SAFE,
	"DEST_IN_SAFE" : CANNOT_ATTACK_DEST_IN_SAFE,
}

SHOT_ERROR_TAIL_DICT = {
	"EMPTY_ARROW" : CANNOT_SHOOT_EMPTY_ARROW,
	"IN_SAFE" : CANNOT_SHOOT_SELF_IN_SAFE,
	"DEST_IN_SAFE" : CANNOT_SHOOT_DEST_IN_SAFE,
}

USE_SKILL_ERROR_TAIL_DICT = {
	"IN_SAFE" : CANNOT_SKILL_SELF_IN_SAFE,
	"NEED_TARGET" : CANNOT_SKILL_NEED_TARGET,
	"NEED_EMPTY_BOTTLE" : CANNOT_SKILL_NEED_EMPTY_BOTTLE,
	"NEED_POISON_BOTTLE" : CANNOT_SKILL_NEED_POISON_BOTTLE,
	"REMOVE_FISHING_ROD" : CANNOT_SKILL_REMOVE_FISHING_ROD,
	"NOT_YET_LEARN" : CANNOT_SKILL_NOT_YET_LEARN,
	"NOT_MATCHABLE_WEAPON" : CANNOT_SKILL_NOT_MATCHABLE_WEAPON,
	"WAIT_COOLTIME" : CANNOT_SKILL_WAIT_COOLTIME,
	"NOT_ENOUGH_HP" : CANNOT_SKILL_NOT_ENOUGH_HP,
	"NOT_ENOUGH_SP" : CANNOT_SKILL_NOT_ENOUGH_SP,
	"CANNOT_USE_SELF" : CANNOT_SKILL_USE_SELF,
	"ONLY_FOR_ALLIANCE" : CANNOT_SKILL_ONLY_FOR_ALLIANCE,
	"CANNOT_ATTACK_ENEMY_IN_SAFE_AREA" : CANNOT_SKILL_DEST_IN_SAFE,
	"CANNOT_APPROACH" : CANNOT_SKILL_APPROACH,
	"CANNOT_ATTACK" : CANNOT_SKILL_ATTACK,
	"ONLY_FOR_CORPSE" : CANNOT_SKILL_ONLY_FOR_CORPSE,
	"EQUIP_FISHING_ROD" : CANNOT_SKILL_EQUIP_FISHING_ROD,
	"NOT_HORSE_SKILL" : CANNOT_SKILL_NOT_HORSE_SKILL,
	"HAVE_TO_RIDE" : CANNOT_SKILL_HAVE_TO_RIDE,
}

if app.ENABLE_MOUNT_UPGRADE_SYSTEM:
	LEVEL_LIST = ["", HORSE_LEVEL1, HORSE_LEVEL2, HORSE_LEVEL3, HORSE_LEVEL4]
else:
	LEVEL_LIST = ["", HORSE_LEVEL1, HORSE_LEVEL2, HORSE_LEVEL3]

HEALTH_LIST = [
	HORSE_HEALTH0,
	HORSE_HEALTH1,
	HORSE_HEALTH2,
	HORSE_HEALTH3,
]

USE_SKILL_ERROR_CHAT_DICT = {
	"NEED_EMPTY_BOTTLE" : SKILL_NEED_EMPTY_BOTTLE,
	"NEED_POISON_BOTTLE" : SKILL_NEED_POISON_BOTTLE,
	"ONLY_FOR_GUILD_WAR" : SKILL_ONLY_FOR_GUILD_WAR,
}

SHOP_ERROR_DICT = {
	"NOT_ENOUGH_MONEY" : SHOP_NOT_ENOUGH_MONEY,
	"SOLDOUT" : SHOP_SOLDOUT,
	"INVENTORY_FULL" : SHOP_INVENTORY_FULL,
	"INVALID_POS" : SHOP_INVALID_POS,
	"NOT_ENOUGH_MONEY_EX" : SHOP_NOT_ENOUGH_MONEY_EX,
}

if app.ENABLE_SHOPEX_RENEWAL:
	SHOP_ERROR_DICT.update({"NOT_ENOUGH_ITEM": SHOP_NOT_ENOUGH_ITEM,})

STAT_MINUS_DESCRIPTION = {
	"HTH-" : STAT_MINUS_CON,
	"INT-" : STAT_MINUS_INT,
	"STR-" : STAT_MINUS_STR,
	"DEX-" : STAT_MINUS_DEX,
}

MODE_NAME_LIST = (PVP_OPTION_NORMAL, PVP_OPTION_REVENGE, PVP_OPTION_KILL, PVP_OPTION_PROTECT,)
TITLE_NAME_LIST = (PVP_LEVEL0, PVP_LEVEL1, PVP_LEVEL2, PVP_LEVEL3, PVP_LEVEL4, PVP_LEVEL5, PVP_LEVEL6, PVP_LEVEL7, PVP_LEVEL8,)

if app.ENABLE_GUILD_LEADER_GRADE_NAME:
	GUILD_LEADER_GRADE_NAME_LIST = (GUILD_LEADER_GRADE0, GUILD_LEADER_GRADE1)

def GetLetterImageName():
	return "icon/item/scroll_close.tga"
def GetLetterOpenImageName():
	return "icon/item/scroll_open.tga"
def GetLetterCloseImageName():
	return "icon/item/scroll_close.tga"

def GetBlueLetterImageName():
	return "icon/item/scroll_close_blue.tga"
def GetBlueLetterOpenImageName():
	return "icon/item/scroll_open_blue.tga"
def GetBlueLetterCloseImageName():
	return "icon/item/scroll_close_blue.tga"

if 949 == app.GetDefaultCodePage():
	def EUL(name):
		if GetAuxiliaryWordType(name):
			return "¸¦ "
		else:
			return "À» "

	def I(name):
		if GetAuxiliaryWordType(name):
			return "°¡ "
		else:
			return "ÀÌ "

	def DO_YOU_SELL_ITEM(sellItemName, sellItemCount, sellItemPrice):
		name = sellItemName
		if sellItemCount > 1:
			name += " "
			name += str(sellItemCount)
			name += "°³"

		return name + EUL(name) + str(sellItemPrice) + "³É¿¡ ÆÄ½Ã°Ú½À´Ï±î?"

	if app.ENABLE_CHEQUE_SYSTEM:
		def DO_YOU_BUY_ITEM(buyItemName, buyItemCount, buyItemPrice, sellItemCheque = 0):
			if sellItemCheque > 0:
				if buyItemCount > 1 :
					return DO_YOU_BUY_ITEM4 % ( buyItemName, buyItemCount, sellItemCheque, buyItemPrice )
				else:
					return DO_YOU_BUY_ITEM3 % ( buyItemName, sellItemCheque, buyItemPrice )
			else:
				if buyItemCount > 1 :
					return DO_YOU_BUY_ITEM2 % ( buyItemName, buyItemCount, buyItemPrice )
				else:
					return DO_YOU_BUY_ITEM1 % ( buyItemName, buyItemPrice )
	else:
		def DO_YOU_BUY_ITEM(sellItemName, sellItemCount, sellItemPrice):
			name = sellItemName
			if sellItemCount > 1:
				name += " "
				name += str(sellItemCount)
				name += "°³"

			return name + EUL(name) + str(sellItemPrice) + "¿¡ »ç½Ã°Ú½À´Ï±î?"

	#def REFINE_FAILURE_CAN_NOT_ATTACH(attachedItemName):
	#	return attachedItemName+EUL(attachedItemName)+"ºÎÂøÇÒ ¼ö ¾ø´Â ¾ÆÀÌÅÛÀÔ´Ï´Ù"

	def REFINE_FAILURE_NO_SOCKET(attachedItemName):
		return attachedItemName+EUL(attachedItemName)+"ºÎÂøÇÒ ¼ö ÀÖ´Â ¼ÒÄÏÀÌ ¾ø½À´Ï´Ù"

	def REFINE_FAILURE_NO_GOLD_SOCKET(attachedItemName):
		return attachedItemName+EUL(attachedItemName)+"ºÎÂøÇÒ ¼ö ÀÖ´Â È²±İ ¼ÒÄÏÀÌ ¾ø½À´Ï´Ù"

	def HOW_MANY_ITEM_DO_YOU_DROP(dropItemName, dropItemCount):
		name = dropItemName
		if dropItemCount > 1:
			name += " "
			name += str(dropItemCount)
			name += "°³"

		return name + EUL(name) + "¹ö¸®½Ã°Ú½À´Ï±î?"

	def NumberToMoneyString(n):
		if app.ENABLE_CHEQUE_SYSTEM:
			if n <= 0 :
				return "0"

			return "%s" % ('.'.join([ i-3<0 and str(n)[:i] or str(n)[i-3:i] for i in range(len(str(n))%3, len(str(n))+1, 3) if i ]))
		else:
			if n <= 0:
				return "0³É"

			n = str(n)
			result = CutMoneyString(n, 0, 4, "", "")
			result = CutMoneyString(n, 4, 8, "¸¸", result)
			result = CutMoneyString(n, 8, 12, "¾ï", result)
			result = result + "³É"

			return result

	def NumberToSecondaryCoinString(number):
		if number <= 0:
			return "0Àü"

		number = str(number)
		result = CutMoneyString(number, 0, 4, "", "")
		result = CutMoneyString(number, 4, 8, "¸¸", result)
		result = CutMoneyString(number, 8, 12, "¾ï", result)
		result = result + "Àü"

		return result

	def FISHING_NOTIFY(isFish, fishName):
		if isFish:
			return fishName + I(fishName) + "¹® µí ÇÕ´Ï´Ù."
		else:
			return fishName + I(fishName) + "°É¸°µí ÇÕ´Ï´Ù."

	def FISHING_SUCCESS(isFish, fishName):
		if isFish:
			return fishName + EUL(fishName) + "Àâ¾Ò½À´Ï´Ù!"
		else:
			return fishName + EUL(fishName) + "¾ò¾ú½À´Ï´Ù!"

elif 932 == app.GetDefaultCodePage():
	def DO_YOU_SELL_ITEM(sellItemName, sellItemCount, sellItemPrice):
		if sellItemCount > 1 :
			return "%s %s ŒÂ‚ğ %s‚É”„‚è‚Ü‚·‚©H" % ( sellItemName, sellItemCount, NumberToMoneyString(sellItemPrice) )
		else:
			return "%s ‚ğ %s‚Å”„‚è‚Ü‚·‚©H" % (sellItemName, NumberToMoneyString(sellItemPrice) )

	def DO_YOU_BUY_ITEM(buyItemName, buyItemCount, buyItemPrice) :
		if buyItemCount > 1 :
			return "%s %sŒÂ‚ğ %s‚Å”ƒ‚¢‚Ü‚·‚©H" % ( buyItemName, buyItemCount, buyItemPrice )
		else:
			return "%s‚ğ %s‚Å”ƒ‚¢‚Ü‚·‚©H" % ( buyItemName, buyItemPrice )

	#def REFINE_FAILURE_CAN_NOT_ATTACH(attachedItemName):
	#	return "%s‚ğ‘•’…‚Å‚«‚È‚¢ƒAƒCƒe?‚Å‚·B" % (attachedItemName)

	def REFINE_FAILURE_NO_SOCKET(attachedItemName) :
		return "%s‚ğ‘•’…‚·‚é?ƒPƒbƒg‚ª‚ ‚è‚Ü‚¹‚ñB" % (attachedItemName)

	def REFINE_FAILURE_NO_GOLD_SOCKET(attachedItemName) :
		return "%s‚ğ‘•’…‚Å‚«‚é‰©‹à?ƒPƒbƒg‚ª‚ ‚è‚Ü‚¹‚ñB" % (attachedItemName)

	def HOW_MANY_ITEM_DO_YOU_DROP(dropItemName, dropItemCount) :
		if dropItemCount > 1 :
			return "%s %d ŒÂ‚ğÌ‚Ä‚Ü‚·‚©H" % (dropItemName, dropItemCount)
		else :
			return "%s‚ğÌ‚Ä‚Ü‚·‚©H" % (dropItemName)

	def FISHING_NOTIFY(isFish, fishName) :
		if isFish :
			return "%s ‚ªH‚¢‚Â‚¢‚½‚æ‚¤‚Å‚·" % ( fishName )
		else :
			return "%s ‚ª‚©‚©‚Á‚½‚æ‚¤‚Å‚·" % ( fishName )

	def FISHING_SUCCESS(isFish, fishName) :
		if isFish :
			return "%s ‚ğ•ß‚Ü‚¦‚Ü‚µ‚½I" % (fishName)
		else :
			return "%s ‚ğè‚É“ü‚ê‚Ü‚µ‚½I" % (fishName)

	def NumberToMoneyString(number) :
		if number <= 0 :
			return "0—¼"

		number = str(number)
		result = CutMoneyString(number, 0, 4, "", "")
		result = CutMoneyString(number, 4, 8, "–œ", result)
		result = CutMoneyString(number, 8, 12, "‰­", result)
		result = result + "—¼"

		return result

	def NumberToSecondaryCoinString(number):
		if number <= 0:
			return "0jun"

		number = str(number)
		result = CutMoneyString(number, 0, 4, "", "")
		result = CutMoneyString(number, 4, 8, "–œ", result)
		result = CutMoneyString(number, 8, 12, "‰­", result)
		result = result + "jun"

		return result

elif IsHONGKONG():
	def DO_YOU_SELL_ITEM(sellItemName, sellItemCount, sellItemPrice):
		if sellItemCount > 1 :
			return DO_YOU_SELL_ITEM2 % (sellItemName, sellItemCount, NumberToMoneyString(sellItemPrice) )
		else:
			return DO_YOU_SELL_ITEM1 % (sellItemName, NumberToMoneyString(sellItemPrice) )

	def DO_YOU_BUY_ITEM(buyItemName, buyItemCount, buyItemPrice) :
		if buyItemCount > 1 :
			return DO_YOU_BUY_ITEM2 % ( buyItemName, buyItemCount, buyItemPrice )
		else:
			return DO_YOU_BUY_ITEM1 % ( buyItemName, buyItemPrice )

	#def REFINE_FAILURE_CAN_NOT_ATTACH(attachedItemName):
	#	return REFINE_FAILURE_CAN_NOT_ATTACH0 % (attachedItemName)

	def REFINE_FAILURE_NO_SOCKET(attachedItemName) :
		return REFINE_FAILURE_NO_SOCKET0 % (attachedItemName)

	def REFINE_FAILURE_NO_GOLD_SOCKET(attachedItemName) :
		return REFINE_FAILURE_NO_GOLD_SOCKET0 % (attachedItemName)

	def HOW_MANY_ITEM_DO_YOU_DROP(dropItemName, dropItemCount) :
		if dropItemCount > 1 :
			return HOW_MANY_ITEM_DO_YOU_DROP2 % (dropItemName, dropItemCount)
		else :
			return HOW_MANY_ITEM_DO_YOU_DROP1 % (dropItemName)

	def FISHING_NOTIFY(isFish, fishName) :
		if isFish :
			return FISHING_NOTIFY1 % ( fishName )
		else :
			return FISHING_NOTIFY2 % ( fishName )

	def FISHING_SUCCESS(isFish, fishName) :
		if isFish :
			return FISHING_SUCCESS1 % (fishName)
		else :
			return FISHING_SUCCESS2 % (fishName)

	def NumberToMoneyString(number) :
		if number <= 0 :
			return "0 %s" % (MONETARY_UNIT0)

		number = str(number)
		result = CutMoneyString(number, 0, 4, 	"", "")
		result = CutMoneyString(number, 4, 8, 	MONETARY_UNIT1, result)
		result = CutMoneyString(number, 8, 12, 	MONETARY_UNIT2, result)
		result = result + MONETARY_UNIT0

		return result

	def NumberToSecondaryCoinString(number):
		if number <= 0:
			return "0 %s" % (MONETARY_UNIT_JUN)

		number = str(number)
		result = CutMoneyString(number, 0, 4, 	"", "")
		result = CutMoneyString(number, 4, 8, 	MONETARY_UNIT1, result)
		result = CutMoneyString(number, 8, 12, 	MONETARY_UNIT2, result)
		result = result + MONETARY_UNIT_JUN

		return result

elif IsNEWCIBN() or IsCIBN10():
	def DO_YOU_SELL_ITEM(sellItemName, sellItemCount, sellItemPrice):
		if sellItemCount>1:
			return "È·¶¨Òª°Ñ%s¸ö%sÒÔ%s½ğ±ÒÂôµôÂğ£¿" % (str(sellItemCount), sellItemName, str(sellItemPrice))
		else:
			return "È·¶¨Òª°Ñ%sÒÔ%s½ğ±ÒÂôµôÂğ£¿" % (sellItemName, str(sellItemPrice))

	def DO_YOU_BUY_ITEM(sellItemName, sellItemCount, sellItemPrice):
		if sellItemCount>1:
			return "È·¶¨Òª°Ñ%s¸ö%sÒÔ%s½ğ±ÒÂò½øÂğ£¿" % (str(sellItemCount), sellItemName, str(sellItemPrice))
		else:
			return "È·¶¨Òª°Ñ%sÒÔ%s½ğ±ÒÂò½øÂğ£¿" % (sellItemName, str(sellItemPrice))

	#def REFINE_FAILURE_CAN_NOT_ATTACH(attachedItemName):
	#	return "ÎŞ·¨ÏâÇ¶%s µÄ×°±¸" % (attachedItemName)

	def REFINE_FAILURE_NO_SOCKET(attachedItemName):
		return "Ã»ÓĞ¿ÉÒÔÏâÇ¶%s µÄ¿×" % (attachedItemName)

	def REFINE_FAILURE_NO_GOLD_SOCKET(attachedItemName):
		return "Ã»ÓĞ¿ÉÒÔÏâÇ¶%s µÄ»Æ½ğ¿×" % (attachedItemName)

	def HOW_MANY_ITEM_DO_YOU_DROP(dropItemName, dropItemCount):
		if dropItemCount>1:
			return "È·¶¨ÒªÈÓµô%d¸ö%sÂğ?" % (dropItemCount, dropItemName)
		else:
			return "È·¶¨ÒªÈÓµô%sÂğ?" % (dropItemName)

	def FISHING_NOTIFY(isFish, fishName):
		if isFish:
			return fishName # º»·¡ ¿©±â¿¡ ¾î¶² ¸»ÀÌ ºÙ¾îÀÖ´Âµ¥, ÀÎÄÚµùÀÌ ±úÁ®ÀÖ¾î¼­ º¹¿øÇÒ ¼ö°¡ ¾ø´Ù ¤Ğ¤Ğ... cython¿¡¼­ ÀÎÄÚµù ¿¡·¯ ³ª¼­ Áö¿ö¹ö¸²...
		else:
			return "µö×Å" + fishName + "ÁË¡£"

	def FISHING_SUCCESS(isFish, fishName):
		if isFish:
			return "µö×Å" + fishName + "ÁË¡£"
		else:
			return "»ñµÃ" + fishName + "ÁË¡£"

	def NumberToMoneyString(number):
		if number <= 0:
			return "0Á½"

		number = str(number)
		result = CutMoneyString(number, 0, 4, "", "")
		result = CutMoneyString(number, 4, 8, "Íò", result)
		result = CutMoneyString(number, 8, 12, "ÒÚ", result)
		result = result + "Á½"

		return result

	def NumberToSecondaryCoinString(number):
		if number <= 0:
			return "0JUN"

		number = str(number)
		result = CutMoneyString(number, 0, 4, "", "")
		result = CutMoneyString(number, 4, 8, "Íò", result)
		result = CutMoneyString(number, 8, 12, "ÒÚ", result)
		result = result + "JUN"

		return result

elif IsEUROPE() and not IsWE_KOREA() and not IsYMIR():
	def DO_YOU_SELL_ITEM(sellItemName, sellItemCount, sellItemPrice):
		if sellItemCount > 1:
			return DO_YOU_SELL_ITEM2 % (sellItemName, sellItemCount, NumberToMoneyString(sellItemPrice))
		else:
			return DO_YOU_SELL_ITEM1 % (sellItemName, NumberToMoneyString(sellItemPrice))

	if app.ENABLE_CHEQUE_SYSTEM:
		def DO_YOU_BUY_ITEM(buyItemName, buyItemCount, buyItemPrice, sellItemCheque = 0):
			if sellItemCheque > 0:
				if buyItemCount > 1:
					return DO_YOU_BUY_ITEM4 % (buyItemName, buyItemCount, sellItemCheque, buyItemPrice)
				else:
					return DO_YOU_BUY_ITEM3 % (buyItemName, sellItemCheque, buyItemPrice)
			else:
				if buyItemCount > 1:
					return DO_YOU_BUY_ITEM2 % (buyItemName, buyItemCount, buyItemPrice)
				else:
					return DO_YOU_BUY_ITEM1 % (buyItemName, buyItemPrice)
	else:
		def DO_YOU_BUY_ITEM(buyItemName, buyItemCount, buyItemPrice):
			if buyItemCount > 1:
				return DO_YOU_BUY_ITEM2 % (buyItemName, buyItemCount, buyItemPrice)
			else:
				return DO_YOU_BUY_ITEM1 % (buyItemName, buyItemPrice)

	def REFINE_FAILURE_CAN_NOT_ATTACH(attachedItemName):
		return REFINE_FAILURE_CAN_NOT_ATTACH0 % (attachedItemName)

	def REFINE_FAILURE_NO_SOCKET(attachedItemName):
		return REFINE_FAILURE_NO_SOCKET0 % (attachedItemName)

	def REFINE_FAILURE_NO_GOLD_SOCKET(attachedItemName):
		return REFINE_FAILURE_NO_GOLD_SOCKET0 % (attachedItemName)

	def HOW_MANY_ITEM_DO_YOU_DROP(dropItemName, dropItemCount):
		if dropItemCount > 1:
			return HOW_MANY_ITEM_DO_YOU_DROP2 % (dropItemName, dropItemCount)
		else:
			return HOW_MANY_ITEM_DO_YOU_DROP1 % (dropItemName)

	def FISHING_NOTIFY(isFish, fishName):
		if isFish:
			return FISHING_NOTIFY1 % (fishName)
		else:
			return FISHING_NOTIFY2 % (fishName)

	def FISHING_SUCCESS(isFish, fishName):
		if isFish:
			return FISHING_SUCCESS1 % (fishName)
		else:
			return FISHING_SUCCESS2 % (fishName)

	if app.ENABLE_CHEQUE_SYSTEM:
		def NumberToMoneyString(n):
			if n <= 0:
				return "0"

			return "%s" % ('.'.join([ i-3<0 and str(n)[:i] or str(n)[i-3:i] for i in range(len(str(n))%3, len(str(n))+1, 3) if i ]))
	else:
		def NumberToMoneyString(n):
			if n <= 0:
				return "0 %s" % (MONETARY_UNIT0)

			return "%s %s" % ('.'.join([ i-3<0 and str(n)[:i] or str(n)[i-3:i] for i in range(len(str(n))%3, len(str(n))+1, 3) if i ]), MONETARY_UNIT0)

	def NumberToSecondaryCoinString(n):
		if n <= 0:
			return "0 %s" % (MONETARY_UNIT_JUN)

		return "%s %s" % ('.'.join([ i-3<0 and str(n)[:i] or str(n)[i-3:i] for i in range(len(str(n))%3, len(str(n))+1, 3) if i ]), MONETARY_UNIT_JUN)

	def NumberToDisplayedCount(n):
		if n <= 0:
			return "0 %s" % (MONETARY_UNIT0)

		return "%s %s" % ('.'.join([ i-3<0 and str(n)[:i] or str(n)[i-3:i] for i in range(len(str(n))%3, len(str(n))+1, 3) if i ]), MONETARY_UNIT0)

	def NumberToDecimal(n):
		return "%s" % ('.'.join([ i-3<0 and str(n)[:i] or str(n)[i-3:i] for i in range(len(str(n))%3, len(str(n))+1, 3) if i ]))
