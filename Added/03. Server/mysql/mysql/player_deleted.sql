/*
 Navicat Premium Dump SQL

 Source Server         : OfficialFiles
 Source Server Type    : MySQL
 Source Server Version : 80041 (8.0.41)
 Source Host           : 192.168.1.163:3306
 Source Schema         : player

 Target Server Type    : MySQL
 Target Server Version : 80041 (8.0.41)
 File Encoding         : 65001

 Date: 10/06/2026 11:40:10
*/

SET NAMES utf8mb4;
SET FOREIGN_KEY_CHECKS = 0;

-- ----------------------------
-- Table structure for player_deleted
-- ----------------------------
DROP TABLE IF EXISTS `player_deleted`;
CREATE TABLE `player_deleted`  (
  `id` int NOT NULL AUTO_INCREMENT,
  `account_id` int NOT NULL DEFAULT 0,
  `name` varchar(24) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'NONAME',
  `job` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `voice` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `dir` tinyint NOT NULL DEFAULT 0,
  `x` int NOT NULL DEFAULT 0,
  `y` int NOT NULL DEFAULT 0,
  `z` int NOT NULL DEFAULT 0,
  `map_index` int NOT NULL DEFAULT 0,
  `exit_x` int NOT NULL DEFAULT 0,
  `exit_y` int NOT NULL DEFAULT 0,
  `exit_map_index` int NOT NULL DEFAULT 0,
  `hp` bigint NOT NULL DEFAULT 0,
  `mp` bigint NOT NULL DEFAULT 0,
  `stamina` smallint NOT NULL DEFAULT 0,
  `random_hp` smallint UNSIGNED NOT NULL DEFAULT 0,
  `random_sp` smallint UNSIGNED NOT NULL DEFAULT 0,
  `playtime` int NOT NULL DEFAULT 0,
  `level` tinyint UNSIGNED NOT NULL DEFAULT 1,
  `level_step` tinyint(1) NOT NULL DEFAULT 0,
  `st` smallint NOT NULL DEFAULT 0,
  `ht` smallint NOT NULL DEFAULT 0,
  `dx` smallint NOT NULL DEFAULT 0,
  `iq` smallint NOT NULL DEFAULT 0,
  `exp` int UNSIGNED NOT NULL DEFAULT 0,
  `inven_stage` int UNSIGNED NOT NULL DEFAULT 0,
  `gold` int UNSIGNED NOT NULL DEFAULT 0,
  `cheque` int UNSIGNED NOT NULL DEFAULT 0,
  `gem` int UNSIGNED NOT NULL DEFAULT 0,
  `stat_point` smallint NOT NULL DEFAULT 0,
  `skill_point` smallint NOT NULL DEFAULT 0,
  `quickslot` tinyblob NULL,
  `ip` varchar(15) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NULL DEFAULT '0.0.0.0',
  `part_main` int UNSIGNED NOT NULL DEFAULT 0,
  `part_base` int NOT NULL DEFAULT 0,
  `part_hair` int NOT NULL DEFAULT 0,
  `part_acce` int NOT NULL DEFAULT 0,
  `skill_group` tinyint NOT NULL DEFAULT 0,
  `skill_level` blob NULL,
  `alignment` int NOT NULL DEFAULT 0,
  `conqueror_level` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `conqueror_level_step` tinyint(1) NOT NULL DEFAULT 0,
  `sungma_str` smallint NOT NULL DEFAULT 0,
  `sungma_hp` smallint NOT NULL DEFAULT 0,
  `sungma_move` smallint NOT NULL DEFAULT 0,
  `sungma_immune` smallint NOT NULL DEFAULT 0,
  `conqueror_exp` int NOT NULL DEFAULT 0,
  `conqueror_point` smallint NOT NULL DEFAULT 0,
  `last_play` datetime NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `status_message` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT '',
  `change_name` tinyint(1) NOT NULL DEFAULT 0,
  `sub_skill_point` smallint NOT NULL DEFAULT 0,
  `stat_reset_count` tinyint NOT NULL DEFAULT 0,
  `horse_hp` smallint NOT NULL DEFAULT 0,
  `horse_stamina` smallint NOT NULL DEFAULT 0,
  `horse_level` tinyint UNSIGNED NOT NULL DEFAULT 0,
  `horse_hp_droptime` int UNSIGNED NOT NULL DEFAULT 0,
  `horse_riding` tinyint(1) NOT NULL DEFAULT 0,
  `horse_skill_point` smallint NOT NULL DEFAULT 0,
  `shop_gold` bigint NOT NULL DEFAULT 0,
  `shop_cheque` int NOT NULL DEFAULT 0,
  `mount_up_grade_exp` int UNSIGNED NOT NULL DEFAULT 0,
  `mount_up_grade_fail` tinyint UNSIGNED NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`) USING BTREE,
  INDEX `account_id_idx`(`account_id`) USING BTREE,
  INDEX `name_idx`(`name`) USING BTREE
) ENGINE = MyISAM AUTO_INCREMENT = 38 CHARACTER SET = utf8mb4 COLLATE = utf8mb4_unicode_ci ROW_FORMAT = DYNAMIC;

-- ----------------------------
-- Records of player_deleted
-- ----------------------------

SET FOREIGN_KEY_CHECKS = 1;
