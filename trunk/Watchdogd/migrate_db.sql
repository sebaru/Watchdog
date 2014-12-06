-- phpMyAdmin SQL Dump
-- version 3.3.9
-- http://www.phpmyadmin.net
--
-- Serveur: localhost
-- Généré le : Ven 11 Février 2011 à 16:49
-- Version du serveur: 5.1.54
-- Version de PHP: 5.3.4

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

DROP PROCEDURE IF EXISTS MigrateDB;
DELIMITER //
CREATE PROCEDURE MigrateDB ()
 BEGIN
  SELECT @db_ver := valeur FROM config where nom_thread="GLOBAL" AND nom="database_version" LIMIT 1;
  IF(@db_ver IS NULL) THEN
     INSERT INTO config(instance_id,nom_thread,nom,valeur) VALUES ('MASTER','GLOBAL','database_version','0');
     SET @db_ver = 0;
  END IF;

  IF @db_ver = 0 THEN 
     ALTER TABLE users DROP `imsg_bit_presence`;
     ALTER TABLE users ADD `ssrv_bit_presence` INT NOT NULL DEFAULT '0';
  END IF;
  UPDATE config SET valeur = 1 where nom_thread="GLOBAL" AND nom="database_version";
END //
DELIMITER ;
CALL MigrateDB();
