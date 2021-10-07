#/bin/sh

ALL=$(echo "SELECT * from icone" | mysql -u watchdog WatchdogDB -p | tail -n +2 | cut -f 2)

for CAT in $ALL

	do
		echo Processing cateforie $CAT
		FORMES=$(ls ../Watchdogd/IHM/img/$CAT_*.svg)
                for FORME in $FORMES
		do
			echo $CAT - $FORME
		done
	done
