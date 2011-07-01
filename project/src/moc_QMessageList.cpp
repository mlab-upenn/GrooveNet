/****************************************************************************
** QMessageList meta object code from reading C++ file 'QMessageList.h'
**
** Created: Thu May 19 14:47:00 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "QMessageList.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QMessageList::className() const
{
    return "QMessageList";
}

QMetaObject *QMessageList::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QMessageList( "QMessageList", &QMessageList::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QMessageList::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QMessageList", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QMessageList::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QMessageList", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QMessageList::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"QMessageList", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QMessageList.setMetaObject( metaObj );
    return metaObj;
}

void* QMessageList::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QMessageList" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool QMessageList::qt_invoke( int _id, QUObject* _o )
{
    return QWidget::qt_invoke(_id,_o);
}

bool QMessageList::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QMessageList::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool QMessageList::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
