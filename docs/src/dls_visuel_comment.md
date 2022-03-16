# Les Visuels Commentaires

Les visuels **commentaires** d'un module D.L.S sont des visuels de `forme`="comment" représentant son `libelle`.
Par exemple, voici la définition complète d'un visuel **commentaire**:

    /* Nous sommes dans le D.L.S "TECH_ID" */
    /* Déclaration d'un visuel de forme 'commentaire' */
    #define MON_LIBELLE <-> _I(forme="comment", libelle="Voici mon titre", mode="titre", color="yellow", cligno);

Cette déclaration permet de creer un commentaire **Voici mon titre**, en tant que **titre**, de couleur **yellow**, et clignotant.


---
##Les catégories de commentaires

Il existe 3 catégories de commentaires. Celles ci sont indiquées dans le `mode` du commentaire lui-même.
Celles-ci sont les suivantes:

| `mode` | Description |
|:------:|-------------|
| titre  | Le commentaire est affiché en taille 38 |
| soustitre  | Le commentaire est affiché en taille 26 |
| annotation | Le commentaire est affiché en taille 18 |


Si le `mode` n'est pas renseigné, celui-ci sera considéré comme **annotation**.

##Les couleurs des commentaires

Toutes les [couleurs](dls_visuels.md#les-couleurs) traditionnelles sont reconnues.

Quelques exemples pilotant un commentaire :

    /* Nous sommes dans le D.L.S "TECH_ID" */
    #define COMMENT <-> _I(forme="comment", libelle="mon commentaire");
    - condition_1 -> COMMENT(color="red");   /* le libelle est rouge si la condition 1 est active */
    -/condition_1 -> COMMENT(color="green"); /* le libelle est vert si la condition 1 est fausse */
    - condition_2 -> COMMENT(color="yellow");  /* le libelle est jaune si la condition 2 est active */

##Attribut de clignotement

Un commentaire, peut éventuellement etre clignotant, si l'option `cligno` est renseignée dans ses options.
Par défaut, il ne l'est pas.
