# <img src="/img/abls.svg" width=100> Bienvenue sur Watchdog !

Vous trouverez sur ce site l'ensemble de la documentation technique permettant de prendre en main ce système de gestion d'habitat.
Cette documentation s'adresse aux personnes ayant la responsabilité de l'installation et du maintien des systèmes et sous-systèmes composant un domaine complet.

Elle présente les guides d'installation, l'architecture, les concepts, les modes d'emploi, ainsi les bonnes pratiques de mise en oeuvre.

---
## Pré-requis

Les socles minimums sur lesquels le système a été testé puis validé:

* Fedora 34 ou supérieure
* Debian Bullseye ou supérieure
* RaspiOS (basée sur Bullseye)

Vous aurez également besoin des droits d'administration, via *sudo* par exemple.

---
##Installation

1. Pour une primo-installation, la procédure d'installation recommandée est celle en [ligne de commande](#installation-en-ligne-de-commande)
1. Puis complétez par [l'installation web](#installation-web).

Vous pouvez également, dans le cadre d'upgrade par exemple, utiliser la méthode de mise à jour basée sur [le repository SVN](#upgrader-une-instance-existante).

###Installation en ligne de commande

Depuis un terminal, lancez la commande suivante et répondez ensuite aux questions posées:

     sudo bash -c "$(wget https://svn.abls-habitat.fr/repo/Watchdog/prod/INSTALL.sh -q -O -)"

Si vous souhaitez installer en mode **User**, précisez-le lorsque cela vous sera demandé.

* Le mode **System** est préconisé pour des machines ne s'éteignant jamais, en général headless. Il est adapté aux instances *Master* ou *Slave*
* Le mode **User** est adapté aux machines allumées de manière intermittente, comme les PC utilisés comme media center par exemple.
Il est plutot utilisé avec des instances *Slave*

N'oubliez pas de noter le mot de passe associé à la base de données.

Laissez-vous ensuite [guider](#installation-web) par l'installation graphique.

### Installation web

Dans le cadre d'une instance **User**, ouvrez dans un navigateur l'URL suivante:

     https://localhost:5560/install

Pour une instance **System**, remplacez *localhost* par le nom de la machine qui héberge cette instance.

Vous obtiendrez la page d'installation suivante après avoir accepté l'exception de sécurité liée au certificat SSL auto-signé:

![install](/img/ihm_install.png)

Entrez alors les paramètres de votre nouvelle instance.
Pour une instance **Master** en mode **System**, la configuration par défaut est adaptée.

1. Le nom de votre habitat (exemple: *Ma maison*, *Mon appart*)
1. Le nom du serveur de base de données associé. Pour une instance *Master*, *localhost* est le choix par défaut.
Pour une instance **Slave** le serveur de base de données est en général celui portant l'instance **Master**
1. Le port présentant le service de base de données, par défaut *3306*
1. Le nom de la base de données, par défaut *WatchdogDB*
1. Le nom de l'utilisateur de la base de données, par défaut *watchdog*
1. Le mot de passe associé a l'utilisateur, noté lors de la première phase d'installation en ligne de commande
1. L'utilisateur système qui fera tourner le service. Par défaut *watchdog*. Porte uniquement pour les instances en mode **System**
1. Précisez si le *working directory* est le home (SubDirectory=*No*), ou home/.watchdog (SubDirectory=*Yes*)
1. Précisez si l'instance est **Master** ou **Slave**
1. Enfin, dans le cadre d'instance **Slave**, précisez le hostname de l'instance **Master**

---
## Arrêt/Relance et suivi de l'instance **Système**
###Commandes de lancement et d'arret

Les commandes suivantes permettent alors de demarrer, stopper, restarter l'instance Système:

    [watchdog@Server ~]$ sudo systemctl start Watchdogd.service
    [watchdog@Server ~]$ sudo systemctl stop Watchdogd.service
    [watchdog@Server ~]$ sudo systemctl restart Watchdogd.service

### Commandes d'affichage des logs

Les commandes suivantes permettent d'afficher les logs de l'instance:

    [watchdog@Server ~]$ sudo journalctl -f -u Watchdogd.service

---
## Arrêt/Relance et suivi de l'instance **User**

###Commandes de lancement et d'arret

Les commandes suivantes permettent alors de demarrer, stopper, restarter l'instance en UserMode:

    [moi@Server ~]$ systemctl --user start Watchdogd-user.service
    [moi@Server ~]$ systemctl --user stop Watchdogd-user.service
    [moi@Server ~]$ systemctl --user restart Watchdogd-user.service

### Commandes d'affichage des logs

Les commandes suivantes permettent d'afficher les logs de l'instance:

    [moi@Server ~]$ journalctl --user -f -u Watchdogd-user.service

---
##Upgrader une instance existante

Pour mettre à niveau votre instance, vous pouvez suivre les mises à jour automatiques de la branche de production
du repository **[prod](https://svn.abls-habitat.fr/websvn/browse/Watchdog/prod/)**.

Pour cela, tapez les commandes suivantes dans un terminal:

    svn co https://svn.abls-habitat.fr/repo/Watchdog/prod Watchdog
    cd Watchdog
    ./autogen.sh
    sudo make install
    sudo systemctl restart Watchdogd


---
## Utilisateurs par défaut

A l'installation, deux comptes sont pre-enregistrés: les comptes **root** et **guest**

* Le compte **root** est un compte administrateur (privilège maximum : Level 9). Son mot de passe par défaut est **password**
* Le compte **guest** est un compte utilisateur avec des privilèges minimaux (Level 1). Son mot de passe par défaut est **guest**
