
#include "main_window.hpp"

#include <QMessageBox>

#include <metacall/metacall_version.h>

#include "ui_main_window.h"

main_window::main_window() :
	impl(new Ui::main_window)
{
	impl->setupUi(this);
}

main_window::~main_window()
{
}

void main_window::on_editNumber_valueChanged(int value)
{
	impl->result->setText(QString::number(value));
}

void main_window::on_about()
{
	QString about = QString() + "Version: " + METACALL_VERSION + "\n";

	QMessageBox::about(this, "template project", about);
}
