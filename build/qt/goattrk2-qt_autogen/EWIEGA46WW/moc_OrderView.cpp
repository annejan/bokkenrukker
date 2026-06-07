/****************************************************************************
** Meta object code from reading C++ file 'OrderView.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../qt/OrderView.h"
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'OrderView.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_OrderListModel_t {
    uint offsetsAndSizes[2];
    char stringdata0[15];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_OrderListModel_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_OrderListModel_t qt_meta_stringdata_OrderListModel = {
    {
        QT_MOC_LITERAL(0, 14)   // "OrderListModel"
    },
    "OrderListModel"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_OrderListModel[] = {

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

Q_CONSTINIT const QMetaObject OrderListModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractTableModel::staticMetaObject>(),
    qt_meta_stringdata_OrderListModel.offsetsAndSizes,
    qt_meta_data_OrderListModel,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_OrderListModel_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<OrderListModel, std::true_type>
    >,
    nullptr
} };

void OrderListModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *OrderListModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OrderListModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_OrderListModel.stringdata0))
        return static_cast<void*>(this);
    return QAbstractTableModel::qt_metacast(_clname);
}

int OrderListModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    return _id;
}
namespace {
struct qt_meta_stringdata_OrderView_t {
    uint offsetsAndSizes[32];
    char stringdata0[10];
    char stringdata1[7];
    char stringdata2[1];
    char stringdata3[17];
    char stringdata4[2];
    char stringdata5[19];
    char stringdata6[10];
    char stringdata7[10];
    char stringdata8[18];
    char stringdata9[20];
    char stringdata10[13];
    char stringdata11[10];
    char stringdata12[12];
    char stringdata13[7];
    char stringdata14[7];
    char stringdata15[7];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_OrderView_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_OrderView_t qt_meta_stringdata_OrderView = {
    {
        QT_MOC_LITERAL(0, 9),  // "OrderView"
        QT_MOC_LITERAL(10, 6),  // "edited"
        QT_MOC_LITERAL(17, 0),  // ""
        QT_MOC_LITERAL(18, 16),  // "onSubtuneChanged"
        QT_MOC_LITERAL(35, 1),  // "v"
        QT_MOC_LITERAL(37, 18),  // "onSelectionChanged"
        QT_MOC_LITERAL(56, 9),  // "insertRow"
        QT_MOC_LITERAL(66, 9),  // "deleteRow"
        QT_MOC_LITERAL(76, 17),  // "insertTransposeUp"
        QT_MOC_LITERAL(94, 19),  // "insertTransposeDown"
        QT_MOC_LITERAL(114, 12),  // "insertRepeat"
        QT_MOC_LITERAL(127, 9),  // "insertRst"
        QT_MOC_LITERAL(137, 11),  // "gotoPattern"
        QT_MOC_LITERAL(149, 6),  // "swap12"
        QT_MOC_LITERAL(156, 6),  // "swap13"
        QT_MOC_LITERAL(163, 6)   // "swap23"
    },
    "OrderView",
    "edited",
    "",
    "onSubtuneChanged",
    "v",
    "onSelectionChanged",
    "insertRow",
    "deleteRow",
    "insertTransposeUp",
    "insertTransposeDown",
    "insertRepeat",
    "insertRst",
    "gotoPattern",
    "swap12",
    "swap13",
    "swap23"
};
#undef QT_MOC_LITERAL
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_OrderView[] = {

 // content:
      10,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   92,    2, 0x06,    1 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       3,    1,   93,    2, 0x08,    2 /* Private */,
       5,    0,   96,    2, 0x08,    4 /* Private */,
       6,    0,   97,    2, 0x08,    5 /* Private */,
       7,    0,   98,    2, 0x08,    6 /* Private */,
       8,    0,   99,    2, 0x08,    7 /* Private */,
       9,    0,  100,    2, 0x08,    8 /* Private */,
      10,    0,  101,    2, 0x08,    9 /* Private */,
      11,    0,  102,    2, 0x08,   10 /* Private */,
      12,    0,  103,    2, 0x08,   11 /* Private */,
      13,    0,  104,    2, 0x08,   12 /* Private */,
      14,    0,  105,    2, 0x08,   13 /* Private */,
      15,    0,  106,    2, 0x08,   14 /* Private */,

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
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject OrderView::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_OrderView.offsetsAndSizes,
    qt_meta_data_OrderView,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_OrderView_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<OrderView, std::true_type>,
        // method 'edited'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onSubtuneChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<int, std::false_type>,
        // method 'onSelectionChanged'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'insertRow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'deleteRow'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'insertTransposeUp'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'insertTransposeDown'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'insertRepeat'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'insertRst'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'gotoPattern'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'swap12'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'swap13'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'swap23'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void OrderView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<OrderView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->edited(); break;
        case 1: _t->onSubtuneChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->onSelectionChanged(); break;
        case 3: _t->insertRow(); break;
        case 4: _t->deleteRow(); break;
        case 5: _t->insertTransposeUp(); break;
        case 6: _t->insertTransposeDown(); break;
        case 7: _t->insertRepeat(); break;
        case 8: _t->insertRst(); break;
        case 9: _t->gotoPattern(); break;
        case 10: _t->swap12(); break;
        case 11: _t->swap13(); break;
        case 12: _t->swap23(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (OrderView::*)();
            if (_t _q_method = &OrderView::edited; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
    }
}

const QMetaObject *OrderView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *OrderView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_OrderView.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int OrderView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void OrderView::edited()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
