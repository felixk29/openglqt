#pragma once

#include <QMainWindow> 
class Widget; 

class Window: public QMainWindow{
    Q_OBJECT

public: 
    explicit Window(QWidget *parent =nullptr);

private:
    Widget *m_viewport=nullptr;

};