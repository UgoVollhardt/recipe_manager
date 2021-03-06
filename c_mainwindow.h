#ifndef C_MAINWINDOW_H
#define C_MAINWINDOW_H

#include <QMainWindow>
#include "utils/utils.h"
#include "utils/c_dbmanager.h"
#include "ui/c_stepview.h"

QT_BEGIN_NAMESPACE
namespace Ui { class c_mainWindow; }
QT_END_NAMESPACE

class c_mainWindow : public QMainWindow
{
    Q_OBJECT

public:
    c_mainWindow(QWidget *parent = nullptr);
    ~c_mainWindow();

    void test();

private:
    Ui::c_mainWindow *ui;
    c_dbManager* dbmanager;
    c_ioManager* iomanager;

    c_stepView *stepView;
};
#endif // C_MAINWINDOW_H
