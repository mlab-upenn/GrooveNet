/****************************************************************************
** QMapWidget meta object code from reading C++ file 'QMapWidget.h'
**
** Created: Thu May 19 14:46:57 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "QMapWidget.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QMapWidget::className() const
{
    return "QMapWidget";
}

QMetaObject *QMapWidget::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QMapWidget( "QMapWidget", &QMapWidget::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QMapWidget::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QMapWidget", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QMapWidget::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QMapWidget", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QMapWidget::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QWidget::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "e", &static_QUType_ptr, "QPaintEvent", QUParameter::In }
    };
    static const QUMethod slot_0 = {"paintEvent", 1, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "e", &static_QUType_ptr, "QResizeEvent", QUParameter::In }
    };
    static const QUMethod slot_1 = {"resizeEvent", 1, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "e", &static_QUType_ptr, "QKeyEvent", QUParameter::In }
    };
    static const QUMethod slot_2 = {"keyPressEvent", 1, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_3 = {"mousePressEvent", 1, param_slot_3 };
    static const QUParameter param_slot_4[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_4 = {"mouseMoveEvent", 1, param_slot_4 };
    static const QUParameter param_slot_5[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_5 = {"mouseReleaseEvent", 1, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ "e", &static_QUType_ptr, "QMouseEvent", QUParameter::In }
    };
    static const QUMethod slot_6 = {"mouseDoubleClickEvent", 1, param_slot_6 };
    static const QMetaData slot_tbl[] = {
	{ "paintEvent(QPaintEvent*)", &slot_0, QMetaData::Protected },
	{ "resizeEvent(QResizeEvent*)", &slot_1, QMetaData::Protected },
	{ "keyPressEvent(QKeyEvent*)", &slot_2, QMetaData::Protected },
	{ "mousePressEvent(QMouseEvent*)", &slot_3, QMetaData::Protected },
	{ "mouseMoveEvent(QMouseEvent*)", &slot_4, QMetaData::Protected },
	{ "mouseReleaseEvent(QMouseEvent*)", &slot_5, QMetaData::Protected },
	{ "mouseDoubleClickEvent(QMouseEvent*)", &slot_6, QMetaData::Protected }
    };
    static const QUMethod signal_0 = {"selectionChanged", 0, 0 };
    static const QMetaData signal_tbl[] = {
	{ "selectionChanged()", &signal_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"QMapWidget", parentObject,
	slot_tbl, 7,
	signal_tbl, 1,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QMapWidget.setMetaObject( metaObj );
    return metaObj;
}

void* QMapWidget::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QMapWidget" ) )
	return this;
    return QWidget::qt_cast( clname );
}

// SIGNAL selectionChanged
void QMapWidget::selectionChanged()
{
    activate_signal( staticMetaObject()->signalOffset() + 0 );
}

bool QMapWidget::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: paintEvent((QPaintEvent*)static_QUType_ptr.get(_o+1)); break;
    case 1: resizeEvent((QResizeEvent*)static_QUType_ptr.get(_o+1)); break;
    case 2: keyPressEvent((QKeyEvent*)static_QUType_ptr.get(_o+1)); break;
    case 3: mousePressEvent((QMouseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 4: mouseMoveEvent((QMouseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 5: mouseReleaseEvent((QMouseEvent*)static_QUType_ptr.get(_o+1)); break;
    case 6: mouseDoubleClickEvent((QMouseEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QWidget::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool QMapWidget::qt_emit( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->signalOffset() ) {
    case 0: selectionChanged(); break;
    default:
	return QWidget::qt_emit(_id,_o);
    }
    return TRUE;
}
#ifndef QT_NO_PROPERTIES

bool QMapWidget::qt_property( int id, int f, QVariant* v)
{
    return QWidget::qt_property( id, f, v);
}

bool QMapWidget::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
