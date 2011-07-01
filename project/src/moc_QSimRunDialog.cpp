/****************************************************************************
** QSimRunDialog meta object code from reading C++ file 'QSimRunDialog.h'
**
** Created: Thu May 19 14:46:55 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "QSimRunDialog.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QSimRunDialog::className() const
{
    return "QSimRunDialog";
}

QMetaObject *QSimRunDialog::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QSimRunDialog( "QSimRunDialog", &QSimRunDialog::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QSimRunDialog::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QSimRunDialog", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QSimRunDialog::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QSimRunDialog", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QSimRunDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"slotSimulationType", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "value", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_1 = {"slotTrialsChanged", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "on", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_2 = {"slotDurationToggled", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "strText", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"slotDurationChanged", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"slotSimulationTime", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ "strText", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"slotIncrementChanged", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ "index", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_6 = {"slotMessageHighlighted", 1, param_slot_6 };
    static const QUMethod slot_7 = {"slotAddMessage", 0, 0 };
    static const QUMethod slot_8 = {"slotEditMessage", 0, 0 };
    static const QUMethod slot_9 = {"slotRemoveMessage", 0, 0 };
    static const QUParameter param_slot_10[] = {
	{ "on", &static_QUType_bool, 0, QUParameter::In }
    };
    static const QUMethod slot_10 = {"slotProfileToggled", 1, param_slot_10 };
    static const QMetaData slot_tbl[] = {
	{ "slotSimulationType(int)", &slot_0, QMetaData::Protected },
	{ "slotTrialsChanged(int)", &slot_1, QMetaData::Protected },
	{ "slotDurationToggled(bool)", &slot_2, QMetaData::Protected },
	{ "slotDurationChanged(const QString&)", &slot_3, QMetaData::Protected },
	{ "slotSimulationTime(int)", &slot_4, QMetaData::Protected },
	{ "slotIncrementChanged(const QString&)", &slot_5, QMetaData::Protected },
	{ "slotMessageHighlighted(int)", &slot_6, QMetaData::Protected },
	{ "slotAddMessage()", &slot_7, QMetaData::Protected },
	{ "slotEditMessage()", &slot_8, QMetaData::Protected },
	{ "slotRemoveMessage()", &slot_9, QMetaData::Protected },
	{ "slotProfileToggled(bool)", &slot_10, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"QSimRunDialog", parentObject,
	slot_tbl, 11,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QSimRunDialog.setMetaObject( metaObj );
    return metaObj;
}

void* QSimRunDialog::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QSimRunDialog" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool QSimRunDialog::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotSimulationType((int)static_QUType_int.get(_o+1)); break;
    case 1: slotTrialsChanged((int)static_QUType_int.get(_o+1)); break;
    case 2: slotDurationToggled((bool)static_QUType_bool.get(_o+1)); break;
    case 3: slotDurationChanged((const QString&)static_QUType_QString.get(_o+1)); break;
    case 4: slotSimulationTime((int)static_QUType_int.get(_o+1)); break;
    case 5: slotIncrementChanged((const QString&)static_QUType_QString.get(_o+1)); break;
    case 6: slotMessageHighlighted((int)static_QUType_int.get(_o+1)); break;
    case 7: slotAddMessage(); break;
    case 8: slotEditMessage(); break;
    case 9: slotRemoveMessage(); break;
    case 10: slotProfileToggled((bool)static_QUType_bool.get(_o+1)); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool QSimRunDialog::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QSimRunDialog::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool QSimRunDialog::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
