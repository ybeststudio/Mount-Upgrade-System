// Add the following packet definitions in the related packet section:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	HEADER_CG_MOUNT_UP_GRADE = 202,
#endif

// Add the following packet definitions in the related packet section:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
	HEADER_GC_MOUNT_UP_GRADE = 154,
	HEADER_GC_MOUNT_UP_GRADE_CHAT = 155,
#endif

// Add the following packet definitions in the related packet section:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
#define	SKILL_MAX_NUM 320
#else
#define	SKILL_MAX_NUM 255
#endif

// Add the following packet definitions in the related packet section:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
typedef struct SPacketCGMountUpGrade
{
	uint8_t header;
	uint8_t iSubHeader;
	uint32_t arg;
} TPacketCGMountUpGrade;

typedef struct SPacketGCMountUpGrade
{
	uint8_t header;
	uint8_t subheader;
	uint8_t horse_level;
	uint8_t is_fail;
	uint32_t existing_exp;
} TPacketGCMountUpGrade;

typedef struct SPacketGCMountUpGradeChat
{
	uint8_t header;
	uint8_t type;
	uint16_t value;
} TPacketGCMountUpGradeChat;
#endif
