# Programmation D.L.S: les Entrées T.O.R

Une entrée TOR (Tout Ou Rien) est un bit interne de la [mémoire d'information](dls.md#memoire-d'informations), de classse *_E*.

Chaque entrée TOR à pour valeur booléenne soit 0, soit 1, et ne peut être positionnée que dans une **EXPRESSION**.

## Déclarer une Entrée TOR

Dans la zone d'[ALIAS](dls_alias.md), déclarez une entrée TOR de cette façon:

    #define ACRONYME <-> _E(options);

Elle commence directement par une chaine de caractères représentant l'**ACRONYME**, puis une double flèche (un inférieur « < », un tiret « - », un supérieur « > »),
la classe _E, et enfin un point virgule terminal.

Les options suivantes peuvent être affectées à ce bit interne :

* libelle: Permet d'adjoindre une description au bit interne.

Exemple complet:

    /* Nous sommes dans le DLS "PORTE" */
    #define MON_ENTREE <-> _E (libelle="Capteur d'ouverture de la porte");

## Usage dans une EXPRESSION

Exemple de syntaxe dans une **EXPRESSION**:

    - MON_ENTREE → MON_BISTABLE;       /* Si MON_ENTREE = 1 alors MON_BISTABLE = 1 */
    - /MON_ENTREE → /MON_BISTABLE;     /* Si MON_ENTREE = 0 alors MON_BISTABLE = 0 */

## Variation DLS

Dans une **EXPRESSION**, il est possible de moduler la sémantique du bit interne en utilisant les options suivantes:

* MON_ENTREE(edge_up): permet de ne prendre en compte que les fronts montants (vrai lorsque l'entrée passe de 0 à 1)
* MON_ENTREE(edge_down): permet de ne prendre en compte que les fronts descendants (vrai lorsque l'entrée passe de 1 à 0)
