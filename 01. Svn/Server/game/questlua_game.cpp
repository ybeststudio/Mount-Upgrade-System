// Add to includes:
#if defined(__MOUNT_UPGRADE__)
	#include "mount_up_grade.h"
#endif

// In `ACMD(do_in_game_mall)`, find this block:
{
		LPCHARACTER pChar = CQuestManager::instance().GetCurrentCharacterPtr();
		lua_pushnumber(L, pChar ? CMiniGameRoulette::GetMyScoreValue(L, pChar->GetPlayerID()) : 0);
		return 1;
	}

// Add after it:
#if defined(__MOUNT_UPGRADE__)
	int game_open_mount_up_grade(lua_State* L)
	{
		const LPCHARACTER ch = CQuestManager::instance().GetCurrentCharacterPtr();
		if (nullptr == ch)
		{
			sys_err("game_open_mount_up_grade ERROR");
			return 0;
		}

		CMountUpGrade::Instance().OpenMountUpGrade(ch);

		return 0;
	}
#endif

// Find this line:
{ "get_minigame_roulette_my_score", game_get_minigame_roulette_my_score },

// Add after it:
#if defined(__MOUNT_UPGRADE__)
			{ "open_mount_up_grade", game_open_mount_up_grade },
#endif
