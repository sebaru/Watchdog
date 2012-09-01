/**********************************************************************************************************/
/* Watchdogd/Admin/admin_process.c        Gestion des connexions Admin PROCESS au serveur watchdog        */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_process.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
 *
 * Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 #include <glib.h>
 #include <unistd.h>
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Activer_process: Appeller lorsque l'admin envoie une commande 'process' dans la ligne de commande      */
/* Entrée: La connexion cliente et la ligne de commande                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_process ( struct CLIENT_ADMIN *client, gchar *ligne )
  { struct LIBRAIRIE *lib;
    GSList *liste;
    gchar commande[128];

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { gchar thread[128], chaine[128];
       guint num;
       sscanf ( ligne, "%s %s %d", commande, thread, &num );

       g_snprintf( chaine, sizeof(chaine), " Trying to start %s\n", thread );
       Write_admin ( client->connexion, chaine );

       if ( ! strcmp ( thread, "arch" ) )
        { if (!Demarrer_arch())                                            /* Demarrage gestion Archivage */
           { Info_new( Config.log, Config.log_all, LOG_INFO, "Admin: Pb ARCH -> Arret" ); }
        } else
       if ( ! strcmp ( thread, "modbus" ) )
        { if (!Demarrer_modbus())                                      /* Demarrage gestion module MODBUS */
           { Info_new( Config.log, Config.log_all, LOG_INFO, "Admin: Pb MODBUS -> Arret" ); }
        } else
       if ( ! strcmp ( thread, "sms" ) )
        { if (!Demarrer_sms())                                                        /* Démarrage S.M.S. */
           { Info_new( Config.log, Config.log_all, LOG_INFO, "Admin: Pb SMS -> Arret" ); }
        } else
       if ( ! strcmp ( thread, "audio" ) )
        { if (!Demarrer_audio())                                                   /* Démarrage A.U.D.I.O */
           { Info_new( Config.log, Config.log_all, LOG_INFO, "Admin: Pb AUDIO -> Arret" ); }
        } else
       if ( ! strcmp ( thread, "dls" ) )
        { if (!Demarrer_dls())                                                        /* Démarrage D.L.S. */
           { Info_new( Config.log, Config.log_all, LOG_INFO, "Admin: Pb DLS -> Arret" ); }
        }  else
       if ( ! strcmp ( thread, "onduleur" ) )
        { if (!Demarrer_onduleur())                                                 /* Démarrage ONDULEUR */
           { Info_new( Config.log, Config.log_all, LOG_INFO, "Admin: Pb ONDULEUR -> Arret" ); }
        }  else
       if ( ! strcmp ( thread, "tellstick" ) )
        { if (!Demarrer_tellstick())                                               /* Démarrage TELLSTICK */
           { Info_new( Config.log, Config.log_all, LOG_INFO, "Admin: Pb TELLSTICK -> Arret" ); }
        }  else
       if ( ! strcmp ( thread, "lirc" ) )
        { if (!Demarrer_lirc())                                                         /* Démarrage LIRC */
           { Info_new( Config.log, Config.log_all, LOG_INFO, "Admin: Pb LIRC -> Arret" ); }
        }  else
       if ( ! strcmp ( thread, "ssrv" ) )
        { if (num<0 || num>=Config.max_serveur)
           { g_snprintf( chaine, sizeof(chaine), " num %d out of range\n", num );
             Write_admin ( client->connexion, chaine );
           }
          else if (!Demarrer_sous_serveur(num))                                    /* Démarrage d'un SSRV */
           { Info_new( Config.log, Config.log_all, LOG_INFO, "Admin: Pb SSRV -> Arret" ); }
          else Gerer_jeton();                              /* Affectation du jeton a un des sous-serveurs */
        }
       else
        { GSList *liste;
          liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { struct LIBRAIRIE *lib;
             lib = (struct LIBRAIRIE *)liste->data;
             if ( ! strcmp( lib->admin_prompt, thread ) )
              { if (Start_librairie(lib))
                 { g_snprintf( chaine, sizeof(chaine), " Library %s started\n", lib->admin_prompt ); }
                else
                 { g_snprintf( chaine, sizeof(chaine), " Error while starting library %s\n", lib->admin_prompt ); }
                Write_admin ( client->connexion, chaine );
              }
             liste = liste->next;
           }
        }
     } else
    if ( ! strcmp ( commande, "load" ) )
     { gchar thread[128], chaine[128];
       struct LIBRAIRIE *lib;
       sscanf ( ligne, "%s %s", commande, thread );
       g_snprintf( chaine, sizeof(chaine), "libwatchdog-server-%s.so", thread );
       if ( (lib = Charger_librairie_par_fichier( NULL, chaine )) )/* Chargement de la librairie dynamique */
        { g_snprintf( chaine, sizeof(chaine), " Library %s loaded\n", thread );
          if (Start_librairie(lib))
           { g_snprintf( chaine, sizeof(chaine), " Library %s started\n", thread ); }
           else
           { g_snprintf( chaine, sizeof(chaine), " Error while starting library %s\n", thread ); }
        }
       else
        { g_snprintf( chaine, sizeof(chaine), " Error while loading library %s\n", thread ); }
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "unload" ) )
     { gchar thread[128], chaine[128];
       sscanf ( ligne, "%s %s", commande, thread );
       if (Decharger_librairie_par_prompt( thread ))               /* Déchargement de la librairie dynamique */
        { g_snprintf( chaine, sizeof(chaine), " Library %s stopped and unloaded\n", thread ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " Error while unloading library %s\n", thread ); }
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "stop" ) )
     { gchar thread[128], chaine[128];
       sscanf ( ligne, "%s %s", commande, thread );

       g_snprintf( chaine, sizeof(chaine), " Trying to stop %s\n", thread );
       Write_admin ( client->connexion, chaine );

       if ( ! strcmp ( thread, "all" ) )
        { Stopper_fils(FALSE);                           /* Termine tous les process sauf le thread ADMIN */
        } else
       if ( ! strcmp ( thread, "arch"      ) ) { Partage->com_arch.Thread_run      = FALSE; } else
       if ( ! strcmp ( thread, "modbus"    ) ) { Partage->com_modbus.Thread_run    = FALSE; } else
       if ( ! strcmp ( thread, "sms"       ) ) { Partage->com_sms.Thread_run       = FALSE; } else
       if ( ! strcmp ( thread, "audio"     ) ) { Partage->com_audio.Thread_run     = FALSE; } else
       if ( ! strcmp ( thread, "dls"       ) ) { Partage->com_dls.Thread_run       = FALSE; } else
       if ( ! strcmp ( thread, "onduleur"  ) ) { Partage->com_onduleur.Thread_run  = FALSE; } else
       if ( ! strcmp ( thread, "tellstick" ) ) { Partage->com_tellstick.Thread_run = FALSE; } else
       if ( ! strcmp ( thread, "lirc"      ) ) { Partage->com_lirc.Thread_run      = FALSE; }
       else
        { GSList *liste;
          liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { struct LIBRAIRIE *lib;
             lib = (struct LIBRAIRIE *)liste->data;
             if ( ! strcmp( lib->admin_prompt, thread ) )
              { if (Stop_librairie(lib))
                 { g_snprintf( chaine, sizeof(chaine), " Library %s (%s) stopped\n",
                   lib->admin_prompt, lib->nom_fichier );
                 }
                else
                 { g_snprintf( chaine, sizeof(chaine), " Error while stopping library %s (%s) \n",
                               lib->admin_prompt, lib->nom_fichier );
                 }
                Write_admin ( client->connexion, chaine );
              }
             liste = liste->next;
           }
        }

     } else
    if ( ! strcmp ( commande, "list" ) )
     { gchar chaine[128];
       guint i;

       g_snprintf( chaine, sizeof(chaine), " -- Liste des process\n" );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Partage->top = %d\n", Partage->top );
       Write_admin ( client->connexion, chaine );

       for (i=0; i<Config.max_serveur; i++)
        { g_snprintf( chaine, sizeof(chaine), " Built-in SSRV[%d]  -> ------------- running = %s, TID = %d\n", i,
                      (Partage->Sous_serveur[i].Thread_run ? "YES" : " NO"), (gint)Partage->Sous_serveur[i].pid
                    );
          Write_admin ( client->connexion, chaine );
        }

       g_snprintf( chaine, sizeof(chaine), " Built-in D.L.S    -> ------------- running = %s, TID = %d\n",
                   (Partage->com_dls.Thread_run ? "YES" : " NO"), (gint)Partage->com_dls.TID
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Built-in SMS      -> ------------- running = %s, TID = %d\n",
                   (Partage->com_sms.Thread_run ? "YES" : " NO"), (gint)Partage->com_sms.TID
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Built-in MODBUS   -> ------------- running = %s, TID = %d\n",
                   (Partage->com_modbus.Thread_run ? "YES" : " NO"), (gint)Partage->com_modbus.TID
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Built-in AUDIO    -> ------------- running = %s, TID = %d\n",
                   (Partage->com_audio.Thread_run ? "YES" : " NO"), (gint)Partage->com_audio.TID
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Built-in ONDULEUR -> ------------- running = %s, TID = %d\n",
                   (Partage->com_onduleur.Thread_run ? "YES" : " NO"), (gint)Partage->com_onduleur.TID
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Built-in ARCHIVE  -> ------------- running = %s, TID = %d\n",
                   (Partage->com_arch.Thread_run ? "YES" : " NO"), (gint)Partage->com_arch.TID
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Library LIRC      -> loaded = %s, running = %s, TID = %d\n",
                   (Partage->com_lirc.dl_handle ? "YES" : " NO"),
                   (Partage->com_lirc.Thread_run ? "YES" : " NO"), (gint)Partage->com_lirc.TID
                 );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Library TELLSTICK -> loaded = %s, running = %s, TID = %d\n",
                   (Partage->com_tellstick.dl_handle ? "YES" : " NO"),
                   (Partage->com_tellstick.Thread_run ? "YES" : " NO"), (gint)Partage->com_tellstick.TID
                 );
       Write_admin ( client->connexion, chaine );


       liste = Partage->com_msrv.Librairies;                           /* Parcours de toutes les librairies */
       while(liste)
        { gchar result[256];
          lib = (struct LIBRAIRIE *)liste->data;
          memset ( result, ' ', sizeof(result) );
          memcpy ( result + 0, " Library ", 9 );
          memcpy ( result + 9, lib->admin_prompt, strlen(lib->admin_prompt) );
          if (lib->Thread_run == TRUE)
           { memcpy ( result + 25, "-> running YES, TID = ", 22 ); }
          else
           { memcpy ( result + 25, "-> running  NO, TID = ", 22 ); }
          g_snprintf( chaine, sizeof(chaine), "%d (%s)\n", (gint) lib->TID, lib->nom_fichier );
          memcpy( result + 47, chaine, strlen(chaine) + 1 );   /* +1 pour choper le \0 de fin de chaine ! */
          Write_admin ( client->connexion, result );
          liste = liste->next;
        }

     } else
    if ( ! strcmp ( commande, "SHUTDOWN" ) )
     { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Admin_process : SHUTDOWN demandé" );
       Write_admin ( client->connexion, "SHUTDOWN in progress\n" );
       Ajouter_audio( 9998 ); /* Message audio avant reboot */
       sleep(2);
       Partage->com_msrv.Thread_run = FALSE;
     } else
    if ( ! strcmp ( commande, "REBOOT" ) )
     { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Admin_process : REBOOT demandé" );
       Write_admin ( client->connexion, "REBOOT in progress\n" );
       Ajouter_audio( 9999 ); /* Message audio avant reboot */
       sleep(2);
       Partage->com_msrv.Thread_reboot = TRUE;
       Partage->com_msrv.Thread_run = FALSE;
     } else
    if ( ! strcmp ( commande, "CLEAR-REBOOT" ) )
     { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Admin_process : CLEAR-REBOOT demandé" );
       Write_admin ( client->connexion, "CLEAR-REBOOT in progress\n" );
       Partage->com_msrv.Thread_clear_reboot = TRUE;
       Partage->com_msrv.Thread_reboot = TRUE;
       Partage->com_msrv.Thread_run = FALSE;
     } else
    if ( ! strcmp ( commande, "RELOAD" ) )
     { Info_new( Config.log, Config.log_all, LOG_NOTICE, "Admin_process : RELOAD demandé" );
       Write_admin ( client->connexion, "RELOAD in progress\n" );
       Partage->com_msrv.Thread_reload = TRUE;
     } else
    if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'PROCESS'\n" );
       Write_admin ( client->connexion,
                     "  load thread          - Load a library (but not start it !)\n" );
       Write_admin ( client->connexion,
                     "  unload thread        - Unload a library\n" );
       Write_admin ( client->connexion,
                     "  start thread         - Start a thread (arch,rs485,modbus,sms,audio,dls,onduleur,tellstick,ssrv num, or library name)\n" );
       Write_admin ( client->connexion,
                     "  stop                 - Stop thread (all,arch,rs485,modbus,sms,audio,dls,onduleur,tellstick,ssrv, or library name)\n" );
       Write_admin ( client->connexion,
                     "  list                 - Liste les statut des threads\n" );
       Write_admin ( client->connexion,
                     "  RELOAD               - Reload configuration\n" );
       Write_admin ( client->connexion,
                     "  REBOOT               - Restart all processes\n" );
       Write_admin ( client->connexion,
                     "  CLEAR-REBOOT         - Restart all processes with no DATA import/export\n" );
       Write_admin ( client->connexion,
                     "  SHUTDOWN             - Stop processes\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown PROCESS command : %s\n", ligne );
       Write_admin ( client->connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
