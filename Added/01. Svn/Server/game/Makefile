# Mythos2 Game - gmake -j16

include ../../Makefile.inc

CC       = clang++
CFLAGS   = -m32 -g -Wall -O2 -pipe -fexceptions -D_THREAD_SAFE -DNDEBUG -fdiagnostics-color=always
CXXFLAGS = -std=c++17 -Wno-invalid-source-encoding -Wno-unused-private-field \
           -Wno-unknown-pragmas -Wno-format-security

BSD_VERSION = $(shell uname -v 2>&1 | cut -d' ' -f2 | cut -d'.' -f1)
PLATFORM    = $(shell file /bin/ls | cut -d' ' -f3 | cut -d'-' -f1)

BINDIR = ..
BIN    = $(BINDIR)/game
DEPLOY_SRV_BIN ?= /home/files/main/srv1/share/bin

INCDIR =
LIBDIR =
LIBS   = -pthread -lm -lmd

OBJDIR = OBJDIR
$(shell if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi)

# External
INCDIR += -I../../../External/include
LIBDIR += -L../../../External/library

# DevIL
INCDIR += -I../../../External/include/devil/
LIBS   += -lIL -lpng -ltiff -lmng -llcms -ljpeg

# MySQL
ifeq ($(BSD_VERSION), 14)
INCDIR += -I../../libmysql/8.0.33/14.0
LIBDIR += -L../../libmysql/8.0.33/14.0
else ifeq ($(BSD_VERSION), 13)
INCDIR += -I../../libmysql/8.0.33/13.0
LIBDIR += -L../../libmysql/8.0.33/13.0
else
INCDIR += -I../../libmysql/8.0.33
LIBDIR += -L../../libmysql/8.0.33
endif
LIBS += -lmysqlclient -lz -lzstd

# OpenSSL, CryptoPP
LIBS += ../../../External/library/libcryptopp.a
LIBS += -lssl -lcrypto

# Project Library
LIBDIR += -L../../libthecore/lib
LIBS   += -lthecore
LIBDIR += -L../../libpoly
LIBS   += -lpoly
LIBDIR += -L../../libsql
LIBS   += -lsql
LIBDIR += -L../../libgame/lib
LIBS   += -lgame
INCDIR += -I../../liblua/include
LIBDIR += -L../../liblua/lib
LIBS   += -llua -llualib
INCDIR += -I../../libserverkey
LIBDIR += -L../../libserverkey
LIBS   += -lserverkey

TARGET   = $(BIN)
BUILD_LOG = $(BINDIR)/game_log.txt
CFLAGS_NOCLR = $(filter-out -fdiagnostics-color=always,$(CFLAGS)) -fno-color-diagnostics

CFILE = minilzo.c

CPPFILE = affect.cpp ani.cpp arena.cpp auth_brazil.cpp banword.cpp battle.cpp BattleArena.cpp blend_item.cpp block_country.cpp \
	BlueDragon.cpp BlueDragon_Binder.cpp buff_on_attributes.cpp buffer_manager.cpp building.cpp castle.cpp changelook.cpp char.cpp \
	char_acce.cpp char_affect.cpp char_aura.cpp char_battle.cpp char_change_empire.cpp char_dragonsoul.cpp char_horse.cpp char_item.cpp \
	char_manager.cpp char_quickslot.cpp char_resist.cpp char_skill.cpp char_state.cpp cipher.cpp ClientPackageCryptInfo.cpp cmd.cpp cmd_emotion.cpp \
	cmd_general.cpp cmd_gm.cpp cmd_oxevent.cpp config.cpp constants.cpp crc32.cpp CsvReader.cpp cube.cpp dawnmist_dungeon.cpp db.cpp desc.cpp \
	desc_client.cpp desc_manager.cpp desc_p2p.cpp dev_log.cpp dragon_soul_table.cpp DragonLair.cpp DragonSoul.cpp dungeon.cpp empire_text_convert.cpp \
	entity.cpp entity_view.cpp event.cpp event_queue.cpp exchange.cpp file_loader.cpp FileMonitor_FreeBSD.cpp fishing.cpp FSM.cpp GemShop.cpp gm.cpp \
	group_text_parse_tree.cpp guild.cpp guild_manager.cpp guild_war.cpp horse_rider.cpp horsename_manager.cpp input.cpp input_auth.cpp input_db.cpp \
	input_login.cpp input_main.cpp input_p2p.cpp input_udp.cpp ip_ban.cpp item.cpp item_addon.cpp item_apply_random_table.cpp item_attribute.cpp \
	item_manager.cpp item_manager_idrange.cpp item_manager_read_tables.cpp locale.cpp locale_service.cpp log.cpp login_data.cpp LootFilter.cpp \
	lzo_manager.cpp MailBox.cpp map_location.cpp MarkConvert.cpp MarkImage.cpp MarkManager.cpp marriage.cpp messenger_manager.cpp ingame_event_manager.cpp \
	minigame_catchking.cpp minigame_rumi.cpp minigame_yutnori.cpp mining.cpp mob_manager.cpp monarch.cpp motion.cpp mt_thunder_dungeon.cpp over9refine.cpp OXEvent.cpp \
	p2p.cpp packet_info.cpp panama.cpp party.cpp pcbang.cpp PetSystem.cpp polymorph.cpp priv_manager.cpp pvp.cpp questevent.cpp questlua.cpp \
	questlua_affect.cpp questlua_arena.cpp questlua_attr67add.cpp questlua_ba.cpp questlua_building.cpp questlua_danceevent.cpp questlua_dragonlair.cpp \
	questlua_dragonsoul.cpp questlua_dungeon.cpp questlua_forked.cpp questlua_game.cpp questlua_global.cpp questlua_guild.cpp questlua_horse.cpp \
	questlua_item.cpp questlua_marriage.cpp questlua_mgmt.cpp questlua_monarch.cpp questlua_npc.cpp questlua_oxevent.cpp questlua_party.cpp questlua_pc.cpp \
	questlua_pet.cpp questlua_quest.cpp questlua_defense_wave.cpp questlua_target.cpp questmanager.cpp questnpc.cpp questpc.cpp Ranking.cpp refine.cpp \
	regen.cpp safebox.cpp sectree.cpp sectree_manager.cpp sequence.cpp defense_wave.cpp shop.cpp shop_manager.cpp shopEx.cpp skill.cpp skill_power.cpp \
	start_position.cpp target.cpp text_file_loader.cpp threeway_war.cpp trigger.cpp utils.cpp vector.cpp war_map.cpp wedding.cpp xmas_event.cpp \
	guild_dragonlair.cpp questlua_guild_dragonlair.cpp minigame_roulette.cpp flower_event.cpp GrowthPetSystem.cpp mcp_bridge.cpp mcp_rpc.cpp \
	mount_up_grade.cpp GroupMatchManager.cpp MonstercardSystem.cpp npc_location_helper.cpp treasure_hunt_manager.cpp questlua_treasure_hunt.cpp

