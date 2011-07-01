/****************************************************************************
** QBoundingRegionConfDialog meta object code from reading C++ file 'QBoundingRegionConfDialog.h'
**
** Created: Thu May 19 14:46:59 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "QBoundingRegionConfDialog.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QBoundingRegionConfDialog::className() const
{
    return "QBoundingRegionConfDialog";
}

QMetaObject *QBoundingRegionConfDialog::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QBoundingRegionConfDialog( "QBoundingRegionConfDialog", &QBoundingRegionConfDialog::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QBoundingRegionConfDialog::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QBoundingRegionConfDialog", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QBoundingRegionConfDialog::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QBoundingRegionConfDialog", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QBoundingRegionConfDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"slotMapSelectionChanged", 0, 0 };
    static const QUParameter param_slot_1[] = {
	{ "strText", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"slotParamChanged", 1, param_slot_1 };
    static const QMetaData slot_tbl[] = {
	{ "slotMapSelectionChanged()", &slot_0, QMetaData::Protected },
	{ "slotParamChanged(const QString&)", &slot_1, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"QBoundingRegionConfDialog", parentObject,
	slot_tbl, 2,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QBoundingRegionConfDialog.setMetaObject( metaObj );
    return metaObj;
}

void* QBoundingRegionConfDialog::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QBoundingRegionConfDialog" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool QBoundingRegionConfDialog::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotMapSelectionChanged(); break;
    case 1: slotParamChanged((const QString&)static_QUType_QString.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool QBoundingRegionConfDialog::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QBoundingRegionConfDialog::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool QBoundingRegionConfDialog::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
