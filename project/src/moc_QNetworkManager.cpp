/****************************************************************************
** QNetworkManager meta object code from reading C++ file 'QNetworkManager.h'
**
** Created: Thu May 19 14:46:53 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "QNetworkManager.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QNetworkManager::className() const
{
    return "QNetworkManager";
}

QMetaObject *QNetworkManager::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QNetworkManager( "QNetworkManager", &QNetworkManager::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QNetworkManager::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QNetworkManager", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QNetworkManager::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QNetworkManager", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QNetworkManager::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUMethod slot_0 = {"slotNetworkInit", 0, 0 };
    static const QUMethod slot_1 = {"slotNetworkClose", 0, 0 };
    static const QUMethod slot_2 = {"slotNetworkServer", 0, 0 };
    static const QUParameter param_slot_3[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_3 = {"slotNetworkServer", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "strText", &static_QUType_QString, 0, QUParameter::In }
    };
    static const QUMethod slot_4 = {"slotClientTextChanged", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"slotClientConnect", 1, param_slot_5 };
    static const QUMethod slot_6 = {"slotClientDisconnect", 0, 0 };
    static const QUParameter param_slot_7[] = {
	{ "index", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_7 = {"slotClientListHighlighted", 1, param_slot_7 };
    static const QMetaData slot_tbl[] = {
	{ "slotNetworkInit()", &slot_0, QMetaData::Protected },
	{ "slotNetworkClose()", &slot_1, QMetaData::Protected },
	{ "slotNetworkServer()", &slot_2, QMetaData::Protected },
	{ "slotNetworkServer(int)", &slot_3, QMetaData::Protected },
	{ "slotClientTextChanged(const QString&)", &slot_4, QMetaData::Protected },
	{ "slotClientConnect(int)", &slot_5, QMetaData::Protected },
	{ "slotClientDisconnect()", &slot_6, QMetaData::Protected },
	{ "slotClientListHighlighted(int)", &slot_7, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"QNetworkManager", parentObject,
	slot_tbl, 8,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QNetworkManager.setMetaObject( metaObj );
    return metaObj;
}

void* QNetworkManager::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QNetworkManager" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool QNetworkManager::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotNetworkInit(); break;
    case 1: slotNetworkClose(); break;
    case 2: slotNetworkServer(); break;
    case 3: slotNetworkServer((int)static_QUType_int.get(_o+1)); break;
    case 4: slotClientTextChanged((const QString&)static_QUType_QString.get(_o+1)); break;
    case 5: slotClientConnect((int)static_QUType_int.get(_o+1)); break;
    case 6: slotClientDisconnect(); break;
    case 7: slotClientListHighlighted((int)static_QUType_int.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool QNetworkManager::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QNetworkManager::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool QNetworkManager::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