COBJS   = $(CFILE:%.c=$(OBJDIR)/%.o)
CPPOBJS = $(CPPFILE:%.cpp=$(OBJDIR)/%.o)
MAINOBJ = $(OBJDIR)/main.o
MAINCPP = main.cpp
ALLOBJS = $(CPPOBJS) $(COBJS) $(MAINOBJ)

.PHONY: all default clean rebuild dep help yardim tag limit_time

all default: $(TARGET)

$(TARGET): $(ALLOBJS)
	$(MERGE_BUILD_LOG)
	@echo "$(CLR_Y)$(CLR_B)[BAGLANTI]$(CLR_R) $(CLR_B)$(TARGET)$(CLR_R)"
	@$(CC) $(CFLAGS_NOCLR) $(CXXFLAGS) $(LIBDIR) $(COBJS) $(CPPOBJS) $(MAINOBJ) $(LIBS) -o $(TARGET) 2>$(OBJDIR)/link.warn; \
	EC=$$?; \
	if [ -s $(OBJDIR)/link.warn ]; then \
		echo "=== link ===" >> $(BUILD_LOG); \
		cat $(OBJDIR)/link.warn >> $(BUILD_LOG); \
	fi; \
	rm -f $(OBJDIR)/link.warn; \
	if [ $$EC -ne 0 ]; then \
		echo "$(CLR_RED)$(CLR_B)[HATA]$(CLR_R) baglanti basarisiz"; \
		tail -20 $(BUILD_LOG) 2>/dev/null; \
		exit 1; \
	fi
	@if [ -d "$(DEPLOY_SRV_BIN)" ]; then \
		cp -f $(TARGET) "$(DEPLOY_SRV_BIN)/game" && chmod 777 "$(DEPLOY_SRV_BIN)/game"; \
		echo "$(CLR_M)[DEPLOY]$(CLR_R) $(CLR_D)$(DEPLOY_SRV_BIN)/game$(CLR_R)"; \
	fi
	@echo "$(CLR_G)$(CLR_B)[TAMAM]$(CLR_R) $(CLR_B)game$(CLR_R)"
	$(SHOW_LOG_SUMMARY)

$(OBJDIR)/minilzo.o: minilzo.c $(COMMON_HDRS)
	@echo "$(CLR_C)[DERLEME]$(CLR_R) $(CLR_D)$<$(CLR_R)"
	@$(CC) -x c $(CFLAGS_NOCLR) $(INCDIR) $(DEPFLAGS) -c $< -o $@ 2>$(OBJDIR)/minilzo.warn; \
	EC=$$?; \
	if [ $$EC -ne 0 ]; then \
		echo "$(CLR_RED)$(CLR_B)[HATA]$(CLR_R) $(CLR_B)$<$(CLR_R)"; \
		cat $(OBJDIR)/minilzo.warn 2>/dev/null; \
		rm -f $(OBJDIR)/minilzo.warn; \
		exit 1; \
	fi; \
	if [ ! -s $(OBJDIR)/minilzo.warn ]; then rm -f $(OBJDIR)/minilzo.warn; fi

$(OBJDIR)/%.o: %.cpp $(COMMON_HDRS)
	$(call DO_COMPILE,$<,$@,$(OBJDIR)/$*.warn)

$(OBJDIR)/main.o: $(MAINCPP) $(COMMON_HDRS)
	$(call DO_COMPILE,$(MAINCPP),$@,$(OBJDIR)/main.warn)

rebuild: clean all

clean:
	@echo "$(CLR_Y)[TEMIZLIK]$(CLR_R) $(CLR_D)game$(CLR_R)"
	@rm -f $(ALLOBJS) $(OBJDIR)/*.d $(OBJDIR)/*.warn $(BUILD_LOG) $(BINDIR)/game_r* $(BINDIR)/conv Depend

dep:
	@rm -f Depend
	@echo "$(CLR_D)dep gerekmez (-MMD + common/)$(CLR_R)"

help yardim:
	@echo "$(CLR_C)$(CLR_B)Mythos2 game$(CLR_R)  $(CLR_G)gmake$(CLR_R) | $(CLR_G)gmake clean$(CLR_R) | $(CLR_G)gmake -j16$(CLR_R)"

tag:
	ctags *.cpp *.h *.c

limit_time:
	@true
