#include "StdAfx.h"

#include "Locale_inc.h"

#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
#	include "PythonMountUpGrade.h"
#	include "PythonNetworkStream.h"
#	include "PythonPlayer.h"
#	include "InstanceBase.h"

CPythonMountUpGrade::CPythonMountUpGrade()
{
	Reset();
	m_poHandler = nullptr;
}

CPythonMountUpGrade::~CPythonMountUpGrade()
{
	ClearHandler();
	Reset();
}

void CPythonMountUpGrade::Reset()
{
	m_HorseLevel = CPythonMountUpGrade::EMountUpGradeReset::RESET;
	m_MountUpGradeFail = CPythonMountUpGrade::EMountUpGradeReset::RESET;
	m_MountUpGradeExistingExp = CPythonMountUpGrade::EMountUpGradeReset::RESET;
	m_SkillLevelUpVnum = 0;
	m_bUIOpen = false;
}

void CPythonMountUpGrade::SetHandler(PyObject* poHandler)
{
	if (poHandler == Py_None)
		poHandler = nullptr;

	if (m_poHandler == poHandler)
		return;

	Py_XINCREF(poHandler);
	Py_CLEAR(m_poHandler);
	m_poHandler = poHandler;
}

void CPythonMountUpGrade::ClearHandler()
{
	Py_CLEAR(m_poHandler);
}

PyObject* CPythonMountUpGrade::GetHandler() const
{
	return m_poHandler;
}

void CPythonMountUpGrade::Process(const uint8_t subheader, const uint8_t horse_level, const uint8_t is_fail, const uint32_t existing_exp)
{
	SetHorseLevel(horse_level);
	SetMountUpGradeFail(is_fail);
	SetMountUpGradeExistingExp(existing_exp);

	if (!m_poHandler)
		return;

	if (subheader == SUBHEADER_GC_MOUNT_UP_GRADE_OPEN)
		m_bUIOpen = true;

	PyCallClassMemberFunc(m_poHandler, "MountUpgradeSystemProcess",
		Py_BuildValue("(i(iii))", subheader, horse_level, is_fail, existing_exp));
}

void CPythonMountUpGrade::Chat(const uint8_t type, const uint16_t value)
{
	if (!m_poHandler)
		return;

	PyCallClassMemberFunc(m_poHandler, "MountUpgradeSystemChat",
		Py_BuildValue("(ii)", type, value));
}

void CPythonMountUpGrade::SetHorseLevel(const uint8_t level)
{
	m_HorseLevel = level;
}

uint8_t CPythonMountUpGrade::GetHorseLevel() const
{
	return m_HorseLevel;
}

void CPythonMountUpGrade::SetMountUpGradeFail(const uint8_t fail)
{
	m_MountUpGradeFail = fail;
}

uint8_t CPythonMountUpGrade::IsMountUpGradeFail() const
{
	return m_MountUpGradeFail;
}

void CPythonMountUpGrade::SetMountUpGradeExistingExp(const uint32_t exp)
{
	m_MountUpGradeExistingExp = exp;
}

uint32_t CPythonMountUpGrade::GetMountUpGradeExistingExp() const
{
	return m_MountUpGradeExistingExp;
}

uint32_t CPythonMountUpGrade::GetMountUpGradeNecessaryExp() const
{
	return mount_up_grade_exp_table[GetHorseLevel() + 1];
}

uint32_t CPythonMountUpGrade::GetMountUpGradePrice() const
{
	return mount_up_grade_price_table[GetHorseLevel() + 1];
}

void CPythonMountUpGrade::SetSkillLevelUpVnum(const uint32_t vnum)
{
	m_SkillLevelUpVnum = vnum;
}

uint32_t CPythonMountUpGrade::GetSkillLevelUpVnum() const
{
	return m_SkillLevelUpVnum;
}

uint32_t CPythonMountUpGrade::ResolveSkillLevelUpVnum() const
{
	if (m_SkillLevelUpVnum)
		return m_SkillLevelUpVnum;

	if (!m_poHandler)
		return 0;

	if (PyObject_HasAttrString(m_poHandler, "_MountUpgradeSystemWindow__skill_levelup_skill_vnum"))
	{
		PyObject* poVnum = PyObject_GetAttrString(m_poHandler, "_MountUpgradeSystemWindow__skill_levelup_skill_vnum");
		if (poVnum)
		{
			const long iVnum = PyInt_AsLong(poVnum);
			Py_DECREF(poVnum);
			if (iVnum > 0)
				return static_cast<uint32_t>(iVnum);
		}
	}

	return 0;
}

