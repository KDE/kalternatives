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

#ifndef _KALTERNATIVES_H_
#define _KALTERNATIVES_H_

#include <kcmodule.h>

#include "ui_mainwindow.h"

#define KALT_VERSION "0.12"

class AltFilesManager;

class Kalternatives : public KCModule
{
    Q_OBJECT

    bool m_bisRoot;
    AltFilesManager *m_mgr;
	K3ListView* m_optionsList;
	K3ListView* m_altList;
	KComboBox* m_statusCombo;
	QLabel* m_altTilte;
	QCheckBox* m_hideAlt;
	Ui::MainWindow m_ui;
	
	void clearList(K3ListView* list);
	
public:
    Kalternatives(QWidget *parent=0, const QVariantList& = QVariantList() );
    virtual ~Kalternatives();
	K3ListView *optionsList() const {return m_optionsList;}
	bool isBisRoot() const {return m_bisRoot;}
	
	virtual void load();
	virtual void save();
	virtual QString quickHelp() const;

public slots:
	void configChanged();

private slots:
	void slotSelectAlternativesClicked(Q3ListViewItem *);
	void slotHideAlternativesClicked();
    void die();
	void slotOptionClicked(Q3ListViewItem *option);
	void slotAddClicked();
	void slotDeleteClicked();
	void slotPropertiesClicked();
};

#endif // _KALTERNATIVES_H_
