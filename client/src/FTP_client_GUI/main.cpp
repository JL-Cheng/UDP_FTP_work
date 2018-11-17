#include "FTP_client_GUI.h"
#include <QtWidgets/QApplication>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	FTP_client_GUI w;
	w.show();
	return a.exec();

}
