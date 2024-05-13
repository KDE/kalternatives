/***************************************************************************
 *   Copyright (C) 2004 by Juanjo Álvarez Martinez <juanjo@juanjoalvarez.net> *
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

#ifndef _KALTERNATIVES_H_
#define _KALTERNATIVES_H_

#include <KCModule>
#include <KPluginMetaData>
#include "ui_mainwindow.h"

class AlternativeItemProxyModel;
class AlternativeAltModel;

class Kalternatives : public KCModule
{
    Q_OBJECT

    bool m_bisRoot;
	Ui::MainWindow m_ui;
	AlternativeItemProxyModel* m_itemProxyModel;
	AlternativeAltModel* m_altModel;
	
public:
    Kalternatives(QObject *parent = nullptr, const KPluginMetaData &data = {});
    ~Kalternatives() override;
	bool isBisRoot() const {return m_bisRoot;}
	
	void load() override;
	void save() override;

public Q_SLOTS:
	void configChanged();

private Q_SLOTS:
	void slotSelectAlternativesActivated(const QModelIndex &);
	void slotHideAlternativesClicked();
	void slotAddClicked();
	void slotDeleteClicked();
	void slotPropertiesClicked();
	void slotUpdateStatusCombo();
	void slotUpdateButtons();
};

#endif // _KALTERNATIVES_H_
