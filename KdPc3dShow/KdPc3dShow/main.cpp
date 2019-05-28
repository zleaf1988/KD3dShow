#include "KdPc3dShow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	KdPc3dShow w;
	w.show();
	return a.exec();
}
