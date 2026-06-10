#include "StdAfx.h"

#include "InstanceBase.h"
#include "PythonBackground.h"
#include "PythonNonPlayer.h"
#include "PythonPlayer.h"
#include "PythonCharacterManager.h"
#include "AbstractPlayer.h"
#include "AbstractApplication.h"
#include "Packet.h"

#include "../EterLib/StateManager.h"
#include "../GameLib/ItemManager.h"
#include "../GameLib/RaceManager.h"

#if defined(ENABLE_METINSTONE_SWAP)
#	include "MetinstoneSwapManager.h"
#endif
#if defined(ENABLE_CHAR_CONFIG)
#	include "PythonSystem.h"
#endif
#if defined(ENABLE_LEFT_SEAT) || defined(ENABLE_TREASURE_HUNT)
#	include "PythonNetworkStream.h"
#endif

BOOL HAIR_COLOR_ENABLE = FALSE;
BOOL USE_ARMOR_SPECULAR = FALSE;
BOOL RIDE_HORSE_ENABLE = TRUE;
const float c_fDefaultRotationSpeed = 1200.0f;
const float c_fDefaultHorseRotationSpeed = 600.0f; // 300.0f

bool IsWall(unsigned race)
{
	switch (race)
	{
		case 14201:
		case 14202:
		case 14203:
		case 14204:
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////////////////

CInstanceBase::SHORSE::SHORSE()
{
	__Initialize();
}

CInstanceBase::SHORSE::~SHORSE()
{
	assert(m_pkActor == NULL);
}

void CInstanceBase::SHORSE::__Initialize()
{
	m_isMounting = false;
	m_pkActor = NULL;
}

void CInstanceBase::SHORSE::SetAttackSpeed(UINT uAtkSpd)
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor = GetActorRef();
	rkActor.SetAttackSpeed(uAtkSpd / 100.0f);
}

void CInstanceBase::SHORSE::SetMoveSpeed(UINT uMovSpd)
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor = GetActorRef();
	rkActor.SetMoveSpeed(uMovSpd / 100.0f);
}

void CInstanceBase::SHORSE::Create(const TPixelPosition& c_rkPPos, UINT eRace, UINT eHitEffect)
{
	assert(NULL == m_pkActor && "CInstanceBase::SHORSE::Create - ALREADY MOUNT");

	m_pkActor = std::make_unique<CActorInstance>();

	CActorInstance& rkActor = GetActorRef();
	rkActor.SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());
	if (!rkActor.SetRace(eRace))
	{
		m_pkActor.reset();
		return;
	}

	rkActor.SetShape(0);
	rkActor.SetBattleHitEffect(eHitEffect);
	rkActor.SetAlphaValue(0.0f);
	rkActor.BlendAlphaValue(1.0f, 0.5f);
	rkActor.SetMoveSpeed(1.0f);
	rkActor.SetAttackSpeed(1.0f);
	rkActor.SetMotionMode(CRaceMotionData::MODE_GENERAL);
	rkActor.Stop();
	rkActor.RefreshActorInstance();

	rkActor.SetCurPixelPosition(c_rkPPos);

	m_isMounting = true;
}

void CInstanceBase::SHORSE::Destroy()
{
	if (m_pkActor)
	{
		m_pkActor->Destroy();
		m_pkActor.reset();
	}

	__Initialize();
}

CActorInstance& CInstanceBase::SHORSE::GetActorRef()
{
	assert(NULL != m_pkActor && "CInstanceBase::SHORSE::GetActorRef");
	return *m_pkActor;
}

CActorInstance* CInstanceBase::SHORSE::GetActorPtr()
{
	return m_pkActor.get();
}

enum eMountType { MOUNT_TYPE_NONE = 0, MOUNT_TYPE_NORMAL = 1, MOUNT_TYPE_COMBAT = 2, MOUNT_TYPE_MILITARY = 3 };
eMountType GetMountLevelByVnum(DWORD dwMountVnum, bool IsNew)
{
	if (!dwMountVnum)
		return MOUNT_TYPE_NONE;

	switch (dwMountVnum)
	{
		// ### YES SKILL
		// @fixme116 begin
		case 20107: // normal military horse (no guild)
		case 20108: // normal military horse (guild member)
		case 20109: // normal military horse (guild master)
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
		case 20149: // normal military horse (no guild)
		case 20150: // normal military horse (guild member)
		case 20151: // normal military horse (guild master)
#endif
			if (IsNew)
				return MOUNT_TYPE_NONE;
		// @fixme116 end
		// Classic
		case 20110: // Classic Boar
		case 20111: // Classic Wolf
		case 20112: // Classic Tiger
		case 20113: // Classic Lion
		case 20114: // White Lion
		// Special Lv2
		case 20115: // Wild Battle Boar
		case 20116: // Fight Wolf
		case 20117: // Storm Tiger
		case 20118: // Battle Lion (bugged)
		case 20205: // Wild Battle Boar (alternative)
		case 20206: // Fight Wolf (alternative)
		case 20207: // Storm Tiger (alternative)
		case 20208: // Battle Lion (bugged) (alternative)
		// Royal Tigers
		case 20120: // blue
		case 20121: // dark red
		case 20122: // gold
		case 20123: // green
		case 20124: // pied
		case 20125: // white
		// Royal mounts (Special Lv3)
		case 20209: // Royal Boar
		case 20210: // Royal Wolf
		case 20211: // Royal Tiger
		case 20212: // Royal Lion

		case 20215: // Rudolph m Lv3 (yes skill, yes atk)
		case 20218: // Rudolph f Lv3 (yes skill, yes atk)
		case 20225: // Dyno Lv3 (yes skill, yes atk)
		case 20230: // Turkey Lv3 (yes skill, yes atk)
			return MOUNT_TYPE_MILITARY;
			break;
		// ### NO SKILL YES ATK
		// @fixme116 begin
		case 20104: // normal combat horse (no guild)
		case 20105: // normal combat horse (guild member)
		case 20106: // normal combat horse (guild master)
			if (IsNew)
				return MOUNT_TYPE_NONE;
		// @fixme116 end
		case 20119: // Black Horse (no skill, yes atk)
		case 20214: // Rudolph m Lv2 (no skill, yes atk)
		case 20217: // Rudolph f Lv2 (no skill, yes atk)
		case 20219: // Equus Porphyreus (no skill, yes atk)
		case 20220: // Comet (no skill, yes atk)
		case 20221: // Polar Predator (no skill, yes atk)
		case 20222: // Armoured Panda (no skill, yes atk)
		case 20224: // Dyno Lv2 (no skill, yes atk)
		case 20226: // Nightmare (no skill, yes atk)
		case 20227: // Unicorn (no skill, yes atk)
		case 20229: // Turkey Lv2 (no skill, yes atk)
		case 20231: // Leopard (no skill, yes atk)
		case 20232: // Black Panther (no skill, yes atk)
			return MOUNT_TYPE_COMBAT;
			break;
		// ### NO SKILL NO ATK
		// @fixme116 begin
		case 20101: // normal beginner horse (no guild)
		case 20102: // normal beginner horse (guild member)
		case 20103: // normal beginner horse (guild master)
			if (IsNew)
				return MOUNT_TYPE_NONE;
		// @fixme116 end
		case 20213: // Rudolph m Lv1 (no skill, no atk)
		case 20216: // Rudolph f Lv1 (no skill, no atk)
		// Special Lv1
		case 20201: // Boar Lv1 (no skill, no atk)
		case 20202: // Wolf Lv1 (no skill, no atk)
		case 20203: // Tiger Lv1 (no skill, no atk)
		case 20204: // Lion Lv1 (no skill, no atk)

		case 20223: // Dyno Lv1 (no skill, no atk)
		case 20228: // Turkey Lv1 (no skill, no atk)
			return MOUNT_TYPE_NORMAL;
			break;
		default:
			return MOUNT_TYPE_NONE;
			break;
	}
}

UINT CInstanceBase::SHORSE::GetLevel()
{
	if (m_pkActor)
	{
        DWORD dwMountVnum = m_pkActor->GetRace();
        
        if (dwMountVnum == 20101 || dwMountVnum == 20102 || dwMountVnum == 20103)
        {
            return MOUNT_TYPE_NONE;
        }

#if defined(ENABLE_NO_MOUNT_CHECK)
		return MOUNT_TYPE_MILITARY;
#else
		return static_cast<UINT>(GetMountLevelByVnum(dwMountVnum, false));
#endif
	}
	return 0;
}

bool CInstanceBase::SHORSE::IsNewMount()
{
#if !defined(ENABLE_NO_MOUNT_CHECK)
	if (m_pkActor)
	{
		DWORD dwMountVnum = m_pkActor->GetRace();
		eMountType mountType = GetMountLevelByVnum(dwMountVnum, true);
		return (mountType != MOUNT_TYPE_NONE) && (mountType != MOUNT_TYPE_NORMAL);
	}
#endif

	return false;
}

bool CInstanceBase::SHORSE::CanUseSkill()
{
	if (IsMounting())
		return 2 < GetLevel();

	return true;
}

bool CInstanceBase::SHORSE::CanAttack()
{
	if (IsMounting())
	{
		if (GetLevel() <= 1)
			return false;
	}

	return true;
}

bool CInstanceBase::SHORSE::IsMounting()
{
	return m_isMounting;
}

void CInstanceBase::SHORSE::Deform()
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor = GetActorRef();
	rkActor.INSTANCEBASE_Deform();
}

void CInstanceBase::SHORSE::Render()
{
	if (!IsMounting())
		return;

	CActorInstance& rkActor = GetActorRef();
	rkActor.Render();
}

void CInstanceBase::__AttachHorseSaddle()
{
	if (!IsMountingHorse())
		return;
	m_kHorse.m_pkActor->AttachModelInstance(CRaceData::PART_MAIN, "saddle", m_GraphicThingInstance, CRaceData::PART_MAIN);
}

void CInstanceBase::__DetachHorseSaddle()
{
	if (!IsMountingHorse())
		return;
	m_kHorse.m_pkActor->DetachModelInstance(CRaceData::PART_MAIN, m_GraphicThingInstance, CRaceData::PART_MAIN);
}

//////////////////////////////////////////////////////////////////////////////////////

void CInstanceBase::BlockMovement()
{
	m_GraphicThingInstance.BlockMovement();
}

bool CInstanceBase::IsBlockObject(const CGraphicObjectInstance& c_rkBGObj)
{
	return m_GraphicThingInstance.IsBlockObject(c_rkBGObj);
}

bool CInstanceBase::AvoidObject(const CGraphicObjectInstance& c_rkBGObj)
{
#ifdef __MOVIE_MODE__
	if (IsMovieMode())
		return false;
#endif

	return m_GraphicThingInstance.AvoidObject(c_rkBGObj);
}

///////////////////////////////////////////////////////////////////////////////////

bool __ArmorVnumToShape(int iVnum, DWORD* pdwShape)
{
	*pdwShape = iVnum;

	/////////////////////////////////////////

	if (0 == iVnum || 1 == iVnum)
		return false;

	if (!USE_ARMOR_SPECULAR)
		return false;

	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(iVnum, &pItemData))
		return false;

	enum
	{
		SHAPE_VALUE_SLOT_INDEX = 3,
	};

	*pdwShape = pItemData->GetValue(SHAPE_VALUE_SLOT_INDEX);

	return true;
}

// 2004.07.05.myevan.?????? ????? ????
class CActorInstanceBackground : public IBackground
{
public:
	CActorInstanceBackground() {}
	virtual ~CActorInstanceBackground() {}
	bool IsBlock(int x, int y)
	{
		CPythonBackground& rkBG = CPythonBackground::Instance();
		return rkBG.isAttrOn(x, y, CTerrainImpl::ATTRIBUTE_BLOCK);
	}
};

static CActorInstanceBackground gs_kActorInstBG;

bool CInstanceBase::LessRenderOrder(CInstanceBase* pkInst)
{
	int nMainAlpha = (__GetAlphaValue() < 1.0f) ? 1 : 0;
	int nTestAlpha = (pkInst->__GetAlphaValue() < 1.0f) ? 1 : 0;
	if (nMainAlpha < nTestAlpha)
		return true;
	if (nMainAlpha > nTestAlpha)
		return false;

	if (GetRace() < pkInst->GetRace())
		return true;
	if (GetRace() > pkInst->GetRace())
		return false;

	if (GetShape() < pkInst->GetShape())
		return true;

	if (GetShape() > pkInst->GetShape())
		return false;

	UINT uLeftLODLevel = __LessRenderOrder_GetLODLevel();
	UINT uRightLODLevel = pkInst->__LessRenderOrder_GetLODLevel();
	if (uLeftLODLevel < uRightLODLevel)
		return true;
	if (uLeftLODLevel > uRightLODLevel)
		return false;

	if (m_adwPart[CRaceData::PART_WEAPON] < pkInst->m_adwPart[CRaceData::PART_WEAPON])
		return true;

	return false;
}

UINT CInstanceBase::__LessRenderOrder_GetLODLevel()
{
	CGrannyLODController* pLODCtrl = m_GraphicThingInstance.GetLODControllerPointer(0);
	if (!pLODCtrl)
		return 0;

	return pLODCtrl->GetLODLevel();
}

bool CInstanceBase::__Background_GetWaterHeight(const TPixelPosition& c_rkPPos, float* pfHeight)
{
	long lHeight;
	if (!CPythonBackground::Instance().GetWaterHeight(int(c_rkPPos.x), int(c_rkPPos.y), &lHeight))
		return false;

	*pfHeight = float(lHeight);

	return true;
}

bool CInstanceBase::__Background_IsWaterPixelPosition(const TPixelPosition& c_rkPPos)
{
	return CPythonBackground::Instance().isAttrOn(c_rkPPos.x, c_rkPPos.y, CTerrainImpl::ATTRIBUTE_WATER);
}

const float PC_DUST_RANGE = 2000.0f;
const float NPC_DUST_RANGE = 1000.0f;

DWORD CInstanceBase::ms_dwUpdateCounter = 0;
DWORD CInstanceBase::ms_dwRenderCounter = 0;
DWORD CInstanceBase::ms_dwDeformCounter = 0;

CDynamicPool<CInstanceBase> CInstanceBase::ms_kPool;

bool CInstanceBase::__IsInDustRange()
{
	if (!__IsExistMainInstance())
		return false;

	CInstanceBase* pkInstMain = __GetMainInstancePtr();

	float fDistance = NEW_GetDistanceFromDestInstance(*pkInstMain);

	if (IsPC())
	{
		if (fDistance <= PC_DUST_RANGE)
			return true;
	}

	if (fDistance <= NPC_DUST_RANGE)
		return true;

	return false;
}

void CInstanceBase::__EnableSkipCollision()
{
	if (__IsMainInstance())
	{
		TraceError("CInstanceBase::__EnableSkipCollision - ????? ?��?????? ??? ????!!");
		return;
	}
	m_GraphicThingInstance.EnableSkipCollision();
}

void CInstanceBase::__DisableSkipCollision()
{
	m_GraphicThingInstance.DisableSkipCollision();
}

