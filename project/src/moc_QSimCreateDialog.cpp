/****************************************************************************
** QSimCreateDialog meta object code from reading C++ file 'QSimCreateDialog.h'
**
** Created: Thu May 19 14:46:56 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "QSimCreateDialog.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QSimCreateDialog::className() const
{
    return "QSimCreateDialog";
}

QMetaObject *QSimCreateDialog::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QSimCreateDialog( "QSimCreateDialog", &QSimCreateDialog::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QSimCreateDialog::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QSimCreateDialog", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QSimCreateDialog::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QSimCreateDialog", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QSimCreateDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "index", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"slotVehiclesListHighlighted", 1, param_slot_0 };
    static const QUMethod slot_1 = {"slotVehiclesListSelChanged", 0, 0 };
    static const QUParameter param_slot_2[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"slotVehiclesAdd", 1, param_slot_2 };
    static const QUMethod slot_3 = {"slotVehiclesRemove", 0, 0 };
    static const QUMethod slot_4 = {"slotVehiclesDuplicate", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ "strName", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"slotVehicleNameChanged", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"slotVehiclePropertiesValueChanged", 2, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"slotVehiclePropertiesCurrentChanged", 2, param_slot_7 };
    static const QUMethod slot_8 = {"slotVehiclesAutoGen", 0, 0 };
    static const QUMethod slot_9 = {"slotVehiclesRemoveAll", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ "index", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"slotInfrastructureNodesListHighlighted", 1, param_slot_10 };
    static const QUMethod slot_11 = {"slotInfrastructureNodesListSelChanged", 0, 0 };
    static const QUParameter param_slot_12[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_12 = {"slotInfrastructureNodesAdd", 1, param_slot_12 };
    static const QUMethod slot_13 = {"slotInfrastructureNodesRemove", 0, 0 };
    static const QUMethod slot_14 = {"slotInfrastructureNodesDuplicate", 0, 0 };
    static const QUParameter param_slot_15[] = {
	{ "strName", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_15 = {"slotInfrastructureNodeNameChanged", 1, param_slot_15 };
    static const QUParameter param_slot_16[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_16 = {"slotInfrastructureNodePropertiesValueChanged", 2, param_slot_16 };
    static const QUParameter param_slot_17[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_17 = {"slotInfrastructureNodePropertiesCurrentChanged", 2, param_slot_17 };
    static const QUMethod slot_18 = {"slotInfrastructureNodesAutoGen", 0, 0 };
    static const QUMethod slot_19 = {"slotInfrastructureNodesRemoveAll", 0, 0 };
    static const QUParameter param_slot_20[] = {
	{ "index", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_20 = {"slotModelsListHighlighted", 1, param_slot_20 };
    static const QUMethod slot_21 = {"slotModelsListSelChanged", 0, 0 };
    static const QUParameter param_slot_22[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_22 = {"slotModelsAdd", 1, param_slot_22 };
    static const QUMethod slot_23 = {"slotModelsRemove", 0, 0 };
    static const QUMethod slot_24 = {"slotModelsDuplicate", 0, 0 };
    static const QUParameter param_slot_25[] = {
	{ "strName", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_25 = {"slotModelNameChanged", 1, param_slot_25 };
    static const QUParameter param_slot_26[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_26 = {"slotModelPropertiesValueChanged", 2, param_slot_26 };
    static const QUParameter param_slot_27[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_27 = {"slotModelPropertiesCurrentChanged", 2, param_slot_27 };
    static const QUMethod slot_28 = {"slotModelsRemoveAll", 0, 0 };
    static const QUMethod slot_29 = {"slotOK", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "slotVehiclesListHighlighted(int)", &slot_0, QMetaData::Protected },
	{ "slotVehiclesListSelChanged()", &slot_1, QMetaData::Protected },
	{ "slotVehiclesAdd(int)", &slot_2, QMetaData::Protected },
	{ "slotVehiclesRemove()", &slot_3, QMetaData::Protected },
	{ "slotVehiclesDuplicate()", &slot_4, QMetaData::Protected },
	{ "slotVehicleNameChanged(const QString&)", &slot_5, QMetaData::Protected },
	{ "slotVehiclePropertiesValueChanged(int,int)", &slot_6, QMetaData::Protected },
	{ "slotVehiclePropertiesCurrentChanged(int,int)", &slot_7, QMetaData::Protected },
	{ "slotVehiclesAutoGen()", &slot_8, QMetaData::Protected },
	{ "slotVehiclesRemoveAll()", &slot_9, QMetaData::Protected },
	{ "slotInfrastructureNodesListHighlighted(int)", &slot_10, QMetaData::Protected },
	{ "slotInfrastructureNodesListSelChanged()", &slot_11, QMetaData::Protected },
	{ "slotInfrastructureNodesAdd(int)", &slot_12, QMetaData::Protected },
	{ "slotInfrastructureNodesRemove()", &slot_13, QMetaData::Protected },
	{ "slotInfrastructureNodesDuplicate()", &slot_14, QMetaData::Protected },
	{ "slotInfrastructureNodeNameChanged(const QString&)", &slot_15, QMetaData::Protected },
	{ "slotInfrastructureNodePropertiesValueChanged(int,int)", &slot_16, QMetaData::Protected },
	{ "slotInfrastructureNodePropertiesCurrentChanged(int,int)", &slot_17, QMetaData::Protected },
	{ "slotInfrastructureNodesAutoGen()", &slot_18, QMetaData::Protected },
	{ "slotInfrastructureNodesRemoveAll()", &slot_19, QMetaData::Protected },
	{ "slotModelsListHighlighted(int)", &slot_20, QMetaData::Protected },
	{ "slotModelsListSelChanged()", &slot_21, QMetaData::Protected },
	{ "slotModelsAdd(int)", &slot_22, QMetaData::Protected },
	{ "slotModelsRemove()", &slot_23, QMetaData::Protected },
	{ "slotModelsDuplicate()", &slot_24, QMetaData::Protected },
	{ "slotModelNameChanged(const QString&)", &slot_25, QMetaData::Protected },
	{ "slotModelPropertiesValueChanged(int,int)", &slot_26, QMetaData::Protected },
	{ "slotModelPropertiesCurrentChanged(int,int)", &slot_27, QMetaData::Protected },
	{ "slotModelsRemoveAll()", &slot_28, QMetaData::Protected },
	{ "slotOK()", &slot_29, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"QSimCreateDialog", parentObject,
	slot_tbl, 30,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QSimCreateDialog.setMetaObject( metaObj );
    return metaObj;
}

void* QSimCreateDialog::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QSimCreateDialog" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool QSimCreateDialog::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotVehiclesListHighlighted((int)static_QUType_int.get(_o+1)); break;
    case 1: slotVehiclesListSelChanged(); break;
    case 2: slotVehiclesAdd((int)static_QUType_int.get(_o+1)); break;
    case 3: slotVehiclesRemove(); break;
    case 4: slotVehiclesDuplicate(); break;
    case 5: slotVehicleNameChanged((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: slotVehiclePropertiesValueChanged((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 7: slotVehiclePropertiesCurrentChanged((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 8: slotVehiclesAutoGen(); break;
    case 9: slotVehiclesRemoveAll(); break;
    case 10: slotInfrastructureNodesListHighlighted((int)static_QUType_int.get(_o+1)); break;
    case 11: slotInfrastructureNodesListSelChanged(); break;
    case 12: slotInfrastructureNodesAdd((int)static_QUType_int.get(_o+1)); break;
    case 13: slotInfrastructureNodesRemove(); break;
    case 14: slotInfrastructureNodesDuplicate(); break;
    case 15: slotInfrastructureNodeNameChanged((const QString&)static_QUType_QString.get(_o+1)); break;
    case 16: slotInfrastructureNodePropertiesValueChanged((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 17: slotInfrastructureNodePropertiesCurrentChanged((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 18: slotInfrastructureNodesAutoGen(); break;
    case 19: slotInfrastructureNodesRemoveAll(); break;
    case 20: slotModelsListHighlighted((int)static_QUType_int.get(_o+1)); break;
    case 21: slotModelsListSelChanged(); break;
    case 22: slotModelsAdd((int)static_QUType_int.get(_o+1)); break;
    case 23: slotModelsRemove(); break;
    case 24: slotModelsDuplicate(); break;
    case 25: slotModelNameChanged((const QString&)static_QUType_QString.get(_o+1)); break;
    case 26: slotModelPropertiesValueChanged((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 27: slotModelPropertiesCurrentChanged((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 28: slotModelsRemoveAll(); break;
    case 29: slotOK(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool QSimCreateDialog::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QSimCreateDialog::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool QSimCreateDialog::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
