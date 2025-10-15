-- MySQL dump 10.13  Distrib 8.0.42, for Win64 (x86_64)
--
-- Host: localhost    Database: low_altitude_traffic_system
-- ------------------------------------------------------
-- Server version	8.0.42

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!50503 SET NAMES utf8mb4 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `api_flight_tasks`
--

DROP TABLE IF EXISTS `api_flight_tasks`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `api_flight_tasks` (
  `id` int NOT NULL AUTO_INCREMENT,
  `name` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `description` text COLLATE utf8mb4_unicode_ci,
  `route` json DEFAULT NULL,
  `status` enum('pending','ongoing','completed') COLLATE utf8mb4_unicode_ci DEFAULT 'pending',
  `scheduled_time` datetime DEFAULT NULL,
  `user_id` int NOT NULL,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_user_id` (`user_id`),
  KEY `idx_status` (`status`),
  KEY `idx_scheduled_time` (`scheduled_time`),
  KEY `idx_created_at` (`created_at`),
  CONSTRAINT `api_flight_tasks_ibfk_1` FOREIGN KEY (`user_id`) REFERENCES `users` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB AUTO_INCREMENT=6 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `api_flight_tasks`
--

LOCK TABLES `api_flight_tasks` WRITE;
/*!40000 ALTER TABLE `api_flight_tasks` DISABLE KEYS */;
INSERT INTO `api_flight_tasks` VALUES (1,'????????','?޸????????񴴽?????','{\"end\": {\"lat\": 39.9142, \"lng\": 116.417396}, \"start\": {\"lat\": 39.9042, \"lng\": 116.407396}}','pending','2025-09-30 11:33:37',7,'2025-09-30 11:33:37','2025-09-30 11:33:37'),(3,'无人机飞行0930','10.1起飞','{\"end\": {\"lat\": 39.9142, \"lng\": 116.417396}, \"start\": {\"lat\": 39.9042, \"lng\": 116.407396}}','pending','2025-09-30 11:37:27',8,'2025-09-30 11:37:27','2025-09-30 11:37:27'),(4,'?????û?????','????ͨ?û???????????','{\"end\": {\"lat\": 39.9142, \"lng\": 116.417396}, \"start\": {\"lat\": 39.9042, \"lng\": 116.407396}}','pending','2025-09-30 11:59:39',6,'2025-09-30 11:59:39','2025-09-30 11:59:39'),(5,'无人机飞11','1','{\"end\": {\"lat\": 39.9142, \"lng\": 116.417396}, \"start\": {\"lat\": 39.9042, \"lng\": 116.407396}}','pending','2025-09-30 12:19:27',8,'2025-09-30 12:19:27','2025-09-30 12:19:27');
/*!40000 ALTER TABLE `api_flight_tasks` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `emergency_events`
--

DROP TABLE IF EXISTS `emergency_events`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `emergency_events` (
  `id` bigint NOT NULL AUTO_INCREMENT,
  `event_code` varchar(50) NOT NULL,
  `task_id` bigint NOT NULL,
  `drone_id` bigint DEFAULT NULL,
  `type` varchar(50) NOT NULL,
  `severity` varchar(20) NOT NULL,
  `status` varchar(20) NOT NULL DEFAULT 'active',
  `title` varchar(200) NOT NULL,
  `description` text,
  `location` json NOT NULL,
  `response_action` text,
  `response_notes` text,
  `responded_by` bigint DEFAULT NULL,
  `responded_at` timestamp NULL DEFAULT NULL,
  `resolved_at` timestamp NULL DEFAULT NULL,
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  UNIQUE KEY `event_code` (`event_code`),
  KEY `idx_task_id` (`task_id`),
  KEY `idx_status` (`status`),
  KEY `idx_severity` (`severity`),
  KEY `idx_created_at` (`created_at`)
) ENGINE=InnoDB AUTO_INCREMENT=5 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='紧急事件表';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `emergency_events`
--

LOCK TABLES `emergency_events` WRITE;
/*!40000 ALTER TABLE `emergency_events` DISABLE KEYS */;
INSERT INTO `emergency_events` VALUES (1,'EMG-20251004-7867',1,NULL,'weather_emergency','medium','active','天气','天气','{\"lat\": 39.9042, \"lon\": 116.4074}',NULL,NULL,NULL,NULL,NULL,'2025-10-04 08:23:15','2025-10-04 08:23:15'),(2,'EMG-20251004-3138',1,NULL,'weather_emergency','medium','active','天气','天气','{\"lat\": 39.9042, \"lon\": 116.4074}',NULL,NULL,NULL,NULL,NULL,'2025-10-04 08:27:30','2025-10-04 08:27:30'),(3,'EMG-20251004-3596',3,NULL,'other','medium','cancelled','16.40','16.40','{\"lat\": 39.9042, \"lon\": 116.4074}',NULL,NULL,NULL,NULL,NULL,'2025-10-04 08:40:19','2025-10-04 10:07:24'),(4,'EMG-20251004-5276',4,NULL,'other','medium','resolved','0004','0004','{\"lat\": 39.9042, \"lon\": 116.4074}','解决','',8,'2025-10-04 10:07:02','2025-10-04 10:13:21','2025-10-04 08:52:06','2025-10-04 10:13:21');
/*!40000 ALTER TABLE `emergency_events` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `flight_logs`
--

DROP TABLE IF EXISTS `flight_logs`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `flight_logs` (
  `id` int NOT NULL AUTO_INCREMENT COMMENT '鏃ュ織ID',
  `task_id` int NOT NULL COMMENT '鍏宠仈浠诲姟ID',
  `timestamp` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT '璁板綍鏃堕棿',
  `latitude` decimal(10,8) DEFAULT NULL COMMENT '绾?害',
  `longitude` decimal(11,8) DEFAULT NULL COMMENT '缁忓害',
  `altitude` decimal(8,2) DEFAULT NULL COMMENT '楂樺害(绫?',
  `speed` decimal(8,2) DEFAULT NULL COMMENT '閫熷害(km/h)',
  `heading` decimal(5,2) DEFAULT NULL COMMENT '鑸?悜瑙?搴?',
  `battery_level` decimal(5,2) DEFAULT NULL COMMENT '鐢垫睜鐢甸噺(%)',
  `signal_strength` int DEFAULT NULL COMMENT '淇″彿寮哄害',
  `status_message` text COMMENT '鐘舵?淇℃伅',
  PRIMARY KEY (`id`),
  KEY `idx_task_id` (`task_id`),
  KEY `idx_timestamp` (`timestamp`),
  CONSTRAINT `flight_logs_ibfk_1` FOREIGN KEY (`task_id`) REFERENCES `flight_tasks` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='椋炶?鏃ュ織琛';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `flight_logs`
--

LOCK TABLES `flight_logs` WRITE;
/*!40000 ALTER TABLE `flight_logs` DISABLE KEYS */;
/*!40000 ALTER TABLE `flight_logs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `flight_tasks`
--

DROP TABLE IF EXISTS `flight_tasks`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `flight_tasks` (
  `id` int NOT NULL AUTO_INCREMENT COMMENT '浠诲姟ID',
  `task_name` varchar(100) NOT NULL COMMENT '浠诲姟鍚嶇О',
  `pilot_id` int NOT NULL COMMENT '椋炶?鍛業D',
  `aircraft_id` varchar(50) DEFAULT NULL COMMENT '椋炴満鏍囪瘑',
  `task_type` enum('survey','transport','patrol','emergency','training') NOT NULL COMMENT '浠诲姟绫诲瀷',
  `priority` enum('low','medium','high','critical') DEFAULT 'medium' COMMENT '浼樺厛绾',
  `status` enum('pending','approved','executing','completed','cancelled','failed') DEFAULT 'pending' COMMENT '浠诲姟鐘舵?',
  `planned_start_time` datetime NOT NULL COMMENT '璁″垝寮??鏃堕棿',
  `planned_end_time` datetime NOT NULL COMMENT '璁″垝缁撴潫鏃堕棿',
  `actual_start_time` datetime DEFAULT NULL COMMENT '瀹為檯寮??鏃堕棿',
  `actual_end_time` datetime DEFAULT NULL COMMENT '瀹為檯缁撴潫鏃堕棿',
  `departure_location` varchar(200) NOT NULL COMMENT '鍑哄彂鍦扮偣',
  `destination_location` varchar(200) NOT NULL COMMENT '鐩?殑鍦',
  `flight_altitude` int DEFAULT NULL COMMENT '椋炶?楂樺害(绫?',
  `flight_speed` decimal(8,2) DEFAULT NULL COMMENT '椋炶?閫熷害(km/h)',
  `route_data` json DEFAULT NULL COMMENT '鑸?嚎鏁版嵁(JSON鏍煎紡)',
  `weather_conditions` text COMMENT '澶╂皵鏉′欢',
  `notes` text COMMENT '澶囨敞淇℃伅',
  `created_by` int NOT NULL COMMENT '鍒涘缓浜篒D',
  `approved_by` int DEFAULT NULL COMMENT '鎵瑰噯浜篒D',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT '鍒涘缓鏃堕棿',
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '鏇存柊鏃堕棿',
  PRIMARY KEY (`id`),
  KEY `approved_by` (`approved_by`),
  KEY `idx_pilot_id` (`pilot_id`),
  KEY `idx_status` (`status`),
  KEY `idx_task_type` (`task_type`),
  KEY `idx_planned_start` (`planned_start_time`),
  KEY `idx_created_by` (`created_by`),
  CONSTRAINT `flight_tasks_ibfk_1` FOREIGN KEY (`pilot_id`) REFERENCES `users` (`id`) ON DELETE RESTRICT,
  CONSTRAINT `flight_tasks_ibfk_2` FOREIGN KEY (`created_by`) REFERENCES `users` (`id`) ON DELETE RESTRICT,
  CONSTRAINT `flight_tasks_ibfk_3` FOREIGN KEY (`approved_by`) REFERENCES `users` (`id`) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='椋炶?浠诲姟琛';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `flight_tasks`
--

LOCK TABLES `flight_tasks` WRITE;
/*!40000 ALTER TABLE `flight_tasks` DISABLE KEYS */;
/*!40000 ALTER TABLE `flight_tasks` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `map_markers`
--

DROP TABLE IF EXISTS `map_markers`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `map_markers` (
  `id` int NOT NULL AUTO_INCREMENT,
  `lng` double NOT NULL,
  `lat` double NOT NULL,
  `title` varchar(255) COLLATE utf8mb4_unicode_ci NOT NULL,
  `type` varchar(50) COLLATE utf8mb4_unicode_ci NOT NULL DEFAULT 'marker',
  `description` text COLLATE utf8mb4_unicode_ci,
  `is_active` tinyint(1) DEFAULT '1',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`),
  KEY `idx_active` (`is_active`),
  KEY `idx_created` (`created_at`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `map_markers`
--

LOCK TABLES `map_markers` WRITE;
/*!40000 ALTER TABLE `map_markers` DISABLE KEYS */;
/*!40000 ALTER TABLE `map_markers` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `system_config`
--

DROP TABLE IF EXISTS `system_config`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `system_config` (
  `id` int NOT NULL AUTO_INCREMENT COMMENT '閰嶇疆ID',
  `config_key` varchar(100) NOT NULL COMMENT '閰嶇疆閿',
  `config_value` text COMMENT '閰嶇疆鍊',
  `description` text COMMENT '閰嶇疆鎻忚堪',
  `data_type` enum('string','number','boolean','json') DEFAULT 'string' COMMENT '鏁版嵁绫诲瀷',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT '鍒涘缓鏃堕棿',
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '鏇存柊鏃堕棿',
  PRIMARY KEY (`id`),
  UNIQUE KEY `config_key` (`config_key`),
  KEY `idx_config_key` (`config_key`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='绯荤粺閰嶇疆琛';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `system_config`
--

LOCK TABLES `system_config` WRITE;
/*!40000 ALTER TABLE `system_config` DISABLE KEYS */;
/*!40000 ALTER TABLE `system_config` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `users`
--

DROP TABLE IF EXISTS `users`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!50503 SET character_set_client = utf8mb4 */;
CREATE TABLE `users` (
  `id` int NOT NULL AUTO_INCREMENT COMMENT '鐢ㄦ埛ID',
  `username` varchar(50) NOT NULL COMMENT '鐢ㄦ埛鍚',
  `password_hash` varchar(255) NOT NULL COMMENT '瀵嗙爜鍝堝笇',
  `email` varchar(100) DEFAULT NULL COMMENT '閭??',
  `phone` varchar(20) DEFAULT NULL COMMENT '鐢佃瘽',
  `role` enum('admin','operator','user') DEFAULT 'user',
  `status` enum('active','inactive','banned') DEFAULT 'active',
  `created_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP COMMENT '鍒涘缓鏃堕棿',
  `updated_at` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '鏇存柊鏃堕棿',
  `last_login` timestamp NULL DEFAULT NULL COMMENT '鏈?悗鐧诲綍鏃堕棿',
  `full_name` varchar(100) DEFAULT NULL COMMENT '用户姓名',
  `avatar_url` varchar(255) DEFAULT NULL COMMENT '头像URL',
  `last_login_at` datetime DEFAULT NULL COMMENT '上次登录时间',
  PRIMARY KEY (`id`),
  UNIQUE KEY `username` (`username`),
  UNIQUE KEY `email` (`email`),
  KEY `idx_username` (`username`),
  KEY `idx_email` (`email`),
  KEY `idx_role` (`role`)
) ENGINE=InnoDB AUTO_INCREMENT=9 DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_0900_ai_ci COMMENT='鐢ㄦ埛琛';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Dumping data for table `users`
--

LOCK TABLES `users` WRITE;
/*!40000 ALTER TABLE `users` DISABLE KEYS */;
INSERT INTO `users` VALUES (1,'testuser123','9ddbd0b75aede181091b6351a2cd91d6f31d7b0dbb533c010b44b2a57351ada834d4c608da9e6e3d16dad591b043ae2c','test@example.com','','user','active','2025-09-27 07:45:52','2025-09-27 07:46:23',NULL,'Test User','','2025-09-27 15:46:23'),(2,'newuser123','a769d2b16333986b82aee1d43219a7737cf4fb22ad6355c9f88e98a38f3cef772a43020ea3dfe55565e895426682dc38','newuser123@example.com','','user','active','2025-09-27 08:09:02','2025-09-30 01:06:34',NULL,'newuser123','','2025-09-30 09:06:34'),(3,'lihanning','bd2c5f67045f98effac94048e1a03a6b06e2acdc7189f57fb08e6e2a43bb0407d334505dfa241b1833a6770ca788fd32','1465966244@qq.com','','user','active','2025-09-27 08:11:17','2025-09-27 08:11:17',NULL,'李晗宁','',NULL),(4,'testadmin','b26187f757909f2af96d373bbfef3d73ed8ac9a26e9775faf6dbe9ec67ffdd5b68a700638bfda581cebb002bcd4b1dc6','admin@example.com','','user','active','2025-09-27 08:55:12','2025-09-27 08:55:12',NULL,'Test Admin','',NULL),(5,'admin123','9f32acf782d6ac635e19fe131c128afa49330081613eee33fba81d77f0810a699ba79d7ddba229fafbe116ea569e6ba9','admin123@example.com','','admin','active','2025-09-27 09:04:42','2025-09-27 09:05:04',NULL,'Test Admin','','2025-09-27 17:05:04'),(6,'testuser','2535db6624cf49511112fc56bde55be32af9516bd65cfd919f6323aeb56dc36fa06c5586dd299bb58c06c8c3b2e68bcf','testuser@example.com','','user','active','2025-09-30 00:33:52','2025-10-04 10:19:41',NULL,'Test User','','2025-10-04 18:19:41'),(7,'admin','6f1bc83d108b0ed1632049d67573c831d0ffb4e7be69ce3202ce7e4ad8e60272cc6d0de74180ebd2dc85e51a04fd6688','admin@system.com','','user','active','2025-09-30 00:50:25','2025-09-30 12:00:19',NULL,'System Administrator','','2025-09-30 20:00:19'),(8,'superadmin','0a9ccbaddca6d17d26dfac8620a04288f4a699d5fb36a76799601571c0d6cf1593aef6090c03d2f7af8503a7d61c8d23','superadmin@system.com','','admin','active','2025-09-30 00:51:03','2025-10-13 09:07:41',NULL,'Super Administrator','','2025-10-13 17:07:41');
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

-- Dump completed on 2025-10-13 17:14:21