void CInstanceBase::EnableSkipHorseCollision()
{
	if (!IsMountingHorse())
		return;

	CActorInstance* actor = m_kHorse.GetActorPtr();
	if (actor)
		actor->EnableSkipCollision();
}

float CInstanceBase::__GetBackgroundHeight(float x, float y)
{
	CPythonBackground& rkBG = CPythonBackground::Instance();
	return rkBG.GetHeight(x, y);
}

#ifdef __MOVIE_MODE__
BOOL CInstanceBase::IsMovieMode()
{
	if (IsAffect(AFFECT_INVISIBILITY))
		return true;

	return false;
}
#endif

BOOL CInstanceBase::IsInvisibility()
{
	if (IsAffect(AFFECT_INVISIBILITY))
		return true;

	return false;
}

BOOL CInstanceBase::IsStealth()
{
	if (IsAffect(AFFECT_EUNHYEONG))
		return true;

	return false;
}

BOOL CInstanceBase::IsParalysis()
{
	return m_GraphicThingInstance.IsParalysis();
}

BOOL CInstanceBase::IsGameMaster()
{
	if (m_kAffectFlagContainer.IsSet(AFFECT_YMIR))
		return true;
	return false;
}

BOOL CInstanceBase::IsSameEmpire(CInstanceBase& rkInstDst)
{
	if (0 == rkInstDst.m_dwEmpireID)
		return TRUE;

	if (IsGameMaster())
		return TRUE;

	if (rkInstDst.IsGameMaster())
		return TRUE;

	if (rkInstDst.m_dwEmpireID == m_dwEmpireID)
		return TRUE;

	return FALSE;
}

DWORD CInstanceBase::GetEmpireID()
{
	return m_dwEmpireID;
}

DWORD CInstanceBase::GetGuildID()
{
	return m_dwGuildID;
}

#if defined(ENABLE_GUILD_LEADER_GRADE_NAME)
BYTE CInstanceBase::GetGuildLeaderGrade()
{
	return m_bGuildLeaderGrade;
}
#endif

#ifdef WJ_SHOW_MOB_INFO
DWORD CInstanceBase::GetAIFlag()
{
	return m_dwAIFlag;
}
#endif


int CInstanceBase::GetAlignment()
{
	return m_sAlignment;
}

UINT CInstanceBase::GetAlignmentGrade()
{
	if (m_sAlignment >= 12000)
		return 0;
	else if (m_sAlignment >= 8000)
		return 1;
	else if (m_sAlignment >= 4000)
		return 2;
	else if (m_sAlignment >= 1000)
		return 3;
	else if (m_sAlignment >= 0)
		return 4;
	else if (m_sAlignment > -4000)
		return 5;
	else if (m_sAlignment > -8000)
		return 6;
	else if (m_sAlignment > -12000)
		return 7;
	else
		return 8;

	return 0;
}

int CInstanceBase::GetAlignmentType()
{
	switch (GetAlignmentGrade())
	{
		case 0:
		case 1:
		case 2:
		case 3:
		{
			return ALIGNMENT_TYPE_WHITE;
			break;
		}

		case 5:
		case 6:
		case 7:
		case 8:
		{
			return ALIGNMENT_TYPE_DARK;
			break;
		}
	}

	return ALIGNMENT_TYPE_NORMAL;
}

#if defined(ENABLE_GUILD_LEADER_GRADE_NAME)
BYTE CInstanceBase::GetGuildLeaderGradeType()
{
	if (m_bGuildLeaderGrade == 3)
		return 0;
	else if (m_bGuildLeaderGrade == 2)
		return 1;

	return 2;
}
#endif

BYTE CInstanceBase::GetPKMode()
{
	return m_byPKMode;
}

bool CInstanceBase::IsKiller()
{
	return m_isKiller;
}

bool CInstanceBase::IsPartyMember()
{
	return m_isPartyMember;
}

BOOL CInstanceBase::IsInSafe()
{
	const TPixelPosition& c_rkPPosCur = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	if (CPythonBackground::Instance().isAttrOn(c_rkPPosCur.x, c_rkPPosCur.y, CTerrainImpl::ATTRIBUTE_BANPK))
		return TRUE;

	return FALSE;
}

float CInstanceBase::CalculateDistanceSq3d(const TPixelPosition& c_rkPPosDst)
{
	const TPixelPosition& c_rkPPosSrc = m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
	return SPixelPosition_CalculateDistanceSq3d(c_rkPPosSrc, c_rkPPosDst);
}

void CInstanceBase::OnSelected()
{
#ifdef __MOVIE_MODE__
	if (!__IsExistMainInstance())
		return;
#endif

	if (IsStoneDoor())
		return;

	if (IsDead())
		return;

#if defined(ENABLE_RENDER_LOGIN_EFFECTS)
	if (!m_GraphicThingInstance.IsLoginRender())
		__AttachSelectEffect();
#else
	__AttachSelectEffect();
#endif
}

void CInstanceBase::OnUnselected()
{
	__DetachSelectEffect();
}

void CInstanceBase::OnTargeted()
{
#ifdef __MOVIE_MODE__
	if (!__IsExistMainInstance())
		return;
#endif

	if (IsStoneDoor())
		return;

	if (IsDead())
		return;

	__AttachTargetEffect();
}

void CInstanceBase::OnUntargeted()
{
	__DetachTargetEffect();
}

void CInstanceBase::DestroySystem()
{
	ms_kPool.Clear();
}

void CInstanceBase::CreateSystem(UINT uCapacity)
{
	ms_kPool.Create(uCapacity);

	memset(ms_adwCRCAffectEffect, 0, sizeof(ms_adwCRCAffectEffect));

	ms_fDustGap = 250.0f;
	ms_fHorseDustGap = 500.0f;
}

CInstanceBase* CInstanceBase::New()
{
	return ms_kPool.Alloc();
}

void CInstanceBase::Delete(CInstanceBase* pkInst)
{
	pkInst->Destroy();
	ms_kPool.Free(pkInst);
}

void CInstanceBase::SetMainInstance()
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();

	DWORD dwVID = GetVirtualID();
	rkChrMgr.SetMainInstance(dwVID);

	m_GraphicThingInstance.SetMainInstance();
}

CInstanceBase* CInstanceBase::__GetMainInstancePtr()
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	return rkChrMgr.GetMainInstancePtr();
}

void CInstanceBase::__ClearMainInstance()
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	rkChrMgr.ClearMainInstance();
}

bool CInstanceBase::__IsMainInstance()
{
	if (this == __GetMainInstancePtr())
		return true;

	return false;
}

bool CInstanceBase::__IsExistMainInstance()
{
	if (__GetMainInstancePtr())
		return true;

	return false;
}

bool CInstanceBase::__MainCanSeeHiddenThing()
{
	return false;
}

float CInstanceBase::__GetBowRange()
{
	float fRange = 2500.0f - 100.0f;

	if (__IsMainInstance())
	{
		IAbstractPlayer& rPlayer = IAbstractPlayer::GetSingleton();
		fRange += float(rPlayer.GetStatus(POINT_BOW_DISTANCE));
	}

	return fRange;
}

CInstanceBase* CInstanceBase::__FindInstancePtr(DWORD dwVID)
{
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	return rkChrMgr.GetInstancePtr(dwVID);
}

bool CInstanceBase::__FindRaceType(DWORD dwRace, BYTE* pbType)
{
	CPythonNonPlayer& rkNonPlayer = CPythonNonPlayer::Instance();
	return rkNonPlayer.GetInstanceType(dwRace, pbType);
}

bool CInstanceBase::Create(const SCreateData& c_rkCreateData)
{
	IAbstractApplication::GetSingleton().SkipRenderBuffering(300);

	SetInstanceType(c_rkCreateData.m_bType);

#if defined(ENABLE_METINSTONE_SWAP)
	m_GraphicThingInstance.SetRaceDataStoneShapeEvent(c_rkCreateData.m_dwRace);
#endif

#if defined(ENABLE_RACE_SWAP)
	if (!SetRace(c_rkCreateData.m_dwRace, c_rkCreateData.m_dwEventRace))
		return false;
#else
	if (!SetRace(c_rkCreateData.m_dwRace))
		return false;
#endif

	SetVirtualID(c_rkCreateData.m_dwVID);

#if defined(ENABLE_CHAR_CONFIG)
	__Create_SetName(c_rkCreateData);

	if (c_rkCreateData.m_isMain)
	{
		SetMainInstance();
		CPythonSystem::Instance().LoadCharConfig();
#if defined(ENABLE_LEFT_SEAT)
		CPythonPlayer::Instance().LoadLeftSeatData();
#endif
	}
#else
	if (c_rkCreateData.m_isMain)
	{
		SetMainInstance();
#if defined(ENABLE_LEFT_SEAT)
		CPythonPlayer::Instance().LoadLeftSeatData();
#endif
	}
#endif

	if (IsGuildWall())
	{
		unsigned center_x;
		unsigned center_y;

		c_rkCreateData.m_kAffectFlags.ConvertToPosition(&center_x, &center_y);

		float center_z = __GetBackgroundHeight(center_x, center_y);
		NEW_SetPixelPosition(TPixelPosition(float(c_rkCreateData.m_lPosX), float(c_rkCreateData.m_lPosY), center_z));
	}
	else
	{
		SCRIPT_SetPixelPosition(float(c_rkCreateData.m_lPosX), float(c_rkCreateData.m_lPosY));
	}

	if (0 != c_rkCreateData.m_dwMountVnum)
		MountHorse(c_rkCreateData.m_dwMountVnum);

	SetArmor(c_rkCreateData.m_dwArmor);

	if (IsPC())
	{
#if defined(ENABLE_REFINE_ELEMENT_SYSTEM)
		SetRefineElement(c_rkCreateData.m_wRefineElementAffectType);
#endif

		SetHair(c_rkCreateData.m_dwHair);
#if defined(ENABLE_ACCE_COSTUME_SYSTEM)
		SetAcce(c_rkCreateData.m_dwAcce);
#endif
#if defined(ENABLE_AURA_COSTUME_SYSTEM)
		SetAura(c_rkCreateData.m_dwAura);
#endif
		SetWeapon(c_rkCreateData.m_dwWeapon);
#if defined(ENABLE_QUIVER_SYSTEM)
		SetArrow(c_rkCreateData.m_dwArrow);
#endif
	}

#if !defined(ENABLE_CHAR_CONFIG)
	__Create_SetName(c_rkCreateData);
#endif

	m_dwLevel = c_rkCreateData.m_dwLevel;
#if defined(ENABLE_CONQUEROR_LEVEL)
	m_dwConquerorLevel = c_rkCreateData.m_dwConquerorLevel;
#endif
#ifdef WJ_SHOW_MOB_INFO
	m_dwAIFlag = c_rkCreateData.m_dwAIFlag;
#endif
	m_dwGuildID = c_rkCreateData.m_dwGuildID;
	m_dwEmpireID = c_rkCreateData.m_dwEmpireID;
#if defined(ENABLE_GUILD_LEADER_GRADE_NAME)
	m_bGuildLeaderGrade = c_rkCreateData.m_bGuildLeaderGrade;
#endif
#if defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
	m_stCountry = c_rkCreateData.m_stCountry;
#endif

	SetVirtualNumber(c_rkCreateData.m_dwRace);
	SetRotation(c_rkCreateData.m_fRot);

	SetAlignment(c_rkCreateData.m_sAlignment);
	SetPKMode(c_rkCreateData.m_byPKMode);

	SetMoveSpeed(c_rkCreateData.m_dwMovSpd);
	SetAttackSpeed(c_rkCreateData.m_dwAtkSpd);
#if defined(ENABLE_GRAPHIC_ON_OFF)
	SetRank(CPythonNonPlayer::Instance().GetRank(c_rkCreateData.m_dwRace));
#endif

	float fScale = 1.0f;
	BYTE bScale = CPythonNonPlayer::Instance().GetMonsterScalePercent(c_rkCreateData.m_dwRace);
	if (bScale != 100)
		fScale = static_cast<float>(bScale / 100.0f);

#if defined(ENABLE_GROWTH_PET_SYSTEM)
	if (c_rkCreateData.m_bType == CActorInstance::TYPE_PET)
		fScale = c_rkCreateData.m_dwLevel * 0.005f + 1.1f;
#endif

	m_GraphicThingInstance.SetScaleWorld(fScale, fScale, fScale);

	if (!IsWearingDress())
	{
		m_GraphicThingInstance.SetAlphaValue(0.0f);
		m_GraphicThingInstance.BlendAlphaValue(1.0f, 0.5f);
	}

	if (!IsGuildWall())
	{
		SetAffectFlagContainer(c_rkCreateData.m_kAffectFlags);
	}

	AttachTextTail();
	RefreshTextTail();

	if (c_rkCreateData.m_dwStateFlags & ADD_CHARACTER_STATE_SPAWN)
	{
		if (IsAffect(AFFECT_SPAWN))
			__AttachEffect(EFFECT_SPAWN_APPEAR);

		if (IsPC())
		{
			Refresh(CRaceMotionData::NAME_WAIT, true);
		}
		else
		{
			Refresh(CRaceMotionData::NAME_SPAWN, false);
		}
	}
	else
	{
		Refresh(CRaceMotionData::NAME_WAIT, true);
	}

	__AttachEmpireEffect(c_rkCreateData.m_dwEmpireID);

	RegisterBoundingSphere();

	if (c_rkCreateData.m_dwStateFlags & ADD_CHARACTER_STATE_DEAD)
		m_GraphicThingInstance.DieEnd();

	SetStateFlags(c_rkCreateData.m_dwStateFlags);

	m_GraphicThingInstance.SetBattleHitEffect(ms_adwCRCAffectEffect[EFFECT_HIT]);

#if defined(ENABLE_METINSTONE_SWAP)
	const DWORD c_dwBodyColor = CPythonNonPlayer::Instance().GetMonsterColor(c_rkCreateData.m_dwRace);
	if (c_dwBodyColor)
	{
		if (IsStone() && MetinStoneSwap::Manager::Instance().GetEventFlag())
		{
			const MetinStoneSwap::SwapInfo* swapInfo = MetinStoneSwap::Manager::Instance().GetSwapDataByVnum(c_rkCreateData.m_dwRace);
			if (!swapInfo || swapInfo->use_color)
				SetColorOperation(c_dwBodyColor);
		}
		else
		{
			SetColorOperation(c_dwBodyColor);
		}
	}
#else
	if (!IsPC())
	{
		DWORD dwBodyColor = CPythonNonPlayer::Instance().GetMonsterColor(c_rkCreateData.m_dwRace);
		if (0 != dwBodyColor)
		{
			SetModulateRenderMode();
			SetAddColor(dwBodyColor);
		}
	}
#endif

	__AttachHorseSaddle();

	const int c_iGuildSymbolRace = 14200;
	if (c_iGuildSymbolRace == GetRace())
	{
		std::string strFileName = GetGuildSymbolFileName(m_dwGuildID);
		if (IsFile(strFileName.c_str()))
			m_GraphicThingInstance.ChangeMaterial(strFileName.c_str());
	}
	return true;
}

void CInstanceBase::__Create_SetName(const SCreateData& c_rkCreateData)
{
	if (IsGoto())
	{
		SetNameString("", 0);
		return;
	}

	if (IsWarp())
	{
		__Create_SetWarpName(c_rkCreateData);
		return;
	}

	SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
}

