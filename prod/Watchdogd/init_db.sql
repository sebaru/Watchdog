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
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Base de données: `WatchdogDB`
--

-- --------------------------------------------------------

--
-- Structure de la table `cameras`
--

CREATE TABLE IF NOT EXISTS `cameras` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `location` varchar(600) NOT NULL,
  `type` int(11) NOT NULL,
  `bit` int(11) NOT NULL,
  `objet` text NOT NULL,
  `libelle` text NOT NULL,
  `num` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `class`
--

CREATE TABLE IF NOT EXISTS `class` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `libelle` varchar(241) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

INSERT INTO `class` (`id`, `libelle`) VALUES
(1, 'Divers'),
(4, 'Etats_des_Commandes'),
(5, 'Plans du Site'),
(6, 'Commandes_Manu'),
(7, 'Indicateurs'),
(16, 'Capteurs_Statiques'),
(9, 'Actionneurs'),
(10, 'Ouvrants_Habitat'),
(11, 'Alerter'),
(18, 'Outillage'),
(14, 'Composants_Hydrauliques'),
(21, 'Composants_Infrastructures'),
(17, 'Composants_Electrique'),
(19, 'Fond_d''Ã©cran'),
(20, 'DÃ©tecter'),
(22, 'Fils_Electriques'),
(23, 'Tuyaux_Plomberie'),
(24, 'Capteurs Dynamiques'),
(25, 'Electro_MÃ©nager'),
(26, 'Meubles'),
(27, 'Traitement de l''Air');

-- --------------------------------------------------------

--
-- Structure de la table `dls`
--

