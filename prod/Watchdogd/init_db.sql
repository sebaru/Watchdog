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
  `instance_id` varchar(80) COLLATE utf8_unicode_ci NOT NULL,
  `nom_thread` varchar(80) COLLATE utf8_unicode_ci NOT NULL,
  `nom` varchar(80) COLLATE utf8_unicode_ci NOT NULL,
  `valeur` varchar(128) COLLATE utf8_unicode_ci NOT NULL,
  UNIQUE (`instance_id`,`nom_thread`,`nom`)
) ENGINE=ARIA DEFAULT CHARSET=utf8 COLLATE utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `cameras`
--

CREATE TABLE IF NOT EXISTS `cameras` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `location` varchar(600) NOT NULL,
  `libelle` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `class`
--

CREATE TABLE IF NOT EXISTS `class` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `date_create` datetime NOT NULL DEFAULT NOW(),
  `libelle` varchar(241) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `icons`
--

CREATE TABLE IF NOT EXISTS `icons` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` varchar(4) NOT NULL DEFAULT 'svg',
  `date_create` datetime NOT NULL DEFAULT NOW(),
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `id_classe` int(11) NOT NULL DEFAULT '0',
  `nbr_matrice` INT(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  FOREIGN KEY (`id_classe`) REFERENCES `class` (`id`) ON DELETE CASCADE
) ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

-- --------------------------------------------------------

--
-- Structure de la table `syns`
--

CREATE TABLE IF NOT EXISTS `syns` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `parent_id` int(11) NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `page` text COLLATE utf8_unicode_ci NOT NULL,
  `access_level` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  FOREIGN KEY (`parent_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;
INSERT INTO `syns` (`id`, `parent_id`, `libelle`, `page`, `access_level` ) VALUES
(1, 1, 'Accueil', 'Defaut Page', 0);

-- --------------------------------------------------------

--
-- Structure de la table `dls`
--

CREATE TABLE IF NOT EXISTS `dls` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
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
  PRIMARY KEY (`id`),
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE
) ENGINE=INNODB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;
INSERT INTO `dls` (`id`, `syn_id`, `name`, `shortname`, `tech_id`, `actif`, `compil_date`, `compil_status` ) VALUES
(1, 1, 'Système', 'Système', 'SYS', FALSE, 0, 0);

-- --------------------------------------------------------

--
-- Structure de la table `mnemos`
--

