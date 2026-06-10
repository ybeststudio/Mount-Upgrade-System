ALTER TABLE `player`
	ADD COLUMN `mount_up_grade_exp` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `horse_skill_point`,
	ADD COLUMN `mount_up_grade_fail` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `mount_up_grade_exp`;

ALTER TABLE `player_deleted`
	ADD COLUMN `mount_up_grade_exp` INT UNSIGNED NOT NULL DEFAULT 0 AFTER `horse_skill_point`,
	ADD COLUMN `mount_up_grade_fail` TINYINT UNSIGNED NOT NULL DEFAULT 0 AFTER `mount_up_grade_exp`;
