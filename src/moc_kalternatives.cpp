/****************************************************************************
** ItemsWidget meta object code from reading C++ file 'kalternatives.h'
**
** Created: Mon Sep 6 23:28:38 2004
**      by: The Qt MOC ($Id$)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "kalternatives.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *ItemsWidget::className() const
{
    return "ItemsWidget";
}

QMetaObject *ItemsWidget::metaObj = 0;
static QMetaObjectCleanUp cleanUp_ItemsWidget( "ItemsWidget", &ItemsWidget::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString ItemsWidget::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "ItemsWidget", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString ItemsWidget::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "ItemsWidget", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* ItemsWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = KListView::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ 0, &static_QUType_ptr, "QListViewItem", QUParameter::In }
    };
    static const QUMethod slot_0 = {"slotItemClicked", 1, param_slot_0 };
    static const QMetaData slot_tbl[] = {
	{ "slotItemClicked(QListViewItem*)", &slot_0, QMetaData::Public }
    };
    static const QUMethod signal_0 = {"iwChanged", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "iwChanged()", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"ItemsWidget", parentObject,
	slot_tbl, 1,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_ItemsWidget.setMetaObject( metaObj );
    return metaObj;
}

void* ItemsWidget::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "ItemsWidget" ) )
	return this;
    return KListView::qt_cast( clname );
}

// SIGNAL iwChanged
void ItemsWidget::iwChanged()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool ItemsWidget::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotItemClicked((QListViewItem*)static_QUType_ptr.get(_o+1)); break;
    default:
	return KListView::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool ItemsWidget::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: iwChanged(); break;
    default:
	return KListView::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool ItemsWidget::qt_property( int id, int f, QVariant* v)
{
    return KListView::qt_property( id, f, v);
}

bool ItemsWidget::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *kalternatives::className() const
{
    return "kalternatives";
}

QMetaObject *kalternatives::metaObj = 0;
static QMetaObjectCleanUp cleanUp_kalternatives( "kalternatives", &kalternatives::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString kalternatives::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "kalternatives", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString kalternatives::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "kalternatives", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* kalternatives::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = KMainWindow::staticMetaObject();
    static const QUMethod slot_0 = {"slotApplyClicked", 0, 0 };
    static const QUMethod slot_1 = {"slotExpandClicked", 0, 0 };
    static const QUMethod slot_2 = {"slotCollapseClicked", 0, 0 };
    static const QUMethod slot_3 = {"slotCloseClicked", 0, 0 };
    static const QUMethod slot_4 = {"slotAboutClicked", 0, 0 };
    static const QUMethod slot_5 = {"slotSelectionChanged", 0, 0 };
    static const QUMethod slot_6 = {"die", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "slotApplyClicked()", &slot_0, QMetaData::Private },
	{ "slotExpandClicked()", &slot_1, QMetaData::Private },
	{ "slotCollapseClicked()", &slot_2, QMetaData::Private },
	{ "slotCloseClicked()", &slot_3, QMetaData::Private },
	{ "slotAboutClicked()", &slot_4, QMetaData::Private },
	{ "slotSelectionChanged()", &slot_5, QMetaData::Private },
	{ "die()", &slot_6, QMetaData::Private }
    };
    metaObj = QMetaObject::new_metaobject(
	"kalternatives", parentObject,
	slot_tbl, 7,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_kalternatives.setMetaObject( metaObj );
    return metaObj;
}

void* kalternatives::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "kalternatives" ) )
	return this;
    return KMainWindow::qt_cast( clname );
}

bool kalternatives::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotApplyClicked(); break;
    case 1: slotExpandClicked(); break;
    case 2: slotCollapseClicked(); break;
    case 3: slotCloseClicked(); break;
    case 4: slotAboutClicked(); break;
    case 5: slotSelectionChanged(); break;
    case 6: die(); break;
    default:
	return KMainWindow::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool kalternatives::qt_emit( int _id, QUObject* _o )
{
    return KMainWindow::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool kalternatives::qt_property( int id, int f, QVariant* v)
{
    return KMainWindow::qt_property( id, f, v);
}

bool kalternatives::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
