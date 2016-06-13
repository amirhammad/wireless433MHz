#include <QCoreApplication>
#include "Communication.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	new Communication();

	return a.exec();
}
