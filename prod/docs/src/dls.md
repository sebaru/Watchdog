# Présentation de la programmation D.L.S

Les modules d'intelligence sont nommés Module D.L.S. Ils permettent, via un langage de programmation issu des automates, de définir quel doit etre l'état de sortie d'un objet en fonction d'un ensemble d'états d'entrées.

Dans l'interface web, vous pouvez lister et éditer ces modules D.L.S en utilisant le menu **Configuration->Modules D.L.S**.

---
## Mémoire d'informations

L'ensemble des différents modules D.L.S partagent la même mémoire d'informations. Toutes les informations sont accessibles en lecture et écriture par tous les plugins D.L.S.

Chaque élément est caractérisé

* par sa **classe** : une entrée, une sortie, un message, un compteur, ...
* par son **tech_id**, représentant le module D.L.S auquel il est rattaché
* par son **acronyme**, représentant son nom dans la mémoire d'information

Par exemple: le bit **GARAGE:OUVERTURE** peut représenter une entrée correspondant à l'état d'OUVERTURE ou de fermeture de l'ouvrant GARAGE.

Ou encore, le bit **PORTAIL:NBR_OUVERTURE** peut représenter un compteur associé au nombre d'ouverture du PORTAIL.

Deux bits de deux modules différents peuvent porter le même acronyme, dans la mesure ou leurs tech_id seront différents. Exemple : **JARDIN:TEMPERATURE** et **CUISINE:TEMPERATURE**

Par construction, dans un code source D.L.S, si un bit interne ne spécifie pas son tech_id, celui du module D.L.S d'appartenance est utilisé.

Les différentes classes utilisées sont les suivantes:

| Classe    | Représentation 	|	Exemple |	Description |
|:---------:|:----------------|:--------|:----------- |
| [_E](dls_entre_tor.md)       | Entrée TOR        |	PORTE:OUVERTE             |	Une entrée TOR peut avoir 2 valeurs : 0 ou 1 et représente l'état physique d'un capteur
| [_EA](dls_entre_ana.md) 	    | Entrée Analogique | TEMP:JARDIN 	             | Une entrée Analogique représente une valeur d'un capteur analogique.<br> Celle-ci dispose d'options permettant au systeme de savoir comment interpréter<br>les informations fournies par les capteurs (4/20mA, 0-10V, ...)
| [_A](dls_sortie_tor.md) 	    | Sortie TOR 	      | VANNE:OUVRIR 	            | Une sortie TOR peut avoir 2 valeurs : 0 ou 1, et représente l'état souhaité d'un actionneur
| [_AA](dls_sortie_ana.md) 	   | Sortie Analogique |	CHAUDIERE:CONSIGNE 	      | Une sortie Analogique represente la valeur souhaitée d'un actionneur analogique. <br> Elle dispose d'options permettant au systeme de traduire une valeur reelle en valeur<br>compréhensible par les actionneurs.
| [_B](dls_bistables.md) 	     | Bistable          |	SYS:FLIPFLOP_1SEC 	       | Bit dont la valeur est 0 ou 1, maintenu dans le temps. <br>Il faut explicitement coder la mise à zero du bit pour que celui-ci soit effectivement remis à 0.
| [_M](dls_monostables.md) 	   | Monostable 	      |SYS:TOP_1SEC 	             | Les monostables sont des bits furtifs, non maintenus dans le temps.<br>Si la condition initiale qui imposait le maintien du bit n'est plus vraie,<br>ce bit va alors tomber de lui-meme à 0.
| [_CI](dls_cpti.md) 	         | Compteur d'impulsions | 	PUIT:LITRE 	         | Incrémenté à chaque front montant de sa condition de pilotage
| [_CH](dls_cpth.md) 	         | Compteur Horaire 	| POMPE:DUREE_VIE 	         | Temps seconde représentant la durée effective de maintien de sa condition de pilotage
| [_T](dls_tempo.md) 	         | Temporisation 	   | GARAGE:TEMPO_OUV_PORTE 	  | Les temporisations permettent de décaler, maintenir ou limiter dans le temps<br> un evenement particulier
| [_R](dls_registres.md) 	     | Registre 	        | EDF:EQUIV_KWH 	           | Les registres permettent de manipuler des points de consignes, de seuil,<br>et permettent de réaliser des calculs
| [_HORLOGE](dls_horloge.md) 	 | Horloge 	         | SALON:DMD_FERMETURE_VOLET |	Les horloges sont des valeurs binaires positionnées a une heure bien précise de la journée
| [_WATCHDOG](dls_watchdog.md)	| Watchdog 	        | PRESENCE:CPT_A_REBOURG 	  | Les comptes a rebourg permettent de decompter le temps à partir d'un evenement<br>et de réagir si cet evenement n'est pas revenu au bout d'une consigne précise.
| [_MSG](dls_messages.md)     	| Watchdog 	        | PRESENCE:CPT_A_REBOURG 	  | Les comptes a rebourg permettent de decompter le temps à partir d'un evenement<br>et de réagir si cet evenement n'est pas revenu au bout d'une consigne précise.
| [_I](dls_visuels.md)        	| Watchdog 	        | PRESENCE:CPT_A_REBOURG 	  | Les comptes a rebourg permettent de decompter le temps à partir d'un evenement<br>et de réagir si cet evenement n'est pas revenu au bout d'une consigne précise.

---
## Définition d'un module D.L.S

