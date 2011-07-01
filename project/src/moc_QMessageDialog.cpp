/****************************************************************************
** QMessageDialog meta object code from reading C++ file 'QMessageDialog.h'
**
** Created: Thu May 19 14:46:59 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "QMessageDialog.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QMessageDialog::className() const
{
    return "QMessageDialog";
}

QMetaObject *QMessageDialog::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QMessageDialog( "QMessageDialog", &QMessageDialog::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QMessageDialog::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QMessageDialog", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QMessageDialog::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QMessageDialog", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QMessageDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"slotMsgTypeChanged", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "index", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"slotBoundingRegionTypeChanged", 1, param_slot_1 };
    static const QUMethod slot_2 = {"slotBoundingRegionConfig", 0, 0 };
    static const QUMethod slot_3 = {"accept", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "slotMsgTypeChanged(int)", &slot_0, QMetaData::Protected },
	{ "slotBoundingRegionTypeChanged(int)", &slot_1, QMetaData::Protected },
	{ "slotBoundingRegionConfig()", &slot_2, QMetaData::Protected },
	{ "accept()", &slot_3, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"QMessageDialog", parentObject,
	slot_tbl, 4,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QMessageDialog.setMetaObject( metaObj );
    return metaObj;
}

void* QMessageDialog::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QMessageDialog" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool QMessageDialog::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotMsgTypeChanged((int)static_QUType_int.get(_o+1)); break;
    case 1: slotBoundingRegionTypeChanged((int)static_QUType_int.get(_o+1)); break;
    case 2: slotBoundingRegionConfig(); break;
    case 3: accept(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool QMessageDialog::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QMessageDialog::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool QMessageDialog::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