bool CPythonMountUpGrade::IsUIOpen() const
{
	return m_bUIOpen;
}

PyObject* SetMountHandler(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poHandler = nullptr;
	if (!PyTuple_GetObject(poArgs, 0, &poHandler))
		return Py_BadArgument();

	CPythonMountUpGrade::Instance().SetHandler(poHandler);
	return Py_BuildNone();
}

PyObject* GetMountHandler(PyObject* poSelf, PyObject* poArgs)
{
	PyObject* poHandler = CPythonMountUpGrade::Instance().GetHandler();
	if (!poHandler)
		Py_RETURN_NONE;

	Py_INCREF(poHandler);
	return poHandler;
}

PyObject* SendMountUpgradeSystemGiveFeed(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().MountUpGrade(CPythonMountUpGrade::SUBHEADER_CG_MOUNT_UP_GRADE_EXP, 0);
	return Py_BuildNone();
}

PyObject* SendMountUpgradeSystemLevelUp(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().MountUpGrade(CPythonMountUpGrade::SUBHEADER_CG_MOUNT_UP_GRADE_LEVEL_UP, 0);
	return Py_BuildNone();
}

PyObject* SendMountUpgradeSystemClose(PyObject* poSelf, PyObject* poArgs)
{
	CPythonNetworkStream::Instance().MountUpGrade(CPythonMountUpGrade::SUBHEADER_CG_MOUNT_UP_GRADE_CLOSE, 0);
	CPythonMountUpGrade::Instance().Reset();
	return Py_BuildNone();
}

PyObject* SendMountUpgradeSystemSkillLevelUp(PyObject* poSelf, PyObject* poArgs)
{
	const uint32_t skillVnum = CPythonMountUpGrade::Instance().ResolveSkillLevelUpVnum();
	CPythonNetworkStream::Instance().MountUpGrade(CPythonMountUpGrade::SUBHEADER_CG_MOUNT_UP_GRADE_SKILL_LEVEL_UP, skillVnum);
	return Py_BuildNone();
}

PyObject* IsMountingHorse(PyObject* poSelf, PyObject* poArgs)
{
	CInstanceBase* pMainInstance = CPythonPlayer::Instance().NEW_GetMainActorPtr();
	if (!pMainInstance)
		return Py_BuildValue("i", 0);

	return Py_BuildValue("i", pMainInstance->IsMountingHorse() ? 1 : 0);
}

PyObject* IsMountUpgradeUIOpen(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonMountUpGrade::Instance().IsUIOpen() ? 1 : 0);
}

PyObject* GetHorseLevel(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonMountUpGrade::Instance().GetHorseLevel());
}

PyObject* GetMountExistingExp(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonMountUpGrade::Instance().GetMountUpGradeExistingExp());
}

PyObject* GetMountNecessaryExp(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonMountUpGrade::Instance().GetMountUpGradeNecessaryExp());
}

PyObject* GetMountUpGradePrice(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonMountUpGrade::Instance().GetMountUpGradePrice());
}

PyObject* IsMountUpGradeFail(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonMountUpGrade::Instance().IsMountUpGradeFail());
}

