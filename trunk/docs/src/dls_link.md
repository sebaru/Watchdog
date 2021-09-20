# Programmation D.L.S: les liens, ou dépendances inter-modules

Les liens (ou dépendances) sont utilisés pour référencer dans un module **cible** un bit interne d'un module **source**.
Cela permet également, en y ajoutant les **options** adéquates, de compléter la définition en ajoutant par exemple un [cadran](dls_cadran.md)
sur un synoptique particulier.

## Déclaration des liens

Un lien se définit de la manière suivante dans la zone d'**Alias** du module D.L.S **cible**:

    #link TECH_ID_SOURCE:ACRONYME_SOURCE (liste_options);

**TECH_ID_SOURCE** et **ACRONYME_SOURCE** referencent le bit interne distant faisant l'objet du lien.
La **liste_options** permet de surcharger les options associés au bit interne cible.

Typiquement, prenons l'exemple du synoptique **METEO**, auquel est associé le module DLS **VENT**,
proposant lui-même l'[entrée analogique](dls_ea.md) **VITESSE**:

    #define VITESSE <-> _AI(libelle="Vitesse du vent mesurée par le capteur", unite="m/s" );

Pour matérialiser un [cadran](dls_cadran.md) sur ce bit interne **VENT:VITESSE** sur un synoptique **MON_AUTRE_SYNOPTIQUE**
hébergeant le module D.L.S **MON_AUTRE_DLS**, déclarez un lien de la manière suivante:

     /* Module DLS 'MON_AUTRE_DLS', associé au synoptique 'MON_AUTRE_SYNOPTIQUE' */
     #link VITESSE:VENT (cadran="simple");

## Suppression d'un lien

Pour supprimer un lien d'un module DLS particulier, il suffit de supprimer sa ligne de déclaration
