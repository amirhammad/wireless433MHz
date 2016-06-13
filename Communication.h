#pragma once
#include <QObject>

class Communication : public QObject {
	Q_OBJECT

	int m_fd;
	QByteArray m_buffer;
	int m_index;
	int m_sendIndex;

	void sendSync();
	void parse();

public:
	Communication(QObject *parent = 0);

private slots:
	void on_socketNotifier_activated(int fd);
	void on_timer_timeout();
};
