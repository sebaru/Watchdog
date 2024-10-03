/******************************************************************************************************************************/
/* Watchdogd/mail.c        Fonctions helper pour la manipulation des payload au format JSON                                   */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                                27.06.2019 09:38:40 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * mail.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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

 #include "watchdogd.h"
/******************************************************************************************************************************/
/* Send_mail: Envoi un mail en utilisation le sendmail du system                                                              */
/* Entrée: Le sujet, le destinataire, le corps du message                                                                     */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Send_mail ( gchar *sujet, gchar *dest, gchar *body )
  { gchar fichier[32], commande[128], chaine[256];
    gint fd;
    g_snprintf( fichier, sizeof(fichier), "WTDMail_XXXXXX" );
    fd = mkstemp ( fichier );
    if (fd==-1)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "mkstemp failed" );
       return(FALSE);
     }

    g_snprintf( chaine, sizeof(chaine), "From: WatchdogServer\n"
                                        "Subject: %s\n"
                                        "To: %s\n"
                                        "Content-Type: text/html\n"
                                        "MIME-Version: 1.0\n"
                                        "\n", sujet, dest );

    if (write ( fd, chaine, strlen(chaine) ) < 0)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "writing header failed" );
       close(fd);
       return(FALSE);
     }


    gchar *headers = "<html><body>\n";
    if (write ( fd, headers, strlen(headers) ) < 0)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "writing header failed" );
       close(fd);
       return(FALSE);
     }

    if (write ( fd, body, strlen(body) ) < 0)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "writing body failed" );
       close(fd);
       return(FALSE);
     }

    gchar *suffixe = "</body></html>";
    if (write ( fd, suffixe, strlen(suffixe) ) < 0)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "writing suffixe failed" );
       close(fd);
       return(FALSE);
     }
    close(fd);

    g_snprintf ( commande, sizeof(commande), "cat %s | sendmail -t", fichier );
    system(commande);
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "mail '%s' sent to '%s'", sujet, dest );
    unlink(fichier);
    return(TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
