# Programmation D.L.S: les Alias

Un alias permet de créer un bit interne dans la [mémoire d'information](dls.md#memoire-d'informations) en lui donnant une classe et des paramètres de configuration

## Déclarer un ALIAS

Cette zone permet d'associer un **ACRONYME** à une classe de bit interne au sein d'un module D.L.S **TECH_ID**.
Cet **ACRONYME** pourra être utilisé dans le code de fonctionnement du module **TECH_ID** lui-même.
Il pourra également être utilisé dans d'autres modules D.L.S sous sa forme complète **TECH_ID**:**ACRONYME**

Il s'agit d'une zone dont la syntaxe est la suivante:

    #define ACRONYME <-> _CLASSE;

Elle commence directement par une chaine de caractères représentant l'**ACRONYME**, puis une double flèche (un inférieur « < », un tiret « - », un supérieur « > »), une Classe, et enfin un point virgule terminal.

Des options peuvent être affectées à cet alias, en utilisant la syntaxe suivante :

    #define ACRONYME <-> _CLASSE (OPTIONS); /* OPTIONS étant une liste de champ=valeur, séparée par des virgules ',' */

Exemple de syntaxe complète :

    #define VOLET_OUVERT <-> _E; /* VOLET_OUVERT sera mappé à une Entrée Physique (par exemple, à un module MODBUS, ou à la reception d'un SMS) */
    #define CDE_VOLET    <-> _M;    /* CDE_VOLET sera utilisé en tant que monostable dans la suite du code D.L.S */
