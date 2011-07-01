/****************************************************************************
** QFilePushButton meta object code from reading C++ file 'QFilePushButton.h'
**
** Created: Thu May 19 14:46:55 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "QFilePushButton.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QFilePushButton::className() const
{
    return "QFilePushButton";
}

QMetaObject *QFilePushButton::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QFilePushButton( "QFilePushButton", &QFilePushButton::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QFilePushButton::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QFilePushButton", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QFilePushButton::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QFilePushButton", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QFilePushButton::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QPushButton::staticMetaObject();
    static const QUMethod slot_0 = {"slotChangeFile", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "slotChangeFile()", &slot_0, QMetaData::Protected }
    };
    static const QUMethod signal_0 = {"fileChanged", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "fileChanged()", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"QFilePushButton", parentObject,
	slot_tbl, 1,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QFilePushButton.setMetaObject( metaObj );
    return metaObj;
}

void* QFilePushButton::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QFilePushButton" ) )
	return this;
    return QPushButton::qt_cast( clname );
}

// SIGNAL fileChanged
void QFilePushButton::fileChanged()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool QFilePushButton::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotChangeFile(); break;
    default:
	return QPushButton::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool QFilePushButton::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: fileChanged(); break;
    default:
	return QPushButton::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool QFilePushButton::qt_property( int id, int f, QVariant* v)
{
    return QPushButton::qt_property( id, f, v);
}

bool QFilePushButton::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
