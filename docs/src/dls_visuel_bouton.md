# Les Visuels Bouton

Les visuels **bouton** d'un module D.L.S sont des visuels de `forme`="bouton" représentant un bouton dont le titre est son `libelle`
et affiché de la couleur `color`.
Par exemple, voici la définition complète d'un visuel **bouton**:

    /* Nous sommes dans le D.L.S "TECH_ID" */
    /* Déclaration d'un visuel de forme 'bouton' */
    #define MON_BOUTON <-> _I(forme="bouton", libelle="Cliquez moi !", mode="enabled", color="blue");

Cette déclaration permet de creer un bouton "Cliquez moi !", de couleur **bleu**, et clignotant.

Afin de capter l'évènement de clic de la part de l'utilisateur,
a chaque bouton est automatique accroché un bit **DI** nommé par la concaténation de son acronyme et de **_CLIC**.

Exemple:

    #define MON_BOUTON <-> _I(forme="bouton", libelle="Cliquez moi !", color="blue");
    /*------MON_BOUTON_CLIC <-> _DI; Automatiquement le bit DI MON_BOUTON_CLIC est créé */


---
##Les modes associés aux boutons

Il existe 2 modes de boutons.
Ceux-ci sont les suivants:

| `mode` | Description |
|:------:|-------------|
| enabled  | Le bouton est actif et l'utilisateur peut cliquer dessus |
| disabled | Le bouton est inactif et l'utilisateur ne peut pas cliquer dessus |

Si le `mode` n'est pas renseigné, celui-ci sera considéré par défaut comme **enabled**.

##Les couleurs des boutons

Les couleurs possibles des boutons sont les suivantes:

* blue
* red
* orange
* green
* grey
* black

##Attribut de clignotement

Un bouton, peut éventuellement etre clignotant, si l'option `cligno` est renseignée dans ses options.
Cependant, l'usage de cet attribut peut nuire a la compréhension.
