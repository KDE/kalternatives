/***************************************************************************
 *   Copyright (C) 2004 by Mario Bensi <nef@ipsquad.net>                   *
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
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

#include "addalternatives.h"
#include "altparser.h"
#include "slavewidget.h"

#include <klocale.h>
#include <kseparator.h>

AddAlternatives::AddAlternatives(Item* item, int slaveCount, QWidget *parent)
	: KDialog(parent), m_item(item), m_alternative(0), m_countSlave(slaveCount)
{
	setupUi(mainWidget());
	mainWidget()->layout()->setMargin(0);
	
	setButtons(Ok | Cancel);
	setCaption(i18n("Add Alternative"));
	showButtonSeparator(true);
	
	m_Path->setWindowTitle( i18n( "Choose Alternative" ) );
	m_Path->setFilter( i18n( "*|All Files" ) );
	m_Path->setMode( KFile::File | KFile::LocalOnly );
	
	if (m_countSlave > 0)
	{
		SlaveList *slaves = item->getSlaves();
		
		QWidget *w = new QWidget;
		QVBoxLayout *lay = new QVBoxLayout(w);
		for (int i = 0; i < m_countSlave; ++i)
		{
			if (i > 0)
				lay->addWidget(new KSeparator(Qt::Horizontal, w));
			SlaveWidget *sw = new SlaveWidget(slaves->at(i), w);
			lay->addWidget(sw);
			m_slaveWidgets.append(sw);
			connect(sw, SIGNAL(slaveChanged(QString)), this, SLOT(slotCheckSlaves()));
		}
		w->show();
		m_slavesArea->setWidget(w);
	}
	else
	{
		m_slavesGroup->hide();
	}
	
	enableButtonOk(false);
	connect(m_Path, SIGNAL(textChanged(QString)), this, SLOT(slotCheckSlaves()));
	connect(this, SIGNAL(okClicked()), this, SLOT(slotOkClicked()));
}

AddAlternatives::~AddAlternatives()
{
}

QSize AddAlternatives::sizeHint() const
{
	return QSize(400, KDialog::sizeHint().height());
}

void AddAlternatives::slotCheckSlaves()
{
	bool ok = !m_Path->url().isEmpty();
	int i = 0;
	while ((i < m_slaveWidgets.count()) && ok)
	{
		ok = !m_slaveWidgets.at(i)->slavePath().isEmpty();
		++i;
	}
	
	enableButtonOk(ok);
}

void AddAlternatives::slotOkClicked()
{
	m_alternative = new Alternative(m_item);
	Q_ASSERT(!m_Path->url().toLocalFile().isEmpty());
	m_alternative->setPath(m_Path->url().toLocalFile());
	m_alternative->setPriority(m_Priority->value());
	Q_FOREACH (SlaveWidget *sw, m_slaveWidgets)
	{
		Q_ASSERT(!sw->slavePath().isEmpty());
		m_alternative->addSlave(sw->slavePath());
	}
}

#include "addalternatives.moc"
