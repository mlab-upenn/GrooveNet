/****************************************************************************
** QAutoGenModelDialog meta object code from reading C++ file 'QAutoGenModelDialog.h'
**
** Created: Thu May 19 14:46:58 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "QAutoGenModelDialog.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QAutoGenModelDialog::className() const
{
    return "QAutoGenModelDialog";
}

QMetaObject *QAutoGenModelDialog::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QAutoGenModelDialog( "QAutoGenModelDialog", &QAutoGenModelDialog::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QAutoGenModelDialog::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QAutoGenModelDialog", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QAutoGenModelDialog::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QAutoGenModelDialog", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QAutoGenModelDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"slotPropertiesValueChanged", 2, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"slotPropertiesCurrentChanged", 2, param_slot_1 };
    static const QUMethod slot_2 = {"accept", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "slotPropertiesValueChanged(int,int)", &slot_0, QMetaData::Protected },
	{ "slotPropertiesCurrentChanged(int,int)", &slot_1, QMetaData::Protected },
	{ "accept()", &slot_2, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"QAutoGenModelDialog", parentObject,
	slot_tbl, 3,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QAutoGenModelDialog.setMetaObject( metaObj );
    return metaObj;
}

void* QAutoGenModelDialog::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QAutoGenModelDialog" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool QAutoGenModelDialog::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotPropertiesValueChanged((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 1: slotPropertiesCurrentChanged((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 2: accept(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool QAutoGenModelDialog::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QAutoGenModelDialog::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool QAutoGenModelDialog::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
