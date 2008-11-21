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

//#ifdef HAVE_CONFIG_H
//#include <config.h>
//#endif

#include <k3listview.h>
#include <kpushbutton.h> 
#include <kcombobox.h> 
#include <qwidget.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <kcmodule.h>
#include <kaboutdata.h>

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
	KAboutData *myAboutData;
	
	void clearList(K3ListView* list);
	
public:
    Kalternatives(QWidget *parent=0, const QVariantList& = QVariantList() );
    virtual ~Kalternatives();
	K3ListView *optionsList() const {return m_optionsList;}
	bool isBisRoot() const {return m_bisRoot;}
	
	virtual void load();
	virtual void save();
	virtual QString quickHelp() const;
	virtual const KAboutData *aboutData()const { return myAboutData; };

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