void CInstanceBase::__Create_SetWarpName(const SCreateData& c_rkCreateData)
{
	const char* c_szName;
	if (CPythonNonPlayer::Instance().GetName(c_rkCreateData.m_dwRace, &c_szName))
	{
		std::string strName = c_szName;
		int iFindingPos = strName.find_first_of(" ", 0);
		if (iFindingPos > 0)
		{
			strName.resize(iFindingPos);
		}
		SetNameString(strName.c_str(), strName.length());
	}
	else
	{
		SetNameString(c_rkCreateData.m_stName.c_str(), c_rkCreateData.m_stName.length());
	}
}

void CInstanceBase::SetNameString(const char* c_szName, int len)
{
	m_stName.assign(c_szName, len);
}

bool CInstanceBase::SetRace(DWORD eRace
#if defined(ENABLE_RACE_SWAP)
	, DWORD eEventRace
#endif
)
{
	m_dwRace = eRace;

#if defined(ENABLE_RACE_SWAP)
	if (!m_GraphicThingInstance.SetRace(eRace, eEventRace))
		return false;
#else
	if (!m_GraphicThingInstance.SetRace(eRace))
		return false;
#endif

	if (!__FindRaceType(m_dwRace, &m_eRaceType))
		m_eRaceType = CActorInstance::TYPE_PC;

	return true;
}

BOOL CInstanceBase::__IsChangableWeapon(int iWeaponID)
{
	if (!IsWearingDress())
		return true;

	const int c_iBouquets[] =
	{
		50201, // Bouquet for Assassin
		50202, // Bouquet for Shaman
		50203,
		50204,
		0,
	};

	for (int i = 0; c_iBouquets[i] != 0; ++i)
		if (iWeaponID == c_iBouquets[i])
			return true;

	return false;
}

BOOL CInstanceBase::IsWearingDress()
{
	const int c_iWeddingDressShape = 201;
	return c_iWeddingDressShape == m_eShape;
}

BOOL CInstanceBase::IsHoldingPickAxe()
{
	const int c_iPickAxeStart = 29101;
	const int c_iPickAxeEnd = 29110;
	return m_adwPart[CRaceData::PART_WEAPON] >= c_iPickAxeStart && m_adwPart[CRaceData::PART_WEAPON] <= c_iPickAxeEnd;
}

BOOL CInstanceBase::IsHoldingFishingRod()
{
	const int c_iFishingRodStart = 27400;
	const int c_iFishingRodEnd = 27591;
	return m_adwPart[CRaceData::PART_WEAPON] >= c_iFishingRodStart && m_adwPart[CRaceData::PART_WEAPON] <= c_iFishingRodEnd;
}

BOOL CInstanceBase::IsNewMount()
{
	return m_kHorse.IsNewMount();
}

BOOL CInstanceBase::IsMountingHorse()
{
	return m_kHorse.IsMounting();
}

void CInstanceBase::MountHorse(UINT eRace)
{
	m_kHorse.Destroy();
	m_kHorse.Create(m_GraphicThingInstance.NEW_GetCurPixelPositionRef(), eRace, ms_adwCRCAffectEffect[EFFECT_HIT]);

	SetMotionMode(CRaceMotionData::MODE_HORSE);
	SetRotationSpeed(c_fDefaultHorseRotationSpeed);

	m_GraphicThingInstance.MountHorse(m_kHorse.GetActorPtr());
	m_GraphicThingInstance.Stop();
	m_GraphicThingInstance.RefreshActorInstance();
}

void CInstanceBase::DismountHorse()
{
	m_kHorse.Destroy();
}

void CInstanceBase::GetInfo(std::string* pstInfo)
{
	char szInfo[256];
	sprintf(szInfo, "Inst - UC %d, RC %d Pool - %d ",
		ms_dwUpdateCounter,
		ms_dwRenderCounter,
		ms_kPool.GetCapacity()
	);

	pstInfo->append(szInfo);
}

void CInstanceBase::ResetPerformanceCounter()
{
	ms_dwUpdateCounter = 0;
	ms_dwRenderCounter = 0;
	ms_dwDeformCounter = 0;
}

bool CInstanceBase::NEW_IsLastPixelPosition()
{
	return m_GraphicThingInstance.IsPushing();
}

const TPixelPosition& CInstanceBase::NEW_GetLastPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetLastPixelPositionRef();
}

void CInstanceBase::NEW_SetDstPixelPositionZ(FLOAT z)
{
	m_GraphicThingInstance.NEW_SetDstPixelPositionZ(z);
}

void CInstanceBase::NEW_SetDstPixelPosition(const TPixelPosition& c_rkPPosDst)
{
	m_GraphicThingInstance.NEW_SetDstPixelPosition(c_rkPPosDst);
}

void CInstanceBase::NEW_SetSrcPixelPosition(const TPixelPosition& c_rkPPosSrc)
{
	m_GraphicThingInstance.NEW_SetSrcPixelPosition(c_rkPPosSrc);
}

const TPixelPosition& CInstanceBase::NEW_GetCurPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetCurPixelPositionRef();
}

const TPixelPosition& CInstanceBase::NEW_GetDstPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetDstPixelPositionRef();
}

const TPixelPosition& CInstanceBase::NEW_GetSrcPixelPositionRef()
{
	return m_GraphicThingInstance.NEW_GetSrcPixelPositionRef();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

void CInstanceBase::OnSyncing()
{
	m_GraphicThingInstance.__OnSyncing();
}

void CInstanceBase::OnWaiting()
{
	m_GraphicThingInstance.__OnWaiting();
}

void CInstanceBase::OnMoving()
{
	m_GraphicThingInstance.__OnMoving();
}

void CInstanceBase::ChangeGuild(DWORD dwGuildID
#if defined(ENABLE_GUILD_LEADER_GRADE_NAME)
	, BYTE bGuildLeaderGrade
#endif
)
{
	m_dwGuildID = dwGuildID;
#if defined(ENABLE_GUILD_LEADER_GRADE_NAME)
	m_bGuildLeaderGrade = bGuildLeaderGrade;
#endif

	DetachTextTail();
	AttachTextTail();
	RefreshTextTail();
}

DWORD CInstanceBase::GetPart(CRaceData::EParts part)
{
	assert(part >= 0 && part < CRaceData::PART_MAX_NUM);
	return m_adwPart[part];
}

DWORD CInstanceBase::GetShape()
{
	return m_eShape;
}

bool CInstanceBase::CanAct()
{
	return m_GraphicThingInstance.CanAct();
}

bool CInstanceBase::CanMove()
{
	return m_GraphicThingInstance.CanMove();
}

bool CInstanceBase::CanUseSkill(DWORD dwSkillIndex)
{
	if (IsPoly())
		return false;

	if (IsWearingDress())
		return false;

	if (IsHoldingPickAxe())
		return false;

	if (!m_kHorse.CanUseSkill())
		return false;

	if (!m_GraphicThingInstance.CanUseSkill())
		return false;

	return true;
}

bool CInstanceBase::CanAttack()
{
	if (!m_kHorse.CanAttack())
		return false;

	if (IsWearingDress())
		return false;

	if (IsHoldingPickAxe())
		return false;

	if (IsHoldingFishingRod())
		return false;

	return m_GraphicThingInstance.CanAttack();
}

bool CInstanceBase::CanFishing()
{
	return m_GraphicThingInstance.CanFishing();
}

BOOL CInstanceBase::IsBowMode()
{
	return m_GraphicThingInstance.IsBowMode();
}

BOOL CInstanceBase::IsHandMode()
{
	return m_GraphicThingInstance.IsHandMode();
}

BOOL CInstanceBase::IsFishingMode()
{
	if (CRaceMotionData::MODE_FISHING == m_GraphicThingInstance.GetMotionMode())
		return true;

	return false;
}

BOOL CInstanceBase::IsFishing()
{
	return m_GraphicThingInstance.IsFishing();
}

BOOL CInstanceBase::IsDead()
{
	return m_GraphicThingInstance.IsDead();
}

BOOL CInstanceBase::IsStun()
{
	return m_GraphicThingInstance.IsStun();
}

BOOL CInstanceBase::IsSleep()
{
	return m_GraphicThingInstance.IsSleep();
}

BOOL CInstanceBase::__IsSyncing()
{
	return m_GraphicThingInstance.__IsSyncing();
}

void CInstanceBase::NEW_SetOwner(DWORD dwVIDOwner)
{
	m_GraphicThingInstance.SetOwner(dwVIDOwner);
}

float CInstanceBase::GetLocalTime()
{
	return m_GraphicThingInstance.GetLocalTime();
}

void CInstanceBase::PushUDPState(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg)
{
}

DWORD ELTimer_GetServerFrameMSec();

void CInstanceBase::PushTCPStateExpanded(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg, UINT uTargetVID)
{
	SCommand kCmdNew = {};
	kCmdNew.m_kPPosDst = c_rkPPosDst;
	kCmdNew.m_dwChkTime = dwCmdTime + 100;
	kCmdNew.m_dwCmdTime = dwCmdTime;
	kCmdNew.m_fDstRot = fDstRot;
	kCmdNew.m_eFunc = eFunc;
	kCmdNew.m_uArg = uArg;
	kCmdNew.m_uTargetVID = uTargetVID;
	m_kQue_kCmdNew.push_back(kCmdNew);
}

void CInstanceBase::PushTCPState(DWORD dwCmdTime, const TPixelPosition& c_rkPPosDst, float fDstRot, UINT eFunc, UINT uArg)
{
	if (__IsMainInstance())
	{
		TraceError("CInstanceBase::PushTCPState You can't send move packets to yourself!");
		return;
	}

	const int nNetworkGap = ELTimer_GetServerFrameMSec() - dwCmdTime;

	m_nAverageNetworkGap = (m_nAverageNetworkGap * 70 + nNetworkGap * 30) / 100;

	//m_dwBaseChkTime - m_dwBaseCmdTime + ELTimer_GetServerMSec();

	SCommand kCmdNew = {};
	kCmdNew.m_kPPosDst = c_rkPPosDst;
	kCmdNew.m_dwChkTime = dwCmdTime + m_nAverageNetworkGap; //m_dwBaseChkTime + (dwCmdTime - m_dwBaseCmdTime);// + nNetworkGap;
	kCmdNew.m_dwCmdTime = dwCmdTime;
	kCmdNew.m_fDstRot = fDstRot;
	kCmdNew.m_eFunc = eFunc;
	kCmdNew.m_uArg = uArg;
	m_kQue_kCmdNew.push_back(kCmdNew);
}

BOOL CInstanceBase::__CanProcessNetworkStatePacket()
{
	if (m_GraphicThingInstance.IsDead())
		return FALSE;

	if (m_GraphicThingInstance.IsKnockDown())
		return FALSE;

	if (m_GraphicThingInstance.IsUsingSkill())
		if (!m_GraphicThingInstance.CanCancelSkill())
			return FALSE;

	return TRUE;
}

BOOL CInstanceBase::__IsEnableTCPProcess(UINT eCurFunc)
{
	if (m_GraphicThingInstance.IsActTargetEmotion())
		return FALSE;

	if (!m_bEnableTCPState)
	{
		if (FUNC_EMOTION != eCurFunc)
		{
			return FALSE;
		}
	}

	return TRUE;
}

void CInstanceBase::StateProcess()
{
	while (true)
	{
		if (m_kQue_kCmdNew.empty())
			return;

		DWORD dwDstChkTime = m_kQue_kCmdNew.front().m_dwChkTime;
		DWORD dwCurChkTime = ELTimer_GetServerFrameMSec();

		if (dwCurChkTime < dwDstChkTime)
			return;

		SCommand kCmdTop = m_kQue_kCmdNew.front();
		m_kQue_kCmdNew.pop_front();

		TPixelPosition kPPosDst = kCmdTop.m_kPPosDst;
		//DWORD dwCmdTime = kCmdTop.m_dwCmdTime;
		FLOAT fRotDst = kCmdTop.m_fDstRot;
		UINT eFunc = kCmdTop.m_eFunc;
		UINT uArg = kCmdTop.m_uArg;

		UINT uVID = GetVirtualID();
		UINT uTargetVID = kCmdTop.m_uTargetVID;

		TPixelPosition kPPosCur;
		NEW_GetPixelPosition(&kPPosCur);

		/*
		if (IsPC())
			Tracenf("%d cmd: vid=%d[%s] func=%d arg=%d curPos=(%f, %f) dstPos=(%f, %f) rot=%f (time %d)",
				ELTimer_GetMSec(),
				uVID, m_stName.c_str(), eFunc, uArg,
				kPPosCur.x, kPPosCur.y,
				kPPosDst.x, kPPosDst.y, fRotDst, dwCmdTime - m_dwBaseCmdTime);
		*/

		TPixelPosition kPPosDir = kPPosDst - kPPosCur;
		float fDirLen = (float)sqrt(kPPosDir.x * kPPosDir.x + kPPosDir.y * kPPosDir.y);

		if (!__CanProcessNetworkStatePacket())
		{
			Lognf(0, "vid=%d ?????? ?? ???? ???��? ??? IsDead=%d, IsKnockDown=%d", uVID, m_GraphicThingInstance.IsDead(), m_GraphicThingInstance.IsKnockDown());
			return;
		}

		if (!__IsEnableTCPProcess(eFunc))
			return;

		switch (eFunc)
		{
			case FUNC_WAIT:
			{
				if (fDirLen > 1.0f)
				{
					//NEW_GetSrcPixelPositionRef() = kPPosCur;
					//NEW_GetDstPixelPositionRef() = kPPosDst;
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);

					__EnableSkipCollision();

					m_fDstRot = fRotDst;
					m_isGoing = TRUE;

					m_kMovAfterFunc.eFunc = FUNC_WAIT;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					m_isGoing = FALSE;

					if (!IsWaiting())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					SetAdvancingRotation(fRotDst);
					SetRotation(fRotDst);
				}
			}
			break;

			case FUNC_MOVE:
			{
				//NEW_GetSrcPixelPositionRef() = kPPosCur;
				//NEW_GetDstPixelPositionRef() = kPPosDst;
				NEW_SetSrcPixelPosition(kPPosCur);
				NEW_SetDstPixelPosition(kPPosDst);
				m_fDstRot = fRotDst;
				m_isGoing = TRUE;
				__EnableSkipCollision();
				//m_isSyncMov = TRUE;

				m_kMovAfterFunc.eFunc = FUNC_MOVE;

				if (!IsWalking())
				{
					StartWalking();
				}
				else
				{
				}
			}
			break;

			case FUNC_COMBO:
			{
				if (fDirLen >= 50.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_COMBO;
					m_kMovAfterFunc.uArg = uArg;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					RunComboAttack(fRotDst, uArg);
				}
			}
			break;

			case FUNC_ATTACK:
			{
				if (fDirLen >= 50.0f)
				{
					//NEW_GetSrcPixelPositionRef() = kPPosCur;
					//NEW_GetDstPixelPositionRef() = kPPosDst;
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();
					//m_isSyncMov = TRUE;

					m_kMovAfterFunc.eFunc = FUNC_ATTACK;

					if (!IsWalking())
						StartWalking();

				}
				else
				{

					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					BlendRotation(fRotDst);

					RunNormalAttack(fRotDst);

				}
			}
			break;

			case FUNC_MOB_SKILL:
			{
				if (fDirLen >= 50.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;
					__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_MOB_SKILL;
					m_kMovAfterFunc.uArg = uArg;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					m_isGoing = FALSE;

					if (IsWalking())
						EndWalking();

					SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
					BlendRotation(fRotDst);

					m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPECIAL_1 + uArg);
				}
			}
			break;

			case FUNC_EMOTION:
			{
				if (fDirLen > 100.0f)
				{
					NEW_SetSrcPixelPosition(kPPosCur);
					NEW_SetDstPixelPosition(kPPosDst);
					m_fDstRot = fRotDst;
					m_isGoing = TRUE;

					if (__IsMainInstance())
						__EnableSkipCollision();

					m_kMovAfterFunc.eFunc = FUNC_EMOTION;
					m_kMovAfterFunc.uArg = uArg;
					m_kMovAfterFunc.uArgExpanded = uTargetVID;
					m_kMovAfterFunc.kPosDst = kPPosDst;

					if (!IsWalking())
						StartWalking();
				}
				else
				{
					__ProcessFunctionEmotion(uArg, uTargetVID, kPPosDst);
				}
			}
			break;

			default:
			{
				if (eFunc & FUNC_SKILL)
				{
					if (fDirLen >= 50.0f)
					{
						//NEW_GetSrcPixelPositionRef() = kPPosCur;
						//NEW_GetDstPixelPositionRef() = kPPosDst;
						NEW_SetSrcPixelPosition(kPPosCur);
						NEW_SetDstPixelPosition(kPPosDst);
						m_fDstRot = fRotDst;
						m_isGoing = TRUE;
						//m_isSyncMov = TRUE;
						__EnableSkipCollision();

						m_kMovAfterFunc.eFunc = eFunc;
						m_kMovAfterFunc.uArg = uArg;

						if (!IsWalking())
							StartWalking();
					}
					else
					{
						m_isGoing = FALSE;

						if (IsWalking())
							EndWalking();

						SCRIPT_SetPixelPosition(kPPosDst.x, kPPosDst.y);
						SetAdvancingRotation(fRotDst);
						SetRotation(fRotDst);

						NEW_UseSkill(0, eFunc & 0x7f, uArg & 0x0f, (uArg >> 4) ? true : false);
					}
				}
			}
			break;
		}
	}
}

