#include "handymouse.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	HandyMouse w;
	w.show();
	return a.exec();
}
