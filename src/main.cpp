/***************************************************************************
 *   Copyright (C) 2004 by Juanjo                                          *
 *   juanjux@yahoo.es                                                      *
 *                                                                         *
 *   Copyright (C) 2004 by Mario Bensi                                     *
 *   nef@ipsquad.net                                                       *
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
 *   51 Franklin Steet, Fifth Floor, Boston, MA  02111-1307, USA.          *
 ***************************************************************************/

#include "main.h"
#include "kalternatives.h"
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kmessagebox.h>

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
					KAboutData::License_GPL, "(C) 2004 Juanjo Alvarez Martinez and Mario Bensi", 0, 0, "juanjux@yahoo.es, nef@ipsquad.net");
	about.addAuthor( "Juanjo Alvarez Martinez and Mario Bensi", 0, "juanjux@yahoo.es, nef@ipsquad.net" );
	
	KCmdLineArgs::init(argc, argv, &about);
	KCmdLineArgs::addCmdLineOptions( options );
	KUniqueApplication rapp;
	app = &rapp;
	Kalternatives *mainWin = 0;

	if (app->isRestored())
	{
		//RESTORE(kalternatives);
	}
	else
	{
		// no session.. just start up normally
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

		/// @todo do something with the command line args here

		mainWin = new Kalternatives();
		app->setMainWidget( mainWin );
		mainWin->show();
		

		args->clear();
	}
	return app->exec();
}

