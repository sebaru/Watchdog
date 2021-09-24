# Programmation D.L.S: la zone de calcul

Le calcul D.L.S permet, en fonction d'une condition prédéfinie,
de calculer la valeur d'un registre en fonction d'expression booléennes ou arithmétiques.
Tout comme la ligne logique, celle-ci commence par un tiret « - », suivi d'une « EXPRESSION »
qui représente la condition de calcul, suivi d'un tiret, puis,
entre parenthèse, d'un calcul arithmétique, suivi pour finir d'une flèche (tiret « - » puis supérieur « > »),
et un registre de destination. Le point virgule terminal reste obligatoire.

Exemple, si la condition **EXPRESSION** est vraie, alors le Registre **RESULTAT** sera mis à jour avec la valeur du **CALCUL** entre parenthèses.

     - EXPRESSION - ( CALCUL ) -> RESULTAT;

Si l'EXPRESSION est fausse, le calcul n'est pas réalisé.

2nd Exemple:

     - SYS:TOP_1SEC - ( VITESSE + 1.0 ) -> VITESSE;

Toutes les secondes, le registre **VITESSE** est augmenté de 1.
