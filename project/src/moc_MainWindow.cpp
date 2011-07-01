/****************************************************************************
** MainWindow meta object code from reading C++ file 'MainWindow.h'
**
** Created: Thu May 19 14:46:52 2011
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "MainWindow.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8b. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *MainWindow::className() const
{
    return "MainWindow";
}

QMetaObject *MainWindow::metaObj = 0;
static QMetaObjectCleanUp cleanUp_MainWindow( "MainWindow", &MainWindow::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString MainWindow::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MainWindow", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString MainWindow::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "MainWindow", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* MainWindow::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QMainWindow::staticMetaObject();
    static const QUMethod slot_0 = {"OnFileNew", 0, 0 };
    static const QUMethod slot_1 = {"OnFileEdit", 0, 0 };
    static const QUMethod slot_2 = {"OnFileOpen", 0, 0 };
    static const QUMethod slot_3 = {"OnFileSave", 0, 0 };
    static const QUMethod slot_4 = {"OnFileConfig", 0, 0 };
    static const QUMethod slot_5 = {"OnFileExit", 0, 0 };
    static const QUMethod slot_6 = {"OnSimRun", 0, 0 };
    static const QUMethod slot_7 = {"OnSimPause", 0, 0 };
    static const QUMethod slot_8 = {"OnSimSkip", 0, 0 };
    static const QUMethod slot_9 = {"OnSimStop", 0, 0 };
    static const QUMethod slot_10 = {"OnNetInit", 0, 0 };
    static const QUMethod slot_11 = {"OnNetClose", 0, 0 };
    static const QUMethod slot_12 = {"OnNetServer", 0, 0 };
    static const QUParameter param_slot_13[] = {
	{ "id", &static_QUType_int, 0, QUParameter::In }
    };
    static const QUMethod slot_13 = {"OnNetServer", 1, param_slot_13 };
    static const QMetaData slot_tbl[] = {
	{ "OnFileNew()", &slot_0, QMetaData::Protected },
	{ "OnFileEdit()", &slot_1, QMetaData::Protected },
	{ "OnFileOpen()", &slot_2, QMetaData::Protected },
	{ "OnFileSave()", &slot_3, QMetaData::Protected },
	{ "OnFileConfig()", &slot_4, QMetaData::Protected },
	{ "OnFileExit()", &slot_5, QMetaData::Protected },
	{ "OnSimRun()", &slot_6, QMetaData::Protected },
	{ "OnSimPause()", &slot_7, QMetaData::Protected },
	{ "OnSimSkip()", &slot_8, QMetaData::Protected },
	{ "OnSimStop()", &slot_9, QMetaData::Protected },
	{ "OnNetInit()", &slot_10, QMetaData::Protected },
	{ "OnNetClose()", &slot_11, QMetaData::Protected },
	{ "OnNetServer()", &slot_12, QMetaData::Protected },
	{ "OnNetServer(int)", &slot_13, QMetaData::Protected }
    };
    metaObj = QMetaObject::new_metaobject(
	"MainWindow", parentObject,
	slot_tbl, 14,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_MainWindow.setMetaObject( metaObj );
    return metaObj;
}

void* MainWindow::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "MainWindow" ) )
	return this;
    return QMainWindow::qt_cast( clname );
}

bool MainWindow::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: OnFileNew(); break;
    case 1: OnFileEdit(); break;
    case 2: OnFileOpen(); break;
    case 3: OnFileSave(); break;
    case 4: OnFileConfig(); break;
    case 5: OnFileExit(); break;
    case 6: OnSimRun(); break;
    case 7: OnSimPause(); break;
    case 8: OnSimSkip(); break;
    case 9: OnSimStop(); break;
    case 10: OnNetInit(); break;
    case 11: OnNetClose(); break;
    case 12: OnNetServer(); break;
    case 13: OnNetServer((int)static_QUType_int.get(_o+1)); break;
    default:
	return QMainWindow::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool MainWindow::qt_emit( int _id, QUObject* _o )
{
    return QMainWindow::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool MainWindow::qt_property( int id, int f, QVariant* v)
{
    return QMainWindow::qt_property( id, f, v);
}

bool MainWindow::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
