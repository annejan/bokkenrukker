/****************************************************************************
** Meta object code from reading C++ file 'TablesView.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qt/TablesView.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'TablesView.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_SidTableModel_t {
    uint offsetsAndSizes[2];
    char stringdata0[14];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_SidTableModel_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_SidTableModel_t qt_meta_stringdata_SidTableModel = {
    {
        QT_MOC_LITERAL(0, 13)   // "SidTableModel"
    },
    "SidTableModel"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_SidTableModel[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject SidTableModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractTableModel::staticMetaObject>(),
    qt_meta_stringdata_SidTableModel.offsetsAndSizes,
    qt_meta_data_SidTableModel,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_SidTableModel_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SidTableModel, std::true_type>
    >,
    nullptr
} };

void SidTableModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *SidTableModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SidTableModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SidTableModel.stringdata0))
        return static_cast<void*>(this);
    return QAbstractTableModel::qt_metacast(_clname);
}

int SidTableModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_stringdata_TablesView_t {
    uint offsetsAndSizes[26];
    char stringdata0[11];
    char stringdata1[7];
    char stringdata2[1];
    char stringdata3[13];
    char stringdata4[4];
    char stringdata5[23];
    char stringdata6[10];
    char stringdata7[10];
    char stringdata8[7];
    char stringdata9[9];
    char stringdata10[12];
    char stringdata11[10];
    char stringdata12[11];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_TablesView_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_TablesView_t qt_meta_stringdata_TablesView = {
    {
        QT_MOC_LITERAL(0, 10),  // "TablesView"
        QT_MOC_LITERAL(11, 6),  // "edited"
        QT_MOC_LITERAL(18, 0),  // ""
        QT_MOC_LITERAL(19, 12),  // "onTabChanged"
        QT_MOC_LITERAL(32, 3),  // "idx"
        QT_MOC_LITERAL(36, 22),  // "onCellSelectionChanged"
        QT_MOC_LITERAL(59, 9),  // "insertRow"
        QT_MOC_LITERAL(69, 9),  // "deleteRow"
        QT_MOC_LITERAL(79, 6),  // "negate"
        QT_MOC_LITERAL(86, 8),  // "optimize"
        QT_MOC_LITERAL(95, 11),  // "limitToTime"
        QT_MOC_LITERAL(107, 9),  // "clearCell"
        QT_MOC_LITERAL(117, 10)   // "insertJump"
    },
    "TablesView",
    "edited",
    "",
    "onTabChanged",
    "idx",
    "onCellSelectionChanged",
    "insertRow",
    "deleteRow",
    "negate",
    "optimize",
    "limitToTime",
    "clearCell",
    "insertJump"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_TablesView[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   74,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       3,    1,   75,    2, 0x08,    2 /* Private */,
       5,    0,   78,    2, 0x08,    4 /* Private */,
       6,    0,   79,    2, 0x08,    5 /* Private */,
       7,    0,   80,    2, 0x08,    6 /* Private */,
       8,    0,   81,    2, 0x08,    7 /* Private */,
       9,    0,   82,    2, 0x08,    8 /* Private */,
      10,    0,   83,    2, 0x08,    9 /* Private */,
      11,    0,   84,    2, 0x08,   10 /* Private */,
      12,    0,   85,    2, 0x08,   11 /* Private */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    4,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject TablesView::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_TablesView.offsetsAndSizes,
    qt_meta_data_TablesView,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_TablesView_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<TablesView, std::true_type>,
        // method 'edited'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onTabChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onCellSelectionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'insertRow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'deleteRow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'negate'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'optimize'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'limitToTime'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'clearCell'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'insertJump'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void TablesView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<TablesView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->edited(); break;
        case 1: _t->onTabChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->onCellSelectionChanged(); break;
        case 3: _t->insertRow(); break;
        case 4: _t->deleteRow(); break;
        case 5: _t->negate(); break;
        case 6: _t->optimize(); break;
        case 7: _t->limitToTime(); break;
        case 8: _t->clearCell(); break;
        case 9: _t->insertJump(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (TablesView::*)();
            if (_t _q_method = &TablesView::edited; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *TablesView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *TablesView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_TablesView.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int TablesView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void TablesView::edited()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