void CInstanceBase::MovementProcess()
{
	TPixelPosition kPPosCur;
	NEW_GetPixelPosition(&kPPosCur);

	TPixelPosition kPPosNext;
	{
		const D3DXVECTOR3& c_rkV3Mov = m_GraphicThingInstance.GetMovementVectorRef();

		kPPosNext.x = kPPosCur.x + (+c_rkV3Mov.x);
		kPPosNext.y = kPPosCur.y + (-c_rkV3Mov.y);
		kPPosNext.z = kPPosCur.z + (+c_rkV3Mov.z);
	}

	TPixelPosition kPPosDeltaSC = kPPosCur - NEW_GetSrcPixelPositionRef();
	TPixelPosition kPPosDeltaSN = kPPosNext - NEW_GetSrcPixelPositionRef();
	TPixelPosition kPPosDeltaSD = NEW_GetDstPixelPositionRef() - NEW_GetSrcPixelPositionRef();

	float fCurLen = sqrtf(kPPosDeltaSC.x * kPPosDeltaSC.x + kPPosDeltaSC.y * kPPosDeltaSC.y);
	float fNextLen = sqrtf(kPPosDeltaSN.x * kPPosDeltaSN.x + kPPosDeltaSN.y * kPPosDeltaSN.y);
	float fTotalLen = sqrtf(kPPosDeltaSD.x * kPPosDeltaSD.x + kPPosDeltaSD.y * kPPosDeltaSD.y);
	float fRestLen = fTotalLen - fCurLen;

	if (__IsMainInstance())
	{
		if (m_isGoing && IsWalking())
		{
			float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

			SetAdvancingRotation(fDstRot);

			if (fRestLen <= 0.0)
			{
				if (IsWalking())
					EndWalking();


				m_isGoing = FALSE;

				BlockMovement();

				if (FUNC_EMOTION == m_kMovAfterFunc.eFunc)
				{
					DWORD dwMotionNumber = m_kMovAfterFunc.uArg;
					DWORD dwTargetVID = m_kMovAfterFunc.uArgExpanded;
					__ProcessFunctionEmotion(dwMotionNumber, dwTargetVID, m_kMovAfterFunc.kPosDst);
					m_kMovAfterFunc.eFunc = FUNC_WAIT;
					return;
				}
			}
		}
	}
	else
	{
		if (m_isGoing && IsWalking())
		{
			float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(NEW_GetSrcPixelPositionRef(), NEW_GetDstPixelPositionRef());

			SetAdvancingRotation(fDstRot);

			if (fRestLen < -100.0f)
			{
				NEW_SetSrcPixelPosition(kPPosCur);

				float fDstRot = NEW_GetAdvancingRotationFromPixelPosition(kPPosCur, NEW_GetDstPixelPositionRef());
				SetAdvancingRotation(fDstRot);
				//Tracenf("VID %d ???? ?????? (%f, %f) %f rest %f", GetVirtualID(), kPPosCur.x, kPPosCur.y, fDstRot, fRestLen);

				if (FUNC_MOVE == m_kMovAfterFunc.eFunc)
					m_kMovAfterFunc.eFunc = FUNC_WAIT;
			}
			else if (fCurLen <= fTotalLen && fTotalLen <= fNextLen)
			{
				if (m_GraphicThingInstance.IsDead() || m_GraphicThingInstance.IsKnockDown())
				{
					__DisableSkipCollision();


					m_isGoing = FALSE;

				}
				else
				{
					switch (m_kMovAfterFunc.eFunc)
					{
						case FUNC_ATTACK:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							SetAdvancingRotation(m_fDstRot);
							SetRotation(m_fDstRot);

							RunNormalAttack(m_fDstRot);
							break;
						}

						case FUNC_COMBO:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							RunComboAttack(m_fDstRot, m_kMovAfterFunc.uArg);
							break;
						}

						case FUNC_EMOTION:
						{
							m_isGoing = FALSE;
							m_kMovAfterFunc.eFunc = FUNC_WAIT;
							__DisableSkipCollision();
							BlockMovement();

							DWORD dwMotionNumber = m_kMovAfterFunc.uArg;
							DWORD dwTargetVID = m_kMovAfterFunc.uArgExpanded;
							__ProcessFunctionEmotion(dwMotionNumber, dwTargetVID, m_kMovAfterFunc.kPosDst);
							break;
						}

						case FUNC_MOVE:
						{
#if defined(ENABLE_TREASURE_HUNT)
							// Treasure goblin escort: client visual arrival (server spawns wave on packet).
							if (GetVirtualNumber() == 20856)
							{
								if (IsWalking())
									EndWalking();

								__DisableSkipCollision();
								m_isGoing = FALSE;

								BlockMovement();
								SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
								SetAdvancingRotation(m_fDstRot);
								BlendRotation(m_fDstRot);

								const char* pszMap = CPythonBackground::Instance().GetWarpMapName();
								if (pszMap && 0 == strcmp(pszMap, "metin2_map_treasure_hunt"))
									CPythonNetworkStream::Instance().SendTreasureHuntGoblinArrived();

								m_kMovAfterFunc.eFunc = FUNC_WAIT;
							}
#endif
							break;
						}

						case FUNC_MOB_SKILL:
						{
							if (IsWalking())
								EndWalking();

							__DisableSkipCollision();
							m_isGoing = FALSE;

							BlockMovement();
							SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
							SetAdvancingRotation(m_fDstRot);
							SetRotation(m_fDstRot);

							m_GraphicThingInstance.InterceptOnceMotion(CRaceMotionData::NAME_SPECIAL_1 + m_kMovAfterFunc.uArg);
							break;
						}

						default:
						{
							if (m_kMovAfterFunc.eFunc & FUNC_SKILL)
							{
								SetAdvancingRotation(m_fDstRot);
								BlendRotation(m_fDstRot);
								NEW_UseSkill(0, m_kMovAfterFunc.eFunc & 0x7f, m_kMovAfterFunc.uArg & 0x0f, (m_kMovAfterFunc.uArg >> 4) ? true : false);
							}
							else
							{
								//Tracenf("VID %d ??? ???? (%f, %f) rot %f", GetVirtualID(), NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y, m_fDstRot);

								__DisableSkipCollision();
								m_isGoing = FALSE;

								BlockMovement();
								SCRIPT_SetPixelPosition(NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y);
								SetAdvancingRotation(m_fDstRot);
								BlendRotation(m_fDstRot);
								if (!IsWaiting())
								{
									EndWalking();
								}

								//Tracenf("VID %d ???? (%f, %f) rot %f IsWalking %d", GetVirtualID(), NEW_GetDstPixelPositionRef().x, NEW_GetDstPixelPositionRef().y, m_fDstRot, IsWalking());
							}
							break;
						}
					}
				}
			}
		}
	}

	if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill())
	{
		float fRotation = m_GraphicThingInstance.GetRotation();
		float fAdvancingRotation = m_GraphicThingInstance.GetAdvancingRotation();
		int iDirection = GetRotatingDirection(fRotation, fAdvancingRotation);

		if (DEGREE_DIRECTION_SAME != m_iRotatingDirection)
		{
			if (DEGREE_DIRECTION_LEFT == iDirection)
			{
				fRotation = fmodf(fRotation + m_fRotSpd * m_GraphicThingInstance.GetSecondElapsed(), 360.0f);
			}
			else if (DEGREE_DIRECTION_RIGHT == iDirection)
			{
				fRotation = fmodf(fRotation - m_fRotSpd * m_GraphicThingInstance.GetSecondElapsed() + 360.0f, 360.0f);
			}

			if (m_iRotatingDirection != GetRotatingDirection(fRotation, fAdvancingRotation))
			{
				m_iRotatingDirection = DEGREE_DIRECTION_SAME;
				fRotation = fAdvancingRotation;
			}

			m_GraphicThingInstance.SetRotation(fRotation);
		}

		if (__IsInDustRange())
		{
			float fDustDistance = NEW_GetDistanceFromDestPixelPosition(m_kPPosDust);
			if (IsMountingHorse())
			{
				if (fDustDistance > ms_fHorseDustGap)
				{
					NEW_GetPixelPosition(&m_kPPosDust);
					__AttachEffect(EFFECT_HORSE_DUST);
				}
			}
			else
			{
				if (fDustDistance > ms_fDustGap)
				{
					NEW_GetPixelPosition(&m_kPPosDust);
					__AttachEffect(EFFECT_DUST);
				}
			}
		}
	}
}

void CInstanceBase::__ProcessFunctionEmotion(DWORD dwMotionNumber, DWORD dwTargetVID, const TPixelPosition& c_rkPosDst)
{
	if (IsWalking())
		EndWalkingWithoutBlending();

	__EnableChangingTCPState();
	SCRIPT_SetPixelPosition(c_rkPosDst.x, c_rkPosDst.y);

	CInstanceBase* pTargetInstance = CPythonCharacterManager::Instance().GetInstancePtr(dwTargetVID);
	if (pTargetInstance)
	{
		pTargetInstance->__EnableChangingTCPState();

		if (pTargetInstance->IsWalking())
			pTargetInstance->EndWalkingWithoutBlending();

		WORD wMotionNumber1 = HIWORD(dwMotionNumber);
		WORD wMotionNumber2 = LOWORD(dwMotionNumber);

		int src_job = RaceToJob(GetRace());
		int dst_job = RaceToJob(pTargetInstance->GetRace());

		NEW_LookAtDestInstance(*pTargetInstance);
		m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber1 + dst_job);
		m_GraphicThingInstance.SetRotation(m_GraphicThingInstance.GetTargetRotation());
		m_GraphicThingInstance.SetAdvancingRotation(m_GraphicThingInstance.GetTargetRotation());

		pTargetInstance->NEW_LookAtDestInstance(*this);
		pTargetInstance->m_GraphicThingInstance.InterceptOnceMotion(wMotionNumber2 + src_job);
		pTargetInstance->m_GraphicThingInstance.SetRotation(pTargetInstance->m_GraphicThingInstance.GetTargetRotation());
		pTargetInstance->m_GraphicThingInstance.SetAdvancingRotation(pTargetInstance->m_GraphicThingInstance.GetTargetRotation());

		if (pTargetInstance->__IsMainInstance())
		{
			IAbstractPlayer& rPlayer = IAbstractPlayer::GetSingleton();
			rPlayer.EndEmotionProcess();
		}
	}

	if (__IsMainInstance())
	{
		IAbstractPlayer& rPlayer = IAbstractPlayer::GetSingleton();
		rPlayer.EndEmotionProcess();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Update & Deform & Render

int g_iAccumulationTime = 0;

void CInstanceBase::Update()
{
	++ms_dwUpdateCounter;

	StateProcess();
	m_GraphicThingInstance.PhysicsProcess();
	m_GraphicThingInstance.RotationProcess();
	m_GraphicThingInstance.ComboProcess();
	m_GraphicThingInstance.AccumulationMovement();

	if (m_GraphicThingInstance.IsMovement())
	{
		TPixelPosition kPPosCur;
		NEW_GetPixelPosition(&kPPosCur);

		DWORD dwCurTime = ELTimer_GetFrameMSec();
		//if (m_dwNextUpdateHeightTime<dwCurTime)
		{
			m_dwNextUpdateHeightTime = dwCurTime;
			kPPosCur.z = __GetBackgroundHeight(kPPosCur.x, kPPosCur.y);
			NEW_SetPixelPosition(kPPosCur);
		}
	}

	m_GraphicThingInstance.UpdateAdvancingPointInstance();

#if defined(ENABLE_OPTIMIZATION)
	if (CPythonSystem::Instance().IsShowOtherCharAttacked())
		AttackProcess();
	else
		AttackProcessOnlySelf();
#else
	AttackProcess();
#endif
	MovementProcess();

#if defined(ENABLE_GRAPHIC_ON_OFF)
	if (!m_GraphicThingInstance.IsMainInstance() && HasPrivateShopSign())
		m_GraphicThingInstance.SetPrivateShopDistance(GetDistanceBetweenShop());

	if (IsPC())
		m_GraphicThingInstance.SetInvisible(IsInvisibility());
#endif

	m_GraphicThingInstance.MotionProcess(IsPC());

#if defined(ENABLE_EMOTION_HIDE_WEAPON)
	if (m_GraphicThingInstance.GetCurrentMotionIndex() < CRaceMotionData::NAME_CLAP || m_GraphicThingInstance.GetCurrentMotionIndex() == CRaceMotionData::NAME_DIG)
	{
		if (m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON) != m_adwPart[CRaceData::PART_WEAPON])
		{
			m_GraphicThingInstance.AttachWeapon(m_adwPart[CRaceData::PART_WEAPON]);
			CItemData* pItemData;
			if (CItemManager::Instance().GetItemDataPointer(m_adwPart[CRaceData::PART_WEAPON], &pItemData))
				__GetRefinedEffect(pItemData);

#if defined(ENABLE_HIDE_ATTACHING_EFFECT_ON_ACTING)
			m_GraphicThingInstance.ShowAllAttachingEffect();
#endif
		}
	}
	else if (m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON))
	{
		m_GraphicThingInstance.AttachWeapon(0);
		__ClearWeaponRefineEffect();
#if defined(ENABLE_HIDE_ATTACHING_EFFECT_ON_ACTING)
		m_GraphicThingInstance.HideAllAttachingEffect();
#endif
	}
