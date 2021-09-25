# Programmation D.L.S: la zone de logique

C'est dans cette zone où toute l'intelligence logique du plugin va résider.
C'est elle qui définit l'ensemble des comportements attendus en fonction de l'environnement du module.

Cette zone est composée d'une suite de ligne (appelée dans la suite « ligne D.L.S ») dont le contenu d'une ligne suit la syntaxe suivante :

     - EXPRESSION -> LISTE_ACTIONS;

Elle commence par un tiret « - », suivi d'une « EXPRESSION », suivi par une flèche (tiret « - » puis supérieur « > »), une liste d'actions séparées par des virgules et enfin, un point virgule terminal.
Zone de calcul

##Lier une expression à une action

une « EXPRESSION » est un **ALIAS**, ou un ensemble **ALIAS** liés entre eux par des [opérateurs de base](operateurs-de-base).

« LISTE_ACTIONS » est une liste d'un ou plusieurs d'**ALIAS**, séparés par des virgules.

Chacun des **ALIAS**, s'il est suivi par des parenthèses peut être complété par une [liste d'options](dls_options.md),
elles aussi séparées par des virgules.

Voici un exemple de syntaxe :

    - PORTE:OUVERTE . ( /VERROU + TEMPO_BIPE ) → VISUEL_PORTE(mode="ouvert,couleur="red"); /* Un exemple */

Nous détaillons la grammaire dans chacun des paragraphes ci dessous.

## Opérateurs de base

###Le ET « . »

Dans une « EXPRESSION », le ET « . » permet d'opérer la fonction logique ET entre deux sous-expressions.
« a . b est vrai » si et seulement si « a est vrai » et « b est vrai ».
Exemple de syntaxe :
 - a . b → c;                /* Si a et b sont vrais alors nous positionnons c */

Le ET n'a pas de sens dans une « LISTE_ACTIONS ».

### Le OU « + »

Dans une « EXPRESSION », le OU « + » permet d'opérer la fonction logique OU entre deux sous-expressions.
« a + b est vrai » si « a est vrai » ou « b est vrai ».
Exemple de syntaxe :
 - a + b → c;              /* Si a ou b sont vrais alors nous positionnons c */

Le OU n'a pas de sens dans une « LISTE_ACTIONS ».

### Le complément « / »

Dans une « EXPRESSION » ou une « LISTE_ACTIONS », le complément « / » permet d'opérer la fonction logique NON sur l'expression suivante.
« /a est vrai » si « a est faux ».

Exemple de syntaxe :

    - /a → c;          /* Si a est faux alors nous positionnons c à 1 */
    - a . b → /c;      /* Si a et b sont vrais alors nous positionnons c à 0 */

### Précédences et parenthèses

Les priorités d'opérations sont les suivantes, dans l'ordre décroissant de priorité :
* Le NON
* Le ET
* Le OU

Ce système de priorité peut être modifié en utilisant les parenthèses ouvrantes et fermantes.

Exemple de syntaxe:

    -  a + b . c → d;        /* Si a est vrai, ou b et c sont vrais, alors nous positionnons d à 1 */
    - (a+b) . c → d;        /* Si a ou b est vrai, et c est vrai alors nous positionnons d à 1 */
    - a . /(b+c) → /c;      /* Si a est vrai, et que l'on a ni b ni c, alors nous positionnons c à 0 */

##Logique étendue

Il est possible de compléter un comportement par des options. La modification du comportement sera fonction des options elles-mêmes.
La syntaxe retenue est la suivante:

     - EXPRESSION -- liste_options -> LISTE_ACTIONS;

Aujourd'hui, les options disponibles sont les suivantes;

* daa: Ajoute un délai entre le momemt ou l'EXPRESSION devrait vraie et le moment ou la LISTE_ACTIONS sera effectivement positionnée

     - EXPRESSION -- daa=100 -> ACTION; /* ACTION sera lancée 10 secondes après que l'EXPRESSION devienne vraie.
