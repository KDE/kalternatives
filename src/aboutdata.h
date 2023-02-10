/***************************************************************************
 *   Copyright (C) 2008-2009 by Pino Toscano <pino@kde.org>                *
 *   Copyright (C) 2008 by Armin Berres <armin@space-based.de>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef ABOUTDATA_H
#define ABOUTDATA_H

#include <kaboutdata.h>

#define KALT_VERSION "0.13"

inline KAboutData aboutData(const QString &name, const char* iname)
{
    KAboutData about(
        name,
        i18n(iname),
        KALT_VERSION,
        i18n("KDE manager for the distribution alternatives system."),
        KAboutLicense::GPL,
        i18n("© 2004 Juanjo Álvarez Martinez\n"
             "© 2004 Mario Bensi\n"
             "© 2008-2009 Pino Toscano")
    );

    about.addAuthor(i18n("Pino Toscano"), i18n("Current maintainer"), "pino@kde.org");
    about.addAuthor(i18n("Juanjo Alvarez Martinez"), i18n("Original author"), "juanjo@juanjoalvarez.net", "http://juanjoalvarez.net");
    about.addAuthor(i18n("Mario Bensi"), i18n("Original author"), "nef@ipsquad.net", "http://ipsquad.net");

    return about;
}

#endif
