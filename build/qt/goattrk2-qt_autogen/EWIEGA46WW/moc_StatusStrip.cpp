/****************************************************************************
** Meta object code from reading C++ file 'StatusStrip.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qt/StatusStrip.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'StatusStrip.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
namespace {
struct qt_meta_stringdata_StatusStrip_t {
    uint offsetsAndSizes[18];
    char stringdata0[12];
    char stringdata1[11];
    char stringdata2[1];
    char stringdata3[12];
    char stringdata4[14];
    char stringdata5[13];
    char stringdata6[12];
    char stringdata7[14];
    char stringdata8[12];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_StatusStrip_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_StatusStrip_t qt_meta_stringdata_StatusStrip = {
    {
        QT_MOC_LITERAL(0, 11),  // "StatusStrip"
        QT_MOC_LITERAL(12, 10),  // "sidClicked"
        QT_MOC_LITERAL(23, 0),  // ""
        QT_MOC_LITERAL(24, 11),  // "sid2Clicked"
        QT_MOC_LITERAL(36, 13),  // "followClicked"
        QT_MOC_LITERAL(50, 12),  // "tempoClicked"
        QT_MOC_LITERAL(63, 11),  // "ntscClicked"
        QT_MOC_LITERAL(75, 13),  // "octaveClicked"
        QT_MOC_LITERAL(89, 11)   // "octaveDelta"
    },
    "StatusStrip",
    "sidClicked",
    "",
    "sid2Clicked",
    "followClicked",
    "tempoClicked",
    "ntscClicked",
    "octaveClicked",
    "octaveDelta"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_StatusStrip[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       7,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   56,    2, 0x06,    1 /* Public */,
       3,    0,   57,    2, 0x06,    2 /* Public */,
       4,    0,   58,    2, 0x06,    3 /* Public */,
       5,    0,   59,    2, 0x06,    4 /* Public */,
       6,    0,   60,    2, 0x06,    5 /* Public */,
       7,    0,   61,    2, 0x06,    6 /* Public */,
       8,    1,   62,    2, 0x06,    7 /* Public */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    2,

       0        // eod
};

Q_CONSTINIT const QMetaObject StatusStrip::staticMetaObject = { {
    QMetaObject::SuperData::link<QFrame::staticMetaObject>(),
    qt_meta_stringdata_StatusStrip.offsetsAndSizes,
    qt_meta_data_StatusStrip,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_StatusStrip_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<StatusStrip, std::true_type>,
        // method 'sidClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sid2Clicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'followClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'tempoClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'ntscClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'octaveClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'octaveDelta'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>
    >,
    nullptr
} };

void StatusStrip::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<StatusStrip *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->sidClicked(); break;
        case 1: _t->sid2Clicked(); break;
        case 2: _t->followClicked(); break;
        case 3: _t->tempoClicked(); break;
        case 4: _t->ntscClicked(); break;
        case 5: _t->octaveClicked(); break;
        case 6: _t->octaveDelta((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (StatusStrip::*)();
            if (_t _q_method = &StatusStrip::sidClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (StatusStrip::*)();
            if (_t _q_method = &StatusStrip::sid2Clicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (StatusStrip::*)();
            if (_t _q_method = &StatusStrip::followClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (StatusStrip::*)();
            if (_t _q_method = &StatusStrip::tempoClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (StatusStrip::*)();
            if (_t _q_method = &StatusStrip::ntscClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (StatusStrip::*)();
            if (_t _q_method = &StatusStrip::octaveClicked; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (StatusStrip::*)(int );
            if (_t _q_method = &StatusStrip::octaveDelta; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 6;
                return;
            }
        }
    }
}

const QMetaObject *StatusStrip::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StatusStrip::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_StatusStrip.stringdata0))
        return static_cast<void*>(this);
    return QFrame::qt_metacast(_clname);
}

int StatusStrip::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void StatusStrip::sidClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void StatusStrip::sid2Clicked()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void StatusStrip::followClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void StatusStrip::tempoClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void StatusStrip::ntscClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void StatusStrip::octaveClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void StatusStrip::octaveDelta(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 6, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
