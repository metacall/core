#ifndef METACALL_GUI_MAIN_WINDOW_HPP
#define METACALL_GUI_MAIN_WINDOW_HPP 1

#include <QMainWindow>
#include <QScopedPointer>

namespace Ui
{
class main_window;
}

/**
*  @brief
*    Main window of the metacallgui example
*/
class main_window : public QMainWindow
{
	Q_OBJECT

public:
	/**
    *  @brief
    *    Constructor
    */
	main_window();

	/**
    *  @brief
    *    Destructor
    */
	virtual ~main_window();

protected slots:
	void on_editNumber_valueChanged(int value);
	void on_about();

protected:
	const QScopedPointer<Ui::main_window> impl;
};

#endif // METACALL_GUI_MAIN_WINDOW_HPP
