#include "Communication.h"

/* github.com/bang-olufsen/yahdlc/ @ a0fa93b  (v1.0.0) */
#include <yahdlc/yahdlc.h>

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <QSocketNotifier>
#include <QDebug>
#include <QTimer>
#include <iostream>
#include <termios.h>

using namespace std;

void Communication::sendSync()
{
	static char sync[50] = {0};
	::write(m_fd, sync, sizeof(sync));
}

void Communication::parse()
{
	yahdlc_get_data_reset();
	yahdlc_control_t control;
	QByteArray destArray;
	destArray.resize(m_buffer.size());
	unsigned int destLen = 0;
	int yret = yahdlc_get_data(&control, m_buffer.data(), m_buffer.size(), destArray.data(), &destLen);
	if (yret < 0) {
		switch (yret) {
		case -EIO:
			m_buffer.remove(0, destLen);
			break;
		case -EINVAL:
			cerr << "invalid parameter" << endl;
			Q_ASSERT(0);
			break;

		case -ENOMSG:
//			cerr << "-";
			break;
		}
		return;
	}
	Q_ASSERT(yret >= 0);
	cout << "DATA(" << m_index++ << "):" << destArray.data() << endl;
	m_buffer.remove(0, yret);
}

Communication::Communication(QObject *parent)
	: QObject(parent)
	, m_fd(-1)
	, m_index(0)
	, m_sendIndex(0)
{
	int fd = ::open("/dev/serial/by-id/usb-FTDI_FT232R_USB_UART_AH012X1L-if00-port0", O_RDWR);
	if (fd < 0) {
		cerr << "Error opening serial" << endl;
		return;
	}
	cerr << "1M" << endl;
	struct termios t;
	cfmakeraw(&t);
	cfsetspeed(&t, B1000000);
	t.c_cflag = CS8 | CREAD | CLOCAL;
	tcsetattr(fd,TCSANOW, &t);
	tcdrain(fd);

	m_fd = fd;
	QSocketNotifier *sockNotif = new QSocketNotifier(fd, QSocketNotifier::Read, this);
	connect(sockNotif, SIGNAL(activated(int)), this, SLOT(on_socketNotifier_activated(int)));
	sockNotif->setEnabled(true);

	QTimer *timer = new QTimer(this);
	timer->setInterval(200);
	timer->setSingleShot(false);
	connect(timer, SIGNAL(timeout()), this, SLOT(on_timer_timeout()));
	timer->start();
}

void Communication::on_socketNotifier_activated(int fd)
{
	char buffer[256];
	int c = ::read(fd, buffer, sizeof(buffer));
	if (c > 0) {
#ifdef DEBUG_DATA
		cout << "DATA" << endl;
		for (int i = 0; i < c; i++) {
			cout << hex << buffer[i] << "  ";
		}
		cout << endl;
#endif
		m_buffer.append(buffer, c);
		parse();
	}
}

void Communication::on_timer_timeout()
{
	QByteArray arr = "ahoj svet";
	QByteArray yahdlcEncoded;
	yahdlcEncoded.resize(arr.size()*2 + 32);
	yahdlc_control_t control;
	control.frame = YAHDLC_FRAME_DATA;
	unsigned int destSize = 0;
	int ret = yahdlc_frame_data(&control, arr.data(), arr.size(), yahdlcEncoded.data(), &destSize);
	Q_ASSERT(ret == 0);

	// Send the synchronization before actual data. It will help communication.
	sendSync();

	// Send YAHDLC encoded data with BROADCAST SET
	int written = ::write(m_fd, yahdlcEncoded.data(), yahdlcEncoded.size());
	cout << "SENDINDEX=" << m_sendIndex++ << "   ";
	Q_ASSERT(written == yahdlcEncoded.size());
}
