// In `bool CHARACTER::CanReceiveItem(LPCHARACTER from, LPITEM item)`, extend the switch statement with:
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

// In `void CHARACTER::ReceiveItem(LPCHARACTER from, LPITEM item)`, extend the switch statement with:
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
