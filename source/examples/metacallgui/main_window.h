
#pragma once


#include <QMainWindow>
#include <QScopedPointer>


namespace Ui {
    class MainWindow;
}


/**
*  @brief
*    Main window of the metacallgui example
*/
class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    /**
    *  @brief
    *    Constructor
    */
    MainWindow();

    /**
    *  @brief
    *    Destructor
    */
    virtual ~MainWindow();


protected slots:
    void on_editNumber_valueChanged(int value);
    void on_about();


protected:
    const QScopedPointer<Ui::MainWindow> m_ui;
};
