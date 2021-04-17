-- phpMyAdmin SQL Dump
-- phpMyAdmin SQL Dump
-- version 3.3.9
-- http://www.phpmyadmin.net
--
-- Serveur: localhost
-- Généré le : Ven 11 Février 2011 à 16:49
-- Version du serveur: 5.1.54
-- Version de PHP: 5.3.4

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Base de données: `WatchdogDB`
--

-- --------------------------------------------------------

--
-- Structure de la table `config`
--

CREATE TABLE IF NOT EXISTS `config` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `instance_id` varchar(80) COLLATE utf8_unicode_ci NOT NULL,
  `nom_thread` varchar(80) COLLATE utf8_unicode_ci NOT NULL,
  `nom` varchar(80) COLLATE utf8_unicode_ci NOT NULL,
  `valeur` varchar(128) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE (`instance_id`,`nom_thread`,`nom`)
) ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE utf8_unicode_ci AUTO_INCREMENT=1;

-- --------------------------------------------------------

--
-- Structure de la table `cameras`
--

CREATE TABLE IF NOT EXISTS `cameras` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `location` varchar(600) NOT NULL,
  `libelle` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1;

--
-- Structure de la table `icone`
--

CREATE TABLE IF NOT EXISTS `icone` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `forme` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,
  `extension` VARCHAR(4) NOT NULL DEFAULT 'svg',
  `mode_affichage` VARCHAR(32) NOT NULL DEFAULT 'cadre',
  PRIMARY KEY (`id`)
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- --------------------------------------------------------

--
-- Structure de la table `syns`
--