CREATE TABLE IF NOT EXISTS `dls` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` int(11) NOT NULL,
  `syn_id` int(11) NOT NULL DEFAULT '0',
  `name` text COLLATE utf8_unicode_ci NOT NULL,
  `shortname` text COLLATE utf8_unicode_ci NOT NULL,
  `actif` tinyint(1) NOT NULL DEFAULT '0',
  `compil_date` int(11) NOT NULL,
  `compil_status` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;
INSERT INTO `dls` (`id`, `type`, `syn_id`, `name`, `shortname`, `actif`, `compil_date`, `compil_status` ) VALUES
(1, 0, 1, 'Systeme', 'Systeme', FALSE, 0, 0);

-- --------------------------------------------------------

--
-- Structure de la table `mnemos_CptHoraire`
--

CREATE TABLE IF NOT EXISTS `mnemos_CptHoraire` (
  `id_mnemo` int(11) NOT NULL DEFAULT '0',
  `valeur` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id_mnemo`),
  CONSTRAINT `id_mnemo` FOREIGN KEY (`id_mnemo`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `dls_cpt_imp`
--

CREATE TABLE IF NOT EXISTS `mnemos_CptImp` (
  `id_mnemo` int(11) NOT NULL,
  `valeur` float NOT NULL,
  `type_ci` int(11) NOT NULL,
  `multi` float NOT NULL DEFAULT '1',
  `unite_string` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id_mnemo`),
  CONSTRAINT `id_mnemo` FOREIGN KEY (`id_mnemo`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

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
  CONSTRAINT `id_mnemo` FOREIGN KEY (`id_mnemo`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

INSERT INTO `mnemos_AnalogInput` (`id_mnemo`, `type`, `min`, `max`, `unite`) VALUES
(10, 0, 0, 100, 'ms'),
(11, 0, 0, 100, 't/s'),
(12, 0, 0, 100, 'bit/s'),
(13, 0, 0, 100, 'arch'),
(14, 0, 0, 100, 'dbs');

-- --------------------------------------------------------

--
-- Structure de la table `mnemos_Registre`
--

CREATE TABLE IF NOT EXISTS `mnemos_Registre` (
  `id_mnemo` int(11) NOT NULL,
  `unite` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id_mnemo`),
  CONSTRAINT `id_mnemo` FOREIGN KEY (`id_mnemo`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos_DigitalInput`
--

CREATE TABLE IF NOT EXISTS `mnemos_DigitalInput` (
  `id_mnemo` int(11) NOT NULL,
  PRIMARY KEY (`id_mnemo`),
  CONSTRAINT `id_mnemo` FOREIGN KEY (`id_mnemo`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `gids`
--

CREATE TABLE IF NOT EXISTS `gids` (
  `id_util` int(11) NOT NULL DEFAULT '0',
  `gids` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `groups`
--

CREATE TABLE IF NOT EXISTS `groups` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(97) COLLATE utf8_unicode_ci NOT NULL,
  `comment` varchar(241) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

INSERT INTO `groups` (`id`, `name`, `comment`) VALUES
(0, 'Everybody', 'The default group'),
(1, 'Admin-UserDB', 'Members can add/remove/edit users/groups'),
(2, 'Admin-MsgDB', 'Members can add/remove/edit Msgs'),
(3, 'Admin-iconDB', 'Members can add/remove/edit icons'),
(4, 'Admin-synopDB', 'Members can add/remove/edit syn'),
(5, 'Admin-mnemos', 'Members can manage Mnemonique'),
(6, 'Admin-dlsDB', 'Members can add/remove/edit DLS plugins'),
(7, 'Admin-histoDB', 'Members can ack/query histo'),
(8, 'Admin-scenarioDB', 'Members can add/remove Scenario'),
(9, 'Admin-Lowlevel I/O', 'Configuration MODBUS/RS485/ONDULEUR/...'),
(10, 'Admin-CommandLineInterface', 'Command Line Interface Access'),
(11, 'Satellite', 'Add Satellite Capabilities');

-- --------------------------------------------------------

--
-- Structure de la table `histo_bit`
--

CREATE TABLE IF NOT EXISTS `histo_bit` (
  `type` int(11) NOT NULL DEFAULT '0',
  `num` int(11) NOT NULL DEFAULT '0',
  `date_sec` int(11) NOT NULL DEFAULT '0',
  `date_usec` int(11) NOT NULL DEFAULT '0',
  `date_time` DATETIME(6) NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `valeur` float NOT NULL DEFAULT '0.0',
  PRIMARY KEY `key` (`type`,`num`,`date_time`)  
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci
  PARTITION BY RANGE COLUMNS (date_time) PARTITIONS 2
   ( PARTITION p0 VALUES LESS THAN ('2016-01-01') ENGINE = InnoDB,
     PARTITION p_MAX VALUES LESS THAN MAXVALUE
   );

-- --------------------------------------------------------

--
-- Structure de la table `histo_msgs`
--

CREATE TABLE IF NOT EXISTS `histo_msgs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `id_msg` int(11) NOT NULL DEFAULT '0',
  `alive` tinyint(1) NOT NULL,
  `nom_ack` varchar(97) COLLATE utf8_unicode_ci DEFAULT NULL,
  `date_create_sec` int(11) NOT NULL DEFAULT '0',
  `date_create_usec` int(11) DEFAULT '0',
  `date_fixe` int(11) NOT NULL DEFAULT '0',
  `date_fin` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `date_create_sec` (`date_create_sec`),
  KEY `alive` (`alive`),
  KEY `fk_id_msg` (`id_msg`),
  CONSTRAINT `fk_id_msg` FOREIGN KEY (`id_msg`) REFERENCES `msgs` (`id`) ON DELETE CASCADE
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `icons`
--

CREATE TABLE IF NOT EXISTS `icons` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `id_classe` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;

--
-- Contenu de la table `icons`
--

INSERT INTO `icons` (`id`, `libelle`, `id_classe`) VALUES
(7, 'Tournevis', 18),
(8, 'Poterie', 0),
(12, 'Boule_rouge', 7),
(13, 'Boule_verte', 7),
(15, 'Local_Marche', 4),
(14, 'Local_Arret', 4),
(16, 'Local_SystÃ¨me', 4),
(11, 'CÃ©dÃ©S', 0),
(17, 'Poussoir_Rond_Marche', 6),
(19, 'Nuit_EtoilÃ©', 19),
(20, 'H_noir', 7),
(31, 'Paturage', 19),
(35, 'Poussoir_Ovale_Marche.', 6),
(36, 'Poussoir_Ovale_ ArrÃªt.', 6),
(37, 'Poussoir_Ovale_Acquit_DÃ©faut.', 6),
(38, 'RÃ©veil_Commande_Horaire_ProgrammÃ©e.', 6),
(39, 'Auto_/Manu_/Manu+horloge_/Down', 4),
(40, 'Vue_AÃ©rienne_Soisy', 5),
(25, 'Fil_CourbÃ©_Bas_Droite', 22),
(226, 'Tuyau_Syphon_Grand_ModÃ¨le_Gauche', 23),
(107, 'Sablier_V2', 7),
(211, 'AmpÃ¨remÃ¨tre', 16),
(30, 'PrÃ©sence_U_Eclair', 24),
(33, 'TÃ©lÃ©rupteur_Bipolaire', 9),
(34, 'TÃ©lÃ©rupteur_Bobine', 17),
(23, 'Contacteur_Bipolaire ', 9),
(24, 'Contacteur_Bobine', 17),
(22, 'Batterie', 17),
(41, 'En construction', 0),
(42, 'FenÃªtre un battant', 10),
(44, 'Haut Parleur', 11),
(45, 'Moteur', 9),
(46, 'CarrÃ©', 7),
(32, 'Moteur_Rideaux', 9),
(47, 'CarrÃ©_Noir', 7),
(48, 'Rectangle', 7),
(49, 'Rectangle_Noir', 7),
(182, 'Poussoir_CarrÃ©_ArrÃªt', 6),
(51, 'Voyant', 9),
(186, 'Fil_DÃ©rivation_Ã _Gauche', 22),
(53, 'Bouteille', 16),
(54, 'Girouette_2', 16),
(56, 'Niveau Site GÃ©nÃ©ral de Soisy.', 5),
(57, 'Niveau -1', 5),
(59, 'Chauffe Eau', 14),
(60, 'RÃ©sistance Chauffage', 9),
(63, 'FlÃ¨che_Haut_Gauche', 7),
(70, 'Gyrophare rouge', 11),
(72, 'Bonhomme en marche', 0),
(76, '3xPignons_3D', 0),
(77, 'Disquette', 0),
(78, 'Girouette', 16),
(79, 'Gyrophare_Gros', 11),
(80, 'Tarif Jour ou nuit', 16),
(189, 'Fil_Horizontal', 22),
(82, 'Bonhomme marcheur_2', 0),
(83, 'Bonhomme marcheur_3', 0),
(84, 'Habitat_RdChaussÃ©e', 5),
(297, 'Mur_7Px_Vertical_Court', 21),
(86, 'Pointer_du_doigt', 7),
(87, 'Pressostat', 16),
(88, 'Radar', 20),
(89, 'Sablier', 7),
(92, 'FlÃ¨che_Tendance_Basse ', 7),
(93, 'FlÃ¨che_Tendance_Hausse', 7),
(285, 'ThermomÃ¨tre', 16),
(203, 'Tuyau_TÃ©_Bas', 23),
(190, 'Fil_Horizontal_Long', 22),
(108, 'Sablier_V2_Mini', 7),
(204, 'Tuyau_TÃ©_Haut', 23),
(207, 'Tuyau_TÃ©_Droite', 23),
(206, 'Tuyau_TÃ©_Gauche', 23),
(208, 'Tuyau_DÃ©rivation_Croix', 23),
(209, 'Tuyau_Vertical', 23),
(119, 'Vanne_de_RÃ©gulation', 9),
(121, 'Petit_Cadenas_Statique', 7),
(122, 'Photo_Garage', 5),
(123, 'Lapin', 0),
(142, 'Visu_Mode_Manuel', 7),
(125, 'Horloge_2', 7),
(296, 'Porte_Poussant_Droite', 10),
(127, 'Moteur_2', 9),
(291, 'Mur_7Px_Angle_FermÃ©', 21),
(290, 'Mur_7Px_Angle_Court_FermÃ©', 21),
(131, 'Naviguer_Droite', 7),
(132, 'Naviguer_Gauche', 7),
(133, 'Ventilateur', 9),
(134, 'Habitat_Niveau-1', 5),
(135, 'Site_de_Soisy_2', 5),
(136, 'Site Ã  voir', 5),
(137, 'Interrupteur_Unipolaire', 17),
(153, 'Disjoncteur_Bipolaire', 17),
(139, 'Poussoir_Rectangulaire_Auto', 6),
(140, 'Poussoir_Rectangulaire_Manu', 6),
(141, 'Visu_Mode_Horloge', 7),
(187, 'Fil_DÃ©rivation_en_Bas', 22),
(185, 'Fil_DÃ©rivation_Ã _Droite', 22),
(149, 'Poussoir_Rectangulaire_Semi Auto', 6),
(150, 'Poussoir_CarrÃ©_Marche', 6),
(192, 'Fil_Vertical_Long', 22),
(159, 'Fil_Angle_Bas_Droite', 22),
(160, 'Fil_Angle_Bas_Gauche', 22),
(161, 'Fil_Angle_Haut_Droite', 22),
(162, 'Fil_Angle_Haut_Gauche', 22),
(191, 'Fil_Vertical', 22),
(193, 'Fil_DÃ©rivation_en_Croix ', 22),
(184, 'Visu_Mode_SemiAuto', 7),
(166, 'Disjoncteur_Unipolaire_V3', 17),
(167, 'Interrupteur_Unipolaire', 17),
(183, 'Poussoir_CarrÃ©_Acquit_DÃ©faut', 6),
(169, 'Interrupteur_Unipolaire', 17),
(177, 'Contacteur_Bipolaire_Dynamique', 17),
(172, 'AnÃ©momÃ¨tre', 16),
(188, 'Fil_DÃ©rivation_en_Haut', 22),
(210, 'Horloge_EDF', 7),
(229, 'Tuyau_Vertical_CoudÃ©_Droite', 23),
(231, 'Tuyau_CoudÃ©_Bas_Droite_Mini', 23),
(215, 'Tuyau_CoudÃ©_Haut_Droite', 23),
(216, 'Tuyau_CoudÃ©_Haut_Gauche', 23),
(217, 'Tuyau_CoudÃ©_Bas_Gauche', 23),
(218, 'Tuyau_CoudÃ©_Bas_Droite', 23),
(219, 'Tuyau_Vertical_Court', 23),
(220, 'Tuyau_Vertical_Long', 23),
(221, 'Tuyau_Horizontal_Long', 23),
(222, 'Tuyau_Horizontal_Court', 23),
(223, 'Tuyau_Syphon_Gauche', 23),
(224, 'Tuyau-Syphon_Droite', 23),
(225, 'Vanne_Tor_Horizontale', 14),
(227, 'Tuyau_Syphon_Grand_ModÃ¨le_Droite', 23),
(230, 'Tuyau_CoudÃ©_Bas_Gauche_Mini', 23),
(232, 'Tuyau_CoudÃ©_Haut_Droite_Mini', 23),
(233, 'Tuyau_CoudÃ©_Haut_Gauche_Mini', 23),
(234, 'Vanne_Verticale_Droite_Mini', 14),
(235, 'Vanne_Verticale_Gauche_Mini', 14),
(236, 'Vase_Expansion', 14),
(240, 'FlÃ¨che_Horizontale_Gauche', 7),
(238, 'FlÃ¨che_Verticale_Bas', 7),
(239, 'FlÃ¨che_Horizontale_Droite', 7),
(241, 'FlÃ¨che_Verticale_Haut', 7),
(242, 'FlÃ¨che_Verticale_Haut_Maxi', 7),
(243, 'FlÃ¨che_Horizontale_Droite_Maxi', 7),
(244, 'FlÃ¨che_Verticale_Bas_Maxi', 7),
(245, 'FlÃ¨che_Horizontale_Gauche_Maxi', 7),
(246, 'Vanne_Horizontale_Bas_Mini', 14),
(247, 'Vanne_Horizontale_Haut_Mini', 14),
(248, 'Soupape_Verticale_Gauche_Mini', 14),
(249, 'Soupape_Verticale_Droite_Mini', 14),
(250, 'Soupape_Horizontale_Haut_Mini', 14),
(251, 'Soupape_Horizontale_Bas_Mini', 14),
(252, 'Soupape_Horizontale_Bas', 14),
(253, 'Soupape_Horizontale_Haut', 14),
(254, 'Soupape_Verticale_Droite', 14),
(255, 'Soupape_Verticale_Gauche', 14),
(256, 'Sigle_Abls_Mini', 0),
(257, 'Voyant_12X12', 7),
(258, 'Voyant_16X16', 7),
(259, 'DÃ©tendeur_Horizontal', 14),
(260, 'DÃ©tendeur_Vertical', 14),
(264, 'DÃ©tendeur_Vertical_Mini', 14),
(263, 'DÃ©tendeur_Horizontal_Mini', 14),
(265, 'Clapet-Horizontal_Droite_Mini', 14),
(266, 'Clapet_Horizontal_Gauche_Mini', 14),
(267, 'Clapet_Vertical_Haut_Mini', 14),
(268, 'Clapet_Vertical_Bas_Mini', 14),
(269, 'Clapet_Vertical_Bas', 14),
(270, 'Clapet_Vertical_Haut', 14),
(271, 'Clapet_Horizontal_Droite', 14),
(272, 'Clapet_Horizontal_Gauche', 14),
(273, 'Pressostat', 24),
(274, 'Pressostat', 0),
(275, 'Pressostat_Mini', 0),
(276, 'Compteur_Mini', 24),
(277, 'Compteur', 24),
(278, 'AmpÃ¨remÃ¨tre_Bas_Mini', 24),
(279, 'AmpÃ¨remÃ¨tre_Droite_Mini', 24),
(280, 'Poussoir_CarrÃ©_FlÃ¨che_Haut', 6),
(281, 'Poussoir_CarrÃ©_FlÃ¨che_Bas', 6),
(282, 'Volet_Roulant_Toit', 10),
(283, 'Lucarne_Outeau', 10),
(284, 'FenÃªtre_Toit', 10),
(286, 'Mur_10Px_Angle_Bas_Droite', 21),
(287, 'Mur_7Px_Angle_Bas_Droite', 21),
(288, 'Mur_7px_Horizontal_FermÃ©_Droite', 21),
(292, 'Mur_Long_Horizontal', 21),
(295, 'Mur_Grand_Angle_FermÃ©', 21),
(294, 'Mur_7Px_Vertical_Long', 21),
(298, 'Gyrophare_Statique', 11),
(304, 'Porte IntÃ©rieure', 10),
(303, 'CongÃ©lateur', 25),
(305, 'Porte IntÃ©rieure miroir', 10),
(306, 'Cloison IntÃ©rieure_4Px', 21),
(307, 'Cloison IntÃ©rieure croix', 21),
(308, 'Cloison IntÃ©rieure TÃ©', 21),
(309, 'Cloison intÃ©rieure angle', 21),
(315, 'Mur extÃ©rieur 7Px croix', 21),
(313, 'Porte intÃ©rieure grande', 10),
(314, 'Porte intÃ©rieure grande miroir', 10),
(319, 'Porte extÃ©rieure tirant Ã  gauche', 10),
(320, 'Mur_7Px_Angle_Bas_Droite_V2', 21),
(323, 'Mur_7Px_Angle_bas_gauche', 21),
(325, 'FenÃªtre_un_battant_tirant_gauche_7px ', 10),
(326, 'fenÃªtre_un_battant_tirant_droite_7Px', 10),
(327, 'FenÃªtre_un_battant_poussant_droite_7Px', 10),
(328, 'FenÃªtre_2_battants_poussant_7px', 10),
(329, 'Mur_7Px_Angle _Bas_Gauche_Court', 21),
(330, 'Mur_7Px_Angle_Bas_Droite_Court', 21),
(331, 'Porte_roulante ', 10),
(332, 'Porte_DÃ©roulante-Haut ', 10),
(333, 'Mur_7Px_Horizontal_Court', 21),
(334, 'Cloison_intÃ©rieure_longue', 21),
(335, 'Porte intÃ©rieure grande', 10),
(336, 'Porte intÃ©rieure miroir grande', 10),
(337, 'Porte intÃ©rieure grande V2', 10),
(338, 'Mur_7Px_angle_bas_gauche_court_V3', 21),
(339, 'Mur_7Px_angle_bas_gauche_court_V4', 21),
(340, 'Porte_extÃ©rieure_poussant_droite_7Px', 10),
(341, 'Placard 30X60', 26),
(342, 'Placard 30X30', 26),
(343, 'Placard 15X30', 26),
(344, 'Placard horizontal 30X50', 26),
(345, 'Cloison intÃ©rieure courte fermÃ©e', 21),
(346, 'Cloison intÃ©rieure trÃ¨s courte fermÃ©e', 21),
(347, 'CheminÃ©e d''angle 65X65', 27),
(348, 'Escalier tournant Ã  droite 65X65', 21),
(349, 'Escalier tournant Ã  droite 70X220', 21),
(351, 'Escalier Droit Court 45X60', 21),
(352, 'Escalier Droit Court 45X120', 21),
(355, 'Escalier Droit Court 45X100', 21),
(354, 'Auvent 30X60', 21),
(356, 'Escalier droit court et plateau 45X95', 21),
(357, 'Volet un battant', 10),
(371, 'Pressostat', 16),
(375, 'Pompe Arrosage', 17),
(370, 'Pressostat', 16),
(369, 'Bache Eaux Naturelles', 14),
(386, 'Pompe Puit', 17),
(364, 'Puit', 14),
(365, 'Puit', 21),
(367, '', 9),
(368, 'Bache Eau Naturelle', 14),
(372, 'Puit !', 21),
(373, 'ElectroVanne', 9),
(376, 'Pressostat Carre', 16),
(377, 'NTH a droite', 16),
(378, 'NTB a droite', 16),
(379, 'NTH a gauche', 16),
(380, 'NB a droite', 16),
(381, 'NH a droite', 16),
(382, 'Pressostat Carre 3 Px', 16),
(383, 'Pressostat carre base', 16),
(384, 'Tuyau Horizontal AnimÃ© ', 23),
(385, 'Tuyau horizontal Ã©pais AnimÃ©', 23),
(387, 'Tuyau_CoudÃ©_AnimÃ©_Epais ', 23),
(388, 'Tuyau_CoudÃ©_AnimÃ©', 23),
(389, 'Tuyau_TÃ©_AnimÃ©', 23),
(390, 'Pompe_Arrosage-AnimÃ©', 14),
(391, 'Bouton_Marche_AnimÃ© ', 6),
(392, 'Bouton_ArrÃªt_AnimÃ©', 6),
(393, 'Pompe_Puit_AnimÃ© ', 17),
(394, 'Filtre_Eau', 14),
(395, 'Filtre_Eau_Long', 14),
(396, 'GouttiÃ¨re', 23),
(398, 'Ampoule a vis', 17),
(399, 'Applique demi ronde', 17),
(400, 'Applique demi ronde petite', 17),
(401, 'Volet 1 Battant', 10),
(402, 'Volet 1 Battant DÃ©portÃ©', 10),
(403, 'Volets 2 battants', 10),
(405, 'Digicode ExtÃ©rieur', 4),
(406, 'Digicode ExtÃ©rieur Petit', 4),
(407, 'Bouton_Inhibition_AnimÃ©', 6),
(408, 'Bouton_DÃ©sinhibition_AnimÃ©', 6),
(409, 'wago', 1),
(410, 'Niveau TrÃ¨s Bas Ã  Droite', 16),
(423, '', 9),
(412, 'Niveau Bas Ã  Droite', 16),
(414, 'Bouton Maintenance AnimÃ©', 0),
(415, 'ccc', 0),
(416, 'Bouton Maintenance AnimÃ©', 0),
(417, 'Bouton Maintenance Anime', 0),
(418, 'Bouton Silence Anime', 0),
(419, 'Bouton Silence Anime', 6),
(420, 'Bouton Essai AnimÃ©', 6),
(421, 'Bouton Maintenance AnimÃ©', 6),
(422, 'Bouton En Service AnimÃ©', 6),
(424, 'Bouton Stop Klaxon', 6),
(426, 'Volet EntrÃ©e un Battant', 10),
(428, 'Ampoule Vis', 17),
(430, 'Applique', 17),
(431, 'Oeuil Veille', 4),
(432, 'Bouton Veille animÃ©', 6),
(433, 'Bouton Manu AnimÃ©', 6),
(434, 'Bouton In AnimÃ©', 6),
(435, 'Bouton Out AnimÃ©', 6),
(436, 'Vigie Ã©tiquette', 4),
(440, 'Abls mini', 1),
(441, 'ABLS', 1),
(444, 'Compteur prise en bas', 24),
(443, 'Fuite liquide', 16),
(445, 'Compteur en ligne', 24),
(450, 'Vignette Activites', 7),
(449, 'Bouton Acquit', 6),
(453, 'Infra Rouge_V3', 24);

-- --------------------------------------------------------

--
-- Structure de la table `icons_new`
--

CREATE TABLE IF NOT EXISTS `icons_new` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `description` VARCHAR(160) COLLATE utf8_unicode_ci UNIQUE NOT NULL,
  `classe` VARCHAR(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT '0',
  KEY (`classe`),
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000;


--
-- Structure de la table `mnemos`
--

CREATE TABLE IF NOT EXISTS `mnemos` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` int(11) NOT NULL DEFAULT '0',
  `num` int(11) NOT NULL DEFAULT '0',
  `num_plugin` int(11) NOT NULL DEFAULT '0',
  `acronyme` text COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `command_text` text COLLATE utf8_unicode_ci NOT NULL,
  `tableau` text COLLATE utf8_unicode_ci NOT NULL,
  `acro_syn` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`), FULLTEXT(`command_text`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

INSERT INTO `mnemos` (`id`, `type`, `num`, `num_plugin`, `acronyme`, `libelle`, `command_text`) VALUES
(01, 0,   0, 1, 'SYS_TOGGLE_RUN', 'Change d''Ã©tat tous les tours programme', ''),
(02, 0,   1, 1, 'SYS_ALWAYS_0', 'Toujours Ã  0', ''),
(03, 0,   2, 1, 'SYS_ALWAYS_1', 'Toujours Ã  1', ''),
(04, 0,   3, 1, 'SYS_ONE_RUN', '0 le premier tour programme puis tout le temps 1', ''),
(05, 0,   4, 1, 'SYS_TICK_1S', 'Cligno toutes les secondes', ''),
(06, 0,   5, 1, 'SYS_TICK_0.5S', 'Cligno toutes les demi-secondes', ''),
(07, 0,   6, 1, 'SYS_TICK_0.3S', 'Cligno toutes les 3 dixièmes de seconde', ''),
(08, 0,   7, 1, 'SYS_SHUTDOWN', 'System is halting', ''),
(09, 0,   8, 1, 'SYS_REBOOT', 'System is rebooting', ''),
(10, 5, 123, 1, 'SYS_DLS_WAIT', 'Number of milli-second to wait to get target turn/sec', ''),
(11, 5, 124, 1, 'SYS_TOUR_DLS_PER_SEC', 'Number of D.L.S turn in second', ''),
(12, 5, 125, 1, 'SYS_BITS_PER_SEC', 'Number of bits toggled in one second', ''),
(13, 5, 126, 1, 'SYS_ARCHREQUEST', 'Number of ArchiveRequest to proceed', ''),
(14, 5, 127, 1, 'SYS_DBREQUEST_SIMULT', 'Number of simultaneous SQL request', ''),
(15, 7,   1, 1, 'SYSTEME', 'Motif toujours en mode 1 couleur rouge', ''),
(16, 7,   4, 1, 'SYSTEME', 'rÃ©servÃ©', ''),
(17, 7,   3, 1, 'SYSTEME', 'rÃ©servÃ©', ''),
(18, 7,   0, 1, 'SYSTEME', 'rÃ©servÃ©', ''),
(19, 7,   2, 1, 'SYSTEME', 'rÃ©servÃ©', ''),
(20, 1,   4, 1, 'SYS_AUDIO_START', 'Emission de message Audio.', ''),
(21, 1,   5, 1, 'SYS_AUDIO_END', 'Fin d''emission de message Audio.', ''),
(22, 1,   6, 1, 'SYS_AUDIO_INHIB', 'Inhibition des messages vocaux (hors alerte).', ''),
(23, 3,9999, 1, 'EVENT_NONE_TOR', 'Used for detected Event with no mapping yet.', ''),
(24, 5,9999, 1, 'EVENT_NONE_ANA', 'Used for detected Event with no mapping yet.', ''),
(25, 5, 122, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(26, 5, 121, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(27, 5, 120, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(28, 5, 119, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(29, 5, 118, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(30, 5, 117, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(31, 5, 116, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(32, 5, 115, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(33, 5, 114, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(34, 5, 113, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(35, 5, 112, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(36, 5, 111, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(37, 5, 110, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(38, 5, 109, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(39, 5, 108, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(40, 5, 107, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(41, 5, 106, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(42, 5, 105, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(43, 5, 104, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(44, 5, 103, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(45, 5, 102, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(46, 5, 101, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(47, 5, 100, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
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
(79, 0,  39, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(80, 0,  38, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(81, 0,  37, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(82, 0,  36, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(83, 0,  35, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(84, 0,  34, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(85, 0,  33, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(86, 0,  32, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(87, 0,  31, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(88, 0,  30, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(89, 0,  29, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(90, 0,  28, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(91, 0,  27, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(92, 0,  26, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(93, 0,  25, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(94, 0,  24, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(95, 0,  23, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(96, 0,  22, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(97, 0,  21, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(98, 0,  20, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(99, 0,  19, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(100, 0, 18, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(101, 0, 17, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(102, 0, 16, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(103, 0, 15, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(104, 0, 14, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(105, 0, 13, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(106, 0, 12, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(107, 0, 11, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(108, 0, 10, 1, 'SYS_RESERVED', 'Reserved for internal use', ''),
(109, 0, 09, 1, 'SYS_RESERVED', 'Reserved for internal use', '');

-- --------------------------------------------------------

--
-- Structure de la table `modbus_modules`
--

CREATE TABLE IF NOT EXISTS `modbus_modules` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `instance_id` text NOT NULL,
  `enable` tinyint(1) NOT NULL,
  `ip` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `watchdog` int(11) NOT NULL,
  `bit` int(11) NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `map_E` int(11) NOT NULL,
  `map_EA` int(11) NOT NULL,
  `map_A` int(11) NOT NULL,
  `map_AA` int(11) NOT NULL,  PRIMARY KEY (`id`),
  UNIQUE KEY `ip` (`ip`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `config`
--

CREATE TABLE IF NOT EXISTS `config` (
  `instance_id` varchar(80) COLLATE utf8_unicode_ci NOT NULL,
  `nom_thread` varchar(80) COLLATE utf8_unicode_ci NOT NULL,
  `nom` varchar(80) COLLATE utf8_unicode_ci NOT NULL,
  `valeur` varchar(128) COLLATE utf8_unicode_ci NOT NULL,
  UNIQUE KEY `instance_id` (`instance_id`,`nom_thread`,`nom`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `msgs`
--

CREATE TABLE IF NOT EXISTS `msgs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `num` int(11) NOT NULL DEFAULT '0',
  `dls_id` int(11) NOT NULL DEFAULT '0',
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `libelle_audio` text COLLATE utf8_unicode_ci NOT NULL,
  `libelle_sms` text COLLATE utf8_unicode_ci NOT NULL,
  `type` int(11) NOT NULL DEFAULT '0',
  `audio` tinyint(1) NOT NULL DEFAULT '0',
  `bit_audio` int(11) DEFAULT NULL,
  `enable` tinyint(1) NOT NULL DEFAULT '0',
  `persist` tinyint(1) NOT NULL DEFAULT '0',
  `sms` int(11) NOT NULL DEFAULT '0',
  `time_repeat` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

INSERT INTO `msgs` (`id`, `num`, `dls_id`, `libelle`, `libelle_audio`, `libelle_sms`, `type`, `enable`, `sms` ) VALUES
(1, 0, 1, 'Warning, system is halting', 'Warning, system is halting', 'Warning, system is halting', 1, TRUE, FALSE ),
(2, 1, 1, 'Warning, system is rebooting', 'Warning, system is rebooting', 'Warning, system is rebooting', 1, TRUE, FALSE );

-- --------------------------------------------------------

--
-- Structure de la table `ups`
--

CREATE TABLE IF NOT EXISTS `ups` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `instance_id` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `enable` tinyint(1) NOT NULL,
  `host` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `ups` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `username` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `password` text CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `bit_comm` int(11) NOT NULL,
  `map_EA` int(11) NOT NULL,
  `map_E` int(11) NOT NULL,
  `map_A` int(11) NOT NULL,

  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;


-- --------------------------------------------------------
--
-- Structure de la table ``
--

CREATE TABLE IF NOT EXISTS `rs485` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `instance_id` text NOT NULL,
  `num` int(11) NOT NULL DEFAULT '0',
  `enable` tinyint(1) NOT NULL DEFAULT '0',
  `ea_min` int(11) NOT NULL DEFAULT '-1',
  `ea_max` int(11) NOT NULL DEFAULT '-1',
  `e_min` int(11) NOT NULL DEFAULT '-1',
  `e_max` int(11) NOT NULL DEFAULT '-1',
  `s_min` int(11) NOT NULL DEFAULT '-1',
  `s_max` int(11) NOT NULL DEFAULT '-1',
  `sa_min` int(11) NOT NULL DEFAULT '-1',
  `sa_max` int(11) NOT NULL DEFAULT '-1',
  `bit_comm` int(11) NOT NULL DEFAULT '0',
  `libelle` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `tempo`
--

CREATE TABLE IF NOT EXISTS `mnemos_Tempo` (
  `id_mnemo` int(11) NOT NULL,
  `delai_on` int(11) NOT NULL DEFAULT '0',
  `delai_off` int(11) NOT NULL DEFAULT '0',
  `min_on` int(11) NOT NULL DEFAULT '0',
  `max_on` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id_mnemo`),
  CONSTRAINT `id_mnemo` FOREIGN KEY (`id_mnemo`) REFERENCES `mnemos` (`id`) ON DELETE CASCADE
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

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
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns`
--

CREATE TABLE IF NOT EXISTS `syns` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `groupe` text COLLATE utf8_unicode_ci NOT NULL,
  `page` text COLLATE utf8_unicode_ci NOT NULL,
  `access_groupe` int(11) NOT NULL DEFAULT '0',
  `vignette_activite` int(11) NOT NULL DEFAULT '0',
  `vignette_secu_bien` int(11) NOT NULL DEFAULT '0',
  `vignette_secu_personne` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;
INSERT INTO `syns` (`id`, `libelle`, `groupe`, `page`, `access_groupe` ) VALUES
(1, 'Accueil', 'Defaut Groupe', 'Defaut Page', 0);

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
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_capteurs`
--

CREATE TABLE IF NOT EXISTS `syns_capteurs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `syn_id` int(11) NOT NULL DEFAULT '0',
  `bitctrl` int(11) NOT NULL DEFAULT '0',
  `posx` int(11) NOT NULL DEFAULT '0',
  `posy` int(11) NOT NULL DEFAULT '0',
  `type` int(11) NOT NULL DEFAULT '0',
  `angle` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

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
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_motifs`
--

CREATE TABLE IF NOT EXISTS `syns_motifs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `icone` int(11) NOT NULL DEFAULT '0',
  `syn` int(11) NOT NULL DEFAULT '0',
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `gid` int(11) NOT NULL DEFAULT '0',
  `bitctrl` int(11) NOT NULL DEFAULT '0',
  `bitclic` int(11) NOT NULL DEFAULT '0',
  `bitclic2` int(11) NOT NULL DEFAULT '0',
  `rafraich` int(11) NOT NULL DEFAULT '0',
  `posx` int(11) NOT NULL DEFAULT '0',
  `posy` int(11) NOT NULL DEFAULT '0',
  `larg` float NOT NULL DEFAULT '0',
  `haut` float NOT NULL DEFAULT '0',
  `angle` float NOT NULL DEFAULT '0',
  `dialog` int(11) NOT NULL DEFAULT '0',
  `gestion` int(11) NOT NULL DEFAULT '0',
  `rouge` int(11) NOT NULL DEFAULT '0',
  `vert` int(11) NOT NULL DEFAULT '0',
  `bleu` int(11) NOT NULL DEFAULT '0',
  `layer` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_palettes`
--

CREATE TABLE IF NOT EXISTS `syns_palettes` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `syn_id` int(11) NOT NULL DEFAULT '0',
  `syn_cible_id` int(11) NOT NULL DEFAULT '0',
  `pos` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

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
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;

-- --------------------------------------------------------

--
-- Structure de la table `users`
--

CREATE TABLE IF NOT EXISTS `users` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(96) COLLATE utf8_unicode_ci NOT NULL,
  `mustchangepwd` tinyint(1) NOT NULL DEFAULT '0',
  `cansetpwd` tinyint(1) NOT NULL DEFAULT '0',
  `salt` varchar(130) COLLATE utf8_unicode_ci NOT NULL,
  `hash` varchar(130) COLLATE utf8_unicode_ci NOT NULL,
  `comment` varchar(240) COLLATE utf8_unicode_ci DEFAULT NULL,
  `login_failed` int(11) NOT NULL DEFAULT '0',
  `enable` tinyint(1) NOT NULL DEFAULT '0',
  `date_create` int(11) NOT NULL DEFAULT '0',
  `enable_expire` tinyint(1) NOT NULL DEFAULT '0',
  `date_expire` int(11) DEFAULT NULL,
  `date_modif` int(11) DEFAULT NULL,
  `sms_enable` tinyint(1) NOT NULL DEFAULT '0',
  `sms_phone` varchar(80) COLLATE utf8_unicode_ci NOT NULL,
  `sms_allow_cde` tinyint(1) NOT NULL DEFAULT '0',
  `imsg_enable` tinyint(1) NOT NULL DEFAULT '0',
  `imsg_jabberid` varchar(80) COLLATE utf8_unicode_ci NOT NULL,
  `imsg_allow_cde` tinyint(1) NOT NULL DEFAULT '0',
  `imsg_available` tinyint(1) NOT NULL DEFAULT '0',
  `ssrv_bit_presence` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;

INSERT INTO `users` (`id`, `name`, `mustchangepwd`, `cansetpwd`, `salt`, `hash`, `comment`, `login_failed`, `enable`, `date_create`, `enable_expire`, `date_expire`, `date_modif`) VALUES
(0, 'root', 1, 1, '9311D076CDB709623503B3D3461EA8E9DFE842076C8A6B348AA78215BF7B7B797ABBE33F29CDF86B88F1B2D6071D4916ACAD1C997B832AE774D3AB4186077386', '529612B992460427C7C6FF21F5AC6965C36A735B8AB813FC3FF083AA3D2D19190AB1A700BEE2ADFA9D797F301C2E3D491D12AA04C69C7652CE875721E1E6F1B4', 'Utilisateur Root', 0, 1, 0, 0, 0, 0),
(1, 'guest', 0, 0, '0FE3B94BCC1E52AC4BEE0DE31D6306890854EAFC77F855FBD9D17BB0D7256A5E23ED8D58FA85E345FE71D046211745B6B50382CD939DC7FDAA2FBE6B7D586069', '6E14D7124DF5FC4C018D845F351553F751265C37834455B96EE3014BCA7CFE53B87CAD8FFA739B39C4A5BCD61E267560EAA7F2AEFFAB3C457B1E0F6BE5BCF8C4', 'Utilisateur Guest', 0, 1, 0, 0, 0, 0);


