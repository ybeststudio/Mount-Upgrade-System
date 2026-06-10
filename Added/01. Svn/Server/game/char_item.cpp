#include "stdafx.h"

#include <stack>

#include "utils.h"
#include "config.h"
#include "char.h"
#if defined(__NPC_LOCATION_HELPER__)
#	include "npc_location_helper.h"
#endif
#if defined(ENABLE_MONSTER_CARD)
#	include "MonstercardSystem.h"
#endif
#if defined(ENABLE_TREASURE_HUNT)
#	include "treasure_hunt_manager.h"
#endif
#include "char_manager.h"
#include "item_manager.h"
#include "desc.h"
#include "desc_client.h"
#include "desc_manager.h"
#include "packet.h"
#include "affect.h"
#include "skill.h"
#include "start_position.h"
#include "mob_manager.h"
#include "db.h"
#include "log.h"
#include "vector.h"
#include "buffer_manager.h"
#include "questmanager.h"
#include "fishing.h"
#include "party.h"
#include "dungeon.h"
#include "refine.h"
#include "unique_item.h"
#include "war_map.h"
#include "xmas_event.h"
#include "marriage.h"
#include "monarch.h"
#include "polymorph.h"
#include "blend_item.h"
#include "castle.h"
#include "BattleArena.h"
#include "arena.h"
#include "dev_log.h"
#include "pcbang.h"
#include "threeway_war.h"

#include "safebox.h"
#include "shop.h"
#include "pvp.h"

#include "../../common/VnumHelper.h"
#include "DragonSoul.h"
#include "buff_on_attributes.h"
#include "belt_inventory_helper.h"
#if defined(__LOOT_FILTER_SYSTEM__)
#	include "LootFilter.h"
#endif

#if defined(__MT_THUNDER_DUNGEON__)
#	include "mt_thunder_dungeon.h"
#endif
#if defined(__GUILD_DRAGONLAIR_SYSTEM__)
#	include "guild_dragonlair.h"
#endif

#if defined(__GROWTH_PET_SYSTEM__)
#	include "GrowthPetSystem.h"
#endif

#include "../../libgame/include/grid.h"

#include <optional>

const int ITEM_BROKEN_METIN_VNUM = 28960;

// CHANGE_ITEM_ATTRIBUTES
const DWORD CHARACTER::msc_dwDefaultChangeItemAttrCycle = 10;
const char CHARACTER::msc_szLastChangeItemAttrFlag[] = "Item.LastChangeItemAttr";
const char CHARACTER::msc_szChangeItemAttrCycleFlag[] = "change_itemattr_cycle";
// END_OF_CHANGE_ITEM_ATTRIBUTES
const POINT_TYPE g_aBuffOnAttrPoints[] = { POINT_ENERGY, POINT_COSTUME_ATTR_BONUS };

struct FFindStone
{
	std::map<DWORD, LPCHARACTER> m_mapStone;

	void operator()(LPENTITY pEnt)
	{
		if (pEnt->IsType(ENTITY_CHARACTER) == true)
		{
			LPCHARACTER pChar = (LPCHARACTER)pEnt;

			if (pChar->IsStone() == true)
			{
				m_mapStone[(DWORD)pChar->GetVID()] = pChar;
			}
		}
	}
};

#if defined(__MT_THUNDER_DUNGEON__)
struct FFindMobVnum
{
	DWORD dwMobVnum;
	FFindMobVnum(DWORD dwMobVnum) : dwMobVnum(dwMobVnum) {}

	std::map<DWORD, LPCHARACTER> m_mapVID;
	void operator()(LPENTITY pEnt)
	{
		if (pEnt && pEnt->IsType(ENTITY_CHARACTER))
		{
			LPCHARACTER pChar = dynamic_cast<LPCHARACTER>(pEnt);

			if (pChar && pChar->GetRaceNum() == dwMobVnum)
			{
				m_mapVID[(DWORD)pChar->GetVID()] = pChar;
			}
		}
	}
};
#endif

// Changed like GameForge
bool IS_SUMMON_ITEM(LPITEM item, int map_index)
{
	if (item->GetVnum() == ITEM_MARRIAGE_RING)
		return true;

	switch (item->GetType())
	{
		case ITEM_QUEST:
		{
			if (item->GetSubType() == QUEST_WARP)
			{
				// NOTE : Force allow warp ring in certain maps indexes.
				if (item->GetSpecialGroup() == UNIQUE_GROUP_WARP_RING)
				{
					switch (map_index)
					{
						case MAP_SKIPIA_DUNGEON_01:
							return false;
						default:
							return true;
					}
				}
				return true;
			}
		}
		break;
		case ITEM_USE:
		{
			if (item->GetSubType() == USE_TALISMAN)
				return true;
		}
		break;
	}

	return false;
}

bool IS_MONKEY_DUNGEON(int map_index)
{
	switch (map_index)
	{
		case MAP_MONKEY_DUNGEON_11:
		case MAP_MONKEY_DUNGEON_12:
		case MAP_MONKEY_DUNGEON_13:
		case MAP_MONKEY_DUNGEON:
		case MAP_MONKEY_DUNGEON2:
		case MAP_MONKEY_DUNGEON3:
			return true;
	}

	return false;
}

bool IS_MAZE_DUNGEON(int map_index)
{
	switch (map_index)
	{
		case MAP_MAZE_DUNGEON1:
		case MAP_MAZE_DUNGEON2:
		case MAP_MAZE_DUNGEON3:
			return true;
	}

	return false;
}

#if defined(__SNOW_DUNGEON__)
bool IS_SNOW_DUNGEON(int map_index)
{
	if (map_index >= MAP_N_SNOW_DUNGEON_01 * 10000 && map_index < (MAP_N_SNOW_DUNGEON_01 + 1) * 10000)
		return true;
#if defined(__LABYRINTH_DUNGEON__)
	else if (map_index >= MAP_BOSS_CRACK_SNOW * 10000 && map_index < (MAP_BOSS_CRACK_SNOW + 1) * 10000)
		return true;
	else if (map_index >= MAP_BOSS_AWAKEN_SNOW * 10000 && map_index < (MAP_BOSS_AWAKEN_SNOW + 1) * 10000)
		return true;
#endif

	return false;
}
#endif

#if defined(__ELEMENTAL_DUNGEON__)
bool IS_ELEMENTAL_DUNGEON(int map_index)
{
	switch (map_index)
	{
		case MAP_ELEMENTAL_01:
		case MAP_ELEMENTAL_02:
		case MAP_ELEMENTAL_03:
		case MAP_ELEMENTAL_04:
			return true;
	}

	return false;
}
#endif

bool IS_SUMMONABLE_ZONE(int map_index)
{
	if (IS_MONKEY_DUNGEON(map_index))
		return false;

	if (IS_MAZE_DUNGEON(map_index))
		return false;

	if (IS_CASTLE_MAP(map_index))
		return false;

	switch (map_index)
	{
		case MAP_DEVILTOWER1:
		case MAP_SPIDERDUNGEON_02:
		case MAP_SKIPIA_DUNGEON_01:
		case MAP_SKIPIA_DUNGEON_02:
#if 0
		case 193:
		case 184:
		case 185:
		case 186:
		case 187:
		case 188:
		case 189:
#endif
		//case 206:
		case MAP_DEVILSCATACOMB:
		case MAP_SPIDERDUNGEON_03:
		case MAP_SKIPIA_DUNGEON_BOSS:
		case MAP_OXEVENT: // OX Event
		case MAP_12ZI_STAGE: // 12ZI
		case MAP_BATTLEFIED: // Battlefield
			return false;
	}

	if (CBattleArena::IsBattleArenaMap(map_index))
		return false;

	if (map_index > 10000)
		return false;

	return true;
}

bool IS_BOTARYABLE_ZONE(int nMapIndex)
{
	if (LC_IsYMIR() == false && LC_IsKorea() == false) return true;

	switch (nMapIndex)
	{
		case MAP_A1:
		case MAP_A3:
		case MAP_B1:
		case MAP_B3:
		case MAP_C1:
		case MAP_C3:
		case MAP_PRIVATESHOP:
			return true;
	}

	return false;
}

// item socket
static bool FN_check_item_socket(LPITEM item)
{
	switch (item->GetVnum())
	{
		case ITEM_AUTO_HP_RECOVERY_S:
		case ITEM_AUTO_HP_RECOVERY_M:
		case ITEM_AUTO_HP_RECOVERY_L:
		case ITEM_AUTO_HP_RECOVERY_X:
		case ITEM_AUTO_SP_RECOVERY_S:
		case ITEM_AUTO_SP_RECOVERY_M:
		case ITEM_AUTO_SP_RECOVERY_L:
		case ITEM_AUTO_SP_RECOVERY_X:
		case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS:
		case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S:
		case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS:
		case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S:
		{
			if (item->GetSocket(0) == 0 && item->GetSocket(1) == 0 && item->GetSocket(2) == item->GetProto()->alValues[0])
				return true;
		}
		break;
	}

	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		if (item->GetSocket(i) != item->GetProto()->alSockets[i])
			return false;
	}

	return true;
}

// item socket
static void FN_copy_item_socket(LPITEM dest, LPITEM src)
{
	for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
	{
		dest->SetSocket(i, src->GetSocket(i));
	}
}

static bool FN_check_item_sex(LPCHARACTER ch, LPITEM item)
{
	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_MALE))
	{
		if (SEX_MALE == GET_SEX(ch))
			return false;
	}

	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_FEMALE))
	{
		if (SEX_FEMALE == GET_SEX(ch))
			return false;
	}

	return true;
}

#define VERIFY_POTION(affect, afftype) \
	if (FindAffect(affect, afftype)) \
	{ \
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ??? ??????.")); \
		return false; \
	}

/////////////////////////////////////////////////////////////////////////////
// ITEM HANDLING
/////////////////////////////////////////////////////////////////////////////
bool CHARACTER::CanHandleItem(bool bSkipCheckRefine, bool bSkipObserver)
{
	if (!bSkipObserver)
		if (m_bIsObserver)
			return false;

	if (GetMyShop())
		return false;

	if (!bSkipCheckRefine)
	{
		if (IsUnderRefine())
			return false;

#if defined(__REFINE_ELEMENT_SYSTEM__)
		if (IsUnderRefineElement())
			return false;
#endif
	}

	if (IsWarping())
		return false;

	if (IsCubeOpen())
		return false;

#if defined(__DRAGON_SOUL_SYSTEM__)
	if (NULL != DragonSoul_RefineWindow_GetOpener())
		return false;
#endif

#if defined(__MOVE_COSTUME_ATTR__)
	if (IsItemComb())
		return false;
#endif

#if defined(__CHANGE_LOOK_SYSTEM__)
	if (GetChangeLook())
		return false;
#endif

#if defined(__MAILBOX__)
	if (GetMailBox())
		return false;
#endif

#if defined(__GEM_SHOP__)
	if (GetGemShop())
		return false;
#endif

#if defined(__ACCE_COSTUME_SYSTEM__)
	if (IsAcceRefineWindowOpen())
		return false;
#endif

#if defined(__AURA_COSTUME_SYSTEM__)
	if (IsAuraRefineWindowOpen() || NULL != GetAuraRefineWindowOpener())
		return false;
#endif

#if defined(__CHANGED_ATTR__)
	if (IsSelectAttr())
		return false;
#endif

#if defined(__GROWTH_PET_SYSTEM__)
	if (IsGrowthPetDetermineWindow())
		return false;

	if (IsGrowthPetPremiumFeedWindow())
		return false;
#endif

#if defined(__LUCKY_BOX__)
	if (IsLuckyBoxOpen())
		return false;
#endif

#if defined(__SUMMER_EVENT_ROULETTE__)
	if (GetMiniGameRoulette())
		return false;
#endif

	return true;
}

LPITEM CHARACTER::GetInventoryItem(WORD wCell) const
{
	return GetItem(TItemPos(INVENTORY, wCell));
}

LPITEM CHARACTER::GetEquipmentItem(WORD wCell) const
{
	return GetItem(TItemPos(EQUIPMENT, wCell));
}

LPITEM CHARACTER::GetDragonSoulInventoryItem(WORD wCell) const
{
	return GetItem(TItemPos(DRAGON_SOUL_INVENTORY, wCell));
}

LPITEM CHARACTER::GetBeltInventoryItem(WORD wCell) const
{
	return GetItem(TItemPos(BELT_INVENTORY, wCell));
}

#if defined(__GROWTH_PET_SYSTEM__)
LPITEM CHARACTER::GetInventoryFeedItem(uint16_t wCell) const
{
	return GetItem(TItemPos(PET_FEED, wCell));
}
#endif

LPITEM CHARACTER::GetItem(TItemPos Cell) const
{
	if (!IsValidItemPosition(Cell))
		return nullptr;

	WORD wCell = Cell.cell;
	BYTE window_type = Cell.window_type;

	switch (window_type)
	{
		case INVENTORY:
		{
			if (wCell >= INVENTORY_MAX_NUM)
			{
				sys_err("CHARACTER::GetItem: Invalid Inventory item! Window %d Cell %d", window_type, wCell);
				return nullptr;
			}
			return m_pointsInstant.pInventoryItems[wCell];
		}
		break;

		case EQUIPMENT:
		{
			if (wCell >= EQUIPMENT_MAX_NUM)
			{
				sys_err("CHARACTER::GetItem: Invalid Equipment item! Window %d Cell %d", window_type, wCell);
				return nullptr;
			}
			return m_pointsInstant.pEquipmentItems[wCell];
		}
		break;

#if defined(__DRAGON_SOUL_SYSTEM__)
		case DRAGON_SOUL_INVENTORY:
		{
			if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
			{
				sys_err("CHARACTER::GetItem: Invalid Dragon Soul item! window %d cell %d", window_type, wCell);
				return nullptr;
			}
			return m_pointsInstant.pDragonSoulInventoryItems[wCell];
		}
		break;
#endif

		case BELT_INVENTORY:
		{
			if (wCell >= BELT_INVENTORY_SLOT_COUNT)
			{
				sys_err("CHARACTER::GetItem: Invalid Belt item! window %d cell %d", window_type, wCell);
				return nullptr;
			}
			return m_pointsInstant.pBeltInventoryItems[wCell];
		}
		break;

#if defined(__ATTR_6TH_7TH__)
		case NPC_STORAGE:
		{
			if (wCell >= NPC_STORAGE_SLOT_MAX)
			{
				sys_err("CHARACTER::GetItem: invalid NPC_STORAGE item cell %d", wCell);
				return nullptr;
			}
			return m_pointsInstant.pNPCStorageItems;
		}
#endif

		default:
			return nullptr;
	}

	return nullptr;
}

void CHARACTER::SetItem(TItemPos Cell, LPITEM pItem
#if defined(__WJ_PICKUP_ITEM_EFFECT__)
	, bool isHighLight
#endif
)
{
	BYTE bWindowType = Cell.window_type;
	WORD wCell = Cell.cell;

	if ((unsigned long)((CItem*)pItem) == 0xff || (unsigned long)((CItem*)pItem) == 0xffffffff)
	{
		sys_err("!!! FATAL ERROR !!! item == 0xff (char: %s cell: %u)", GetName(), wCell);
		core_dump();
		return;
	}

	if (pItem && pItem->GetOwner())
	{
		assert(!"GetOwner exist");
		return;
	}

	// ?? ?????
	switch (bWindowType)
	{
		case INVENTORY:
		{
			if (wCell >= INVENTORY_MAX_NUM)
			{
				sys_err("CHARACTER::SetItem: Invalid Inventory item! Window %d Cell %d", bWindowType, wCell);
				return;
			}

			LPITEM pOldItem = m_pointsInstant.pInventoryItems[wCell];
			if (pOldItem)
			{
				if (wCell < INVENTORY_MAX_NUM)
				{
					for (BYTE bSize = 0; bSize < pOldItem->GetSize(); ++bSize)
					{
						WORD wSlot = wCell + (bSize * INVENTORY_WIDTH);
#if defined(__EXTEND_INVEN_SYSTEM__)
						if (wSlot >= GetExtendInvenMax())
							continue;
#else
						if (wSlot >= INVENTORY_MAX_NUM)
							continue;
#endif

						if (m_pointsInstant.pInventoryItems[wSlot] && m_pointsInstant.pInventoryItems[wSlot] != pOldItem)
							continue;

						m_pointsInstant.wInventoryItemGrid[wSlot] = 0;
					}
				}
				else
					m_pointsInstant.wInventoryItemGrid[wCell] = 0;
			}

			if (pItem)
			{
				if (wCell < INVENTORY_MAX_NUM)
				{
					for (BYTE bSize = 0; bSize < pItem->GetSize(); ++bSize)
					{
						WORD wSlot = wCell + (bSize * INVENTORY_WIDTH);
#if defined(__EXTEND_INVEN_SYSTEM__)
						if (wSlot >= GetExtendInvenMax())
							continue;
#else
						if (wSlot >= INVENTORY_MAX_NUM)
							continue;
#endif

						// wCell + 1 ?? ??? ???? ????? ???? ?? ????
						// ???????? ???????? ????
						m_pointsInstant.wInventoryItemGrid[wSlot] = wCell + 1;
					}
				}
				else
					m_pointsInstant.wInventoryItemGrid[wCell] = wCell + 1;
			}

			m_pointsInstant.pInventoryItems[wCell] = pItem;
		}
		break;

		case EQUIPMENT:
		{
			if (wCell >= EQUIPMENT_MAX_NUM)
			{
				sys_err("CHARACTER::SetItem: Invalid Equipment item! Window %d Cell %d", bWindowType, wCell);
				return;
			}

			LPITEM pOldItem = m_pointsInstant.pEquipmentItems[wCell];
			if (pOldItem)
			{
				if (wCell < EQUIPMENT_MAX_NUM)
				{
					for (BYTE bSize = 0; bSize < pOldItem->GetSize(); ++bSize)
					{
						BYTE bSlot = wCell + bSize;
						if (bSlot >= EQUIPMENT_MAX_NUM)
							continue;

						if (m_pointsInstant.pEquipmentItems[bSlot] && m_pointsInstant.pEquipmentItems[bSlot] != pOldItem)
							continue;

						m_pointsInstant.bEquipmentItemGrid[bSlot] = 0;
					}
				}
				else
					m_pointsInstant.bEquipmentItemGrid[wCell] = 0;
			}

			if (pItem)
			{
				if (wCell < EQUIPMENT_MAX_NUM)
				{
					for (BYTE bSize = 0; bSize < pItem->GetSize(); ++bSize)
					{
						BYTE bSlot = wCell + bSize;
						if (bSlot >= EQUIPMENT_MAX_NUM)
							continue;

						m_pointsInstant.bEquipmentItemGrid[bSlot] = wCell + 1;
					}
				}
				else
					m_pointsInstant.bEquipmentItemGrid[wCell] = wCell + 1;
			}

			m_pointsInstant.pEquipmentItems[wCell] = pItem;
		}
		break;

#if defined(__DRAGON_SOUL_SYSTEM__)
		case DRAGON_SOUL_INVENTORY:
		{
			if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
			{
				sys_err("CHARACTER::SetItem: Invalid Dragon Soul item! Window %d Cell %d", bWindowType, wCell);
				return;
			}

			LPITEM pOldItem = m_pointsInstant.pDragonSoulInventoryItems[wCell];
			if (pOldItem)
			{
				if (wCell < DRAGON_SOUL_INVENTORY_MAX_NUM)
				{
					for (BYTE bSize = 0; bSize < pOldItem->GetSize(); ++bSize)
					{
						WORD wSlot = wCell + (bSize * DRAGON_SOUL_BOX_COLUMN_NUM);
						if (wSlot >= DRAGON_SOUL_INVENTORY_MAX_NUM)
							continue;

						if (m_pointsInstant.pDragonSoulInventoryItems[wSlot] && m_pointsInstant.pDragonSoulInventoryItems[wSlot] != pOldItem)
							continue;

						m_pointsInstant.wDragonSoulInventoryItemGrid[wSlot] = 0;
					}
				}
				else
					m_pointsInstant.wDragonSoulInventoryItemGrid[wCell] = 0;
			}

			if (pItem)
			{
				if (wCell < DRAGON_SOUL_INVENTORY_MAX_NUM)
				{
					for (BYTE bSize = 0; bSize < pItem->GetSize(); ++bSize)
					{
						WORD wSlot = wCell + (bSize * DRAGON_SOUL_BOX_COLUMN_NUM);
						if (wSlot >= DRAGON_SOUL_INVENTORY_MAX_NUM)
							continue;

						m_pointsInstant.wDragonSoulInventoryItemGrid[wSlot] = wCell + 1;
					}
				}
				else
					m_pointsInstant.wDragonSoulInventoryItemGrid[wCell] = wCell + 1;
			}

			m_pointsInstant.pDragonSoulInventoryItems[wCell] = pItem;
		}
		break;
#endif

		case BELT_INVENTORY:
		{
			if (wCell >= BELT_INVENTORY_SLOT_COUNT)
			{
				sys_err("CHARACTER::SetItem: Invalid Belt item! Window %d Cell %d", bWindowType, wCell);
				return;
			}

			LPITEM pOldItem = m_pointsInstant.pBeltInventoryItems[wCell];
			if (pOldItem)
			{
				if (wCell < BELT_INVENTORY_SLOT_COUNT)
				{
					for (BYTE bSize = 0; bSize < pOldItem->GetSize(); ++bSize)
					{
						WORD wSlot = wCell + (bSize * BELT_INVENTORY_SLOT_WIDTH);
						if (wSlot >= BELT_INVENTORY_SLOT_COUNT)
							continue;

						if (m_pointsInstant.pBeltInventoryItems[wSlot] && m_pointsInstant.pBeltInventoryItems[wSlot] != pOldItem)
							continue;

						m_pointsInstant.bBeltInventoryItemGrid[wSlot] = 0;
					}
				}
				else
					m_pointsInstant.bBeltInventoryItemGrid[wCell] = 0;
			}

			if (pItem)
			{
				if (wCell < BELT_INVENTORY_SLOT_COUNT)
				{
					for (BYTE bSize = 0; bSize < pItem->GetSize(); ++bSize)
					{
						WORD wSlot = wCell + (bSize * BELT_INVENTORY_SLOT_WIDTH);
						if (wSlot >= BELT_INVENTORY_SLOT_COUNT)
							continue;

						m_pointsInstant.bBeltInventoryItemGrid[wSlot] = wCell + 1;
					}
				}
				else
					m_pointsInstant.bBeltInventoryItemGrid[wCell] = wCell + 1;
			}

			m_pointsInstant.pBeltInventoryItems[wCell] = pItem;
		}
		break;

#if defined(__ATTR_6TH_7TH__)
		case NPC_STORAGE:
		{
			if (wCell >= NPC_STORAGE_SLOT_MAX)
			{
				sys_err("CHARACTER::SetItem: invalid ATTR67_ADD item cell %d", wCell);
				return;
			}
			m_pointsInstant.pNPCStorageItems = pItem;
		}
		break;
#endif

		default:
			sys_err("Invalid Inventory type %d", bWindowType);
			return;
	}

	if (GetDesc())
	{
		// ??? ??????: ???????? ?????? ????? ?????? ??????
		if (pItem)
		{
			TPacketGCItemSet pack;
			pack.bHeader = HEADER_GC_ITEM_SET;
			pack.Cell = Cell;
			pack.dwVnum = pItem->GetVnum();
			pack.dwCount = pItem->GetCount();
			pack.dwFlags = pItem->GetFlag();
			pack.dwAntiFlags = pItem->GetAntiFlag();
#if defined(__WJ_PICKUP_ITEM_EFFECT__)
			if (isHighLight)
				pack.bHighLight = true;
			else
				pack.bHighLight = (Cell.window_type == DRAGON_SOUL_INVENTORY);
#else
			pack.bHighLight = (Cell.window_type == DRAGON_SOUL_INVENTORY);
#endif
#if defined(__SOUL_BIND_SYSTEM__)
			pack.lSealDate = pItem->GetSealDate();
#endif
			thecore_memcpy(pack.alSockets, pItem->GetSockets(), sizeof(pack.alSockets));
			thecore_memcpy(pack.aAttr, pItem->GetAttributes(), sizeof(pack.aAttr));
#if defined(__CHANGE_LOOK_SYSTEM__)
			pack.dwTransmutationVnum = pItem->GetTransmutationVnum();
#endif
#if defined(__REFINE_ELEMENT_SYSTEM__)
			thecore_memcpy(&pack.RefineElement, pItem->GetRefineElement(), sizeof(pack.RefineElement));
#endif
#if defined(__ITEM_APPLY_RANDOM__)
			thecore_memcpy(pack.aApplyRandom, pItem->GetRandomApplies(), sizeof(pack.aApplyRandom));
#endif
#if defined(__SET_ITEM__)
			pack.bSetValue = pItem->GetItemSetValue();
#endif

			GetDesc()->Packet(&pack, sizeof(TPacketGCItemSet));
		}
		else
		{
			TPacketGCItemSetEmpty pack;
			pack.bHeader = HEADER_GC_ITEM_DEL;
			pack.Cell = Cell;
			pack.dwVnum = 0;
			pack.dwCount = 0;
			memset(pack.alSockets, 0, sizeof(pack.alSockets));
			memset(pack.aAttr, 0, sizeof(pack.aAttr));
#if defined(__CHANGE_LOOK_SYSTEM__)
			pack.dwTransmutationVnum = 0;
#endif
#if defined(__REFINE_ELEMENT_SYSTEM__)
			memset(&pack.RefineElement, 0, sizeof(pack.RefineElement));
#endif
#if defined(__ITEM_APPLY_RANDOM__)
			memset(pack.aApplyRandom, 0, sizeof(pack.aApplyRandom));
#endif
#if defined(__SET_ITEM__)
			pack.bSetValue = 0;
#endif

			GetDesc()->Packet(&pack, sizeof(TPacketGCItemSetEmpty));
		}
	}

	if (pItem)
	{
		pItem->SetCell(this, wCell);
		switch (bWindowType)
		{
			case INVENTORY:
				pItem->SetWindow(INVENTORY);
				break;

			case EQUIPMENT:
				pItem->SetWindow(EQUIPMENT);
				break;

#if defined(__DRAGON_SOUL_SYSTEM__)
			case DRAGON_SOUL_INVENTORY:
				pItem->SetWindow(DRAGON_SOUL_INVENTORY);
				break;
#endif

			case BELT_INVENTORY:
				pItem->SetWindow(BELT_INVENTORY);
				break;

#if defined(__ATTR_6TH_7TH__)
			case NPC_STORAGE:
				pItem->SetWindow(NPC_STORAGE);
				break;
#endif

			default:
				sys_err("Trying to set window %d, non determined behaviour!", bWindowType);
		}
	}
}

LPITEM CHARACTER::GetWear(WORD wCell) const
{
	if (wCell >= EQUIPMENT_MAX_NUM)
	{
		sys_err("CHARACTER::GetWear: invalid wear cell %d", wCell);
		return nullptr;
	}

	return m_pointsInstant.pEquipmentItems[wCell];
}

void CHARACTER::SetWear(WORD wCell, LPITEM item)
{
	if (wCell >= EQUIPMENT_MAX_NUM)
	{
		sys_err("CHARACTER::SetItem: invalid item cell %d", wCell);
		return;
	}

	SetItem(TItemPos(EQUIPMENT, wCell), item);

	if (!item && wCell == WEAR_WEAPON)
	{
		// ??? ??? ?? ???? ?????? ????? ????? ???.
		if (IsAffectFlag(AFF_GWIGUM))
			RemoveAffect(SKILL_GWIGEOM);

		if (IsAffectFlag(AFF_GEOMGYEONG))
			RemoveAffect(SKILL_GEOMKYUNG);
	}
}

void CHARACTER::ClearItem()
{
	int i;
	LPITEM item;

	for (i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		if ((item = GetInventoryItem(i)))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::Instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);

			SyncQuickslot(SLOT_TYPE_INVENTORY, i, WORD_MAX);
		}
	}

	for (i = 0; i < EQUIPMENT_MAX_NUM; ++i)
	{
		if ((item = GetEquipmentItem(i)))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::Instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}

#if defined(__DRAGON_SOUL_SYSTEM__)
	for (i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if ((item = GetDragonSoulInventoryItem(i)))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::Instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);
		}
	}
#endif

	for (i = 0; i < BELT_INVENTORY_SLOT_COUNT; ++i)
	{
		if ((item = GetBeltInventoryItem(i)))
		{
			item->SetSkipSave(true);
			ITEM_MANAGER::Instance().FlushDelayedSave(item);

			item->RemoveFromCharacter();
			M2_DESTROY_ITEM(item);

			SyncQuickslot(SLOT_TYPE_BELT_INVENTORY, i, WORD_MAX);
		}
	}

#if defined(__ATTR_6TH_7TH__)
	if ((item = GetNPCStorageItem()))
	{
		item->SetSkipSave(true);
		ITEM_MANAGER::Instance().FlushDelayedSave(item);

		item->RemoveFromCharacter();
		M2_DESTROY_ITEM(item);
	}
#endif
}

bool CHARACTER::IsEmptyItemGrid(TItemPos Cell, BYTE bSize, int iExceptionCell) const
{
	switch (Cell.window_type)
	{
		case INVENTORY:
		{
			const WORD wCell = Cell.cell;
#if defined(__EXTEND_INVEN_SYSTEM__)
			if (wCell >= GetExtendInvenMax())
#else
			if (wCell >= INVENTORY_MAX_NUM)
#endif
				return false;

			// bItemCell?? 0?? false???? ??????? ???? + 1 ??? ????.
			// ???? iExceptionCell?? 1?? ???? ?????.
			++iExceptionCell;

			if (m_pointsInstant.wInventoryItemGrid[wCell])
			{
				if (m_pointsInstant.wInventoryItemGrid[wCell] == iExceptionCell)
				{
					if (bSize == 1)
						return true;

					int iOffset = 1;
#if defined(__EXTEND_INVEN_SYSTEM__)
					const WORD wPage = wCell / INVENTORY_PAGE_SIZE;
#else
					const WORD wPage = wCell / (INVENTORY_MAX_NUM / INVENTORY_PAGE_COUNT);
#endif
					do
					{
						const WORD wSlot = wCell + (INVENTORY_WIDTH * iOffset);
#if defined(__EXTEND_INVEN_SYSTEM__)
						if (wSlot >= GetExtendInvenMax())
							return false;

						if (wSlot / INVENTORY_PAGE_SIZE != wPage)
							return false;
#else
						if (wSlot >= INVENTORY_MAX_NUM)
							return false;

						if (wSlot / (INVENTORY_MAX_NUM / INVENTORY_PAGE_COUNT) != wPage)
							return false;
#endif

						if (m_pointsInstant.wInventoryItemGrid[wSlot])
						{
							if (m_pointsInstant.wInventoryItemGrid[wSlot] != iExceptionCell)
								return false;
						}
					} while (++iOffset < bSize);

					return true;
				}
				else
					return false;
			}

			// ??? 1??? ????? ??????? ?????? ??? ????
			if (1 == bSize)
				return true;
			else
			{
				int iOffset = 1;
#if defined(__EXTEND_INVEN_SYSTEM__)
				const WORD wPage = wCell / INVENTORY_PAGE_SIZE;
#else
				const WORD wPage = wCell / (INVENTORY_MAX_NUM / INVENTORY_PAGE_COUNT);
#endif

				do
				{
					const WORD wSlot = wCell + (INVENTORY_WIDTH * iOffset);
#if defined(__EXTEND_INVEN_SYSTEM__)
					if (wSlot >= GetExtendInvenMax())
						return false;

					if (wSlot / INVENTORY_PAGE_SIZE != wPage)
						return false;
#else
					if (wSlot >= INVENTORY_MAX_NUM)
						return false;

					if (wSlot / (INVENTORY_MAX_NUM / INVENTORY_PAGE_COUNT) != wPage)
						return false;
#endif

					if (m_pointsInstant.wInventoryItemGrid[wSlot])
					{
						if (m_pointsInstant.wInventoryItemGrid[wSlot] != iExceptionCell)
							return false;
					}
				} while (++iOffset < bSize);

				return true;
			}
		}
		break;

#if defined(__DRAGON_SOUL_SYSTEM__)
		case DRAGON_SOUL_INVENTORY:
		{
			const WORD wCell = Cell.cell;
			if (wCell >= DRAGON_SOUL_INVENTORY_MAX_NUM)
				return false;

			++iExceptionCell;

			if (m_pointsInstant.wDragonSoulInventoryItemGrid[wCell])
			{
				if (m_pointsInstant.wDragonSoulInventoryItemGrid[wCell] == iExceptionCell)
				{
					if (bSize == 1)
						return true;

					int iOffset = 1;

					do
					{
						const WORD wSlot = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * iOffset);
						if (wSlot >= DRAGON_SOUL_INVENTORY_MAX_NUM)
							return false;

						if (m_pointsInstant.wDragonSoulInventoryItemGrid[wSlot])
						{
							if (m_pointsInstant.wDragonSoulInventoryItemGrid[wSlot] != iExceptionCell)
								return false;
						}
					} while (++iOffset < bSize);

					return true;
				}
				else
					return false;
			}

			if (1 == bSize)
				return true;
			else
			{
				int iOffset = 1;

				do
				{
					const WORD wSlot = wCell + (DRAGON_SOUL_BOX_COLUMN_NUM * iOffset);
					if (wSlot >= DRAGON_SOUL_INVENTORY_MAX_NUM)
						return false;

					if (m_pointsInstant.wDragonSoulInventoryItemGrid[wSlot])
					{
						if (m_pointsInstant.wDragonSoulInventoryItemGrid[wSlot] != iExceptionCell)
							return false;
					}
				} while (++iOffset < bSize);

				return true;
			}
		}
		break;
#endif

		case BELT_INVENTORY:
		{
			const WORD wCell = Cell.cell;
			if (wCell >= BELT_INVENTORY_MAX_NUM)
				return false;

			++iExceptionCell;

			const LPITEM pBeltItem = GetWear(WEAR_BELT);
			if (pBeltItem == nullptr)
				return false;

			if (!CBeltInventoryHelper::IsAvailableCell(wCell - BELT_INVENTORY_SLOT_START, pBeltItem->GetValue(0)))
				return false;

			if (m_pointsInstant.bBeltInventoryItemGrid[wCell])
			{
				if (m_pointsInstant.bBeltInventoryItemGrid[wCell] == iExceptionCell)
				{
					if (bSize == 1)
						return true;

					int iOffset = 1;

					do
					{
						const WORD wSlot = wCell + (BELT_INVENTORY_SLOT_WIDTH * iOffset);
						if (wSlot >= BELT_INVENTORY_MAX_NUM)
							return false;

						if (m_pointsInstant.bBeltInventoryItemGrid[wSlot])
						{
							if (m_pointsInstant.bBeltInventoryItemGrid[wSlot] != iExceptionCell)
								return false;
						}
					} while (++iOffset < bSize);

					return true;
				}
				else
					return false;
			}

			if (1 == bSize)
				return true;
			else
			{
				int iOffset = 1;

				do
				{
					const WORD wSlot = wCell + (BELT_INVENTORY_SLOT_WIDTH * iOffset);
					if (wSlot >= BELT_INVENTORY_MAX_NUM)
						return false;

					if (m_pointsInstant.bBeltInventoryItemGrid[wSlot])
					{
						if (m_pointsInstant.bBeltInventoryItemGrid[wSlot] != iExceptionCell)
							return false;
					}
				} while (++iOffset < bSize);

				return true;
			}
		}
		break;
	}

	return false;
}

int CHARACTER::GetEmptyInventory(BYTE size) const
{
	// NOTE : ???? ?? ????? ?????? ????, ??? ???? ?????? ?? ?? ??????? ?? ??? ? ???? ????? ????,
	// ??? ??????? ??? ????????? ??????? ????? ???. (?? ?????: INVENTORY_MAX_NUM ?????? ???)
#if defined(__EXTEND_INVEN_SYSTEM__)
	for (int i = 0; i < GetExtendInvenMax(); ++i)
#else
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
#endif
		if (IsEmptyItemGrid(TItemPos(INVENTORY, i), size))
			return i;
	return -1;
}

int CHARACTER::GetEmptyInventoryCount(BYTE size) const
{
	// NOTE : ???? ?? ????? ?????? ????, ??? ???? ?????? ?? ?? ??????? ?? ??? ? ???? ????? ????,
	// ??? ??????? ??? ????????? ??????? ????? ???. (?? ?????: INVENTORY_MAX_NUM ?????? ???)
	int emptyCount = 0;
#if defined(__EXTEND_INVEN_SYSTEM__)
	for (int i = 0; i < GetExtendInvenMax(); ++i)
#else
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
#endif
	{
		if (IsEmptyItemGrid(TItemPos(INVENTORY, i), size))
			++emptyCount;
	}

	return emptyCount;
}

#if defined(__DRAGON_SOUL_SYSTEM__)
int CHARACTER::GetEmptyDragonSoulInventory(LPITEM pItem) const
{
	if (NULL == pItem || !pItem->IsDragonSoul())
		return -1;

	if (!DragonSoul_IsQualified())
		return -1;

	BYTE bSize = pItem->GetSize();
	WORD wBaseCell = DSManager::instance().GetBasePosition(pItem);

	if (WORD_MAX == wBaseCell)
		return -1;

	for (int i = 0; i < DRAGON_SOUL_BOX_SIZE; ++i)
		if (IsEmptyItemGrid(TItemPos(DRAGON_SOUL_INVENTORY, i + wBaseCell), bSize))
			return i + wBaseCell;

	return -1;
}

void CHARACTER::CopyDragonSoulItemGrid(std::vector<WORD>& vDragonSoulItemGrid) const
{
	vDragonSoulItemGrid.resize(DRAGON_SOUL_INVENTORY_MAX_NUM);
	std::copy(m_pointsInstant.wDragonSoulInventoryItemGrid, m_pointsInstant.wDragonSoulInventoryItemGrid + DRAGON_SOUL_INVENTORY_MAX_NUM, vDragonSoulItemGrid.begin());
}
#endif

int CHARACTER::CountEmptyInventory() const
{
	int count = 0;

#if defined(__EXTEND_INVEN_SYSTEM__)
	for (int i = 0; i < GetExtendInvenMax(); ++i)
#else
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
#endif
		if (GetInventoryItem(i))
			count += GetInventoryItem(i)->GetSize();

#if defined(__EXTEND_INVEN_SYSTEM__)
	return (GetExtendInvenMax() - count);
#else
	return (INVENTORY_MAX_NUM - count);
#endif
}

bool CHARACTER::HasEnoughInventorySpace(std::vector<TItemData>& vItems) const
{
	if (vItems.empty())
		return true;

	static std::array<CGrid, 4> aGrids = {
		CGrid(5, INVENTORY_MAX_NUM / 5 / INVENTORY_PAGE_COUNT),
		CGrid(5, INVENTORY_MAX_NUM / 5 / INVENTORY_PAGE_COUNT),
		CGrid(5, INVENTORY_MAX_NUM / 5 / INVENTORY_PAGE_COUNT),
		CGrid(5, INVENTORY_MAX_NUM / 5 / INVENTORY_PAGE_COUNT)
	};
	for (auto& kGrid : aGrids)
		kGrid.Clear();

	LPITEM pItem = nullptr;
	for (int i = 0; i < INVENTORY_PAGE_SIZE * 4; ++i)
	{
		if ((pItem = GetInventoryItem(i)))
		{
			aGrids[i / INVENTORY_PAGE_SIZE].Put(i % INVENTORY_PAGE_SIZE, 1, pItem->GetSize());
		}
	}

	for (const TItemData& kItem : vItems)
	{
		const TItemTable* pItemTable = ITEM_MANAGER::Instance().GetTable(kItem.dwVnum);
		if (!pItemTable)
			return true;

		int iPos = -1;
		for (int j = 0; j < aGrids.size(); ++j)
		{
			iPos = aGrids[j].FindBlank(1, pItemTable->bSize);
			if (iPos >= 0)
			{
#if defined(__EXTEND_INVEN_SYSTEM__)
				if (j >= 2)
				{
					int iExtendMaxPos = (INVENTORY_WIDTH * (GetExtendInvenStage() - (j - 1))) - 1;
					if (iPos > iExtendMaxPos)
						return false;

					if (pItemTable->bSize > 1)
					{
						iExtendMaxPos -= (INVENTORY_WIDTH * (pItemTable->bSize - 1));
						if (iPos > iExtendMaxPos)
							return false;
					}
				}
#endif
				aGrids[j].Put(iPos, 1, pItemTable->bSize);
				break;
			}
		}

		if (iPos == -1)
			return false;
	}

	return true;
}

#if defined(ENABLE_TREASURE_HUNT)
static int GetTreasureHuntRefineProbBonus(LPCHARACTER ch)
{
	if (!ch)
		return 0;

	const CAffect* pkAff = ch->FindAffect(AFFECT_INCREASE_REFINE_PCT);
	if (!pkAff)
		return 0;

	return pkAff->lApplyValue;
}

static bool HasTreasureHuntRefineFreeMaterial(LPCHARACTER ch)
{
	return ch && ch->FindAffect(AFFECT_REFINE_FREE_MATERIAL);
}

static void ConsumeTreasureHuntRefineAffects(LPCHARACTER ch, bool bUsedProbBonus, bool bUsedFreeMaterial)
{
	if (!ch)
		return;

	if (bUsedProbBonus)
		ch->RemoveAffect(AFFECT_INCREASE_REFINE_PCT);

	if (bUsedFreeMaterial)
		ch->RemoveAffect(AFFECT_REFINE_FREE_MATERIAL);
}
#endif

void TransformRefineItem(LPITEM pkOldItem, LPITEM pkNewItem)
{
	// ACCESSORY_REFINE
	if (pkOldItem->IsAccessoryForSocket())
	{
		for (int i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
		{
			pkNewItem->SetSocket(i, pkOldItem->GetSocket(i));
		}
		//pkNewItem->StartAccessorySocketExpireEvent();
	}
	// END_OF_ACCESSORY_REFINE
	else
	{
		// ???? ???????? ????????? ??? ??
		for (int i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
		{
			if (!pkOldItem->GetSocket(i))
				break;
			else
				pkNewItem->SetSocket(i, 1);
		}

		// ???? ????
		int slot = 0;

		for (int i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
		{
			long socket = pkOldItem->GetSocket(i);

			if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
				pkNewItem->SetSocket(slot++, socket);
		}

	}

#if defined(__ITEM_SOCKET6__)
	for (int i = METIN_SOCKET_MAX_NUM; i < ITEM_SOCKET_MAX_NUM; ++i)
		pkNewItem->SetSocket(i, pkOldItem->GetSocket(i));
#endif

	// ???? ?????? ????
	pkOldItem->CopyAttributeTo(pkNewItem);
}

void NotifyRefineSuccess(LPCHARACTER ch, LPITEM item, const char* way)
{
	if (NULL != ch && item != NULL)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "RefineSuceeded");

		LogManager::instance().RefineLog(ch->GetPlayerID(), item->GetName(), item->GetID(), item->GetRefineLevel(), 1, way);
	}
}

void NotifyRefineFail(LPCHARACTER ch, LPITEM item, const char* way, int success = 0)
{
	if (NULL != ch && NULL != item)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "RefineFailed");

		LogManager::instance().RefineLog(ch->GetPlayerID(), item->GetName(), item->GetID(), item->GetRefineLevel(), success, way);
	}
}

#if defined(__REFINE_MSG_ADD__)
void NotifyRefineFailType(const LPCHARACTER ch, const LPITEM item, const BYTE type, const char* way, const BYTE success = 0)
{
	if (ch && item)
	{
		ch->ChatPacket(CHAT_TYPE_COMMAND, "RefineFailedType %d", type);
		LogManager::instance().RefineLog(ch->GetPlayerID(), item->GetName(), item->GetID(), item->GetRefineLevel(), success, way);
	}
}
#endif

void CHARACTER::SetRefineNPC(LPCHARACTER ch)
{
	if (ch != NULL)
	{
		m_dwRefineNPCVID = ch->GetVID();
	}
	else
	{
		m_dwRefineNPCVID = 0;
	}
}

bool CHARACTER::DoRefine(LPITEM item, bool bMoneyOnly)
{
	if (!CanHandleItem(true))
	{
		ClearRefineMode();
		return false;
	}

	//???? ????? : upgrade_refine_scroll.quest ???? ?????? 5??????? ??? ??????
	//??????? ????
	if (quest::CQuestManager::instance().GetEventFlag("update_refine_time") != 0)
	{
		if (get_global_time() < quest::CQuestManager::instance().GetEventFlag("update_refine_time") + (60 * 5))
		{
			sys_log(0, "can't refine %d %s", GetPlayerID(), GetName());
			return false;
		}
	}

	const TRefineTable* prt = CRefineManager::instance().GetRefineRecipe(item->GetRefineSet());
	if (!prt)
		return false;

	DWORD result_vnum = item->GetRefinedVnum();

	// REFINE_COST
	int cost = ComputeRefineFee(prt->cost);

	int RefineChance = GetQuestFlag("main_quest_lv7.refine_chance");

	if (RefineChance > 0)
	{
		if (!item->CheckItemUseLevel(20) || item->GetType() != ITEM_WEAPON)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ???? ????? 20 ?????? ???? ????????"));
			return false;
		}
		cost = 0;
		SetQuestFlag("main_quest_lv7.refine_chance", RefineChance - 1);
	}
	// END_OF_REFINE_COST

	if (result_vnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ??? ?????? ?? ???????."));
		return false;
	}

	if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TUNING)
		return false;

	TItemTable* pProto = ITEM_MANAGER::instance().GetTable(item->GetRefinedVnum());

	if (!pProto)
	{
		sys_err("DoRefine NOT GET ITEM PROTO %d", item->GetRefinedVnum());
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ???????? ?????? ?? ???????."));
		return false;
	}

	// Check level limit in korea only
	if (!g_iUseLocale)
	{
		for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
		{
			long limit = pProto->aLimits[i].lValue;

			switch (pProto->aLimits[i].bType)
			{
				case LIMIT_LEVEL:
				{
					if (GetLevel() < limit)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ?? ???????? ???? ??????? ?????? ???????."));
						return false;
					}
				}
				break;

#if defined(__CONQUEROR_LEVEL__)
				case LIMIT_NEWWORLD_LEVEL:
				{
					if (GetConquerorLevel() < limit)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ?? ???????? ???? ??????? ?????? ???????."));
						return false;
					}
				}
				break;
#endif
			}
		}
	}

	// REFINE_COST
	if (GetGold() < (long long)cost)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ??? ???? ???? ????????."));
		return false;
	}

#if defined(__REFINE_STACK_FIX__)
	int iEmptyPos = GetEmptyInventory(item->GetSize());
	if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
	{
		if (-1 == iEmptyPos)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ?? ?????? ???????."));
			return false;
		}
	}
#endif

	if (!bMoneyOnly && !RefineChance)
	{
#if defined(ENABLE_TREASURE_HUNT)
		const bool bTreasureHuntFreeMaterial = HasTreasureHuntRefineFreeMaterial(this);
#else
		const bool bTreasureHuntFreeMaterial = false;
#endif
		if (!bTreasureHuntFreeMaterial)
		{
			for (int i = 0; i < prt->material_count; ++i)
			{
				if (CountSpecifyItem(prt->materials[i].vnum, item->GetCell()) < prt->materials[i].count)
				{
					if (test_server)
					{
						ChatPacket(CHAT_TYPE_INFO, "Find %d, count %d, require %d", prt->materials[i].vnum, CountSpecifyItem(prt->materials[i].vnum), prt->materials[i].count);
					}

					ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ??? ???? ??? ????????."));
					return false;
				}
			}

			for (int i = 0; i < prt->material_count; ++i)
				RemoveSpecifyItem(prt->materials[i].vnum, prt->materials[i].count, item->GetCell());
		}
	}

#if defined(ENABLE_TREASURE_HUNT)
	const bool bUsedTreasureHuntFreeMaterial = HasTreasureHuntRefineFreeMaterial(this);
	const int iTreasureHuntProbBonus = GetTreasureHuntRefineProbBonus(this);
	const bool bUsedTreasureHuntProbBonus = (iTreasureHuntProbBonus > 0);
#else
	const bool bUsedTreasureHuntFreeMaterial = false;
	const int iTreasureHuntProbBonus = 0;
	const bool bUsedTreasureHuntProbBonus = false;
#endif

	int prob = number(1, 100);
	int iSuccessProb = prt->prob;

#if defined(ENABLE_TREASURE_HUNT)
	if (iTreasureHuntProbBonus > 0)
	{
		iSuccessProb += iTreasureHuntProbBonus;
		if (iSuccessProb > 100)
			iSuccessProb = 100;
	}
#endif

	if (IsRefineThroughGuild() || bMoneyOnly)
		prob -= 10;

	// END_OF_REFINE_COST

	if (prob <= iSuccessProb)
	{
		// ????! ??? ???????? ???????, ???? ????? ??? ?????? ???
		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(result_vnum, 1, 0, false);

		if (pkNewItem)
		{
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);

#if defined(__ITEM_APPLY_RANDOM__)
			TPlayerItemAttribute aApplyRandom[ITEM_APPLY_MAX_NUM];
			item->GetRandomApplyTable(aApplyRandom, CApplyRandomTable::GET_NEXT);
			if (aApplyRandom)
				pkNewItem->SetRandomApplies(aApplyRandom);
#endif

			LogManager::instance().ItemLog(this, pkNewItem, "REFINE SUCCESS", pkNewItem->GetName());

			UINT bCell = item->GetCell();

			// DETAIL_REFINE_LOG
			NotifyRefineSuccess(this, item, IsRefineThroughGuild() ? "GUILD" : "POWER");
			DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -cost);
			// END_OF_DETAIL_REFINE_LOG

#if defined(__REFINE_STACK_FIX__)
			if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
			{
				item->SetCount(item->GetCount() - 1);
				AutoGiveItem(pkNewItem, true, true
#if defined(__WJ_PICKUP_ITEM_EFFECT__)
					, true
#endif
				);
			}
			else
			{
				ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");
				pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, bCell));
			}
#else
			ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");
			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, bCell));
#endif

			ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);
			pkNewItem->AttrLog();

			sys_log(0, "Refine Success %d", cost);
			//PointChange(POINT_GOLD, -cost);
			sys_log(0, "PayPee %d", cost);
			PayRefineFee(cost);
			sys_log(0, "PayPee End %d", cost);
		}
		else
		{
			// DETAIL_REFINE_LOG
			// ?????? ?????? ???? -> ???? ????? ????
			sys_err("cannot create item %u", result_vnum);
#if defined(__REFINE_MSG_ADD__)
			NotifyRefineFailType(this, item, REFINE_FAIL_KEEP_GRADE, IsRefineThroughGuild() ? "GUILD" : "POWER");
#else
			NotifyRefineFail(this, item, IsRefineThroughGuild() ? "GUILD" : "POWER");
#endif
			// END_OF_DETAIL_REFINE_LOG
		}
	}
	else
	{
		// ????! ??? ???????? ?????.
		DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -cost);
#if defined(__REFINE_MSG_ADD__)
		NotifyRefineFailType(this, item, REFINE_FAIL_DEL_ITEM, IsRefineThroughGuild() ? "GUILD" : "POWER");
#else
		NotifyRefineFail(this, item, IsRefineThroughGuild() ? "GUILD" : "POWER");
#endif
		item->AttrLog();

#if defined(__REFINE_STACK_FIX__)
		if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
		{
			item->SetCount(item->GetCount() - 1);
		}
		else
		{
			ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE FAIL)");
		}
#else
		ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE FAIL)");
#endif

		//PointChange(POINT_GOLD, -cost);
		PayRefineFee(cost);
	}

#if defined(ENABLE_TREASURE_HUNT)
	if (bUsedTreasureHuntProbBonus || bUsedTreasureHuntFreeMaterial)
		ConsumeTreasureHuntRefineAffects(this, bUsedTreasureHuntProbBonus, bUsedTreasureHuntFreeMaterial);
#endif

	return true;
}

enum ERefineScroll
{
	CHUKBOK_SCROLL = 0,
	HYUNIRON_STONE = 1,
	YONGSIN_SCROLL = 2,
	MUSIN_SCROLL = 3,
	YAGONG_SCROLL = 4,
	MEMO_SCROLL = 5,
	BDRAGON_SCROLL = 6,
#if defined(__STONE_OF_BLESS__)
	BLESSING_STONE = 7,
#endif
#if defined(__SOUL_SYSTEM__)
	SOUL_AWAKE_SCROLL = 8,
	SOUL_EVOLVE_SCROLL = 9,
#endif
#if defined(__STONE_OF_BLESS__)
	MALL_BLESSING_STONE = 10,
#endif
};

#if defined(__SOUL_SYSTEM__)
bool CHARACTER::DoRefineSoul(LPITEM item)
{
	if (!CanHandleItem(true))
	{
		ClearRefineMode();
		return false;
	}

	ClearRefineMode();

	const TRefineTable* prt = CRefineManager::instance().GetRefineRecipe(item->GetRefineSet());
	if (!prt)
		return false;

	if (m_iRefineAdditionalCell < 0)
		return false;

	LPITEM item_scroll = GetInventoryItem(m_iRefineAdditionalCell);
	if (item_scroll == nullptr)
		return false;

	if (!(item_scroll->GetType() == ITEM_USE && item_scroll->GetSubType() == USE_TUNING))
		return false;

	if (item_scroll->GetVnum() == item->GetVnum())
		return false;

	DWORD result_vnum = item->GetRefinedVnum();
	if (result_vnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ??? ?????? ?? ???????."));
		return false;
	}

	TItemTable* proto = ITEM_MANAGER::instance().GetTable(item->GetRefinedVnum());
	if (!proto)
	{
		sys_err("DoRefineSoul NOT GET ITEM PROTO %d", item->GetRefinedVnum());
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ???????? ?????? ?? ???????."));
		return false;
	}

	int prob = number(1, 100);
	int success_prob = prt->prob;

	switch (item_scroll->GetValue(0))
	{
		case SOUL_AWAKE_SCROLL:
			success_prob = 100;
			break;

		case SOUL_EVOLVE_SCROLL:
			success_prob = soul_refine_prob[MINMAX(0, item->GetValue(1), SOUL_GRADE_MAX)];
			break;
	}

	item_scroll->SetCount(item_scroll->GetCount() - 1);

	if (prob <= success_prob)
	{
		LPITEM new_item = ITEM_MANAGER::instance().CreateItem(result_vnum, 1, 0, false);
		if (new_item)
		{
			WORD wCell = item->GetCell();

			LogManager::instance().ItemLog(this, new_item, "SOUL REFINE FAIL", new_item->GetName());
			ChatPacket(CHAT_TYPE_COMMAND, "RefineSoulSuceeded");

			ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (SOUL REFINE FAIL)");

			new_item->AddToCharacter(this, TItemPos(INVENTORY, wCell));
			ITEM_MANAGER::instance().FlushDelayedSave(new_item);
		}
		else
		{
			sys_err("cannot create item %u", result_vnum);
			ChatPacket(CHAT_TYPE_COMMAND, "RefineSoulFailed");
		}
	}
	else
	{
		ChatPacket(CHAT_TYPE_COMMAND, "RefineSoulFailed");
	}

	return true;
}
#endif

bool CHARACTER::DoRefineWithScroll(LPITEM item)
{
	if (!CanHandleItem(true))
	{
		ClearRefineMode();
		return false;
	}

	ClearRefineMode();

	// ???? ????? : upgrade_refine_scroll.quest ???? ?????? 5??????? ??? ??????
	// ??????? ????
	if (quest::CQuestManager::instance().GetEventFlag("update_refine_time") != 0)
	{
		if (get_global_time() < quest::CQuestManager::instance().GetEventFlag("update_refine_time") + (60 * 5))
		{
			sys_log(0, "can't refine %d %s", GetPlayerID(), GetName());
			return false;
		}
	}

	const TRefineTable* prt = CRefineManager::instance().GetRefineRecipe(item->GetRefineSet());
	if (!prt)
		return false;

	LPITEM pkItemScroll;

	// ?????? ??
	if (m_iRefineAdditionalCell < 0)
		return false;

	pkItemScroll = GetInventoryItem(m_iRefineAdditionalCell);

	if (!pkItemScroll)
		return false;

	if (!(pkItemScroll->GetType() == ITEM_USE && pkItemScroll->GetSubType() == USE_TUNING))
		return false;

	if (pkItemScroll->GetVnum() == item->GetVnum())
		return false;

	DWORD result_vnum = item->GetRefinedVnum();
	DWORD result_fail_vnum = item->GetRefineFromVnum();

	if (result_vnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ??? ?????? ?? ???????."));
		return false;
	}

	TItemTable* pProto = ITEM_MANAGER::instance().GetTable(item->GetRefinedVnum());
	if (!pProto)
	{
		sys_err("DoRefineWithScroll NOT GET ITEM PROTO %d", item->GetRefinedVnum());
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ???????? ?????? ?? ???????."));
		return false;
	}

	if (GetGold() < prt->cost)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ??? ???? ???? ????????."));
		return false;
	}

#if defined(__REFINE_STACK_FIX__)
	int iEmptyPos = GetEmptyInventory(item->GetSize());
	if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
	{
		if (-1 == iEmptyPos)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ?? ?????? ???????."));
			return false;
		}
	}
#endif

	for (int i = 0; i < prt->material_count; ++i)
	{
#if defined(ENABLE_TREASURE_HUNT)
		if (HasTreasureHuntRefineFreeMaterial(this))
			break;
#endif
		if (CountSpecifyItem(prt->materials[i].vnum, item->GetCell()) < prt->materials[i].count)
		{
			if (test_server)
			{
				ChatPacket(CHAT_TYPE_INFO, "Find %d, count %d, require %d", prt->materials[i].vnum, CountSpecifyItem(prt->materials[i].vnum), prt->materials[i].count);
			}

			ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ??? ???? ??? ????????."));
			return false;
		}
	}

#if defined(ENABLE_TREASURE_HUNT)
	const bool bUsedTreasureHuntFreeMaterialScroll = HasTreasureHuntRefineFreeMaterial(this);
	if (!bUsedTreasureHuntFreeMaterialScroll)
#endif
	{
		for (int i = 0; i < prt->material_count; ++i)
			RemoveSpecifyItem(prt->materials[i].vnum, prt->materials[i].count, item->GetCell());
	}

#if defined(ENABLE_TREASURE_HUNT)
	const int iTreasureHuntProbBonusScroll = GetTreasureHuntRefineProbBonus(this);
	const bool bUsedTreasureHuntProbBonusScroll = (iTreasureHuntProbBonusScroll > 0);
#else
	const int iTreasureHuntProbBonusScroll = 0;
	const bool bUsedTreasureHuntProbBonusScroll = false;
#endif

	int prob = number(1, 100);
	const SRefineScrollData& kScroll = GetScrollRefinePct(prt->prob, item->GetRefinedVnum(), pkItemScroll->GetValue(0));
	int iSuccessProb = kScroll.bSuccessProb;

#if defined(ENABLE_TREASURE_HUNT)
	if (iTreasureHuntProbBonusScroll > 0)
	{
		iSuccessProb += iTreasureHuntProbBonusScroll;
		if (iSuccessProb > 100)
			iSuccessProb = 100;
	}
#endif
	const char* szRefineType = "SCROLL";

	switch (pkItemScroll->GetValue(0))
	{
		case HYUNIRON_STONE:
			szRefineType = "HYUNIRON_STONE";
			break;

		case YONGSIN_SCROLL:
			szRefineType = "GOD_SCROLL";
			break;

		case MUSIN_SCROLL:
			szRefineType = "MUSIN_SCROLL";
			break;

		case YAGONG_SCROLL:
			szRefineType = "YAGONG_SCROLL";
			break;

		case MEMO_SCROLL:
			szRefineType = "MEMO_SCROLL";
			break;

		case BDRAGON_SCROLL:
			szRefineType = "BDRAGON_SCROLL";
			break;

#if defined(__STONE_OF_BLESS__)
		case BLESSING_STONE:
			szRefineType = "BLESSING_STONE";
			break;

		case MALL_BLESSING_STONE:
			szRefineType = "MALL_BLESSING_STONE";
			break;
#endif

		default:
			sys_err("REFINE : Unknown refine scroll item. Value0: %d", pkItemScroll->GetValue(0));
			break;
	}

	if (test_server)
		ChatPacket(CHAT_TYPE_INFO, "[Only Test] SuccessProb %d, RefineLevel %d ", iSuccessProb, item->GetRefineLevel());

	pkItemScroll->SetCount(pkItemScroll->GetCount() - 1);

	if (prob <= iSuccessProb)
	{
		// ????! ??? ???????? ???????, ???? ????? ??? ?????? ???
		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(result_vnum, 1, 0, false);

		if (pkNewItem)
		{
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);

#if defined(__ITEM_APPLY_RANDOM__)
			TPlayerItemAttribute aApplyRandom[ITEM_APPLY_MAX_NUM];
			item->GetRandomApplyTable(aApplyRandom, CApplyRandomTable::GET_NEXT);
			if (aApplyRandom)
				pkNewItem->SetRandomApplies(aApplyRandom);
#endif

			LogManager::instance().ItemLog(this, pkNewItem, "REFINE SUCCESS", pkNewItem->GetName());

			UINT bCell = item->GetCell();

			NotifyRefineSuccess(this, item, szRefineType);
			DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -prt->cost);

#if defined(__REFINE_STACK_FIX__)
			if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
			{
				item->SetCount(item->GetCount() - 1);
				AutoGiveItem(pkNewItem, true, true
#if defined(__WJ_PICKUP_ITEM_EFFECT__)
					, true
#endif
				);
			}
			else
			{
				ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");
				pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, bCell));
			}
#else
			ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE SUCCESS)");
			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, bCell));
#endif

			ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);
			pkNewItem->AttrLog();

			//PointChange(POINT_GOLD, -prt->cost);
			PayRefineFee(prt->cost);
		}
		else
		{
			// ?????? ?????? ???? -> ???? ????? ????
			sys_err("cannot create item %u", result_vnum);
#if defined(__REFINE_MSG_ADD__)
			NotifyRefineFailType(this, item, REFINE_FAIL_KEEP_GRADE, szRefineType);
#else
			NotifyRefineFail(this, item, szRefineType);
#endif
		}
	}
	else if (!kScroll.bKeepGrade && result_fail_vnum)
	{
		// ????! ??? ???????? ???????, ???? ????? ???? ????? ?????? ???
		LPITEM pkNewItem = ITEM_MANAGER::instance().CreateItem(result_fail_vnum, 1, 0, false);

		if (pkNewItem)
		{
			ITEM_MANAGER::CopyAllAttrTo(item, pkNewItem);

#if defined(__ITEM_APPLY_RANDOM__)
			TPlayerItemAttribute aApplyRandom[ITEM_APPLY_MAX_NUM];
			item->GetRandomApplyTable(aApplyRandom, CApplyRandomTable::GET_PREVIOUS);
			if (aApplyRandom)
				pkNewItem->SetRandomApplies(aApplyRandom);
#endif

			LogManager::instance().ItemLog(this, pkNewItem, "REFINE FAIL", pkNewItem->GetName());

			UINT bCell = item->GetCell();

			DBManager::instance().SendMoneyLog(MONEY_LOG_REFINE, item->GetVnum(), -prt->cost);
#if defined(__REFINE_MSG_ADD__)
			NotifyRefineFailType(this, item, REFINE_FAIL_GRADE_DOWN, szRefineType, -1);
#else
			NotifyRefineFail(this, item, szRefineType, -1);

#endif

#if defined(__REFINE_STACK_FIX__)
			if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
			{
				item->SetCount(item->GetCount() - 1);
				AutoGiveItem(pkNewItem, true, true
#if defined(__WJ_PICKUP_ITEM_EFFECT__)
					, true
#endif
				);
			}
			else
			{
				ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE FAIL)");
				pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, bCell));
			}
#else
			ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (REFINE FAIL)");
			pkNewItem->AddToCharacter(this, TItemPos(INVENTORY, bCell));
#endif

			ITEM_MANAGER::instance().FlushDelayedSave(pkNewItem);
			pkNewItem->AttrLog();

			//PointChange(POINT_GOLD, -prt->cost);
			PayRefineFee(prt->cost);
		}
		else
		{
			// ?????? ?????? ???? -> ???? ????? ????
			sys_err("cannot create item %u", result_fail_vnum);
#if defined(__REFINE_MSG_ADD__)
			NotifyRefineFailType(this, item, REFINE_FAIL_KEEP_GRADE, szRefineType);
#else
			NotifyRefineFail(this, item, szRefineType);
#endif
		}
	}
	else
	{
#if defined(__REFINE_MSG_ADD__)
		NotifyRefineFailType(this, item, REFINE_FAIL_KEEP_GRADE, szRefineType);
#else
		NotifyRefineFail(this, item, szRefineType); // ?????? ?????? ??????? ????
#endif
		PayRefineFee(prt->cost);
	}

#if defined(ENABLE_TREASURE_HUNT)
	if (bUsedTreasureHuntProbBonusScroll || bUsedTreasureHuntFreeMaterialScroll)
		ConsumeTreasureHuntRefineAffects(this, bUsedTreasureHuntProbBonusScroll, bUsedTreasureHuntFreeMaterialScroll);
#endif

	return true;
}

CHARACTER::SRefineScrollData CHARACTER::GetScrollRefinePct(BYTE bProb, BYTE bRefineLevel, BYTE bScrollType)
{
	SRefineScrollData Scroll{};
	Scroll.bSuccessProb = bProb;
	Scroll.bKeepGrade = false;

	// ???, ????? ????, ????? ?????? ?
	switch (bScrollType)
	{
		case CHUKBOK_SCROLL:
			break;

		case HYUNIRON_STONE:
			Scroll.bSuccessProb = MINMAX(1, bProb + 10, 100);
			Scroll.bKeepGrade = true;
			break;

		case YONGSIN_SCROLL:
			Scroll.bSuccessProb = MINMAX(1, bProb + 10, 100);
			break;

		case MUSIN_SCROLL: // ?????? ?????? 100% ???? (+4??????)
			Scroll.bSuccessProb = 100;
			break;

		case YAGONG_SCROLL:
			Scroll.bSuccessProb = MINMAX(1, bProb + 15, 100);
			break;

		case MEMO_SCROLL:
			Scroll.bSuccessProb = 100;
			break;

		case BDRAGON_SCROLL:
			Scroll.bSuccessProb = 80;
			break;

#if defined(__STONE_OF_BLESS__)
		case BLESSING_STONE:
			Scroll.bSuccessProb = MINMAX(1, bProb + 15, 100);
			Scroll.bKeepGrade = true;
			break;

		case MALL_BLESSING_STONE:
			Scroll.bSuccessProb = MINMAX(1, bProb + 20, 100);
			Scroll.bKeepGrade = true;
			break;
#endif
	}

	return Scroll;
}

bool CHARACTER::RefineInformation(WORD wCell, BYTE bType, int iAdditionalCell)
{
	if (wCell > INVENTORY_MAX_NUM)
		return false;

	const LPITEM item = GetInventoryItem(wCell);
	if (!item)
		return false;

	// REFINE_COST
	if (bType == REFINE_TYPE_MONEY_ONLY && !GetQuestFlag("deviltower_zone.can_refine"))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ??? ??? ?????? ??????? ?????????."));
		return false;
	}
	// END_OF_REFINE_COST

	TPacketGCRefineInformation p;
	p.header = HEADER_GC_REFINE_INFORMATION;
	p.pos = wCell;
	p.src_vnum = item->GetVnum();
	p.result_vnum = item->GetRefinedVnum();
	p.type = bType;

#if defined(__REFINE_ELEMENT_SYSTEM__)
	thecore_memcpy(&p.RefineElement, item->GetRefineElement(), sizeof(p.RefineElement));
#endif

#if defined(__ITEM_APPLY_RANDOM__)
	TPlayerItemAttribute aApplyRandom[ITEM_APPLY_MAX_NUM];
	item->GetRandomApplyTable(aApplyRandom, CApplyRandomTable::GET_NEXT);
	thecore_memcpy(&p.aApplyRandom, aApplyRandom, sizeof(p.aApplyRandom));
#endif

	if (p.result_vnum == 0)
	{
		sys_err("RefineInformation p.result_vnum == 0");
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ???????? ?????? ?? ???????."));
		return false;
	}

	if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TUNING)
	{
		if (bType == 0)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ???????? ?? ???????? ?????? ?? ???????."));
			return false;
		}
		else
		{
			const LPITEM& item_scroll = GetInventoryItem(iAdditionalCell);
			if (!item_scroll || item->GetVnum() == item_scroll->GetVnum())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ???????? ??? ???? ???????."));
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ???? ????? ??? ?? ??????."));
				return false;
			}
		}
	}

#if defined(__SOUL_SYSTEM__)
	if (item->GetType() == ITEM_SOUL)
	{
		if (bType == REFINE_TYPE_SOUL_AWAKE || bType == REFINE_TYPE_SOUL_EVOLVE)
		{
			p.cost = 0;
			p.prob = bType == REFINE_TYPE_SOUL_AWAKE ? 100 : soul_refine_prob[MINMAX(0, item->GetValue(1), SOUL_GRADE_MAX)];
			p.material_count = 0;
			std::memset(p.materials, 0, sizeof(p.materials));

			GetDesc()->Packet(&p, sizeof(TPacketGCRefineInformation));

			SetRefineMode(iAdditionalCell);
			return true;
		}
	}
#endif

	CRefineManager& rm = CRefineManager::instance();
	const TRefineTable* prt = rm.GetRefineRecipe(item->GetRefineSet());
	if (!prt)
	{
		sys_err("RefineInformation NOT GET REFINE SET %d", item->GetRefineSet());
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ???????? ?????? ?? ???????."));
		return false;
	}

	// REFINE_COST

	// MAIN_QUEST_LV7
	if (GetQuestFlag("main_quest_lv7.refine_chance") > 0)
	{
		// ????? ????
		if (!item->CheckItemUseLevel(20) || item->GetType() != ITEM_WEAPON)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ???? ????? 20 ?????? ???? ????????"));
			return false;
		}
		p.cost = 0;
	}
	else
		p.cost = ComputeRefineFee(prt->cost);
	// END_MAIN_QUEST_LV7

	const LPITEM& item_scroll = GetInventoryItem(iAdditionalCell);
	if (item_scroll)
	{
		const SRefineScrollData& kScroll = GetScrollRefinePct(prt->prob, item->GetRefinedVnum(), item_scroll->GetValue(0));
		p.prob = kScroll.bSuccessProb;
	}
	else
	{
		p.prob = prt->prob;

		if (IsRefineThroughGuild())
			p.prob += 10;
	}

#if defined(ENABLE_TREASURE_HUNT)
	{
		const int iTreasureHuntProbBonus = GetTreasureHuntRefineProbBonus(this);
		if (iTreasureHuntProbBonus > 0)
		{
			p.prob += iTreasureHuntProbBonus;
			if (p.prob > 100)
				p.prob = 100;
		}
	}
#endif

	if (p.prob > 100)
		p.prob = 100;

	if (bType == REFINE_TYPE_MONEY_ONLY)
	{
		p.material_count = 0;
		memset(p.materials, 0, sizeof(p.materials));
	}
#if defined(ENABLE_TREASURE_HUNT)
	else if (HasTreasureHuntRefineFreeMaterial(this))
	{
		p.material_count = 0;
		memset(p.materials, 0, sizeof(p.materials));
	}
#endif
	else
	{
		p.material_count = prt->material_count;
		thecore_memcpy(&p.materials, prt->materials, sizeof(prt->materials));
	}
	// END_OF_REFINE_COST

	GetDesc()->Packet(&p, sizeof(TPacketGCRefineInformation));

	SetRefineMode(iAdditionalCell);
	return true;
}

bool CHARACTER::RefineItem(LPITEM pkItem, LPITEM pkTarget)
{
	if (!CanHandleItem())
		return false;

	if (pkItem->GetSubType() == USE_TUNING)
	{
		// XXX ????, ???? ???????? ??????????...
		// XXX ??????????? ???? ???? ?????!

		BYTE bRefineType = REFINE_TYPE_NORMAL;
		switch (pkItem->GetValue(0))
		{
			case HYUNIRON_STONE:
				bRefineType = REFINE_TYPE_HYUNIRON;
				break;

			case MUSIN_SCROLL:
			{
				if (pkTarget->GetRefineLevel() >= 4)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ???????? ?? ??? ?????? ?? ???????."));
					return false;
				}

				bRefineType = REFINE_TYPE_MUSIN;
			}
			break;

			case MEMO_SCROLL:
			{
				if (pkTarget->GetRefineLevel() != pkItem->GetValue(1))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ???????? ?????? ?? ???????."));
					return false;
				}

				bRefineType = REFINE_TYPE_NOT_USED1;
			}
			break;

			case BDRAGON_SCROLL:
			{
				if (pkTarget->GetType() != ITEM_METIN || pkTarget->GetRefineLevel() != 4)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ?????????? ?????? ?? ???????."));
					return false;
				}

				if (pkTarget->GetRefineSet() != 702)
					return false;
			}
			break;

#if defined(__STONE_OF_BLESS__)
			case BLESSING_STONE:
			{
				if (pkTarget->GetLevelLimit() <= 80)
					return false;

				bRefineType = REFINE_TYPE_BLESSING_STONE;
			}
			break;

			case MALL_BLESSING_STONE:
				bRefineType = REFINE_TYPE_BLESSING_STONE;
				break;
#endif

#if defined(__SOUL_SYSTEM__)
			case SOUL_AWAKE_SCROLL:
			case SOUL_EVOLVE_SCROLL:
			{
				if (pkTarget->GetType() != ITEM_SOUL)
					return false;

				if (pkTarget->GetValue(0) >= SOUL_GRADE_ILUMINED)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("[Soul System] %s is already at the highest level.", LC_ITEM(pkTarget->GetVnum())));
					return false;
				}

				if (pkItem->GetValue(0) == SOUL_AWAKE_SCROLL)
					bRefineType = REFINE_TYPE_SOUL_AWAKE;
				else if (pkItem->GetValue(0) == SOUL_EVOLVE_SCROLL)
					bRefineType = REFINE_TYPE_SOUL_EVOLVE;
			}
			break;
#endif

			default:
			{
				if (pkTarget->GetRefineSet() == 501)
					return false;

				if (pkTarget->GetVnum() >= 28330 && pkTarget->GetVnum() <= 28343) // ????+3
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("+3 ?????? ?? ?????????? ?????? ?? ???????"));
					return false;
				}

				if (pkTarget->GetVnum() >= 28430 && pkTarget->GetVnum() <= 28443) // ????+4
				{
					if (pkItem->GetVnum() != 71056) // ????????
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ?? ?????????? ?????? ?? ???????"));
						return false;
					}
				}

				bRefineType = REFINE_TYPE_SCROLL;
			}
			break;
		}

		RefineInformation(pkTarget->GetCell(), bRefineType, pkItem->GetCell());
	}
	else if (pkItem->GetSubType() == USE_DETACHMENT && IS_SET(pkTarget->GetFlag(), ITEM_FLAG_REFINEABLE) && pkTarget->IsRemovableSocket())
	{
		LogManager::instance().ItemLog(this, pkTarget, "USE_DETACHMENT", pkTarget->GetName());

		bool bHasMetinStone = false;

		for (int i = 0; i < METIN_SOCKET_MAX_NUM; i++)
		{
			long socket = pkTarget->GetSocket(i);
			if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
			{
				bHasMetinStone = true;
				break;
			}
		}

		if (bHasMetinStone)
		{
			for (int i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
			{
				long socket = pkTarget->GetSocket(i);
				if (socket > 2 && socket != ITEM_BROKEN_METIN_VNUM)
				{
#if defined(__GLOVE_SYSTEM__)
					if (pkTarget->IsGlove() && socket >= 1000000)
					{
						DWORD dwBaseIndex = 28046;
						dwBaseIndex += (((socket / 1000) % 10) * 100);
						dwBaseIndex += ((socket / 100) % 10) - 1;

						const TItemTable* pItemData = ITEM_MANAGER::instance().GetTable(dwBaseIndex);
						if (pItemData == nullptr)
							continue;

						socket = dwBaseIndex;
					}
#endif
					AutoGiveItem(socket);
					//TItemTable* pTable = ITEM_MANAGER::instance().GetTable(pkTarget->GetSocket(i));
					//pkTarget->SetSocket(i, pTable->alValues[2]);
					// ???????? ????????
					pkTarget->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
				}
			}
			pkItem->SetCount(pkItem->GetCount() - 1);
			return true;
		}
		else
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ?? ??? ??????? ???????."));
			return false;
		}
	}

	return false;
}

EVENTFUNC(kill_campfire_event)
{
	char_event_info* info = dynamic_cast<char_event_info*>(event->info);

	if (info == NULL)
	{
		sys_err("kill_campfire_event> <Factor> Null pointer");
		return 0;
	}

	LPCHARACTER ch = info->ch;

	if (ch == NULL) // <Factor>
		return 0;

	ch->m_pkMiningEvent = NULL;
	M2_DESTROY_CHARACTER(ch);
	return 0;
}

bool CHARACTER::GiveRecallItem(LPITEM item)
{
	int idx = GetMapIndex();
	int iEmpireByMapIndex = -1;

	if (idx < 20)
		iEmpireByMapIndex = 1;
	else if (idx < 40)
		iEmpireByMapIndex = 2;
	else if (idx < 60)
		iEmpireByMapIndex = 3;
	else if (idx < 10000)
		iEmpireByMapIndex = 0;

	switch (idx)
	{
		case 66:
		case 216:
		case 301:
		case 302:
		case 303:
		case 304:
			iEmpireByMapIndex = -1;
			break;
	}

	if (iEmpireByMapIndex && GetEmpire() != iEmpireByMapIndex)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ?? ?? ???? ??? ????."));
		return false;
	}

	int pos;

	if (item->GetCount() == 1) // ???????? ?????? ??? ????.
	{
		item->SetSocket(0, GetX());
		item->SetSocket(1, GetY());
	}
	else if ((pos = GetEmptyInventory(item->GetSize())) != -1) // ????? ???? ??? ????? ?????? ??.
	{
		LPITEM item2 = ITEM_MANAGER::instance().CreateItem(item->GetVnum(), 1);
		if (item2 != nullptr)
		{
			item2->SetSocket(0, GetX());
			item2->SetSocket(1, GetY());
			item2->AddToCharacter(this, TItemPos(INVENTORY, pos));

			item->SetCount(item->GetCount() - 1);
		}
	}
	else
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ?? ?????? ???????."));
		return false;
	}

	return true;
}

void CHARACTER::ProcessRecallItem(LPITEM item)
{
	int idx;

	if ((idx = SECTREE_MANAGER::instance().GetMapIndex(item->GetSocket(0), item->GetSocket(1))) == 0)
		return;

	int iEmpireByMapIndex = -1;

	if (idx < 20)
		iEmpireByMapIndex = 1;
	else if (idx < 40)
		iEmpireByMapIndex = 2;
	else if (idx < 60)
		iEmpireByMapIndex = 3;
	else if (idx < 10000)
		iEmpireByMapIndex = 0;

	switch (idx)
	{
		case 66:
		case 216:
			iEmpireByMapIndex = -1;
			break;
			// ????? ???
		case 301:
		case 302:
		case 303:
		case 304:
			if (GetLevel() < 90)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("???????? ???? ??????? ?????? ???????."));
				return;
			}
			else
				break;
	}

	if (iEmpireByMapIndex && GetEmpire() != iEmpireByMapIndex)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ????? ??????? ???? ??? ????? ?? ???????."));
		item->SetSocket(0, 0);
		item->SetSocket(1, 0);
	}
	else
	{
		sys_log(1, "Recall: %s %d %d -> %d %d", GetName(), GetX(), GetY(), item->GetSocket(0), item->GetSocket(1));
		WarpSet(item->GetSocket(0), item->GetSocket(1));
		item->SetCount(item->GetCount() - 1);
	}
}

void CHARACTER::__OpenPrivateShop(
#if defined(__MYSHOP_DECO__)
	BYTE bTabCount, bool bIsCashItem
#endif
)
{
/* Unnecessary check IsActivePet
#if defined(__GROWTH_PET_SYSTEM__)
	if (GetGrowthPetSystem()->IsActivePet())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot open a private shop whilst summoning your pet."));
		return;
	}
#endif
*/

	if (IsRiding())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this bundle whilst riding."));
		return;
	}

	DWORD dwBodyPart = GetPart(PART_MAIN);
	switch (dwBodyPart)
	{
		case 0:
		case 1:
		case 2:
		{
#if defined(__MYSHOP_DECO__)
			OpenPrivateShop(bTabCount, bIsCashItem);
#else
			ChatPacket(CHAT_TYPE_COMMAND, "OpenPrivateShop");
#endif
		}
		break;

		default:
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ????? ???? ?????? ?? ?? ??????."));
			break;
	}
}

// MYSHOP_PRICE_LIST

void CHARACTER::SendMyShopPriceListCmd(DWORD dwItemVnum, DWORD dwItemPrice
#if defined(__CHEQUE_SYSTEM__)
	, DWORD dwItemCheque
#endif
)
{
	char szLine[256];
#if defined(__CHEQUE_SYSTEM__)
	snprintf(szLine, sizeof(szLine), "MyShopPriceList %u %u %u", dwItemVnum, dwItemPrice, dwItemCheque);
#else
	snprintf(szLine, sizeof(szLine), "MyShopPriceList %u %u", dwItemVnum, dwItemPrice);
#endif
	ChatPacket(CHAT_TYPE_COMMAND, szLine);
	sys_log(0, szLine);
}

//
// DB
//
void CHARACTER::UseSilkBotaryReal(const TPacketMyshopPricelistHeader* p)
{
	const TItemPriceInfo* pInfo = (const TItemPriceInfo*)(p + 1);

#if defined(__CHEQUE_SYSTEM__)
	if (!p->byCount)
		SendMyShopPriceListCmd(1, 0, 0);
	else
	{
		for (int idx = 0; idx < p->byCount; idx++)
			SendMyShopPriceListCmd(pInfo[idx].dwVnum, pInfo[idx].dwPrice, pInfo[idx].dwCheque);
	}
#else
	if (!p->byCount)
		SendMyShopPriceListCmd(1, 0);
	else {
		for (int idx = 0; idx < p->byCount; idx++)
			SendMyShopPriceListCmd(pInfo[idx].dwVnum, pInfo[idx].dwPrice);
	}
#endif

#if defined(__MYSHOP_DECO__)
	__OpenPrivateShop(MYSHOP_MAX_TABS /* bTabCount */, true /* bIsCashItem */);
#else
	__OpenPrivateShop();
#endif
}

void CHARACTER::UseSilkBotary(void)
{
	if (m_bNoOpenedShop)
	{
		DWORD dwPlayerID = GetPlayerID();
		db_clientdesc->DBPacket(HEADER_GD_MYSHOP_PRICELIST_REQ, GetDesc()->GetHandle(), &dwPlayerID, sizeof(DWORD));
		m_bNoOpenedShop = false;
	}
	else
	{
#if defined(__MYSHOP_DECO__)
		__OpenPrivateShop(MYSHOP_MAX_TABS /* bTabCount */, true /* bIsCashItem */);
#else
		__OpenPrivateShop();
#endif
	}
}
// END_OF_MYSHOP_PRICE_LIST

int CalculateConsume(LPCHARACTER ch)
{
	static const int WARP_NEED_LIFE_PERCENT = 30;
	static const int WARP_MIN_LIFE_PERCENT = 10;
	// CONSUME_LIFE_WHEN_USE_WARP_ITEM
	int consumeLife = 0;
	{
		// CheckNeedLifeForWarp
		const int curLife = ch->GetHP();
		const int needPercent = WARP_NEED_LIFE_PERCENT;
		const int needLife = ch->GetMaxHP() * needPercent / 100;
		if (curLife < needLife)
		{
			ch->ChatPacket(CHAT_TYPE_INFO, LC_STRING("You don't have enough HP."));
			return -1;
		}

		consumeLife = needLife;

		// CheckMinLifeForWarp
		const int minPercent = WARP_MIN_LIFE_PERCENT;
		const int minLife = ch->GetMaxHP() * minPercent / 100;
		if (curLife - needLife < minLife)
			consumeLife = curLife - minLife;

		if (consumeLife < 0)
			consumeLife = 0;
	}
	// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM
	return consumeLife;
}

int CalculateConsumeSP(LPCHARACTER lpChar)
{
	static const int NEED_WARP_SP_PERCENT = 30;

	const int curSP = lpChar->GetSP();
	const int needSP = lpChar->GetMaxSP() * NEED_WARP_SP_PERCENT / 100;

	if (curSP < needSP)
	{
		lpChar->ChatPacket(CHAT_TYPE_INFO, LC_STRING("You don't have enough Spell Points (SP) to use this."));
		return -1;
	}

	return needSP;
}

bool CHARACTER::UseItemEx(LPITEM item, TItemPos DestCell)
{
	int iLimitRealtimeStartFirstUseFlagIndex = -1;
	int iLimitTimerBasedOnWearFlagIndex = -1;

	WORD wDestCell = DestCell.cell;
	BYTE bDestInven = DestCell.window_type;

	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		long limitValue = item->GetProto()->aLimits[i].lValue;

		switch (item->GetProto()->aLimits[i].bType)
		{
			case LIMIT_LEVEL:
			{
				if (GetLevel() < limitValue)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("Your level is too low to use this item."));
					return false;
				}
			}
			break;

#if defined(__CONQUEROR_LEVEL__)
			case LIMIT_NEWWORLD_LEVEL:
			{
				if (GetConquerorLevel() < limitValue)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("???????? ???? ??????? ?????? ???????."));
					return false;
				}
			}
			break;
#endif

			case LIMIT_REAL_TIME_START_FIRST_USE:
				iLimitRealtimeStartFirstUseFlagIndex = i;
				break;

			case LIMIT_TIMER_BASED_ON_WEAR:
				iLimitTimerBasedOnWearFlagIndex = i;
				break;
		}
	}

	if (test_server)
	{
		sys_log(0, "USE_ITEM %s, Inven %d, Cell %d, ItemType %d, SubType %d", item->GetName(), bDestInven, wDestCell, item->GetType(), item->GetSubType());
	}

	if (CArenaManager::instance().IsLimitedItem(GetMapIndex(), item->GetVnum()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this item in a duel."));
		return false;
	}

	// After the initial use of an item, time is deducted even if it is not used.
	if (-1 != iLimitRealtimeStartFirstUseFlagIndex
#if defined(__GROWTH_PET_SYSTEM__)
		&& item->GetType() != ITEM_PET
#endif
		)
	{
		if (0 == item->GetSocket(1))
		{
			long duration = (0 != item->GetSocket(0)) ? item->GetSocket(0) : item->GetProto()->aLimits[iLimitRealtimeStartFirstUseFlagIndex].lValue;

			if (0 == duration)
				duration = 60 * 60 * 24 * 7;

			item->SetSocket(0, time(0) + duration);
			item->StartRealTimeExpireEvent();
		}

		if (false == item->IsEquipped())
			item->SetSocket(1, item->GetSocket(1) + 1);
	}

#if defined(__MAILBOX__)
	if (item->GetVnum() == ITEM_MOBILE_MAILBOX)
	{
		CMailBox::Open(this);
		return true;
	}
#endif

#if defined(__EXTEND_INVEN_SYSTEM__)
	if (item->GetVnum() == ITEM_EXTEND_INVEN_TICKET ||
		item->GetVnum() == ITEM_EXTEND_INVEN_TICKET_MALL)
	{
		ExtendInvenRequest();
		return true;
	}
#endif

#if defined(ENABLE_AUTO_SYSTEM)
	// Test: 38014 -> PREMIUM_AUTO_USE +7 gun (sunucu otomatik av izni).
	if (item->GetVnum() == 38014)
	{
		const int kSevenDaysSec = 60 * 60 * 24 * 7;
		AddPremiumSeconds(PREMIUM_AUTO_USE, kSevenDaysSec);
		// Premium suresini DB'ye de yaz (client kapat/ac ile kaybolmasin).
		// account.account.auto_use alanina mevcut bitise ekler (yoksa simdiden baslatir).
		DBManager::instance().DirectQuery(
			"UPDATE account.account "
			"SET auto_use = DATE_ADD(GREATEST(IFNULL(auto_use, CURRENT_TIMESTAMP()), CURRENT_TIMESTAMP()), INTERVAL 7 DAY) "
			"WHERE id = %u;",
			GetAID());
		item->SetCount(item->GetCount() - 1);
		ChatPacket(CHAT_TYPE_INFO, "Otomatik Av: 7 gun sure eklendi.");
		return true;
	}
#endif

	switch (item->GetType())
	{
		case ITEM_HAIR:
			return ItemProcess_Hair(item, wDestCell);

		case ITEM_POLYMORPH:
			return ItemProcess_Polymorph(item);

		case ITEM_QUEST:
		{
			if (GetArena() != NULL || IsObserverMode() == true)
			{
				if (item->GetVnum() == ITEM_VNUM_HORSE_PICTURE || item->GetVnum() == ITEM_VNUM_ARMED_HORSE_BOOK || item->GetVnum() == ITEM_VNUM_MILITARY_HORSE_BOOK)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this item in a duel."));
					return false;
				}
			}

			if (!IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE))
			{
				if (item->GetSIGVnum() == 0)
				{
					quest::CQuestManager::instance().UseItem(GetPlayerID(), item, false);
				}
				else
				{
					quest::CQuestManager::instance().SIGUse(GetPlayerID(), item->GetSIGVnum(), item, false);
				}
			}
		}
		break;

		case ITEM_CAMPFIRE:
		{
			float fx, fy;
			GetDeltaByDegree(GetRotation(), 100.0f, &fx, &fy);

			LPSECTREE tree = SECTREE_MANAGER::instance().Get(GetMapIndex(), (long)(GetX() + fx), (long)(GetY() + fy));

			if (!tree)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot build a campfire here."));
				return false;
			}

			if (tree->IsAttr((long)(GetX() + fx), (long)(GetY() + fy), ATTR_WATER))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot build a campfire under water."));
				return false;
			}

			LPCHARACTER campfire = CHARACTER_MANAGER::instance().SpawnMob(fishing::CAMPFIRE_MOB, GetMapIndex(), (long)(GetX() + fx), (long)(GetY() + fy), 0, false, number(0, 359));

			char_event_info* info = AllocEventInfo<char_event_info>();

			info->ch = campfire;

			campfire->m_pkMiningEvent = event_create(kill_campfire_event, info, PASSES_PER_SEC(40));

			item->SetCount(item->GetCount() - 1);
		}
		break;

		case ITEM_UNIQUE:
		{
			switch (item->GetSubType())
			{
				case USE_ABILITY_UP:
				{
					if (FindAffect(AFFECT_BLEND, item->GetValue(0)))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ??? ??????."));
						return false;
					}

					switch (item->GetValue(0))
					{
						case APPLY_MOV_SPEED:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_MOV_SPEED,
								item->GetValue(2), AFF_MOV_SPEED_POTION, item->GetValue(1), 0, true, true);
							break;

						case APPLY_ATT_SPEED:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ATT_SPEED,
								item->GetValue(2), AFF_ATT_SPEED_POTION, item->GetValue(1), 0, true, true);
							break;

						case APPLY_STR:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ST,
								item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_DEX:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_DX,
								item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_CON:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_HT,
								item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_INT:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_IQ,
								item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_CAST_SPEED:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_CASTING_SPEED,
								item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_RESIST_MAGIC:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_RESIST_MAGIC,
								item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_ATT_GRADE_BONUS:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ATT_GRADE_BONUS,
								item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;

						case APPLY_DEF_GRADE_BONUS:
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_DEF_GRADE_BONUS,
								item->GetValue(2), 0, item->GetValue(1), 0, true, true);
							break;
					}

					if (GetDungeon())
						GetDungeon()->UsePotion(this);

					if (GetWarMap())
						GetWarMap()->UsePotion(this, item);

					item->SetCount(item->GetCount() - 1);
				}
				break;

				case UNIQUE_BUNDLE:
				{
					if (IsRiding())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this bundle whilst riding."));
						return false;
					}

					if (GetWear(ARMOR_BODY))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ????? ???? ?????? ?? ?? ??????."));
						return false;
					}

					switch (item->GetVnum())
					{
						case 71049: // ????????
						{
							if (LC_IsYMIR() == true || LC_IsKorea() == true)
							{
								if (IS_BOTARYABLE_ZONE(GetMapIndex()) == true)
								{
									UseSilkBotary();
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ?????? ?? ?? ???? ????????"));
								}
							}
							else
							{
								UseSilkBotary();
							}
						}
						break;

#if defined(__MYSHOP_DECO__)
						case ITEM_KASHMIR_BUNDLE:
							ChatPacket(CHAT_TYPE_COMMAND, "OpenMyShopDecoWnd");
							break;
#endif
					}
				}
				break;

				default:
				{
					if (!item->IsEquipped())
						EquipItem(item);
					else
						UnequipItem(item);
				}
				break;
			}
		}
		break;

		case ITEM_COSTUME:
		case ITEM_WEAPON:
		case ITEM_ARMOR:
		case ITEM_ROD:
		case ITEM_RING:
		case ITEM_BELT:
		case ITEM_PICK: // MINING
#if defined(ENABLE_PASSIVE_SYSTEM)
		case ITEM_PASSIVE:
#endif
		{
			if (!item->IsEquipped())
				EquipItem(item);
			else
				UnequipItem(item);
		}
		break;

		/*
		* Dragon Spirit Stones that are not worn cannot be used.
		* If it's a normal cla, you can't send an item use packet about Dragon Spirit
		* Wearing Dragon Spirit Stone is done by item move packet.
		* Extract the worn dragon spirit stone.
		*/
#if defined(__DRAGON_SOUL_SYSTEM__)
		case ITEM_DS:
		{
			if (!item->IsEquipped())
				return false;

			return DSManager::instance().PullOut(this, NPOS, item);
		}

		case ITEM_SPECIAL_DS:
		{
			if (!item->IsEquipped())
				EquipItem(item);
			else
				UnequipItem(item);
		}
		break;
#endif

		case ITEM_FISH:
		{
			if (CArenaManager::instance().IsArenaMap(GetMapIndex()))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this item in a duel."));
				return false;
			}

			if (item->GetSubType() == FISH_ALIVE)
				fishing::UseFish(this, item);
		}
		break;

		case ITEM_TREASURE_BOX:
		{
			return false;
			//ChatPacket(CHAT_TYPE_TALKING, LC_STRING("Closed. You should look for the key."));
		}
		break;

		case ITEM_TREASURE_KEY:
		{
			LPITEM item2;

			if (!GetItem(DestCell) || !(item2 = GetItem(DestCell)))
				return false;

			if (item2->IsExchanging())
				return false;

			if (item2->GetType() != ITEM_TREASURE_BOX)
			{
				ChatPacket(CHAT_TYPE_TALKING, LC_STRING("????? ???? ?????? ???? ????."));
				return false;
			}

			if (item->GetValue(0) == item2->GetValue(0))
			{
				//ChatPacket(CHAT_TYPE_TALKING, LC_STRING("????? ?????? ?????? ??? ??? ?????? ?????????."));

				if (GiveItemFromSpecialItemGroup(item2->GetVnum()))
				{
					item->SetCount(item->GetCount() - 1);
					item2->SetCount(item2->GetCount() - 1);
				}
				else
				{
					ChatPacket(CHAT_TYPE_TALKING, LC_STRING("???? ???? ??? ?? ????."));
					return false;
				}
			}
			else
			{
				ChatPacket(CHAT_TYPE_TALKING, LC_STRING("???? ???? ??? ?? ????."));
				return false;
			}
		}
		break;

		case ITEM_GIFTBOX:
		{
			DWORD dwBoxVnum = item->GetVnum();

			if (dwBoxVnum == 50033 && LC_IsYMIR()) // ??????? ????
			{
				if (GetLevel() < 15)
				{
					ChatPacket(CHAT_TYPE_INFO, "15???? ????????? ????? ?? ???????.");
					return false;
				}
			}

#if defined(__DRAGON_SOUL_SYSTEM__)
			if ((dwBoxVnum > 51500 && dwBoxVnum < 52000) || (dwBoxVnum >= 50255 && dwBoxVnum <= 50260)) // ?????????
			{
				if (!(this->DragonSoul_IsQualified()))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ????? ??????? ??????? ????."));
					return false;
				}
			}
#endif

			if (!GiveItemFromSpecialItemGroup(dwBoxVnum))
			{
				ChatPacket(CHAT_TYPE_TALKING, LC_STRING("?????? ???? ?? ?????????."));
				return false;
			}

			item->SetCount(item->GetCount() - 1);
		}
		break;

		case ITEM_SKILLFORGET:
		{
			if (!item->GetSocket(0))
			{
				ITEM_MANAGER::instance().RemoveItem(item);
				return false;
			}

			DWORD dwVnum = item->GetSocket(0);

			if (SkillLevelDown(dwVnum))
			{
				ITEM_MANAGER::instance().RemoveItem(item);
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ?????? ??????? ????????????."));
			}
			else
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ?????? ???? ?? ???????."));
		}
		break;

		case ITEM_SKILLBOOK:
		{
			if (IsPolymorphed())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("????????? ? ?????? ???????."));
				return false;
			}

			DWORD dwVnum = item->GetValue(0);

			if (0 == dwVnum && item->GetVnum() == ITEM_SKILLBOOK_VNUM)
				dwVnum = item->GetSocket(0);

			if (0 == dwVnum)
			{
				ITEM_MANAGER::instance().RemoveItem(item);
				return false;
			}

			if (true == LearnSkillByBook(dwVnum))
			{
				item->SetCount(item->GetCount() - 1);

				int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);

				if (distribution_test_server)
					iReadDelay /= 3;

				// ??? ?????? ????? ??? 24?? ????
				if (LC_IsKorea())
					iReadDelay = 86400;

				SetSkillNextReadTime(dwVnum, get_global_time() + iReadDelay);
			}
		}
		break;

		case ITEM_USE:
		{
			if (item->GetVnum() > 50800 && item->GetVnum() <= 50820)
			{
				if (test_server)
					sys_log(0, "ADD addtional effect : vnum(%d) subtype(%d)", item->GetOriginalVnum(), item->GetSubType());

				int affect_type = AFFECT_EXP_BONUS_EURO_FREE;
				int apply_type = aApplyInfo[item->GetValue(0)].wPointType;
				int apply_value = item->GetValue(2);
				int apply_duration = item->GetValue(1);

				switch (item->GetSubType())
				{
					case USE_ABILITY_UP:
					{
						if (FindAffect(affect_type, apply_type))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ??? ??????."));
							return false;
						}

						if (FindAffect(AFFECT_BLEND, item->GetValue(0)))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ??? ??????."));
							return false;
						}

						switch (item->GetValue(0))
						{
							case APPLY_MOV_SPEED:
							{
								AddAffect(affect_type, apply_type, apply_value, AFF_MOV_SPEED_POTION, apply_duration, 0, true, true);
								break;
							}

							case APPLY_ATT_SPEED:
							{
								AddAffect(affect_type, apply_type, apply_value, AFF_ATT_SPEED_POTION, apply_duration, 0, true, true);
								break;
							}

							case APPLY_STR:
							case APPLY_DEX:
							case APPLY_CON:
							case APPLY_INT:
							case APPLY_CAST_SPEED:
							case APPLY_CRITICAL_PCT:
							case APPLY_PENETRATE_PCT:
							case APPLY_RESIST_MAGIC:
							case APPLY_ATT_GRADE_BONUS:
							case APPLY_DEF_GRADE_BONUS:
#if defined(__CONQUEROR_LEVEL__)
							case APPLY_SUNGMA_STR:
							case APPLY_SUNGMA_HP:
							case APPLY_SUNGMA_MOVE:
							case APPLY_SUNGMA_IMMUNE:
#endif
							{
								AddAffect(affect_type, apply_type, apply_value, 0, apply_duration, 0, true, true);
								break;
							}

						}

						if (GetDungeon())
							GetDungeon()->UsePotion(this);

						if (GetWarMap())
							GetWarMap()->UsePotion(this, item);

						item->SetCount(item->GetCount() - 1);
					}
					break;

					case USE_AFFECT:
					{
						if (FindAffect(AFFECT_EXP_BONUS_EURO_FREE, aApplyInfo[item->GetValue(1)].wPointType))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ??? ??????."));
						}
						else
						{
							AddAffect(AFFECT_EXP_BONUS_EURO_FREE, aApplyInfo[item->GetValue(1)].wPointType, item->GetValue(2), 0, item->GetValue(3), 0, false, true);
							item->SetCount(item->GetCount() - 1);
						}
					}
					break;

					case USE_POTION_NODELAY:
					{
						if (CArenaManager::instance().IsArenaMap(GetMapIndex()))
						{
							if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit") > 0)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ?????? ?? ???????."));
								return false;
							}

							switch (item->GetVnum())
							{
								case 70020: // Peach Flower Wine
								case 71018: // Blessing of Life
								case 71019: // Blessing of Magic
								case 71020: // Blessing of the Dragon
								{
									if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit_count") < 10000)
									{
										if (m_nPotionLimit <= 0)
										{
											ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ??????? ???????????."));
											return false;
										}
									}
								}
								break;

								default:
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ?????? ?? ???????."));
									return false;
								}
							}
						}

						bool bUsed = false;

						if (item->GetValue(0) != 0) // HP ???? ???
						{
							if (GetHP() < GetMaxHP())
							{
								PointChange(POINT_HP, item->GetValue(0) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
								EffectPacket(SE_HPUP_RED);
								bUsed = true;
							}
						}

						if (item->GetValue(1) != 0) // SP ???? ???
						{
							if (GetSP() < GetMaxSP())
							{
								PointChange(POINT_SP, item->GetValue(1) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
								EffectPacket(SE_SPUP_BLUE);
								bUsed = true;
							}
						}

						if (item->GetValue(3) != 0) // HP % ???
						{
							if (GetHP() < GetMaxHP())
							{
								PointChange(POINT_HP, item->GetValue(3) * GetMaxHP() / 100);
								EffectPacket(SE_HPUP_RED);
								bUsed = true;
							}
						}

						if (item->GetValue(4) != 0) // SP % ???
						{
							if (GetSP() < GetMaxSP())
							{
								PointChange(POINT_SP, item->GetValue(4) * GetMaxSP() / 100);
								EffectPacket(SE_SPUP_BLUE);
								bUsed = true;
							}
						}

						if (bUsed)
						{
							if (item->GetVnum() == 50085 || item->GetVnum() == 50086)
							{
								if (test_server)
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ??? ???? ?? ???????????"));

								SetUseSeedOrMoonBottleTime();
							}

							if (GetDungeon())
								GetDungeon()->UsePotion(this);

							if (GetWarMap())
								GetWarMap()->UsePotion(this, item);

							m_nPotionLimit--;

							// RESTRICT_USE_SEED_OR_MOONBOTTLE
							item->SetCount(item->GetCount() - 1);
							// END_RESTRICT_USE_SEED_OR_MOONBOTTLE
						}
					}
					break;
				}

				return true;
			}

			if (item->GetVnum() >= 27863 && item->GetVnum() <= 27883)
			{
				if (CArenaManager::instance().IsArenaMap(GetMapIndex()))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????? ?? ???? ???????."));
					return false;
				}
			}

			if (test_server)
			{
				sys_log(0, "USE_ITEM %s Type %d SubType %d vnum %d", item->GetName(), item->GetType(), item->GetSubType(), item->GetOriginalVnum());
			}

			switch (item->GetSubType())
			{
#if defined(__REFINE_ELEMENT_SYSTEM__)
				case USE_ELEMENT_UPGRADE:
				case USE_ELEMENT_DOWNGRADE:
				case USE_ELEMENT_CHANGE:
					RefineElementInformation(item, DestCell);
					break;
#endif

#if defined(__EXPRESSING_EMOTIONS__)
				case USE_EMOTION_PACK:
				{
					AddEmote();
					item->SetCount(item->GetCount() - 1);
				}
				break;
#endif

#if defined(__DRAGON_SOUL_SYSTEM__)
				case USE_TIME_CHARGE_PER:
				{
					LPITEM pDestItem = GetItem(DestCell);
					if (NULL == pDestItem)
						return false;

					// ?? ??????? ??????? ????? ???.
					if (pDestItem->IsDragonSoul())
					{
						int ret;
						char buf[128];
						if (item->GetVnum() == ITEM_DRAGON_HEART_VNUM)
						{
							ret = pDestItem->GiveMoreTime_Per((float)item->GetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX));
						}
						else
						{
							ret = pDestItem->GiveMoreTime_Per((float)item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
						}

						if (ret > 0)
						{
#if defined(__DS_SET__)
							DragonSoul_SetBonus();
#endif

							if (item->GetVnum() == ITEM_DRAGON_HEART_VNUM)
							{
								sprintf(buf, "Inc %ds by item{VN:%d SOC%d:%d}", ret, item->GetVnum(), ITEM_SOCKET_CHARGING_AMOUNT_IDX, item->GetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX));
							}
							else
							{
								sprintf(buf, "Inc %ds by item{VN:%d VAL%d:%d}", ret, item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
							}

							ChatPacket(CHAT_TYPE_INFO, LC_STRING("%d?? ??? ????????????.", ret));
							item->SetCount(item->GetCount() - 1);
							LogManager::instance().ItemLog(this, item, "DS_CHARGING_SUCCESS", buf);

							return true;
						}
						else
						{
							if (item->GetVnum() == ITEM_DRAGON_HEART_VNUM)
							{
								sprintf(buf, "No change by item{VN:%d SOC%d:%d}", item->GetVnum(), ITEM_SOCKET_CHARGING_AMOUNT_IDX, item->GetSocket(ITEM_SOCKET_CHARGING_AMOUNT_IDX));
							}
							else
							{
								sprintf(buf, "No change by item{VN:%d VAL%d:%d}", item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
							}

							ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ?? ???????."));
							LogManager::instance().ItemLog(this, item, "DS_CHARGING_FAILED", buf);
							return false;
						}
					}
					else
						return false;
				}
				break;

				case USE_TIME_CHARGE_FIX:
				{
					LPITEM pDestItem = GetItem(DestCell);
					if (NULL == pDestItem)
						return false;

					// ?? ??????? ??????? ????? ???.
					if (pDestItem->IsDragonSoul())
					{
						int ret = pDestItem->GiveMoreTime_Fix(item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
						char buf[128];
						if (ret)
						{
#if defined(__DS_SET__)
							DragonSoul_SetBonus();
#endif

							ChatPacket(CHAT_TYPE_INFO, LC_STRING("%d?? ??? ????????????.", ret));

							sprintf(buf, "Increase %ds by item{VN:%d VAL%d:%d}", ret, item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
							LogManager::instance().ItemLog(this, item, "DS_CHARGING_SUCCESS", buf);
							item->SetCount(item->GetCount() - 1);

							return true;
						}
						else
						{
							ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ?? ???????."));
							sprintf(buf, "No change by item{VN:%d VAL%d:%d}", item->GetVnum(), ITEM_VALUE_CHARGING_AMOUNT_IDX, item->GetValue(ITEM_VALUE_CHARGING_AMOUNT_IDX));
							LogManager::instance().ItemLog(this, item, "DS_CHARGING_FAILED", buf);
							return false;
						}
					}
					else
						return false;
				}
				break;
#endif

#if defined(__NPC_LOCATION_HELPER__)
				case USE_MAP:
				{
					if (item->GetVnum() >= 79800 && item->GetVnum() <= 79822)
					{
						if (CNpcLocationHelperManager::instance().UnlockMapByItem(this, item->GetVnum(), item->GetValue(0)))
							item->SetCount(item->GetCount() - 1);
						return true;
					}
				}
				break;
#endif
				case USE_SPECIAL:
				{
					switch (item->GetVnum())
					{
#if defined(ENABLE_TREASURE_HUNT)
						case TREASURE_HUNT_ITEM_TICKET:
							if (CTreasureHuntManager::instance().TryEnterDungeon(this, item))
								return true;
							break;
#endif
#if defined(__GUILD_DRAGONLAIR_SYSTEM__)
						case ITEM_GUILD_DRAGONLAIR_POTION:
						{
							if (GetGuildDragonLair() == NULL)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use that here."));
								return false;
							}

							if (FindAffect(AFFECT_RED_DRAGONLAIR_BUFF))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ??? ??????."));
								return false;
							}
							else
							{
								const long duration = item->GetValue(1);

								AddAffect(AFFECT_RED_DRAGONLAIR_BUFF, POINT_SKILL_DEFEND_BONUS, 15, AFF_NONE, duration, 0, true, true);
								AddAffect(AFFECT_RED_DRAGONLAIR_BUFF, POINT_NORMAL_HIT_DEFEND_BONUS, 15, AFF_NONE, duration, 0, true, true);

								item->SetCount(item->GetCount() - 1);
							}
						}
						break;
#endif

#if defined(__ELEMENTAL_DUNGEON__)
						case ITEM_PRIMAL_FORCE_MEDAL:
						{
							if (IS_ELEMENTAL_DUNGEON(GetMapIndex()) == false)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use that here."));
								return false;
							}

							CAffect* pAffect = FindAffect(AFFECT_PROTECTION_OF_ELEMENTAL);
							if (pAffect)
							{
								// NOTE : Check if adding another 600 seconds (10 minutes) would exceed 3600 seconds (60 minutes).
								const long lDuration = pAffect->lDuration;
								if (lDuration + 600 > 3600)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use that any more."));
									return false;
								}

								AddAffect(AFFECT_PROTECTION_OF_ELEMENTAL, APPLY_NONE, 0, AFF_NONE, lDuration + 600, 0, true);
							}
							else
								AddAffect(AFFECT_PROTECTION_OF_ELEMENTAL, APPLY_NONE, 0, AFF_NONE, 600, 0, true);

							item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_ELEMENTAL_DEFENSE_POTION:
						{
							if (IS_ELEMENTAL_DUNGEON(GetMapIndex()) == false)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use that here."));
								return false;
							}

							const long duration = item->GetValue(1);
							static const std::unordered_map<POINT_TYPE, POINT_VALUE> map_affect = {
								{ APPLY_MELEE_MAGIC_ATTBONUS_PER, 5 },
								{ APPLY_RESIST_FIRE, 10 },
								{ APPLY_RESIST_ELEC, 10 },
								{ APPLY_RESIST_ICE, 10 },
								{ APPLY_RESIST_EARTH, 10 },
								{ APPLY_RESIST_DARK, 10 },
							};
							for (const auto& it : map_affect)
								AddAffect(AFFECT_POTION_OF_ELEMENTAL, it.first, it.second, AFF_NONE, duration, 0, true, true);

							item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_PRIMAL_FORCE_RING:
						case ITEM_PRIMAL_FORCE_RING_MALL:
						{
							if (IS_ELEMENTAL_DUNGEON(GetMapIndex()) == false)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use that here."));
								return false;
							}

							if (number(0, 100) < item->GetValue(0))
							{
								PIXEL_POSITION WarpPos;
								if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(MAP_ELEMENTAL_04, GetEmpire(), WarpPos))
									WarpSet(WarpPos.x, WarpPos.y);
								else
									sys_err("CHARACTER::UseItem : cannot find spawn position (name %s, %d x %d)", GetName(), GetX(), GetY());
							}
							else
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("Teleportation to the Plateau of Primal Forces has failed."));
							
							item->SetCount(item->GetCount() - 1);
						}
						break;
#endif

#if defined(__MINI_GAME_RUMI__) && defined(__OKEY_EVENT_FLAG_RENEWAL__)
						case ITEM_VNUM_RUMI_CARD_PIECE:
						{
							if (CMiniGameRumi::UpdateQuestFlag(this))
								item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_VNUM_RUMI_CARD_PACK:
						{
							const WORD wCardCount = GetQuestFlag("minigame_rumi.card_count");
							if (wCardCount < CMiniGameRumi::RUMI_CARD_COUNT_MAX)
							{
								SetQuestFlag("minigame_rumi.card_count", wCardCount + 1);
								CMiniGameRumi::RequestQuestFlag(this, RUMI_GC_SUBHEADER_SET_CARD_FLAG);

								item->SetCount(item->GetCount() - 1);
							}
							else
							{
								CMiniGameRumi::RequestQuestFlag(this, RUMI_GC_SUBHEADER_NO_MORE_GAIN);
							}
						}
						break;
#endif

#if defined(__MINI_GAME_YUTNORI__) && defined(__YUTNORI_EVENT_FLAG_RENEWAL__)
						case ITEM_VNUM_YUT_PIECE:
						{
							if (CMiniGameYutnori::UpdateQuestFlag(this))
								item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_VNUM_YUT_BOARD:
						{
							const WORD wYutBoardCount = GetQuestFlag("minigame_yutnori.board_count");
							if (wYutBoardCount < CMiniGameYutnori::YUTNORI_BOARD_COUNT_MAX)
							{
								SetQuestFlag("minigame_yutnori.board_count", wYutBoardCount + 1);
								CMiniGameYutnori::RequestQuestFlag(this, YUTNORI_GC_SUBHEADER_SET_YUT_BOARD_FLAG);

								item->SetCount(item->GetCount() - 1);
							}
							else
							{
								CMiniGameYutnori::RequestQuestFlag(this, YUTNORI_GC_SUBHEADER_NO_MORE_GAIN);
							}
						}
						break;
#endif

#if defined(__MINI_GAME_CATCH_KING__) && defined(__CATCH_KING_EVENT_FLAG_RENEWAL__)
						case ITEM_VNUM_CATCH_KING_PIECE:
						{
							if (CMiniGameCatchKing::UpdateQuestFlag(this))
								item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_VNUM_CATCH_KING_PACK:
						{
							const WORD wPackCount = GetQuestFlag("minigame_catchking.pack_count");
							if (wPackCount < CMiniGameCatchKing::CATCH_KING_PACK_COUNT_MAX)
							{
								SetQuestFlag("minigame_catchking.pack_count", wPackCount + 1);
								CMiniGameCatchKing::RequestQuestFlag(this, CATCHKING_GC_SET_CARD_FLAG);

								item->SetCount(item->GetCount() - 1);
							}
							else
							{
								CMiniGameCatchKing::RequestQuestFlag(this, CATCHKING_GC_NO_MORE_GAIN);
							}
						}
						break;
#endif

#if defined(__EASTER_EVENT__)
						case ITEM_MAGIC_EASTER_EGG:
						{
							int nMaxUse = 3;
							int nNextUseTime = 1800;

							int nUseTime = get_global_time() - item->GetSocket(1);
							int nUseCount = item->GetSocket(0);

							if (nUseTime >= nNextUseTime)
							{
								if (GiveItemFromSpecialItemGroup(ITEM_MAGIC_EASTER_EGG))
								{
									item->SetSocket(1, get_global_time());
									if (nUseCount >= (nMaxUse - 1))
										item->SetCount(item->GetCount() - 1);
									else
										item->SetSocket(0, nUseCount + 1);

									ChatPacket(CHAT_TYPE_INFO, LC_STRING("Total %d: You can still unpack %d.", nMaxUse, (nMaxUse - 1) - nUseCount));
								}
								else
									ChatPacket(CHAT_TYPE_TALKING, LC_STRING("?????? ???? ?? ?????????."));
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("Unpack the next gift in %d minutes.", 31 - nUseTime / 60));
							}
						}
						break;
#endif

#if defined(__GEM_CONVERTER__)
						case ITEM_VNUM_GEM_CONVERTER:
						{
							LPITEM pTargetItem;
							if (!IsValidItemPosition(DestCell) || !(pTargetItem = GetItem(DestCell)))
								return false;

							if (pTargetItem->GetVnum() != ITEM_VNUM_GEM_STONE)
								return false;

							if (item->IsExchanging() || pTargetItem->IsExchanging())
								return false;

							if (item->isLocked() || pTargetItem->isLocked())
								return false;

							int iNeedGemStoneCount = item->GetCount() * item->GetValue(0);
							int iConvertToGem = iNeedGemStoneCount / item->GetValue(0);
							int iConvertCost = item->GetCount() * item->GetValue(1);

							if (pTargetItem->GetCount() < iNeedGemStoneCount)
								return false;

							if (GetGold() < iConvertCost)
								return false;

							if (GetGem() + iConvertToGem >= GEM_MAX)
								return false;

							item->SetCount(item->GetCount() - 1);
							pTargetItem->SetCount(pTargetItem->GetCount() - iNeedGemStoneCount);

							PointChange(POINT_GOLD, -iConvertCost);
							PointChange(POINT_GEM, iConvertToGem);
						}
						break;
#endif

#if defined(__SNOWFLAKE_STICK_EVENT__)
						case ITEM_VNUM_SNOWFLAKE_STICK:
						{
							if (CSnowflakeStickEvent::UseStick(this))
							{
								EffectPacket(SE_USE_SNOWFLAKE_STICK);
								item->SetCount(item->GetCount() - 1);
							}
						}
						break;
#endif

#if defined(__ACCE_COSTUME_SYSTEM__)
						case ITEM_ACCE_REVERSAL:
						case ITEM_ACCE_REVERSAL_MALL:
						{
							LPITEM lpTargetItem;
							if (!IsValidItemPosition(DestCell) || !(lpTargetItem = GetItem(DestCell)))
								return false;

							if (lpTargetItem->IsExchanging())
								return false;

							if (lpTargetItem->isLocked())
								return false;

#if defined(__SOUL_BIND_SYSTEM__)
							if (lpTargetItem->IsSealed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this item on soulbound items."));
								return false;
							}
#endif

							if (IsAcceRefineWindowOpen())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this item while the windows for absorption and combination are open."));
								return false;
							}

							lpTargetItem->SetSocket(ITEM_SOCKET_ACCE_DRAIN_ITEM_VNUM, 0);
							lpTargetItem->ClearAllAttribute();
							lpTargetItem->UpdatePacket();

							item->SetCount(item->GetCount() - 1);
						}
						break;
#endif

#if defined(__CHANGE_LOOK_SYSTEM__)
						case ITEM_CHANGE_LOOK_REVERSAL:
						case ITEM_CHANGE_LOOK_MOUNT_REVERSAL:
						{
							LPITEM item2;
							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item2->IsExchanging())
								return false;

							if (item2->isLocked())
								return false;

							if (item2->GetTransmutationVnum() == 0)
								return false;

							if (item->GetVnum() == ITEM_CHANGE_LOOK_MOUNT_REVERSAL && !item2->IsCostumeMount())
								return false;

#if defined(__SOUL_BIND_SYSTEM__)
							if (item2->IsSealed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this item on soulbound items."));
								return false;
							}
#endif

							item2->SetTransmutationVnum(0);
							item2->UpdatePacket();

							item->SetCount(item->GetCount() - 1);
						}
						break;
#endif

#if defined(__AURA_COSTUME_SYSTEM__)
						case ITEM_VNUM_AURA_CLEAR:
						{
							LPITEM lpTargetItem;
							if (!IsValidItemPosition(DestCell) || !(lpTargetItem = GetItem(DestCell)))
								return false;

							if (lpTargetItem->IsExchanging())
								return false;

							if (lpTargetItem->isLocked())
								return false;

#if defined(__SOUL_BIND_SYSTEM__)
							if (lpTargetItem->IsSealed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this item on soulbound items."));
								return false;
							}
#endif

							if (IsAuraRefineWindowOpen())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this item while the windows for absorption and combination are open."));
								return false;
							}

							lpTargetItem->SetSocket(ITEM_SOCKET_AURA_DRAIN_ITEM_VNUM, 0);
							lpTargetItem->ClearAllAttribute();
							lpTargetItem->UpdatePacket();

							item->SetCount(item->GetCount() - 1);
						}
						break;
#endif

#if defined(__SET_ITEM__)
						case ITEM_SET_CLEAR_SCROLL:
						{
							LPITEM item2;
							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item->IsExchanging() || item2->IsExchanging() || item2->IsEquipped())
								return false;

							if (item->isLocked() || item2->isLocked())
								return false;

#if defined(__SOUL_BIND_SYSTEM__)
							if (item2->IsSealed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this item on soulbound items."));
								return false;
							}
#endif

							item2->SetItemSetValue(0);
							item2->UpdatePacket();
							item->SetCount(item->GetCount() - 1);
						}
						break;
#endif

						// RESEARCHER_ELIXIR
						case ITEM_RESEARCHERS_ELIXIR:
						case ITEM_RESEARCHERS_ELIXIR_GIFT:
						case ITEM_RESEARCHERS_ELIXIR_MALL:
						{
							if (FindAffect(AFFECT_RESEARCHER_ELIXIR))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ??? ??????."));
							}
							else
							{
								AddAffect(AFFECT_RESEARCHER_ELIXIR, APPLY_NONE, 1, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true);
								item->SetCount(item->GetCount() - 1);
							}
						}
						break;
						// END_RESEARCHER_ELIXIR

#if defined(__SOUL_BIND_SYSTEM__)
						case ITEM_SCROLL_BINDING:
						{
							if (!g_bSoulBind)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot soulbind items on this server."));
								return false;
							}

							LPITEM item2;
							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item2->IsExchanging())
								return false;

							if (!item2->CanSealItem())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot bind this item to your soul."));
								return false;
							}

							if (item2->IsSealed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("This item is already soulbound."));
								return false;
							}

							item2->SealItem();

							char buf[256 + 1];
							snprintf(buf, sizeof(buf), "%s(%u) %s(%u)",
								item->GetName(), item->GetID(), item2->GetName(), item2->GetID());
							LogManager::instance().ItemLog(this, item, "SEAL_SCROLL_USE_SUCCESS", buf);

							item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_SCROLL_UNBINDING:
#if defined(__UN_SEAL_SCROLL_PLUS__)
						case ITEM_SCROLL_UNBINDING_MALL:
#endif
						{
							LPITEM item2;
							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item2->isLocked() || item2->IsExchanging())
								return false;

							if (!item2->IsSealed())
								return false;

							if (item2->GetSealDate() > E_SEAL_DATE_DEFAULT_TIMESTAMP)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("The soulbind on this item is already in the process of being removed."));
								return false;
							}

#if defined(__UN_SEAL_SCROLL_PLUS__)
							if (item->GetVnum() == ITEM_SCROLL_UNBINDING_MALL)
							{
								item2->SealItem(E_SEAL_DATE_DEFAULT_TIMESTAMP);

								char buf[256 + 1];
								snprintf(buf, sizeof(buf), "%s(%u) %s(%u)",
									item->GetName(), item->GetID(), item2->GetName(), item2->GetID());
								LogManager::instance().ItemLog(this, item, "UN_SEAL_SCROLL_PLUS_USE_SUCCESS", buf);
							}
							else
#endif
							{
								long lSealDate = time(0);

								if (test_server)
									lSealDate += 60;
								else
									lSealDate += (60 * 60 * SEAL_DATE_MAX);

								item2->SealItem(lSealDate);
								item2->StartSealDateExpireTimerEvent();

								char buf[256 + 1];
								snprintf(buf, sizeof(buf), "%s(%u) %s(%u)",
									item->GetName(), item->GetID(), item2->GetName(), item2->GetID());
								LogManager::instance().ItemLog(this, item, "UN_SEAL_SCROLL_USE_SUCCESS", buf);
							}

							item->SetCount(item->GetCount() - 1);
						}
						break;
#endif

						// ????????? ????
						case ITEM_NOG_POCKET:
						{
							/*
							??????? : item_proto value ???
								?????? value 1
								????? value 2
								????? value 3
								????? value 0 (???? ??)
							*/
							if (FindAffect(AFFECT_NOG_ABILITY))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ??? ??????."));
								return false;
							}
							long time = item->GetValue(0);
							long moveSpeedPer = item->GetValue(1);
							long attPer = item->GetValue(2);
							long expPer = item->GetValue(3);
							AddAffect(AFFECT_NOG_ABILITY, POINT_MOV_SPEED, moveSpeedPer, AFF_MOV_SPEED_POTION, time, 0, true, true);
							AddAffect(AFFECT_NOG_ABILITY, POINT_MALL_ATTBONUS, attPer, AFF_NONE, time, 0, true, true);
							AddAffect(AFFECT_NOG_ABILITY, POINT_MALL_EXPBONUS, expPer, AFF_NONE, time, 0, true, true);
							item->SetCount(item->GetCount() - 1);
						}
						break;

						// ????? ????
						case ITEM_RAMADAN_CANDY:
						{
							/*
							???????? : item_proto value ???
								?????? value 1
								????? value 2
								????? value 3
								????? value 0 (???? ??)
							*/
							if (FindAffect(AFFECT_RAMADAN_ABILITY))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ??? ??????."));
								return false;
							}

							long time = item->GetValue(0);
							long moveSpeedPer = item->GetValue(1);
							long attPer = item->GetValue(2);
							long expPer = item->GetValue(3);
							AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MOV_SPEED, moveSpeedPer, AFF_MOV_SPEED_POTION, time, 0, true, true);
							AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MALL_ATTBONUS, attPer, AFF_NONE, time, 0, true, true);
							AddAffect(AFFECT_RAMADAN_ABILITY, POINT_MALL_EXPBONUS, expPer, AFF_NONE, time, 0, true, true);
							item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_MARRIAGE_RING:
						{
							marriage::TMarriage* pMarriage = marriage::CManager::instance().Get(GetPlayerID());
							if (pMarriage)
							{
								if (pMarriage->ch1 != NULL)
								{
									if (CArenaManager::instance().IsArenaMap(pMarriage->ch1->GetMapIndex()))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????? ?? ???? ???????."));
										break;
									}
								}

								if (pMarriage->ch2 != NULL)
								{
									if (CArenaManager::instance().IsArenaMap(pMarriage->ch2->GetMapIndex()))
									{
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????? ?? ???? ???????."));
										break;
									}
								}

								int consumeSP = CalculateConsumeSP(this);

								if (consumeSP < 0)
									return false;

								PointChange(POINT_SP, -consumeSP, false);

								WarpToPID(pMarriage->GetOther(GetPlayerID()));
							}
							else
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ???? ????????? ????? ?? ???????."));
						}
						break;

						case ITEM_WHITE_FLAG:
							ForgetMyAttacker();
							item->SetCount(item->GetCount() - 1);
							break;

#if defined(__NPC_LOCATION_HELPER__)
						case 79667: // ITEM_NPC_LOCATION_HELPER_ACTIVATOR
							if (!CNpcLocationHelperManager::instance().IsActive(this))
								item->SetCount(item->GetCount() - 1);
							CNpcLocationHelperManager::instance().Activate(this);
							break;
#endif

#if defined(__PRIVATESHOP_SEARCH_SYSTEM__)
						case ITEM_PRIVATESHOP_SEARCH_LOOKING_GLASS:
						case ITEM_PRIVATESHOP_SEARCH_TRADING_GLASS:
							OpenPrivateShopSearch(item->GetVnum());
							break;
#endif

						case 30093:
						case 30094:
						case 30095:
						case 30096:
							// ??????
						{
							const int MAX_BAG_INFO = 26;
							static struct LuckyBagInfo
							{
								DWORD count;
								int prob;
								DWORD vnum;
							} b1[MAX_BAG_INFO] =
							{
								{ 1000, 302, 1 },
								{ 10, 150, 27002 },
								{ 10, 75, 27003 },
								{ 10, 100, 27005 },
								{ 10, 50, 27006 },
								{ 10, 80, 27001 },
								{ 10, 50, 27002 },
								{ 10, 80, 27004 },
								{ 10, 50, 27005 },
								{ 1, 10, ITEM_SKILLBOOK_VNUM },
								{ 1, 6, 92 },
								{ 1, 2, 132 },
								{ 1, 6, 1052 },
								{ 1, 2, 1092 },
								{ 1, 6, 2082 },
								{ 1, 2, 2122 },
								{ 1, 6, 3082 },
								{ 1, 2, 3122 },
								{ 1, 6, 5052 },
								{ 1, 2, 5082 },
								{ 1, 6, 7082 },
								{ 1, 2, 7122 },
								{ 1, 1, 11282 },
								{ 1, 1, 11482 },
								{ 1, 1, 11682 },
								{ 1, 1, 11882 },
							};

							struct LuckyBagInfo b2[MAX_BAG_INFO] =
							{
								{ 1000, 302, 1 },
								{ 10, 150, 27002 },
								{ 10, 75, 27002 },
								{ 10, 100, 27005 },
								{ 10, 50, 27005 },
								{ 10, 80, 27001 },
								{ 10, 50, 27002 },
								{ 10, 80, 27004 },
								{ 10, 50, 27005 },
								{ 1, 10, ITEM_SKILLBOOK_VNUM },
								{ 1, 6, 92 },
								{ 1, 2, 132 },
								{ 1, 6, 1052 },
								{ 1, 2, 1092 },
								{ 1, 6, 2082 },
								{ 1, 2, 2122 },
								{ 1, 6, 3082 },
								{ 1, 2, 3122 },
								{ 1, 6, 5052 },
								{ 1, 2, 5082 },
								{ 1, 6, 7082 },
								{ 1, 2, 7122 },
								{ 1, 1, 11282 },
								{ 1, 1, 11482 },
								{ 1, 1, 11682 },
								{ 1, 1, 11882 },
							};

							LuckyBagInfo* bi = NULL;
							if (LC_IsHongKong())
								bi = b2;
							else
								bi = b1;

							int pct = number(1, 1000);

							int i;
							for (i = 0; i < MAX_BAG_INFO; i++)
							{
								if (pct <= bi[i].prob)
									break;
								pct -= bi[i].prob;
							}
							if (i >= MAX_BAG_INFO)
								return false;

							if (bi[i].vnum == ITEM_SKILLBOOK_VNUM)
							{
								// ?????????? ?????? ???.
								GiveRandomSkillBook();
							}
							else if (bi[i].vnum == 1)
							{
								PointChange(POINT_GOLD, 1000, true);
							}
							else
							{
								AutoGiveItem(bi[i].vnum, bi[i].count);
							}
							ITEM_MANAGER::instance().RemoveItem(item);
						}
						break;

						case 50004: // ?????? ??????
						{
							if (item->GetSocket(0))
							{
								item->SetSocket(0, item->GetSocket(0) + 1);
							}
							else
							{
								// ? ????
								int iMapIndex = GetMapIndex();

								PIXEL_POSITION pos;

								if (SECTREE_MANAGER::instance().GetRandomLocation(iMapIndex, pos, 700))
								{
									item->SetSocket(0, 1);
									item->SetSocket(1, pos.x);
									item->SetSocket(2, pos.y);
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ?????? ?????? ?????? ???????? ???? ???????."));
									return false;
								}
							}

							int dist = 0;
							float distance = (DISTANCE_SQRT(GetX() - item->GetSocket(1), GetY() - item->GetSocket(2)));

							if (distance < 1000.0f)
							{
								// ???!
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ?????? ????? ???? ???? ????????."));

								// ???????? ???? ??? ???????? ????? ???.
								struct TEventStoneInfo
								{
									DWORD dwVnum;
									int count;
									int prob;
								};
								const int EVENT_STONE_MAX_INFO = 15;
								TEventStoneInfo info_10[EVENT_STONE_MAX_INFO] =
								{
									{ 27001, 10, 8 },
									{ 27004, 10, 6 },
									{ 27002, 10, 12 },
									{ 27005, 10, 12 },
									{ 27100, 1, 9 },
									{ 27103, 1, 9 },
									{ 27101, 1, 10 },
									{ 27104, 1, 10 },
									{ 27999, 1, 12 },

									{ 25040, 1, 4 },

									{ 27410, 1, 0 },
									{ 27600, 1, 0 },
									{ 25100, 1, 0 },

									{ 50001, 1, 0 },
									{ 50003, 1, 1 },
								};
								TEventStoneInfo info_7[EVENT_STONE_MAX_INFO] =
								{
									{ 27001, 10, 1 },
									{ 27004, 10, 1 },
									{ 27004, 10, 9 },
									{ 27005, 10, 9 },
									{ 27100, 1, 5 },
									{ 27103, 1, 5 },
									{ 27101, 1, 10 },
									{ 27104, 1, 10 },
									{ 27999, 1, 14 },

									{ 25040, 1, 5 },

									{ 27410, 1, 5 },
									{ 27600, 1, 5 },
									{ 25100, 1, 5 },

									{ 50001, 1, 0 },
									{ 50003, 1, 5 },

								};
								TEventStoneInfo info_4[EVENT_STONE_MAX_INFO] =
								{
									{ 27001, 10, 0 },
									{ 27004, 10, 0 },
									{ 27002, 10, 0 },
									{ 27005, 10, 0 },
									{ 27100, 1, 0 },
									{ 27103, 1, 0 },
									{ 27101, 1, 0 },
									{ 27104, 1, 0 },
									{ 27999, 1, 25 },

									{ 25040, 1, 0 },

									{ 27410, 1, 0 },
									{ 27600, 1, 0 },
									{ 25100, 1, 15 },

									{ 50001, 1, 10 },
									{ 50003, 1, 50 },

								};

								{
									TEventStoneInfo* info;
									if (item->GetSocket(0) <= 4)
										info = info_4;
									else if (item->GetSocket(0) <= 7)
										info = info_7;
									else
										info = info_10;

									int prob = number(1, 100);

									for (int i = 0; i < EVENT_STONE_MAX_INFO; ++i)
									{
										if (!info[i].prob)
											continue;

										if (prob <= info[i].prob)
										{
											if (info[i].dwVnum == 50001)
											{
												DWORD* pdw = M2_NEW DWORD[2];

												pdw[0] = info[i].dwVnum;
												pdw[1] = info[i].count;

												// ??????? ?????? ???????
												DBManager::instance().ReturnQuery(QID_LOTTO, GetPlayerID(), pdw,
													"INSERT INTO lotto_list VALUES(0, 'server%s', %u, NOW())",
													get_table_postfix(), GetPlayerID());
											}
											else
												AutoGiveItem(info[i].dwVnum, info[i].count);

											break;
										}
										prob -= info[i].prob;
									}
								}

								char chatbuf[CHAT_MAX_LEN + 1];
								int len = snprintf(chatbuf, sizeof(chatbuf), "StoneDetect %u 0 0", (DWORD)GetVID());

								if (len < 0 || len >= (int)sizeof(chatbuf))
									len = sizeof(chatbuf) - 1;

								++len; // \0 ??????? ??????

								TPacketGCChat pack_chat;
								pack_chat.header = HEADER_GC_CHAT;
								pack_chat.size = sizeof(TPacketGCChat) + len;
								pack_chat.type = CHAT_TYPE_COMMAND;
								pack_chat.id = 0;
								pack_chat.bEmpire = GetDesc()->GetEmpire();
								//pack_chat.id = vid;

								TEMP_BUFFER buf;
								buf.write(&pack_chat, sizeof(TPacketGCChat));
								buf.write(chatbuf, len);

								PacketAround(buf.read_peek(), buf.size());

								ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (DETECT_EVENT_STONE) 1");
								return true;
							}
							else if (distance < 20000)
								dist = 1;
							else if (distance < 70000)
								dist = 2;
							else
								dist = 3;

							// ???? ????????? ???????.
							const int STONE_DETECT_MAX_TRY = 10;
							if (item->GetSocket(0) >= STONE_DETECT_MAX_TRY)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ?????? ?????? ???? ????????."));
								ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (DETECT_EVENT_STONE) 0");
								AutoGiveItem(27002);
								return true;
							}

							if (dist)
							{
								char chatbuf[CHAT_MAX_LEN + 1];
								int len = snprintf(chatbuf, sizeof(chatbuf),
									"StoneDetect %u %d %d",
									(DWORD)GetVID(), dist, (int)GetDegreeFromPositionXY(GetX(), item->GetSocket(2), item->GetSocket(1), GetY()));

								if (len < 0 || len >= (int)sizeof(chatbuf))
									len = sizeof(chatbuf) - 1;

								++len; // \0 ??????? ??????

								TPacketGCChat pack_chat;
								pack_chat.header = HEADER_GC_CHAT;
								pack_chat.size = sizeof(TPacketGCChat) + len;
								pack_chat.type = CHAT_TYPE_COMMAND;
								pack_chat.id = 0;
								pack_chat.bEmpire = GetDesc()->GetEmpire();
								//pack_chat.id = vid;

								TEMP_BUFFER buf;
								buf.write(&pack_chat, sizeof(TPacketGCChat));
								buf.write(chatbuf, len);

								PacketAround(buf.read_peek(), buf.size());
							}
						}
						break;

						case 27989: // ??????????
						case 76006: // ?????? ??????????
						{
							LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(GetMapIndex());

							if (pMap != NULL)
							{
								item->SetSocket(0, item->GetSocket(0) + 1);

								FFindStone f;

								// <Factor> SECTREE::for_each -> SECTREE::for_each_entity
								pMap->for_each(f);

								if (f.m_mapStone.size() > 0)
								{
									std::map<DWORD, LPCHARACTER>::iterator stone = f.m_mapStone.begin();

									DWORD max = UINT_MAX;
									LPCHARACTER pTarget = stone->second;

									while (stone != f.m_mapStone.end())
									{
										DWORD dist = (DWORD)DISTANCE_SQRT(GetX() - stone->second->GetX(), GetY() - stone->second->GetY());

										if (dist != 0 && max > dist)
										{
											max = dist;
											pTarget = stone->second;
										}
										stone++;
									}

									if (pTarget != NULL)
									{
										int val = 3;

										if (max < 10000) val = 2;
										else if (max < 70000) val = 1;

										ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u %d %d", (DWORD)GetVID(), val,
											(int)GetDegreeFromPositionXY(GetX(), pTarget->GetY(), pTarget->GetX(), GetY()));
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ?????????? ??????? ?????? ???????."));
									}
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ?????????? ??????? ?????? ???????."));
								}

								if (item->GetSocket(0) >= 6)
								{
									ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u 0 0", (DWORD)GetVID());
									ITEM_MANAGER::instance().RemoveItem(item);
								}
							}
							break;
						}
						break;

#if defined(__MT_THUNDER_DUNGEON__)
						case 79602:
						{
							if (GetMapIndex() != CMTThunderDungeon::MAP_INDEX)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("This action is not possible on this map."));
								return false;
							}

							LPSECTREE_MAP pMap = SECTREE_MANAGER::instance().GetMap(GetMapIndex());

							if (pMap != NULL)
							{
								item->SetSocket(0, item->GetSocket(0) + 1);

								FFindMobVnum f(6405);

								// <Factor> SECTREE::for_each -> SECTREE::for_each_entity
								pMap->for_each(f);

								if (f.m_mapVID.size() > 0)
								{
									std::map<DWORD, LPCHARACTER>::iterator it = f.m_mapVID.begin();

									DWORD max = UINT_MAX;
									LPCHARACTER pTarget = it->second;

									while (it != f.m_mapVID.end())
									{
										DWORD dist = (DWORD)DISTANCE_SQRT(GetX() - it->second->GetX(), GetY() - it->second->GetY());

										if (dist != 0 && max > dist)
										{
											max = dist;
											pTarget = it->second;
										}
										it++;
									}

									if (pTarget != NULL)
									{
										int val = 3;

										if (max < 10000) val = 2;
										else if (max < 70000) val = 1;

										ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u %d %d", (DWORD)GetVID(), val,
											(int)GetDegreeFromPositionXY(GetX(), pTarget->GetY(), pTarget->GetX(), GetY()));
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("The Compass was used, but the Guard wasn?t found."));
									}
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("The Compass was used, but the Guard wasn?t found."));
								}

								if (item->GetSocket(0) >= 6)
								{
									ChatPacket(CHAT_TYPE_COMMAND, "StoneDetect %u 0 0", (DWORD)GetVID());
									ITEM_MANAGER::instance().RemoveItem(item);
								}
							}
						}
						break;
#endif

						case 27996: // ????
							item->SetCount(item->GetCount() - 1);
							/*
							if (GetSkillLevel(SKILL_CREATE_POISON))
								AddAffect(AFFECT_ATT_GRADE, POINT_ATT_GRADE, 3, AFF_DRINK_POISON, 15 * 60, 0, true);
							else
							{
								// ?????? ?????? 50% ??? 50% ????? +2
								if (number(0, 1))
								{
									if (GetHP() > 100)
										PointChange(POINT_HP, -(GetHP() - 1));
									else
										Dead();
								}
								else
									AddAffect(AFFECT_ATT_GRADE, POINT_ATT_GRADE, 2, AFF_DRINK_POISON, 15 * 60, 0, true);
							}
							*/
							break;

						case fishing::SHELLFISH_VNUM: // ????
							// 50 ?????? 47990
							// 30 ??
							// 10 ?????? 47992
							// 7 ????? 47993
							// 3 ?????? 47994
						{
							item->SetCount(item->GetCount() - 1);

							int r = number(1, 100);

							if (r <= 50)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("???????? ???????? ????????."));
								AutoGiveItem(fishing::STONEPIECE_VNUM);
							}
							else
							{
								const int prob_table_euckr[] =
								{
									80, 90, 97
								};

								const int prob_table_gb2312[] =
								{
									95, 97, 99
								};

								const int* prob_table = !g_iUseLocale ? prob_table_euckr : prob_table_gb2312;

								if (r <= prob_table[0])
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ?????? ???? ????????."));
								}
								else if (r <= prob_table[1])
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("???????? ??????? ????????."));
									AutoGiveItem(fishing::WHITE_PEARL_VNUM);
								}
								else if (r <= prob_table[2])
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("???????? ?????? ????????."));
									AutoGiveItem(fishing::BLUE_PEARL_VNUM);
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("???????? ??????? ????????."));
									AutoGiveItem(fishing::RED_PEARL_VNUM);
								}
							}
						}
						break;

						case 71013: // ??????????
							CreateFly(number(FLY_FIREWORK1, FLY_FIREWORK6), this);
							item->SetCount(item->GetCount() - 1);
							break;

						case 50100: // ????
						case 50101:
						case 50102:
						case 50103:
						case 50104:
						case 50105:
						case 50106:
							CreateFly(item->GetVnum() - 50100 + FLY_FIREWORK1, this);
							item->SetCount(item->GetCount() - 1);
							break;

						case 50200: // ??????
						{
							if (LC_IsYMIR() == true || LC_IsKorea() == true)
							{
								if (IS_BOTARYABLE_ZONE(GetMapIndex()) == true)
								{
									__OpenPrivateShop();
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ?????? ?? ?? ???? ????????"));
								}
							}
							else
							{
								__OpenPrivateShop();
							}
						}
						break;

						case fishing::FISH_MIND_PILL_VNUM:
							AddAffect(AFFECT_FISH_MIND_PILL, POINT_NONE, 0, AFF_FISH_MIND, 20 * 60, 0, true);
							item->SetCount(item->GetCount() - 1);
							break;

						case 50301: // ???? ?????
						case 50302:
						case 50303:
						{
							if (IsPolymorphed() == true)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????? ??? ?? ???????."));
								return false;
							}

							int lv = GetSkillLevel(SKILL_LEADERSHIP);

							if (lv < item->GetValue(0))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ? ??? ????? ??????? ??????."));
								return false;
							}

							if (lv >= item->GetValue(1))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ? ????? ???? ?????? ?? ?? ???? ??????."));
								return false;
							}

							if (LearnSkillByBook(SKILL_LEADERSHIP))
							{
								item->SetCount(item->GetCount() - 1);

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(SKILL_LEADERSHIP, get_global_time() + iReadDelay);
							}
						}
						break;

						case 50304: // ????? ?????
						case 50305:
						case 50306:
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????????? ? ?????? ???????."));
								return false;
							}

							if (GetSkillLevel(SKILL_COMBO) == 0 && GetLevel() < 30)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? 30?? ??? ?????? ?????? ?? ???? ?? ???? ??????."));
								return false;
							}

							if (GetSkillLevel(SKILL_COMBO) == 1 && GetLevel() < 50)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? 50?? ??? ?????? ?????? ?? ???? ?? ???? ??????."));
								return false;
							}

							if (GetSkillLevel(SKILL_COMBO) >= 2)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ????? ?????? ?? ???????."));
								return false;
							}

							int iPct = item->GetValue(0);

							if (LearnSkillByBook(SKILL_COMBO, iPct))
							{
								item->SetCount(item->GetCount() - 1);

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(SKILL_COMBO, get_global_time() + iReadDelay);
							}
						}
						break;

						case 50311: // ??? ?????
						case 50312:
						case 50313:
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????????? ? ?????? ???????."));
								return false;
							}

							DWORD dwSkillVnum = item->GetValue(0);
							int iPct = MINMAX(0, item->GetValue(1), 100);
							if (GetSkillLevel(dwSkillVnum) >= 20 || dwSkillVnum - SKILL_LANGUAGE1 + 1 == GetEmpire())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ?????? ?????? ?? ??? ??????."));
								return false;
							}

							if (LearnSkillByBook(dwSkillVnum, iPct))
							{
								item->SetCount(item->GetCount() - 1);

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
							}
						}
						break;

						case 50061: // ??? ?? ??? ??? ?????
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????????? ? ?????? ???????."));
								return false;
							}

							DWORD dwSkillVnum = item->GetValue(0);
							int iPct = MINMAX(0, item->GetValue(1), 100);

							if (GetSkillLevel(dwSkillVnum) >= 10)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ??? ?????? ?? ???????."));
								return false;
							}

							if (LearnSkillByBook(dwSkillVnum, iPct))
							{
								item->SetCount(item->GetCount() - 1);

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
							}
						}
						break;

						// LEARNABLE_SKILL_BOOKS_BY_LEVEL
						case 50314: case 50315: case 50316: // ???? ?????
						case 50325: // Book of Precision
						{
							if (IsPolymorphed() == true)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????? ??? ?? ???????."));
								return false;
							}

							int iSkillLevelLowLimit = item->GetValue(0);
							int iSkillLevelHighLimit = item->GetValue(1);
							int iPct = MINMAX(0, item->GetValue(2), 100);
							int iLevelLimit = item->GetValue(3);
							DWORD dwSkillVnum = 0;

							switch (item->GetVnum())
							{
								case 50314: case 50315: case 50316:
									dwSkillVnum = SKILL_POLYMORPH;
									break;

#if defined(__CONQUEROR_LEVEL__)
								case 50325: // Book of Precision
									dwSkillVnum = SKILL_HIT;
									break;
#endif

								default:
									return false;
							}

							if (0 == dwSkillVnum)
								return false;

#if defined(__CONQUEROR_LEVEL__)
							if (dwSkillVnum == SKILL_HIT && GetConquerorLevel() < iSkillLevelLowLimit)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ? ???????? ?????? ?? ????? ????."));
								return false;
							}
							else
#endif
							{
								if (GetLevel() < iLevelLimit)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ? ???????? ?????? ?? ????? ????."));
									return false;
								}
							}

							if (GetSkillLevel(dwSkillVnum) >= 40)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ??? ?????? ?? ???????."));
								return false;
							}

							if (GetSkillLevel(dwSkillVnum) < iSkillLevelLowLimit
#if defined(__CONQUEROR_LEVEL__)
								&& dwSkillVnum != SKILL_HIT
#endif
								)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ? ??? ????? ??????? ??????."));
								return false;
							}

							if (GetSkillLevel(dwSkillVnum) >= iSkillLevelHighLimit)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ???? ?? ??? ?????? ?? ???????."));
								return false;
							}

							if (LearnSkillByBook(dwSkillVnum, iPct))
							{
								item->SetCount(item->GetCount() - 1);

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
							}
						}
						break;
						// END_LEARNABLE_SKILL_BOOKS_BY_LEVEL

						// LEARNABLE_SKILL_BOOKS_BY_EXP
#if defined(__PARTY_PROFICY__)
						case 50338:	// Charisma of a Rookie
						case 50339:	// Charisma of an Adept
						case 50340:	// Charisma of an Expert
#endif
#if defined(__PARTY_INSIGHT__)
						case 50341:	// Inspiration of a Rookie
						case 50342:	// Inspiration of an Adept
						case 50343:	// Inspiration of an Expert
#endif
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????? ??? ?? ???????."));
								return false;
							}

							int iSkillLevelLowLimit = item->GetValue(0);
							int iSkillLevelHighLimit = item->GetValue(1);
							int iPct = MINMAX(0, item->GetValue(2), 100);
							int iNeedExp = item->GetValue(3);
							DWORD dwSkillVnum = 0;

							switch (item->GetVnum())
							{
#if defined(__PARTY_PROFICY__)
								case 50338:	// Charisma of a Rookie
								case 50339:	// Charisma of an Adept
								case 50340:	// Charisma of an Expert
									dwSkillVnum = SKILL_ROLE_PROFICIENCY;
									break;
#endif

#if defined(__PARTY_INSIGHT__)
								case 50341:	// Inspiration of a Rookie
								case 50342:	// Inspiration of an Adept
								case 50343:	// Inspiration of an Expert
									dwSkillVnum = SKILL_INSIGHT;
									break;
#endif

								default:
									return false;
							}

							if (0 == dwSkillVnum)
								return false;

							if (GetExp() < static_cast<DWORD>(iNeedExp))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ??????? ? ???? ?? ???????."));
								return false;
							}

							if (GetSkillLevel(dwSkillVnum) >= 40)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ?? ???? ???????."));
								return false;
							}

							if (GetSkillLevel(dwSkillVnum) < iSkillLevelLowLimit)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ? ??? ????? ??????? ??????."));
								return false;
							}

							if (GetSkillLevel(dwSkillVnum) >= iSkillLevelHighLimit)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ???? ?? ??? ?????? ?? ???????."));
								return false;
							}

							if (LearnSkillByBook(dwSkillVnum, iPct))
							{
								item->SetCount(item->GetCount() - 1);

								if (dwSkillVnum != SKILL_HIT)
									PointChange(POINT_EXP, -iNeedExp);

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server)
									iReadDelay /= 3;

								SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
							}
						}
						break;
						// END_LEARNABLE_SKILL_BOOKS_BY_EXP

						case 50902:
						case 50903:
						case 50904:
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????????? ? ?????? ???????."));
								return false;
							}

							DWORD dwSkillVnum = SKILL_CREATE;
							int iPct = MINMAX(0, item->GetValue(1), 100);

							if (GetSkillLevel(dwSkillVnum) >= 40)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ??? ?????? ?? ???????."));
								return false;
							}

							if (LearnSkillByBook(dwSkillVnum, iPct))
							{
								item->SetCount(item->GetCount() - 1);

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);

								if (test_server)
								{
									ChatPacket(CHAT_TYPE_INFO, "[TEST_SERVER] Success to learn skill ");
								}
							}
							else
							{
								if (test_server)
								{
									ChatPacket(CHAT_TYPE_INFO, "[TEST_SERVER] Failed to learn skill ");
								}
							}
						}
						break;

						// MINING
						case ITEM_MINING_SKILL_TRAIN_BOOK:
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????????? ? ?????? ???????."));
								return false;
							}

							DWORD dwSkillVnum = SKILL_MINING;
							int iPct = MINMAX(0, item->GetValue(1), 100);

							if (GetSkillLevel(dwSkillVnum) >= 40)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ??? ?????? ?? ???????."));
								return false;
							}

							if (LearnSkillByBook(dwSkillVnum, iPct))
							{
								item->SetCount(item->GetCount() - 1);

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
							}
						}
						break;
						// END_OF_MINING

						case ITEM_HORSE_SKILL_TRAIN_BOOK:
						{
							if (IsPolymorphed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????????? ? ?????? ???????."));
								return false;
							}

							DWORD dwSkillVnum = SKILL_HORSE;
							int iPct = MINMAX(0, item->GetValue(1), 100);

							if (GetLevel() < 50)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ??? ????? ?????? ?? ??? ?????? ?????."));
								return false;
							}

							if (!test_server && get_global_time() < GetSkillNextReadTime(dwSkillVnum))
							{
								if (FindAffect(AFFECT_SKILL_BOOK_NO_DELAY))
								{
									// ?????? ???????? ?? ???? ????
									RemoveAffect(AFFECT_SKILL_BOOK_NO_DELAY);
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("???????? ???? ?????????? ????????????."));
								}
								else
								{
									SkillLearnWaitMoreTimeMessage(GetSkillNextReadTime(dwSkillVnum) - get_global_time());
									return false;
								}
							}

							if (GetPoint(POINT_HORSE_SKILL) >= 20 ||
								GetSkillLevel(SKILL_HORSE_WILDATTACK) + GetSkillLevel(SKILL_HORSE_CHARGE) + GetSkillLevel(SKILL_HORSE_ESCAPE) >= 60 ||
								GetSkillLevel(SKILL_HORSE_WILDATTACK_RANGE) + GetSkillLevel(SKILL_HORSE_CHARGE) + GetSkillLevel(SKILL_HORSE_ESCAPE) >= 60)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ??? ??? ??????? ???? ?? ???????."));
								return false;
							}

							if (number(1, 100) <= iPct)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ??????? ??? ??? ??? ??????? ????????."));
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ???????? ??? ????? ?????? ??? ?? ??????."));
								PointChange(POINT_HORSE_SKILL, 1);

								int iReadDelay = number(SKILLBOOK_DELAY_MIN, SKILLBOOK_DELAY_MAX);
								if (distribution_test_server) iReadDelay /= 3;

								if (!test_server)
									SetSkillNextReadTime(dwSkillVnum, get_global_time() + iReadDelay);
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????? ????????????."));
							}

							ITEM_MANAGER::instance().RemoveItem(item);
						}
						break;

						case 70102: // ????
						case 70103: // ????
						{
							if (GetAlignment() >= 0)
								return false;

							int delta = MIN(-GetAlignment(), item->GetValue(0));

							sys_log(0, "%s ALIGNMENT ITEM %d", GetName(), delta);

							UpdateAlignment(delta);
							item->SetCount(item->GetCount() - 1);

							if (delta / 10 > 0)
							{
								ChatPacket(CHAT_TYPE_TALKING, LC_STRING("?????? ?????????. ?????? ???????? ?????? ?? ???????? ???????."));
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? %d ????????????.", delta / 10));
							}
						}
						break;

						case 71107: // ?????????
						case 39032:
						{
							quest::CQuestManager& q = quest::CQuestManager::Instance();
							quest::PC* pPC = q.GetPC(GetPlayerID());
							if (pPC)
							{
								int val = item->GetValue(0);
								int interval = item->GetValue(1);
								int last_use_time = pPC->GetFlag("mythical_peach.last_use_time");
								if (get_global_time() - last_use_time < interval * 60 * 60)
								{
									if (test_server == false)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ????? ?? ???????."));
										return false;
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ???? ????? ???"));
									}
								}

								if (GetAlignment() == 200000)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ?? ??? ??? ?? ???????."));
									return false;
								}

								if (200000 - GetAlignment() < val * 10)
								{
									val = (200000 - GetAlignment()) / 10;
								}

								int old_alignment = GetAlignment() / 10;

								UpdateAlignment(val * 10);

								item->SetCount(item->GetCount() - 1);
								pPC->SetFlag("mythical_peach.last_use_time", get_global_time());

								ChatPacket(CHAT_TYPE_TALKING, LC_STRING("?????? ?????????. ?????? ???????? ?????? ?? ???????? ???????."));
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? %d ????????????.", val));

								char buf[256 + 1];
								snprintf(buf, sizeof(buf), "%d %d", old_alignment, GetAlignment() / 10);
								LogManager::instance().CharLog(this, val, "MYTHICAL_PEACH", buf);
							}
						}
						break;

						case 71109: // ?????
						case 39033:
						{
							LPITEM item2;

							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item2->IsExchanging() || item2->IsEquipped())
								return false;

							if (item2->GetSocketCount() == 0)
								return false;

							switch (item2->GetType())
							{
								case ITEM_WEAPON:
									break;

								case ITEM_ARMOR:
									switch (item2->GetSubType())
									{
										case ARMOR_EAR:
										case ARMOR_WRIST:
										case ARMOR_NECK:
											ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ?????? ???????"));
											return false;
									}
									break;

								default:
									return false;
							}

							std::stack<long> socket;

							for (int i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
							{
#if defined(__GLOVE_SYSTEM__)
								DWORD dwSocketData = item2->GetSocket(i);
								if (item2->IsGlove() && dwSocketData >= 1000000 && dwSocketData != ITEM_BROKEN_METIN_VNUM)
								{
									DWORD dwBaseIndex = 28046;
									dwBaseIndex += (((dwSocketData / 1000) % 10) * 100);
									dwBaseIndex += ((dwSocketData / 100) % 10) - 1;

									const TItemTable* pItemData = ITEM_MANAGER::instance().GetTable(dwBaseIndex);
									if (pItemData == nullptr)
										continue;

									dwSocketData = dwBaseIndex;
								}
								socket.push(dwSocketData);
#else
								socket.push(item2->GetSocket(i));
#endif
							}

							int idx = METIN_SOCKET_MAX_NUM - 1;

							while (socket.size() > 0)
							{
								if (socket.top() > 2 && socket.top() != ITEM_BROKEN_METIN_VNUM)
									break;

								idx--;
								socket.pop();
							}

							if (socket.size() == 0)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ?????? ???????"));
								return false;
							}

							LPITEM pItemReward = AutoGiveItem(socket.top());
							if (pItemReward != NULL)
							{
								item2->SetSocket(idx, 1);

								char buf[256 + 1];
								snprintf(buf, sizeof(buf), "%s(%u) %s(%u)",
									item2->GetName(), item2->GetID(), pItemReward->GetName(), pItemReward->GetID());
								LogManager::instance().ItemLog(this, item, "USE_DETACHMENT_ONE", buf);

								item->SetCount(item->GetCount() - 1);
							}
						}
						break;

						case 70201: // ?????
						case 70202: // ??????(???)
						case 70203: // ??????(???)
						case 70204: // ??????(??????)
						case 70205: // ??????(????)
						case 70206: // ??????(??????)
						{
							// NEW_HAIR_STYLE_ADD
							if (GetPart(PART_HAIR) >= 1001)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ??????????? ?????? ????? ?????????."));
							}
							// END_NEW_HAIR_STYLE_ADD
							else
							{
								quest::CQuestManager& q = quest::CQuestManager::instance();
								quest::PC* pPC = q.GetPC(GetPlayerID());
								if (pPC)
								{
									int last_dye_level = pPC->GetFlag("dyeing_hair.last_dye_level");

									if (last_dye_level == 0 ||
										last_dye_level + 3 <= GetLevel() ||
										item->GetVnum() == 70201)
									{
										SetPart(PART_HAIR, item->GetVnum() - 70201);

										if (item->GetVnum() == 70201)
											pPC->SetFlag("dyeing_hair.last_dye_level", 0);
										else
											pPC->SetFlag("dyeing_hair.last_dye_level", GetLevel());

										item->SetCount(item->GetCount() - 1);
										UpdatePacket();
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("%d ?????? ???? ??? ??????? ?? ??????.", last_dye_level + 3));
									}
								}
							}
						}
						break;

						case ITEM_NEW_YEAR_GREETING_VNUM:
						{
							if (GiveItemFromSpecialItemGroup(ITEM_NEW_YEAR_GREETING_VNUM))
								item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_VALENTINE_ROSE:
						case ITEM_VALENTINE_CHOCOLATE:
						{
							if (item->GetVnum() == ITEM_VALENTINE_ROSE && SEX_MALE == GET_SEX(this) ||
								item->GetVnum() == ITEM_VALENTINE_CHOCOLATE && SEX_FEMALE == GET_SEX(this))
							{
								// ?????? ??????? ?? ?? ????.
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ??????? ?? ???????? ?? ?? ???????."));
								return false;
							}

							if (GiveItemFromSpecialItemGroup(item->GetVnum()))
								item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_WHITEDAY_CANDY:
						case ITEM_WHITEDAY_ROSE:
						{
							if (item->GetVnum() == ITEM_WHITEDAY_CANDY && SEX_MALE == GET_SEX(this) ||
								item->GetVnum() == ITEM_WHITEDAY_ROSE && SEX_FEMALE == GET_SEX(this))
							{
								// ?????? ??????? ?? ?? ????.
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ??????? ?? ???????? ?? ?? ???????."));
								return false;
							}

							if (GiveItemFromSpecialItemGroup(item->GetVnum()))
								item->SetCount(item->GetCount() - 1);
						}
						break;

						case 50011: // ????????
						{
							if (!GiveItemFromSpecialItemGroup(50011))
							{
								ChatPacket(CHAT_TYPE_TALKING, LC_STRING("?????? ???? ?? ?????????."));
								return false;
							}

							item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_GIVE_STAT_RESET_COUNT_VNUM:
						{
							//PointChange(POINT_GOLD, -iCost);
							PointChange(POINT_STAT_RESET_COUNT, 1);
							item->SetCount(item->GetCount() - 1);
						}
						break;

						case 50107:
						{
							if (CArenaManager::instance().IsArenaMap(GetMapIndex()))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????? ?? ???? ???????."));
								return false;
							}

							EffectPacket(SE_CHINA_FIREWORK);
							// ???? ?????? ??????
							AddAffect(AFFECT_CHINA_FIREWORK, POINT_STUN_PCT, 30, AFF_CHINA_FIREWORK, 5 * 60, 0, true);
							item->SetCount(item->GetCount() - 1);
						}
						break;

						case 50108:
						{
							if (CArenaManager::instance().IsArenaMap(GetMapIndex()))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????? ?? ???? ???????."));
								return false;
							}

							EffectPacket(SE_SPIN_TOP);
							// ???? ?????? ??????
							AddAffect(AFFECT_CHINA_FIREWORK, POINT_STUN_PCT, 30, AFF_CHINA_FIREWORK, 5 * 60, 0, true);
							item->SetCount(item->GetCount() - 1);
						}
						break;

						case ITEM_WONSO_BEAN_VNUM:
							PointChange(POINT_HP, GetMaxHP() - GetHP());
							item->SetCount(item->GetCount() - 1);
							break;

						case ITEM_WONSO_SUGAR_VNUM:
							PointChange(POINT_SP, GetMaxSP() - GetSP());
							item->SetCount(item->GetCount() - 1);
							break;

						case ITEM_WONSO_FRUIT_VNUM:
							PointChange(POINT_STAMINA, GetMaxStamina() - GetStamina());
							item->SetCount(item->GetCount() - 1);
							break;

						case ITEM_ELK_VNUM: // ???????
						{
							int iGold = item->GetSocket(0);
							ITEM_MANAGER::instance().RemoveItem(item);
							PointChange(POINT_GOLD, iGold);
						}
						break;

						// ?????? ???
						case 70021:
						{
							int HealPrice = quest::CQuestManager::instance().GetEventFlag("MonarchHealGold");
							if (HealPrice == 0)
								HealPrice = 2000000;

							if (CMonarch::instance().HealMyEmpire(this, HealPrice))
							{
								char szNotice[256];
								snprintf(szNotice, sizeof(szNotice), LC_STRING("?????? ?????? ?????? %s ?????? HP,SP?? ??? ??????.", EMPIRE_NAME(GetEmpire())));
								SendNoticeMap(szNotice, GetMapIndex(), false);

								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ???? ???????????."));
							}
						}
						break;

						case 27995:
						{
						}
						break;

						case 71092: // ???? ????? ???
						{
							if (m_pkChrTarget != NULL)
							{
								if (m_pkChrTarget->IsPolymorphed())
								{
									m_pkChrTarget->SetPolymorph(0);
									m_pkChrTarget->RemoveAffect(AFFECT_POLYMORPH);
								}
							}
							else
							{
								if (IsPolymorphed())
								{
									SetPolymorph(0);
									RemoveAffect(AFFECT_POLYMORPH);
								}
							}
						}
						break;

						case 71051: // ????
						{
							LPITEM item2;

							if (!IsValidItemPosition(DestCell) || !(item2 = GetInventoryItem(wDestCell)))
								return false;

							if (item2->IsExchanging())
								return false;

							if (item2->IsEquipped())
								return false;

							if (item2->GetType() == ITEM_COSTUME)
								return false;

							//if (item2->GetAttributeCount() < 5)
							//	return false;

							if (item2->GetAttributeSetIndex() == -1)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ?????? ?? ???? ??????????."));
								return false;
							}

							if (item2->AddRareAttribute() == true)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????????????."));

								int iAddedIdx = item2->GetRareAttrCount() + 4;
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());

								LogManager::instance().ItemLog(
									GetPlayerID(),
									item2->GetAttributeType(iAddedIdx),
									item2->GetAttributeValue(iAddedIdx),
									item->GetID(),
									"ADD_RARE_ATTR",
									buf,
									GetDesc()->GetHostName(),
									item->GetOriginalVnum());

								if (!g_bUnlimitedAddRareAttributes)
									item->SetCount(item->GetCount() - 1);
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????????????."));
							}
						}
						break;

						case 71052: // ?????
						{
							LPITEM item2;

							if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
								return false;

							if (item2->IsEquipped())
								return false;

							if (item2->GetType() == ITEM_COSTUME)
								return false;

							if (item2->IsExchanging() == true)
								return false;

							if (item2->GetAttributeSetIndex() == -1)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ?????? ?? ???? ??????????."));
								return false;
							}

							if (item2->ChangeRareAttribute() == true)
							{
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());
								LogManager::instance().ItemLog(this, item, "CHANGE_RARE_ATTR", buf);

								if (!g_bUnlimitedChangeRareAttributes)
									item->SetCount(item->GetCount() - 1);
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ????????????."));
							}
						}
						break;

						case ITEM_AUTO_HP_RECOVERY_S:
						case ITEM_AUTO_HP_RECOVERY_M:
						case ITEM_AUTO_HP_RECOVERY_L:
						case ITEM_AUTO_HP_RECOVERY_X:
						case ITEM_AUTO_SP_RECOVERY_S:
						case ITEM_AUTO_SP_RECOVERY_M:
						case ITEM_AUTO_SP_RECOVERY_L:
						case ITEM_AUTO_SP_RECOVERY_X:
							// ????????????? ?????? ??? ?? ?????? ??????...
							// ????? ??? ??? ???. ???? ????? ??????? ???????.
						case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS:
						case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S:
						case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS:
						case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S:
						{
							if (CArenaManager::instance().IsArenaMap(GetMapIndex()))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ?????? ?? ???????."));
								return false;
							}

							EAffectTypes type = AFFECT_NONE;
							bool isSpecialPotion = false;

							switch (item->GetVnum())
							{
								case ITEM_AUTO_HP_RECOVERY_X:
									isSpecialPotion = true;

								case ITEM_AUTO_HP_RECOVERY_S:
								case ITEM_AUTO_HP_RECOVERY_M:
								case ITEM_AUTO_HP_RECOVERY_L:
								case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_XS:
								case REWARD_BOX_ITEM_AUTO_HP_RECOVERY_S:
									type = AFFECT_AUTO_HP_RECOVERY;
									break;

								case ITEM_AUTO_SP_RECOVERY_X:
									isSpecialPotion = true;

								case ITEM_AUTO_SP_RECOVERY_S:
								case ITEM_AUTO_SP_RECOVERY_M:
								case ITEM_AUTO_SP_RECOVERY_L:
								case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_XS:
								case REWARD_BOX_ITEM_AUTO_SP_RECOVERY_S:
									type = AFFECT_AUTO_SP_RECOVERY;
									break;
							}

							if (AFFECT_NONE == type)
								break;

							if (item->GetCount() > 1)
							{
								int pos = GetEmptyInventory(item->GetSize());

								if (-1 == pos)
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ?? ?????? ???????."));
									break;
								}

								item->SetCount(item->GetCount() - 1);

								LPITEM item2 = ITEM_MANAGER::instance().CreateItem(item->GetVnum(), 1);
								item2->AddToCharacter(this, TItemPos(INVENTORY, pos));

								if (item->GetSocket(1) != 0)
								{
									item2->SetSocket(1, item->GetSocket(1));
								}

								item = item2;
							}

							CAffect* pAffect = FindAffect(type);

							if (NULL == pAffect)
							{
								EPointTypes bonus = POINT_NONE;

								if (true == isSpecialPotion)
								{
									if (type == AFFECT_AUTO_HP_RECOVERY)
									{
										bonus = POINT_MAX_HP_PCT;
									}
									else if (type == AFFECT_AUTO_SP_RECOVERY)
									{
										bonus = POINT_MAX_SP_PCT;
									}
								}

								AddAffect(type, bonus, 4, item->GetID(), INFINITE_AFFECT_DURATION, 0, true, false);

								item->Lock(true);
								item->SetSocket(0, true);

								AutoRecoveryItemProcess(type);
							}
							else
							{
								if (item->GetID() == pAffect->dwFlag)
								{
									RemoveAffect(pAffect);

									item->Lock(false);
									item->SetSocket(0, false);
								}
								else
								{
									LPITEM old = FindItemByID(pAffect->dwFlag);

									if (NULL != old)
									{
										old->Lock(false);
										old->SetSocket(0, false);
									}

									RemoveAffect(pAffect);

									EPointTypes bonus = POINT_NONE;

									if (true == isSpecialPotion)
									{
										if (type == AFFECT_AUTO_HP_RECOVERY)
										{
											bonus = POINT_MAX_HP_PCT;
										}
										else if (type == AFFECT_AUTO_SP_RECOVERY)
										{
											bonus = POINT_MAX_SP_PCT;
										}
									}

									AddAffect(type, bonus, 4, item->GetID(), INFINITE_AFFECT_DURATION, 0, true, false);

									item->Lock(true);
									item->SetSocket(0, true);

									AutoRecoveryItemProcess(type);
								}
							}
						}
						break;
					}
				}
				break;

				case USE_CLEAR:
				{
					switch (item->GetVnum())
					{
						case 27124: // Bandage
							RemoveBleeding();
							break;

						case 27874: // Grilled Perch
							RemoveBadAffect();
							break;
					}
					item->SetCount(item->GetCount() - 1);
				}
				break;

				case USE_INVISIBILITY:
				{
					if (item->GetVnum() == 70026)
					{
						quest::CQuestManager& q = quest::CQuestManager::instance();
						quest::PC* pPC = q.GetPC(GetPlayerID());

						if (pPC != NULL)
						{
							int last_use_time = pPC->GetFlag("mirror_of_disapper.last_use_time");

							if (get_global_time() - last_use_time < 10 * 60)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ????? ?? ???????."));
								return false;
							}

							pPC->SetFlag("mirror_of_disapper.last_use_time", get_global_time());
						}
					}

					AddAffect(AFFECT_INVISIBILITY, POINT_NONE, 0, AFF_INVISIBILITY, 300, 0, true);
					item->SetCount(item->GetCount() - 1);
				}
				break;

				case USE_POTION_NODELAY:
				{
					if (CArenaManager::instance().IsArenaMap(GetMapIndex()))
					{
						if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit") > 0)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ?????? ?? ???????."));
							return false;
						}

						switch (item->GetVnum())
						{
							case 70020:
							case 71018:
							case 71019:
							case 71020:
							{
								if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit_count") < 10000)
								{
									if (m_nPotionLimit <= 0)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ??????? ???????????."));
										return false;
									}
								}
							}
							break;

							default:
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ?????? ?? ???????."));
								return false;
						}
					}

					bool bUsed = false;

					if (item->GetValue(0) != 0) // HP ???? ???
					{
						if (GetHP() < GetMaxHP())
						{
							PointChange(POINT_HP, item->GetValue(0) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
							EffectPacket(SE_HPUP_RED);
							bUsed = true;
						}
					}

					if (item->GetValue(1) != 0) // SP ???? ???
					{
						if (GetSP() < GetMaxSP())
						{
							PointChange(POINT_SP, item->GetValue(1) * (100 + GetPoint(POINT_POTION_BONUS)) / 100);
							EffectPacket(SE_SPUP_BLUE);
							bUsed = true;
						}
					}

					if (item->GetValue(3) != 0) // HP % ???
					{
						if (GetHP() < GetMaxHP())
						{
							PointChange(POINT_HP, item->GetValue(3) * GetMaxHP() / 100);
							EffectPacket(SE_HPUP_RED);
							bUsed = true;
						}
					}

					if (item->GetValue(4) != 0) // SP % ???
					{
						if (GetSP() < GetMaxSP())
						{
							PointChange(POINT_SP, item->GetValue(4) * GetMaxSP() / 100);
							EffectPacket(SE_SPUP_BLUE);
							bUsed = true;
						}
					}

					if (bUsed)
					{
						if (item->GetVnum() == 50085 || item->GetVnum() == 50086)
						{
							if (test_server)
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ??? ???? ?? ???????????"));

							SetUseSeedOrMoonBottleTime();
						}

						if (GetDungeon())
							GetDungeon()->UsePotion(this);

						if (GetWarMap())
							GetWarMap()->UsePotion(this, item);

						m_nPotionLimit--;

						// RESTRICT_USE_SEED_OR_MOONBOTTLE
						item->SetCount(item->GetCount() - 1);
						// END_RESTRICT_USE_SEED_OR_MOONBOTTLE
					}
				}
				break;

				case USE_POTION:
				{
					if (CArenaManager::instance().IsArenaMap(GetMapIndex()) == true)
					{
						if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit") > 0)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ?????? ?? ???????."));
							return false;
						}

						switch (item->GetVnum())
						{
							case 27001: // Red Potion (S)
							case 27002: // Red Potion (M)
							case 27003: // Red Potion (L)
							case 27007: // Red Potion (XXL)
							case 27004: // Blue Potion (S)
							case 27005: // Blue Potion (M)
							case 27006: // Blue Potion (L)
							case 27008: // Red Potion (XXL)
							{
								if (quest::CQuestManager::instance().GetEventFlag("arena_potion_limit_count") < 10000)
								{
									if (m_nPotionLimit <= 0)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ??????? ???????????."));
										return false;
									}
								}
							}
							break;

							default:
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ?????? ?? ???????."));
								return false;
						}
					}

					if (item->GetValue(1) != 0)
					{
						if (GetPoint(POINT_SP_RECOVERY) + GetSP() >= GetMaxSP())
							return false;

						PointChange(POINT_SP_RECOVERY, item->GetValue(1) * MIN(200, (100 + GetPoint(POINT_POTION_BONUS))) / 100);
						StartAffectEvent();
						EffectPacket(SE_SPUP_BLUE);
					}

					if (item->GetValue(0) != 0)
					{
						if (GetPoint(POINT_HP_RECOVERY) + GetHP() >= GetMaxHP())
							return false;

						PointChange(POINT_HP_RECOVERY, item->GetValue(0) * MIN(200, (100 + GetPoint(POINT_POTION_BONUS))) / 100);
						StartAffectEvent();
						EffectPacket(SE_HPUP_RED);
					}

					if (GetDungeon())
						GetDungeon()->UsePotion(this);

					if (GetWarMap())
						GetWarMap()->UsePotion(this, item);

					item->SetCount(item->GetCount() - 1);
					m_nPotionLimit--;
				}
				break;

				case USE_POTION_CONTINUE:
				{
					if (item->GetValue(0) != 0)
						AddAffect(AFFECT_HP_RECOVER_CONTINUE, POINT_HP_RECOVER_CONTINUE, item->GetValue(0), 0, item->GetValue(2), 0, true);
					else if (item->GetValue(1) != 0)
						AddAffect(AFFECT_SP_RECOVER_CONTINUE, POINT_SP_RECOVER_CONTINUE, item->GetValue(1), 0, item->GetValue(2), 0, true);
					else
						return false;

					if (GetDungeon())
						GetDungeon()->UsePotion(this);

					if (GetWarMap())
						GetWarMap()->UsePotion(this, item);

					item->SetCount(item->GetCount() - 1);
				}
				break;

				case USE_ABILITY_UP:
				{
					if (FindAffect(AFFECT_BLEND, item->GetValue(0)))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ??? ??????."));
						return false;
					}

					switch (item->GetValue(0))
					{
						case APPLY_MOV_SPEED:
						{
							VERIFY_POTION(AFFECT_MOV_SPEED, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_MOV_SPEED, POINT_MOV_SPEED, item->GetValue(2), AFF_MOV_SPEED_POTION, item->GetValue(1), 0, true);
							EffectPacket(SE_DXUP_PURPLE);
						}
						break;

						case APPLY_ATT_SPEED:
						{
							VERIFY_POTION(AFFECT_ATT_SPEED, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_ATT_SPEED, POINT_ATT_SPEED, item->GetValue(2), AFF_ATT_SPEED_POTION, item->GetValue(1), 0, true);
							EffectPacket(SE_SPEEDUP_GREEN);
						}
						break;

						case APPLY_STR:
						{
							VERIFY_POTION(AFFECT_STR, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_STR, POINT_ST, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_DEX:
						{
							VERIFY_POTION(AFFECT_DEX, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_DEX, POINT_DX, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_CON:
						{
							VERIFY_POTION(AFFECT_CON, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_CON, POINT_HT, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_INT:
						{
							VERIFY_POTION(AFFECT_INT, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_INT, POINT_IQ, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_CAST_SPEED:
						{
							VERIFY_POTION(AFFECT_CAST_SPEED, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_CAST_SPEED, POINT_CASTING_SPEED, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_ATT_GRADE_BONUS:
						{
							VERIFY_POTION(AFFECT_ATT_GRADE, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_ATT_GRADE, POINT_ATT_GRADE_BONUS, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_DEF_GRADE_BONUS:
						{
							VERIFY_POTION(AFFECT_DEF_GRADE, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_DEF_GRADE, POINT_DEF_GRADE_BONUS, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_RESIST_MAGIC:
						{
							VERIFY_POTION(AFFECT_UNIQUE_ABILITY, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_RESIST_MAGIC, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
						}
						break;

#if defined(__CONQUEROR_LEVEL__)
						case APPLY_SUNGMA_STR:
						{
							VERIFY_POTION(AFFECT_SUNGMA_STR, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_SUNGMA_STR, POINT_SUNGMA_STR, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_SUNGMA_HP:
						{
							VERIFY_POTION(AFFECT_SUNGMA_HP, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_SUNGMA_HP, POINT_SUNGMA_HP, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_SUNGMA_MOVE:
						{
							VERIFY_POTION(AFFECT_SUNGMA_MOVE, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_SUNGMA_MOVE, POINT_SUNGMA_MOVE, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;

						case APPLY_SUNGMA_IMMUNE:
						{
							VERIFY_POTION(AFFECT_SUNGMA_IMMUNE, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_SUNGMA_IMMUNE, POINT_SUNGMA_IMMUNE, item->GetValue(2), 0, item->GetValue(1), 0, true);
						}
						break;
#endif

						case APPLY_ATTBONUS_MONSTER:
						{
							VERIFY_POTION(AFFECT_UNIQUE_ABILITY, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_ATTBONUS_MONSTER, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
						}
						break;

						case APPLY_MALL_EXPBONUS:
						{
							VERIFY_POTION(AFFECT_UNIQUE_ABILITY, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_MALL_EXPBONUS, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
						}
						break;

						case APPLY_MELEE_MAGIC_ATTBONUS_PER:
						{
							VERIFY_POTION(AFFECT_UNIQUE_ABILITY, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_MELEE_MAGIC_ATT_BONUS_PER, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
						}
						break;

						case APPLY_RESIST_ICE:
						{
							VERIFY_POTION(AFFECT_UNIQUE_ABILITY, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_RESIST_ICE, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
						}
						break;

						case APPLY_RESIST_EARTH:
						{
							VERIFY_POTION(AFFECT_UNIQUE_ABILITY, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_RESIST_EARTH, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
						}
						break;

						case APPLY_RESIST_DARK:
						{
							VERIFY_POTION(AFFECT_UNIQUE_ABILITY, aApplyInfo[item->GetValue(0)].wPointType);
							AddAffect(AFFECT_UNIQUE_ABILITY, POINT_RESIST_DARK, item->GetValue(2), 0, item->GetValue(1), 0, true, true);
						}
						break;
					}

					if (GetDungeon())
						GetDungeon()->UsePotion(this);

					if (GetWarMap())
						GetWarMap()->UsePotion(this, item);

					item->SetCount(item->GetCount() - 1);
				}
				break;

				case USE_TALISMAN:
				{
					const int TOWN_PORTAL = 1;
					const int MEMORY_PORTAL = 2;

					// gm_guild_build, oxevent ????? ????? ??????? ?? ??????? ????
					if (GetMapIndex() == 200 || GetMapIndex() == 113)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ??????? ????? ?? ???????."));
						return false;
					}

					if (CArenaManager::instance().IsArenaMap(GetMapIndex()))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????? ?? ???? ???????."));
						return false;
					}

					if (m_pkWarpEvent)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ??? ??????????? ?????? ?????? ???????"));
						return false;
					}

					// CONSUME_LIFE_WHEN_USE_WARP_ITEM
					int consumeLife = CalculateConsume(this);

					if (consumeLife < 0)
						return false;
					// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

					if (item->GetValue(0) == TOWN_PORTAL) // ?????
					{
						if (item->GetSocket(0) == 0)
						{
							if (!GetDungeon())
								if (!GiveRecallItem(item))
									return false;

							PIXEL_POSITION posWarp;

							if (SECTREE_MANAGER::instance().GetRecallPositionByEmpire(GetMapIndex(), GetEmpire(), posWarp))
							{
								// CONSUME_LIFE_WHEN_USE_WARP_ITEM
								PointChange(POINT_HP, -consumeLife, false);
								// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

								WarpSet(posWarp.x, posWarp.y);
							}
							else
							{
								sys_err("CHARACTER::UseItem : cannot find spawn position (name %s, %d x %d)", GetName(), GetX(), GetY());
							}
						}
						else
						{
							if (test_server)
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ????? ????"));

							ProcessRecallItem(item);
						}
					}
					else if (item->GetValue(0) == MEMORY_PORTAL) // ???????
					{
						if (item->GetSocket(0) == 0)
						{
							if (GetDungeon())
							{
								const char* c_szConv = (g_iUseLocale ? "" : (under_han(item->GetName()) ? LC_STRING("??") : LC_STRING("??")));
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ??????? %s%s ????? ?? ???????.", LC_ITEM(item->GetVnum()), c_szConv));
								return false;
							}

							if (!GiveRecallItem(item))
								return false;
						}
						else
						{
							// CONSUME_LIFE_WHEN_USE_WARP_ITEM
							PointChange(POINT_HP, -consumeLife, false);
							// END_OF_CONSUME_LIFE_WHEN_USE_WARP_ITEM

							ProcessRecallItem(item);
						}
					}
				}
				break;

				case USE_TUNING:
				case USE_DETACHMENT:
				{
					LPITEM item2;

					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsEquipped())
						return false;

					if (item2->IsExchanging())
						return false;

#if defined(__SOUL_BIND_SYSTEM__)
					if (item2->IsSealed())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot improve a soulbound item."));
						return false;
					}
#endif

					if (PreventTradeWindow(WND_ALL))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("?,?????? ???? ????????? ?????? ????? ???????"));
						return false;
					}

					RefineItem(item, item2);
				}
				break;

				// ACCESSORY_REFINE & ADD/CHANGE_ATTRIBUTES
				case USE_PUT_INTO_BELT_SOCKET:
				case USE_PUT_INTO_RING_SOCKET:
				case USE_PUT_INTO_ACCESSORY_SOCKET:
				case USE_ADD_ACCESSORY_SOCKET:
				case USE_CLEAN_SOCKET:
				case USE_CHANGE_ATTRIBUTE:
#if defined(__ATTR_6TH_7TH__)
				case USE_CHANGE_ATTRIBUTE2:
#endif
				case USE_ADD_ATTRIBUTE:
				case USE_ADD_ATTRIBUTE2:
#if defined(__MOVE_COSTUME_ATTR__)
				case USE_RESET_COSTUME_ATTR:
				case USE_CHANGE_COSTUME_ATTR:
#endif
#if defined(__CHANGED_ATTR__)
				case USE_SELECT_ATTRIBUTE:
#endif
				{
					LPITEM item2;
					if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
						return false;

					if (item2->IsEquipped())
					{
						BuffOnAttr_RemoveBuffsFromItem(item2);
					}

					// [NOTE] ???? ????????? ?????? ???? ?????? ???? ????? ??????, ????? ????? ??????? ????? ?????.
					// ???? ANTI_CHANGE_ATTRIBUTE ???? ?????? Flag?? ?????? ??? ???????? ??????? ????? ?? ?? ????? ?? ???????????
					// ????? ???????? ????? ???? ?????? ??? ???? ????... -_-
					/*
					if (IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_CHANGE_ATTRIBUTE) && !item->IsSocketModifyingItem())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot change the upgrade of this item."));
						return false;
					}
					*/

					if (ITEM_COSTUME == item2->GetType()
#if defined(__MOVE_COSTUME_ATTR__)	
						&& item->GetSubType() != USE_RESET_COSTUME_ATTR && item->GetSubType() != USE_CHANGE_COSTUME_ATTR
#endif
#if defined(__CHANGED_ATTR__)
						&& item->GetSubType() != USE_SELECT_ATTRIBUTE
#endif
						)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ?????? ?? ???? ??????????."));
						return false;
					}

					if (item2->IsExchanging() || item2->IsEquipped())
						return false;

					switch (item->GetSubType())
					{
						case USE_CLEAN_SOCKET:
						{
							int i;
							for (i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
							{
								if (item2->GetSocket(i) == ITEM_BROKEN_METIN_VNUM)
									break;
							}

							if (i == METIN_SOCKET_MAX_NUM)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ???? ???????? ??????."));
								return false;
							}

							int j = 0;

							for (i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
							{
								if (item2->GetSocket(i) != ITEM_BROKEN_METIN_VNUM && item2->GetSocket(i) != 0)
									item2->SetSocket(j++, item2->GetSocket(i));
							}

							for (; j < METIN_SOCKET_MAX_NUM; ++j)
							{
								if (item2->GetSocket(j) > 0)
									item2->SetSocket(j, 1);
							}

							{
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());
								LogManager::instance().ItemLog(this, item, "CLEAN_SOCKET", buf);
							}

							if (!g_bUnlimitedCleanSocket)
								item->SetCount(item->GetCount() - 1);
						}
						break;

						case USE_CHANGE_ATTRIBUTE:
						{
#if defined(__SOUL_BIND_SYSTEM__)
							if (item2->IsSealed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot change the attributes of a soulbound item."));
								return false;
							}
#endif

							if (IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_ENCHANT))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("Enchant Item cannot be used on this item."));
								return false;
							}

							if (item2->GetAttributeSetIndex() == -1)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ?????? ?? ???? ??????????."));
								return false;
							}

							if (item2->GetAttributeCount() == 0)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ????? ???????."));
								return false;
							}

							if (g_bChangeItemAttrCycle)
							{
								if (GM_PLAYER == GetGMLevel() && false == test_server)
								{
									//
									// Event Flag ?? ???? ?????? ?????? ??? ?????? ?? ????? ???? ????? ??? ?????? ??????
									// ??? ????? ????? ???? ??????? ???? ??? ?????? ???.
									//

									DWORD dwChangeItemAttrCycle = quest::CQuestManager::instance().GetEventFlag(msc_szChangeItemAttrCycleFlag);
									if (dwChangeItemAttrCycle < msc_dwDefaultChangeItemAttrCycle)
										dwChangeItemAttrCycle = msc_dwDefaultChangeItemAttrCycle;

									quest::PC* pPC = quest::CQuestManager::instance().GetPC(GetPlayerID());

									if (pPC)
									{
										DWORD dwNowMin = get_global_time() / 60;

										DWORD dwLastChangeItemAttrMin = pPC->GetFlag(msc_szLastChangeItemAttrFlag);

										if (dwLastChangeItemAttrMin + dwChangeItemAttrCycle > dwNowMin)
										{
											ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ????? %d?? ??????? ??? ?????? ?? ???????.(%d ?? ????)",
												dwChangeItemAttrCycle, dwChangeItemAttrCycle - (dwNowMin - dwLastChangeItemAttrMin)));
											return false;
										}

										pPC->SetFlag(msc_szLastChangeItemAttrFlag, dwNowMin);
									}
								}
							}

							if (item->GetVnum() == 76014)
							{
								int aiChangeProb[ITEM_ATTRIBUTE_MAX_LEVEL] =
								{
									0, 10, 50, 39, 1
								};

								item2->ChangeAttribute(aiChangeProb);
							}
							else
							{
								// ????? ????
								// ????? ???? ??? ????? ??? ??? ?????.
								if (item->GetVnum() == 71151 || item->GetVnum() == 76023)
								{
									if ((item2->GetType() == ITEM_WEAPON) || (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_BODY))
									{
										bool bCanUse = true;
										for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
										{
											if (item2->GetLimitType(i) == LIMIT_LEVEL && item2->GetLimitValue(i) > 40)
											{
												bCanUse = false;
												break;
											}
										}
										if (false == bCanUse)
										{
											ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ???????? ???? ????? ?????????."));
											break;
										}
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ??????? ??? ????????."));
										break;
									}
								}
								item2->ChangeAttribute();
							}

							ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ????????????."));
							{
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());
								LogManager::instance().ItemLog(this, item, "CHANGE_ATTRIBUTE", buf);
							}

							if (!g_bUnlimitedChangeAttributes)
								item->SetCount(item->GetCount() - 1);
						}
						break;

						case USE_ADD_ATTRIBUTE:
						{
							if (IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_REINFORCE))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("Reinforce Item cannot be used on this item."));
								return false;
							}

							if (item2->GetAttributeSetIndex() == -1)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ?????? ?? ???? ??????????."));
								return false;
							}

							if (item2->GetAttributeCount() < 4)
							{
								// ???? ????
								// ????? ???? ??? ????? ??? ??? ?????.
								if (item->GetVnum() == 71152 || item->GetVnum() == 76024)
								{
									if ((item2->GetType() == ITEM_WEAPON) || (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_BODY))
									{
										bool bCanUse = true;
										for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
										{
											if (item2->GetLimitType(i) == LIMIT_LEVEL && item2->GetLimitValue(i) > 40)
											{
												bCanUse = false;
												break;
											}
										}
										if (false == bCanUse)
										{
											ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ???????? ???? ????? ?????????."));
											break;
										}
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ??????? ??? ????????."));
										break;
									}
								}

								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());

								if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()] || g_bNeverFailAddAttributes == true)
								{
									item2->AddAttribute();
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????????????."));

									int iAddedIdx = item2->GetAttributeCount() - 1;
									LogManager::instance().ItemLog(
										GetPlayerID(),
										item2->GetAttributeType(iAddedIdx),
										item2->GetAttributeValue(iAddedIdx),
										item->GetID(),
										"ADD_ATTRIBUTE_SUCCESS",
										buf,
										GetDesc()->GetHostName(),
										item->GetOriginalVnum()
									);
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????????????."));
									LogManager::instance().ItemLog(this, item, "ADD_ATTRIBUTE_FAIL", buf);
								}

								if (!g_bUnlimitedAddAttributes)
									item->SetCount(item->GetCount() - 1);
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ?? ???????? ?????? ????? ????? ?? ???????."));
							}
						}
						break;

						case USE_ADD_ATTRIBUTE2:
						{
							if (IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_REINFORCE))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("Reinforce Item cannot be used on this item."));
								return false;
							}

							// ???? ????
							// ?????? ???? ????? 4?? ??? ??? ??????? ????? ????? ????? ?? ??????.
							if (item2->GetAttributeSetIndex() == -1)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ?????? ?? ???? ??????????."));
								return false;
							}

							// ????? ??? 4?? ??? ????? ???? ????? ??? ???????.
							if (item2->GetAttributeCount() == 4)
							{
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());

								if (number(1, 100) <= aiItemAttributeAddPercent[item2->GetAttributeCount()] || g_bNeverFailAddAttributes == true)
								{
									item2->AddAttribute();
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????????????."));

									int iAddedIdx = item2->GetAttributeCount() - 1;
									LogManager::instance().ItemLog(
										GetPlayerID(),
										item2->GetAttributeType(iAddedIdx),
										item2->GetAttributeValue(iAddedIdx),
										item->GetID(),
										"ADD_ATTRIBUTE2_SUCCESS",
										buf,
										GetDesc()->GetHostName(),
										item->GetOriginalVnum()
									);
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????????????."));
									LogManager::instance().ItemLog(this, item, "ADD_ATTRIBUTE2_FAIL", buf);
								}

								if (!g_bUnlimitedAddAttributes)
									item->SetCount(item->GetCount() - 1);
							}
							else if (item2->GetAttributeCount() == 5)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ??? ?? ???????? ?????? ????? ????? ?? ???????."));
							}
							else if (item2->GetAttributeCount() < 4)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ?????? ?????? ????? ??????? ?????."));
							}
							else
							{
								// wtf ?!
								sys_err("ADD_ATTRIBUTE2 : Item has wrong AttributeCount(%d)", item2->GetAttributeCount());
							}
						}
						break;

#if defined(__ATTR_6TH_7TH__)
						case USE_CHANGE_ATTRIBUTE2:
						{
#if defined(__SOUL_BIND_SYSTEM__)
							if (item2->IsSealed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot change the attributes of a soulbound item."));
								return false;
							}
#endif

							if (IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_ENCHANT))
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("Enchant Item cannot be used on this item."));
								return false;
							}

							if (item2->GetAttributeSetIndex() == -1)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ?????? ?? ???? ??????????."));
								return false;
							}

							if (item2->GetRareAttrCount() == 0)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ????? ???????."));
								return false;
							}

							if (number(1, 100) <= item->GetValue(0))
							{
								if (item2->ChangeRareAttribute())
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ????????????."));
									{
										char szBuff[21];
										snprintf(szBuff, sizeof(szBuff), "%u", item2->GetID());
										LogManager::instance().ItemLog(this, item, "CHANGE_ATTRIBUTE2", szBuff);
									}
								}
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("Upgrade change failed."));
							}

							item->SetCount(item->GetCount() - 1);
						}
						break;
#endif

						case USE_ADD_ACCESSORY_SOCKET:
						{
							char buf[21];
							snprintf(buf, sizeof(buf), "%u", item2->GetID());

							if (item2->IsAccessoryForSocket())
							{
								if (item2->GetAccessorySocketMaxGrade() < ITEM_ACCESSORY_SOCKET_MAX_NUM)
								{
									if (number(1, 100) <= 50 || g_bNeverFailAccessorySocket == true)
									{
										item2->SetAccessorySocketMaxGrade(item2->GetAccessorySocketMaxGrade() + 1);
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ?????????? ???????????."));
										LogManager::instance().ItemLog(this, item, "ADD_SOCKET_SUCCESS", buf);
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ????? ????????????."));
										LogManager::instance().ItemLog(this, item, "ADD_SOCKET_FAIL", buf);
									}

									if (!g_bUnlimitedAddAccessorySocket)
										item->SetCount(item->GetCount() - 1);
								}
								else
								{
									ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ??????????? ????? ?????? ????? ?????? ???????."));
								}
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ?????????? ?????? ????? ?? ???? ??????????."));
							}
						}
						break;

						case USE_PUT_INTO_BELT_SOCKET:
						case USE_PUT_INTO_ACCESSORY_SOCKET:
						{
							if (item2->IsAccessoryForSocket() && item->CanPutInto(item2))
							{
								char buf[21];
								snprintf(buf, sizeof(buf), "%u", item2->GetID());

								if (item2->GetAccessorySocketGrade() < item2->GetAccessorySocketMaxGrade())
								{
									if (number(1, 100) <= aiAccessorySocketPutPct[item2->GetAccessorySocketGrade()] || g_bNeverFailAccessory == true)
									{
										item2->SetAccessorySocketGrade(item2->GetAccessorySocketGrade() + 1);
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ????????????."));
										LogManager::instance().ItemLog(this, item, "PUT_SOCKET_SUCCESS", buf);
									}
									else
									{
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ????????????."));
										LogManager::instance().ItemLog(this, item, "PUT_SOCKET_FAIL", buf);
									}

									if (!g_bUnlimitedAddAccessory)
										item->SetCount(item->GetCount() - 1);
								}
								else
								{
									if (item2->GetAccessorySocketMaxGrade() == 0)
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ???????? ????????? ?????? ??????????."));
									else if (item2->GetAccessorySocketMaxGrade() < ITEM_ACCESSORY_SOCKET_MAX_NUM)
									{
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ??????????? ????? ?????? ?????? ???????."));
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("???????? ?????? ??????????."));
									}
									else
										ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ??????????? ????? ?????? ?????? ?? ???????."));
								}
							}
							else
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ???????? ?????? ?? ???????."));
							}
						}
						break;

#if defined(__MOVE_COSTUME_ATTR__)
						case USE_RESET_COSTUME_ATTR:
						case USE_CHANGE_COSTUME_ATTR:
						{
#if defined(__SOUL_BIND_SYSTEM__)
							if (item2->IsSealed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot change the attributes of a soulbound item."));
								return false;
							}
#endif

							if (!item2->CanChangeCostumeAttr())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ?????? ?? ???? ??????????."));
								return false;
							}

							// NOTE: Prevent changing costumes without bonus.
							if (item2->GetAttributeCount() == 0)
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ?????? ?? ???? ??????????."));
								return false;
							}

							switch (item->GetSubType())
							{
								case USE_RESET_COSTUME_ATTR:
								{
									item2->ClearAttribute();
									item2->AlterToMagicItem();

									char buf[21];
									snprintf(buf, sizeof(buf), "%u", item2->GetID());
									LogManager::instance().ItemLog(this, item, "RESET_COSTUME_ATTR", buf);

									if (!g_bUnlimitedResetCostumeAttributes)
										item->SetCount(item->GetCount() - 1);

									ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ????????????."));
								}
								break;

								case USE_CHANGE_COSTUME_ATTR:
								{
									item2->ChangeAttribute();

									char buf[21];
									snprintf(buf, sizeof(buf), "%u", item2->GetID());
									LogManager::instance().ItemLog(this, item, "CHANGE_COSTUME_ATTR", buf);

									if (!g_bUnlimitedChangeCostumeAttributes)
										item->SetCount(item->GetCount() - 1);

									ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ????????????."));
								}
								break;
							}
						}
						break;
#endif

#if defined(__CHANGED_ATTR__)
						case USE_SELECT_ATTRIBUTE:
						{
#if defined(__SOUL_BIND_SYSTEM__)
							if (item2->IsSealed())
							{
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot change the attributes of a soulbound item."));
								return false;
							}
#endif
							SelectAttr(item, item2);
						}
						break;
#endif
					}

					if (item2->IsEquipped())
					{
						BuffOnAttr_AddBuffsFromItem(item2);
					}
				}
				break;
				// END_OF_ACCESSORY_REFINE & END_OF_ADD_ATTRIBUTES & END_OF_CHANGE_ATTRIBUTES

				case USE_CALL:
					AggregateMonster();
					if (!g_bUnlimitedCapeOfCourage)
						item->SetCount(item->GetCount() - 1);
					break;

				case USE_BAIT:
				{
					if (m_pkFishingEvent)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ??? ????? ??????? ?? ???????."));
						return false;
					}

					LPITEM weapon = GetWear(WEAR_WEAPON);

					if (!weapon || weapon->GetType() != ITEM_ROD)
						return false;

					if (weapon->GetSocket(2))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ??????? ????? ???? %s?? ??????.", LC_ITEM(item->GetVnum())));
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? %s?? ????? ??????.", LC_ITEM(item->GetVnum())));
					}

					weapon->SetSocket(2, item->GetValue(0));
					item->SetCount(item->GetCount() - 1);
				}
				break;

				case USE_MOVE:
				case USE_TREASURE_BOX:
				case USE_MONEYBAG:
					break;

#if defined(ENABLE_TITLE_SYSTEM)
				case USE_TITLE:
				{
					int iDuration = item->GetValue(1);
					if (iDuration <= 0)
						iDuration = item->GetValue(2);

					const DWORD dwVnum = item->GetVnum();
					const int iBundle0[5] = { 1000, 1001, 1002, 1003, 1004 };
					const int iBundle1[5] = { 1005, 2000, 2001, 2002, 2003 };
					const int iBundle2[5] = { 3000, 3001, 3002, 3003, 3004 };

					const int* pBundle = NULL;
					int iBundleCount = 0;
					switch (dwVnum)
					{
						case 57000: pBundle = iBundle0; iBundleCount = 5; break;
						case 57001: pBundle = iBundle1; iBundleCount = 5; break;
						case 57002: pBundle = iBundle2; iBundleCount = 5; break;
					}

					if (pBundle && iBundleCount > 0)
					{
						int iUnlockedCount = 0;
						for (int i = 0; i < iBundleCount; ++i)
						{
							const int iTitleID = pBundle[i];
							char szOwned[64];
							char szExpire[64];
							snprintf(szOwned, sizeof(szOwned), "title_system.owned.%d", iTitleID);
							snprintf(szExpire, sizeof(szExpire), "title_system.expire.%d", iTitleID);
		
							if (GetQuestFlag(szOwned) > 0)
								continue;
		
							SetQuestFlag(szOwned, 1);
							if (iDuration > 0)
								SetQuestFlag(szExpire, get_global_time() + iDuration);
							else
								SetQuestFlag(szExpire, 0);
							++iUnlockedCount;
						}

						if (iUnlockedCount <= 0)
						{
							ChatPacket(CHAT_TYPE_INFO, "You already own all titles in this package.");
							break;
						}

						item->SetCount(item->GetCount() - 1);
						ChatPacket(CHAT_TYPE_INFO, "Title package unlocked: %d new titles.", iUnlockedCount);
						break;
					}

					int iTitleID = item->GetValue(0);
					if (iTitleID <= 0)
					{
						ChatPacket(CHAT_TYPE_INFO, "Invalid title certificate.");
						break;
					}

					char szOwned[64];
					char szExpire[64];
					snprintf(szOwned, sizeof(szOwned), "title_system.owned.%d", iTitleID);
					snprintf(szExpire, sizeof(szExpire), "title_system.expire.%d", iTitleID);
					if (GetQuestFlag(szOwned) > 0)
					{
						ChatPacket(CHAT_TYPE_INFO, "You already own this title.");
						break;
					}

					const int iActiveTitle = GetQuestFlag("title_system.active");
					if (iActiveTitle > 0 && iActiveTitle != iTitleID)
					{
						ChatPacket(CHAT_TYPE_INFO, "Remove your current title first.");
						break;
					}

					SetQuestFlag(szOwned, 1);
					if (iDuration > 0)
						SetQuestFlag(szExpire, get_global_time() + iDuration);
					else
						SetQuestFlag(szExpire, 0);

					SetQuestFlag("title_system.active", iTitleID);
					const char* c_szTitleEffect = "";
					switch (iTitleID)
					{
						case 1000: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_05_medal.mse"; break;
						case 1001: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_06_banner_gold.mse"; break;
						case 1002: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_07_banner_red.mse"; break;
						case 1003: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_08_banner_blue.mse"; break;
						case 1004: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_02_dragon.mse"; break;
						case 1005: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_01_shield.mse"; break;
						case 2000: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_04_trophy.mse"; break;
						case 2001: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_03_fist.mse"; break;
						case 2002: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_02_dragon.mse"; break;
						case 2003: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_01_shield.mse"; break;
						case 3000: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_06_banner_gold.mse"; break;
						case 3001: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_07_banner_red.mse"; break;
						case 3002: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_08_banner_blue.mse"; break;
						case 3003: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_02_dragon.mse"; break;
						case 3004: c_szTitleEffect = "d:/ymir work/effect/etc/title/title_05_medal.mse"; break;
					}

					if (c_szTitleEffect && *c_szTitleEffect)
						SpecificEffectPacket(c_szTitleEffect);

					ChatPacket(CHAT_TYPE_INFO, "Title unlocked and equipped (ID: %d)", iTitleID);
					item->SetCount(item->GetCount() - 1);
				}
				break;
#endif

				case USE_AFFECT:
				{
#if defined(__FLOWER_EVENT__)
					if (item->GetValue(0) == AFFECT_FLOWER_EVENT)
						return CFlowerEvent::UseFlower(this, item);
#endif

#if defined(__SUMMER_EVENT_ROULETTE__)
					if (item->GetValue(0) == AFFECT_LATE_SUMMER_EVENT_BUFF)
					{
						if (FindAffect(item->GetValue(0)) || FindAffect(AFFECT_LATE_SUMMER_EVENT_PRIMIUM_BUFF))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_STRING("This effect is already activated."));
							return false;
						}

						AddAffect(item->GetValue(0), POINT_NONE, 0, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true);
						item->SetCount(item->GetCount() - 1);
						return true;
					}
					else if (item->GetValue(0) == AFFECT_LATE_SUMMER_EVENT_PRIMIUM_BUFF)
					{
						if (FindAffect(item->GetValue(0)) || FindAffect(AFFECT_LATE_SUMMER_EVENT_BUFF))
						{
							ChatPacket(CHAT_TYPE_INFO, LC_STRING("This effect is already activated."));
							return false;
						}

						AddAffect(item->GetValue(0), POINT_NONE, 0, AFF_NONE, INFINITE_AFFECT_DURATION, 0, true);
						item->SetCount(item->GetCount() - 1);
						return true;
					}
#endif

#if defined(__LOOT_FILTER_SYSTEM__) && defined(__PREMIUM_LOOT_FILTER__)
					if (item->GetValue(0) == AFFECT_LOOTING_SYSTEM)
					{
						CAffect* pAffect = FindAffect(AFFECT_LOOTING_SYSTEM);
						long lDuration = item->GetValue(3);

						if (pAffect)
						{
							if (lDuration > 0)
							{
								if (pAffect->lDuration > LONG_MAX - lDuration)
								{
									sys_err("LOOT_FILTER_SYSTEM: Duration overflow, affect duration (%ld), item value3 (%ld)",
										pAffect->lDuration, lDuration);
									return false;
								}
							}
							else
							{
								sys_err("LOOT_FILTER_SYSTEM: Item duration value3 (%ld) is not positive.", lDuration);
								return false;
							}

							pAffect->bUpdate = true;
							pAffect->lApplyValue += lDuration;

							ChatPacket(CHAT_TYPE_NOTICE, LC_STRING("Loot filter duration extended."));
						}
						else
						{
#if defined(__AFFECT_RENEWAL__)
							AddRealTimeAffect(AFFECT_LOOTING_SYSTEM, POINT_NONE, 0, AFF_NONE, lDuration, 0, false, false);
#else
							AddAffect(AFFECT_LOOTING_SYSTEM, POINT_NONE, 0, AFF_NONE, lDuration, 0, false, false);
#endif
							ChatPacket(CHAT_TYPE_NOTICE, LC_STRING("Loot filter activated."));
						}

						item->SetCount(item->GetCount() - 1);
						return true;
					}
#endif

					if (FindAffect(item->GetValue(0), aApplyInfo[item->GetValue(1)].wPointType))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ??? ??????."));
					}
					else
					{
						AddAffect(item->GetValue(0), aApplyInfo[item->GetValue(1)].wPointType, item->GetValue(2), 0, item->GetValue(3), 0, false);
						item->SetCount(item->GetCount() - 1);
					}
				}
				break;

				case USE_CREATE_STONE:
				{
					AutoGiveItem(number(28000, 28012));
					item->SetCount(item->GetCount() - 1);
				}
				break;

				case USE_RECIPE:
				{
					LPITEM pSource1 = FindSpecifyItem(item->GetValue(1));
					DWORD dwSourceCount1 = item->GetValue(2);

					LPITEM pSource2 = FindSpecifyItem(item->GetValue(3));
					DWORD dwSourceCount2 = item->GetValue(4);

					if (dwSourceCount1 != 0)
					{
						if (pSource1 == NULL)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ?????? ???? ??? ????????."));
							return false;
						}
					}

					if (dwSourceCount2 != 0)
					{
						if (pSource2 == NULL)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ?????? ???? ??? ????????."));
							return false;
						}
					}

					if (pSource1 != NULL)
					{
						if (pSource1->GetCount() < dwSourceCount1)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_STRING("???(%s)?? ????????.", LC_ITEM(pSource1->GetVnum())));
							return false;
						}

						pSource1->SetCount(pSource1->GetCount() - dwSourceCount1);
					}

					if (pSource2 != NULL)
					{
						if (pSource2->GetCount() < dwSourceCount2)
						{
							ChatPacket(CHAT_TYPE_INFO, LC_STRING("???(%s)?? ????????.", LC_ITEM(pSource2->GetVnum())));
							return false;
						}

						pSource2->SetCount(pSource2->GetCount() - dwSourceCount2);
					}

					LPITEM pBottle = FindSpecifyItem(50901);

					if (!pBottle || pBottle->GetCount() < 1)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ???? ????????."));
						return false;
					}

					pBottle->SetCount(pBottle->GetCount() - 1);

					if (number(1, 100) > item->GetValue(5))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ?????? ??????????."));
						return false;
					}

					AutoGiveItem(item->GetValue(0));
				}
				break;
			}
		}
		break;

		case ITEM_METIN:
		{
			LPITEM item2;

			if (!IsValidItemPosition(DestCell) || !(item2 = GetItem(DestCell)))
				return false;

			if (item2->IsExchanging())
				return false;

			if (item2->IsEquipped())
				return false;

			// Block item add metin sock.
			if (IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_APPLY))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("This Spirit Stone cannot be attached to this type of item."));
				return false;
			}

			if (item2->GetType() == ITEM_PICK)
				return false;

			if (item2->GetType() == ITEM_ROD)
				return false;

			int i;

			for (i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
			{
				DWORD dwVnum;

				if ((dwVnum = item2->GetSocket(i)) <= 2)
					continue;

				const TItemTable* p = ITEM_MANAGER::instance().GetTable(dwVnum);

				if (!p)
					continue;

				if (item->GetValue(5) == p->alValues[5]
#if defined(__GLOVE_SYSTEM__)
					&& item->GetSubType() != METIN_SUNGMA
#endif
					)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot attach several stones of the same type."));
					return false;
				}
			}

			if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_BODY)
			{
				if (!IS_SET(item->GetWearFlag(), WEARABLE_BODY) || !IS_SET(item2->GetWearFlag(), WEARABLE_BODY))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("This Spirit Stone cannot be attached to this type of item."));
					return false;
				}
			}
#if defined(__GLOVE_SYSTEM__)
			else if (item2->GetType() == ITEM_ARMOR && item2->GetSubType() == ARMOR_GLOVE)
			{
				if (!IS_SET(item->GetWearFlag(), WEARABLE_GLOVE) || !IS_SET(item2->GetWearFlag(), WEARABLE_GLOVE))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("This Spirit Stone cannot be attached to this type of item."));
					return false;
				}
			}
#endif
			else if (item2->GetType() == ITEM_WEAPON)
			{
				if (!IS_SET(item->GetWearFlag(), WEARABLE_WEAPON))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot attach this Spirit Stone to a weapon."));
					return false;
				}
			}
			else
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("No slot free."));
				return false;
			}

			for (i = 0; i < METIN_SOCKET_MAX_NUM; ++i)
			{
				if (item2->GetSocket(i) >= 1 && item2->GetSocket(i) <= 2 && item2->GetSocket(i) >= item->GetValue(2))
				{
					if (number(1, 100) <= 30 || g_bNeverFailMetin == true)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("You have attached the Spirit Stone successfully."));

#if defined(__GLOVE_SYSTEM__)
						DWORD dwValue = item->GetVnum();
						if (item->GetSubType() == METIN_SUNGMA)
						{
							bool bMultiplier = false;
							if (item->GetValue(5) == 2 && number(1, 100) <= 30)
							{
								bMultiplier = true;
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("Perfect! You have attached the Spirit Stone successfully and you even receive a double stat bonus."));
							}
							dwValue = item2->GetRandomSungMaSocketValue(item->GetValue(5), item->GetRefineLevel(), bMultiplier);
						}
						item2->SetSocket(i, dwValue);
#else
						item2->SetSocket(i, item->GetVnum());
#endif
					}
					else
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("The Spirit Stone broke while being attached."));
						item2->SetSocket(i, ITEM_BROKEN_METIN_VNUM);
					}

					LogManager::instance().ItemLog(this, item2, "SOCKET", item->GetName());
					item->SetCount(item->GetCount() - 1);
					break;
				}
			}

			if (i == METIN_SOCKET_MAX_NUM)
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("No slot free."));
		}
		break;

		case ITEM_AUTOUSE:
		case ITEM_MATERIAL:
		case ITEM_SPECIAL:
		case ITEM_TOOL:
		case ITEM_LOTTERY:
			break;

		case ITEM_TOTEM:
		{
			if (!item->IsEquipped())
				EquipItem(item);
		}
		break;

		case ITEM_BLEND:
		{
			sys_log(0, "ITEM_BLEND!!");

			if (Blend_Item_find(item->GetVnum()))
			{
				int affect_type = AFFECT_BLEND;
				int apply_type = aApplyInfo[item->GetSocket(0)].wPointType;
				int apply_value = item->GetSocket(1);
				int apply_duration = item->GetSocket(2);

				if (FindAffect(affect_type, apply_type))
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("This effect is already activated."));
				}
				else
				{
					if (FindAffect(AFFECT_EXP_BONUS_EURO_FREE, POINT_RESIST_MAGIC))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("This effect is already activated."));
					}
					else
					{
						AddAffect(affect_type, apply_type, apply_value, 0, apply_duration, 0, false);
						item->SetCount(item->GetCount() - 1);
					}
				}
			}
		}
		break;

		case ITEM_EXTRACT:
		{
			LPITEM pDestItem = GetItem(DestCell);
			if (NULL == pDestItem)
				return false;

#if defined(__SOUL_BIND_SYSTEM__)
			if (item->IsSealed())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this item on soulbound items."));
				return false;
			}
#endif

			switch (item->GetSubType())
			{
#if defined(__DRAGON_SOUL_SYSTEM__)
				case EXTRACT_DRAGON_SOUL:
				{
					if (pDestItem->IsDragonSoul())
						return DSManager::instance().PullOut(this, NPOS, pDestItem, item);
					return false;
				}
				case EXTRACT_DRAGON_HEART:
				{
					if (pDestItem->IsDragonSoul())
						return DSManager::instance().ExtractDragonHeart(this, pDestItem, item);
					return false;
				}
#endif

				default:
					return false;
			}
		}
		break;

		case ITEM_PET:
		{
			switch (item->GetSubType())
			{
#if defined(__GROWTH_PET_SYSTEM__)
				case PET_EGG:
				{
					if (PreventTradeWindow(WND_EXCHANGE | WND_MYSHOP | WND_SHOPOWNER | WND_SAFEBOX | WND_CUBE))
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use the Incubator whilst trading or whilst you have your storeroom or private shop open."));
						return false;
					}
				}
				break;

				case PET_UPBRINGING:
				{
					if (m_pkFishingEvent)
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot carry out this action while fishing."));
						return false;
					}

					if (GetExchange())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot summon your pet whilst trading."));
						return false;
					}

					CGrowthPetSystem* petSystem = GetGrowthPetSystem();
					if (!petSystem)
						return false;

					// Check pet evolution
					const uint32_t dwPetCurrentVnum = GetQuestFlag("growthpet_system.growthpet_summoned");
					uint32_t dwPetMobVnum = item->GetValue(0);
					const uint8_t dwPetEvolution = item->GetGrowthPetItemInfo().evol_level;
					if (dwPetEvolution >= 4)
						dwPetMobVnum = item->GetValue(3);

					// Check Pet Dead
					if (static_cast<long>((item->GetSocket(0) - get_global_time()) / 60) <= 0)
						return false;

					const auto petLevel = static_cast<int>(item->GetGrowthPetItemInfo().pet_level);
					if (petLevel > GetLevel())
					{
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("Your level is too low to summon the pet."));
						return false;
					}

					if (petSystem->IsActivePet())
					{
						petSystem->Dismiss();
						if (dwPetMobVnum != dwPetCurrentVnum)
						{
							petSystem->Summon(dwPetMobVnum, item, "", false);
							SetQuestFlag("growthpet_system.growthpet_summoned", dwPetMobVnum);
							SetQuestFlag("growthpet_system.growthpet_id", item->GetID());
						}
						return false;
					}
					else
					{
						if (petSystem->CountSummoned() >= 1)
							return false;
		
						petSystem->Summon(dwPetMobVnum, item, "", false);
						SetQuestFlag("growthpet_system.growthpet_summoned", dwPetMobVnum);
						SetQuestFlag("growthpet_system.growthpet_id", item->GetID());
						return true;
					}
					return false;
				}
				break;

				case PET_BAG:
				{
					const long GetPetBagTime = (item->GetSocket(0) - get_global_time()) / 60;

					// Bagging
					if (GetPetBagTime <= 0)
					{
						LPITEM item2 = GetItem(DestCell);
						if (!item2)
							return false;
						if (item2->IsExchanging())
							return false;
						if (item2->GetType() != ITEM_PET)
							return false;
						if (item2->isLocked())
							return false;

						const long lExpireTime = time(0) + item->GetLimitValue(0);
						item->SetSocket(0, lExpireTime);
						item->SetSocket(1, item2->GetSocket(1));
						item->SetSocket(2, item2->GetSocket(2));

						item->SetGrowthPetItemInfo(item2->GetGrowthPetItemInfo());

						item->StartRealTimeExpireEvent();

						item->UpdatePacket();
						item->Save();

						ITEM_MANAGER::Instance().RemoveItem(item2, "PET_BAGGING");

						// Send Message
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s is now in a Transport Box."), item2->GetName());

						return true;
					}

					// Unbagging
					if (GetPetBagTime > 0)
					{
						if (item->isLocked())
							return false;

						if (item->IsExchanging())
							return false;

						const uint16_t wCell = item->GetCell();
						const uint32_t dwVnum = item->GetGrowthPetItemInfo().pet_vnum;
						if (!dwVnum)
							return false;

						LPITEM item2 = ITEM_MANAGER::Instance().CreateItem(dwVnum, 1, 0, true);
						if (!item2)
						{
							sys_err("<GrowthPet> there was an error removing the pet from the bag vnum: %d.", dwVnum);
							return false;
						}

						// Check Pet Dead
						if (static_cast<long>((item->GetSocket(0) - get_global_time()) / 60) <= 0)
							return false;

						const long lExpireTime = time(0) + item->GetGrowthPetItemInfo().pet_max_time;
						item2->SetSocket(0, lExpireTime);
						item2->SetSocket(1, item->GetSocket(1));
						item2->SetSocket(2, item->GetSocket(2));

						item2->SetGrowthPetItemInfo(item->GetGrowthPetItemInfo());

						item2->UpdatePacket();
						item2->Save();

						// Remove Bag
						ITEM_MANAGER::Instance().RemoveItem(item, "PET_UNBAGGING");

						// Add Pet to Character
						item2->AddToCharacter(this, TItemPos(INVENTORY, wCell));

						// Send Message
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s emerges from the Transport Box."), item2->GetName());

						return true;
					}
				}
				break;

				case PET_FEEDSTUFF:
				{
					LPITEM item2 = GetItem(DestCell);

					if (!item2)
						return false;

					if (item2->IsExchanging())
						return false;

					if (!((item2->GetType() == ITEM_PET) && (item2->GetSubType() == PET_UPBRINGING)))
						return false;

					bool isDead = false;
					const long duration = (item2->GetSocket(0) - get_global_time()) / 60;
					if (duration < 0)
						isDead = true;

					TGrowthPetInfo petInfo = item2->GetGrowthPetItemInfo();

					time_t pet_duration = petInfo.pet_max_time;
					if (isDead)
					{
						pet_duration /= 2;
						petInfo.pet_birthday = time(0);
					}

					item2->SetSocket(0, time(0) + pet_duration);
					item2->SetGrowthPetItemInfo(petInfo);
		
					item->SetCount(item->GetCount() - 1);

					if (isDead)
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s wakes up and dives into the tasty meal."), item2->GetName());
					else
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("Your pet enthusiastically tucks into the Protein Snack."));
		
					return true;
				}
				break;
#endif

			default:
				quest::CQuestManager::Instance().UseItem(GetPlayerID(), item, false);
				break;
			}
		}
		break;

#if defined(__GACHA_SYSTEM__)
		case ITEM_GACHA:
		{
			switch (item->GetSubType())
			{
				case USE_GACHA:
				{
					if (GiveItemFromSpecialItemGroup(item->GetVnum()))
					{
						if (item->GetSocket(0) > 1)
							item->SetSocket(0, item->GetSocket(0) - 1);
						else
							ITEM_MANAGER::instance().RemoveItem(item, "REMOVE (ITEM_GACHA)");
					}
				}
				break;

#if defined(__LUCKY_BOX__)
				case GEM_LUCKY_BOX_GACHA:
				case SPECIAL_LUCKY_BOX_GACHA:
					SetLuckyBoxSrcItem(item);
					break;
#endif
			}
		}
		break;
#endif

#if defined(__SOUL_SYSTEM__)
		case ITEM_SOUL:
		{
			struct SoulData
			{
				WORD point_type;
				DWORD affect_flag;
			};
			std::unordered_map<BYTE, SoulData> soul_map
			{
				{ RED_SOUL, { AFF_SOUL_RED, AFF_SOUL_RED }, },
				{ BLUE_SOUL, { AFF_SOUL_BLUE, AFF_SOUL_BLUE }, },
			};

			const auto& it = soul_map.find(item->GetSubType());
			if (it == soul_map.end())
			{
				sys_err("ITEM_SOUL: Unknown SubType");
				return false;
			}

			const auto& soul = it->second;

			CAffect* affect = nullptr;
			for (auto const& it : GetAffectContainer())
			{
				if (it != nullptr && it->dwType == AFFECT_SOUL)
				{
					if (it->wApplyOn == soul.point_type)
					{
						if (item->GetSocket(1) != TRUE)
						{
							ChatPacket(CHAT_TYPE_INFO, "The soul color is already beeing used.");
							return false;
						}
					}

					affect = it;
					continue;
				}
			}

			if (item->GetSocket(1) != TRUE)
			{
				AddAffect(AFFECT_SOUL, soul.point_type, item->GetID(), affect ? AFF_NONE : soul.affect_flag, item->GetSocket(0), 0, false/*override*/);

				if (affect)
				{
					std::vector<CAffect> tmp_affects;
					std::vector<CAffect*> to_remove;
					for (const auto& [k, v] : soul_map)
					{
						CAffect* affect = FindAffect(AFFECT_SOUL, v.point_type);
						if (affect)
						{
							tmp_affects.emplace_back(*affect);
							to_remove.emplace_back(affect);
						}
					}

					if (!to_remove.empty())
					{
						for (const auto& it : to_remove)
							RemoveAffect(it);
					}

					if (!tmp_affects.empty())
					{
						for (const auto& it : tmp_affects)
							AddAffect(it.dwType, it.wApplyOn, it.lApplyValue, AFF_NONE, it.lDuration, 0, false/*override*/);
					}

					AddAffect(AFFECT_SOUL, AFF_SOUL_MIX, 0, AFF_SOUL_MIX, item->GetSocket(0), 0, false/*override*/);
				}

				item->SetSocket(1, TRUE);
				item->Lock(true);
			}
			else
			{
				RemoveAffect(AFFECT_SOUL, soul.point_type);
				RemoveAffect(AFFECT_SOUL, AFF_SOUL_MIX); // AFF_SOUL_MIX

				std::vector<CAffect> tmp_affects;
				std::vector<CAffect*> to_remove;
				for (const auto& affect : GetAffectContainer())
				{
					if (affect != nullptr && affect->dwType == AFFECT_SOUL)
					{
						tmp_affects.emplace_back(*affect);
						to_remove.emplace_back(affect);
					}
				}

				for (const auto& affect : to_remove)
					RemoveAffect(affect);

				if (!tmp_affects.empty())
				{
					for (const auto& affect : tmp_affects)
					{
						auto it = std::find_if(soul_map.begin(), soul_map.end(), [&](const auto& kv_pair) {
							return kv_pair.second.point_type == affect.wApplyOn;
							});

						if (it != soul_map.end())
							AddAffect(affect.dwType, affect.wApplyOn, affect.lApplyValue, it->second.affect_flag, affect.lDuration, 0, false/*override*/);
					}
				}

				item->SetSocket(1, FALSE);
				item->Lock(false);
			}
		}
		break;
#endif

		case ITEM_NONE:
			sys_err("Item type NONE %s", item->GetName());
			break;

		default:
			sys_log(0, "UseItemEx: Unknown type %s %d", item->GetName(), item->GetType());
			return false;
	}

	return true;
}

int g_nPortalLimitTime = 10;

bool CHARACTER::UseItem(TItemPos Cell, TItemPos DestCell)
{
	WORD wCell = Cell.cell;
	BYTE window_type = Cell.window_type;

	//WORD wDestCell = DestCell.cell;
	//BYTE bDestInven = DestCell.window_type;

	LPITEM item;

	if (!CanHandleItem())
	{
#if defined(__GROWTH_PET_SYSTEM__)
		if (IsGrowthPetDetermineWindow())
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use items while modifying your pet's stats."));
#endif
		return false;
	}

	if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
		return false;

	// We don't want to use it if we are dragging it over another item of the same type...
	LPITEM destItem = GetItem(DestCell);
	if (destItem && item != destItem && destItem->IsStackable() && !IS_SET(destItem->GetAntiFlag(), ITEM_ANTIFLAG_STACK) && destItem->GetVnum() == item->GetVnum())
	{
		if (MoveItem(Cell, DestCell, 0))
			return false;
	}

	sys_log(0, "%s: USE_ITEM %s (inven %d, cell: %d)", GetName(), item->GetName(), window_type, wCell);

	if (item->IsExchanging())
		return false;

	if (!item->CanUsedBy(this))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ??????? ?? ???????? ????? ?? ???????."));
		return false;
	}

	if (IsStun())
		return false;

#if defined(ENABLE_MONSTER_CARD)
	if (item->GetVnum() == kalisto::MonstercardSystem::s_MONSTERCARD_VNUM || item->GetVnum() == kalisto::MonstercardSystem::s_MONSTERCARD_TRADEABLE_VNUM ||
		item->GetVnum() == kalisto::MonstercardSystem::s_MONSTERCARD_USE_ITEM_VNUM_A || item->GetVnum() == kalisto::MonstercardSystem::s_MONSTERCARD_USE_ITEM_VNUM_B)
	{
		const auto& sys = GetMonsterCardSystem();
		if (sys != nullptr)
		{
			if (sys->UseMonstercard(item))
			{
				item->SetCount(item->GetCount() - 1);
				return true;
			}
			return false;
		}
	}
#endif

	if (false == FN_check_item_sex(this, item))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ??????? ?? ???????? ????? ?? ???????."));
		return false;
	}

#if defined(__CHANGE_LOOK_SYSTEM__)
	DWORD dwTransmutationVnum = item->GetTransmutationVnum();
	if (dwTransmutationVnum != 0)
	{
		TItemTable* pItemTable = ITEM_MANAGER::instance().GetTable(dwTransmutationVnum);

		if (!pItemTable->CanUseByJob(GetJob()))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("[Transmutation] You cannot equip the transmuted item as it does not match your class."));
			return false;
		}

#if defined(__COSTUME_SYSTEM__)
		if (pItemTable && pItemTable->IsCostume())
		{
			if ((IS_SET(pItemTable->GetAntiFlags(), ITEM_ANTIFLAG_MALE) && SEX_MALE == GET_SEX(this)) ||
				(IS_SET(pItemTable->GetAntiFlags(), ITEM_ANTIFLAG_FEMALE) && SEX_FEMALE == GET_SEX(this)))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("[Transmutation] You cannot equip the transmuted item as it does not match your gender."));
				return false;
			}
		}
	}
#endif
#endif

	// PREVENT_TRADE_WINDOW
	if (IS_SUMMON_ITEM(item, GetMapIndex()))
	{
		if (false == IS_SUMMONABLE_ZONE(GetMapIndex()))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this item here."));
			return false;
		}

		if (CThreeWayWar::instance().IsThreeWayWarMapIndex(GetMapIndex()))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("???? ???? ????????? ?????,???????? ?????? ???????."));
			return false;
		}

		int iPulse = thecore_pulse();

		// ? ???? ??
		if (iPulse - GetSafeboxLoadTime() < PASSES_PER_SEC(g_nPortalLimitTime))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ???? %d?? ??????? ?????,???????? ????? ?? ???????.", g_nPortalLimitTime));

			if (test_server)
				ChatPacket(CHAT_TYPE_INFO, "[TestOnly]Pulse %d LoadTime %d PASS %d", iPulse, GetSafeboxLoadTime(), PASSES_PER_SEC(g_nPortalLimitTime));
			return false;
		}

		if (PreventTradeWindow(WND_ALL))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("????,? ???? ?? ????????? ?????,??????? ?? ?????? ???????."));
			return false;
		}

		// PREVENT_REFINE_HACK
		{
			if (iPulse - GetRefineTime() < PASSES_PER_SEC(g_nPortalLimitTime))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ?????? %d?? ??????? ?????,???????? ????? ?? ???????.", g_nPortalLimitTime));
				return false;
			}
		}
		// END_PREVENT_REFINE_HACK

		// PREVENT_ITEM_COPY
		{
			if (iPulse - GetMyShopTime() < PASSES_PER_SEC(g_nPortalLimitTime))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ????? %d?? ??????? ?????,???????? ????? ?? ???????.", g_nPortalLimitTime));
				return false;
			}

#if defined(__MAILBOX__)
			if (iPulse - GetMyMailBoxTime() < PASSES_PER_SEC(g_nPortalLimitTime))
			{
				ChatPacket(CHAT_TYPE_INFO, "You cannot use a Return Scroll %d seconds after opening a mailbox.", g_nPortalLimitTime);
				return false;
			}
#endif
		}
		// END_PREVENT_ITEM_COPY

		// if (item->GetVnum() != 70302)
		if (item->GetType() == ITEM_USE && item->GetSubType() == USE_TALISMAN)
		{
			PIXEL_POSITION posWarp;

			int x = 0;
			int y = 0;

			double nDist = 0;
			const double nDistant = 5000.0;

			if (item->GetVnum() == 22010)
			{
				x = item->GetSocket(0) - GetX();
				y = item->GetSocket(1) - GetY();
			}
			else if (item->GetVnum() == 22000)
			{
				SECTREE_MANAGER::instance().GetRecallPositionByEmpire(GetMapIndex(), GetEmpire(), posWarp);

				if (item->GetSocket(0) == 0)
				{
					x = posWarp.x - GetX();
					y = posWarp.y - GetY();
				}
				else
				{
					x = item->GetSocket(0) - GetX();
					y = item->GetSocket(1) - GetY();
				}
			}

			nDist = sqrt(pow((float)x, 2) + pow((float)y, 2));

			if (nDistant > nDist)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ????? ????? ??? ????? ?????? ?????? ???????."));
				if (test_server)
					ChatPacket(CHAT_TYPE_INFO, "PossibleDistant %f nNowDist %f", nDistant, nDist);
				return false;
			}
		}

		// PREVENT_PORTAL_AFTER_EXCHANGE
		if (iPulse - GetExchangeTime() < PASSES_PER_SEC(g_nPortalLimitTime))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("??? ?? %d?? ??????? ?????,?????????? ????? ?? ???????.", g_nPortalLimitTime));
			return false;
		}
		// END_PREVENT_PORTAL_AFTER_EXCHANGE
	}

	if ((item->GetVnum() == 50200) || (item->GetVnum() == 71049))
	{
		if (PreventTradeWindow(WND_ALL))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("????,? ???? ?? ????????? ??????,?????????? ?????? ???????."));
			return false;
		}
	}
	// END_PREVENT_TRADE_WINDOW

	// Prevent item usage while running quest.
	if (IsRunningQuest())
		return false;

#if defined(ENABLE_PASSIVE_SYSTEM)
	if (item->GetVnum() == 100100 || item->GetVnum() == 100101)
	{
		LPITEM pkRelic = GetWear(WEAR_PASSIVE);
		if (!pkRelic)
		{
			ChatPacket(CHAT_TYPE_INFO, "Equip a relic first.");
			return false;
		}

		const int iSuccessChance = (item->GetVnum() == 100101) ? 100 : 30;
		const int iEmptyPos = GetEmptyInventory(pkRelic->GetSize());
		if (iEmptyPos < 0)
		{
			ChatPacket(CHAT_TYPE_INFO, "You need an empty inventory slot.");
			return false;
		}

		if (item->GetCount() > 0)
			item->SetCount(item->GetCount() - 1);

		if (pkRelic->GetSocket(1) != 0)
		{
			pkRelic->ModifyPoints(false);
			pkRelic->SetSocket(1, 0);
			RemoveAffect(AFFECT_PASSIVE_RELIC_STONE_DEF);
			RemoveAffect(AFFECT_PASSIVE_RELIC_DISMOUNT_SPEED);
			ComputeBattlePoints();
			UpdatePacket();
		}

		if (number(1, 100) > iSuccessChance)
		{
			pkRelic->RemoveFromCharacter();
			pkRelic->SetCount(0);
			ChatPacket(CHAT_TYPE_INFO, "Relic extraction failed. The relic was destroyed.");
			return true;
		}

		pkRelic->RemoveFromCharacter();
		pkRelic->AddToCharacter(this, TItemPos(INVENTORY, iEmptyPos));
		ChatPacket(CHAT_TYPE_INFO, "Relic extraction succeeded.");
		return true;
	}
#endif

	if (IS_SET(item->GetFlag(), ITEM_FLAG_LOG))
	{
		DWORD vid = item->GetVID();
		DWORD oldCount = item->GetCount();
		DWORD vnum = item->GetVnum();

		char hint[ITEM_NAME_MAX_LEN + 32 + 1];
		int len = snprintf(hint, sizeof(hint) - 32, "%s", item->GetName());

		if (len < 0 || len >= (int)sizeof(hint) - 32)
			len = (sizeof(hint) - 32) - 1;

		bool ret = UseItemEx(item, DestCell);

		if (NULL == ITEM_MANAGER::instance().FindByVID(vid)) // UseItemEx
		{
			LogManager::instance().ItemLog(this, vid, vnum, "REMOVE", hint);
		}
		else if (oldCount != item->GetCount())
		{
			snprintf(hint + len, sizeof(hint) - len, " %u", oldCount - 1);
			LogManager::instance().ItemLog(this, vid, vnum, "USE_ITEM", hint);
		}
		return (ret);
	}
	else
		return UseItemEx(item, DestCell);
}

bool CHARACTER::DropItem(TItemPos Cell, WORD wCount)
{
	LPITEM item = NULL;

	if (!CanHandleItem())
	{
#if defined(__DRAGON_SOUL_SYSTEM__)
		if (NULL != DragonSoul_RefineWindow_GetOpener())
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot move the item within the refinement window."));
#endif

#if defined(__GROWTH_PET_SYSTEM__)
		if (IsGrowthPetDetermineWindow())
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot move the item within the refinement window."));
#endif
		return false;
	}

	if (IsDead())
		return false;

	if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
		return false;

	if (item->IsExchanging())
		return false;

	if (true == item->isLocked())
		return false;

#if defined(__SOUL_BIND_SYSTEM__)
	if (item->IsSealed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot drop a soulbound item."));
		return false;
	}
#endif

	if (IsRunningQuest())
		return false;

	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP | ITEM_ANTIFLAG_GIVE))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot drop this item."));
		return false;
	}

	if (wCount == 0 || wCount > item->GetCount())
		wCount = item->GetCount();

	if (INVENTORY == Cell.window_type)
		SyncQuickslot(SLOT_TYPE_INVENTORY, Cell.cell, WORD_MAX);
	else if (BELT_INVENTORY == Cell.window_type)
		SyncQuickslot(SLOT_TYPE_BELT_INVENTORY, Cell.cell, WORD_MAX);

	LPITEM pkItemToDrop;

	if (wCount == item->GetCount())
	{
		item->RemoveFromCharacter();
		pkItemToDrop = item;
	}
	else
	{
		if (wCount == 0)
		{
			if (test_server)
				sys_log(0, "[DROP_ITEM] drop item count == 0");
			return false;
		}

		item->SetCount(item->GetCount() - wCount);
		ITEM_MANAGER::instance().FlushDelayedSave(item);

		pkItemToDrop = ITEM_MANAGER::instance().CreateItem(item->GetVnum(), wCount);

		// Copy item socket -- by mhh
		FN_copy_item_socket(pkItemToDrop, item);

		char szBuf[51 + 1];
		snprintf(szBuf, sizeof(szBuf), "%u %u", pkItemToDrop->GetID(), pkItemToDrop->GetCount());
		LogManager::instance().ItemLog(this, item, "ITEM_SPLIT", szBuf);
	}

	PIXEL_POSITION pxPos = GetXYZ();

	if (pkItemToDrop->AddToGround(GetMapIndex(), pxPos))
	{
		if (LC_IsYMIR())
			item->AttrLog();

		ChatPacket(CHAT_TYPE_INFO, LC_STRING("The dropped item will vanish in 5 minutes."));
		pkItemToDrop->StartDestroyEvent();

		ITEM_MANAGER::instance().FlushDelayedSave(pkItemToDrop);

		char szHint[32 + 1];
		snprintf(szHint, sizeof(szHint), "%s %u %u", pkItemToDrop->GetName(), pkItemToDrop->GetCount(), pkItemToDrop->GetOriginalVnum());
		LogManager::instance().ItemLog(this, pkItemToDrop, "DROP", szHint);
		//Motion(MOTION_PICKUP);
	}

	return true;
}

#if defined(__NEW_DROP_DIALOG__)
bool CHARACTER::DestroyItem(TItemPos Cell)
{
	LPITEM item = NULL;

	if (!CanHandleItem())
	{
#if defined(__DRAGON_SOUL_SYSTEM__)
		if (NULL != DragonSoul_RefineWindow_GetOpener())
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot move the item within the refinement window."));
#endif
#if defined(__GROWTH_PET_SYSTEM__)
		if (IsGrowthPetDetermineWindow())
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot move the item within the refinement window."));
#endif
		return false;
	}

	if (IsDead())
		return false;

	if (!IsValidItemPosition(Cell) || !(item = GetItem(Cell)))
		return false;

	if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DESTROY))
		return false;

	if (item->IsExchanging())
		return false;

	if (true == item->isLocked())
		return false;

	if (IsRunningQuest())
		return false;

	if (item->GetCount() <= 0)
		return false;

#if defined(__SOUL_BIND_SYSTEM__)
	if (item->IsSealed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot destroy a soulbound item."));
		return false;
	}
#endif

	if (INVENTORY == Cell.window_type)
		SyncQuickslot(SLOT_TYPE_INVENTORY, Cell.cell, WORD_MAX);
	else if (BELT_INVENTORY == Cell.window_type)
		SyncQuickslot(SLOT_TYPE_BELT_INVENTORY, Cell.cell, WORD_MAX);

	ChatPacket(CHAT_TYPE_INFO, LC_STRING("You have deleted %s.", LC_ITEM(item->GetVnum())));
#if defined(__GROWTH_PET_SYSTEM__)
	if (item->GetType() == ITEM_PET && item->GetSubType() == PET_UPBRINGING)
		ITEM_MANAGER::Instance().RemoveItem(item, "DESTROYED BY PLAYER");
	else
		ITEM_MANAGER::Instance().RemoveItem(item, "DESTROYED BY PLAYER");
#else
	ITEM_MANAGER::instance().RemoveItem(item, "DESTROYED BY PLAYER");
#endif

	return true;
}
#endif

bool CHARACTER::DropGold(int iAmount)
{
	if (iAmount <= 0 || iAmount > GetGold())
		return false;

	if (!CanHandleItem())
		return false;

	if (0 != g_GoldDropTimeLimitValue)
	{
		if (get_dword_time() < m_dwLastGoldDropTime + g_GoldDropTimeLimitValue)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot throw the Gold away yet."));
			return false;
		}
	}

	m_dwLastGoldDropTime = get_dword_time();

	LPITEM item = ITEM_MANAGER::instance().CreateItem(1, iAmount);

	if (item)
	{
		PIXEL_POSITION pos = GetXYZ();

		if (item->AddToGround(GetMapIndex(), pos))
		{
			//Motion(MOTION_PICKUP);
			PointChange(POINT_GOLD, -iAmount, true);

			if (LC_IsBrazil() == true)
			{
				if (iAmount >= 213)
					LogManager::instance().CharLog(this, iAmount, "DROP_GOLD", "");
			}
			else
			{
				if (iAmount > 1000)
					LogManager::instance().CharLog(this, iAmount, "DROP_GOLD", "");
			}

			if (false == LC_IsBrazil())
			{
				item->StartDestroyEvent(150);
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("The dropped item will disappear after %d minutes.", 150 / 60));
			}
			else
			{
				item->StartDestroyEvent(60);
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("The dropped item will disappear after %d minutes.", 1));
			}
		}

		Save();
		return true;
	}

	return false;
}

#if defined(__CHEQUE_SYSTEM__)
bool CHARACTER::DropCheque(int cheque)
{
	if (cheque <= 0 || (long long)cheque > GetCheque())
		return false;

	if (!CanHandleItem())
		return false;

	if (0 != g_ChequeDropTimeLimitValue)
	{
		if (get_dword_time() < m_dwLastChequeDropTime + g_ChequeDropTimeLimitValue)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot throw the Gold away yet."));
			return false;
		}
	}

	m_dwLastChequeDropTime = get_dword_time();

	LPITEM item = ITEM_MANAGER::instance().CreateItem(80020, cheque);

	if (item)
	{
		PIXEL_POSITION pos = GetXYZ();

		if (item->AddToGround(GetMapIndex(), pos))
		{
			PointChange(POINT_CHEQUE, -cheque, true);

			if (cheque > 1000)
				LogManager::instance().CharLog(this, cheque, "DROP_CHEQUE", "");

			if (false == LC_IsBrazil())
			{
				item->StartDestroyEvent(150);
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("The dropped item will disappear after %d minutes.", 150 / 60));
			}
			else
			{
				item->StartDestroyEvent(60);
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("The dropped item will disappear after %d minutes.", 1));
			}
		}

		Save();
		return true;
	}

	return false;
}
#endif

bool CHARACTER::MoveItem(TItemPos Cell, TItemPos DestCell, WORD count)
{
	LPITEM item = nullptr;

	if (Cell.IsSameItemPosition(DestCell))
		return false;

	if (!IsValidItemPosition(Cell))
		return false;

	if (!(item = GetItem(Cell)))
		return false;

	if (item->IsExchanging())
		return false;

	if (item->GetCount() < count)
		return false;

#if defined(__EXTEND_INVEN_SYSTEM__)
	if (INVENTORY == Cell.window_type && Cell.cell >= GetExtendInvenMax() && IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		return false;
#else
	if (INVENTORY == Cell.window_type && Cell.cell >= INVENTORY_MAX_NUM && IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		return false;
#endif

	if (true == item->isLocked())
		return false;

	if (!IsValidItemPosition(DestCell))
		return false;

	if (!CanHandleItem())
	{
#if defined(__DRAGON_SOUL_SYSTEM__)
		if (DragonSoul_RefineWindow_GetOpener())
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot move the item within the refinement window."));
#endif
#if defined(__GROWTH_PET_SYSTEM__)
		if (IsGrowthPetDetermineWindow())
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot move the item within the refinement window."));
#endif
		return false;
	}

	if (DestCell.IsBeltInventoryPosition())
	{
		// At the request of the planner, only certain types of items can be placed in the belt inventory.
		if (!CBeltInventoryHelper::CanMoveIntoBeltInventory(item))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You can only move stones into this inventory."));
			return false;
		}
	}

	if (Cell.IsEquipPosition())
	{
		if (!CanUnequipNow(item))
			return false;

#if defined(__WEAPON_COSTUME_SYSTEM__)
		const int iWearCell = item->FindEquipCell(this);
		if (iWearCell == WEAR_WEAPON)
		{
			LPITEM pkCostumeWeapon = GetWear(WEAR_COSTUME_WEAPON);
			if (pkCostumeWeapon)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("If you want to change weapons, you must remove the weapon skin first."));
				return false;
			}
		}
#endif
	}

	if (DestCell.IsEquipPosition())
	{
		if (GetItem(DestCell)
#if defined(__DRAGON_SOUL_SYSTEM__)
			&& DestCell.IsDragonSoulEquipPosition()
#endif
			)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You have already equipped this kind of Dragon Stone."));
			return false;
		}

		EquipItem(item, DestCell.cell);
	}
	else
	{
#if defined(__DRAGON_SOUL_SYSTEM__)
		if (item->IsDragonSoul())
		{
			if (item->IsEquipped())
			{
				return DSManager::instance().PullOut(this, DestCell, item);
			}
			else
			{
				if (DestCell.window_type != DRAGON_SOUL_INVENTORY)
				{
					return false;
				}

				if (!DSManager::instance().IsValidCellForThisItem(item, DestCell))
					return false;
			}
		}
		else if (DRAGON_SOUL_INVENTORY == DestCell.window_type)
			return false;
#endif

		LPITEM item2;

		if ((item2 = GetItem(DestCell)) && item != item2 && item2->IsStackable() &&
			!IS_SET(item2->GetAntiFlag(), ITEM_ANTIFLAG_STACK) &&
			item2->GetVnum() == item->GetVnum() && !item2->IsExchanging())
		{
			for (int i = 0; i < ITEM_SOCKET_MAX_NUM; ++i)
				if (item2->GetSocket(i) != item->GetSocket(i))
					return false;

			if (count == 0)
				count = item->GetCount();

			sys_log(0, "%s: ITEM_STACK %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell,
				DestCell.window_type, DestCell.cell, count);

			count = MIN(ITEM_MAX_COUNT - item2->GetCount(), count);

			item->SetCount(item->GetCount() - count);
			item2->SetCount(item2->GetCount() + count);

			return true;
		}

		if (!IsEmptyItemGrid(DestCell, item->GetSize(), Cell.cell))
		{
			return false;
		}

		if (count == 0 || count >= item->GetCount() || !item->IsStackable() || IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
		{
			sys_log(0, "%s: ITEM_MOVE %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell,
				DestCell.window_type, DestCell.cell, count);

			item->RemoveFromCharacter();
			SetItem(DestCell, item);

			if (INVENTORY == Cell.window_type && INVENTORY == DestCell.window_type)
				SyncQuickslot(SLOT_TYPE_INVENTORY, Cell.cell, DestCell.cell);
			else if (BELT_INVENTORY == Cell.window_type && BELT_INVENTORY == DestCell.window_type)
				SyncQuickslot(SLOT_TYPE_BELT_INVENTORY, Cell.cell, DestCell.cell);

			else if (INVENTORY == Cell.window_type && BELT_INVENTORY == DestCell.window_type)
				MoveQuickSlotItem(SLOT_TYPE_INVENTORY, Cell.cell, SLOT_TYPE_BELT_INVENTORY, DestCell.cell);
			else if (BELT_INVENTORY == Cell.window_type && INVENTORY == DestCell.window_type)
				MoveQuickSlotItem(SLOT_TYPE_BELT_INVENTORY, Cell.cell, SLOT_TYPE_INVENTORY, DestCell.cell);
		}
		else if (count < item->GetCount())
		{
			sys_log(0, "%s: ITEM_SPLIT %s (window: %d, cell : %d) -> (window:%d, cell %d) count %d", GetName(), item->GetName(), Cell.window_type, Cell.cell, DestCell.window_type, DestCell.cell, count);

			item->SetCount(item->GetCount() - count);
			LPITEM item2 = ITEM_MANAGER::instance().CreateItem(item->GetVnum(), count);

			// Copy socket -- by mhh
			FN_copy_item_socket(item2, item);

			item2->AddToCharacter(this, DestCell);

			char szBuf[51 + 1];
			snprintf(szBuf, sizeof(szBuf), "%u %u %u %u ", item2->GetID(), item2->GetCount(), item->GetCount(), item->GetCount() + item2->GetCount());
			LogManager::instance().ItemLog(this, item, "ITEM_SPLIT", szBuf);
		}
	}

	return true;
}

namespace NPartyPickupDistribute
{
	struct FFindOwnership
	{
		LPITEM item;
		LPCHARACTER owner;

		FFindOwnership(LPITEM item)
			: item(item), owner(NULL)
		{
		}

		void operator () (LPCHARACTER ch)
		{
			if (item->IsOwnership(ch))
				owner = ch;
		}
	};

	struct FCountNearMember
	{
		int total;
		int x, y;

		FCountNearMember(LPCHARACTER center)
			: total(0), x(center->GetX()), y(center->GetY())
		{
		}

		void operator () (LPCHARACTER ch)
		{
			if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
				total += 1;
		}
	};

	struct FMoneyDistributor
	{
		int total;
		LPCHARACTER c;
		int x, y;
		int iMoney;

		FMoneyDistributor(LPCHARACTER center, int iMoney)
			: total(0), c(center), x(center->GetX()), y(center->GetY()), iMoney(iMoney)
		{
		}

		void operator ()(LPCHARACTER ch)
		{
			if (ch != c)
				if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
				{
					ch->PointChange(POINT_GOLD, iMoney, true);

					if (iMoney > 1000)
						LogManager::instance().CharLog(ch, iMoney, "GET_GOLD", "");
				}
		}
	};

#if defined(__CHEQUE_SYSTEM__)
	struct FChequeDistributor
	{
		int total;
		LPCHARACTER c;
		int x, y;
		int iCheque;

		FChequeDistributor(LPCHARACTER center, int iCheque)
			: total(0), c(center), x(center->GetX()), y(center->GetY()), iCheque(iCheque)
		{
		}

		void operator ()(LPCHARACTER ch)
		{
			if (ch != c)
				if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
				{
					ch->PointChange(POINT_CHEQUE, iCheque, true);

					if (iCheque > 1000)
						LogManager::instance().CharLog(ch, iCheque, "GET_CHEQUE", "");
				}
		}
	};
#endif

#if defined(__GEM_SYSTEM__)
	struct FGemDistributor
	{
		int total;
		LPCHARACTER c;
		int x, y;
		int iGem;

		FGemDistributor(LPCHARACTER center, int iGem)
			: total(0), c(center), x(center->GetX()), y(center->GetY()), iGem(iGem)
		{
		}

		void operator ()(LPCHARACTER ch)
		{
			if (ch != c)
				if (DISTANCE_APPROX(ch->GetX() - x, ch->GetY() - y) <= PARTY_DEFAULT_RANGE)
				{
					ch->PointChange(POINT_GEM, iGem, true);

					if (iGem > 1000)
						LogManager::instance().CharLog(ch, iGem, "GET_GEM", "");
				}
		}
	};
#endif
}

void CHARACTER::GiveGold(int iAmount)
{
	if (iAmount <= 0)
		return;

	sys_log(0, "GIVE_GOLD: %s %d", GetName(), iAmount);

	if (GetParty())
	{
		LPPARTY pParty = GetParty();

		DWORD dwTotal = iAmount;
		DWORD dwMyAmount = dwTotal;

		NPartyPickupDistribute::FCountNearMember funcCountNearMember(this);
		pParty->ForEachOnMapMember(funcCountNearMember, GetMapIndex());

		if (funcCountNearMember.total > 1)
		{
			DWORD dwShare = dwTotal / funcCountNearMember.total;
			dwMyAmount -= dwShare * (funcCountNearMember.total - 1);

			NPartyPickupDistribute::FMoneyDistributor funcMoneyDist(this, dwShare);
			pParty->ForEachOnMapMember(funcMoneyDist, GetMapIndex());
		}

		PointChange(POINT_GOLD, dwMyAmount, true);

		if (dwMyAmount > 1000)
			LogManager::instance().CharLog(this, dwMyAmount, "GET_GOLD", "");
	}
	else
	{
		PointChange(POINT_GOLD, iAmount, true);

		if (LC_IsBrazil() == true)
		{
			if (iAmount >= 213)
				LogManager::instance().CharLog(this, iAmount, "GET_GOLD", "");
		}
		else
		{
			if (iAmount > 1000)
				LogManager::instance().CharLog(this, iAmount, "GET_GOLD", "");
		}
	}
}

#if defined(__CHEQUE_SYSTEM__)
void CHARACTER::GiveCheque(int iAmount)
{
	if (iAmount <= 0)
		return;

	sys_log(0, "GIVE_CHEQUE: %s %lld", GetName(), iAmount);

	if (GetParty())
	{
		LPPARTY pParty = GetParty();

		DWORD dwTotal = iAmount;
		DWORD dwMyAmount = dwTotal;

		NPartyPickupDistribute::FCountNearMember funcCountNearMember(this);
		pParty->ForEachOnMapMember(funcCountNearMember, GetMapIndex());

		if (funcCountNearMember.total > 1)
		{
			DWORD dwShare = dwTotal / funcCountNearMember.total;
			dwMyAmount -= dwShare * (funcCountNearMember.total - 1);

			NPartyPickupDistribute::FChequeDistributor funcMoneyDist(this, dwShare);
			pParty->ForEachOnMapMember(funcMoneyDist, GetMapIndex());
		}

		PointChange(POINT_CHEQUE, dwMyAmount, true);

		if (dwMyAmount > 1000)
			LogManager::instance().CharLog(this, dwMyAmount, "GET_CHEQUE", "");
	}
	else
	{
		PointChange(POINT_CHEQUE, iAmount, true);

		if (LC_IsBrazil() == true)
		{
			if (iAmount >= 213)
				LogManager::instance().CharLog(this, iAmount, "GET_CHEQUE", "");
		}
		else
		{
			if (iAmount > 1000)
				LogManager::instance().CharLog(this, iAmount, "GET_CHEQUE", "");
		}
	}
}
#endif

#if defined(__GEM_SYSTEM__)
void CHARACTER::GiveGem(int iAmount)
{
	if (iAmount <= 0)
		return;

	sys_log(0, "GIVE_GEM: %s %lld", GetName(), iAmount);

	if (GetParty())
	{
		LPPARTY pParty = GetParty();

		DWORD dwTotal = iAmount;
		DWORD dwMyAmount = dwTotal;

		NPartyPickupDistribute::FCountNearMember funcCountNearMember(this);
		pParty->ForEachOnMapMember(funcCountNearMember, GetMapIndex());

		if (funcCountNearMember.total > 1)
		{
			DWORD dwShare = dwTotal / funcCountNearMember.total;
			dwMyAmount -= dwShare * (funcCountNearMember.total - 1);

			NPartyPickupDistribute::FGemDistributor funcMoneyDist(this, dwShare);
			pParty->ForEachOnMapMember(funcMoneyDist, GetMapIndex());
		}

		PointChange(POINT_GEM, dwMyAmount, true);

		if (dwMyAmount > 1000)
			LogManager::instance().CharLog(this, dwMyAmount, "GET_GEM", "");
	}
	else
	{
		PointChange(POINT_GEM, iAmount, true);

		if (LC_IsBrazil() == true)
		{
			if (iAmount >= 213)
				LogManager::instance().CharLog(this, iAmount, "GET_GEM", "");
		}
		else
		{
			if (iAmount > 1000)
				LogManager::instance().CharLog(this, iAmount, "GET_GEM", "");
		}
	}
}
#endif

bool CHARACTER::PickupItem(DWORD dwVID
#if defined(__PET_LOOT_AI__)
	, bool bPetLoot
#endif
)
{
	if (IsDead())
		return false;

	LPITEM item = ITEM_MANAGER::instance().FindByVID(dwVID);

	if (IsObserverMode())
		return false;

	if (!item || !item->GetSectree())
		return false;

	if (item->DistanceValid(this)
#if defined(__PET_LOOT_AI__)
		|| bPetLoot
#endif
		)
	{
		if (item->GetType() == ITEM_QUEST || IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE)
			|| (item->GetType() == ITEM_PET && item->GetSubType() == PET_PAY))
		{
			if (IsRunningQuest())
				return false;
		}

		if (item->IsOwnership(this))
		{
#if defined(__LOOT_FILTER_SYSTEM__)
			if (GetLootFilter() && !GetLootFilter()->CanPickUpItem(item))
			{
				if (!GetLootFilter()->IsLootFilteredItem(dwVID))
				{
					GetLootFilter()->InsertLootFilteredItem(dwVID);

					if (GetDesc())
					{
						TPacketGCLootFilter p;
						p.header = HEADER_GC_LOOT_FILTER;
						p.enable = false;
						p.vid = dwVID;
						GetDesc()->Packet(&p, sizeof(p));
					}
				}

				ChatPacket(CHAT_TYPE_INFO, LC_STRING("No loot will be collected as the loot filter is deactivated."));
				return false;
			}
#endif

			if (item->GetType() == ITEM_ELK)
			{
				GiveGold(item->GetCount());
				item->RemoveFromGround();

				M2_DESTROY_ITEM(item);

				Save();
			}
			else
			{
				if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
				{
					WORD wCount = item->GetCount();

					for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
					{
						LPITEM item2 = GetInventoryItem(i);

						if (!item2)
							continue;

						if (item2->GetVnum() == item->GetVnum())
						{
							int j;

							for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
							{
								if (item2->GetSocket(j) != item->GetSocket(j))
									break;
							}

							if (j != ITEM_SOCKET_MAX_NUM)
								continue;

							WORD wCount2 = MIN(ITEM_MAX_COUNT - item2->GetCount(), wCount);

							wCount -= wCount2;

							item2->SetCount(item2->GetCount() + wCount2);

							if (wCount == 0)
							{
#if defined(__CHATTING_WINDOW_RENEWAL__)
#if defined(__SET_ITEM__)
								ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_PRE_ITEM(item2->GetItemSetValue(), item2->GetVnum())));
#else
								ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_ITEM(item2->GetVnum())));
#endif
#else
#if defined(__SET_ITEM__)
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_PRE_ITEM(item2->GetItemSetValue(), item2->GetVnum())));
#else
								ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_ITEM(item2->GetVnum())));
#endif
#endif
								M2_DESTROY_ITEM(item);

								if (item2->GetType() == ITEM_QUEST)
									quest::CQuestManager::instance().PickupItem(GetPlayerID(), item2);

								return true;
							}
						}
					}

					item->SetCount(wCount);
				}

				int iEmptyCell;
#if defined(__DRAGON_SOUL_SYSTEM__)
				if (item->IsDragonSoul())
				{
					if ((iEmptyCell = GetEmptyDragonSoulInventory(item)) == -1)
					{
						sys_log(0, "No empty ds inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("You have too many items in your inventory."));
						return false;
					}
				}
				else
#endif
				{
					if ((iEmptyCell = GetEmptyInventory(item->GetSize())) == -1)
					{
						sys_log(0, "No empty inventory pid %u size %ud itemid %u", GetPlayerID(), item->GetSize(), item->GetID());
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("You have too many items in your inventory."));
						return false;
					}
				}

				item->RemoveFromGround();

#if defined(__DRAGON_SOUL_SYSTEM__)
				if (item->IsDragonSoul())
					item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell));
				else
#endif
					item->AddToCharacter(this, TItemPos(INVENTORY, iEmptyCell));

				char szHint[32 + 1];
				snprintf(szHint, sizeof(szHint), "%s %u %u", item->GetName(), item->GetCount(), item->GetOriginalVnum());
				LogManager::instance().ItemLog(this, item, "GET", szHint);

#if defined(__CHATTING_WINDOW_RENEWAL__)
#if defined(__SET_ITEM__)
				ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
#else
				ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_ITEM(item->GetVnum())));
#endif
#else
#if defined(__SET_ITEM__)
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
#else
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_ITEM(item->GetVnum())));
#endif
#endif

				if (item->GetType() == ITEM_QUEST || IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE)
					|| (item->GetType() == ITEM_PET && item->GetSubType() == PET_PAY))
				{
					quest::CQuestManager::instance().PickupItem(GetPlayerID(), item);
				}
			}

			//Motion(MOTION_PICKUP);
			return true;
		}
		// Suppressed the inability to pickup items from the party that are within the anti flags.
		else if (!IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_GIVE | ITEM_ANTIFLAG_DROP) && GetParty())
		{
			NPartyPickupDistribute::FFindOwnership funcFindOwnership(item);
			GetParty()->ForEachOnMapMember(funcFindOwnership, GetMapIndex());

			LPCHARACTER owner = funcFindOwnership.owner;
			if (!owner)
				return false;

			int iEmptyCell;
#if defined(__DRAGON_SOUL_SYSTEM__)
			if (item->IsDragonSoul())
			{
				if (!(owner && (iEmptyCell = owner->GetEmptyDragonSoulInventory(item)) != -1))
				{
					owner = this;

					if ((iEmptyCell = GetEmptyDragonSoulInventory(item)) == -1)
					{
						owner->ChatPacket(CHAT_TYPE_INFO, LC_STRING("You have too many items in your inventory."));
						return false;
					}
				}
			}
			else
#endif
			{
				if (!(owner && (iEmptyCell = owner->GetEmptyInventory(item->GetSize())) != -1))
				{
					owner = this;

					if ((iEmptyCell = GetEmptyInventory(item->GetSize())) == -1)
					{
						owner->ChatPacket(CHAT_TYPE_INFO, LC_STRING("You have too many items in your book inventory."));
						return false;
					}
				}
			}

			if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
			{
				WORD wCount = item->GetCount();

				for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
				{
					LPITEM item2 = owner->GetInventoryItem(i);

					if (!item2)
						continue;

					if (item2->GetVnum() == item->GetVnum())
					{
						int j;

						for (j = 0; j < ITEM_SOCKET_MAX_NUM; ++j)
						{
							if (item2->GetSocket(j) != item->GetSocket(j))
								break;
						}

						if (j != ITEM_SOCKET_MAX_NUM)
							continue;

						WORD wCount2 = MIN(ITEM_MAX_COUNT - item2->GetCount(), wCount);

						wCount -= wCount2;

						item2->SetCount(item2->GetCount() + wCount2);

						if (wCount == 0)
						{
#if defined(__CHATTING_WINDOW_RENEWAL__)
#if defined(__SET_ITEM__)
							owner->ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_PRE_ITEM(item2->GetItemSetValue(), item2->GetVnum())));
#else
							owner->ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_ITEM(item2->GetVnum())));
#endif
#else
#if defined(__SET_ITEM__)
							owner->ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_PRE_ITEM(item2->GetItemSetValue(), item2->GetVnum())));
#else
							owner->ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_ITEM(item2->GetVnum())));
#endif
#endif
							M2_DESTROY_ITEM(item);

							if (item2->GetType() == ITEM_QUEST)
								quest::CQuestManager::instance().PickupItem(owner->GetPlayerID(), item2);

							return true;
						}
					}
				}

				item->SetCount(wCount);
			}

#if defined(__LOOT_FILTER_SYSTEM__)
			if (owner->GetLootFilter() && !owner->GetLootFilter()->CanPickUpItem(item))
			{
				if (owner == this)
				{
					if (!owner->GetLootFilter()->IsLootFilteredItem(dwVID))
					{
						owner->GetLootFilter()->InsertLootFilteredItem(dwVID);

						if (owner->GetDesc())
						{
							TPacketGCLootFilter p;
							p.header = HEADER_GC_LOOT_FILTER;
							p.enable = false;
							p.vid = dwVID;
							owner->GetDesc()->Packet(&p, sizeof(p));
						}
					}

					ChatPacket(CHAT_TYPE_INFO, LC_STRING("No loot will be collected as the loot filter is deactivated."));
				}
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("No loot will be collected as a party member's loot filter is deactivated."));
				}

				return false;
			}
#endif

			item->RemoveFromGround();

#if defined(__DRAGON_SOUL_SYSTEM__)
			if (item->IsDragonSoul())
				item->AddToCharacter(owner, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell));
			else
#endif
				item->AddToCharacter(owner, TItemPos(INVENTORY, iEmptyCell));

			char szHint[32 + 1];
			snprintf(szHint, sizeof(szHint), "%s %u %u", item->GetName(), item->GetCount(), item->GetOriginalVnum());
			LogManager::instance().ItemLog(owner, item, "GET", szHint);

			if (owner == this)
			{
#if defined(__CHATTING_WINDOW_RENEWAL__)
#if defined(__SET_ITEM__)
				ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
#else
				ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_ITEM(item->GetVnum())));
#endif
#else
#if defined(__SET_ITEM__)
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
#else
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_ITEM(item->GetVnum())));
#endif
#endif
			}
			else
			{
#if defined(__CHATTING_WINDOW_RENEWAL__)
#if defined(__SET_ITEM__)
				owner->ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s receives %s.", GetName(), LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
				ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("Item Trade: %s, %s", owner->GetName(), LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
#else
				owner->ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s receives %s.", GetName(), LC_ITEM(item->GetVnum())));
				ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("Item Trade: %s, %s", owner->GetName(), LC_ITEM(item->GetVnum())));
#endif
#else
#if defined(__SET_ITEM__)
				owner->ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s receives %s.", GetName(), LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("Item Trade: %s, %s", owner->GetName(), LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
#else
				owner->ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s receives %s.", GetName(), LC_ITEM(item->GetVnum())));
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("Item Trade: %s, %s", owner->GetName(), LC_ITEM(item->GetVnum())));
#endif
#endif
			}

			if (item->GetType() == ITEM_QUEST || IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE | ITEM_FLAG_QUEST_USE_MULTIPLE)
				|| (item->GetType() == ITEM_PET && item->GetSubType() == PET_PAY))
			{
				quest::CQuestManager::instance().PickupItem(owner->GetPlayerID(), item);
			}

			return true;
		}
	}

	return false;
}

#define VERIFY_MSG(exp, msg) \
	if (true == (exp)) \
	{ \
		ChatPacket(CHAT_TYPE_INFO, LC_STRING(msg)); \
		return false; \
	}

bool CHARACTER::SwapItem(WORD wCell, WORD wDestCell)
{
	if (!CanHandleItem())
		return false;

	const TItemPos srcCell(INVENTORY, wCell), destCell(EQUIPMENT, wDestCell);

	//if (bCell >= INVENTORY_MAX_NUM + WEAR_MAX_NUM || bDestCell >= INVENTORY_MAX_NUM + WEAR_MAX_NUM)
#if defined(__DRAGON_SOUL_SYSTEM__)
	if (srcCell.IsDragonSoulEquipPosition() || destCell.IsDragonSoulEquipPosition())
		return false;
#endif

	if (srcCell.IsEquipPosition() && destCell.IsEquipPosition())
		return false;

	LPITEM item1 = nullptr, item2 = nullptr;

	// item2?? ???? ??? ???? ?????.
	if (srcCell.IsEquipPosition())
	{
		item1 = GetEquipmentItem(wDestCell);
		item2 = GetInventoryItem(wCell);
	}
	else
	{
		item1 = GetInventoryItem(wCell);
		item2 = GetEquipmentItem(wDestCell);
	}

	if (!item1 || !item2)
		return false;

	if (item1 == item2)
	{
		sys_log(0, "[WARNING][WARNING][HACK USER!] : %s %d %d", m_stName.c_str(), wCell, wDestCell);
		return false;
	}

	if (!IsEmptyItemGrid(TItemPos(INVENTORY, item1->GetCell()), item2->GetSize(), item1->GetCell()))
		return false;

	if (TItemPos(EQUIPMENT, item2->GetCell()).IsEquipPosition())
	{
		const WORD wEquipCell = item2->GetCell();
		const WORD wInvenCell = item1->GetCell();

		// The item currently being worn can be removed, and the item planned to be worn
		// must be in a wearable state in order to proceed.
		if (!CanUnequipNow(item2, TItemPos(INVENTORY, wEquipCell)) || !CanEquipNow(item1))
			return false;

		if (wEquipCell != item1->FindEquipCell(this)) // When in the same location, it is only allowed
			return false;

		item2->RemoveFromCharacter();

		if (item1->EquipTo(this, wEquipCell))
#if defined(__WJ_PICKUP_ITEM_EFFECT__)
			item2->AddToCharacter(this, TItemPos(INVENTORY, wInvenCell), false);
#else
			item2->AddToCharacter(this, TItemPos(INVENTORY, wInvenCell));
#endif
		else
			sys_err("SwapItem cannot equip %s! item1 %s", item2->GetName(), item1->GetName());
	}
	else
	{
		const WORD wCell1 = item1->GetCell();
		const WORD wCell2 = item2->GetCell();

		item1->RemoveFromCharacter();
		item2->RemoveFromCharacter();

#if defined(__WJ_PICKUP_ITEM_EFFECT__)
		item1->AddToCharacter(this, TItemPos(EQUIPMENT, wCell2), false);
		item2->AddToCharacter(this, TItemPos(INVENTORY, wCell1), false);
#else
		item1->AddToCharacter(this, TItemPos(INVENTORY, wCell2));
		item2->AddToCharacter(this, TItemPos(INVENTORY, wCell1));
#endif
	}

	return true;
}

bool CHARACTER::UnequipItem(LPITEM item)
{
#if defined(__WEAPON_COSTUME_SYSTEM__)
	if (item->FindEquipCell(this) == WEAR_WEAPON)
	{
		LPITEM pCostumeWeapon = GetWear(WEAR_COSTUME_WEAPON);
		if (pCostumeWeapon)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("If you want to change weapons, you must remove the weapon skin first."));
			return false;
		}
	}
#endif

	if (false == CanUnequipNow(item))
		return false;

#if defined(__MOUNT_COSTUME_SYSTEM__)
	if (item->IsCostumeMount() && item->IsEquipped())
		UnMount();
#endif

	int pos;
#if defined(__DRAGON_SOUL_SYSTEM__)
	if (item->IsDragonSoul())
		pos = GetEmptyDragonSoulInventory(item);
	else
#endif
		pos = GetEmptyInventory(item->GetSize());

	// HARD CODING
	if (item->GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
		ShowAlignment(true);

	item->RemoveFromCharacter();
#if defined(__DRAGON_SOUL_SYSTEM__)
	if (item->IsDragonSoul())
#if defined(__WJ_PICKUP_ITEM_EFFECT__)
		item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, pos), false);
#else
		item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, pos));
#endif
	else
#endif
#if defined(__WJ_PICKUP_ITEM_EFFECT__)
		item->AddToCharacter(this, TItemPos(INVENTORY, pos), false);
#else
		item->AddToCharacter(this, TItemPos(INVENTORY, pos));
#endif

	CheckMaximumPoints();

	return true;
}

bool CHARACTER::EquipItem(LPITEM item, int iCandidateCell)
{
	if (item->IsExchanging())
		return false;

	if (false == item->IsEquipable())
		return false;

	if (false == CanEquipNow(item))
		return false;

	int iWearCell = item->FindEquipCell(this, iCandidateCell);

	if (iWearCell < 0)
		return false;

	if (iWearCell == WEAR_BODY && IsRiding() && (item->GetVnum() >= 11901 && item->GetVnum() <= 11904))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use it whilst riding a Horse."));
		return false;
	}

	if (iWearCell != WEAR_ARROW && IsPolymorphed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot change the equipped item while you are transformed."));
		return false;
	}

	if (FN_check_item_sex(this, item) == false)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You are not able to use that item because you do not have the right gender."));
		return false;
	}

#if defined(__CHANGE_LOOK_SYSTEM__)
	DWORD dwTransmutationVnum = item->GetTransmutationVnum();
	if (dwTransmutationVnum != 0)
	{
		TItemTable* pItemTable = ITEM_MANAGER::instance().GetTable(dwTransmutationVnum);

		if (!pItemTable->CanUseByJob(GetJob()))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("[Transmutation] You cannot equip the transmuted item as it does not match your class."));
			return false;
		}

	#if defined(__COSTUME_SYSTEM__)
		if (pItemTable && pItemTable->IsCostume())
		{
			if ((IS_SET(pItemTable->GetAntiFlags(), ITEM_ANTIFLAG_MALE) && SEX_MALE == GET_SEX(this)) ||
				(IS_SET(pItemTable->GetAntiFlags(), ITEM_ANTIFLAG_FEMALE) && SEX_FEMALE == GET_SEX(this)))
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("[Transmutation] You cannot equip the transmuted item as it does not match your gender."));
				return false;
			}
		}
	}
	#endif
#endif

	if (item->IsRideItem())
	{
		if (IsRiding())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You're already riding. Get off first."));
			return false;
		}

		if (IsPolymorphed())
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot ride a Horse while you are transformed."));
			return false;
		}

#if defined(__MOUNT_COSTUME_SYSTEM__)
		if (item->IsRideItem())
		{
			DWORD dwMountVnum = item->GetMountVnum();
			if (dwMountVnum > 0 && FindAffect(AFFECT_MOUNT_FALL) == NULL)
				MountVnum(dwMountVnum);
		}
#endif
	}

	DWORD dwCurTime = get_dword_time();

#if defined(__MOUNT_COSTUME_SYSTEM__)
	if (iWearCell != WEAR_ARROW && iWearCell != WEAR_COSTUME_MOUNT
		&& (dwCurTime - GetLastAttackTime() <= 1500 || dwCurTime - m_dwLastSkillTime <= 1500))
#else
	if (iWearCell != WEAR_ARROW
		&& (dwCurTime - GetLastAttackTime() <= 1500 || dwCurTime - m_dwLastSkillTime <= 1500))
#endif
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You have to stand still to equip the item."));
		return false;
	}

#if defined(__WEAPON_COSTUME_SYSTEM__)
	if (iWearCell == WEAR_WEAPON)
	{
		if (item->GetType() == ITEM_WEAPON)
		{
			LPITEM pkCostumeWeapon = GetWear(WEAR_COSTUME_WEAPON);
			if (pkCostumeWeapon && pkCostumeWeapon->GetValue(3) != item->GetSubType())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("If you want to change weapons, you must remove the weapon skin first."));
				return false;
			}
		}
		else
		{
			LPITEM pkCostumeWeapon = GetWear(WEAR_COSTUME_WEAPON);
			if (pkCostumeWeapon)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("If you want to change weapons, you must remove the weapon skin first."));
				return false;
			}
		}
	}
	else if (iWearCell == WEAR_COSTUME_WEAPON)
	{
		if (item->GetType() == ITEM_COSTUME && item->GetSubType() == COSTUME_WEAPON)
		{
			LPITEM pkWeapon = GetWear(WEAR_WEAPON);
			if (!pkWeapon)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("You need to equip a weapon first."));
				return false;
			}
			else if (pkWeapon->GetType() != ITEM_WEAPON || item->GetValue(3) != pkWeapon->GetSubType())
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this costume for this weapon."));
				return false;
			}
		}
	}
#endif

#if defined(__DRAGON_SOUL_SYSTEM__)
	// ????? ??? ?
	if (item->IsDragonSoul())
	{
		// It cannot be worn if the same type of dragon spirit stone is already in it.
		// Dragon Soul Stone should not support swap.
		if (GetEquipmentItem(iWearCell))
		{
			ChatPacket(CHAT_TYPE_INFO, "You are already carrying a Dragon Stone of this type.");
			return false;
		}

		if (!item->EquipTo(this, iWearCell))
		{
			return false;
		}
	}
	else
#endif
	{
		if (GetWear(iWearCell) && !IS_SET(GetWear(iWearCell)->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		{
			//if (item->GetWearFlag() == WEARABLE_ABILITY)
			//	return false;

			if (!SwapItem(item->GetCell(), iWearCell))
				return false;
		}
		else
		{
			BYTE bOldCell = item->GetCell();

			if (item->EquipTo(this, iWearCell))
				SyncQuickslot(SLOT_TYPE_INVENTORY, bOldCell, iWearCell);
		}
	}

	if (true == item->IsEquipped())
	{
		if (-1 != item->GetProto()->cLimitRealTimeFirstUseIndex)
		{
			if (0 == item->GetSocket(1))
			{
				long duration = (0 != item->GetSocket(0)) ? item->GetSocket(0) : item->GetProto()->aLimits[(BYTE)item->GetProto()->cLimitRealTimeFirstUseIndex].lValue;

				if (0 == duration)
					duration = 60 * 60 * 24 * 7;

				item->SetSocket(0, time(0) + duration);
				item->StartRealTimeExpireEvent();
			}

			item->SetSocket(1, item->GetSocket(1) + 1);
		}

		if (item->GetVnum() == UNIQUE_ITEM_HIDE_ALIGNMENT_TITLE)
			ShowAlignment(false);

		if (item->IsRing() || item->IsCostume()
#if defined(__MOUNT_COSTUME_SYSTEM__)
			&& !item->IsCostumeMount()
#endif
			)
			ChatPacket(CHAT_TYPE_COMMAND, "OpenCostumeWindow");

		const DWORD& dwVnum = item->GetVnum();

		// (71135)
		if (true == CItemVnumHelper::IsRamadanMoonRing(dwVnum))
		{
			this->EffectPacket(SE_EQUIP_RAMADAN_RING);
		}
		// (71136)
		else if (true == CItemVnumHelper::IsHalloweenCandy(dwVnum))
		{
			this->EffectPacket(SE_EQUIP_HALLOWEEN_CANDY);
		}
		// (71143)
		else if (true == CItemVnumHelper::IsHappinessRing(dwVnum))
		{
			this->EffectPacket(SE_EQUIP_HAPPINESS_RING);
		}
		// (71145)
		else if (true == CItemVnumHelper::IsLovePendant(dwVnum))
		{
			this->EffectPacket(SE_EQUIP_LOVE_PENDANT);
		}
#if defined(ENABLE_PASSIVE_SYSTEM)
		// (30272) - (30273) - (30274) - (30275) - (30276) 
		else if (true == CItemVnumHelper::IsPassive(dwVnum))
		{
			this->EffectPacket(SE_PASSIVE_EFFECT);
			this->SpecificEffectPacket("d:/ymir work/effect/etc/buff/buff_passive_01.mse");
		}
#endif
		// ITEM_UNIQUE, SpecialItemGroup, (item->GetSIGVnum() != NULL)
		else if ((ITEM_UNIQUE == item->GetType() || ITEM_RING == item->GetType()) && 0 != item->GetSIGVnum())
		{
			const CSpecialItemGroup* pGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(item->GetSIGVnum());
			if (NULL != pGroup)
			{
				const CSpecialAttrGroup* pAttrGroup = ITEM_MANAGER::instance().GetSpecialAttrGroup(pGroup->GetAttrVnum(item->GetVnum()));
				if (NULL != pAttrGroup)
				{
					const std::string& std = pAttrGroup->m_stEffectFileName;
					SpecificEffectPacket(std.c_str());
				}
			}
		}

#if defined(__ACCE_COSTUME_SYSTEM__)
		if ((item->GetType() == ITEM_COSTUME) && (item->GetSubType() == COSTUME_ACCE
#if defined(__AURA_COSTUME_SYSTEM__)
			|| item->GetSubType() == COSTUME_AURA
#endif
			))
		{
			if (item->GetSocket(ITEM_SOCKET_ACCE_DRAIN_VALUE) > 18)
				this->EffectPacket(SE_ACCE_BACK);
			this->EffectPacket(SE_ACCE_EQUIP);
		}
#endif

		if ((ITEM_UNIQUE == item->GetType() && UNIQUE_SPECIAL_RIDE == item->GetSubType() && IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_USE))
#if defined(__MOUNT_COSTUME_SYSTEM__)
			|| (ITEM_COSTUME == item->GetType() && COSTUME_MOUNT == item->GetSubType())
#endif
			)
		{
			quest::CQuestManager::instance().UseItem(GetPlayerID(), item, false);
		}
	}

	return true;
}

void CHARACTER::BuffOnAttr_AddBuffsFromItem(LPITEM pItem)
{
	for (int i = 0; i < sizeof(g_aBuffOnAttrPoints) / sizeof(g_aBuffOnAttrPoints[0]); i++)
	{
		TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(g_aBuffOnAttrPoints[i]);
		if (it != m_map_buff_on_attrs.end())
		{
			it->second->AddBuffFromItem(pItem);
		}
	}
}

void CHARACTER::BuffOnAttr_RemoveBuffsFromItem(LPITEM pItem)
{
	for (int i = 0; i < sizeof(g_aBuffOnAttrPoints) / sizeof(g_aBuffOnAttrPoints[0]); i++)
	{
		TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(g_aBuffOnAttrPoints[i]);
		if (it != m_map_buff_on_attrs.end())
		{
			it->second->RemoveBuffFromItem(pItem);
		}
	}
}

void CHARACTER::BuffOnAttr_ClearAll()
{
	for (TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.begin(); it != m_map_buff_on_attrs.end(); ++it)
	{
		CBuffOnAttributes* pBuff = it->second;
		if (pBuff)
		{
			pBuff->Initialize();
		}
	}
}

void CHARACTER::BuffOnAttr_ValueChange(POINT_TYPE wPointType, POINT_VALUE llOldValue, POINT_VALUE lNewValue)
{
	TMapBuffOnAttrs::iterator it = m_map_buff_on_attrs.find(wPointType);

	if (0 == lNewValue)
	{
		if (m_map_buff_on_attrs.end() == it)
			return;
		else
			it->second->Off();
	}
	else if (0 == llOldValue)
	{
		CBuffOnAttributes* pBuff;
		if (m_map_buff_on_attrs.end() == it)
		{
			switch (wPointType)
			{
				case POINT_ENERGY:
				{
					static BYTE s_abSlot[] = {
						WEAR_BODY,
						WEAR_HEAD,
						WEAR_FOOTS,
						WEAR_WRIST,
						WEAR_WEAPON,
						WEAR_NECK,
						WEAR_EAR,
						WEAR_SHIELD,
#if defined(__PENDANT_SYSTEM__)
						WEAR_PENDANT,
#endif
#if defined(__GLOVE_SYSTEM__)
						WEAR_GLOVE,
#endif
					};
					static std::vector<POINT_TYPE> s_vSlots(s_abSlot, s_abSlot + _countof(s_abSlot));
					pBuff = M2_NEW CBuffOnAttributes(this, wPointType, &s_vSlots);
				}
				break;

				case POINT_COSTUME_ATTR_BONUS:
				{
					static BYTE s_abSlot[] = {
						WEAR_COSTUME_BODY,
						WEAR_COSTUME_HAIR,
#if defined(__WEAPON_COSTUME_SYSTEM__)
						WEAR_COSTUME_WEAPON,
#endif
					};
					static std::vector<POINT_TYPE> s_vSlots(s_abSlot, s_abSlot + _countof(s_abSlot));
					pBuff = M2_NEW CBuffOnAttributes(this, wPointType, &s_vSlots);
				}
				break;

				default:
					break;

			}
			m_map_buff_on_attrs.insert(TMapBuffOnAttrs::value_type(wPointType, pBuff));

		}
		else
			pBuff = it->second;

		pBuff->On(lNewValue);
	}
	else
	{
		if (m_map_buff_on_attrs.end() == it)
			return;
		else
			it->second->ChangeBuffValue(lNewValue);
	}
}

LPITEM CHARACTER::FindSpecifyItem(DWORD dwVnum
#if defined(__SOUL_BIND_SYSTEM__)
	, bool bIgnoreSoulBound
#endif
#if defined(__SET_ITEM__)
	, bool bIgnoreSetValue
#endif
) const
{
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		if (GetInventoryItem(i) && GetInventoryItem(i)->GetVnum() == dwVnum)
		{
#if defined(__SOUL_BIND_SYSTEM__)
			if (bIgnoreSoulBound && GetInventoryItem(i)->IsSealed())
				continue;
#endif

#if defined(__SET_ITEM__)
			if (bIgnoreSetValue && GetInventoryItem(i)->GetItemSetValue())
				continue;
#endif
			return GetInventoryItem(i);
		}
	}
	return NULL;
}

LPITEM CHARACTER::FindItemByID(DWORD id) const
{
#if defined(__EXTEND_INVEN_SYSTEM__)
	for (WORD wCell = 0; wCell < GetExtendInvenMax(); ++wCell)
#else
	for (WORD wCell = 0; wCell < INVENTORY_MAX_NUM; ++wCell)
#endif
		if (GetInventoryItem(wCell) && GetInventoryItem(wCell)->GetID() == id)
			return GetInventoryItem(wCell);

#if defined(__DRAGON_SOUL_SYSTEM__)
	for (WORD wCell = 0; wCell < DRAGON_SOUL_INVENTORY_MAX_NUM; ++wCell)
		if (GetDragonSoulInventoryItem(wCell) && GetDragonSoulInventoryItem(wCell)->GetID() == id)
			return GetDragonSoulInventoryItem(wCell);
#endif

	for (WORD wCell = 0; wCell < BELT_INVENTORY_MAX_NUM; ++wCell)
		if (GetBeltInventoryItem(wCell) && GetBeltInventoryItem(wCell)->GetID() == id)
			return GetBeltInventoryItem(wCell);

	return NULL;
}

int CHARACTER::CountSpecifyItem(DWORD vnum, int iExceptionCell
#if defined(__SOUL_BIND_SYSTEM__)
	, bool bIgnoreSealedItem
#endif
#if defined(__SET_ITEM__)
	, bool bIgnoreSetValue
#endif
) const
{
	int iCount = 0;
	LPITEM pItem = nullptr;

#if defined(__EXTEND_INVEN_SYSTEM__)
	for (WORD wCell = 0; wCell < GetExtendInvenMax(); ++wCell)
#else
	for (WORD wCell = 0; wCell < INVENTORY_MAX_NUM; ++wCell)
#endif
	{
		if (wCell == iExceptionCell)
			continue;

		pItem = GetInventoryItem(wCell);
		if (pItem && pItem->GetVnum() == vnum)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(pItem->GetID()))
				continue;

#if defined(__SOUL_BIND_SYSTEM__)
			if (bIgnoreSealedItem && pItem->IsSealed())
				continue;
#endif

#if defined(__SET_ITEM__)
			if (bIgnoreSetValue && pItem->GetItemSetValue())
				continue;
#endif

			iCount += pItem->GetCount();
		}
	}

#if defined(__DRAGON_SOUL_SYSTEM__)
	for (WORD wCell = 0; wCell < DRAGON_SOUL_INVENTORY_MAX_NUM; ++wCell)
	{
		pItem = GetDragonSoulInventoryItem(wCell);
		if (pItem && pItem->GetVnum() == vnum)
		{
#if defined(__SOUL_BIND_SYSTEM__)
			if (bIgnoreSealedItem && pItem->IsSealed())
				continue;
#endif

			iCount += pItem->GetCount();
		}
	}
#endif

	return iCount;
}

#if defined(ENABLE_MONSTER_CARD)
int CHARACTER::CountSpecifyItemBySocket(DWORD vnum, int socketIndex, int socketValue, int iExceptionCell) const
{
	int iCount = 0;
	LPITEM pItem = nullptr;

#if defined(__EXTEND_INVEN_SYSTEM__)
	for (WORD wCell = 0; wCell < GetExtendInvenMax(); ++wCell)
#else
	for (WORD wCell = 0; wCell < INVENTORY_MAX_NUM; ++wCell)
#endif
	{
		if (wCell == iExceptionCell)
			continue;

		pItem = GetInventoryItem(wCell);
		if (pItem && pItem->GetVnum() == vnum)
		{
			if (socketIndex >= 0 && pItem->GetSocket(socketIndex) != socketValue)
				continue;

			if (m_pkMyShop && m_pkMyShop->IsSellingItem(pItem->GetID()))
				continue;

			iCount += pItem->GetCount();
		}
	}

	return iCount;
}

void CHARACTER::RemoveSpecifyItemBySocket(DWORD vnum, DWORD count, int socketIndex, int socketValue, int iExceptionCell)
{
	if (0 == count)
		return;

#if defined(__EXTEND_INVEN_SYSTEM__)
	for (UINT i = 0; i < GetExtendInvenMax(); ++i)
#else
	for (UINT i = 0; i < INVENTORY_MAX_NUM; ++i)
#endif
	{
		if (static_cast<int>(i) == iExceptionCell)
			continue;

		LPITEM curItem = GetInventoryItem(i);
		if (curItem == nullptr)
			continue;

		if (curItem->GetVnum() != vnum)
			continue;

		if (socketIndex >= 0 && curItem->GetSocket(socketIndex) != socketValue)
			continue;

		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(curItem->GetID());
			if (isItemSelling)
				continue;
		}

		if (count >= curItem->GetCount())
		{
			count -= curItem->GetCount();
			curItem->SetCount(0);
			if (0 == count)
				return;
		}
		else
		{
			curItem->SetCount(curItem->GetCount() - count);
			return;
		}
	}

	if (count)
		sys_log(0, "CHARACTER::RemoveSpecifyItemBySocket cannot remove enough item vnum %u, still remain %u", vnum, count);
}
#endif

void CHARACTER::RemoveSpecifyItem(DWORD vnum, DWORD count, int iExceptionCell
#if defined(__SOUL_BIND_SYSTEM__)
	, bool bIgnoreSealedItem
#endif
#if defined(__SET_ITEM__)
	, bool bIgnoreSetValue
#endif
)
{
	if (0 == count)
		return;

#if defined(__EXTEND_INVEN_SYSTEM__)
	for (UINT i = 0; i < GetExtendInvenMax(); ++i)
#else
	for (UINT i = 0; i < INVENTORY_MAX_NUM; ++i)
#endif
	{
		if (i == iExceptionCell)
			continue;

		if (NULL == GetInventoryItem(i))
			continue;

		if (GetInventoryItem(i)->GetVnum() != vnum)
			continue;

#if defined(__SOUL_BIND_SYSTEM__)
		if (bIgnoreSealedItem && GetInventoryItem(i)->IsSealed())
			continue;
#endif

#if defined(__SET_ITEM__)
		if (bIgnoreSetValue && GetInventoryItem(i)->GetItemSetValue())
			continue;
#endif

		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (vnum >= 80003 && vnum <= 80007)
			LogManager::instance().GoldBarLog(GetPlayerID(), GetInventoryItem(i)->GetID(), QUEST, "RemoveSpecifyItem");

		if (count >= GetInventoryItem(i)->GetCount())
		{
			count -= GetInventoryItem(i)->GetCount();
			GetInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}

#if defined(__DRAGON_SOUL_SYSTEM__)
	for (UINT i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if (NULL == GetDragonSoulInventoryItem(i))
			continue;

		if (GetDragonSoulInventoryItem(i)->GetVnum() != vnum)
			continue;

#if defined(__SOUL_BIND_SYSTEM__)
		if (bIgnoreSealedItem && GetDragonSoulInventoryItem(i)->IsSealed())
			continue;
#endif

#if defined(__SET_ITEM__)
		if (bIgnoreSetValue && GetDragonSoulInventoryItem(i)->GetItemSetValue())
			continue;
#endif

		if (count >= GetDragonSoulInventoryItem(i)->GetCount())
		{
			count -= GetDragonSoulInventoryItem(i)->GetCount();
			GetDragonSoulInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetDragonSoulInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}
#endif

	if (count)
		sys_log(0, "CHARACTER::RemoveSpecifyItem cannot remove enough item vnum %u, still remain %d", vnum, count);
}

#if defined(__GROWTH_PET_SYSTEM__)
int CHARACTER::CountSpecifyPetFeedItem(uint32_t vnum, int iExceptionCell) const
{
	int	count = 0;
	LPITEM item;

	for (int i = 0; i < PET_MAX_FEED_SLOT; ++i)
	{
		if (i == iExceptionCell)
			continue;

		item = GetInventoryFeedItem(i);
		if (nullptr != item && item->GetVnum() == vnum)
		{
			if (m_pkMyShop && m_pkMyShop->IsSellingItem(item->GetID()))
			{
				continue;
			}
			else
			{
				count += item->GetCount();
			}
		}
	}

	return count;
}

void CHARACTER::RemoveSpecifyPetFeedItem(uint32_t vnum, uint32_t count, int iExceptionCell)
{
	if (0 == count)
		return;

	for (uint32_t i = 0; i < PET_MAX_FEED_SLOT; ++i)
	{
		if (i == iExceptionCell)
			continue;

		if (nullptr == GetInventoryFeedItem(i))
			continue;

		if (GetInventoryFeedItem(i)->GetVnum() != vnum)
			continue;

		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryFeedItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (count >= GetInventoryFeedItem(i)->GetCount())
		{
			count -= GetInventoryFeedItem(i)->GetCount();
			GetInventoryFeedItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetInventoryFeedItem(i)->SetCount(GetInventoryFeedItem(i)->GetCount() - count);
			return;
		}
	}

	if (count)
		sys_log(0, "CHARACTER::RemoveSpecifyPetFeedItem cannot remove enough item vnum %u, still remain %d", vnum, count);
}
#endif

int CHARACTER::CountSpecifyTypeItem(BYTE type) const
{
	int count = 0;

#if defined(__EXTEND_INVEN_SYSTEM__)
	for (int i = 0; i < GetExtendInvenMax(); ++i)
#else
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
#endif
	{
		LPITEM pItem = GetInventoryItem(i);
		if (pItem != NULL && pItem->GetType() == type)
		{
			count += pItem->GetCount();
		}
	}

	return count;
}

void CHARACTER::RemoveSpecifyTypeItem(BYTE type, DWORD count)
{
	if (0 == count)
		return;

#if defined(__EXTEND_INVEN_SYSTEM__)
	for (UINT i = 0; i < GetExtendInvenMax(); ++i)
#else
	for (UINT i = 0; i < INVENTORY_MAX_NUM; ++i)
#endif
	{
		if (NULL == GetInventoryItem(i))
			continue;

		if (GetInventoryItem(i)->GetType() != type)
			continue;

		if (m_pkMyShop)
		{
			bool isItemSelling = m_pkMyShop->IsSellingItem(GetInventoryItem(i)->GetID());
			if (isItemSelling)
				continue;
		}

		if (count >= GetInventoryItem(i)->GetCount())
		{
			count -= GetInventoryItem(i)->GetCount();
			GetInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}

#if defined(__DRAGON_SOUL_SYSTEM__)
	for (UINT i = 0; i < DRAGON_SOUL_INVENTORY_MAX_NUM; ++i)
	{
		if (NULL == GetDragonSoulInventoryItem(i))
			continue;

		if (GetDragonSoulInventoryItem(i)->GetType() != type)
			continue;

		if (count >= GetDragonSoulInventoryItem(i)->GetCount())
		{
			count -= GetDragonSoulInventoryItem(i)->GetCount();
			GetDragonSoulInventoryItem(i)->SetCount(0);

			if (0 == count)
				return;
		}
		else
		{
			GetDragonSoulInventoryItem(i)->SetCount(GetInventoryItem(i)->GetCount() - count);
			return;
		}
	}
#endif
}

// Fix book stacking while sorting.
void CHARACTER::GiveSkillBook(DWORD dwSkillVnum, WORD wCount)
{
	for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
	{
		LPITEM pkItem = GetInventoryItem(i);

		if (!pkItem)
			continue;

		if ((pkItem->GetType() == ITEM_SKILLBOOK || pkItem->GetType() == ITEM_SKILLFORGET) && pkItem->GetSocket(0) == dwSkillVnum)
		{
			WORD wCount2 = MIN(ITEM_MAX_COUNT - pkItem->GetCount(), wCount);
			wCount -= wCount2;

			pkItem->SetCount(pkItem->GetCount() + wCount2);
			if (wCount == 0)
				return;
		}
	}

	LPITEM pkBookItem = AutoGiveItem(ITEM_SKILLBOOK_VNUM, wCount, false, false);
	if (NULL != pkBookItem)
		pkBookItem->SetSocket(0, dwSkillVnum);
}
//>

LPITEM CHARACTER::AutoGiveItem(DWORD dwItemVnum, WORD wCount, int iRarePct, bool bMsg
#if defined(__WJ_PICKUP_ITEM_EFFECT__)
	, bool isHighLight
#endif
#if defined(__NEW_USER_CARE__)
	, bool bSystemDrop
#endif
)
{
	TItemTable* p = ITEM_MANAGER::instance().GetTable(dwItemVnum);
	if (!p)
		return NULL;

	DBManager::instance().SendMoneyLog(MONEY_LOG_DROP, dwItemVnum, wCount);

	if (p->dwFlags & ITEM_FLAG_STACKABLE/* && p->bType != ITEM_BLEND*/)
	{
		for (int i = 0; i < INVENTORY_MAX_NUM; ++i)
		{
			LPITEM item = GetInventoryItem(i);

			if (!item)
				continue;

			if (item->GetVnum() == dwItemVnum && FN_check_item_socket(item))
			{
				if (IS_SET(p->dwFlags, ITEM_FLAG_MAKECOUNT))
				{
					if (wCount < p->alValues[1])
						wCount = p->alValues[1];
				}

				WORD wCount2 = MIN(ITEM_MAX_COUNT - item->GetCount(), wCount);
				wCount -= wCount2;

				item->SetCount(item->GetCount() + wCount2);

				if (wCount == 0)
				{
					if (bMsg)
					{
#if defined(__CHATTING_WINDOW_RENEWAL__)
    #if defined(__SET_ITEM__)
						ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
    #else
						ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_ITEM(item->GetVnum())));
    #endif
#else
    #if defined(__SET_ITEM__)
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
    #else
						ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_ITEM(item->GetVnum())));
    #endif
#endif
					}

					return item;
				}
			}
		}
	}

	LPITEM item = ITEM_MANAGER::instance().CreateItem(dwItemVnum, wCount, 0, true);

	if (!item)
	{
		sys_err("cannot create item by vnum %u (name: %s)", dwItemVnum, GetName());
		return NULL;
	}

	if (item->GetType() == ITEM_BLEND)
	{
		for (int i = 0; i < INVENTORY_MAX_NUM; i++)
		{
			LPITEM inv_item = GetInventoryItem(i);

			if (inv_item == NULL)
				continue;

			if (inv_item->GetType() == ITEM_BLEND)
			{
				if (inv_item->GetVnum() == item->GetVnum())
				{
					if (inv_item->GetSocket(0) == item->GetSocket(0) &&
						inv_item->GetSocket(1) == item->GetSocket(1) &&
						inv_item->GetSocket(2) == item->GetSocket(2) &&
						inv_item->GetCount() + item->GetCount() <= ITEM_MAX_COUNT)
					{
						inv_item->SetCount(inv_item->GetCount() + item->GetCount());
						M2_DESTROY_ITEM(item);
						return inv_item;
					}
				}
			}
		}
	}

	int iEmptyCell;
#if defined(__DRAGON_SOUL_SYSTEM__)
	if (item->IsDragonSoul())
		iEmptyCell = GetEmptyDragonSoulInventory(item);
	else
#endif
		iEmptyCell = GetEmptyInventory(item->GetSize());

	if (iEmptyCell != -1)
	{
#if defined(__DRAGON_SOUL_SYSTEM__)
		if (item->IsDragonSoul())
			item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, iEmptyCell));
		else
#endif
#if defined(__WJ_PICKUP_ITEM_EFFECT__)
			item->AddToCharacter(this, TItemPos(INVENTORY, iEmptyCell), isHighLight);
#else
			item->AddToCharacter(this, TItemPos(INVENTORY, iEmptyCell));
#endif

		if (bMsg)
#if defined(__CHATTING_WINDOW_RENEWAL__)
#if defined(__SET_ITEM__)
			ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
#else
			ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_ITEM(item->GetVnum())));
#endif
#else
#if defined(__SET_ITEM__)
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
#else
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_ITEM(item->GetVnum())));
#endif
#endif

		LogManager::instance().ItemLog(this, item, "SYSTEM", item->GetName());

		if (item->GetType() == ITEM_USE && item->GetSubType() == USE_POTION)
		{
			TQuickslot* pSlot;

			if (GetQuickslot(0, &pSlot) && pSlot->type == SLOT_TYPE_NONE)
			{
				TQuickslot slot;
				slot.type = SLOT_TYPE_INVENTORY;
				slot.pos = iEmptyCell;
				SetQuickslot(0, slot);
			}
		}
	}
	else
	{
#if defined(__NEW_USER_CARE__)
		if (!bSystemDrop)
		{
			sys_log(0, "AutoGiveItem: No inventory space, SystemDrop disabled!");

			M2_DESTROY_ITEM(item);
			return NULL;
		}
#endif

		item->AddToGround(GetMapIndex(), GetXYZ());
		item->StartDestroyEvent();
		if (IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_DROP))
			item->SetOwnership(this, 300);
		else
			item->SetOwnership(this, 60);

		LogManager::instance().ItemLog(this, item, "SYSTEM_DROP", item->GetName());
	}

	sys_log(0, "7: %d %d", dwItemVnum, wCount);
	return item;
}

#define __AUTO_GIVE_ITEM_STACK__
void CHARACTER::AutoGiveItem(LPITEM item, bool longOwnerShip, bool bMsg
#if defined(__WJ_PICKUP_ITEM_EFFECT__)
	, bool isHighLight
#endif
)
{
	if (NULL == item)
	{
		sys_err("NULL point.");
		return;
	}

	if (item->GetOwner())
	{
		sys_err("item %d 's owner exists!", item->GetID());
		return;
	}

#if defined(__AUTO_GIVE_ITEM_STACK__)
	if (item->IsStackable() && !IS_SET(item->GetAntiFlag(), ITEM_ANTIFLAG_STACK))
	{
		for (WORD i = 0; i < INVENTORY_MAX_NUM; ++i)
		{
			LPITEM item2 = GetInventoryItem(i);
			if (!item2)
				continue;

			if (item2->GetVnum() != item->GetVnum())
				continue;

			BYTE j = 0;
			for (; j < ITEM_SOCKET_MAX_NUM; ++j)
				if (item2->GetSocket(j) != item->GetSocket(j))
					break;

			if (j != ITEM_SOCKET_MAX_NUM)
				continue;

			if (item2->GetCount() >= ITEM_MAX_COUNT)
				continue;

			WORD wCount = item->GetCount();
			if (IS_SET(item->GetFlag(), ITEM_FLAG_MAKECOUNT))
			{
				if (wCount < item->GetValue(1))
					wCount = item->GetValue(1);
			}

			WORD wCount2 = MIN(ITEM_MAX_COUNT - item2->GetCount(), wCount);
			wCount -= wCount2;
			item2->SetCount(item2->GetCount() + wCount2);

			if (wCount == 0)
			{
				if (bMsg)
#if defined(__CHATTING_WINDOW_RENEWAL__)
#if defined(__SET_ITEM__)
					ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
#else
					ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_ITEM(item->GetVnum())));
#endif
#else
#if defined(__SET_ITEM__)
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
#else
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_ITEM(item->GetVnum())));
#endif
#endif

				M2_DESTROY_ITEM(item);

				return;
			}

			item->SetCount(wCount);
		}
	}
#endif

	int cell;
#if defined(__DRAGON_SOUL_SYSTEM__)
	if (item->IsDragonSoul())
	{
		cell = GetEmptyDragonSoulInventory(item);
	}
	else
#endif
	{
		cell = GetEmptyInventory(item->GetSize());
	}

	if (cell != -1)
	{
#if defined(__DRAGON_SOUL_SYSTEM__)
		if (item->IsDragonSoul())
			item->AddToCharacter(this, TItemPos(DRAGON_SOUL_INVENTORY, cell));
		else
#endif
#if defined(__WJ_PICKUP_ITEM_EFFECT__)
			item->AddToCharacter(this, TItemPos(INVENTORY, cell), isHighLight);
#else
			item->AddToCharacter(this, TItemPos(INVENTORY, cell));
#endif

		if (bMsg)
#if defined(__CHATTING_WINDOW_RENEWAL__)
#if defined(__SET_ITEM__)
			ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
#else
			ChatPacket(CHAT_TYPE_ITEM_INFO, LC_STRING("%s received", LC_ITEM(item->GetVnum())));
#endif
#else
#if defined(__SET_ITEM__)
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_PRE_ITEM(item->GetItemSetValue(), item->GetVnum())));
#else
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("%s received", LC_ITEM(item->GetVnum())));
#endif
#endif

		LogManager::instance().ItemLog(this, item, "SYSTEM", item->GetName());

		if (item->GetType() == ITEM_USE && item->GetSubType() == USE_POTION)
		{
			TQuickslot* pSlot;

			if (GetQuickslot(0, &pSlot) && pSlot->type == SLOT_TYPE_NONE)
			{
				TQuickslot slot;
				slot.type = SLOT_TYPE_INVENTORY;
				slot.pos = cell;
				SetQuickslot(0, slot);
			}
		}
	}
	else
	{
		item->AddToGround(GetMapIndex(), GetXYZ());
		item->StartDestroyEvent();

		if (longOwnerShip)
			item->SetOwnership(this, 300);
		else
			item->SetOwnership(this, 60);

		LogManager::instance().ItemLog(this, item, "SYSTEM_DROP", item->GetName());
	}
}

bool CHARACTER::GiveItem(LPCHARACTER victim, TItemPos Cell)
{
	if (!CanHandleItem())
		return false;

	if (IsRunningQuest())
		return false;

	LPITEM item = GetItem(Cell);
	if (item && !item->IsExchanging())
	{
		if (victim->CanReceiveItem(this, item))
		{
			victim->ReceiveItem(this, item);
			return true;
		}
	}

	return false;
}

bool CHARACTER::CanReceiveItem(LPCHARACTER from, LPITEM item) const
{
	if (IsPC())
		return false;

	// TOO_LONG_DISTANCE_EXCHANGE_BUG_FIX
	if (DISTANCE_APPROX(GetX() - from->GetX(), GetY() - from->GetY()) > 2000)
		return false;
	// END_OF_TOO_LONG_DISTANCE_EXCHANGE_BUG_FIX

#if defined(__SOUL_BIND_SYSTEM__)
	if (item->IsSealed())
	{
		from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot improve a soulbound item."));
		return false;
	}
#endif

	switch (GetRaceNum())
	{
        case fishing::CAMPFIRE_MOB:
		{
			if (item->GetType() == ITEM_FISH &&
				(item->GetSubType() == FISH_ALIVE || item->GetSubType() == FISH_DEAD))
				return true;
		}
		break;

		case fishing::FISHER_MOB:
		{
			if (item->GetType() == ITEM_ROD)
				return true;
		}
		break;

		// BUILDING_NPC
		case BLACKSMITH_WEAPON_MOB:
		case DEVILTOWER_BLACKSMITH_WEAPON_MOB:
		{
			if (item->GetType() == ITEM_WEAPON &&
				item->GetRefinedVnum())
				return true;
			else
				return false;
		}
		break;

        case BLACKSMITH_ARMOR_MOB:
        case DEVILTOWER_BLACKSMITH_ARMOR_MOB:
		{
			if (item->GetType() == ITEM_ARMOR &&
				(item->GetSubType() == ARMOR_BODY || item->GetSubType() == ARMOR_SHIELD || item->GetSubType() == ARMOR_HEAD) &&
				item->GetRefinedVnum())
				return true;
			else
				return false;
		}
		break;

		case BLACKSMITH_ACCESSORY_MOB:
		case DEVILTOWER_BLACKSMITH_ACCESSORY_MOB:
		{
			if (item->GetType() == ITEM_ARMOR &&
				!(item->GetSubType() == ARMOR_BODY || item->GetSubType() == ARMOR_SHIELD || item->GetSubType() == ARMOR_HEAD) &&
				item->GetRefinedVnum())
				return true;
			else
				return false;
		}
		break;
		// END_OF_BUILDING_NPC

		case BLACKSMITH_MOB:
		{
			if (item->GetRefinedVnum() && item->GetRefineSet() < 500)
				return true;
			else
				return false;
		}
		break;

		case BLACKSMITH2_MOB:
		{
			if (item->GetRefineSet() >= 500)
				return true;
			else
				return false;
		}
		break;

		case ALCHEMIST_MOB:
		{
			if (item->GetRefinedVnum())
				return true;
		}
		break;

		case 20101:
		case 20102:
		case 20103:
		{
			if (item->GetVnum() == ITEM_REVIVE_HORSE_1)
			{
				if (!IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("Monkey Herbs cannot be fed to living horses. It is used to revive dead horses."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1)
			{
				if (IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot feed a dead Horse."));
					return false;
				}

	    		// Prevent Feeding The Horse When It Is Already Full
	    		unsigned char horselevel = from->GetHorseLevel();
	    		if (from->GetHorseHealth() >= c_aHorseStat[horselevel].iMaxHealth)
	    		{
	    			from->ChatPacket (CHAT_TYPE_INFO, LC_STRING("Your horse's health has reached the maximum, you cannot feed it any more.."));
	    			return false;
	    		}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_2 || item->GetVnum() == ITEM_HORSE_FOOD_3)
			{
				return false;
			}
		}
		break;

		case 20104:
		case 20105:
		case 20106:
		{
			if (item->GetVnum() == ITEM_REVIVE_HORSE_2)
			{
				if (!IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("Monkey Herbs cannot be fed to living horses. It is used to revive dead horses."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_2)
			{
				if (IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot feed a dead Horse."));
					return false;
				}

				// Prevent Feeding The Horse When It Is Already Full
				unsigned char horselevel = from->GetHorseLevel();
				if (from->GetHorseHealth() >= c_aHorseStat[horselevel].iMaxHealth)
				{
					from->ChatPacket (CHAT_TYPE_INFO, LC_STRING("Your horse's health has reached the maximum, you cannot feed it any more.."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1 || item->GetVnum() == ITEM_HORSE_FOOD_3)
			{
				return false;
			}
		}
		break;

		case 20107:
		case 20108:
		case 20109:
		{
			if (item->GetVnum() == ITEM_REVIVE_HORSE_3)
			{
				if (!IsDead())
				{
                    from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("Monkey Herbs cannot be fed to living horses. It is used to revive dead horses."));
					return false;
				}
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_3)
			{
				if (IsDead())
				{
					from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot feed a dead Horse."));
					return false;
				}

                // Prevent Feeding The Horse When It Is Already Full
                unsigned char horselevel = from->GetHorseLevel();
                if (from->GetHorseHealth() >= c_aHorseStat[horselevel].iMaxHealth)
                {
                    from->ChatPacket (CHAT_TYPE_INFO, LC_STRING("Your horse's health has reached the maximum, you cannot feed it any more.."));
                    return false;
                }
				return true;
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1 || item->GetVnum() == ITEM_HORSE_FOOD_2)
			{
				return false;
			}
		}
		break;

		default:
			break;
#if defined(__MOUNT_UPGRADE__)
    	case 20149:
    	case 20150:
    	case 20151:
		{
    		if (item->GetVnum() == ITEM_REVIVE_HORSE_4)
    		{
    			if (!IsDead())
    			{
    				from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("Monkey Herbs cannot be fed to living horses. It is used to revive dead horses."));
    				return false;
    			}
    			return true;
    		}
    		else if (item->GetVnum() == ITEM_HORSE_FOOD_4)
    		{
    			if (IsDead())
    			{
    				from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot feed a dead Horse."));
    				return false;
    			}
    
    			// Prevent Feeding The Horse When It Is Already Full
    			unsigned char horselevel = from->GetHorseLevel();
    			if (from->GetHorseHealth() >= c_aHorseStat[horselevel].iMaxHealth)
    			{
    				from->ChatPacket (CHAT_TYPE_INFO, LC_STRING("Your horse's health has reached the maximum, you cannot feed it any more.."));
    				return false;
    			}
    			return true;
			}
		}
		break;
#endif
	}

	//if (IS_SET(item->GetFlag(), ITEM_FLAG_QUEST_GIVE))
	{
		return true;
	}

	return false;
}

void CHARACTER::ReceiveItem(LPCHARACTER from, LPITEM item)
{
	if (IsPC())
		return;

	switch (GetRaceNum())
	{
		case fishing::CAMPFIRE_MOB:
		{
			if (item->GetType() == ITEM_FISH && (item->GetSubType() == FISH_ALIVE || item->GetSubType() == FISH_DEAD))
				fishing::Grill(from, item);
			else
			{
				// TAKE_ITEM_BUG_FIX
				from->SetQuestNPCID(GetVID());
				// END_OF_TAKE_ITEM_BUG_FIX
				quest::CQuestManager::instance().TakeItem(from->GetPlayerID(), GetRaceNum(), item);
			}
		}
		break;

		// DEVILTOWER_NPC
		case DEVILTOWER_BLACKSMITH_WEAPON_MOB:
		case DEVILTOWER_BLACKSMITH_ARMOR_MOB:
		case DEVILTOWER_BLACKSMITH_ACCESSORY_MOB:
		{
			if (item->GetRefinedVnum() != 0 && item->GetRefineSet() != 0 && item->GetRefineSet() < 500)
			{
				from->SetRefineNPC(this);
				from->RefineInformation(item->GetCell(), REFINE_TYPE_MONEY_ONLY);
			}
			else
			{
				from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ???????? ?????? ?? ???????."));
			}
		}
		break;
		// END_OF_DEVILTOWER_NPC

		case BLACKSMITH_MOB:
		case BLACKSMITH2_MOB:
		case BLACKSMITH_WEAPON_MOB:
		case BLACKSMITH_ARMOR_MOB:
		case BLACKSMITH_ACCESSORY_MOB:
		{
			if (item->GetRefinedVnum())
			{
				from->SetRefineNPC(this);
				from->RefineInformation(item->GetCell(), REFINE_TYPE_NORMAL);
			}
			else
			{
				from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("?? ???????? ?????? ?? ???????."));
			}
		}
		break;

		case 20101:
		case 20102:
		case 20103:
		case 20104:
		case 20105:
		case 20106:
		case 20107:
		case 20108:
		case 20109:
		{
			if (item->GetVnum() == ITEM_REVIVE_HORSE_1 ||
				item->GetVnum() == ITEM_REVIVE_HORSE_2 ||
				item->GetVnum() == ITEM_REVIVE_HORSE_3)
			{
				from->ReviveHorse();
				item->SetCount(item->GetCount() - 1);
				from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ????? ????????."));
			}
			else if (item->GetVnum() == ITEM_HORSE_FOOD_1 ||
				item->GetVnum() == ITEM_HORSE_FOOD_2 ||
				item->GetVnum() == ITEM_HORSE_FOOD_3)
			{
				from->FeedHorse();
				from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ??? ????????."));
				item->SetCount(item->GetCount() - 1);
				EffectPacket(SE_HPUP_RED);
			}
		}
		break;

#if defined(__MOUNT_UPGRADE__)
    	case 20149:
    	case 20150:
    	case 20151:
    		if (item->GetVnum() == ITEM_REVIVE_HORSE_4)
    		{
    			from->ReviveHorse();
    			item->SetCount(item->GetCount() - 1);
    			from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("You fed the Horse with Herbs."));
    		}
    		else if (item->GetVnum() == ITEM_HORSE_FOOD_4)
    		{
    			from->FeedHorse();
    			from->ChatPacket(CHAT_TYPE_INFO, LC_STRING("You have fed the Horse."));
    			item->SetCount(item->GetCount() - 1);
    			EffectPacket(SE_HPUP_RED);
            }
            break;
#endif

		default:
		{
			sys_log(0, "TakeItem %s %d %s", from->GetName(), GetRaceNum(), item->GetName());
			from->SetQuestNPCID(GetVID());
			quest::CQuestManager::instance().TakeItem(from->GetPlayerID(), GetRaceNum(), item);
		}
		break;
	}

#if defined(__GUILD_DRAGONLAIR_SYSTEM__)
	if (CGuildDragonLairManager::Instance().IsRedDragonLair(from->GetMapIndex())
		&& CGuildDragonLairManager::Instance().IsStone(GetRaceNum()))
	{
		if (item->GetVnum() == ITEM_GUILD_DRAGONLAIR_RING)
		{
			sys_log(0, "GuildDragonLair TakeItem %s %d %s", from->GetName(), GetRaceNum(), item->GetName());

			if (from->GetGuildDragonLair())
				from->GetGuildDragonLair()->TakeItem(from, this, item);
		}
	}
#endif
}

bool CHARACTER::IsEquipUniqueItem(DWORD dwItemVnum) const
{
	{
		LPITEM u = GetWear(WEAR_UNIQUE1);

		if (u && u->GetVnum() == dwItemVnum)
			return true;
	}

	{
		LPITEM u = GetWear(WEAR_UNIQUE2);

		if (u && u->GetVnum() == dwItemVnum)
			return true;
	}

	return false;
}

// CHECK_UNIQUE_GROUP
bool CHARACTER::IsEquipUniqueGroup(DWORD dwGroupVnum) const
{
	{
		LPITEM u = GetWear(WEAR_UNIQUE1);

		if (u && u->GetSpecialGroup() == (int)dwGroupVnum)
			return true;
	}

	{
		LPITEM u = GetWear(WEAR_UNIQUE2);

		if (u && u->GetSpecialGroup() == (int)dwGroupVnum)
			return true;
	}

	return false;
}
// END_OF_CHECK_UNIQUE_GROUP

void CHARACTER::SetRefineMode(int iAdditionalCell)
{
	m_iRefineAdditionalCell = iAdditionalCell;
	SetUnderRefine(true);
}

void CHARACTER::ClearRefineMode()
{
	SetUnderRefine(false);
	SetRefineNPC(NULL);
}

//bool CHARACTER::GiveItemFromSpecialItemGroup(DWORD dwGroupNum, std::vector<DWORD>& dwItemVnums,
//	std::vector<DWORD>& dwItemCounts, std::vector <LPITEM>& item_gets, int& count)
bool CHARACTER::GiveItemFromSpecialItemGroup(DWORD dwGroupNum)
{
	const CSpecialItemGroup* pGroup = ITEM_MANAGER::instance().GetSpecialItemGroup(dwGroupNum);

	if (!pGroup)
	{
		sys_err("cannot find special item group %d", dwGroupNum);
		return false;
	}

	std::vector<int> idxes;
	int n = pGroup->GetMultiIndex(idxes);

	bool bSuccess = false;

	for (int i = 0; i < n; i++)
	{
		bSuccess = false;
		int idx = idxes[i];
		DWORD dwVnum = pGroup->GetVnum(idx);
		DWORD dwCount = pGroup->GetCount(idx);
		int iRarePct = pGroup->GetRarePct(idx);
		LPITEM item_get = NULL;
		switch (dwVnum)
		{
			case CSpecialItemGroup::GOLD:
			{
				PointChange(POINT_GOLD, dwCount, true);
				LogManager::instance().CharLog(this, dwCount, "TREASURE_GOLD", "");
				bSuccess = true;
			}
			break;

			case CSpecialItemGroup::EXP:
			{
				PointChange(POINT_EXP, dwCount);
				LogManager::instance().CharLog(this, dwCount, "TREASURE_EXP", "");
				bSuccess = true;
			}
			break;

			case CSpecialItemGroup::MOB:
			{
				sys_log(0, "CSpecialItemGroup::MOB %d", dwCount);
				int x = GetX() + number(-500, 500);
				int y = GetY() + number(-500, 500);

				LPCHARACTER ch = CHARACTER_MANAGER::instance().SpawnMob(dwCount, GetMapIndex(), x, y, 0, true, -1);
				if (ch)
					ch->SetAggressive();
				else
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("This doesn't seem to work here."));
					return false;
				}
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("Look what came out of the box!"));
				bSuccess = true;
			}
			break;

			case CSpecialItemGroup::SLOW:
			{
				sys_log(0, "CSpecialItemGroup::SLOW %d", -(int)dwCount);
				AddAffect(AFFECT_SLOW, POINT_MOV_SPEED, -(int)dwCount, AFF_SLOW, 300, 0, true);
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("If you inhale the red smoke coming out of the box, your speed will increase!"));
				bSuccess = true;
			}
			break;

			case CSpecialItemGroup::DRAIN_HP:
			{
				int iDropHP = GetMaxHP() * dwCount / 100;
				sys_log(0, "CSpecialItemGroup::DRAIN_HP %d", -iDropHP);
				iDropHP = MIN(iDropHP, GetHP() - 1);
				sys_log(0, "CSpecialItemGroup::DRAIN_HP %d", -iDropHP);
				PointChange(POINT_HP, -iDropHP);
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("The box suddenly exploded! You have lost Hit Points (HP)."));
				bSuccess = true;
			}
			break;

			case CSpecialItemGroup::POISON:
			{
				AttackedByPoison(NULL);
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("If you inhale the green smoke that is coming out of the box, the poison will spread through your body!"));
				bSuccess = true;
			}
			break;

			case CSpecialItemGroup::BLEEDING:
			{
				AttackedByBleeding(NULL);
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("If you inhale the red smoke coming out of the box, your speed will increase!"));
				bSuccess = true;
			}
			break;

			case CSpecialItemGroup::MOB_GROUP:
			{
				int sx = GetX() - number(300, 500);
				int sy = GetY() - number(300, 500);
				int ex = GetX() + number(300, 500);
				int ey = GetY() + number(300, 500);
				LPCHARACTER ch = CHARACTER_MANAGER::instance().SpawnGroup(dwCount, GetMapIndex(), sx, sy, ex, ey, NULL, true);
				if (!ch)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("This doesn't seem to work here."));
					return false;
				}
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("Look what came out of the box!"));
				bSuccess = true;
			}
			break;

			default:
			{
#if defined(__WJ_PICKUP_ITEM_EFFECT__)
				item_get = AutoGiveItem(dwVnum, dwCount, iRarePct, true, true);
#else
				item_get = AutoGiveItem(dwVnum, dwCount, iRarePct);
#endif
				if (item_get)
					bSuccess = true;
			}
			break;
		}

		if (!bSuccess)
		{
			ChatPacket(CHAT_TYPE_TALKING, LC_STRING("You have not received anything."));
			return false;
		}
	}
	return bSuccess;
}

// NEW_HAIR_STYLE_ADD
bool CHARACTER::ItemProcess_Hair(LPITEM item, int iDestCell)
{
	if (item->CheckItemUseLevel(GetLevel()) == false)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("Your level is too low to wear this Hairstyle."));
		return false;
	}

	DWORD hair = item->GetVnum();

	switch (GetJob())
	{
		case JOB_WARRIOR:
			hair -= 72000; // 73001 - 72000 = 1001
			break;

		case JOB_ASSASSIN:
			hair -= 71250;
			break;

		case JOB_SURA:
			hair -= 70500;
			break;

		case JOB_SHAMAN:
			hair -= 69750;
			break;

		case JOB_WOLFMAN:
			break;

		default:
			return false;
			break;
	}

	if (hair == GetPart(PART_HAIR))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You already have this Hairstyle."));
		return true;
	}

	item->SetCount(item->GetCount() - 1);

	SetPart(PART_HAIR, hair);
	UpdatePacket();

	return true;
}
// END_NEW_HAIR_STYLE_ADD

bool CHARACTER::ItemProcess_Polymorph(LPITEM item)
{
	if (IsPolymorphed())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You have already transformed."));
		return false;
	}

	if (true == IsRiding())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot transform while you are riding."));
		return false;
	}

	DWORD dwVnum = item->GetSocket(0);

	if (dwVnum == 0)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("That's the wrong trading item."));
		item->SetCount(item->GetCount() - 1);
		return false;
	}

	const CMob* pMob = CMobManager::instance().Get(dwVnum);

	if (pMob == NULL)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("That's the wrong trading item."));
		item->SetCount(item->GetCount() - 1);
		return false;
	}

	switch (item->GetVnum())
	{
		case 70104: // Polymorph Marble
		case 70105: // Polymorph Marble
		case 70106: // Polymorph Marble
		case 70107: // Polymorph Marble
		case 71093: // Polymorph Marble
		{
			sys_log(0, "USE_POLYMORPH_BALL PID(%d) vnum(%d)", GetPlayerID(), dwVnum);

			int iPolymorphLevelLimit = MAX(0, 20 - GetLevel() * 3 / 10);
			if (pMob->m_table.bLevel >= GetLevel() + iPolymorphLevelLimit)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot transform into a monster who has a higher level than you."));
				return false;
			}

			int iDuration = GetSkillLevel(POLYMORPH_SKILL_ID) == 0 ? 5 : (5 + (5 + GetSkillLevel(POLYMORPH_SKILL_ID) / 40 * 25));
			iDuration *= 60;

			DWORD dwBonus = 0;

			if (true == LC_IsYMIR() || true == LC_IsKorea())
			{
				dwBonus = GetSkillLevel(POLYMORPH_SKILL_ID) + 60;
			}
			else
			{
				dwBonus = (2 + GetSkillLevel(POLYMORPH_SKILL_ID) / 40) * 100;
			}

			AddAffect(AFFECT_POLYMORPH, POINT_POLYMORPH, dwVnum, AFF_POLYMORPH, iDuration, 0, true);
#if defined(ENABLE_MONSTER_CARD)
			if (item->GetSocket(1) != kalisto::MonstercardSystem::s_POLY_FROM_MONSTER_CARD_MARKER_SOCKET1)
#endif
			{
				AddAffect(AFFECT_POLYMORPH, POINT_ATT_BONUS, dwBonus, AFF_POLYMORPH, iDuration, 0, false);
			}

			item->SetCount(item->GetCount() - 1);
		}
		break;

		case 50322: // Transformation Role
		{
			sys_log(0, "USE_POLYMORPH_BOOK: %s(%u) vnum(%u)", GetName(), GetPlayerID(), dwVnum);

			if (CPolymorphUtils::instance().PolymorphCharacter(this, item, pMob) == true)
			{
				CPolymorphUtils::instance().UpdateBookPracticeGrade(this, item);
			}
			else
			{
			}
		}
		break;

		default:
			sys_err("POLYMORPH invalid item passed PID(%d) vnum(%d)", GetPlayerID(), item->GetOriginalVnum());
			return false;
	}

	return true;
}

bool CHARACTER::CanDoCube() const
{
	if (m_bIsObserver) return false;
	if (GetShop()) return false;
	if (GetMyShop()) return false;
	if (IsUnderRefine()) return false;
#if defined(__REFINE_ELEMENT_SYSTEM__)
	if (IsUnderRefineElement()) return false;
#endif
	if (IsWarping()) return false;
#if defined(__ACCE_COSTUME_SYSTEM__)
	if (IsAcceRefineWindowOpen()) return false;
#endif
#if defined(__AURA_COSTUME_SYSTEM__)
	if (IsAuraRefineWindowOpen()) return false;
#endif

	return true;
}

bool CHARACTER::UnEquipSpecialRideUniqueItem()
{
	LPITEM Unique1 = GetWear(WEAR_UNIQUE1);
	LPITEM Unique2 = GetWear(WEAR_UNIQUE2);
#if defined(__MOUNT_COSTUME_SYSTEM__)
	LPITEM MountCostume = GetWear(WEAR_COSTUME_MOUNT);
#endif

	if (NULL != Unique1)
	{
		if (UNIQUE_GROUP_SPECIAL_RIDE == Unique1->GetSpecialGroup())
		{
			return UnequipItem(Unique1);
		}
	}

	if (NULL != Unique2)
	{
		if (UNIQUE_GROUP_SPECIAL_RIDE == Unique2->GetSpecialGroup())
		{
			return UnequipItem(Unique2);
		}
	}

#if defined(__MOUNT_COSTUME_SYSTEM__)
	if (MountCostume)
		return UnequipItem(MountCostume);
#endif

	return true;
}

void CHARACTER::AutoRecoveryItemProcess(const EAffectTypes type)
{
	if (true == IsDead() || true == IsStun())
		return;

	if (false == IsPC())
		return;

	if (AFFECT_AUTO_HP_RECOVERY != type && AFFECT_AUTO_SP_RECOVERY != type)
		return;

	if (NULL != FindAffect(AFFECT_STUN))
		return;

	{
		const DWORD stunSkills[] = { SKILL_TANHWAN, SKILL_GEOMPUNG, SKILL_BYEURAK, SKILL_GIGUNG };

		for (size_t i = 0; i < sizeof(stunSkills) / sizeof(DWORD); ++i)
		{
			const CAffect* p = FindAffect(stunSkills[i]);

			if (NULL != p && AFF_STUN == p->dwFlag)
				return;
		}
	}

	const CAffect* pAffect = FindAffect(type);
	const size_t idx_of_amount_of_used = 1;
	const size_t idx_of_amount_of_full = 2;

	if (NULL != pAffect)
	{
		LPITEM pItem = FindItemByID(pAffect->dwFlag);

		if ((NULL != pItem) && (pItem->GetSocket(0) > 0))
		{
			if (false == CArenaManager::instance().IsArenaMap(GetMapIndex()))
			{
				const long amount_of_used = pItem->GetSocket(idx_of_amount_of_used);
				const long amount_of_full = pItem->GetSocket(idx_of_amount_of_full);

				const int32_t avail = amount_of_full - amount_of_used;

				int32_t amount = 0;

				if (AFFECT_AUTO_HP_RECOVERY == type)
				{
					amount = GetMaxHP() - (GetHP() + GetPoint(POINT_HP_RECOVERY));
				}
				else if (AFFECT_AUTO_SP_RECOVERY == type)
				{
					amount = GetMaxSP() - (GetSP() + GetPoint(POINT_SP_RECOVERY));
				}

				if (amount > 0)
				{
					if (avail > amount)
					{
						const int pct_of_used = amount_of_used * 100 / amount_of_full;
						const int pct_of_will_used = (amount_of_used + amount) * 100 / amount_of_full;

						bool bLog = false;

						if ((pct_of_will_used / 10) - (pct_of_used / 10) >= 1)
							bLog = true;
						pItem->SetSocket(idx_of_amount_of_used, amount_of_used + amount, bLog);
					}
					else
					{
						amount = avail;

#if defined(__USE_NEXT_AUTO_POTION__)
						for (WORD wCell = 0; wCell < INVENTORY_MAX_NUM; ++wCell)
						{
							LPITEM pkNextItem = GetInventoryItem(wCell);
							if (NULL == pkNextItem)
								continue;

							if (pItem->GetWindow() == pkNextItem->GetWindow() && pItem->GetCell() != pkNextItem->GetCell())
							{
								UseItemEx(pkNextItem, TItemPos(INVENTORY, wCell));
								break;
							}
						}
#endif

						ITEM_MANAGER::instance().RemoveItem(pItem);
					}

					if (AFFECT_AUTO_HP_RECOVERY == type)
					{
						PointChange(POINT_HP_RECOVERY, amount);
						EffectPacket(SE_AUTO_HPUP);
					}
					else if (AFFECT_AUTO_SP_RECOVERY == type)
					{
						PointChange(POINT_SP_RECOVERY, amount);
						EffectPacket(SE_AUTO_SPUP);
					}
				}
			}
			else
			{
				pItem->Lock(false);
				pItem->SetSocket(0, false);
				RemoveAffect(const_cast<CAffect*>(pAffect));
			}
		}
		else
		{
			RemoveAffect(const_cast<CAffect*>(pAffect));
		}
	}
}

bool CHARACTER::IsValidItemPosition(TItemPos Pos) const
{
	BYTE window_type = Pos.window_type;
	WORD cell = Pos.cell;

	switch (window_type)
	{
		case INVENTORY:
			return cell < INVENTORY_MAX_NUM;

		case EQUIPMENT:
			return cell < EQUIPMENT_MAX_NUM;

#if defined(__DRAGON_SOUL_SYSTEM__)
		case DRAGON_SOUL_INVENTORY:
			return cell < DRAGON_SOUL_INVENTORY_MAX_NUM;
#endif

		case BELT_INVENTORY:
			return cell < BELT_INVENTORY_MAX_NUM;

		case SAFEBOX:
			if (NULL != m_pkSafebox)
				return m_pkSafebox->IsValidPosition(cell);
			else
				return false;

		case MALL:
			if (NULL != m_pkMall)
				return m_pkMall->IsValidPosition(cell);
			else
				return false;

#if defined(__ATTR_6TH_7TH__)
		case NPC_STORAGE:
			return cell < NPC_STORAGE_SLOT_MAX;
#endif

		default:
			return false;
	}
}

bool CHARACTER::CanEquipNow(const LPITEM item, const TItemPos& srcCell, const TItemPos& destCell) /*const*/
{
	if (IsFishing())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot carry out this action while fishing."));
		return false;
	}

	const TItemTable* itemTable = item->GetProto();
	//BYTE itemType = item->GetType();
	//BYTE itemSubType = item->GetSubType();

	switch (GetJob())
	{
		case JOB_WARRIOR:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_WARRIOR)
				return false;
			break;

		case JOB_ASSASSIN:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_ASSASSIN)
				return false;
			break;

		case JOB_SHAMAN:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_SHAMAN)
				return false;
			break;

		case JOB_SURA:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_SURA)
				return false;
			break;

		case JOB_WOLFMAN:
			if (item->GetAntiFlag() & ITEM_ANTIFLAG_WOLFMAN)
				return false;
			break;

	}

	for (int i = 0; i < ITEM_LIMIT_MAX_NUM; ++i)
	{
		long limit = itemTable->aLimits[i].lValue;
		switch (itemTable->aLimits[i].bType)
		{
			case LIMIT_LEVEL:
			{
				if (GetLevel() < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("Your level is too low to equip this item."));
					return false;
				}
			}
			break;

#if defined(__CONQUEROR_LEVEL__)
			case LIMIT_NEWWORLD_LEVEL:
			{
				if (GetConquerorLevel() < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("?????? ???? ?????? ?? ???????."));
					return false;
				}
			}
			break;
#endif

			case LIMIT_STR:
			{
				if (GetPoint(POINT_ST) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("You are not strong enough to equip yourself with this item."));
					return false;
				}
			}
			break;

			case LIMIT_INT:
			{
				if (GetPoint(POINT_IQ) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("Your intelligence is too low to equip yourself with this item."));
					return false;
				}
			}
			break;

			case LIMIT_DEX:
			{
				if (GetPoint(POINT_DX) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("Your dexterity is too low to equip yourself with this item."));
					return false;
				}
			}
			break;

			case LIMIT_CON:
			{
				if (GetPoint(POINT_HT) < limit)
				{
					ChatPacket(CHAT_TYPE_INFO, LC_STRING("Your vitality is too low to equip yourself with this item."));
					return false;
				}
			}
			break;
		}
	}

	if (item->GetWearFlag() & WEARABLE_UNIQUE)
	{
		if ((GetWear(WEAR_UNIQUE1) && GetWear(WEAR_UNIQUE1)->IsSameSpecialGroup(item)) ||
			(GetWear(WEAR_UNIQUE2) && GetWear(WEAR_UNIQUE2)->IsSameSpecialGroup(item)))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot equip this item twice."));
			return false;
		}

		if (marriage::CManager::instance().IsMarriageUniqueItem(item->GetVnum()) &&
			!marriage::CManager::instance().IsMarried(GetPlayerID()))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use this item because you are not married."));
			return false;
		}
	}

#if defined(__PET_SYSTEM__)
	if (item->IsPet() && SECTREE_MANAGER::Instance().IsBlockFilterMapIndex(SECTREE_MANAGER::PET_BLOCK_MAP_INDEX, GetMapIndex()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot summon your mount/pet right now."));
		return false;
	}
#endif

#if defined(__MOUNT_COSTUME_SYSTEM__)
	if (item->IsCostumeMount() && SECTREE_MANAGER::Instance().IsBlockFilterMapIndex(SECTREE_MANAGER::MOUNT_BLOCK_MAP_INDEX, GetMapIndex()))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot summon your mount/pet right now."));
		return false;
	}
#endif

	return true;
}

bool CHARACTER::CanUnequipNow(const LPITEM item, const TItemPos& srcCell, const TItemPos& destCell) /*const*/
//bool CHARACTER::CanUnequipNow(const LPITEM item, const TItemPos& swapCell) /*const*/
{
	if (item->IsBelt())
	{
		if (CBeltInventoryHelper::IsExistItemInBeltInventory(this))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You can only discard the belt when there are no longer any items in its inventory."));
			return false;
		}
	}

	// ?????? ?????? ?? ???? ??????
	if (IS_SET(item->GetFlag(), ITEM_FLAG_IRREMOVABLE))
		return false;

	if (IsFishing())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot carry out this action while fishing."));
		return false;
	}

	// unequip
	{
		int pos = -1;

#if defined(__DRAGON_SOUL_SYSTEM__)
		if (item->IsDragonSoul())
			pos = GetEmptyDragonSoulInventory(item);
		else
#endif
			pos = GetEmptyInventory(item->GetSize());

		if (-1 == pos)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("??????? ?? ?????? ???????."));
			return false;
		}
	}

	return true;
}

#if defined(__MOVE_COSTUME_ATTR__)
void CHARACTER::OpenItemComb()
{
	if (PreventTradeWindow(WND_ALL))
	{
		ChatPacket(CHAT_TYPE_INFO, "You have to close other windows.");
		return;
	}

	const LPCHARACTER npc = GetQuestNPC();
	if (npc == NULL)
	{
		sys_err("Item Combination NPC is NULL (ch: %s)", GetName());
		return;
	}

	SetItemCombNpc(npc);
	ChatPacket(CHAT_TYPE_COMMAND, "ShowItemCombinationDialog");
}

void CHARACTER::ItemCombination(const short MediumIndex, const short BaseIndex, const short MaterialIndex)
{
	if (IsItemComb() == false)
		return;

	const LPITEM MediumItem = GetItem(TItemPos(INVENTORY, MediumIndex));
	const LPITEM BaseItem = GetItem(TItemPos(INVENTORY, BaseIndex));
	const LPITEM MaterialItem = GetItem(TItemPos(INVENTORY, MaterialIndex));

	if (MediumItem == NULL || BaseItem == NULL || MaterialItem == NULL)
		return;

	if (MediumItem->GetType() != ITEM_MEDIUM)
		return;

	if (BaseItem->GetType() != MaterialItem->GetType())
		return;

	if (BaseItem->GetSubType() != MaterialItem->GetSubType())
		return;

	if (BaseItem->GetType() != EItemTypes::ITEM_COSTUME || MaterialItem->GetType() != EItemTypes::ITEM_COSTUME)
		return;

	if (BaseItem->IsEquipped() || MaterialItem->IsEquipped())
		return;

#if defined(__SOUL_BIND_SYSTEM__)
	if (BaseItem->IsSealed() || MaterialItem->IsSealed())
		return;
#endif

	if (MediumItem->GetSubType() == MEDIUM_MOVE_COSTUME_ATTR)
	{
#if defined(__MOUNT_COSTUME_SYSTEM__)
		if (BaseItem->GetSubType() == COSTUME_MOUNT || MaterialItem->GetSubType() == COSTUME_MOUNT)
			return;
#endif

#if defined(__ACCE_COSTUME_SYSTEM__)
		if (BaseItem->GetSubType() == COSTUME_ACCE || MaterialItem->GetSubType() == COSTUME_ACCE)
			return;
#endif

#if defined(__AURA_COSTUME_SYSTEM__)
		if (BaseItem->GetSubType() == COSTUME_AURA || MaterialItem->GetSubType() == COSTUME_AURA)
			return;
#endif

		BaseItem->SetAttributes(MaterialItem->GetAttributes());
		BaseItem->UpdatePacket();

		ITEM_MANAGER::instance().RemoveItem(MaterialItem, "REMOVE (Item Combination)");
		MediumItem->SetCount(MediumItem->GetCount() - 1);
	}
#if defined(__ACCE_COSTUME_SYSTEM__)
	else if (MediumItem->GetSubType() == MEDIUM_MOVE_ACCE_ATTR)
	{
		if (MaterialItem->GetSocket(ITEM_SOCKET_ACCE_DRAIN_ITEM_VNUM) == 0)
		{
			if (MaterialItem->GetAttributeCount() == 0)
			{
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use a sash without a bonus as the source."));
				return;
			}
		}

		if (BaseItem->GetAttributeCount() > 0)
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot use a sash with a bonus for the target."));
			return;
		}

		if (BaseItem->GetSocket(ITEM_SOCKET_ACCE_DRAIN_VALUE) != MaterialItem->GetSocket(ITEM_SOCKET_ACCE_DRAIN_VALUE))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot transfer the bonus as the sashes have different absorption rates."));
			return;
		}

		if (BaseItem->FindApplyValue(APPLY_ACCEDRAIN_RATE) != MaterialItem->FindApplyValue(APPLY_ACCEDRAIN_RATE))
		{
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot transfer the bonus as the sashes have different grades."));
			return;
		}

		int iRandom = number(0, EMediumMoveAcceResult::MEDIUM_MOVE_ACCE_MAX - 1);
		switch (iRandom)
		{
			case EMediumMoveAcceResult::MEDIUM_MOVE_ACCE_FAIL:
			{
				MaterialItem->SetSocket(ITEM_SOCKET_ACCE_DRAIN_VALUE, MAX(MaterialItem->GetSocket(ITEM_SOCKET_ACCE_DRAIN_VALUE) - 1, 11));
				ChatPacket(CHAT_TYPE_INFO, LC_STRING("Failure! The absorption rate was reduced by 1%."));
			}
			break;

			case EMediumMoveAcceResult::MEDIUM_MOVE_ACCE_PARTIAL:
			{
				BaseItem->SetSockets(MaterialItem->GetSockets());
				BaseItem->SetSocket(ITEM_SOCKET_ACCE_DRAIN_VALUE, MAX(MaterialItem->GetSocket(ITEM_SOCKET_ACCE_DRAIN_VALUE) - 1, 11));
				BaseItem->SetAttributes(MaterialItem->GetAttributes());
#if defined(__ITEM_APPLY_RANDOM__)
				MaterialItem->CopyRandomAppliesTo(BaseItem);
#endif
#if defined(__REFINE_ELEMENT_SYSTEM__)
				MaterialItem->CopyElementTo(BaseItem);
#endif
				BaseItem->UpdatePacket();

				ChatPacket(CHAT_TYPE_INFO, LC_STRING("Partial success! The absorption rate was reduced by 1%."));
				ITEM_MANAGER::instance().RemoveItem(MaterialItem, "REMOVE (Item Combination)");
			}
			break;

			case EMediumMoveAcceResult::MEDIUM_MOVE_ACCE_SUCCESS:
			{
				BaseItem->SetSockets(MaterialItem->GetSockets());
				BaseItem->SetAttributes(MaterialItem->GetAttributes());
#if defined(__ITEM_APPLY_RANDOM__)
				MaterialItem->CopyRandomAppliesTo(BaseItem);
#endif
#if defined(__REFINE_ELEMENT_SYSTEM__)
				MaterialItem->CopyElementTo(BaseItem);
#endif
				BaseItem->UpdatePacket();

				ChatPacket(CHAT_TYPE_INFO, LC_STRING("Success! The bonus was transferred successfully."));
				ITEM_MANAGER::instance().RemoveItem(MaterialItem, "REMOVE (Item Combination)");
			}
			break;
		}

		MediumItem->SetCount(MediumItem->GetCount() - 1);
	}
#endif
}
#endif

#if defined(__CHANGED_ATTR__)
void CHARACTER::SelectAttr(LPITEM material, LPITEM item)
{
	const LPDESC d = GetDesc();
	if (d == nullptr)
		return;

	if (PreventTradeWindow(WND_ALL))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You cannot upgrade anything while another window is open."));
		return;
	}

	if (item->GetAttributeSetIndex() == -1)
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("????? ?????? ?? ???? ??????????."));
		return;
	}

	if (item->GetAttributeCount() < 1)
		return;

	const TItemPos pos(item->GetWindow(), item->GetCell());
	if (pos.IsInventoryPosition() == false)
		return;

	m_ItemSelectAttr.dwItemID = item->GetID();
	item->GetSelectAttr(m_ItemSelectAttr.Attr);

	TPacketGCItemSelectAttr p;
	p.bHeader = HEADER_GC_ITEM_SELECT_ATTR;
	p.pItemPos = pos;
	std::copy(std::begin(m_ItemSelectAttr.Attr), std::end(m_ItemSelectAttr.Attr), std::begin(p.aAttr));
	d->Packet(&p, sizeof p);

	material->SetCount(material->GetCount() - 1);
}

void CHARACTER::SelectAttrResult(const bool bNew, const TItemPos& pos)
{
	if (IsSelectAttr() == false)
		return;

	if (bNew)
	{
		const LPITEM item = GetItem(pos);
		if (item && item->GetID() == m_ItemSelectAttr.dwItemID)
		{
			item->SetAttributes(m_ItemSelectAttr.Attr);
			item->UpdatePacket();
			ChatPacket(CHAT_TYPE_INFO, LC_STRING("You have changed the upgrade."));
		}
	}

	memset(&m_ItemSelectAttr, 0, sizeof m_ItemSelectAttr);
}

bool CHARACTER::IsSelectAttr() const
{
	return m_ItemSelectAttr.dwItemID != 0;
}
#endif

#if defined(__LUCKY_BOX__)
void CHARACTER::SetLuckyBoxSrcItem(const LPITEM c_lpItem)
{
	if (c_lpItem == nullptr)
		return;

	if (PreventTradeWindow(WND_ALL))
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You have to close other windows."));
		return;
	}

	ResetLuckyBoxData();

	m_sLuckyBox.dwSrcItemVNum = c_lpItem->GetVnum();
	m_sLuckyBox.dwSrcItemID = c_lpItem->GetID();
	m_sLuckyBox.wSrcSlotIndex = c_lpItem->GetCell();

	SendLuckyBoxInfo();
}

void CHARACTER::SendLuckyBoxInfo()
{
	if (!IsLuckyBoxOpen())
		return;

	if (!GetDesc())
		return;

	CLuckyBoxGroup* pLuckyBox = ITEM_MANAGER::Instance().GetLuckyBoxGroup();
	if (!pLuckyBox)
		return;

	if (!pLuckyBox->ContainsItems(m_sLuckyBox.dwSrcItemVNum))
	{
		ResetLuckyBoxData();
		return;
	}

	m_sLuckyBox.bTryCount++;

	while (true)
	{
		const CLuckyBoxGroup::SLuckyBoxItemInfo& item = pLuckyBox->GetRandomItem(m_sLuckyBox.dwSrcItemVNum);
		if (pLuckyBox->GetItemCount(m_sLuckyBox.dwSrcItemVNum) > 1 && item.dwVNum == m_sLuckyBox.dwItemVNum)
			continue;

		m_sLuckyBox.dwItemVNum = item.dwVNum;
		m_sLuckyBox.bItemCount = item.bCount;
		break;
	}

	TPacketGCLuckyBox Packet;
	Packet.bHeader = HEADER_GC_LUCKY_BOX;
	Packet.dwVNum = m_sLuckyBox.dwItemVNum;
	Packet.bCount = m_sLuckyBox.bItemCount;
	Packet.iNeedMoney = GetLuckyBoxPrice();
	Packet.wSlotIndex = m_sLuckyBox.wSrcSlotIndex;
	GetDesc()->Packet(&Packet, sizeof(Packet));
}

void CHARACTER::LuckyBoxRetry()
{
	if (!IsLuckyBoxOpen())
		return;

	CLuckyBoxGroup* pLuckyBox = ITEM_MANAGER::Instance().GetLuckyBoxGroup();
	if (!pLuckyBox)
		return;

	if (m_sLuckyBox.bTryCount >= pLuckyBox->GetMaxTryCount())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You can't do this anymore."));
		return;
	}

	const int c_iPrice = GetLuckyBoxPrice();
	if (c_iPrice > GetGold())
	{
		ChatPacket(CHAT_TYPE_INFO, LC_STRING("You don't have enough money."));
		return;
	}

	PointChange(POINT_GOLD, -c_iPrice);
	ChatPacket(CHAT_TYPE_INFO, LC_STRING("%d Yang has been deducted.", c_iPrice));
	SendLuckyBoxInfo();
}

void CHARACTER::LuckyBoxReceive()
{
	if (!IsLuckyBoxOpen())
		return;

	const LPITEM c_lpSrcItem = ITEM_MANAGER::Instance().Find(m_sLuckyBox.dwSrcItemID);
	if (c_lpSrcItem)
	{
		ITEM_MANAGER::Instance().RemoveItem(c_lpSrcItem);
		AutoGiveItem(m_sLuckyBox.dwItemVNum, m_sLuckyBox.bItemCount);
	}

	ResetLuckyBoxData();
}

int CHARACTER::GetLuckyBoxPrice() const
{
	CLuckyBoxGroup* pLuckyBox = ITEM_MANAGER::Instance().GetLuckyBoxGroup();
	if (!pLuckyBox)
		return 0;

	int iRet = pLuckyBox->GetPrice();
	for (int i = 1; i < m_sLuckyBox.bTryCount; i++)
		iRet *= 2;

	return iRet;
}

bool CHARACTER::IsLuckyBoxOpen() const
{
	return m_sLuckyBox.dwSrcItemID != 0;
}

void CHARACTER::ResetLuckyBoxData()
{
	memset(&m_sLuckyBox, 0, sizeof(m_sLuckyBox));
}
#endif

#if defined(__ATTR_6TH_7TH__)
LPITEM CHARACTER::GetNPCStorageItem(BYTE bCell) const
{
	return GetItem(TItemPos(NPC_STORAGE, bCell));
}

bool CHARACTER::Attr67Add(const TAttr67AddData kAttr67AddData)
{
	if (!IsPC())
		return false;

	if (PreventTradeWindow(WND_ATTR67ADD, true/*except*/))
		return false;

	if (GetNPCStorageItem())
		return false;

	const LPITEM pkRegistItem = GetInventoryItem(kAttr67AddData.wRegistItemPos);
	if (pkRegistItem == nullptr)
		return false;

#if defined(__SOUL_BIND_SYSTEM__)
	if (pkRegistItem->IsSealed())
		return false;
#endif

	if (pkRegistItem->isLocked())
		return false;

	if (pkRegistItem->IsEquipped())
		return false;

	if (pkRegistItem->GetType() != ITEM_ARMOR && pkRegistItem->GetType() != ITEM_WEAPON)
		return false;

	if (pkRegistItem->GetAttributeCount() < ITEM_MANAGER::MAX_NORM_ATTR_NUM)
		return false;

	DWORD dwItemMaterialVnum = pkRegistItem->Get67AttrMaterial();
	if (CountSpecifyItem(dwItemMaterialVnum) < kAttr67AddData.byMaterialCount)
		return false;

	RemoveSpecifyItem(dwItemMaterialVnum, kAttr67AddData.byMaterialCount);

	long lSupportIncreasePct = 0;
	LPITEM pSupportItem = nullptr;
	{
		pSupportItem = GetInventoryItem(kAttr67AddData.wSupportItemPos);
		if (pSupportItem)
			lSupportIncreasePct = pSupportItem->GetValue(1);
	}

	// Total success percent.
	float fMaterialPct = kAttr67AddData.byMaterialCount * ATTR67_SUCCESS_PER_MATERIAL;
	float fSupportPct = 0.0f;
	if (kAttr67AddData.bySupportItemCount != 0)
	{
		fSupportPct = static_cast<float>(lSupportIncreasePct) / (ATTR67_MATERIAL_MAX_COUNT * ATTR67_SUPPORT_MAX_COUNT);
		fSupportPct *= kAttr67AddData.bySupportItemCount * kAttr67AddData.byMaterialCount;

	}
	float fTotalSuccessPct = fMaterialPct + fSupportPct;

	if (test_server)
	{
		ChatPacket(CHAT_TYPE_INFO, "<Add67> Success Percentage: %.2f", fTotalSuccessPct);
	}

	if (pSupportItem)
	{
		if (CountSpecifyItem(pSupportItem->GetVnum()) < kAttr67AddData.bySupportItemCount)
			return false;

		RemoveSpecifyItem(pSupportItem->GetVnum(), kAttr67AddData.bySupportItemCount);
	}

	pkRegistItem->RemoveFromCharacter();
	SetItem(TItemPos(NPC_STORAGE, 0), pkRegistItem);
	{
		const LPITEM pkAttr67Add = GetNPCStorageItem();
		if (!pkAttr67Add)
		{
			// TODO: Make a backup of the item in case something goes bad.
			sys_err("CHARACTER::Attr67Add: failed to get regist item from ATTR67_ADD (window).");
			return false;
		}

		SetQuestFlag("add_attr67.success", (number(1, 100) <= fTotalSuccessPct ? 1 : 0));
		SetQuestFlag("add_attr67.wait_time", get_global_time() + ATTR67_ADD_WAIT_TIME);
		SetQuestFlag("add_attr67.add", 0);
		// @ attr67add_collect (handling)
	}

	return true;
}
#endif

#if defined(__SOUL_SYSTEM__)
void CHARACTER::SoulItemProcess(ESoulSubTypes eSubType)
{
	LPITEM item = nullptr;
	for (const auto& it : GetAffectContainer())
	{
		if (it == nullptr || it->dwType != AFFECT_SOUL)
			continue;

		switch (eSubType)
		{
			case ESoulSubTypes::RED_SOUL:
			{
				if (it->wApplyOn == AFF_SOUL_RED)
					item = FindItemByID(it->lApplyValue);
			}
			break;

			case ESoulSubTypes::BLUE_SOUL:
			{
				if (it->wApplyOn == AFF_SOUL_BLUE)
					item = FindItemByID(it->lApplyValue);
			}
			break;
		}
	}

	if (item == nullptr)
		return;

	if (!item->isLocked() || item->GetSocket(1) != TRUE)
		return;

	long data = item->GetSocket(2);
	long keep_time = data / 10000;
	//auto max_time = item->GetLimitValue(1);
	long min_time = 60;

	// Minimum use time.
	if (keep_time < min_time)
		return;

	/*
	* Since the `remain_count` is added after the `keep_time`
	* we can decrease `data` directly because the count
	* stays at the end of `data`.
	*/
	//if (test_server)
	//	data -= 5;
	//else
	--data; /* remain_count */;

	/*
	* If the remaining count is equal or below to zero
	* then set decrease the socket data.
	*/
	long new_data = ((keep_time - min_time) * 10000) + item->GetValue(2);
	long remain_count = data % 10000;
	if (remain_count <= 0)
	{
		item->SetSocket(2, new_data);
		item->ResetSoulTimerUseEvent();
		return;
	}

	// Update the item with the new data (decreased count)
	item->SetSocket(2, data, false /* log */);
}

int CHARACTER::GetSoulDamage(ESoulSubTypes eSubType) const
{
	LPITEM item = nullptr;
	for (const auto& it : GetAffectContainer())
	{
		if (it == nullptr || it->dwType != AFFECT_SOUL)
			continue;

		switch (eSubType)
		{
			case ESoulSubTypes::RED_SOUL:
			{
				if (it->wApplyOn == AFF_SOUL_RED)
					item = FindItemByID(it->lApplyValue);
			}
			break;

			case ESoulSubTypes::BLUE_SOUL:
			{
				if (it->wApplyOn == AFF_SOUL_BLUE)
					item = FindItemByID(it->lApplyValue);
			}
			break;
		}
	}

	if (item == nullptr)
		return 0;

	if (item->GetSocket(1) != TRUE)
		return 0;

	int value = 0;
	long data = item->GetSocket(2);
	long keep_time = data / 10000;

	long max_time = item->GetLimitValue(1);
	long min_time = 60;

	// Minimum use time.
	if (keep_time < min_time)
		return 0;

	if (keep_time >= max_time)
		return item->GetValue(5);

	// Damage values in value field (3, 4, 5)
	int value_field = 3 + std::floor(keep_time / (max_time - min_time));
	if (value_field < ITEM_VALUES_MAX_NUM)
		value = item->GetValue(value_field);

	return value;
}
#endif

#if defined(__SET_ITEM__)
void CHARACTER::RefreshItemSetBonus()
{
	RemoveAffect(AFFECT_SET_ITEM);

	bool bSetBonus = false;
	for (const auto& [bSetValue, vItems] : ITEM_MANAGER::Instance().GetItemSetItemMap())
	{
		BYTE bWearCount = 0;

		const auto& rkItemSetValueMap = ITEM_MANAGER::Instance().GetItemSetValueMap();
		if (rkItemSetValueMap.empty())
			break;

		const auto& ItemSetValueMap = rkItemSetValueMap.find(bSetValue);
		if (ItemSetValueMap == rkItemSetValueMap.end())
			continue;

		for (const auto& [bSetType, kItemTuple] : vItems)
		{
			LPITEM pItem = nullptr;
			LPITEM pUnique1 = nullptr;
			LPITEM pUnique2 = nullptr;
			CPetSystem* pPetSystem = nullptr;

			switch (bSetType)
			{
				case SET_ITEM_COSTUME_BODY:
					pItem = GetWear(WEAR_COSTUME_BODY);
					break;

				case SET_ITEM_COSTUME_HAIR:
					pItem = GetWear(WEAR_COSTUME_HAIR);
					break;

#if defined(__MOUNT_COSTUME_SYSTEM__)
				case SET_ITEM_COSTUME_MOUNT:
					pItem = GetWear(WEAR_COSTUME_MOUNT);
					break;
#endif

#if defined(__ACCE_COSTUME_SYSTEM__)
				case SET_ITEM_COSTUME_ACCE:
					pItem = GetWear(WEAR_COSTUME_ACCE);
					break;
#endif

#if defined(__WEAPON_COSTUME_SYSTEM__)
				case SET_ITEM_COSTUME_WEAPON:
					pItem = GetWear(WEAR_COSTUME_WEAPON);
					break;
#endif

				case SET_ITEM_UNIQUE:
					pUnique1 = GetWear(WEAR_UNIQUE1);
					pUnique2 = GetWear(WEAR_UNIQUE2);
					break;

#if defined(__PET_SYSTEM__)
				case SET_ITEM_PET:
					pPetSystem = GetPetSystem();
					break;
#endif
			}

			const auto& [dwMinVnum, dwMaxVnum, bRange] = kItemTuple;

#if defined(__PET_SYSTEM__)
			if (pPetSystem && CHECK_VNUM_RANGE(pPetSystem->GetSummonItemVnum(), dwMinVnum, dwMaxVnum, bRange))
				++bWearCount;
#endif

			if (pUnique1 && CHECK_VNUM_RANGE(pUnique1->GetVnum(), dwMinVnum, dwMaxVnum, bRange))
				++bWearCount;

			if (pUnique2 && CHECK_VNUM_RANGE(pUnique2->GetVnum(), dwMinVnum, dwMaxVnum, bRange))
				++bWearCount;

			if (pItem && CHECK_VNUM_RANGE(pItem->GetVnum(), dwMinVnum, dwMaxVnum, bRange))
				++bWearCount;

			for (const auto& [bCount, vSetBonus] : ItemSetValueMap->second)
			{
				if (bWearCount != bCount)
				{
					bSetBonus = false;
					continue;
				}

				for (const auto& [wApplyType, lApplyValue] : vSetBonus)
				{
					AddAffect(AFFECT_SET_ITEM, aApplyInfo[wApplyType].wPointType, lApplyValue, 0, INFINITE_AFFECT_DURATION, 0, true, true);
					bSetBonus = true;
				}
			}
		}

		if (bSetBonus)
			break;
	}
}

CHARACTER::SetItemCountMap CHARACTER::GetItemSetCountMap() const
{
	std::vector<LPITEM> vItems = { GetWear(WEAR_BODY), GetWear(WEAR_HEAD), GetWear(WEAR_WEAPON) };
	std::map<BYTE, BYTE> mSetCount = {
		{ SET_ITEM_SET_VALUE_1, 0 },
		{ SET_ITEM_SET_VALUE_2, 0 },
		{ SET_ITEM_SET_VALUE_3, 0 },
		{ SET_ITEM_SET_VALUE_4, 0 },
		{ SET_ITEM_SET_VALUE_5, 0 },
	};

	for (const LPITEM& pkItem : vItems)
	{
		if (pkItem == nullptr)
			continue;

		const BYTE bSetValue = pkItem->GetItemSetValue();
		if (bSetValue != SET_ITEM_SET_VALUE_NONE)
			++mSetCount[bSetValue];
	}

	return mSetCount;
}

void CHARACTER::RefreshItemSetBonusByValue()
{
	for (DWORD dwType = AFFECT_SET_ITEM_SET_VALUE_1; dwType <= AFFECT_SET_ITEM_SET_VALUE_5; ++dwType)
		RemoveAffect(dwType);

	const auto& rkItemSetValueMap = ITEM_MANAGER::Instance().GetItemSetValueMap();
	if (rkItemSetValueMap.empty())
		return;

	for (const auto& rkItemSetPair : GetItemSetCountMap())
	{
		BYTE bSetValue = rkItemSetPair.first;
		BYTE bWearCount = rkItemSetPair.second;

		const auto& rkSetItemMap = rkItemSetValueMap.find(bSetValue);
		if (rkSetItemMap == rkItemSetValueMap.end())
			continue;

		for (const auto& kSetItem : rkSetItemMap->second)
		{
			const auto& vSetBonus = kSetItem.second;
			if (bWearCount != kSetItem.first)
				continue;

			for (const auto& kSetBonus : vSetBonus)
				AddAffect((AFFECT_SET_ITEM_SET_VALUE_1 - 1) + bSetValue, aApplyInfo[kSetBonus.first].wPointType, kSetBonus.second, 0, INFINITE_AFFECT_DURATION, 0, true, true);
		}
	}
}
#endif

#if defined(__REFINE_ELEMENT_SYSTEM__)
void CHARACTER::RefineElementInformation(const LPITEM& rSrcItem, const TItemPos& kItemDestCell)
{
	if (!CanHandleItem())
		return;

	LPITEM pDestItem;
	if (!IsValidItemPosition(kItemDestCell) || !(pDestItem = GetItem(kItemDestCell)))
		return;

	if (rSrcItem->IsEquipped() || pDestItem->IsEquipped())
		return;

	if (rSrcItem->IsExchanging() || pDestItem->IsExchanging())
		return;

	if (rSrcItem->isLocked() || pDestItem->isLocked())
		return;

#if defined(__SOUL_BIND_SYSTEM__)
	if (pDestItem->IsSealed())
		return;
#endif

	if (pDestItem->GetType() != ITEM_WEAPON)
		return;

	if (pDestItem->GetRefineLevel() < REFINE_ELEMENT_MIN_REFINE_LEVEL)
		return;

	TPacketGCRefineElement kPacket;
	kPacket.bHeader = HEADER_GC_REFINE_ELEMENT;
	kPacket.bSubHeader = REFINE_ELEMENT_GC_OPEN;
	switch (rSrcItem->GetSubType())
	{
		case USE_ELEMENT_UPGRADE:
			kPacket.bRefineType = REFINE_ELEMENT_UPGRADE;
			break;
		case USE_ELEMENT_DOWNGRADE:
			kPacket.bRefineType = REFINE_ELEMENT_DOWNGRADE;
			break;
		case USE_ELEMENT_CHANGE:
			kPacket.bRefineType = REFINE_ELEMENT_CHANGE;
			break;
		default:
		{
			sys_err("CHARACTER::RefineElementInformation: %s cannot receive information with unsupported material sub type.", GetName());
			return;
		}
	}
	kPacket.bResult = false;
	kPacket.SrcPos = TItemPos(rSrcItem->GetWindow(), rSrcItem->GetCell());
	kPacket.DestPos = kItemDestCell;
	GetDesc()->Packet(&kPacket, sizeof(kPacket));

	SetUnderRefineElement(true, kPacket.bRefineType, kPacket.SrcPos, kPacket.DestPos);
}

void CHARACTER::SetUnderRefineElement(bool bState, BYTE bRefineType, const TItemPos& rkSrcPos, const TItemPos& rkDestPos)
{
	m_kRefineElementItemPos.RefineType = bRefineType;
	m_kRefineElementItemPos.SrcPos = rkSrcPos;
	m_kRefineElementItemPos.DestPos = rkDestPos;
	m_bUnderRefineElement = bState;
}

void CHARACTER::RefineElement(WORD wChangeElement)
{
	if (!IsUnderRefineElement())
		return;

	LPITEM pSrcItem;
	if (!IsValidItemPosition(m_kRefineElementItemPos.SrcPos) || !(pSrcItem = GetItem(m_kRefineElementItemPos.SrcPos)))
		return;

	LPITEM pDestItem;
	if (!IsValidItemPosition(m_kRefineElementItemPos.DestPos) || !(pDestItem = GetItem(m_kRefineElementItemPos.DestPos)))
		return;

#if defined(__SOUL_BIND_SYSTEM__)
	if (pDestItem->IsSealed())
		return;
#endif

	if (pDestItem->GetType() != ITEM_WEAPON)
		return;

	if (pDestItem->GetRefineLevel() < REFINE_ELEMENT_MIN_REFINE_LEVEL)
		return;

	TPacketGCRefineElement kPacket;
	kPacket.bHeader = HEADER_GC_REFINE_ELEMENT;
	kPacket.bSubHeader = REFINE_ELEMENT_GC_RESULT;
	kPacket.bRefineType = m_kRefineElementItemPos.RefineType;
	switch (kPacket.bRefineType)
	{
		case REFINE_ELEMENT_UPGRADE:
		{
			if (GetGold() < REFINE_ELEMENT_UPGRADE_YANG)
				return;

			//const TRefineTable* pRefineTable = CRefineManager::instance().GetRefineRecipe(11);
			//if (number(1, 100) <= (pRefineTable ? pRefineTable->prob : 30))
			if (number(1, 100) <= (test_server ? 80 : 30))
			{
				const WORD wMaterialApplyType = static_cast<WORD>(pSrcItem->GetValue(0));
				const WORD wRefineElementApplyType = pDestItem->GetRefineElementApplyType();
				if ((wRefineElementApplyType != 0) && (wRefineElementApplyType != wMaterialApplyType))
				{
					sys_err("CHARACTER::RefineElement: %s cannot upgrade with wrong material.", GetName());
					return;
				}

				const BYTE bRandomValue = number_even(REFINE_ELEMENT_RANDOM_VALUE_MIN, REFINE_ELEMENT_RANDOM_VALUE_MAX);
				const BYTE bRandomBonusValue = number_even(REFINE_ELEMENT_RANDOM_BONUS_VALUE_MIN, REFINE_ELEMENT_RANDOM_BONUS_VALUE_MAX);

				pDestItem->UpgradeRefineElement(wMaterialApplyType, bRandomValue, bRandomBonusValue);

				kPacket.bResult = true;
			}
			else
			{
				kPacket.bResult = false;
			}

			PointChange(POINT_GOLD, -REFINE_ELEMENT_UPGRADE_YANG);
		}
		break;

		case REFINE_ELEMENT_DOWNGRADE:
		{
			if (GetGold() < REFINE_ELEMENT_DOWNGRADE_YANG)
				return;

			pDestItem->DowngradeRefineElement();
			kPacket.bResult = true;

			PointChange(POINT_GOLD, -REFINE_ELEMENT_DOWNGRADE_YANG);
		}
		break;

		case REFINE_ELEMENT_CHANGE:
		{
			if (GetGold() < REFINE_ELEMENT_CHANGE_YANG)
				return;

			switch (wChangeElement)
			{
				case APPLY_ENCHANT_ELECT:
				case APPLY_ENCHANT_FIRE:
				case APPLY_ENCHANT_ICE:
				case APPLY_ENCHANT_WIND:
				case APPLY_ENCHANT_EARTH:
				case APPLY_ENCHANT_DARK:
					break;

				default:
				{
					sys_err("CHARACTER::RefineElement: %s cannot change element with unsupported apply type.", GetName());
					return;
				}
			}

			const WORD wRefineElementApplyType = pDestItem->GetRefineElementApplyType();
			if (wRefineElementApplyType == wChangeElement)
			{
				sys_err("CHARACTER::RefineElement: %s cannot change element with the current apply type.", GetName());
				return;
			}

			pDestItem->ChangeRefineElement(wChangeElement);
			kPacket.bResult = true;

			PointChange(POINT_GOLD, -REFINE_ELEMENT_CHANGE_YANG);
		}
		break;

		default:
		{
			sys_err("CHARACTER::RefineElement: %s cannot refine with unknown type.", GetName());
			return;
		}
	}
	kPacket.SrcPos = NPOS;
	kPacket.DestPos = NPOS;
	GetDesc()->Packet(&kPacket, sizeof(kPacket));

	pSrcItem->SetCount(pSrcItem->GetCount() - 1);

	SetUnderRefineElement(false);
}

WORD CHARACTER::GetRefineElementEffect() const
{
	const LPITEM pItemWeapon = GetWear(WEAR_WEAPON);
	if ((pItemWeapon) && (pItemWeapon->GetRefineElementGrade() >= REFINE_ELEMENT_MAX))
		return pItemWeapon->GetRefineElementApplyType();
	return 0;
}
#endif
