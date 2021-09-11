# Bienvenue sur Watchdog !

Vous trouverez sur ce site l'ensemble de la documentation technique permettant de prendre en main ce système de gestion d'habitat.
Cette documentation s'adresse aux personnes ayant la responsabilité de l'installation et du maintien des systèmes et sous-systèmes composant un domaine complet.

Elle présente les guides d'installation, l'architecture, les concepts, les modes d'emploi, ainsi les bonnes pratiques de mise en oeuvre.


##Pré-requis

Les socles minimums sur lesquels le système a été testé puis validé:

* Fedora 34 ou supérieure
* Debian Bullseye ou supérieure
* RaspiOS (basée sur Bullseye)

Vous aurez également besoin des droits d'administration, via *sudo* par exemple.

---
##Installation

La procédure d'installation recommandée est celle en ligne de commande.

###Installation en ligne de commande

Depuis un terminal, lancez la commande suivante:

     sudo bash -c "$(wget https://svn.abls-habitat.fr/repo/Watchdog/prod/INSTALL.sh -q -O -)"

Si vous souhaitez installer en mode **User**, précisez-le lorsque cela vous sera demandé.

* Le mode **System** est préconisé pour des machines ne s'éteignant jamais, en général headless. Il est adapté aux instances *Master* ou *Slave*
* Le mode **User** est adapté aux machines allumées de manière intermittente, comme les PC utilisés comme media center par exemple. Il est plutot utilisé avec des instances "Slave"

N'oubliez pas de noter le mot de passe associé à la base de données.
Dans le cadre d'une instance **User**, entrez ensuite dans un navigateur l'URL suivante.

     https://localhost:5560/install

Pour une instance **Système**, remplacez *localhost* par le nom de la machine qui héberge cette instance.

Puis laissez-vous guider !

###Installation depuis le repository SVN

Pour suivre les mises à jour automatiques de la branche de production, importez repository **prod**.
Pour cela, tapez les commandes suivantes dans un terminal:

     svn co https://svn.abls-habitat.fr/repo/Watchdog/prod Watchdog
     cd Watchdog
     ./autogen.sh
     sudo make install
     sudo systemctl start Watchdogd

Ensuite, laissez-vous guider depuis https://localhost:5560/install

---
## Arret/Relance et suivi de l'instance
###Commandes de lancement et d'arret

Les commandes suivantes permettent alors de demarrer, stopper, restarter l'instance:

     sudo systemctl start Watchdogd.service
     sudo systemctl stop Watchdogd.service
     sudo systemctl restart Watchdogd.service

### Commandes d'affichage des logs

Les commandes suivantes permettent d'afficher les logs de l'instance:

[watchdog@Server ~]$ sudo journalctl -f -u Watchdogd.service

---
## Utilisateurs par défaut

A l'installation, deux comptes sont pre-enregistrés: les comptes *root* et *guest*

* Le compte *root* est un compte administrateur (privilège maximum : Level 9). Son mot de passe par défaut est **password**
* Le compte *guest* est un compte utilisateur avec des privilèges minimaux (Level 1). Son mot de passe par défaut est **guest**