#endif

	if (IsMountingHorse())
	{
		m_kHorse.m_pkActor->HORSE_MotionProcess(FALSE);
	}

	__ComboProcess();

	while (ProcessDamage());
}

void CInstanceBase::Transform()
{
	if (__IsSyncing())
	{
		OnSyncing();
	}
	else
	{
		if (IsWalking() || m_GraphicThingInstance.IsUsingMovingSkill())
		{
			const D3DXVECTOR3& c_rv3Movment = m_GraphicThingInstance.GetMovementVectorRef();

			float len = (c_rv3Movment.x * c_rv3Movment.x) + (c_rv3Movment.y * c_rv3Movment.y);
			if (len > 1.0f)
				OnMoving();
			else
				OnWaiting();
		}
	}

	m_GraphicThingInstance.INSTANCEBASE_Transform();
}

void CInstanceBase::Deform()
{
	if (!__CanRender())
		return;

	++ms_dwDeformCounter;

	m_GraphicThingInstance.INSTANCEBASE_Deform();

	m_kHorse.Deform();
}

void CInstanceBase::RenderTrace()
{
	if (!__CanRender())
		return;

	m_GraphicThingInstance.RenderTrace();
}

void CInstanceBase::Render()
{
	if (!__CanRender())
		return;

	++ms_dwRenderCounter;

	m_kHorse.Render();
	m_GraphicThingInstance.Render();

	if (CActorInstance::IsDirLine())
	{
		if (NEW_GetDstPixelPositionRef().x != 0.0f)
		{
			static CScreen s_kScreen;

			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			STATEMANAGER.SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
			STATEMANAGER.SaveRenderState(D3DRS_ZENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, FALSE);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, FALSE);

			TPixelPosition px;
			m_GraphicThingInstance.GetPixelPosition(&px);
			D3DXVECTOR3 kD3DVt3Cur(px.x, px.y, px.z);
			//D3DXVECTOR3 kD3DVt3Cur(NEW_GetSrcPixelPositionRef().x, -NEW_GetSrcPixelPositionRef().y, NEW_GetSrcPixelPositionRef().z);
			D3DXVECTOR3 kD3DVt3Dest(NEW_GetDstPixelPositionRef().x, -NEW_GetDstPixelPositionRef().y, NEW_GetDstPixelPositionRef().z);

			//printf("%s %f\n", GetNameString(), kD3DVt3Cur.y - kD3DVt3Dest.y);
			//float fdx = NEW_GetDstPixelPositionRef().x - NEW_GetSrcPixelPositionRef().x;
			//float fdy = NEW_GetDstPixelPositionRef().y - NEW_GetSrcPixelPositionRef().y;

			s_kScreen.SetDiffuseColor(0.0f, 0.0f, 1.0f);
			s_kScreen.RenderLine3d(kD3DVt3Cur.x, kD3DVt3Cur.y, px.z, kD3DVt3Dest.x, kD3DVt3Dest.y, px.z);
			STATEMANAGER.RestoreRenderState(D3DRS_ZENABLE);
			STATEMANAGER.SetRenderState(D3DRS_FOGENABLE, TRUE);
			STATEMANAGER.SetRenderState(D3DRS_LIGHTING, TRUE);
		}
	}
}

void CInstanceBase::RenderToShadowMap()
{
	if (IsDoor())
		return;

	if (IsBuilding())
		return;

	if (!__CanRender())
		return;

	if (!__IsExistMainInstance())
		return;

	CInstanceBase* pkInstMain = __GetMainInstancePtr();

	const float SHADOW_APPLY_DISTANCE = 7500.0f;

	float fDistance = NEW_GetDistanceFromDestInstance(*pkInstMain);
	if (fDistance >= SHADOW_APPLY_DISTANCE)
		return;

	m_GraphicThingInstance.RenderToShadowMap();
}

