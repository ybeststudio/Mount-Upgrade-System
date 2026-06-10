// Find this line:
std::string strJob = TokenVector[DESC_TOKEN_TYPE_JOB];

// Add after it:
#if defined(ENABLE_MOUNT_UPGRADE_SYSTEM)
			if (strcmp(strJob.c_str(), "MOUNT_UPGRADE") == 0)
			{
				rSkillData.byType = SKILL_TYPE_MOUNT_UPGRADE;
			}
			else
#endif
