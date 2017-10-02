/***************************************************************************
 *   Copyright (C) 2008 by Pino Toscano <pino@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef SLAVEWIDGET_H
#define SLAVEWIDGET_H

#include "ui_slavewidget.h"

struct Slave;

class SlaveWidget : public QWidget, private Ui::SlaveWidgetUi
{
    Q_OBJECT

public:
    explicit SlaveWidget(Slave *slave, QWidget *parent = Q_NULLPTR);
    ~SlaveWidget() Q_DECL_OVERRIDE;

    QString slavePath() const;

signals:
    void slaveChanged(const QString &);

private slots:
    void slotTextChanged(const QString &);

private:
    Slave *m_slave;
};

#endif