CREATE TABLE IF NOT EXISTS `syns` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `parent_id` int(11) NOT NULL,
  `libelle` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,
  `image` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'home',
  `page` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,
  `access_level` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  FOREIGN KEY (`parent_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;
INSERT INTO `syns` (`id`, `parent_id`, `libelle`, `page`, `access_level` ) VALUES
(1, 1, 'Accueil', 'Defaut Page', 0);

-- --------------------------------------------------------

--
-- Structure de la table `dls`
--

CREATE TABLE IF NOT EXISTS `dls` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `is_thread` tinyint(1) NOT NULL DEFAULT '0',
  `tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,
  `package` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT "custom",
  `syn_id` int(11) NOT NULL DEFAULT '0',
  `name` text COLLATE utf8_unicode_ci NOT NULL,
  `shortname` text COLLATE utf8_unicode_ci NOT NULL,
  `actif` tinyint(1) NOT NULL DEFAULT '0',
  `compil_date` DATETIME NOT NULL DEFAULT NOW(),
  `compil_status` int(11) NOT NULL DEFAULT '0',
  `nbr_compil` int(11) NOT NULL DEFAULT '0',
  `sourcecode` MEDIUMTEXT COLLATE utf8_unicode_ci NOT NULL DEFAULT "/* Default ! */",
  `errorlog` TEXT COLLATE utf8_unicode_ci NOT NULL DEFAULT "No Error",
  `nbr_ligne` int(11) NOT NULL DEFAULT '0',
  `debug` TINYINT(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;
INSERT INTO `dls` (`id`, `syn_id`, `name`, `shortname`, `tech_id`, `actif`, `compil_date`, `compil_status` ) VALUES
(1, 1, 'Système', 'Système', 'SYS', FALSE, 0, 0);

-- --------------------------------------------------------

--
-- Structure de la table `tableau`
--

CREATE TABLE IF NOT EXISTS `tableau` (
 `id` INT NOT NULL AUTO_INCREMENT,
 `titre` VARCHAR(128) UNIQUE NOT NULL,
 `syn_id` int(11) NOT NULL,
 `date_create` DATETIME NOT NULL DEFAULT NOW(),
 PRIMARY KEY (`id`)
 FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
 ) ENGINE = InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `courbes`
--

CREATE TABLE IF NOT EXISTS `tableau_map` (
  `id` INT NOT NULL AUTO_INCREMENT ,
  `tableau_id` INT NOT NULL ,
  `tech_id` varchar(32) NOT NULL,
  `acronyme` VARCHAR(64) NOT NULL,
  `color` varchar(32) NOT NULL DEFAULT "blue",
  PRIMARY KEY (`id`),
  INDEX (`tableau_id`),
  FOREIGN KEY (`tableau_id`) REFERENCES `tableau` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos`
--

CREATE TABLE IF NOT EXISTS `mnemos_BOOL` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `deletable` tinyint(1) NOT NULL DEFAULT '1',
  `type` int(11) NOT NULL DEFAULT 0,
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `etat` tinyint(1) NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;


-- --------------------------------------------------------

--
-- Structure de la table `mnemos`
--

CREATE TABLE IF NOT EXISTS `mnemos_WATCHDOG` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `deletable` tinyint(1) NOT NULL DEFAULT '1',
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos`
--

CREATE TABLE IF NOT EXISTS `mnemos_DI` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `deletable` tinyint(1) NOT NULL DEFAULT '1',
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `map_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `map_thread` VARCHAR(20) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `map_tag` VARCHAR(160) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  UNIQUE (`map_tech_id`,`map_tag`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos`
--

CREATE TABLE IF NOT EXISTS `mnemos_DO` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `deletable` tinyint(1) NOT NULL DEFAULT '1',
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `map_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `map_thread` VARCHAR(20) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `map_tag` VARCHAR(40) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `dst_param1` VARCHAR(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  UNIQUE (`map_tech_id`,`map_tag`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos_AI`
--

CREATE TABLE IF NOT EXISTS `mnemos_AI` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `deletable` tinyint(1) NOT NULL DEFAULT '1',
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `type` int(11) NOT NULL DEFAULT '0',
  `min` float NOT NULL DEFAULT '0',
  `max` float NOT NULL DEFAULT '0',
  `valeur` float NOT NULL DEFAULT '0',
  `unite` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `map_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `map_thread` VARCHAR(20) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `map_tag` VARCHAR(160) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `map_question_vocale` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `map_reponse_vocale` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'aucun',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  UNIQUE (`map_tech_id`,`map_tag`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos_AO`
--

CREATE TABLE IF NOT EXISTS `mnemos_AO` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `deletable` tinyint(1) NOT NULL DEFAULT '1',
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `type` int(11) NOT NULL DEFAULT '0',
  `min` float NOT NULL DEFAULT '0',
  `max` float NOT NULL DEFAULT '0',
  `valeur` float NOT NULL DEFAULT '0',
  `map_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `map_thread` VARCHAR(20) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `map_tag` VARCHAR(160) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  UNIQUE (`map_tech_id`,`map_tag`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `dls_cpt_imp`
--

CREATE TABLE IF NOT EXISTS `mnemos_CI` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `etat` BOOLEAN NOT NULL DEFAULT '0',
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `valeur` INT(11) NOT NULL DEFAULT '0',
  `multi` float NOT NULL DEFAULT '1',
  `unite` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'fois',
  `archivage` INT(11) NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;


-- --------------------------------------------------------

--
-- Structure de la table `mnemos_CH`
--

CREATE TABLE IF NOT EXISTS `mnemos_CH` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `etat` BOOLEAN NOT NULL DEFAULT '0',
  `valeur` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;


-- --------------------------------------------------------

--
-- Structure de la table `tempo`
--

CREATE TABLE IF NOT EXISTS `mnemos_Tempo` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;


-- --------------------------------------------------------

--
-- Structure de la table `mnemos_Horloge`
--

CREATE TABLE IF NOT EXISTS `mnemos_HORLOGE` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `deletable` TINYINT(1) NOT NULL DEFAULT '1',
  `access_level` INT(11) NOT NULL DEFAULT '0',
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;


-- --------------------------------------------------------

--
-- Structure de la table `mnemos_Horloge_ticks`
--

CREATE TABLE IF NOT EXISTS `mnemos_HORLOGE_ticks` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `horloge_id` INT(11) NOT NULL,
  `heure` int(11) NOT NULL DEFAULT '0',
  `minute` int(11) NOT NULL DEFAULT '0',
  `lundi` tinyint(1) NOT NULL DEFAULT '0',
  `mardi` tinyint(1) NOT NULL DEFAULT '0',
  `mercredi` tinyint(1) NOT NULL DEFAULT '0',
  `jeudi` tinyint(1) NOT NULL DEFAULT '0',
  `vendredi` tinyint(1) NOT NULL DEFAULT '0',
  `samedi` tinyint(1) NOT NULL DEFAULT '0',
  `dimanche` tinyint(1) NOT NULL DEFAULT '0',
  `date_modif` DATETIME NOT NULL DEFAULT NOW(),
  PRIMARY KEY (`id`),
  FOREIGN KEY (`horloge_id`) REFERENCES `mnemos_HORLOGE` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos_R`
--

CREATE TABLE IF NOT EXISTS `mnemos_R` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL,
  `valeur` FLOAT NOT NULL DEFAULT '0',
  `unite` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `archivage` BOOLEAN NOT NULL DEFAULT '0',
  `map_question_vocale` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `map_reponse_vocale` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'aucun',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_camerasup`
--

CREATE TABLE IF NOT EXISTS `syns_camerasup` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `syn_id` int(11) NOT NULL,
  `camera_src_id` int(11) NOT NULL,
  `posx` int(11) NOT NULL,
  `posy` int(11) NOT NULL,
  PRIMARY KEY (`id`),
  FOREIGN KEY (`camera_src_id`) REFERENCES `cameras` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_cadrans`
--

CREATE TABLE IF NOT EXISTS `syns_cadrans` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `auto_create` tinyint(1) NULL DEFAULT NULL,
  `forme` VARCHAR(80) NOT NULL DEFAULT 'unknown',
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT "SYS",
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `syn_id` int(11) NOT NULL DEFAULT '0',
  `posx` int(11) NOT NULL DEFAULT '0',
  `posy` int(11) NOT NULL DEFAULT '0',
  `minimum` int(11) NOT NULL DEFAULT '0',
  `maximum` int(11) NOT NULL DEFAULT '100',
  `type` int(11) NOT NULL DEFAULT '0',
  `angle` int(11) NOT NULL DEFAULT '0',
  `nb_decimal` int(11) NOT NULL DEFAULT '2',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`, `acronyme`, `auto_create`),
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------


--
-- Structure de la table `syns_comments`
--

CREATE TABLE IF NOT EXISTS `syns_comments` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `syn_id` int(11) NOT NULL DEFAULT '0',
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `font` text COLLATE utf8_unicode_ci NOT NULL,
  `font_size` int(11) NOT NULL DEFAULT '20',
  `def_color` VARCHAR(12) COLLATE utf8_unicode_ci NOT NULL DEFAULT "white",
  `rouge` int(11) NOT NULL DEFAULT '0',
  `vert` int(11) NOT NULL DEFAULT '0',
  `bleu` int(11) NOT NULL DEFAULT '0',
  `posx` int(11) NOT NULL DEFAULT '0',
  `posy` int(11) NOT NULL DEFAULT '0',
  `angle` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_motifs`
--

CREATE TABLE IF NOT EXISTS `syns_motifs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `auto_create` tinyint(1) NULL DEFAULT NULL,
  `forme` VARCHAR(80) NOT NULL DEFAULT 'unknown',
  `icone` int(11) NOT NULL DEFAULT '0',
  `syn_id` int(11) NOT NULL DEFAULT '0',
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `access_level` int(11) NOT NULL DEFAULT '0',
  `rafraich` int(11) NOT NULL DEFAULT '0',
  `posx` int(11) NOT NULL DEFAULT '0',
  `posy` int(11) NOT NULL DEFAULT '0',
  `larg` int(11) NOT NULL DEFAULT '0',
  `haut` int(11) NOT NULL DEFAULT '0',
  `angle` int(11) NOT NULL DEFAULT '0',
  `scale` float NOT NULL DEFAULT '1',
  `dialog` int(11) NOT NULL DEFAULT '0',
  `gestion` int(11) NOT NULL DEFAULT '0',
  `layer` int(11) NOT NULL DEFAULT '0',
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `def_color` varchar(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT "#c0c0c0",
  `clic_tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `clic_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`, `acronyme`, `auto_create`),
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_palettes`
--

CREATE TABLE IF NOT EXISTS `syns_palettes` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `syn_id` int(11) NOT NULL DEFAULT '0',
  `syn_cible_id` int(11) NOT NULL DEFAULT '0',
  `pos` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  FOREIGN KEY (`syn_cible_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_pass`
--

CREATE TABLE IF NOT EXISTS `syns_pass` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `syn_id` int(11) NOT NULL DEFAULT '0',
  `syn_cible_id` int(11) NOT NULL DEFAULT '0',
  `posx` int(11) NOT NULL DEFAULT '0',
  `posy` int(11) NOT NULL DEFAULT '0',
  `angle` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE ON UPDATE CASCADE,
  FOREIGN KEY (`syn_cible_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

CREATE TABLE IF NOT EXISTS `syns_liens` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `syn_id` int(11) NOT NULL DEFAULT 0,
  `src_posx` int(11) NOT NULL DEFAULT 0,
  `src_posy` int(11) NOT NULL DEFAULT 0,
  `dst_posx` int(11) NOT NULL DEFAULT 0,
  `dst_posy` int(11) NOT NULL DEFAULT 0,
  `stroke` varchar(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'black',
  `stroke_dasharray` varchar(32) COLLATE utf8_unicode_ci DEFAULT NULL,
  `stroke_width` int(11) NOT NULL DEFAULT 1,
  `stroke_linecap` varchar(32) COLLATE utf8_unicode_ci DEFAULT 'butt',
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  PRIMARY KEY (`id`),
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

CREATE TABLE IF NOT EXISTS `syns_rectangles` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `syn_id` int(11) NOT NULL DEFAULT 0,
  `posx` int(11) NOT NULL DEFAULT 0,
  `posy` int(11) NOT NULL DEFAULT 0,
  `width` int(11) NOT NULL DEFAULT 10,
  `height` int(11) NOT NULL DEFAULT 10,
  `rx` int(11) NOT NULL DEFAULT 0,
  `ry` int(11) NOT NULL DEFAULT 0,
  `stroke` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'black',
  `def_color` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL,
  `stroke_width` int(11) NOT NULL DEFAULT 1,
  `stroke_dasharray` VARCHAR(32) COLLATE utf8_unicode_ci NULL,
  `tech_id` VARCHAR(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  PRIMARY KEY (`id`),
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `msgs`
--

CREATE TABLE IF NOT EXISTS `thread_classe` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `thread` varchar(32) COLLATE utf8_unicode_ci UNIQUE DEFAULT "",
  `classe` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;


-- --------------------------------------------------------

--
-- Structure de la table `msgs`
--

CREATE TABLE IF NOT EXISTS `msgs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `deletable` tinyint(1) NOT NULL DEFAULT '1',
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT "No libelle",
  `typologie` int(11) NOT NULL DEFAULT '0',
  `sms_notification` int(11) NOT NULL DEFAULT '0',
  `audio_profil` VARCHAR(80) NOT NULL DEFAULT 'P_NONE',
  `audio_libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `etat` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE(`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `histo_msgs`
--

CREATE TABLE IF NOT EXISTS `histo_msgs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `id_msg` int(11) NOT NULL DEFAULT '0',
  `alive` TINYINT(1) NULL DEFAULT NULL,
  `nom_ack` VARCHAR(97) COLLATE utf8_unicode_ci DEFAULT NULL,
  `date_create` DATETIME(2) NULL,
  `date_fixe` DATETIME(2) NULL,
  `date_fin` DATETIME(2) NULL,
  `libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE (`id_msg`,`alive`),
  KEY `date_create` (`date_create`),
  KEY `alive` (`alive`),
  FOREIGN KEY (`id_msg`) REFERENCES `msgs` (`id`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `rfxcom`
--

CREATE TABLE IF NOT EXISTS `rfxcom` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `instance_id` text NOT NULL,
  `type` int(11) NOT NULL DEFAULT '0',
  `sstype` int(11) NOT NULL DEFAULT '0',
  `id1` int(11) NOT NULL DEFAULT '0',
  `id2` int(11) NOT NULL DEFAULT '0',
  `id3` int(11) NOT NULL DEFAULT '0',
  `id4` int(11) NOT NULL DEFAULT '0',
  `housecode` int(11) NOT NULL DEFAULT '0',
  `unitcode` int(11) NOT NULL DEFAULT '0',
  `map_E` int(11) NOT NULL DEFAULT '-1',
  `map_EA` int(11) NOT NULL DEFAULT '-1',
  `map_A` int(11) NOT NULL DEFAULT '-1',
  `libelle` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `users`
--

CREATE TABLE IF NOT EXISTS `users` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,
  `access_level` int(11) NOT NULL DEFAULT '0',
  `salt` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `hash` varchar(255) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `email` varchar(120) COLLATE utf8_unicode_ci NOT NULL,
  `comment` varchar(240) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `login_attempts` int(11) NOT NULL DEFAULT '0',
  `enable` tinyint(1) NOT NULL DEFAULT '0',
  `date_create` DATETIME NOT NULL DEFAULT NOW(),
  `date_modif` DATETIME NOT NULL DEFAULT NOW(),
  `notification` tinyint(1) NOT NULL DEFAULT '0',
  `phone` varchar(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `allow_cde` tinyint(1) NOT NULL DEFAULT '0',
  `xmpp` varchar(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `imsg_available` tinyint(1) NOT NULL DEFAULT '0',
  `ssrv_bit_presence` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY(`username`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

INSERT INTO `users` (`id`, `enable`, `access_level`, `username`, `salt`, `hash`, `comment`, `date_create`, `date_modif`) VALUES
(0, 1, 9, 'root', '539c14d7b1d4205f7bb259f0ea941cd0', '510ccdeda455945773552be07941cfd1d03fc17f0f09b4d93f9d4dd8b6deaff8', 'Root user ', NOW(), NOW()),
(1, 1, 0, 'guest', 'c607ac2c103d731de0cc549f90095734', '4ef847cbf200e403327cce2c8cad26d34442073b4b3c72631631d849875935f3', 'Guest user ', NOW(), NOW());

CREATE TABLE IF NOT EXISTS `users_sessions` (
  `username` VARCHAR(32) NOT NULL,
  `host` VARCHAR(32) NOT NULL,
  `wtd_session` VARCHAR(42) NOT NULL,
  `last_request` INT(11) NOT NULL,
  FOREIGN KEY (`username`) REFERENCES `users` (`username`) ON DELETE CASCADE ON UPDATE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `audit_log` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `access_level` int(11) NOT NULL DEFAULT '0',
  `message` varchar(256) COLLATE utf8_unicode_ci NOT NULL,
  `date` DATETIME NOT NULL DEFAULT NOW(),
  PRIMARY KEY (`id`),
  KEY (`date`),
  KEY (`username`)
) ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;
