//�ͻ�����ʾ����

#pragma once
#pragma execution_character_set("utf-8")

#include <QtWidgets/QMainWindow>
#include "ui_FTP_client_GUI.h"

#include <QDataStream>
#include <QTcpSocket>
#include<QTcpServer>
#include<QString>
#include<QFile>
#include<QTextStream>
#include <QProgressBar>

class ClientThread;

class FTP_client_GUI : public QMainWindow
{
	Q_OBJECT

public:
	FTP_client_GUI(QWidget *parent = Q_NULLPTR);

private slots:
	void enableConnectButton();//���Ե�����Ӱ�ť��������
	void enableSendButton();//���Ե�����Ͱ�ť��������
	
	void displayError(QAbstractSocket::SocketError socketError);//��ʾ���ӹ����еĴ���
	void connectOrDisconnect();//������Ӱ�ť�������ӻ��ߣ�����Ͽ����Ӱ�ť�Ͽ����ӣ�
	void serverConnected();//�ɹ����������������
	void serverDisconnected();//��������Ͽ�����
	void sendMessage();//������Ͱ�ť������Ϣ
	void readMessage();//��ȡ�յ�����Ϣ
	void newDataConnect();//�����µ���������
	void readData();//��ȡ�յ�������
	void sendData();//��������
	
private:
	Ui::FTP_client_GUIClass ui;

	void init();//��ʼ����������
	void addTaskListItem();//�������б����item

	QTcpSocket *m_socket = nullptr;//��Ϣ�����׽���
	QTcpSocket *d_socket = nullptr;//���ݴ����׽���
	QTcpServer *d_server = nullptr;//PORT��ʽ���������ź�

	QString command_status = "";//��¼���ڵ���������
	QString connect_status = "";//��¼���ڵ�����������������
	QString dest_IP = "";//PORT��PASV�������IP��ַ
	QList<QProgressBar*> progress_bars;//�������б�

	QFile *file = nullptr;//��д��Ŀ���ļ�
	int file_row = 0;//�ļ����ڵ��б���
	int dest_port = 0;//PORT��PASV������Ӷ˿�
	int total_size = 0;//�ļ��ܴ�С
	int read_size = 0;//�ļ���ȡ�Ĵ�С

};
