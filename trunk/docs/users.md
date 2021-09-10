# Catégorie d'utilisateurs

Chaque utilisateur dispose d'un niveau d'habilitation, représenté par un numéro de 1 à 9.
Un utilisateur d'un niveau *N* peut:

* Voir et modifier toutes les ressources d'un niveau inférieure ou égale à *N-1*
* Voir les ressources d'un niveau égal à *N*
* Voir et modifier ses propres ressources

Il ne pourra pas accéder ou modifier les ressources de rang plus élevé.

Par défaut, un utilisateur nouvellement créé sera associé au niveau *1*.

## Les rangs *1* à *5*

Les niveaux *1* à *5* sont les niveaux à destination des clients finaux.

## Les rangs *6* à *9*

Les niveaux *6* à *9* sont les niveaux reservés aux utilisateurs de profil **techniciens**.
Ces utilisateurs à privilèges possèdent les droits de modifier le coeur du système:

* L'édition, l'ajout ou la suppression des [modules D.L.S](dls.md)
* L'édition, l'ajout ou la suppression des synoptiques
* La configuration des [connecteurs](connecteurs.md)
