# Les Visuels **_I**

Les visuels, bit de classe **_I** vous permettent de matérialiser un objet, une commande, ou encore un état.
Chaque visuel est représenté par le **TECH_ID** de son D.L.S parent, et par son **ACRONYME**.
Par exemple, voici la définition la plus simple d'un visuel:

    /* Nous sommes dans le D.L.S "SONO" */
    /* Déclaration d'un visuel MON_VISUEL, de forme 'haut_parleur' */
    #define MON_VISUEL <-> _I(forme="haut_parleur");

Cette déclaration vous permet ensuite d'utiliser **MON_VISUEL** a l'intérieur même du D.L.S **SONO**.
Pour faire apparaitre ce visuel sur un autre D.L.S, il faudra utiliser un [lien](dls_link.md) précisant son identification complète
**SONO:MON_VISUEL**.

---
## Comment piloter un visuel

Chaque visuel s'appui sur des options, permettant de définir facilement la déclinaison souhaitée par le [technicien](/users.md)
Ces options sont représentées dans la grammaire [D.L.S](/dls.md) par les mots clefs suivants:

* `mode`: il s'agit du mode principal d'affichage du visuel. Pour une porte, il peut s'agir du mode `ouverte` ou `fermée` par exemple.
* `color`: il s'agit de la couleur souhaitée d'affichage du visuel. Les couleurs sont représentées par leurs noms en anglais.
* `cligno`: cette option permet de faire clignoter le visuel dans l'interface.

Voici des exemples de déclinaisons d'une même forme selon plusieurs `mode` et `color`:

    /* Nous sommes dans le D.L.S "SONO" */
    /* Déclaration d'un visuel MON_VISUEL, de forme 'haut_parleur' */
    /* Par défaut, le visuel est dans le mode inactif, en blanc */
    #define MON_HAUT_PARLEUR <-> _I(forme="haut_parleur", mode="inactif", color="white");

    /* Si MA_CONDITION est vraie, le visuel est actif, de couleur rouge, */
    /* et clignotant */
    - MA_CONDITION -> MON_HAUT_PARLEUR(mode="actif", color="red", cligno);

---
##Les couleurs

Les couleurs HTML sont reconnues, dans le format string, comme ci dessous:

* black
* white
* blue
* darkgreen
* gray
* green
* lightblue
* orange
* red
* yellow

---
## Les visuels complexes

Certaines formes de visuelles sont particulières, dans le sens ou le visuel en question est construit de manière dynamique.
Ces visuels complexes sont les suivants:

| forme            |  description |
|:----------------:|:-------------|
| bouton           | Ce visuel affiche un bouton dont le titre est le `libelle` |
| bloc_maintenance | Ce visuel affiche le bloc Service/Maintenance, en fonction du `mode` |
| [comment](dls_visuel_comment.md) | Ce visuel affiche son `libelle` sous la forme de texte, avec une police et une couleur dependants du `mode` et `color` |
| encadre          | Ce visuel affiche un cadre de couleur `color`, surmonté d'un titre selon le `libelle`. La taille du cadre est determinée selon sont `mode` |




