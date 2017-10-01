/***************************************************************************
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "slavewidget.h"
#include "altparser.h"

#include <klocalizedstring.h>

SlaveWidget::SlaveWidget(Slave *slave, QWidget *parent)
    : QWidget(parent), m_slave(slave)
{
    setupUi(this);
    layout()->setMargin(0);

    m_slavePath->setWindowTitle(i18n("Choose Slave"));
    m_slavePath->setFilter(i18n("*|All Files"));
    m_slavePath->setMode(KFile::File | KFile::LocalOnly);

    m_slaveText->setText(i18n("Slave link for %1:", m_slave->slname));

    connect(m_slavePath, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));
}

SlaveWidget::~SlaveWidget()
{
}

QString SlaveWidget::slavePath() const
{
    return m_slavePath->url().toLocalFile();
}

void SlaveWidget::slotTextChanged(const QString &)
{
    emit slaveChanged(slavePath());
}

#include <slavewidget.moc>