void CInstanceBase::RenderCollision()
{
	m_GraphicThingInstance.RenderCollisionData();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Setting & Getting Data

void CInstanceBase::SetVirtualID(DWORD dwVirtualID)
{
	m_GraphicThingInstance.SetVirtualID(dwVirtualID);
}

void CInstanceBase::SetVirtualNumber(DWORD dwVirtualNumber)
{
	m_dwVirtualNumber = dwVirtualNumber;
}

void CInstanceBase::SetInstanceType(int iInstanceType)
{
	m_GraphicThingInstance.SetActorType(iInstanceType);
}

void CInstanceBase::SetAlignment(short sAlignment)
{
	m_sAlignment = sAlignment;
	RefreshTextTailTitle();
}

void CInstanceBase::SetPKMode(BYTE byPKMode)
{
	if (m_byPKMode == byPKMode)
		return;

	m_byPKMode = byPKMode;

	if (__IsMainInstance())
	{
		IAbstractPlayer& rPlayer = IAbstractPlayer::GetSingleton();
		rPlayer.NotifyChangePKMode();
	}
}

void CInstanceBase::SetKiller(bool bFlag)
{
	if (m_isKiller == bFlag)
		return;

	m_isKiller = bFlag;
	RefreshTextTail();
}

void CInstanceBase::SetPartyMemberFlag(bool bFlag)
{
	m_isPartyMember = bFlag;
}

void CInstanceBase::SetStateFlags(DWORD dwStateFlags)
{
	if (dwStateFlags & ADD_CHARACTER_STATE_KILLER)
		SetKiller(TRUE);
	else
		SetKiller(FALSE);

	if (dwStateFlags & ADD_CHARACTER_STATE_PARTY)
		SetPartyMemberFlag(TRUE);
	else
		SetPartyMemberFlag(FALSE);
}

#if defined(ENABLE_LEFT_SEAT)
void CInstanceBase::SetLeftSeat(bool bLeftSeat)
{
	m_bLeftSeat = bLeftSeat;

	CPythonNetworkStream::Instance().SetLeftSeatLogOutState(bLeftSeat);
}
#endif

void CInstanceBase::SetComboType(UINT uComboType)
{
	m_GraphicThingInstance.SetComboType(uComboType);
}

const char* CInstanceBase::GetNameString()
{
	return m_stName.c_str();
}

int CInstanceBase::GetLevel()
{
	return m_dwLevel;
}

DWORD CInstanceBase::GetRace()
{
	return m_dwRace;
}

bool CInstanceBase::IsConflictAlignmentInstance(CInstanceBase& rkInstVictim)
{
	if (PK_MODE_PROTECT == rkInstVictim.GetPKMode())
		return false;

	switch (GetAlignmentType())
	{
		case ALIGNMENT_TYPE_NORMAL:
		case ALIGNMENT_TYPE_WHITE:
			if (ALIGNMENT_TYPE_DARK == rkInstVictim.GetAlignmentType())
				return true;
			break;
		case ALIGNMENT_TYPE_DARK:
			if (GetAlignmentType() != rkInstVictim.GetAlignmentType())
				return true;
			break;
	}

	return false;
}

void CInstanceBase::SetDuelMode(DWORD type)
{
	m_dwDuelMode = type;
}

DWORD CInstanceBase::GetDuelMode()
{
	return m_dwDuelMode;
}

bool CInstanceBase::IsAttackableInstance(CInstanceBase& rkInstVictim)
{
	if (__IsMainInstance())
	{
		CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
		if (rkPlayer.IsObserverMode())
			return false;
	}

	if (GetVirtualID() == rkInstVictim.GetVirtualID())
		return false;

	if (IsStone())
	{
		if (rkInstVictim.IsPC())
			return true;
	}
	else if (IsPC())
	{
		if (rkInstVictim.IsStone())
			return true;

		if (rkInstVictim.IsPC())
		{
			if (GetDuelMode())
			{
				switch (GetDuelMode())
				{
					case DUEL_CANNOTATTACK:
						return false;
					case DUEL_START:
						if (__FindDUELKey(GetVirtualID(), rkInstVictim.GetVirtualID()))
							return true;
						else
							return false;
				}
			}
			if (PK_MODE_GUILD == GetPKMode())
				if (GetGuildID() == rkInstVictim.GetGuildID())
					return false;

			if (rkInstVictim.IsKiller())
				if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
					return true;

			if (PK_MODE_PROTECT != GetPKMode())
			{
				if (PK_MODE_FREE == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
						if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
							return true;
				}
				if (PK_MODE_GUILD == GetPKMode())
				{
					if (PK_MODE_PROTECT != rkInstVictim.GetPKMode())
						if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
							if (GetGuildID() != rkInstVictim.GetGuildID())
								return true;
				}
			}

			if (IsSameEmpire(rkInstVictim))
			{
				if (IsPVPInstance(rkInstVictim))
					return true;

				if (PK_MODE_REVENGE == GetPKMode())
					if (!IAbstractPlayer::GetSingleton().IsSamePartyMember(GetVirtualID(), rkInstVictim.GetVirtualID()))
						if (IsConflictAlignmentInstance(rkInstVictim))
							return true;
			}
			else
			{
				return true;
			}
		}

		if (rkInstVictim.IsEnemy())
			return true;

		if (rkInstVictim.IsWoodenDoor())
			return true;
	}
	else if (IsEnemy())
	{
		if (rkInstVictim.IsPC())
			return true;

		if (rkInstVictim.IsBuilding())
			return true;

	}
	else if (IsPoly())
	{
		if (rkInstVictim.IsPC())
			return true;

		if (rkInstVictim.IsEnemy())
			return true;
	}
	return false;
}

bool CInstanceBase::IsTargetableInstance(CInstanceBase& rkInstVictim)
{
	return rkInstVictim.CanPickInstance();
}

bool CInstanceBase::CanChangeTarget()
{
	return m_GraphicThingInstance.CanChangeTarget();
}

bool CInstanceBase::CanPickInstance()
{
	if (!__IsInViewFrustum())
		return false;

#if defined(ENABLE_GRAPHIC_ON_OFF)
	if (!CanRenderActor())
		return false;
#endif

	if (IsDoor())
	{
		if (IsDead())
			return false;
	}

	if (IsPC())
	{
		if (IsAffect(AFFECT_EUNHYEONG))
		{
			if (!__MainCanSeeHiddenThing())
				return false;
		}
		if (IsAffect(AFFECT_REVIVE_INVISIBILITY))
			return false;
		if (IsAffect(AFFECT_INVISIBILITY))
			return false;
	}

	if (IsDead())
		return false;

	return true;
}

bool CInstanceBase::CanViewTargetHP(CInstanceBase& rkInstVictim)
{
	if (rkInstVictim.IsStone())
		return true;

	if (rkInstVictim.IsWoodenDoor())
		return true;

	if (rkInstVictim.IsEnemy())
		return true;

#if defined(ENABLE_VIEW_TARGET_PLAYER_HP)
	if (rkInstVictim.IsPC())
		return true;
#endif

	return false;
}

#if defined(ENABLE_GRAPHIC_ON_OFF)
bool CInstanceBase::IsMainInstance() const
{
	return m_GraphicThingInstance.IsMainInstance();
}

bool CInstanceBase::CanRenderActor() const
{
	return m_GraphicThingInstance.CanRenderActor();
}

bool CInstanceBase::CanRenderEffect() const
{
	return m_GraphicThingInstance.CanRenderEffect();
}

bool CInstanceBase::HasPrivateShopSign() const
{
	return m_GraphicThingInstance.HasPrivateShopSign();
}

DWORD CInstanceBase::GetDistanceBetweenShop()
{
	if (m_GraphicThingInstance.IsMainInstance())
		return 0;

	CInstanceBase* pkInstMain = __GetMainInstancePtr();
	if (!pkInstMain)
		return 0;

	TPixelPosition kPPosCur;
	NEW_GetPixelPosition(&kPPosCur);

	TPixelPosition kPPosCur2;
	pkInstMain->NEW_GetPixelPosition(&kPPosCur2);

	const float fX = kPPosCur2.x - kPPosCur.x;
	const float fY = kPPosCur2.y - kPPosCur.y;
	return static_cast<DWORD>(fX * fX + fY * fY);
}

void CInstanceBase::SetRank(BYTE bRank)
{
	m_GraphicThingInstance.SetRank(bRank);
}
#endif

#if defined(ENABLE_OPTIMIZATION)
void CInstanceBase::AttackProcessOnlySelf()
{
	if (!m_GraphicThingInstance.CanCheckAttacking())
		return;

	if (IsMainInstance())
	{
		AttackProcess();
	}
	else
	{
		CInstanceBase* pkInstMain = CPythonCharacterManager::Instance().GetMainInstancePtr();
		if (pkInstMain && IsAttackableInstance(*pkInstMain))
			CheckAttacking(*pkInstMain);
	}
}
#endif

BOOL CInstanceBase::IsPoly()
{
	return m_GraphicThingInstance.IsPoly();
}

BOOL CInstanceBase::IsPC()
{
	return m_GraphicThingInstance.IsPC();
}

BOOL CInstanceBase::IsNPC()
{
	return m_GraphicThingInstance.IsNPC();
}

#if defined(ENABLE_GROWTH_PET_SYSTEM)
BOOL CInstanceBase::IsGrowthPet()
{
	return m_GraphicThingInstance.IsGrowthPet();
}
#endif

BOOL CInstanceBase::IsEnemy()
{
	return m_GraphicThingInstance.IsEnemy();
}

BOOL CInstanceBase::IsStone()
{
	return m_GraphicThingInstance.IsStone();
}

BOOL CInstanceBase::IsHorse()
{
	return m_GraphicThingInstance.IsHorse();
}

BOOL CInstanceBase::IsPet()
{
	return m_GraphicThingInstance.IsPet();
}

BOOL CInstanceBase::IsPetPay()
{
	return m_GraphicThingInstance.IsPetPay();
}

BOOL CInstanceBase::IsShop()
{
	if (GetRace() >= 30000 && GetRace() <= 30009)
		return true;
	return false;
}

BOOL CInstanceBase::IsGuildWall()
{
	return IsWall(m_dwRace);
}

BOOL CInstanceBase::IsResource()
{
	switch (m_dwVirtualNumber)
	{
		case 20047:
		case 20048:
		case 20049:
		case 20050:
		case 20051:
		case 20052:
		case 20053:
		case 20054:
		case 20055:
		case 20056:
		case 20057:
		case 20058:
		case 20059:
		case 30301:
		case 30302:
		case 30303:
		case 30304:
		case 30305:
		case 30306:
		case 30309:
		case 30310:
		case 30311:
		case 30312:
		case 30313:
			return TRUE;
	}

	return FALSE;
}

BOOL CInstanceBase::IsWarp()
{
	return m_GraphicThingInstance.IsWarp();
}

BOOL CInstanceBase::IsGoto()
{
	return m_GraphicThingInstance.IsGoto();
}

BOOL CInstanceBase::IsObject()
{
	return m_GraphicThingInstance.IsObject();
}

BOOL CInstanceBase::IsBuilding()
{
	return m_GraphicThingInstance.IsBuilding();
}

BOOL CInstanceBase::IsDoor()
{
	return m_GraphicThingInstance.IsDoor();
}

BOOL CInstanceBase::IsWoodenDoor()
{
	if (m_GraphicThingInstance.IsDoor())
	{
		int vnum = GetVirtualNumber();
		if (vnum == 13000)
			return true;
		else if (vnum >= 30111 && vnum <= 30119)
			return true;
		else
			return false;
	}
	else
	{
		return false;
	}
}

BOOL CInstanceBase::IsStoneDoor()
{
	return m_GraphicThingInstance.IsDoor() && 13001 == GetVirtualNumber();
}

BOOL CInstanceBase::IsFlag()
{
	if (GetRace() == 20035)
		return TRUE;
	if (GetRace() == 20036)
		return TRUE;
	if (GetRace() == 20037)
		return TRUE;

	return FALSE;
}

#if defined(ENABLE_EVENT_BANNER_FLAG)
BOOL CInstanceBase::IsBannerFlag()
{
	switch (GetRace())
	{
		case 20127: // Anniversary Banner 1
		case 20128: // Anniversary Banner 2
		case 20129: // Anniversary Banner 3
		case 20130: // Anniversary Banner 4
		case 20131: // Anniversary Banner 5
		case 20132: // Anniversary Banner 6
		case 20133: // Anniversary Banner 7
		case 20134: // Solar Banner
		case 20135: // Christmas Banner
		case 20138: // Ramadan Banner
		case 20142: // Halloween Banner
		case 20143: // Easter Banner
			return TRUE;
	}
	return FALSE;
}
#endif

BOOL CInstanceBase::IsForceVisible()
{
	if (IsAffect(AFFECT_SHOW_ALWAYS))
		return TRUE;

	if (IsObject() || IsBuilding() || IsDoor())
		return TRUE;

	return FALSE;
}

int CInstanceBase::GetInstanceType()
{
	return m_GraphicThingInstance.GetActorType();
}

DWORD CInstanceBase::GetVirtualID()
{
	return m_GraphicThingInstance.GetVirtualID();
}

DWORD CInstanceBase::GetVirtualNumber()
{
	return m_dwVirtualNumber;
}

// 2004.07.17.levites.isShow?? ViewFrustumCheck?? ????
bool CInstanceBase::__IsInViewFrustum()
{
	return m_GraphicThingInstance.isShow();
}

bool CInstanceBase::__CanRender()
{
#if defined(RENDER_TARGET)
	if (m_bIsAlwaysRender)
		return true;
#endif

	if (!__IsInViewFrustum())
		return false;

	if (IsAffect(AFFECT_INVISIBILITY))
		return false;

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Graphic Control

bool CInstanceBase::IntersectBoundingBox()
{
	float u, v, t;
	return m_GraphicThingInstance.Intersect(&u, &v, &t);
}

bool CInstanceBase::IntersectDefendingSphere()
{
	return m_GraphicThingInstance.IntersectDefendingSphere();
}

float CInstanceBase::GetDistance(CInstanceBase* pkTargetInst)
{
	TPixelPosition TargetPixelPosition;
	pkTargetInst->m_GraphicThingInstance.GetPixelPosition(&TargetPixelPosition);
	return GetDistance(TargetPixelPosition);
}

float CInstanceBase::GetDistance(const TPixelPosition& c_rPixelPosition)
{
	TPixelPosition PixelPosition;
	m_GraphicThingInstance.GetPixelPosition(&PixelPosition);

	float fdx = PixelPosition.x - c_rPixelPosition.x;
	float fdy = PixelPosition.y - c_rPixelPosition.y;

	return sqrtf((fdx * fdx) + (fdy * fdy));
}

float CInstanceBase::GetBaseHeight()
{
	CActorInstance* pkHorse = m_kHorse.GetActorPtr();
	if (!m_kHorse.IsMounting() || !pkHorse)
		return 0.0f;

	DWORD dwHorseVnum = m_kHorse.m_pkActor->GetRace();
	if ((dwHorseVnum >= 20101 && dwHorseVnum <= 20109) ||
		(dwHorseVnum == 20029 || dwHorseVnum == 20030))
		return 100.0f;

#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	if (dwHorseVnum >= 20149 && dwHorseVnum <= 20151)
		return 100.0f;
#endif

	float fRaceHeight = CRaceManager::Instance().GetRaceHeight(dwHorseVnum);
	if (fRaceHeight == 0.0f)
		return 100.0f;
	else
		return fRaceHeight;
}

CActorInstance& CInstanceBase::GetGraphicThingInstanceRef()
{
	return m_GraphicThingInstance;
}

CActorInstance* CInstanceBase::GetGraphicThingInstancePtr()
{
	return &m_GraphicThingInstance;
}

void CInstanceBase::RefreshActorInstance()
{
	m_GraphicThingInstance.RefreshActorInstance();
}

void CInstanceBase::Refresh(DWORD dwMotIndex, bool isLoop)
{
	RefreshState(dwMotIndex, isLoop);
}

void CInstanceBase::RestoreRenderMode()
{
	m_GraphicThingInstance.RestoreRenderMode();
}

void CInstanceBase::SetAddRenderMode()
{
	m_GraphicThingInstance.SetAddRenderMode();
}

void CInstanceBase::SetModulateRenderMode()
{
	m_GraphicThingInstance.SetModulateRenderMode();
}

void CInstanceBase::SetRenderMode(int iRenderMode)
{
	m_GraphicThingInstance.SetRenderMode(iRenderMode);
}

void CInstanceBase::SetAddColor(const D3DXCOLOR& c_rColor)
{
	m_GraphicThingInstance.SetAddColor(c_rColor);
}

void CInstanceBase::__SetBlendRenderingMode()
{
	m_GraphicThingInstance.SetBlendRenderMode();
}

void CInstanceBase::__SetAlphaValue(float fAlpha)
{
	m_GraphicThingInstance.SetAlphaValue(fAlpha);
}

float CInstanceBase::__GetAlphaValue()
{
	return m_GraphicThingInstance.GetAlphaValue();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Part

void CInstanceBase::SetHair(DWORD eHair)
{
	if (!HAIR_COLOR_ENABLE)
		return;

	if (IsPC() == false)
		return;

	m_adwPart[CRaceData::PART_HAIR] = eHair;
	m_GraphicThingInstance.SetHair(eHair);
}

void CInstanceBase::ChangeHair(DWORD eHair)
{
	if (!HAIR_COLOR_ENABLE)
		return;

	if (IsPC() == false)
		return;

	if (GetPart(CRaceData::PART_HAIR) == eHair)
		return;

	SetHair(eHair);

	//int type = m_GraphicThingInstance.GetMotionMode();

	RefreshState(CRaceMotionData::NAME_WAIT, true);
	//RefreshState(type, true);
}

#if defined(ENABLE_ACCE_COSTUME_SYSTEM)
bool CInstanceBase::SetAcce(DWORD eAcce)
{
	if (IsPoly())
		return false;

	if (__IsShapeAnimalWear())
		return false;

	m_GraphicThingInstance.AttachAcce(eAcce, 0, CRaceData::PART_ACCE);

	if (eAcce)
	{
		CItemData* pItemData;
		if (CItemManager::Instance().GetItemDataPointer(eAcce, &pItemData))
		{
			if (m_acceRefineEffect)
				m_acceRefineEffect = __AttachEffect(EFFECT_ACCE_BACK);

			BYTE bRace = (BYTE)GetRace();
			BYTE bJob = (BYTE)RaceToJob(bRace);
			BYTE bSex = (BYTE)RaceToSex(bRace);

			m_GraphicThingInstance.SetAcceScale(pItemData->GetItemScaleVector(bJob, bSex), bRace);
			m_adwPart[CRaceData::PART_ACCE] = eAcce;
		}
		else
		{
			__ClearAcceRefineEffect();
			m_adwPart[CRaceData::PART_ACCE] = 0;
		}
	}
	else
	{
		__ClearAcceRefineEffect();
		m_adwPart[CRaceData::PART_ACCE] = 0;
	}

	return true;
}

void CInstanceBase::ChangeAcce(DWORD eAcce)
{
	if (eAcce == m_GraphicThingInstance.GetPartItemID(CRaceData::PART_ACCE))
	{
		if (!eAcce)
			__ClearAcceRefineEffect();
	}
	else
	{
		if (SetAcce(eAcce))
			RefreshState(CRaceMotionData::NAME_WAIT, true);
	}
}

void CInstanceBase::__ClearAcceRefineEffect()
{
	if (m_acceRefineEffect)
	{
		__DetachEffect(m_acceRefineEffect);
		m_acceRefineEffect = 0;
	}
}
#endif

#if defined(ENABLE_AURA_COSTUME_SYSTEM)
void CInstanceBase::ChangeAura(DWORD eAura)
{
	if (m_GraphicThingInstance.GetPartItemID(CRaceData::PART_AURA) != eAura)
		SetAura(eAura);
}

bool CInstanceBase::SetAura(DWORD eAura)
{
	if (IsPoly() || __IsShapeAnimalWear())
		return false;

	m_GraphicThingInstance.ChangePart(CRaceData::PART_AURA, eAura);
	if (!eAura)
	{
		if (m_auraRefineEffect)
		{
			__DetachEffect(m_auraRefineEffect);
			m_auraRefineEffect = 0;
		}
		m_adwPart[CRaceData::PART_AURA] = 0;
		return true;
	}

	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(eAura, &pItemData))
	{
		if (m_auraRefineEffect)
		{
			__DetachEffect(m_auraRefineEffect);
			m_auraRefineEffect = 0;
		}
		m_adwPart[CRaceData::PART_AURA] = 0;
		return true;
	}

	BYTE byRace = (BYTE)GetRace();
	BYTE byJob = (BYTE)RaceToJob(byRace);
	BYTE bySex = (BYTE)RaceToSex(byRace);

	D3DXVECTOR3 v3MeshScale = pItemData->GetItemScaleVector(byJob, bySex);
	float fParticleScale = pItemData->GetItemParticleScale(byJob, bySex);

	m_auraRefineEffect = m_GraphicThingInstance.AttachEffectByID(0, "Bip01 Spine2", pItemData->GetAuraEffectID(), NULL, fParticleScale, &v3MeshScale);
	m_adwPart[CRaceData::PART_AURA] = eAura;

	return true;
}
#endif

void CInstanceBase::SetArmor(DWORD dwArmor)
{
	DWORD dwShape;
	if (__ArmorVnumToShape(dwArmor, &dwShape))
	{
		CItemData* pItemData;
		if (CItemManager::Instance().GetItemDataPointer(dwArmor, &pItemData))
		{
			float fSpecularPower = pItemData->GetSpecularPowerf();
			SetShape(dwShape, fSpecularPower);
			__GetRefinedEffect(pItemData);
			return;
		}
		else
		{
			__ClearArmorRefineEffect();
		}
	}

	SetShape(dwArmor);
}

void CInstanceBase::SetShape(DWORD eShape, float fSpecular)
{
	if (IsPoly())
	{
		m_GraphicThingInstance.SetShape(0);
	}
	else
	{
		m_GraphicThingInstance.SetShape(eShape, fSpecular);
	}

	m_eShape = eShape;
}

DWORD CInstanceBase::GetWeaponType()
{
	DWORD dwWeapon = GetPart(CRaceData::PART_WEAPON);
	CItemData* pItemData;
	if (!CItemManager::Instance().GetItemDataPointer(dwWeapon, &pItemData))
		return CItemData::WEAPON_NONE;

	return pItemData->GetWeaponType();
}

bool CInstanceBase::IsUsingFishingRod()
{
	CItemData* pItemData;
	if (CItemManager::Instance().GetItemDataPointer(m_adwPart[CRaceData::PART_WEAPON], &pItemData))
		if (pItemData->GetType() == CItemData::ITEM_TYPE_ROD)
			return true;
	return false;
}

/*
void CInstanceBase::SetParts(const WORD * c_pParts)
{
	if (IsPoly())
		return;

	if (__IsShapeAnimalWear())
		return;

	UINT eWeapon=c_pParts[CRaceData::PART_WEAPON];

	if (__IsChangableWeapon(eWeapon) == false)
			eWeapon = 0;

	if (eWeapon != m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON))
	{
		m_GraphicThingInstance.AttachPart(CRaceData::PART_MAIN, CRaceData::PART_WEAPON, eWeapon);
		m_adwPart[CRaceData::PART_WEAPON] = eWeapon;
	}

	__AttachHorseSaddle();
}
*/

void CInstanceBase::__ClearWeaponRefineEffect()
{
	if (m_swordRefineEffectRight)
	{
		__DetachEffect(m_swordRefineEffectRight);
		m_swordRefineEffectRight = 0;
	}

	if (m_swordRefineEffectLeft)
	{
		__DetachEffect(m_swordRefineEffectLeft);
		m_swordRefineEffectLeft = 0;
	}

#if defined(ENABLE_REFINE_ELEMENT_SYSTEM)
	if (m_dwSwordRefineElementEffectRight)
	{
		__DetachEffect(m_dwSwordRefineElementEffectRight);
		m_dwSwordRefineElementEffectRight = 0;
	}

	if (m_dwSwordRefineElementEffectLeft)
	{
		__DetachEffect(m_dwSwordRefineElementEffectLeft);
		m_dwSwordRefineElementEffectLeft = 0;
	}
#endif
}

void CInstanceBase::__ClearArmorRefineEffect()
{
	if (m_armorRefineEffect)
	{
		__DetachEffect(m_armorRefineEffect);
		m_armorRefineEffect = 0;
	}
}

UINT CInstanceBase::__GetRefinedEffect(CItemData* pItem)
{
	DWORD refine = max(pItem->GetRefine() + pItem->GetSocketCount(), ITEM_METIN_SOCKET_MAX) - ITEM_METIN_SOCKET_MAX;

	switch (pItem->GetType())
	{
		case CItemData::ITEM_TYPE_WEAPON:
		{
			__ClearWeaponRefineEffect();

			if (refine < 7)
				return 0;

			refine = MIN(refine, 10);

#if defined(ENABLE_REFINE_ELEMENT_SYSTEM)
			bool bRefineElementEffect = true;
			DWORD dwRefineElementEffect = 0;
			switch (m_wRefineElementAffectType)
			{
				case CItemData::APPLY_ENCHANT_ELECT:
					dwRefineElementEffect = EFFECT_REFINED_ELEMENT + REFINE_ELEMENT_CATEGORY_ELECT * EFFECT_REFINE_ELEMENT_WEAPON_MAX;
					break;
				case CItemData::APPLY_ENCHANT_FIRE:
					dwRefineElementEffect = EFFECT_REFINED_ELEMENT + REFINE_ELEMENT_CATEGORY_FIRE * EFFECT_REFINE_ELEMENT_WEAPON_MAX;
					break;
				case CItemData::APPLY_ENCHANT_ICE:
					dwRefineElementEffect = EFFECT_REFINED_ELEMENT + REFINE_ELEMENT_CATEGORY_ICE * EFFECT_REFINE_ELEMENT_WEAPON_MAX;
					break;
				case CItemData::APPLY_ENCHANT_WIND:
					dwRefineElementEffect = EFFECT_REFINED_ELEMENT + REFINE_ELEMENT_CATEGORY_WIND * EFFECT_REFINE_ELEMENT_WEAPON_MAX;
					break;
				case CItemData::APPLY_ENCHANT_EARTH:
					dwRefineElementEffect = EFFECT_REFINED_ELEMENT + REFINE_ELEMENT_CATEGORY_EARTH * EFFECT_REFINE_ELEMENT_WEAPON_MAX;
					break;
				case CItemData::APPLY_ENCHANT_DARK:
					dwRefineElementEffect = EFFECT_REFINED_ELEMENT + REFINE_ELEMENT_CATEGORY_DARK * EFFECT_REFINE_ELEMENT_WEAPON_MAX;
					break;
				default:
					bRefineElementEffect = false;
					break;
			}
#endif

			switch (pItem->GetSubType())
			{
				case CItemData::WEAPON_DAGGER:
				{
					m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7 + refine - 7;
					m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7_LEFT + refine - 7;
#if defined(ENABLE_REFINE_ELEMENT_SYSTEM)
					m_dwSwordRefineElementEffectRight = bRefineElementEffect != 0 ? dwRefineElementEffect + EFFECT_REFINE_ELEMENT_WEAPON_DAGGER : 0;
					m_dwSwordRefineElementEffectLeft = bRefineElementEffect != 0 ? dwRefineElementEffect + EFFECT_REFINE_ELEMENT_WEAPON_DAGGER : 0;
#endif
				}
				break;

				case CItemData::WEAPON_BOW:
				{
					m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_BOW_REFINED7 + refine - 7;
#if defined(ENABLE_REFINE_ELEMENT_SYSTEM)
					m_dwSwordRefineElementEffectLeft = bRefineElementEffect != 0 ? dwRefineElementEffect + EFFECT_REFINE_ELEMENT_WEAPON_BOW : 0;
#endif
				}
				break;

				case CItemData::WEAPON_BELL:
				case CItemData::WEAPON_ARROW:
#if defined(ENABLE_QUIVER_SYSTEM)
				case CItemData::WEAPON_QUIVER:
#endif
				{
					m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SMALLSWORD_REFINED7 + refine - 7;
#if defined(ENABLE_REFINE_ELEMENT_SYSTEM)
					m_dwSwordRefineElementEffectRight = bRefineElementEffect != 0 ? dwRefineElementEffect + EFFECT_REFINE_ELEMENT_WEAPON_DAGGER : 0;
#endif
				}
				break;

				case CItemData::WEAPON_FAN:
				{
					m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_FANBELL_REFINED7 + refine - 7;
#if defined(ENABLE_REFINE_ELEMENT_SYSTEM)
					m_dwSwordRefineElementEffectRight = bRefineElementEffect != 0 ? dwRefineElementEffect + EFFECT_REFINE_ELEMENT_WEAPON_FAN : 0;
#endif
				}
				break;

				case CItemData::WEAPON_CLAW:
				{
					m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_CLAW_REFINED7 + refine - 7;
					m_swordRefineEffectLeft = EFFECT_REFINED + EFFECT_CLAW_REFINED7_LEFT + refine - 7;
#if defined(ENABLE_REFINE_ELEMENT_SYSTEM)
					m_dwSwordRefineElementEffectRight = bRefineElementEffect != 0 ? dwRefineElementEffect + EFFECT_REFINE_ELEMENT_WEAPON_CLAW : 0;
					m_dwSwordRefineElementEffectLeft = bRefineElementEffect != 0 ? dwRefineElementEffect + EFFECT_REFINE_ELEMENT_WEAPON_CLAW : 0;
#endif
				}
				break;

				default:
				{
					m_swordRefineEffectRight = EFFECT_REFINED + EFFECT_SWORD_REFINED7 + refine - 7;
#if defined(ENABLE_REFINE_ELEMENT_SYSTEM)
					m_dwSwordRefineElementEffectRight = bRefineElementEffect != 0 ? dwRefineElementEffect + EFFECT_REFINE_ELEMENT_WEAPON_SWORD : 0;
#endif
				}
				break;
			}

			if (m_swordRefineEffectRight)
				m_swordRefineEffectRight = __AttachEffect(m_swordRefineEffectRight);

			if (m_swordRefineEffectLeft)
				m_swordRefineEffectLeft = __AttachEffect(m_swordRefineEffectLeft);

#if defined(ENABLE_REFINE_ELEMENT_SYSTEM)
			if (m_dwSwordRefineElementEffectRight)
				m_dwSwordRefineElementEffectRight = __AttachBoneEffect(CRaceData::PART_WEAPON, m_dwSwordRefineElementEffectRight);

			if (m_dwSwordRefineElementEffectLeft)
				m_dwSwordRefineElementEffectLeft = __AttachBoneEffect(CRaceData::PART_WEAPON_LEFT, m_dwSwordRefineElementEffectLeft);
#endif
		}
		break;

		case CItemData::ITEM_TYPE_ARMOR:
		{
			__ClearArmorRefineEffect();

			if (pItem->GetSubType() == CItemData::ARMOR_BODY)
			{
				DWORD vnum = pItem->GetIndex();

				if (vnum >= 12010 && vnum <= 12049 || vnum >= 21080 && vnum <= 21089)
				{
					__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL);
					__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL2);
				}

				if (vnum >= 20760 && vnum <= 20959)
				{
					__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL3);
				}

				if (vnum >= 12050 && vnum <= 12099)
				{
					__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_SPECIAL4);
				}

				// 7th
				if (vnum > 21219 && vnum <= 21225 ||
					vnum > 21239 && vnum <= 21245 ||
					vnum > 21259 && vnum <= 21265 ||
					vnum > 21279 && vnum <= 21285 ||
					vnum > 21299 && vnum <= 21305)
				{
					__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_REFINED10);
				}

				// Serpent Armour
				if (vnum > 21319 && vnum <= 21325 ||
					vnum > 21339 && vnum <= 21345 ||
					vnum > 21359 && vnum <= 21365 ||
					vnum > 21379 && vnum <= 21385 ||
					vnum > 21399 && vnum <= 21405)
				{
					__AttachEffect(EFFECT_REFINED + EFFECT_BODYARMOR_REFINED10);
				}
			}

			if (refine < 7)
				return 0;

			refine = MIN(refine, 10);

			if (pItem->GetSubType() == CItemData::ARMOR_BODY)
			{
				m_armorRefineEffect = EFFECT_REFINED + EFFECT_BODYARMOR_REFINED7 + refine - 7;
				__AttachEffect(m_armorRefineEffect);
			}
		}
		break;

