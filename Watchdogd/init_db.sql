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

DROP TABLE IF EXISTS `cameras`;
CREATE TABLE IF NOT EXISTS `cameras` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `location` varchar(600) NOT NULL,
  `type` int(11) NOT NULL,
  `bit` int(11) NOT NULL,
  `objet` text NOT NULL,
  `libelle` text NOT NULL,
  `num` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=894 ;

-- --------------------------------------------------------

--
-- Structure de la table `cameras_motion`
--

DROP TABLE IF EXISTS `cameras_motion`;
CREATE TABLE IF NOT EXISTS `cameras_motion` (
  `id` int(11) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `class`
--

DROP TABLE IF EXISTS `class`;
CREATE TABLE IF NOT EXISTS `class` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `libelle` varchar(241) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=28 ;

-- --------------------------------------------------------

--
-- Structure de la table `dls`
--

DROP TABLE IF EXISTS `dls`;
CREATE TABLE IF NOT EXISTS `dls` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` int(11) NOT NULL,
  `groupe` text COLLATE utf8_unicode_ci NOT NULL,
  `ssgroupe` text COLLATE utf8_unicode_ci NOT NULL,
  `name` text COLLATE utf8_unicode_ci NOT NULL,
  `actif` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=78 ;

-- --------------------------------------------------------

--
-- Structure de la table `dls_cpth`
--

DROP TABLE IF EXISTS `dls_cpth`;
CREATE TABLE IF NOT EXISTS `dls_cpth` (
  `id` int(11) NOT NULL DEFAULT '0',
  `val` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `dls_cpt_imp`
--

DROP TABLE IF EXISTS `dls_cpt_imp`;
CREATE TABLE IF NOT EXISTS `dls_cpt_imp` (
  `id_mnemo` int(11) NOT NULL,
  `val` float NOT NULL,
  `unite` int(11) NOT NULL,
  `type_ci` int(11) NOT NULL,
  PRIMARY KEY (`id_mnemo`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `dls_scenario`
--

DROP TABLE IF EXISTS `dls_scenario`;
CREATE TABLE IF NOT EXISTS `dls_scenario` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `bitm` int(11) NOT NULL,
  `heure` int(11) NOT NULL,
  `minute` int(11) NOT NULL,
  `libelle` varchar(200) COLLATE utf8_unicode_ci NOT NULL,
  `lundi` tinyint(1) NOT NULL,
  `mardi` tinyint(1) NOT NULL,
  `mercredi` tinyint(1) NOT NULL,
  `jeudi` tinyint(1) NOT NULL,
  `vendredi` tinyint(1) NOT NULL,
  `samedi` tinyint(1) NOT NULL,
  `dimanche` tinyint(1) NOT NULL,
  `janvier` tinyint(1) NOT NULL,
  `fevrier` tinyint(1) NOT NULL,
  `mars` tinyint(1) NOT NULL,
  `avril` tinyint(1) NOT NULL,
  `mai` tinyint(1) NOT NULL,
  `juin` tinyint(1) NOT NULL,
  `juillet` tinyint(1) NOT NULL,
  `aout` tinyint(1) NOT NULL,
  `septembre` tinyint(1) NOT NULL,
  `octobre` tinyint(1) NOT NULL,
  `novembre` tinyint(1) NOT NULL,
  `decembre` tinyint(1) NOT NULL,
  `ts_jour` tinyint(1) NOT NULL,
  `ts_mois` tinyint(1) NOT NULL,
  `actif` tinyint(1) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=8 ;

-- --------------------------------------------------------

--
-- Structure de la table `eana`
--

DROP TABLE IF EXISTS `eana`;
CREATE TABLE IF NOT EXISTS `eana` (
  `id_mnemo` int(11) NOT NULL,
  `type` int(11) NOT NULL,
  `min` float NOT NULL DEFAULT '0',
  `max` float NOT NULL DEFAULT '0',
  `unite` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id_mnemo`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `gids`
--

DROP TABLE IF EXISTS `gids`;
CREATE TABLE IF NOT EXISTS `gids` (
  `id_util` int(11) NOT NULL DEFAULT '0',
  `gids` int(11) NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `groups`
--

DROP TABLE IF EXISTS `groups`;
CREATE TABLE IF NOT EXISTS `groups` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(97) COLLATE utf8_unicode_ci NOT NULL,
  `comment` varchar(241) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=12 ;

-- --------------------------------------------------------

--
-- Structure de la table `histo`
--

DROP TABLE IF EXISTS `histo`;
CREATE TABLE IF NOT EXISTS `histo` (
  `id` int(11) NOT NULL DEFAULT '0',
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `objet` text COLLATE utf8_unicode_ci NOT NULL,
  `type` int(11) NOT NULL DEFAULT '0',
  `num_syn` int(11) NOT NULL DEFAULT '0',
  `nom_ack` text COLLATE utf8_unicode_ci,
  `date_create_sec` int(11) NOT NULL DEFAULT '0',
  `date_create_usec` int(11) NOT NULL DEFAULT '0',
  `date_fixe` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `histo_bit`
--

DROP TABLE IF EXISTS `histo_bit`;
CREATE TABLE IF NOT EXISTS `histo_bit` (
  `type` int(11) NOT NULL DEFAULT '0',
  `num` int(11) NOT NULL DEFAULT '0',
  `date_sec` int(11) NOT NULL DEFAULT '0',
  `date_usec` int(11) NOT NULL DEFAULT '0',
  `valeur` int(11) NOT NULL DEFAULT '0',
  KEY `test` (`type`,`num`,`date_sec`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `histo_hard`
--

DROP TABLE IF EXISTS `histo_hard`;
CREATE TABLE IF NOT EXISTS `histo_hard` (
  `num` int(11) NOT NULL DEFAULT '0',
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `type` int(11) NOT NULL DEFAULT '0',
  `num_syn` int(11) NOT NULL DEFAULT '0',
  `nom_ack` varchar(97) COLLATE utf8_unicode_ci DEFAULT NULL,
  `objet` text COLLATE utf8_unicode_ci,
  `date_create_sec` int(11) NOT NULL DEFAULT '0',
  `date_create_usec` int(11) DEFAULT '0',
  `date_fixe` int(11) NOT NULL DEFAULT '0',
  `date_fin` int(11) NOT NULL DEFAULT '0',
  KEY `date_create_sec` (`date_create_sec`),
  KEY `type` (`type`),
  KEY `num` (`num`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- --------------------------------------------------------

--
-- Structure de la table `icons`
--

DROP TABLE IF EXISTS `icons`;
CREATE TABLE IF NOT EXISTS `icons` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `id_classe` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=454 ;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos`
--

DROP TABLE IF EXISTS `mnemos`;
CREATE TABLE IF NOT EXISTS `mnemos` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` int(11) NOT NULL DEFAULT '0',
  `num` int(11) NOT NULL DEFAULT '0',
  `objet` text COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` text COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1012 ;

-- --------------------------------------------------------

--
-- Structure de la table `modbus_bornes`
--

DROP TABLE IF EXISTS `modbus_bornes`;
CREATE TABLE IF NOT EXISTS `modbus_bornes` (
  `type` int(11) NOT NULL,
  `adresse` int(11) NOT NULL,
  `min` int(11) NOT NULL,
  `nbr` int(11) NOT NULL,
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `module` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=4 ;

-- --------------------------------------------------------

--
-- Structure de la table `modbus_modules`
--

DROP TABLE IF EXISTS `modbus_modules`;
CREATE TABLE IF NOT EXISTS `modbus_modules` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `actif` tinyint(1) NOT NULL,
  `ip` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `watchdog` int(11) NOT NULL,
  `bit` int(11) NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `ip` (`ip`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=4 ;

-- --------------------------------------------------------

--
-- Structure de la table `msgs`
--

DROP TABLE IF EXISTS `msgs`;
CREATE TABLE IF NOT EXISTS `msgs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `num` int(11) NOT NULL DEFAULT '0',
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `libelle_audio` text COLLATE utf8_unicode_ci NOT NULL,
  `libelle_sms` text COLLATE utf8_unicode_ci NOT NULL,
  `type` int(11) NOT NULL DEFAULT '0',
  `num_syn` int(11) NOT NULL DEFAULT '0',
  `bit_voc` int(11) DEFAULT NULL,
  `enable` tinyint(1) NOT NULL DEFAULT '0',
  `objet` text COLLATE utf8_unicode_ci NOT NULL,
  `sms` int(11) NOT NULL DEFAULT '0',
  `type_voc` int(11) NOT NULL DEFAULT '4',
  `vitesse_voc` int(11) NOT NULL DEFAULT '150',
  `time_repeat` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=487 ;

-- --------------------------------------------------------

--
-- Structure de la table `onduleurs`
--

DROP TABLE IF EXISTS `onduleurs`;
CREATE TABLE IF NOT EXISTS `onduleurs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `actif` tinyint(1) NOT NULL,
  `host` varchar(32) NOT NULL,
  `ups` varchar(32) NOT NULL,
  `bit_comm` int(11) NOT NULL,
  `ea_ups_load` int(11) NOT NULL,
  `ea_ups_real_power` int(11) NOT NULL,
  `ea_battery_charge` int(11) NOT NULL,
  `ea_input_voltage` int(11) NOT NULL,
  `libelle` text NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=6 ;

-- --------------------------------------------------------

--
-- Structure de la table `rs485`
--

DROP TABLE IF EXISTS `rs485`;
CREATE TABLE IF NOT EXISTS `rs485` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `num` int(11) NOT NULL DEFAULT '0',
  `actif` tinyint(1) NOT NULL DEFAULT '0',
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
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=8 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns`
--

DROP TABLE IF EXISTS `syns`;
CREATE TABLE IF NOT EXISTS `syns` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `groupe` text COLLATE utf8_unicode_ci NOT NULL,
  `ssgroupe` text COLLATE utf8_unicode_ci NOT NULL,
  `titre` text COLLATE utf8_unicode_ci NOT NULL,
  `access_groupe` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=30 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_camerasup`
--

DROP TABLE IF EXISTS `syns_camerasup`;
CREATE TABLE IF NOT EXISTS `syns_camerasup` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `syn_id` int(11) NOT NULL,
  `camera_src_id` int(11) NOT NULL,
  `posx` int(11) NOT NULL,
  `posy` int(11) NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=31 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_capteurs`
--

DROP TABLE IF EXISTS `syns_capteurs`;
CREATE TABLE IF NOT EXISTS `syns_capteurs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `syn_id` int(11) NOT NULL DEFAULT '0',
  `bitctrl` int(11) NOT NULL DEFAULT '0',
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `posx` int(11) NOT NULL DEFAULT '0',
  `posy` int(11) NOT NULL DEFAULT '0',
  `type` int(11) NOT NULL DEFAULT '0',
  `angle` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=24 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_comments`
--

DROP TABLE IF EXISTS `syns_comments`;
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
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=148 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_motifs`
--

DROP TABLE IF EXISTS `syns_motifs`;
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
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=912 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_palettes`
--

DROP TABLE IF EXISTS `syns_palettes`;
CREATE TABLE IF NOT EXISTS `syns_palettes` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `syn_id` int(11) NOT NULL DEFAULT '0',
  `syn_cible_id` int(11) NOT NULL DEFAULT '0',
  `pos` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=15 ;

-- --------------------------------------------------------

--
-- Structure de la table `syns_pass`
--

DROP TABLE IF EXISTS `syns_pass`;
CREATE TABLE IF NOT EXISTS `syns_pass` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `syn_id` int(11) NOT NULL DEFAULT '0',
  `syn_cible_id` int(11) NOT NULL DEFAULT '0',
  `bitctrl` int(11) NOT NULL DEFAULT '0',
  `posx` int(11) NOT NULL DEFAULT '0',
  `posy` int(11) NOT NULL DEFAULT '0',
  `bitctrl1` int(11) NOT NULL DEFAULT '0',
  `bitctrl2` int(11) NOT NULL DEFAULT '0',
  `angle` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=58 ;

-- --------------------------------------------------------

--
-- Structure de la table `users`
--

DROP TABLE IF EXISTS `users`;
CREATE TABLE IF NOT EXISTS `users` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `name` varchar(96) COLLATE utf8_unicode_ci NOT NULL,
  `changepass` tinyint(1) NOT NULL DEFAULT '0',
  `cansetpass` tinyint(1) NOT NULL DEFAULT '0',
  `crypt` varchar(24) COLLATE utf8_unicode_ci NOT NULL,
  `comment` varchar(240) COLLATE utf8_unicode_ci DEFAULT NULL,
  `login_failed` int(11) NOT NULL DEFAULT '0',
  `enable` tinyint(1) NOT NULL DEFAULT '0',
  `date_create` int(11) NOT NULL DEFAULT '0',
  `enable_expire` tinyint(1) NOT NULL DEFAULT '0',
  `date_expire` int(11) DEFAULT NULL,
  `date_modif` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=5 ;

-- --------------------------------------------------------

--
-- Doublure de structure pour la vue `utilisation_histo_bit`
--
DROP VIEW IF EXISTS `utilisation_histo_bit`;
CREATE TABLE IF NOT EXISTS `utilisation_histo_bit` (
`cpt` bigint(21)
,`type` int(11)
,`num` int(11)
);
-- --------------------------------------------------------

--
-- Structure de la vue `utilisation_histo_bit`
--
DROP TABLE IF EXISTS `utilisation_histo_bit`;

CREATE ALGORITHM=UNDEFINED DEFINER=`root`@`localhost` SQL SECURITY DEFINER VIEW `utilisation_histo_bit` AS select count(0) AS `cpt`,`histo_bit`.`type` AS `type`,`histo_bit`.`num` AS `num` from `histo_bit` group by `histo_bit`.`type`,`histo_bit`.`num` order by count(0) desc;
