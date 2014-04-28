#include <QApplication>

#include "RangeUI.hpp"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	RangeUI ui;
	ui.show();

	return a.exec();
}