CREATE TABLE IF NOT EXISTS `mnemos` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` int(11) NOT NULL DEFAULT '0',
  `num` int(11) NOT NULL DEFAULT '0',
  `dls_id` int(11) NOT NULL DEFAULT '0',
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `ev_host` VARCHAR(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',
  `ev_thread` VARCHAR(20) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',
  `ev_text` VARCHAR(160) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `tableau` text COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `acro_syn` text COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE (`dls_id`,`acronyme`),
  FOREIGN KEY (`dls_id`) REFERENCES `dls` (`id`) ON DELETE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

INSERT INTO `mnemos` (`id`, `type`, `num`, `dls_id`, `acronyme`, `libelle`, `ev_text`) VALUES
(01, 0,   0, 1, 'SYS_TOGGLE_RUN', 'Change d''Ã©tat tous les tours programme', ''),
(02, 0,   1, 1, 'SYS_ALWAYS_0', 'Toujours Ã  0', ''),
(03, 0,   2, 1, 'SYS_ALWAYS_1', 'Toujours Ã  1', ''),
(04, 0,   3, 1, 'SYS_ONE_RUN', '0 le premier tour programme puis tout le temps 1', ''),
(05, 0,   4, 1, 'SYS_TICK_1S', 'Cligno toutes les secondes', ''),
(06, 0,   5, 1, 'SYS_TICK_0.5S', 'Cligno toutes les demi-secondes', ''),
(07, 0,   6, 1, 'SYS_TICK_0.3S', 'Cligno toutes les 3 dixièmes de seconde', ''),
(08, 0,   7, 1, 'SYS_SHUTDOWN', 'System is halting', ''),
(09, 0,   8, 1, 'SYS_REBOOT', 'System is rebooting', ''),
(10, 5,  -1, 1, 'DLS_WAIT', 'Number of milli-second to wait to get target turn/sec', ''),
(11, 5,  -1, 1, 'DLS_TOUR_PER_SEC', 'Number of D.L.S turn in second', ''),
(12, 5,  -1, 1, 'DLS_BIT_PER_SEC', 'Number of bits toggled in one second', ''),
(13, 5,  -1, 1, 'ARCH_REQUEST_NUMBER', 'Number of ArchiveRequest to proceed', ''),
(14, 5,  -1, 1, 'DB_REQUEST_SIMULT', 'Number of simultaneous SQL request', ''),
(15, 7,   1, 1, 'SYS_I1', 'Motif toujours en mode 1 couleur rouge', ''),
(16, 7,   4, 1, 'SYS_I4', 'rÃ©servÃ©', ''),
(17, 7,   3, 1, 'SYS_I3', 'rÃ©servÃ©', ''),
(18, 7,   0, 1, 'SYS_I0', 'rÃ©servÃ©', ''),
(19, 7,   2, 1, 'SYS_I2', 'rÃ©servÃ©', ''),
(20, 1,   4, 1, 'SYS_AUDIO_START', 'Emission de message Audio.', ''),
(21, 1,   5, 1, 'SYS_AUDIO_END', 'Fin d''emission de message Audio.', ''),
(22, 1,   6, 1, 'SYS_AUDIO_INHIB', 'Inhibition des messages vocaux (hors alerte).', ''),
(23, 3,9999, 1, 'EVENT_NONE_TOR', 'Used for detected Event with no mapping yet.', ''),
(24, 5,9999, 1, 'EVENT_NONE_ANA', 'Used for detected Event with no mapping yet.', ''),
(25, 5, 122, 1, 'SYS_EA_122', 'Reserved for internal use', ''),
(26, 5, 121, 1, 'SYS_EA_121', 'Reserved for internal use', ''),
(27, 5, 120, 1, 'SYS_EA_120', 'Reserved for internal use', ''),
(28, 5, 119, 1, 'SYS_EA_119', 'Reserved for internal use', ''),
(29, 5, 118, 1, 'SYS_EA_118', 'Reserved for internal use', ''),
(30, 5, 117, 1, 'SYS_EA_117', 'Reserved for internal use', ''),
(31, 5, 116, 1, 'SYS_EA_116', 'Reserved for internal use', ''),
(32, 5, 115, 1, 'SYS_EA_115', 'Reserved for internal use', ''),
(33, 5, 114, 1, 'SYS_EA_114', 'Reserved for internal use', ''),
(34, 5, 113, 1, 'SYS_EA_113', 'Reserved for internal use', ''),
(35, 5, 112, 1, 'SYS_EA_112', 'Reserved for internal use', ''),
(36, 5, 111, 1, 'SYS_EA_111', 'Reserved for internal use', ''),
(37, 5, 110, 1, 'SYS_EA_110', 'Reserved for internal use', ''),
(38, 5, 109, 1, 'SYS_EA_109', 'Reserved for internal use', ''),
(39, 5, 108, 1, 'SYS_EA_108', 'Reserved for internal use', ''),
(40, 5, 107, 1, 'SYS_EA_107', 'Reserved for internal use', ''),
(41, 5, 106, 1, 'SYS_EA_106', 'Reserved for internal use', ''),
(42, 5, 105, 1, 'SYS_EA_105', 'Reserved for internal use', ''),
(43, 5, 104, 1, 'SYS_EA_104', 'Reserved for internal use', ''),
(44, 5, 103, 1, 'SYS_EA_103', 'Reserved for internal use', ''),
(45, 5, 102, 1, 'SYS_EA_102', 'Reserved for internal use', ''),
(46, 5, 101, 1, 'SYS_EA_101', 'Reserved for internal use', ''),
(47, 5, 100, 1, 'SYS_EA_100', 'Reserved for internal use', ''),
(48, 1,  30, 1, 'SYS_AUDIO_HP01', 'Ordre systeme Activation Haut Parleur HP01', ''),
(49, 1,  31, 1, 'SYS_AUDIO_HP02', 'Ordre systeme Activation Haut Parleur HP02', ''),
(50, 1,  32, 1, 'SYS_AUDIO_HP03', 'Ordre systeme Activation Haut Parleur HP03', ''),
(51, 1,  33, 1, 'SYS_AUDIO_HP04', 'Ordre systeme Activation Haut Parleur HP04', ''),
(52, 1,  34, 1, 'SYS_AUDIO_HP05', 'Ordre systeme Activation Haut Parleur HP05', ''),
(53, 1,  35, 1, 'SYS_AUDIO_HP06', 'Ordre systeme Activation Haut Parleur HP06', ''),
(54, 1,  36, 1, 'SYS_AUDIO_HP07', 'Ordre systeme Activation Haut Parleur HP07', ''),
(55, 1,  37, 1, 'SYS_AUDIO_HP08', 'Ordre systeme Activation Haut Parleur HP08', ''),
(56, 1,  38, 1, 'SYS_AUDIO_HP09', 'Ordre systeme Activation Haut Parleur HP09', ''),
(57, 1,  39, 1, 'SYS_AUDIO_HP10', 'Ordre systeme Activation Haut Parleur HP10', ''),
(58, 1,  40, 1, 'SYS_AUDIO_HP11', 'Ordre systeme Activation Haut Parleur HP11', ''),
(59, 1,  41, 1, 'SYS_AUDIO_HP12', 'Ordre systeme Activation Haut Parleur HP12', ''),
(60, 1,  42, 1, 'SYS_AUDIO_HP13', 'Ordre systeme Activation Haut Parleur HP13', ''),
(61, 1,  43, 1, 'SYS_AUDIO_HP14', 'Ordre systeme Activation Haut Parleur HP14', ''),
(62, 1,  44, 1, 'SYS_AUDIO_HP15', 'Ordre systeme Activation Haut Parleur HP15', ''),
(63, 1,  45, 1, 'SYS_AUDIO_HP16', 'Ordre systeme Activation Haut Parleur HP16', ''),
(64, 1,  46, 1, 'SYS_AUDIO_HP17', 'Ordre systeme Activation Haut Parleur HP17', ''),
(65, 1,  47, 1, 'SYS_AUDIO_HP18', 'Ordre systeme Activation Haut Parleur HP18', ''),
(66, 1,  48, 1, 'SYS_AUDIO_HP19', 'Ordre systeme Activation Haut Parleur HP19', ''),
(67, 1,  49, 1, 'SYS_AUDIO_HP20', 'Ordre systeme Activation Haut Parleur HP20', ''),
(68, 1,  50, 1, 'SYS_AUDIO_HP21', 'Ordre systeme Activation Haut Parleur HP21', ''),
(69, 1,  51, 1, 'SYS_AUDIO_HP22', 'Ordre systeme Activation Haut Parleur HP22', ''),
(70, 1,  52, 1, 'SYS_AUDIO_HP23', 'Ordre systeme Activation Haut Parleur HP23', ''),
(71, 1,  53, 1, 'SYS_AUDIO_HP24', 'Ordre systeme Activation Haut Parleur HP24', ''),
(72, 1,  54, 1, 'SYS_AUDIO_HP25', 'Ordre systeme Activation Haut Parleur HP25', ''),
(73, 1,  55, 1, 'SYS_AUDIO_HP26', 'Ordre systeme Activation Haut Parleur HP26', ''),
(74, 1,  56, 1, 'SYS_AUDIO_HP27', 'Ordre systeme Activation Haut Parleur HP27', ''),
(75, 1,  57, 1, 'SYS_AUDIO_HP28', 'Ordre systeme Activation Haut Parleur HP28', ''),
(76, 1,  58, 1, 'SYS_AUDIO_HP29', 'Ordre systeme Activation Haut Parleur HP29', ''),
(77, 1,  59, 1, 'SYS_AUDIO_HP30', 'Ordre systeme Activation Haut Parleur HP30', ''),
(110, 1, 07, 1, 'SYS_EVENT_NOT_FOUND', 'Event not found', ''),
(111, 1, 08, 1, 'SYS_NEW_TICK', 'Default Command by Tick', '');

-- --------------------------------------------------------

--
-- Structure de la table `tableau`
--

CREATE TABLE IF NOT EXISTS `tableau` (
 `id` INT NOT NULL AUTO_INCREMENT ,
 `titre` VARCHAR(128) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL ,
 `access_level` int(11) NOT NULL ,
 `date_create` DATETIME NOT NULL ,
 PRIMARY KEY (`id`)) ENGINE = InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `courbes`
--

CREATE TABLE IF NOT EXISTS `courbes` (
  `id` INT NOT NULL AUTO_INCREMENT ,
  `tableau_id` INT NOT NULL ,
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `color` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`),
  INDEX (`tableau_id`),
  FOREIGN KEY (`tableau_id`) REFERENCES `tableau` (`id`) ON DELETE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos`
--

CREATE TABLE IF NOT EXISTS `mnemos_BOOL` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` int(11) NOT NULL DEFAULT 0,
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `etat` BOOLEAN NOT NULL DEFAULT 0,
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;


