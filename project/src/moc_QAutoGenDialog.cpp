/****************************************************************************
** QAutoGenDialog meta object code from reading C++ file 'QAutoGenDialog.h'
**
** Created: Thu May 19 14:46:57 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "QAutoGenDialog.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QAutoGenDialog::className() const
{
    return "QAutoGenDialog";
}

QMetaObject *QAutoGenDialog::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QAutoGenDialog( "QAutoGenDialog", &QAutoGenDialog::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QAutoGenDialog::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QAutoGenDialog", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QAutoGenDialog::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QAutoGenDialog", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QAutoGenDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "index", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"slotVehicleTypeActivated", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "index", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"slotParameterActivated", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "index", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"slotRegionTypeActivated", 1, param_slot_2 };
    static const QUMethod slot_3 = {"slotMapSelectionChanged", 0, 0 };
    static const QUParameter param_slot_4[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"slotPropertiesValueChanged", 2, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"slotPropertiesCurrentChanged", 2, param_slot_5 };
    static const QUMethod slot_6 = {"accept", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "slotVehicleTypeActivated(int)", &slot_0, QMetaData::Protected },
	{ "slotParameterActivated(int)", &slot_1, QMetaData::Protected },
	{ "slotRegionTypeActivated(int)", &slot_2, QMetaData::Protected },
	{ "slotMapSelectionChanged()", &slot_3, QMetaData::Protected },
	{ "slotPropertiesValueChanged(int,int)", &slot_4, QMetaData::Protected },
	{ "slotPropertiesCurrentChanged(int,int)", &slot_5, QMetaData::Protected },
	{ "accept()", &slot_6, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"QAutoGenDialog", parentObject,
	slot_tbl, 7,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QAutoGenDialog.setMetaObject( metaObj );
    return metaObj;
}

void* QAutoGenDialog::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QAutoGenDialog" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool QAutoGenDialog::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotVehicleTypeActivated((int)static_QUType_int.get(_o+1)); break;
    case 1: slotParameterActivated((int)static_QUType_int.get(_o+1)); break;
    case 2: slotRegionTypeActivated((int)static_QUType_int.get(_o+1)); break;
    case 3: slotMapSelectionChanged(); break;
    case 4: slotPropertiesValueChanged((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 5: slotPropertiesCurrentChanged((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 6: accept(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool QAutoGenDialog::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QAutoGenDialog::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool QAutoGenDialog::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
