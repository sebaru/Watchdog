# Liste des visuels

Cette page présente la liste des visuels disponibles dans l'interface Watchdog.

## Comment piloter un visuel

Chaque visuel s'appui sur des options, permettant de définir facilement la déclinaison souhaitée par le [technicien](/users.md)
Ces options sont représentées dans la grammaire [D.L.S](/dls.md) par les mots clefs suivants:

* *mode*: il s'agit du mode principal d'affichage du visuel. Pour une porte, il peut s'agir du mode *ouverte* ou *fermée*
* *color*: il s'agit de la couleur souhaitée d'affichage du visuel. Les couleurs sont représentées par leurs noms en anglais.
* *cligno*: cette option permet de faire clignoter le visuel dans l'interface.

Voici des exemples de déclinaisons d'une même forme selon plusieurs *mode* et *color*:

    /* Déclaration d'un visuel 'haut_parleur' */
    /* Par défaut, le visuel est dans le mode inactif, en blanc */
    #define MA_PORTE <-> _I(forme="haut_parleur", mode="inactif", color="white");

    /* Si MA_CONDITION est vraie, le visuel est actif, de couleur rouge, */
    /* et clignotant */
    - MA_CONDITION -> MA_PORTE(mode="actif", color="red", cligno);

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

## Les catégories d'affichage

Certains visuels proposent plusieurs affichage selon le *mode*
* `mkdocs new [dir-name]` - Create a new project.
* `mkdocs serve` - Start the live-reloading docs server.
* `mkdocs build` - Build the documentation site.
* `mkdocs -h` - Print help message and exit.

## Project layout

    mkdocs.yml    # The configuration file.
    docs/
        index.md  # The documentation homepage.
        ...       # Other markdown pages, images and other files.

## Test

![testtest](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/bouton_io_green.png "Title")
`pirintf`

---------------------
| 1 | 2 | 3 |   |   |
|---|---|---|---|---|
|   |   |   |

ggg
---
###tt