-- --------------------------------------------------------

--
-- Structure de la table `mnemos`
--

CREATE TABLE IF NOT EXISTS `mnemos_DI` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `src_host` VARCHAR(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',
  `src_thread` VARCHAR(20) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',
  `src_text` VARCHAR(160) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos`
--

CREATE TABLE IF NOT EXISTS `mnemos_DO` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `dst_host` VARCHAR(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',
  `dst_thread` VARCHAR(20) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',
  `dst_tag` VARCHAR(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `dst_param1` VARCHAR(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos_AI`
--

CREATE TABLE IF NOT EXISTS `mnemos_AI` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `type` int(11) NOT NULL DEFAULT '0',
  `min` float NOT NULL DEFAULT '0',
  `max` float NOT NULL DEFAULT '0',
  `unite` text COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `map_host` VARCHAR(40) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',
  `map_thread` VARCHAR(20) COLLATE utf8_unicode_ci NOT NULL DEFAULT '*',
  `map_text` VARCHAR(160) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `map_question_vocale` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `map_reponse_vocale` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'aucun',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `dls_cpt_imp`
--

CREATE TABLE IF NOT EXISTS `mnemos_CI` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `etat` BOOLEAN NOT NULL DEFAULT '0',
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `valeur` INT(11) NOT NULL DEFAULT '0',
  `multi` float NOT NULL DEFAULT '1',
  `unite` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'fois',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;


-- --------------------------------------------------------

--
-- Structure de la table `mnemos_CH`
--

CREATE TABLE IF NOT EXISTS `mnemos_CH` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `etat` BOOLEAN NOT NULL DEFAULT '0',
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  `valeur` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;


-- --------------------------------------------------------

--
-- Structure de la table `tempo`
--

CREATE TABLE IF NOT EXISTS `mnemos_Tempo` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;


-- --------------------------------------------------------

--
-- Structure de la table `mnemos_Horloge`
--

CREATE TABLE IF NOT EXISTS `mnemos_HORLOGE` (
  `id` INT(11) NOT NULL AUTO_INCREMENT,
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL DEFAULT 'default',
  PRIMARY KEY (`id`),
  UNIQUE (`tech_id`,`acronyme`),
  FOREIGN KEY (`tech_id`) REFERENCES `dls` (`tech_id`) ON DELETE CASCADE
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
  PRIMARY KEY (`id`),
  FOREIGN KEY (`horloge_id`) REFERENCES `mnemos_HORLOGE` (`id`) ON DELETE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos_CptHoraire`
--

CREATE TABLE IF NOT EXISTS `mnemos_CptHoraire` (
  `id_mnemo` int(11) NOT NULL DEFAULT '0',
  `valeur` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id_mnemo`),
  FOREIGN KEY (`id_mnemo`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE
) ENGINE=ARIA DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `dls_cpt_imp`
--

CREATE TABLE IF NOT EXISTS `mnemos_CptImp` (
  `id_mnemo` int(11) NOT NULL,
  `valeur` float NOT NULL DEFAULT '0',
  `type_ci` int(11) NOT NULL DEFAULT '0',
  `multi` float NOT NULL DEFAULT '1',
  `unite_string` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id_mnemo`),
  FOREIGN KEY (`id_mnemo`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE
) ENGINE=ARIA DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos_AnalogInput`
--

CREATE TABLE IF NOT EXISTS `mnemos_AnalogInput` (
  `id_mnemo` int(11) NOT NULL,
  `type` int(11) NOT NULL,
  `min` float NOT NULL DEFAULT '0',
  `max` float NOT NULL DEFAULT '0',
  `unite` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id_mnemo`),
  FOREIGN KEY (`id_mnemo`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE
) ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos_Registre`
--

CREATE TABLE IF NOT EXISTS `mnemos_Registre` (
  `id_mnemo` int(11) NOT NULL,
  `unite` text COLLATE utf8_unicode_ci NOT NULL,
  FOREIGN KEY (`id_mnemo`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE
) ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

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
  FOREIGN KEY (`camera_src_id`) REFERENCES `cameras` (`id`) ON DELETE CASCADE,
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE
) ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_cadrans`
--

CREATE TABLE IF NOT EXISTS `syns_cadrans` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `syn_id` int(11) NOT NULL DEFAULT '0',
  `bitctrl` int(11) NOT NULL DEFAULT '0',
  `posx` int(11) NOT NULL DEFAULT '0',
  `posy` int(11) NOT NULL DEFAULT '0',
  `type` int(11) NOT NULL DEFAULT '0',
  `angle` int(11) NOT NULL DEFAULT '0',
  `fleche_left` tinyint(1) NOT NULL DEFAULT '0',
  `nb_decimal` int(11) NOT NULL DEFAULT '2',
  PRIMARY KEY (`id`),
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE
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
  `rouge` int(11) NOT NULL DEFAULT '0',
  `vert` int(11) NOT NULL DEFAULT '0',
  `bleu` int(11) NOT NULL DEFAULT '0',
  `posx` int(11) NOT NULL DEFAULT '0',
  `posy` int(11) NOT NULL DEFAULT '0',
  `angle` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE
) ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_motifs`
--

CREATE TABLE IF NOT EXISTS `syns_motifs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `icone` int(11) NOT NULL DEFAULT '0',
  `syn_id` int(11) NOT NULL DEFAULT '0',
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `access_level` int(11) NOT NULL DEFAULT '0',
  `bitctrl` int(11) NOT NULL DEFAULT '0',
  `bitclic` int(11) NOT NULL DEFAULT '0',
  `rafraich` int(11) NOT NULL DEFAULT '0',
  `posx` int(11) NOT NULL DEFAULT '0',
  `posy` int(11) NOT NULL DEFAULT '0',
  `larg` int(11) NOT NULL DEFAULT '0',
  `haut` int(11) NOT NULL DEFAULT '0',
  `angle` int(11) NOT NULL DEFAULT '0',
  `scale` float NOT NULL DEFAULT '1',
  `dialog` int(11) NOT NULL DEFAULT '0',
  `gestion` int(11) NOT NULL DEFAULT '0',
  `rouge` int(11) NOT NULL DEFAULT '0',
  `vert` int(11) NOT NULL DEFAULT '0',
  `bleu` int(11) NOT NULL DEFAULT '0',
  `layer` int(11) NOT NULL DEFAULT '0',
  `mnemo_id` int(11) NOT NULL DEFAULT '0',
  `tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `def_color` varchar(16) COLLATE utf8_unicode_ci NOT NULL DEFAULT "#c0c0c0",
  `clic_tech_id` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `clic_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  PRIMARY KEY (`id`),
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE,
  FOREIGN KEY (`mnemo_id`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE
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
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE,
  FOREIGN KEY (`syn_cible_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE
) ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

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
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE,
  FOREIGN KEY (`syn_cible_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE
) ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

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
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE
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
  FOREIGN KEY (`syn_id`) REFERENCES `syns` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

-- --------------------------------------------------------

--
-- Structure de la table `modbus_modules`
--

CREATE TABLE IF NOT EXISTS `modbus_modules` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `date_create` datetime NOT NULL DEFAULT NOW(),
  `enable` tinyint(1) NOT NULL,
  `hostname` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',
  `tech_id` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT hostname,
  `description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',
  `watchdog` int(11) NOT NULL,
  `bit` int(11) NOT NULL,
  `map_E` int(11) NOT NULL,
  `max_nbr_E` int(11) NOT NULL,
  `map_EA` int(11) NOT NULL,
  `map_A` int(11) NOT NULL,
  `map_AA` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;


-- --------------------------------------------------------

--
-- Structure de la table `msgs`
--

CREATE TABLE IF NOT EXISTS `msgs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `acronyme` VARCHAR(64) COLLATE utf8_unicode_ci NULL DEFAULT NULL,
  `num` int(11) NOT NULL DEFAULT '0',
  `dls_id` int(11) NOT NULL DEFAULT '1',
  `libelle` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT "No libelle",
  `libelle_audio` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT "No audio",
  `libelle_sms` VARCHAR(256) COLLATE utf8_unicode_ci NOT NULL DEFAULT "No sms",
  `type` int(11) NOT NULL DEFAULT '0',
  `audio` tinyint(1) NOT NULL DEFAULT '0',
  `bit_audio` int(11) DEFAULT NULL DEFAULT '0',
  `enable` tinyint(1) NOT NULL DEFAULT '0',
  `persist` tinyint(1) NOT NULL DEFAULT '0',
  `sms` int(11) NOT NULL DEFAULT '0',
  `time_repeat` int(11) NOT NULL DEFAULT '0',
  `is_mp3` tinyint(1) NOT NULL DEFAULT '0',
  `etat` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE(`dls_id`,`acronyme`),
  FOREIGN KEY (`dls_id`) REFERENCES `dls` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

INSERT INTO `msgs` (`id`, `num`, `dls_id`, `libelle`, `libelle_audio`, `libelle_sms`, `type`, `enable`, `sms` ) VALUES
(1, 0, 1, 'Warning, system is halting', 'Warning, system is halting', 'Warning, system is halting', 1, TRUE, FALSE ),
(2, 1, 1, 'Warning, system is rebooting', 'Warning, system is rebooting', 'Warning, system is rebooting', 1, TRUE, FALSE );

-- --------------------------------------------------------

--
-- Structure de la table `histo_msgs`
--

CREATE TABLE IF NOT EXISTS `histo_msgs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `id_msg` int(11) NOT NULL DEFAULT '0',
  `alive` tinyint(1) NULL DEFAULT NULL,
  `nom_ack` varchar(97) COLLATE utf8_unicode_ci DEFAULT NULL,
  `date_create` DATETIME(2) NULL,
  `date_fixe` DATETIME(2) NULL,
  `date_fin` DATETIME(2) NULL,
  PRIMARY KEY (`id`),
  UNIQUE (`id_msg`,`alive`),
  KEY `date_create` (`date_create`),
  KEY `alive` (`alive`),
  FOREIGN KEY (`id_msg`) REFERENCES `msgs` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `ups`
--

CREATE TABLE IF NOT EXISTS `ups` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT "NEW",
  `enable` tinyint(1) NOT NULL,
  `host` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `name` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `username` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `password` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `map_EA` int(11) NOT NULL,
  `map_E` int(11) NOT NULL,
  `map_A` int(11) NOT NULL,
  `date_create` DATETIME NOT NULL DEFAULT NOW(),

  PRIMARY KEY (`id`)
) ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;


-- --------------------------------------------------------
--
-- Structure de la table ``
--

CREATE TABLE IF NOT EXISTS `rs485` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `host` text NOT NULL,
  `date_ajout` DATETIME NOT NULL,
  `num` int(11) NOT NULL DEFAULT '0',
  `enable` tinyint(1) NOT NULL DEFAULT '0',
  `ea_min` int(11) NOT NULL DEFAULT '-1',
  `ea_max` int(11) NOT NULL DEFAULT '-1',
  `e_min` int(11) NOT NULL DEFAULT '-1',
  `forced_e_min` int(11) NOT NULL DEFAULT '0',
  `e_max` int(11) NOT NULL DEFAULT '-1',
  `s_min` int(11) NOT NULL DEFAULT '-1',
  `s_max` int(11) NOT NULL DEFAULT '-1',
  `sa_min` int(11) NOT NULL DEFAULT '-1',
  `sa_max` int(11) NOT NULL DEFAULT '-1',
  `bit_comm` int(11) NOT NULL DEFAULT '0',
  `libelle` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

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
) ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `users`
--

CREATE TABLE IF NOT EXISTS `users` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,
  `access_level` int(11) NOT NULL DEFAULT '0',
  `hash` varchar(255) COLLATE utf8_unicode_ci NOT NULL,
  `email` varchar(120) COLLATE utf8_unicode_ci DEFAULT NULL,
  `comment` varchar(240) COLLATE utf8_unicode_ci DEFAULT NULL,
  `login_attempts` int(11) NOT NULL DEFAULT '0',
  `enable` tinyint(1) NOT NULL DEFAULT '0',
  `date_create` DATETIME NOT NULL DEFAULT NOW(),
  `date_modif` DATETIME DEFAULT NULL,
  `sms_enable` tinyint(1) NOT NULL DEFAULT '0',
  `sms_phone` varchar(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `sms_allow_cde` tinyint(1) NOT NULL DEFAULT '0',
  `imsg_enable` tinyint(1) NOT NULL DEFAULT '0',
  `imsg_jabberid` varchar(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT "",
  `imsg_allow_cde` tinyint(1) NOT NULL DEFAULT '0',
  `imsg_available` tinyint(1) NOT NULL DEFAULT '0',
  `ssrv_bit_presence` int(11) NOT NULL DEFAULT '0',
  `session_id` varchar(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'NONE',
  PRIMARY KEY (`id`),
  KEY(`username`)
) ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

INSERT INTO `users` (`id`, `enable`, `access_level`, `username`, `hash`, `comment`, `date_create`, `date_modif`) VALUES
(0, 1, 10, 'root', '$2y$10$9TVOoxmzBJTl6knJ0plKHOCsoSvSSMiPrldhanBKVApFIF3083x6a', 'Root user ', NOW(), NOW()),
(1, 1, 0, 'guest', '$2y$10$9TVOoxmzBJTl6knJ0plKHOCsoSvSSMiPrldhanBKVApFIF3083x6a', 'Guest user ', NOW(), NOW());

CREATE TABLE `users_sessions` (
  `id` VARCHAR(128) NOT NULL,
  `login` VARCHAR(32) NOT NULL,
  `last_date` datetime NOT NULL,
  `remote_addr` VARCHAR(50) NOT NULL,
  `x_forwarded_for` VARCHAR(50) NOT NULL,
  `data` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

CREATE TABLE IF NOT EXISTS `audit_log` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,
  `access_level` int(11) NOT NULL DEFAULT '0',
  `message` varchar(256) COLLATE utf8_unicode_ci NOT NULL,
  `date` DATETIME NOT NULL DEFAULT NOW(),
  PRIMARY KEY (`id`)
) ENGINE=ARIA  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;