void initMount()
{
	static PyMethodDef s_methods[] =
	{
		{ "SetMountHandler",					SetMountHandler,					METH_VARARGS },
		{ "GetMountHandler",					GetMountHandler,					METH_VARARGS },
		{ "SendMountUpgradeSystemGiveFeed",		SendMountUpgradeSystemGiveFeed,		METH_VARARGS },
		{ "SendMountUpgradeSystemLevelUp",		SendMountUpgradeSystemLevelUp,		METH_VARARGS },
		{ "SendMountUpgradeSystemClose",		SendMountUpgradeSystemClose,		METH_VARARGS },
		{ "SendMountUpgradeSystemSkillLevelUp",	SendMountUpgradeSystemSkillLevelUp,	METH_VARARGS },
		{ "IsMountingHorse",					IsMountingHorse,					METH_VARARGS },
		{ "IsMountUpgradeUIOpen",				IsMountUpgradeUIOpen,				METH_VARARGS },
		{ "GetHorseLevel",						GetHorseLevel,						METH_VARARGS },
		{ "GetMountExistingExp",				GetMountExistingExp,				METH_VARARGS },
		{ "GetMountNecessaryExp",				GetMountNecessaryExp,				METH_VARARGS },
		{ "GetMountUpGradePrice",				GetMountUpGradePrice,				METH_VARARGS },
		{ "IsMountUpGradeFail",					IsMountUpGradeFail,					METH_VARARGS },
		{ NULL, NULL, NULL },
	};

	PyObject* poModule = Py_InitModule("mount", s_methods);

	PyModule_AddIntConstant(poModule, "MOUNT_UPGRADE_SYSTEM_GC_SUBHEADER_OPEN", CPythonMountUpGrade::SUBHEADER_GC_MOUNT_UP_GRADE_OPEN);
	PyModule_AddIntConstant(poModule, "MOUNT_UPGRADE_SYSTEM_GC_SUBHEADER_GIVE_FEED", CPythonMountUpGrade::SUBHEADER_GC_MOUNT_UP_GRADE_GIVE_FEED);
	PyModule_AddIntConstant(poModule, "MOUNT_UPGRADE_SYSTEM_GC_SUBHEADER_HORSE_LEVEL_UP", CPythonMountUpGrade::SUBHEADER_GC_MOUNT_UP_GRADE_HORSE_LEVEL_UP);

	PyModule_AddIntConstant(poModule, "MOUNT_UPGRADE_SYSTEM_LEVEL_UP_RESULT_PERCENT_SUCCESS", CPythonMountUpGrade::MOUNT_UPGRADE_SYSTEM_LEVEL_UP_RESULT_PERCENT_SUCCESS);
	PyModule_AddIntConstant(poModule, "MOUNT_UPGRADE_SYSTEM_LEVEL_UP_RESULT_PERCENT_FAIL", CPythonMountUpGrade::MOUNT_UPGRADE_SYSTEM_LEVEL_UP_RESULT_PERCENT_FAIL);

	PyModule_AddIntConstant(poModule, "MOUNT_UPGRADE_SKILL_LEVELUP_COST_GEM", CPythonMountUpGrade::MOUNT_UPGRADE_SKILL_LEVELUP_COST_GEM);
	PyModule_AddIntConstant(poModule, "MOUNT_UPGRADE_SKILL_LEVELUP_COST_YANG", CPythonMountUpGrade::MOUNT_UPGRADE_SKILL_LEVELUP_COST_YANG);
	PyModule_AddIntConstant(poModule, "MOUNT_UPGRADE_SYSTEM_HORSE_SKILL_LEVEL_MAX", CPythonMountUpGrade::MOUNT_UPGRADE_SYSTEM_HORSE_SKILL_LEVEL_MAX);
	PyModule_AddIntConstant(poModule, "MOUNT_UPGRADE_SYSTEM_HORSE_MIN_LEVEL_CONQUEROR", CPythonMountUpGrade::MOUNT_UPGRADE_SYSTEM_HORSE_MIN_LEVEL_CONQUEROR);
	PyModule_AddIntConstant(poModule, "MOUNT_UPGRADE_SYSTEM_HORSE_LEVEL_MAX", CPythonMountUpGrade::MOUNT_UPGRADE_SYSTEM_HORSE_LEVEL_MAX);

	PyModule_AddIntConstant(poModule, "HORSE_FEED_ITEM_ID", CPythonMountUpGrade::HORSE_FEED_ITEM_ID);
	PyModule_AddIntConstant(poModule, "HORSE_FEED_EXP_COUNT", CPythonMountUpGrade::HORSE_FEED_EXP_COUNT);
	PyModule_AddIntConstant(poModule, "HORSE_FEED_LEVEL_COUNT", CPythonMountUpGrade::HORSE_FEED_LEVEL_COUNT);
	PyModule_AddIntConstant(poModule, "HORSE_LEVEL_DETERMINES_GEM_COST", CPythonMountUpGrade::HORSE_LEVEL_DETERMINES_GEM_COST);
	PyModule_AddIntConstant(poModule, "HORSE_LOWER_LEVEL_RETRY_GEM_COST", CPythonMountUpGrade::HORSE_LOWER_LEVEL_RETRY_GEM_COST);
	PyModule_AddIntConstant(poModule, "HORSE_UPPER_LEVEL_RETRY_GEM_COST", CPythonMountUpGrade::HORSE_UPPER_LEVEL_RETRY_GEM_COST);
	PyModule_AddIntConstant(poModule, "MOUNT_UP_GRADE_FAIL_OFF", CPythonMountUpGrade::MOUNT_UP_GRADE_FAIL_OFF);
	PyModule_AddIntConstant(poModule, "MOUNT_UP_GRADE_FAIL_ON", CPythonMountUpGrade::MOUNT_UP_GRADE_FAIL_ON);
}
#endif
