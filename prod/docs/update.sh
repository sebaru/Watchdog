#/bin/sh

read  -p "password:" -r PASSWORD

#echo "SELECT * from icone" | mysql -u watchdog WatchdogDB -p$PASSWORD | tail -n +2 > categorie.sql
echo "SELECT DISTINCT categorie FROM icone" | mysql -u watchdog WatchdogDB -p$PASSWORD | tail -n +2 > categorie.sql

#| while read -r line
for CAT in $(cat categorie.sql)
	do
  echo;
  echo "------------- Processing Categorie $CAT"

  RESULT=src/visuels_$CAT.md
  echo "" > $RESULT
  echo "# Liste des visuels de la catégorie '"$CAT"'" >> $RESULT
  echo "" >> $RESULT

  echo "SELECT forme, extension, ihm_affichage FROM icone WHERE categorie='"$CAT"'" | mysql -u watchdog WatchdogDB -p$PASSWORD | tail -n +2 > forme.sql

  cat forme.sql | while read -r line
   do
    FORME=$(echo $line | cut -d ' ' -f1 -)
    EXTENSION=$(echo $line | cut -d ' ' -f2 -)
    IHM_AFFICHAGE=$(echo $line | cut -d ' ' -f3 -)

    if [ $IHM_AFFICHAGE = "complexe" ]
     then
      continue
    fi

    echo "------------- processing $CAT - $FORME - $EXTENSION - $IHM_AFFICHAGE"

    echo "---" >> $RESULT
    echo "## \`forme\`='$FORME'" >> $RESULT
    echo "" >> $RESULT

    if [ $IHM_AFFICHAGE = "by_color" ]
     then
       echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$FORME"_white."$EXTENSION")" >> $RESULT
       echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$FORME"_lightblue."$EXTENSION")" >> $RESULT
       echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$FORME"_blue."$EXTENSION")" >> $RESULT
       echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$FORME"_darkgreen."$EXTENSION")" >> $RESULT
       echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$FORME"_gray."$EXTENSION")" >> $RESULT
       echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$FORME"_green."$EXTENSION")" >> $RESULT
       echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$FORME"_orange."$EXTENSION")" >> $RESULT
       echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$FORME"_red."$EXTENSION")" >> $RESULT
       echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$FORME"_yellow."$EXTENSION")" >> $RESULT
       echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$FORME"_black."$EXTENSION")" >> $RESULT
    fi

    if [ $IHM_AFFICHAGE = "by_mode" ]
     then
      echo "Modes:" >> $RESULT
      echo "" >> $RESULT

       for FILE in $(ls ../Watchdogd/IHM/img/$FORME*$EXTENSION)
        do
          step=$(basename $FILE .$EXTENSION)
          taille=$((${#FORME}+1))
          MODE=${step:$taille}
          echo "* $MODE" >> $RESULT
        done

       echo "" >> $RESULT
       for FILE in $(ls ../Watchdogd/IHM/img/$FORME*$EXTENSION)
        do
          step=$(basename $FILE)
          echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$step")" >> $RESULT
        done

    fi

    if [ $IHM_AFFICHAGE = "by_mode_color" ]
     then
      echo "Modes:" >> $RESULT
      echo "" >> $RESULT

       for FILE in $(ls ../Watchdogd/IHM/img/$FORME*_source.$EXTENSION)
        do
          step=$(basename $FILE _source.$EXTENSION)
          taille=$((${#FORME}+1))
          MODE=${step:$taille}
          echo "* $MODE" >> $RESULT
        done

       echo "" >> $RESULT
       for FILE in $(ls ../Watchdogd/IHM/img/$FORME*_source.$EXTENSION)
        do
          step=$(basename $FILE _source.$EXTENSION)
          echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$step"_white."$EXTENSION")" >> $RESULT
          echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$step"_lightblue."$EXTENSION")" >> $RESULT
          echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$step"_blue."$EXTENSION")" >> $RESULT
          echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$step"_darkgreen."$EXTENSION")" >> $RESULT
          echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$step"_gray."$EXTENSION")" >> $RESULT
          echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$step"_green."$EXTENSION")" >> $RESULT
          echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$step"_orange."$EXTENSION")" >> $RESULT
          echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$step"_red."$EXTENSION")" >> $RESULT
          echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$step"_yellow."$EXTENSION")" >> $RESULT
          echo "![imgvisuel](https://svn.abls-habitat.fr/repo/Watchdog/prod/Watchdogd/IHM/img/"$step"_black."$EXTENSION")" >> $RESULT
          echo "" >> $RESULT
        done

    fi

    echo "" >> $RESULT
   done
  svn add $RESULT
  rm forme.sql
done

rm categorie.sql
