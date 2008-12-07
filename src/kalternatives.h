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

#include <kcmodule.h>

#include "ui_mainwindow.h"

#define KALT_VERSION "0.12"

class AltFilesManager;
class AlternativeItemProxyModel;
class AlternativeAltModel;

class Kalternatives : public KCModule
{
    Q_OBJECT

    bool m_bisRoot;
    AltFilesManager *m_mgr;
	Ui::MainWindow m_ui;
	AlternativeItemProxyModel* m_itemProxyModel;
	AlternativeAltModel* m_altModel;
	
public:
    Kalternatives(QWidget *parent=0, const QVariantList& = QVariantList() );
    virtual ~Kalternatives();
	bool isBisRoot() const {return m_bisRoot;}
	
	virtual void load();
	virtual void save();
	virtual QString quickHelp() const;

public slots:
	void configChanged();

private slots:
	void slotSelectAlternativesActivated(const QModelIndex &);
	void slotHideAlternativesClicked();
    void die();
	void slotAddClicked();
	void slotDeleteClicked();
	void slotPropertiesClicked();
	void slotUpdateStatusCombo();
};

#endif // _KALTERNATIVES_H_
