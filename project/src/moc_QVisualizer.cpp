/****************************************************************************
** QVisualizer meta object code from reading C++ file 'QVisualizer.h'
**
** Created: Thu May 19 14:46:52 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "QVisualizer.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QVisualizer::className() const
{
    return "QVisualizer";
}

QMetaObject *QVisualizer::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QVisualizer( "QVisualizer", &QVisualizer::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QVisualizer::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QVisualizer", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QVisualizer::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QVisualizer", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QVisualizer::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"QVisualizer", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QVisualizer.setMetaObject( metaObj );
    return metaObj;
}

void* QVisualizer::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QVisualizer" ) )
	return this;
    return QWidget::qt_cast( clname );
}

bool QVisualizer::qt_invoke( int _id, QUObject* _o )
{
    return QWidget::qt_invoke(_id,_o);
}

bool QVisualizer::qt_emit( int _id, QUObject* _o )
{
    return QWidget::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QVisualizer::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool QVisualizer::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
