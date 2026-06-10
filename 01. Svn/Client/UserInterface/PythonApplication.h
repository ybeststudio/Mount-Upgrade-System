// Add the following declaration/member block related section:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	#include "PythonMountUpGrade.h"
#endif

// Find this line:
CPythonLoading m_pyLoading;

// Add after it:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	CPythonMountUpGrade m_pyMountUpGrade;
#endif