#if defined(ENABLE_COSTUME_SYSTEM)
		case CItemData::ITEM_TYPE_COSTUME:
		{
			if (pItem->GetSubType() == CItemData::COSTUME_BODY)
				__ClearArmorRefineEffect();

#if defined(ENABLE_WEAPON_COSTUME_SYSTEM)
			if (pItem->GetSubType() == CItemData::COSTUME_WEAPON)
				__ClearWeaponRefineEffect();
#endif
		}
		break;
#endif
	}

	return 0;
}

bool CInstanceBase::SetWeapon(DWORD eWeapon)
{
	if (IsPoly())
		return false;

	if (__IsShapeAnimalWear())
		return false;

	if (__IsChangableWeapon(eWeapon) == false)
		eWeapon = 0;

	m_GraphicThingInstance.AttachWeapon(eWeapon);
	m_adwPart[CRaceData::PART_WEAPON] = eWeapon;

	// Weapon Effect
	CItemData* pItemData;
	if (CItemManager::Instance().GetItemDataPointer(eWeapon, &pItemData))
	{
		__GetRefinedEffect(pItemData);
	}
	else
	{
		__ClearWeaponRefineEffect();
	}

	return true;
}

#if defined(ENABLE_QUIVER_SYSTEM)
bool CInstanceBase::SetArrow(DWORD eArrow)
{
	if (IsPoly())
		return false;

	if (__IsShapeAnimalWear())
		return false;

	if (eArrow)
	{
		CItemData* pItemData;
		if (!CItemManager::Instance().GetItemDataPointer(eArrow, &pItemData))
			return false;

		if (pItemData->GetType() == CItemData::ITEM_TYPE_WEAPON)
		{
			if (pItemData->GetSubType() == CItemData::WEAPON_ARROW)
			{
				m_GraphicThingInstance.SetQuiverEquipped(false);
				m_GraphicThingInstance.SetQuiverEffectID(0);
				return true;
			}

			if (pItemData->GetSubType() == CItemData::WEAPON_QUIVER)
			{
				m_GraphicThingInstance.SetQuiverEquipped(true);
				m_GraphicThingInstance.SetQuiverEffectID(pItemData->GetValue(0));
				return true;
			}
		}
	}

	m_GraphicThingInstance.SetQuiverEquipped(false);
	m_GraphicThingInstance.SetQuiverEffectID(0);
	return false;
}
#endif

#if defined(ENABLE_REFINE_ELEMENT_SYSTEM)
void CInstanceBase::SetRefineElement(WORD wAffectType)
{
	m_wRefineElementAffectType = wAffectType;
}
#endif

void CInstanceBase::ChangeWeapon(DWORD eWeapon)
{
	if (eWeapon == m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON))
		return;

	if (SetWeapon(eWeapon))
		RefreshState(CRaceMotionData::NAME_WAIT, true);
}

bool CInstanceBase::ChangeArmor(DWORD dwArmor)
{
	DWORD eShape;
	__ArmorVnumToShape(dwArmor, &eShape);

	if (GetShape() == eShape)
		return false;

	CAffectFlagContainer kAffectFlagContainer;
	kAffectFlagContainer.CopyInstance(m_kAffectFlagContainer);

	DWORD dwVID = GetVirtualID();
	DWORD dwRace = GetRace();
	DWORD eHair = GetPart(CRaceData::PART_HAIR);
	DWORD eWeapon = GetPart(CRaceData::PART_WEAPON);
#if defined(ENABLE_ACCE_COSTUME_SYSTEM)
	DWORD eAcce = GetPart(CRaceData::PART_ACCE);
#endif
#if defined(ENABLE_AURA_COSTUME_SYSTEM)
	DWORD eAura = GetPart(CRaceData::PART_AURA);
#endif

	float fRot = GetRotation();
	float fAdvRot = GetAdvancingRotation();

	if (IsWalking())
		EndWalking();

	//////////////////////////////////////////////////////
	__ClearAffects();
	//////////////////////////////////////////////////////

	if (!SetRace(dwRace))
	{
		TraceError("CPythonCharacterManager::ChangeArmor - SetRace VID[%d] Race[%d] ERROR", dwVID, dwRace);
		return false;
	}

	SetArmor(dwArmor);
	SetHair(eHair);
	SetWeapon(eWeapon);
#if defined(ENABLE_ACCE_COSTUME_SYSTEM)
	SetAcce(eAcce);
#endif
#if defined(ENABLE_AURA_COSTUME_SYSTEM)
	SetAura(eAura);
#endif

	SetRotation(fRot);
	SetAdvancingRotation(fAdvRot);

	__AttachHorseSaddle();

	RefreshState(CRaceMotionData::NAME_WAIT, TRUE);

	/////////////////////////////////////////////////
	SetAffectFlagContainer(kAffectFlagContainer);
	/////////////////////////////////////////////////

	CActorInstance::IEventHandler& rkEventHandler = GetEventHandlerRef();
	rkEventHandler.OnChangeShape();

	return true;
}

bool CInstanceBase::__IsShapeAnimalWear()
{
	if (100 == GetShape() ||
		101 == GetShape() ||
		102 == GetShape() ||
		103 == GetShape())
		return true;

	return false;
}

DWORD CInstanceBase::__GetRaceType()
{
	return m_eRaceType;
}

