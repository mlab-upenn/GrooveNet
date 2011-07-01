/****************************************************************************
** QDraggingTable meta object code from reading C++ file 'QTableVisualizer.h'
**
** Created: Thu May 19 14:46:53 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "QTableVisualizer.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *QDraggingTable::className() const
{
    return "QDraggingTable";
}

QMetaObject *QDraggingTable::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QDraggingTable( "QDraggingTable", &QDraggingTable::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QDraggingTable::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QDraggingTable", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QDraggingTable::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QDraggingTable", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QDraggingTable::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QTable::staticMetaObject();
    metaObj = QMetaObject::new_metaobject(
	"QDraggingTable", parentObject,
	0, 0,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QDraggingTable.setMetaObject( metaObj );
    return metaObj;
}

void* QDraggingTable::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QDraggingTable" ) )
	return this;
    return QTable::qt_cast( clname );
}

bool QDraggingTable::qt_invoke( int _id, QUObject* _o )
{
    return QTable::qt_invoke(_id,_o);
}

bool QDraggingTable::qt_emit( int _id, QUObject* _o )
{
    return QTable::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QDraggingTable::qt_property( int id, int f, QVariant* v)
{
    return QTable::qt_property( id, f, v);
}

bool QDraggingTable::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES


const char *QTableVisualizer::className() const
{
    return "QTableVisualizer";
}

QMetaObject *QTableVisualizer::metaObj = 0;
static QMetaObjectCleanUp cleanUp_QTableVisualizer( "QTableVisualizer", &QTableVisualizer::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString QTableVisualizer::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QTableVisualizer", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString QTableVisualizer::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "QTableVisualizer", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* QTableVisualizer::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QVisualizer::staticMetaObject();
    static const QUParameter param_slot_0[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_0 = {"slotCurrentChanged", 2, param_slot_0 };
    static const QUParameter param_slot_1[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In },
	{ "button", &static_QUType_int, 0, QUParameter::In },
	{ "mousePos", &static_QUType_varptr, "\x0e", QUParameter::In }
    };
    static const QUMethod slot_1 = {"slotClicked", 4, param_slot_1 };
    static const QUParameter param_slot_2[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In },
	{ "button", &static_QUType_int, 0, QUParameter::In },
	{ "mousePos", &static_QUType_varptr, "\x0e", QUParameter::In }
    };
    static const QUMethod slot_2 = {"slotDoubleClicked", 4, param_slot_2 };
    static const QUParameter param_slot_3[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In },
	{ "button", &static_QUType_int, 0, QUParameter::In },
	{ "mousePos", &static_QUType_varptr, "\x0e", QUParameter::In }
    };
    static const QUMethod slot_3 = {"slotPressed", 4, param_slot_3 };
    static const QUMethod slot_4 = {"slotSelectionChanged", 0, 0 };
    static const QUParameter param_slot_5[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_5 = {"slotValueChanged", 2, param_slot_5 };
    static const QUParameter param_slot_6[] = {
	{ "row", &static_QUType_int, 0, QUParameter::In },
	{ "col", &static_QUType_int, 0, QUParameter::In },
	{ "pos", &static_QUType_varptr, "\x0e", QUParameter::In }
    };
    static const QUMethod slot_6 = {"slotContextMenuRequested", 3, param_slot_6 };
    static const QUParameter param_slot_7[] = {
	{ "e", &static_QUType_ptr, "QDropEvent", QUParameter::In }
    };
    static const QUMethod slot_7 = {"slotDropped", 1, param_slot_7 };
    static const QMetaData slot_tbl[] = {
	{ "slotCurrentChanged(int,int)", &slot_0, QMetaData::Protected },
	{ "slotClicked(int,int,int,const QPoint&)", &slot_1, QMetaData::Protected },
	{ "slotDoubleClicked(int,int,int,const QPoint&)", &slot_2, QMetaData::Protected },
	{ "slotPressed(int,int,int,const QPoint&)", &slot_3, QMetaData::Protected },
	{ "slotSelectionChanged()", &slot_4, QMetaData::Protected },
	{ "slotValueChanged(int,int)", &slot_5, QMetaData::Protected },
	{ "slotContextMenuRequested(int,int,const QPoint&)", &slot_6, QMetaData::Protected },
	{ "slotDropped(QDropEvent*)", &slot_7, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"QTableVisualizer", parentObject,
	slot_tbl, 8,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_QTableVisualizer.setMetaObject( metaObj );
    return metaObj;
}

void* QTableVisualizer::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "QTableVisualizer" ) )
	return this;
    return QVisualizer::qt_cast( clname );
}

bool QTableVisualizer::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: slotCurrentChanged((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 1: slotClicked((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(const QPoint&)*((const QPoint*)static_QUType_ptr.get(_o+4))); break;
    case 2: slotDoubleClicked((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(const QPoint&)*((const QPoint*)static_QUType_ptr.get(_o+4))); break;
    case 3: slotPressed((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(int)static_QUType_int.get(_o+3),(const QPoint&)*((const QPoint*)static_QUType_ptr.get(_o+4))); break;
    case 4: slotSelectionChanged(); break;
    case 5: slotValueChanged((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2)); break;
    case 6: slotContextMenuRequested((int)static_QUType_int.get(_o+1),(int)static_QUType_int.get(_o+2),(const QPoint&)*((const QPoint*)static_QUType_ptr.get(_o+3))); break;
    case 7: slotDropped((QDropEvent*)static_QUType_ptr.get(_o+1)); break;
    default:
	return QVisualizer::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool QTableVisualizer::qt_emit( int _id, QUObject* _o )
{
    return QVisualizer::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool QTableVisualizer::qt_property( int id, int f, QVariant* v)
{
    return QVisualizer::qt_property( id, f, v);
}

bool QTableVisualizer::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
