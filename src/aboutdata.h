/***************************************************************************
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef ABOUTDATA_H
#define ABOUTDATA_H

#include <kaboutdata.h>

#define KALT_VERSION "0.12"

inline KAboutData aboutData(const char* name, const char* iname)
{
    KAboutData about(
        name,
        "kalternatives",
        ki18n(iname),
        KALT_VERSION,
        ki18n("KDE manager for the distribution alternatives system."),
        KAboutData::License_GPL,
        ki18n("© 2004 Juanjo Álvarez Martinez\n"
              "© 2004 Mario Bensi\n"
              "© 2008-2009 Pino Toscano")
    );

    about.addAuthor(ki18n("Pino Toscano"), ki18n("Current maintainer"), "pino@kde.org");
    about.addAuthor(ki18n("Juanjo Alvarez Martinez"), ki18n("Original author"), "juanjo@juanjoalvarez.net", "http://juanjoalvarez.net");
    about.addAuthor(ki18n("Mario Bensi"), ki18n("Original author"), "nef@ipsquad.net", "http://ipsquad.net");

    return about;
}

#endif
