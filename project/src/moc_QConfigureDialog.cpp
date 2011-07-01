/****************************************************************************
** QConfigureDialog meta object code from reading C++ file 'QConfigureDialog.h'
**
** Created: Thu May 19 14:46:54 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "QConfigureDialog.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QConfigureDialog::className() const
{
    return "QConfigureDialog";
}

QMetaObject *QConfigureDialog::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QConfigureDialog( "QConfigureDialog", &QConfigureDialog::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QConfigureDialog::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QConfigureDialog", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QConfigureDialog::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QConfigureDialog", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QConfigureDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"slotOK", 0, 0 };
    static const QUMethod slot_1 = {"slotApply", 0, 0 };
    static const QUMethod slot_2 = {"slotDefault", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"slotSettingsChanged", 2, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In },
	{ "button", &static_QUType_int, 0, QUParameter::In },
	{ "mousePos", &static_QUType_varptr, "\x0e", QUParameter::In }
    };
    static const QUMethod slot_4 = {"slotTableDoubleClicked", 4, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In },
	{ "button", &static_QUType_int, 0, QUParameter::In },
	{ "mousePos", &static_QUType_varptr, "\x0e", QUParameter::In }
    };
    static const QUMethod slot_5 = {"slotTableClicked", 4, param_slot_5 };
    static const QMetaData slot_tbl[] = {
	{ "slotOK()", &slot_0, QMetaData::Protected },
	{ "slotApply()", &slot_1, QMetaData::Protected },
	{ "slotDefault()", &slot_2, QMetaData::Protected },
	{ "slotSettingsChanged(int,int)", &slot_3, QMetaData::Protected },
	{ "slotTableDoubleClicked(int,int,int,const QPoint&)", &slot_4, QMetaData::Protected },
	{ "slotTableClicked(int,int,int,const QPoint&)", &slot_5, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"QConfigureDialog", parentObject,
	slot_tbl, 6,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QConfigureDialog.setMetaObject( metaObj );
    return metaObj;
}

void* QConfigureDialog::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QConfigureDialog" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool QConfigureDialog::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotOK(); break;
    case 1: slotApply(); break;
    case 2: slotDefault(); break;
    case 3: slotSettingsChanged((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 4: slotTableDoubleClicked((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(const QPoint&)*((const QPoint*)static_QUType_ptr.get(_o+4))); break;
    case 5: slotTableClicked((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(const QPoint&)*((const QPoint*)static_QUType_ptr.get(_o+4))); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool QConfigureDialog::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QConfigureDialog::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool QConfigureDialog::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
