#ifndef LUA_QTABWIDGET_H
#define LUA_QTABWIDGET_H
#include "lua_qt_wrapper.hpp"

typedef class_<QTabWidget, QWidget> LQTabWidget;
typedef class_<QToolBox, QFrame> LQToolBox;

LQTabWidget lqtabwidget();
LQToolBox lqtoolbox();

#endif // LUA_QTABWIDGET_H
