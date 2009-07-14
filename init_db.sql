-- phpMyAdmin SQL Dump
-- version 3.1.3.2
-- http://www.phpmyadmin.net
--
-- Serveur: localhost
-- Généré le : Lun 13 Juillet 2009 à 16:51
-- Version du serveur: 5.1.34
-- Version de PHP: 5.2.9

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";

--
-- Base de données: `WatchdogDB`
--
DROP DATABASE `WatchdogDB`;
CREATE DATABASE `WatchdogDB` DEFAULT CHARACTER SET utf8 COLLATE utf8_unicode_ci;
USE `WatchdogDB`;

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
  `name` varchar(240) COLLATE utf8_unicode_ci NOT NULL,
  `actif` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=70 ;

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
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `num` int(11) NOT NULL DEFAULT '0',
  `min` float NOT NULL DEFAULT '0',
  `max` float NOT NULL DEFAULT '0',
  `unite` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10 ;

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
  `objet` varchar(181) COLLATE utf8_unicode_ci NOT NULL,
  `type` int(11) NOT NULL DEFAULT '0',
  `num_syn` int(11) NOT NULL DEFAULT '0',
  `nom_ack` varchar(97) COLLATE utf8_unicode_ci DEFAULT NULL,
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
  KEY `test` (`type`,`num`)
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
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=414 ;

-- --------------------------------------------------------

--
-- Structure de la table `mnemos`
--

DROP TABLE IF EXISTS `mnemos`;
CREATE TABLE IF NOT EXISTS `mnemos` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `type` int(11) NOT NULL DEFAULT '0',
  `num` int(11) NOT NULL DEFAULT '0',
  `objet` varchar(181) COLLATE utf8_unicode_ci NOT NULL,
  `acronyme` varchar(73) COLLATE utf8_unicode_ci NOT NULL,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=753 ;

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
  PRIMARY KEY (`id`),
  UNIQUE KEY `ip` (`ip`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=2 ;

-- --------------------------------------------------------

--
-- Structure de la table `msgs`
--

DROP TABLE IF EXISTS `msgs`;
CREATE TABLE IF NOT EXISTS `msgs` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `num` int(11) NOT NULL DEFAULT '0',
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `type` int(11) NOT NULL DEFAULT '0',
  `num_syn` int(11) NOT NULL DEFAULT '0',
  `num_voc` int(11) DEFAULT NULL,
  `not_inhibe` tinyint(1) NOT NULL DEFAULT '0',
  `objet` varchar(181) COLLATE utf8_unicode_ci NOT NULL,
  `sms` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=448 ;

-- --------------------------------------------------------

--
-- Structure de la table `rs485`
--

DROP TABLE IF EXISTS `rs485`;
CREATE TABLE IF NOT EXISTS `rs485` (
  `id` int(11) NOT NULL,
  `actif` tinyint(1) NOT NULL,
  `ea_min` int(11) NOT NULL DEFAULT '-1',
  `ea_max` int(11) NOT NULL DEFAULT '-1',
  `e_min` int(11) NOT NULL DEFAULT '-1',
  `e_max` int(11) NOT NULL DEFAULT '-1',
  `ec_min` int(11) NOT NULL DEFAULT '-1',
  `ec_max` int(11) NOT NULL DEFAULT '-1',
  `s_min` int(11) NOT NULL DEFAULT '-1',
  `s_max` int(11) NOT NULL DEFAULT '-1',
  `sa_min` int(11) NOT NULL DEFAULT '-1',
  `sa_max` int(11) NOT NULL DEFAULT '-1',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1;

-- --------------------------------------------------------

--
-- Structure de la table `syns`
--

DROP TABLE IF EXISTS `syns`;
CREATE TABLE IF NOT EXISTS `syns` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `libelle` text COLLATE utf8_unicode_ci NOT NULL,
  `mnemo` varchar(73) COLLATE utf8_unicode_ci NOT NULL,
  `groupe` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `mnemo` (`mnemo`)
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=17 ;

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
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=21 ;

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
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=113 ;

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
  `bitclic2` int(11) NOT NULL,
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
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=816 ;

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
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=19 ;

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
) ENGINE=MyISAM  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=4 ;

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

