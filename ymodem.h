/*	
 * Copyright 2015 Omid Sakhi (www.omidsakhi.com)
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of California, Berkeley nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _YMODEM_H
#define _YMODEM_H

#include <QObject>
#include <QSerialPort>
#include <QBuffer>
#include <QFileInfo>

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

#define DLY_1S 1000
#define MAXRETRANS 25

class YMODEM : public QObject
{
    Q_OBJECT
private:
	bool m_cancel;
	QSerialPort *m_serialPort;	
	int yTransmitRead(unsigned short timeout);	
	void yTransmitWrite(const QByteArray &ba );
	void yTransmitWrite( int c );
	void yTransmitFlush();
	unsigned short crc16_ccitt(const unsigned char *buf, int len);			
	int yTransmitPacket(const QFileInfo &fileInfo, QIODevice &dev, const quint32 &payloadSize, quint8 sequenceNum = 0,bool firstPacket = true);
public:
	enum YMODEMResponce {
		MODEM_SUCCESS = 0,
		MODEM_PACKET_FAILED = -1,
		MODEM_NO_SYNC = -2,
		MODEM_REMOTE_CANCEL = -3,
		MODEM_EOT_ERROR = -4,
		MODEM_LOCAL_CANCEL = -5,
		MODEM_SIZE_ERROR = -6,
		MODEM_RESTART = -7,
		MODEM_FILE_NOT_FOUND_ERROR = -8,
		MODEM_FILE_NOT_OPEN_ERROR = -9
	};
    explicit YMODEM(QSerialPort *sp, QObject *parent = 0);
	int yTransmit(QString filePath);
	int yTransmitClose();
	bool isConnected();
	void cancel();
signals:
	void packet(int,int);
	void progress(int);
	void message(QString);
};

#endif // _YMODEM_H
