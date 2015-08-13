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

#include "ymodem.h"

#include <QDebug>
#include <QSerialPort>
#include <QFile>
#include <QFileInfo>
#include <QBuffer>
#include <QEventLoop>
#include <QTimer>
#include <QDateTime>

YMODEM::YMODEM( QSerialPort *sp, QObject *parent /*= 0*/ ) : QObject(parent)
{
	m_serialPort = sp;
	m_cancel = false;
}

static const unsigned short crc16tab[256]= {
	/* Copyright 2001-2010 Georges Menie (www.menie.org) */
	0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
	0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
	0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
	0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
	0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
	0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
	0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
	0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
	0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
	0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
	0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
	0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
	0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
	0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
	0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
	0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
	0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
	0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
	0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
	0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
	0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
	0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
	0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
	0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
	0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
	0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
	0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
	0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
	0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
	0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
	0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
	0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

unsigned short YMODEM::crc16_ccitt( const unsigned char *buf, int len )
{
	/* Copyright 2001-2010 Georges Menie (www.menie.org) */
	register int counter;
	register unsigned short crc = 0;
	for( counter = 0; counter < len; counter++)
		crc = (crc<<8) ^ crc16tab[((crc>>8) ^ *(char *)buf++)&0x00FF];
	return crc;
}

int YMODEM::yTransmitRead( unsigned short timeout )
{
	m_serialPort->waitForReadyRead(timeout);
	return m_serialPort->read(1)[0];	
}

void YMODEM::yTransmitWrite(const QByteArray &ba )
{	
	m_serialPort->write(ba);	
	m_serialPort->waitForBytesWritten(1000);
}

void YMODEM::yTransmitWrite( int c )
{
	QByteArray ba;
	ba.append(QChar(c));
	m_serialPort->write(ba);	
	m_serialPort->waitForBytesWritten(1000);
}

void YMODEM::yTransmitFlush()
{
	m_serialPort->readAll();
}

bool YMODEM::isConnected()
{
	return m_serialPort->isOpen();
}

void YMODEM::cancel()
{
	m_cancel = true;
}

int YMODEM::yTransmit( QString filePath )
{
	QFile file(filePath);
	if (!file.exists())
		return MODEM_FILE_NOT_FOUND_ERROR;
	QFileInfo fileInfo(filePath);	
	if (!file.open(QIODevice::ReadOnly))
		return MODEM_FILE_NOT_OPEN_ERROR;
	quint32 payloadSize = 1024; // 1K
	int response;
	for(int retry = 0; retry < 10 && !m_cancel; ++retry)
	{
		int c;
		if ((c = yTransmitRead((DLY_1S)<<1)) >= 0) {			
			switch (c) {
			case '\0':
			case 'c':
				qDebug() << "c";
			case 'C':
				qDebug() << "C";
			case NAK:
				qDebug() << "NAK";
				response = yTransmitPacket(fileInfo,file,payloadSize);
				if (response == MODEM_RESTART)
				{
					file.seek(0);
					retry = 0;
					continue;
				}
				return response;
				break;
			case CAN:
				if ((c = yTransmitRead(DLY_1S)) == CAN) {
					yTransmitWrite(ACK);
					yTransmitFlush();						
					qDebug() << "XMODEM::yTransmit - canceled by remote";
					return MODEM_REMOTE_CANCEL;
				}
				break;
			default:		
				qDebug() << "XYMODEM::yTransmit - Unknown : " << c;
				break;
			}
		}
	}
	yTransmitWrite(CAN);
	yTransmitWrite(CAN);
	yTransmitWrite(CAN);
	yTransmitFlush();
	file.close();
	if (m_cancel)
	{
		qDebug() << "XMODEM::yTransmit - canceled by user";
		return MODEM_LOCAL_CANCEL;
	}
	else
	{
		qDebug() << "XMODEM::yTransmit - no sync";
		return MODEM_NO_SYNC;	
	}
}

int YMODEM::yTransmitPacket(const QFileInfo &fileInfo, QIODevice &dev, const quint32 &payloadSize, quint8 sequenceNum, bool firstPacket)
{		
	QByteArray payload;
	int res;
	if (dev.atEnd())
	{
		for (int retry = 0; retry < 10; ++retry) {
			yTransmitWrite(EOT);
		if ((res = yTransmitRead((DLY_1S)<<1)) == ACK) break;
		}
		return (res == ACK)?MODEM_SUCCESS:MODEM_EOT_ERROR;
	}
	if (firstPacket)
	{		
		payload += fileInfo.fileName().toUtf8();
		payload += (quint32) fileInfo.size();
		payload += (quint32) fileInfo.created().toTime_t();		
	}
	else
	{
		payload = dev.read(payloadSize);		
	}
	payload += QString(CTRLZ).repeated(payloadSize - payload.size()).toUtf8();
	QByteArray packet;
	packet.append(QString(STX).toUtf8());
	packet.append(sequenceNum);
	packet.append(~sequenceNum);
	packet.append(payload);
	unsigned short ccrc = crc16_ccitt((unsigned char*)payload.constData(),payloadSize);
	packet.append((unsigned short)(ccrc>>8) & 0xFF);
	packet.append((unsigned short)ccrc & 0xFF);
	int pos = dev.pos();
	qDebug() << "XMODEM:xBeginTransmit - Packet # : " << pos/payloadSize << "/" << (int)floor((float)fileInfo.size()/payloadSize + 0.5f);
	//emit message(QString("Sending Packet %1/%2").arg(pos/payloadSize).arg((int)floor((float)fileInfo.size()/payloadSize)));
	emit message(QString("Sending Packets ..."));
	emit progress(floor((float)pos/fileInfo.size()*100.0f));
	Q_ASSERT(packet.size() == 1029);	
	for (int retry = 0; retry < 10 && !m_cancel; ++retry) {	
		yTransmitWrite(packet);
		if ((res = yTransmitRead(DLY_1S)) >= 0 ) {
			switch (res) {			
			case ACK:
				qDebug() << "ACK";								
				return yTransmitPacket(fileInfo,dev,payloadSize,sequenceNum+1,false);				
			case CAN:
				qDebug() << "CAN";
				if ((res = yTransmitRead(DLY_1S)) == CAN) {
					yTransmitWrite(ACK);
					yTransmitFlush();
					qDebug() << "XMODEM::xBeginTransmit - canceled by remote";
					return MODEM_REMOTE_CANCEL;
				}
				break;
			case NAK:
				qDebug() << "NAK";
				break;
			case 'c':
			case 'C':
				qDebug() << "C";				
				retry--;
				continue;
			case '>':
				return MODEM_RESTART;
			case '\0':
				qDebug() << "\\0";
				break;
			default:
				qDebug() << "XYMODEM::yTransmitPacket - Unknown " << res;
				break;
			}
		}
	}
	yTransmitWrite(CAN);
	yTransmitWrite(CAN);
	yTransmitWrite(CAN);
	yTransmitFlush();
	if (m_cancel)
		return MODEM_LOCAL_CANCEL;
	else
		return MODEM_PACKET_FAILED;
}

int YMODEM::yTransmitClose()
{
	QByteArray payload;
	payload += QString().toUtf8();
	int zero = 0;
	payload += (quint32) zero;
	payload += (quint32) zero;		
	payload += QString(CTRLZ).repeated(1024 - payload.size()).toUtf8();
	QByteArray packet;
	packet.append(QString(STX).toUtf8());
	int sequenceNum = 0;
	packet.append(sequenceNum);
	packet.append(~sequenceNum);
	packet.append(payload);
	unsigned short ccrc = crc16_ccitt((unsigned char*)payload.constData(),1024);
	packet.append((unsigned short)(ccrc>>8) & 0xFF);
	packet.append((unsigned short)ccrc & 0xFF);
	int res;
	for (int retry = 0; retry < 10; ++retry) 
	{
		yTransmitWrite(packet);
		if ((res = yTransmitRead((DLY_1S)<<1)) == ACK) break;
	}
	return (res == ACK)?MODEM_SUCCESS:MODEM_EOT_ERROR;	
}
