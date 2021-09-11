# Les Visuels **_I**

a compléter

---
## Comment piloter un visuel

Chaque visuel s'appui sur des options, permettant de définir facilement la déclinaison souhaitée par le [technicien](/users.md)
Ces options sont représentées dans la grammaire [D.L.S](/dls.md) par les mots clefs suivants:

* `mode`: il s'agit du mode principal d'affichage du visuel. Pour une porte, il peut s'agir du mode `ouverte` ou `fermée` par exemple.
* `color`: il s'agit de la couleur souhaitée d'affichage du visuel. Les couleurs sont représentées par leurs noms en anglais.
* `cligno`: cette option permet de faire clignoter le visuel dans l'interface.

Voici des exemples de déclinaisons d'une même forme selon plusieurs `mode` et `color`:

    /* Déclaration d'un visuel 'haut_parleur' */
    /* Par défaut, le visuel est dans le mode inactif, en blanc */
    #define MA_PORTE <-> _I(forme="haut_parleur", mode="inactif", color="white");

    /* Si MA_CONDITION est vraie, le visuel est actif, de couleur rouge, */
    /* et clignotant */
    - MA_CONDITION -> MA_PORTE(mode="actif", color="red", cligno);

---
## Couleurs possibles

Seules les couleurs suivantes sont reconnues:

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
## Les catégories d'affichage

Certains visuels proposent plusieurs affichage selon le `mode`


