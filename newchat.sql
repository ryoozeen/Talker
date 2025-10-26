-- MySQL dump 10.13  Distrib 8.0.36, for Linux (x86_64)
--
-- Host: localhost    Database: talker
-- ------------------------------------------------------
-- Server version	8.0.43-0ubuntu0.24.04.1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `dm_rooms`
--

DROP TABLE IF EXISTS `dm_rooms`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `dm_rooms` (
  `room_id` bigint NOT NULL AUTO_INCREMENT,
  `user_a` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `user_b` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `a_socket` int DEFAULT NULL,
  `b_socket` int DEFAULT NULL,
  `a_ip` varchar(45) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `b_ip` varchar(45) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`room_id`),
  UNIQUE KEY `uniq_pair` (`user_a`,`user_b`),
  KEY `fk_dm_b` (`user_b`),
  CONSTRAINT `fk_dm_a` FOREIGN KEY (`user_a`) REFERENCES `users` (`login_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_dm_b` FOREIGN KEY (`user_b`) REFERENCES `users` (`login_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=16 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `dm_rooms`
--

LOCK TABLES `dm_rooms` WRITE;
/*!40000 ALTER TABLE `dm_rooms` DISABLE KEYS */;
INSERT INTO `dm_rooms` VALUES (5,'jihee123','ryoozeen',23,23,'10.10.21.104','10.10.21.104','2025-08-21 07:02:29');
/*!40000 ALTER TABLE `dm_rooms` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `friends`
--

DROP TABLE IF EXISTS `friends`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `friends` (
  `owner_id` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `friend_id` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `created_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`owner_id`,`friend_id`),
  KEY `idx_friends_owner_created` (`owner_id`,`created_at` DESC),
  KEY `idx_friends_friend` (`friend_id`),
  CONSTRAINT `fk_friends_friend` FOREIGN KEY (`friend_id`) REFERENCES `users` (`login_id`) ON DELETE CASCADE ON UPDATE CASCADE,
  CONSTRAINT `fk_friends_owner` FOREIGN KEY (`owner_id`) REFERENCES `users` (`login_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `friends`
--

LOCK TABLES `friends` WRITE;
/*!40000 ALTER TABLE `friends` DISABLE KEYS */;
INSERT INTO `friends` VALUES ('jihee123','ryoozeen','2025-08-21 03:29:11'),('ryoozeen','jihee123','2025-08-21 03:29:04');
/*!40000 ALTER TABLE `friends` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `messages`
--

DROP TABLE IF EXISTS `messages`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `messages` (
  `msg_id` bigint NOT NULL AUTO_INCREMENT,
  `room_id` bigint NOT NULL,
  `sender_login_id` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `body` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `sent_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`msg_id`),
  KEY `fk_msg_sender` (`sender_login_id`),
  KEY `idx_room_time` (`room_id`,`sent_at`),
  CONSTRAINT `fk_msg_room` FOREIGN KEY (`room_id`) REFERENCES `dm_rooms` (`room_id`) ON DELETE CASCADE,
  CONSTRAINT `fk_msg_sender` FOREIGN KEY (`sender_login_id`) REFERENCES `users` (`login_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=98 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `messages`
--

LOCK TABLES `messages` WRITE;
/*!40000 ALTER TABLE `messages` DISABLE KEYS */;
INSERT INTO `messages` VALUES (36,5,'ryoozeen','안녕','2025-08-21 07:02:34'),(37,5,'ryoozeen','반가워','2025-08-21 07:02:38'),(38,5,'ryoozeen','실시간으로 채팅 변경 되네?','2025-08-21 07:02:44'),(39,5,'jihee123','응','2025-08-21 07:02:47'),(40,5,'jihee123','이제 채팅방 나가볼게','2025-08-21 07:02:49'),(41,5,'jihee123','기다려봐','2025-08-21 07:02:50'),(42,5,'ryoozeen','왜ㅑ','2025-08-21 07:32:51'),(43,5,'ryoozeen','안','2025-08-21 07:32:52'),(44,5,'ryoozeen','된','2025-08-21 07:32:53'),(45,5,'ryoozeen','냐','2025-08-21 07:32:53'),(46,5,'ryoozeen','다','2025-08-21 07:32:56'),(47,5,'ryoozeen','고','2025-08-21 07:32:58'),(48,5,'ryoozeen','왜 안 되냐고','2025-08-21 07:33:00'),(49,5,'jihee123','모르지','2025-08-21 07:33:05'),(50,5,'ryoozeen','흠','2025-08-21 07:33:51'),(51,5,'ryoozeen','거의다','2025-08-21 07:37:52'),(52,5,'ryoozeen','왔다고','2025-08-21 07:37:54'),(53,5,'ryoozeen','어이','2025-08-21 07:37:55'),(54,5,'ryoozeen','어이','2025-08-21 07:37:57'),(55,5,'ryoozeen','어이','2025-08-21 07:37:57'),(56,5,'ryoozeen','거의다','2025-08-21 07:37:59'),(57,5,'ryoozeen','왔어','2025-08-21 07:38:00'),(58,5,'jihee123','대박이지?','2025-08-21 07:38:04'),(59,5,'jihee123','진짜?','2025-08-21 07:38:17'),(60,5,'ryoozeen','대박','2025-08-21 07:38:19'),(61,5,'ryoozeen','이거 줄까?','2025-08-21 07:38:20'),(62,5,'jihee123','아이고','2025-08-21 07:52:28'),(63,5,'jihee123','흠','2025-08-21 07:52:55'),(64,5,'jihee123','안녕','2025-08-21 07:53:15'),(65,5,'jihee123','채팅','2025-08-21 08:05:32'),(66,5,'jihee123','기록','2025-08-21 08:05:33'),(67,5,'jihee123','남','2025-08-21 08:05:34'),(68,5,'jihee123','아','2025-08-21 08:05:34'),(69,5,'jihee123','있을까?','2025-08-21 08:05:36'),(70,5,'jihee123','응','2025-08-21 08:05:39'),(71,5,'jihee123','왜','2025-08-21 08:07:16'),(72,5,'jihee123','?','2025-08-21 08:07:16'),(73,5,'jihee123','안 돼니','2025-08-21 08:07:20'),(74,5,'jihee123','잘봐','2025-08-21 08:19:40'),(75,5,'jihee123','채팅','2025-08-21 08:19:41'),(76,5,'jihee123','남아있잖아','2025-08-21 08:19:43'),(77,5,'jihee123','그치','2025-08-21 08:19:46'),(78,5,'ryoozeen','dasdasdsadas','2025-08-21 08:32:06'),(79,5,'ryoozeen','그래','2025-08-21 08:32:10'),(80,5,'jihee123','다 같이','2025-08-21 08:32:37'),(81,5,'jihee123','삭제','2025-08-21 08:32:39'),(82,5,'jihee123','되는데?','2025-08-21 08:32:40'),(83,5,'jihee123','된다','2025-08-21 08:33:24'),(84,5,'ryoozeen','ㅇㅇㅇ','2025-08-21 08:33:27'),(85,5,'jihee123','우와','2025-08-21 08:34:01'),(86,5,'ryoozeen','아니','2025-08-21 08:34:19'),(87,5,'jihee123','우리 대하ㅗㅓ','2025-08-21 08:34:24'),(88,5,'jihee123','화','2025-08-21 08:34:24'),(89,5,'jihee123','계속','2025-08-21 08:34:25'),(90,5,'jihee123','하','2025-08-21 08:34:26'),(91,5,'jihee123','다','2025-08-21 08:34:26'),(92,5,'jihee123','자','2025-08-21 08:34:27'),(93,5,'jihee123','바','2025-08-21 08:34:27'),(94,5,'jihee123','자','2025-08-21 08:34:28'),(95,5,'jihee123','다','2025-08-21 08:34:28'),(96,5,'ryoozeen','기록','2025-08-21 08:34:51'),(97,5,'ryoozeen','남아있네','2025-08-21 08:34:52');
/*!40000 ALTER TABLE `messages` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user_profile`
--

DROP TABLE IF EXISTS `user_profile`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `user_profile` (
  `login_id` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `status_message` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `avatar_url` varchar(255) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci DEFAULT NULL,
  `updated_at` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`login_id`),
  CONSTRAINT `fk_profile_user` FOREIGN KEY (`login_id`) REFERENCES `users` (`login_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `user_profile`
--

LOCK TABLES `user_profile` WRITE;
/*!40000 ALTER TABLE `user_profile` DISABLE KEYS */;
INSERT INTO `user_profile` VALUES ('jihee123','채팅방 삭제 확인중',NULL,'2025-08-21 03:29:32'),('ryoozeen','지금은 테스트 중입니다...',NULL,'2025-08-21 03:28:46');
/*!40000 ALTER TABLE `user_profile` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `user_room_state`
--

DROP TABLE IF EXISTS `user_room_state`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `user_room_state` (
  `room_id` bigint NOT NULL,
  `login_id` varchar(64) NOT NULL,
  `hidden` tinyint(1) NOT NULL DEFAULT '0',
  `cleared_at` datetime DEFAULT NULL,
  PRIMARY KEY (`room_id`,`login_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `user_room_state`
--

LOCK TABLES `user_room_state` WRITE;
/*!40000 ALTER TABLE `user_room_state` DISABLE KEYS */;
INSERT INTO `user_room_state` VALUES (5,'jihee123',0,'2025-08-21 17:34:08'),(5,'ryoozeen',0,'2025-08-21 17:34:35');
/*!40000 ALTER TABLE `user_room_state` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `users` (
  `number` int NOT NULL AUTO_INCREMENT,
  `login_id` varchar(32) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `name` varchar(50) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `gender` enum('M','G') CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `email` varchar(100) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `phone` varchar(20) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `salt` char(16) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `pass_hash` char(64) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`number`),
  UNIQUE KEY `login_id` (`login_id`),
  UNIQUE KEY `email` (`email`),
  UNIQUE KEY `phone` (`phone`)
) ENGINE=InnoDB AUTO_INCREMENT=3 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `users`
--

LOCK TABLES `users` WRITE;
/*!40000 ALTER TABLE `users` DISABLE KEYS */;
INSERT INTO `users` VALUES (1,'ryoozeen','유진','M','ryoozeen@naver.com','010-4410-4990','2f54d03de687c8ff','a99a41173db9c910d96cdee79e3e267b34a7540bd66e0c257a14d4fafa18a6f5','2025-08-21 03:27:18'),(2,'jihee123','지희','G','jihee123@naver.com','010-1234-5678','5d2a686ab98c721a','e504a57d7a173f378443cf168b2e10cf9493c51a71b9a876991880b424eb9c4b','2025-08-21 03:28:22');
/*!40000 ALTER TABLE `users` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2025-08-21 17:36:12