void CInstanceBase::RefreshState(DWORD dwMotIndex, bool isLoop)
{
	DWORD dwPartItemID = m_GraphicThingInstance.GetPartItemID(CRaceData::PART_WEAPON);

	BYTE byItemType = 0xff;
	BYTE bySubType = 0xff;

	CItemManager& rkItemMgr = CItemManager::Instance();
	CItemData* pItemData;

	if (rkItemMgr.GetItemDataPointer(dwPartItemID, &pItemData))
	{
		byItemType = pItemData->GetType();
		bySubType = pItemData->GetWeaponType();
	}

	if (IsPoly())
	{
		SetMotionMode(CRaceMotionData::MODE_GENERAL);
	}
	else if (IsWearingDress())
	{
		SetMotionMode(CRaceMotionData::MODE_WEDDING_DRESS);
	}
	else if (byItemType == CItemData::ITEM_TYPE_PICK) //if (IsHoldingPickAxe())
	{
		if (m_kHorse.IsMounting())
		{
			SetMotionMode(CRaceMotionData::MODE_HORSE);
		}
		else
		{
			SetMotionMode(CRaceMotionData::MODE_GENERAL);
		}
	}
	else if (byItemType == CItemData::ITEM_TYPE_ROD) //if (IsHoldingFishingRod())
	{
		if (m_kHorse.IsMounting())
		{
			SetMotionMode(CRaceMotionData::MODE_HORSE);
		}
		else
		{
			SetMotionMode(CRaceMotionData::MODE_FISHING);
		}
	}
	else if (m_kHorse.IsMounting())
	{
		switch (bySubType)
		{
			case CItemData::WEAPON_SWORD:
				SetMotionMode(CRaceMotionData::MODE_HORSE_ONEHAND_SWORD);
				break;

			case CItemData::WEAPON_TWO_HANDED:
				SetMotionMode(CRaceMotionData::MODE_HORSE_TWOHAND_SWORD); // Only Warrior
				break;

			case CItemData::WEAPON_DAGGER:
				SetMotionMode(CRaceMotionData::MODE_HORSE_DUALHAND_SWORD); // Only Assassin
				break;

			case CItemData::WEAPON_FAN:
				SetMotionMode(CRaceMotionData::MODE_HORSE_FAN); // Only Shaman
				break;

			case CItemData::WEAPON_BELL:
				SetMotionMode(CRaceMotionData::MODE_HORSE_BELL); // Only Shaman
				break;

			case CItemData::WEAPON_BOW:
				SetMotionMode(CRaceMotionData::MODE_HORSE_BOW); // Only Shaman
				break;

			case CItemData::WEAPON_CLAW:
				SetMotionMode(CRaceMotionData::MODE_HORSE_CLAW); // Only Wolfman
				break;

			default:
				SetMotionMode(CRaceMotionData::MODE_HORSE);
				break;
		}
	}
	else
	{
		switch (bySubType)
		{
			case CItemData::WEAPON_SWORD:
				SetMotionMode(CRaceMotionData::MODE_ONEHAND_SWORD);
				break;

			case CItemData::WEAPON_TWO_HANDED:
				SetMotionMode(CRaceMotionData::MODE_TWOHAND_SWORD); // Only Warrior
				break;

			case CItemData::WEAPON_DAGGER:
				SetMotionMode(CRaceMotionData::MODE_DUALHAND_SWORD); // Only Assassin
				break;

			case CItemData::WEAPON_BOW:
				SetMotionMode(CRaceMotionData::MODE_BOW); // Only Assassin
				break;

			case CItemData::WEAPON_FAN:
				SetMotionMode(CRaceMotionData::MODE_FAN); // Only Shaman
				break;

			case CItemData::WEAPON_BELL:
				SetMotionMode(CRaceMotionData::MODE_BELL); // Only Shaman
				break;

			case CItemData::WEAPON_CLAW:
				SetMotionMode(CRaceMotionData::MODE_CLAW); // Only Wolfman
				break;

			case CItemData::WEAPON_ARROW:
			default:
				SetMotionMode(CRaceMotionData::MODE_GENERAL);
				break;
		}
	}

	if (isLoop)
		m_GraphicThingInstance.InterceptLoopMotion(dwMotIndex);
	else
		m_GraphicThingInstance.InterceptOnceMotion(dwMotIndex);

	RefreshActorInstance();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Device

void CInstanceBase::RegisterBoundingSphere()
{
	if (!IsStone())
	{
		m_GraphicThingInstance.DeformNoSkin();
	}

	m_GraphicThingInstance.RegisterBoundingSphere();
}

bool CInstanceBase::CreateDeviceObjects()
{
	return m_GraphicThingInstance.CreateDeviceObjects();
}

void CInstanceBase::DestroyDeviceObjects()
{
	m_GraphicThingInstance.DestroyDeviceObjects();
}

void CInstanceBase::Destroy()
{
	DetachTextTail();

	DismountHorse();

	m_kQue_kCmdNew.clear();

	__EffectContainer_Destroy();
	__StoneSmoke_Destroy();

	if (__IsMainInstance())
		__ClearMainInstance();

	m_GraphicThingInstance.Destroy();

	__Initialize();
}

void CInstanceBase::__InitializeRotationSpeed()
{
	SetRotationSpeed(c_fDefaultRotationSpeed);
}

void CInstanceBase::__Warrior_Initialize()
{
	m_kWarrior.m_dwGeomgyeongEffect = 0;
}

void CInstanceBase::__Initialize()
{
	__Warrior_Initialize();
	__StoneSmoke_Inialize();
	__EffectContainer_Initialize();
	__InitializeRotationSpeed();

	SetEventHandler(CActorInstance::IEventHandler::GetEmptyPtr());

	m_kAffectFlagContainer.Clear();

	m_dwLevel = 0;
#if defined(ENABLE_CONQUEROR_LEVEL)
	m_dwConquerorLevel = 0;
#endif
#ifdef WJ_SHOW_MOB_INFO
	m_dwAIFlag = 0;
#endif
	m_dwEmpireID = 0;
	m_dwGuildID = 0;
#if defined(ENABLE_GUILD_LEADER_GRADE_NAME)
	m_bGuildLeaderGrade = 0;
#endif
#if defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
	m_stCountry = "";
#endif
#if defined(ENABLE_LEFT_SEAT)
	m_bLeftSeat = false;
#endif

	m_eType = 0;
	m_eRaceType = 0;
	m_eShape = 0;
	m_dwRace = 0;

	m_dwVirtualNumber = 0;

	m_dwBaseCmdTime = 0;
	m_dwBaseChkTime = 0;
	m_dwSkipTime = 0;

	m_GraphicThingInstance.Initialize();

	m_dwAdvActorVID = 0;
	m_dwLastDmgActorVID = 0;

	m_nAverageNetworkGap = 0;
	m_dwNextUpdateHeightTime = 0;

	// Moving by keyboard
	m_iRotatingDirection = DEGREE_DIRECTION_SAME;

	// Moving by mouse
	m_isTextTail = FALSE;
	m_isGoing = FALSE;
	NEW_SetSrcPixelPosition(TPixelPosition(0, 0, 0));
	NEW_SetDstPixelPosition(TPixelPosition(0, 0, 0));

	m_kPPosDust = TPixelPosition(0, 0, 0);

	m_kQue_kCmdNew.clear();

	m_dwLastComboIndex = 0;

	m_swordRefineEffectRight = 0;
	m_swordRefineEffectLeft = 0;
#if defined(ENABLE_REFINE_ELEMENT_SYSTEM)
	m_wRefineElementAffectType = 0;
	m_dwSwordRefineElementEffectRight = 0;
	m_dwSwordRefineElementEffectLeft = 0;
#endif
	m_armorRefineEffect = 0;
#if defined(ENABLE_ACCE_COSTUME_SYSTEM)
	m_acceRefineEffect = 0;
#endif
#if defined(ENABLE_AURA_COSTUME_SYSTEM)
	m_auraRefineEffect = 0;
#endif
#if defined(ENABLE_GUILD_DRAGONLAIR_SYSTEM)
	m_dragonLairStoneEffect = 0;
#endif
#if defined(ENABLE_SUMMER_EVENT_ROULETTE)
	m_specialRouletteEffect = 0;
#endif
#if defined(ENABLE_TITLE_SYSTEM)
	 m_dwOverHeadSpecialEffect = 0;
#endif

	m_sAlignment = 0;
	m_byPKMode = 0;
	m_isKiller = false;
	m_isPartyMember = false;

	m_bEnableTCPState = TRUE;

	m_stName = "";

	memset(m_adwPart, 0, sizeof(m_adwPart));
	memset(m_adwCRCAffectEffect, 0, sizeof(m_adwCRCAffectEffect));
	//memset(m_adwCRCEmoticonEffect, 0, sizeof(m_adwCRCEmoticonEffect));
	memset(&m_kMovAfterFunc, 0, sizeof(m_kMovAfterFunc));

	m_bDamageEffectType = false;
	m_dwDuelMode = DUEL_NONE;
	m_dwEmoticonTime = 0;
#if defined(ENABLE_MONSTER_CARD)
	m_pInterfaceModel = nullptr;
	m_textTailDisable = false;
#endif
#if defined(RENDER_TARGET)
	m_bIsAlwaysRender = false;
#endif
}

#if defined(ENABLE_MONSTER_CARD)
void CInstanceBase::EnableAlwaysRender()
{
#if defined(RENDER_TARGET)
	SetAlwaysRender(true);
#endif
}

void CInstanceBase::DisableAlwaysRender()
{
#if defined(RENDER_TARGET)
	SetAlwaysRender(false);
#endif
}
#endif

CInstanceBase::CInstanceBase()
{
	__Initialize();
}

CInstanceBase::~CInstanceBase()
{
	Destroy();
}

#if defined(RENDER_TARGET)
void CInstanceBase::SetLODLimits(DWORD dwModelIndex, float fFarLOD)
{
	CGrannyLODController* pLODCtrl = m_GraphicThingInstance.GetLODControllerPointer(dwModelIndex);
	if (!pLODCtrl)
		return;

	pLODCtrl->SetLODLimits(0.0f, fFarLOD);
}

void CInstanceBase::SetAlwaysRender(bool val)
{
	m_bIsAlwaysRender = val;
	m_GraphicThingInstance.SetAlwaysRender(val);
}
#endif

void CInstanceBase::GetBoundBox(D3DXVECTOR3* vtMin, D3DXVECTOR3* vtMax)
{
	m_GraphicThingInstance.GetBoundBox(vtMin, vtMax);
}

#if defined(ENABLE_METINSTONE_SWAP)
bool CInstanceBase::ChangeStoneShape(DWORD dwRace, int iShape)
{
	m_dwRace = dwRace;

	if (!m_GraphicThingInstance.SetRace(dwRace))
		return false;

	m_GraphicThingInstance.SetShape(iShape);

	DWORD eSmoke;
	if (m_kAffectFlagContainer.IsSet(STONE_SMOKE8))
		eSmoke = 3;
	else if (m_kAffectFlagContainer.IsSet(STONE_SMOKE5) || m_kAffectFlagContainer.IsSet(STONE_SMOKE6) || m_kAffectFlagContainer.IsSet(STONE_SMOKE7))
		eSmoke = 2;
	else if (m_kAffectFlagContainer.IsSet(STONE_SMOKE2) || m_kAffectFlagContainer.IsSet(STONE_SMOKE3) || m_kAffectFlagContainer.IsSet(STONE_SMOKE4))
		eSmoke = 1;
	else
		eSmoke = 0;

	__StoneSmoke_Destroy();
	__StoneSmoke_Create(eSmoke);

	m_GraphicThingInstance.RefreshActorInstance();
	Refresh(CRaceMotionData::NAME_SPAWN, false);

	return true;
}

void CInstanceBase::SetColorOperation(DWORD dwColor)
{
	SetModulateRenderMode();
	SetAddColor(dwColor);
}

void CInstanceBase::ResetRaceDataStoneShapeEvent()
{
	m_GraphicThingInstance.ResetRaceDataStoneShapeEvent();
}

bool CInstanceBase::SetRaceDataStoneShapeEvent(DWORD dwRace)
{
	return m_GraphicThingInstance.SetRaceDataStoneShapeEvent(dwRace);
}
#endif

#if defined(ENABLE_AUTO_SYSTEM)
DWORD CInstanceBase::FindNearestVictimVID(DWORD dwCurrentVID)
{
	// int VICTIM_TARGET_SELECT_RANGE_MAX = 0;
	// CPythonPlayer& rkPlayer = CPythonPlayer::Instance();
	// if (!rkPlayer.GetAutoRangeOnOff())
		// VICTIM_TARGET_SELECT_RANGE_MAX = 30000;
	// else
	float passDistance = 0.0f;
	int VICTIM_TARGET_SELECT_RANGE_MAX = 3000;
	CPythonCharacterManager& rkChrMgr = CPythonCharacterManager::Instance();
	CInstanceBase* pkInstSelf = rkChrMgr.GetMainInstancePtr();
	typedef std::map<DWORD, int> TNearestVictimMap;
	TNearestVictimMap mapVictimsVIDDistance;
	for (CPythonCharacterManager::CharacterIterator i = rkChrMgr.CharacterInstanceBegin(); i != rkChrMgr.CharacterInstanceEnd(); ++i)
	{
		CInstanceBase* pkInstEach = *i;
		if (pkInstEach == pkInstSelf || pkInstEach == this)
			continue;
		if (pkInstEach->GetVirtualID() == dwCurrentVID)
			continue;

		int iDistance = int(pkInstEach->NEW_GetDistanceFromDestInstance(*pkInstSelf));
		if (iDistance < VICTIM_TARGET_SELECT_RANGE_MAX && (pkInstEach->__GetRaceType() == CActorInstance::TYPE_ENEMY || pkInstEach->__GetRaceType() == CActorInstance::TYPE_STONE) && pkInstEach->CanPickInstance())
		{
			CPythonPlayer& rkAutoPlayer = CPythonPlayer::Instance();
			if (pkInstSelf && !rkAutoPlayer.IsAutoHuntTargetAllowed(pkInstSelf, pkInstEach))
				continue;
			if (pkInstSelf && rkAutoPlayer.GetAutoRangeOnOff())
			{
				const float fFromStart = pkInstEach->NEW_GetDistanceFromDestPixelPosition(rkAutoPlayer.AutoHuntGetStartLocation());
				if (abs(fFromStart) >= rkAutoPlayer.GetAutoHuntFocusRadiusPx())
					continue;
			}
			CActorInstance& rkActorSelf = m_GraphicThingInstance;
			CActorInstance& rkActorEach = pkInstEach->GetGraphicThingInstanceRef();

			CPythonBackground& rkBG = CPythonBackground::Instance();
			const D3DXVECTOR3& rv3PositionSelf = rkActorSelf.GetPosition();
			const D3DXVECTOR3& rv3PositionEach = rkActorEach.GetPosition();

			int iX = 1, iY = 1;
			int iStep = 100, iRange = 50;

			float fDistanceX = rv3PositionSelf.x - rv3PositionEach.x;
			float fDistanceY = rv3PositionSelf.y - rv3PositionEach.y;

			if (rv3PositionEach.x >= rv3PositionSelf.x)
				fDistanceX = rv3PositionEach.x - rv3PositionSelf.x;
			else
				iX = -iX;

			if (rv3PositionEach.y >= rv3PositionSelf.y)
				fDistanceY = rv3PositionEach.y - rv3PositionSelf.y;
			else
				iY = -iY;

			if (fDistanceX <= fDistanceY)
				iStep = static_cast<int>(fDistanceY);
			else
				iStep = static_cast<int>(fDistanceX);

			float fRangeX = static_cast<float>(iX * iRange);
			float fRangeY = static_cast<float>(iY * iRange);
			bool bIsBlocked = false;

			for (int j = 0; j < iStep; ++j)
			{
				D3DXVECTOR3 v3CheckPosition = D3DXVECTOR3(fRangeX + rv3PositionSelf.x, fRangeY + rv3PositionSelf.y, 0.0f);
				if (rkBG.isAttrOn(v3CheckPosition.x, -v3CheckPosition.y, CTerrainImpl::ATTRIBUTE_BLOCK))
				{
					bIsBlocked = true;
					break;
				}

				bool bCheck = false;
				if (iX >= 0)
					bCheck = rv3PositionEach.x > v3CheckPosition.x;
				else
					bCheck = rv3PositionEach.x < v3CheckPosition.x;

				if (bCheck)
					fRangeX += iX * iRange;

				if (iY >= 0)
					bCheck = rv3PositionEach.y > v3CheckPosition.y;
				else
					bCheck = rv3PositionEach.y < v3CheckPosition.y;

				if (bCheck)
					fRangeY += static_cast<float>(iY * iRange);

				if (iX >= 0)
					bCheck = rv3PositionEach.x > v3CheckPosition.x;
				else
					bCheck = rv3PositionEach.x < v3CheckPosition.x;

				if (!bCheck)
				{
					if (iY >= 0)
						bCheck = rv3PositionEach.y > v3CheckPosition.y;
					else
						bCheck = rv3PositionEach.y < v3CheckPosition.y;

					if (!bCheck)
						break;
				}
			}

			if (!bIsBlocked)
				if (mapVictimsVIDDistance.find(pkInstEach->GetVirtualID()) == mapVictimsVIDDistance.end())
					mapVictimsVIDDistance.insert(TNearestVictimMap::value_type(pkInstEach->GetVirtualID(), iDistance));
		}
	}

	DWORD dwNearestVictimVID = 0;
	int iLowestDistance = VICTIM_TARGET_SELECT_RANGE_MAX;
	for (TNearestVictimMap::iterator it = mapVictimsVIDDistance.begin(); it != mapVictimsVIDDistance.end(); it++)
	{
		CInstanceBase* pkTarget = __FindInstancePtr(it->first);
		if (!pkTarget || pkTarget->IsDead() || pkTarget->GetVirtualID() == dwCurrentVID)
			continue;

		if (it->second < iLowestDistance)
		{
			dwNearestVictimVID = it->first;
			iLowestDistance = it->second;
		}
	}

	mapVictimsVIDDistance.clear();
	return dwNearestVictimVID;
}
#endif