Un module D.L.S présentera deux zones distinctes :

* la zone de description des [**ALIAS**](dls.md#zone-de-declaration-des-alias)
* la zone de description du fonctionnement [**logique et de calcul**](dls.md#zone-de-logique-et-de-calcul)

### Zone de déclaration des ALIAS

Cette zone permet d'associer un nom (ou IDentificateur) aux bits internes. Cet IDentificateur pourra être utilisé dans le code de fonctionnement pour faire référence au bit interne auquel il est rattaché.

Il s'agit d'une zone donc la syntaxe est la suivante:

     #define IDentificateur <-> _CLASSE;

Elle commence directement par une chaine de caractères représentant l'IDentificateur, puis une double flèche (un inférieur « < », un tiret « - », un supérieur « > »), une Classe, et enfin un point virgule terminal.

Des options peuvent être affectées à cet alias, en utilisant la syntaxe suivante :

     #define IDentificateur <-> _CLASSE (OPTIONS); /* OPTIONS étant une liste de champ=valeur, séparée par des virgules ',' */

Exemple de syntaxe complète :

     #define VOLET_OUVERT <-> _E; /* VOLET_OUVERT devra être mappé à une Entrée Physique (par exemple, a un module MODBUS, ou à la reception d'un SMS) */
     #define CDE_VOLET <-> _M;    /* CDE_VOLET sera utilisé en tant que monostable dans la suite du code D.L.S */

### Zone de logique et de calcul

C'est dans cette zone où toute l'intelligence du plugin va résider. C'est elle qui définie l'ensemble des comportements attendu en fonction de l'environnement du module.
Zone de logique

Cette zone est composée d'une suite de ligne (appelée dans la suite « ligne D.L.S ») dont le contenu d'une ligne suit la syntaxe suivante :

     - EXPRESSION -> LISTE_ACTIONS;

Elle commence par un tiret « - », suivi d'une « EXPRESSION », suivi par une flèche (tiret « - » puis supérieur « > »), une liste d'actions séparées par des virgules et enfin, un point virgule terminal.
Zone de calcul

Le calcul D.L.S permet, sous une condition, de calculer la valeur d'un registre en fonction d'expression booléennes ou arithmétiques. Tout comme la ligne logique, celle-ci commence par un tiret « - », suivi d'une « EXPRESSION » qui représente la condition de calcul, suivi d'un tiret, puis, entre parenthèse, d'un calcul arithmétique, suivi pour finir d'une flèche (tiret « - » puis supérieur « > »), et un registre de destination. Le point virgule terminal reste obligatoire.

Si la condition est vraie, alors le Registre **RESULTAT** sera mis à jour avec la valeur du calcul entre parenthèses.

     - EXPRESSION - ( CALCUL ) -> RESULTAT;

Exemple:

     - SYS:TOP_1SEC - ( VITESSE + 1.0 ) -> VITESSE;

Toutes les secondes, le registre **VITESSE** est augmenté de 1.

---
## Bits locaux à chaque module

| Nom du bit 	| Classe  | 	Positionné par |	Défaut 	| Description
|:------------|:--------|:----------------|:-------:|:-----------
| _MEMSA_COMM | 	Activité 	| Module | 	TRUE | TRUE si la communication est OK, sinon FALSE.
| _MEMSA_DEFAUT|  	Activité | 	Module | 	FALSE | TRUE si le module est en défaut
| _MEMSA_DEFAUT_FIXE|  	Activité | 	Module | 	FALSE | TRUE si le module est en défaut fixe
| _MEMSA_ALARME|  	Activité | 	Module | 	FALSE| TRUE si le module est en alarme
| _MEMSA_ALARME_FIXE | 	Activité | 	Module | 	FALSE| TRUE si le module est en alarme fixe
| _MEMSA_OK | 	Activité 	| Système 	| TRUE | Bit de synthèse de l'activité. Calculé par rapport aux 5 bits précédents
| _MEMSSB_VEILLE | 	Sécurité des Biens | 	Module|  	FALSE | TRUE si le module est en veille
| _MEMSSB_ALERTE | 	Sécurité des Biens | 	Module | 	FALSE | TRUE si le module est en alerte
| _MEMSSB_ALERTE_FUGITIVE | 	Sécurité des Biens | 	Module | 	FALSE | TRUE si le module est en alerte fugitive
| _MEMSSB_ALERTE_FIXE | 	Sécurité des Biens | 	Module | 	FALSE | 	TRUE si le module est en alerte fixe
| _MEMSSP_DERANGEMENT | 	Sécurité des Personnes | 	Module | 	FALSE | TRUE si le module est en dérangement
| _MEMSSP_DERANGEMENT_FIXE | 	Sécurité des Personnes | 	Module|  	FALSE | TRUE si le module est en dérangement fixe
| _MEMSSP_DANGER | 	Sécurité des Personnes | 	Module | 	FALSE | TRUE si le module remonte un danger imminent
| _MEMSSP_DANGER_FIXE | 	Sécurité des Personnes 	| Module | 	FALSE | 	TRUE si le module remonte un danger imminent (fixe).
| _MEMSSP_OK 	| Sécurité des Personnes 	| Système 	| TRUE | 	Bit de synthèse de la sécurité des personnes. Calculé par rapport aux 4 bits précédents.
| _OSYN_ACQUIT | 	Acquit 	| Système | 	FALSE | Bit positionné par le système lors d'une demande d'acquit synoptique
