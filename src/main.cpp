/***************************************************************************
 *   Copyright (C) 2004 by Juanjo                                          *
 *   juanjux@yahoo.es                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "main.h"
#include "kalternatives.h"
#include "mainwindow.h"
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

KUniqueApplication *app = 0; // Global

static const char description[] =
    I18N_NOOP("A KDE Manager for the Debian/Mandrake alternatives system");

static const char version[] = "0.10";

static KCmdLineOptions options[] =
{
//    { "+[URL]", I18N_NOOP( "Document to open." ), 0 },
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("kalternatives", I18N_NOOP("kalternatives"), version, description,
                     KAboutData::License_GPL, "(C) 2004 Juanjo Alvarez Martinez", 0, 0, "juanjux@yahoo.es");
    about.addAuthor( "Juanjo Alvarez Martinez", 0, "juanjux@yahoo.es" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );
    /*KUniqueApplication rapp;
    app = &rapp;
    kalternatives *mainWin = 0;

    if (app->isRestored())
    {
        //RESTORE(kalternatives);
    }
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        /// @todo do something with the command line args here

        mainWin = new kalternatives();
        app->setMainWidget( mainWin->m_mainwindow );
        mainWin->m_mainwindow->show();
		

        args->clear();
    }*/
	KApplication app( argc, argv );

    //mainwindow mainwin;
	Kalternatives *mainwin = new Kalternatives();
    app.setMainWidget( mainwin/*.m_mainwindow*/ );
    mainwin/*.m_mainwindow*/->show();

	// mainWin has WDestructiveClose flag by default, so it will delete itself.
    return app.exec();
	
    
}

