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
#include<QRegExp>
#include <QProgressBar>
#include<QScrollBar>
#include <QAction>
#include<QMenu>
#include<QFileDialog>

class ClientThread;

class FTP_client_GUI : public QMainWindow
{
	Q_OBJECT

public:
	FTP_client_GUI(QWidget *parent = Q_NULLPTR);

private slots:
	void enableConnectButton();//���Ե�����Ӱ�ť��������
	void enableLoginButton();//���Ե����¼��ť
	void enableSendButton();//���Ե�����Ͱ�ť��������

	void showFileActionsMenu(QPoint pos);//�Ҽ������ʾ�˵�
	void refreshFilesList();//ˢ���ļ��б�
	void uploadFile();//�����ļ�
	void downloadFile();//�����ļ�
	void renameFile();//�������ļ�
	void makeWorkingDir();//����Ŀ¼
	void removeWorkingDir();//ɾ��Ŀ¼
	void backWorkingDir();//������һ��Ŀ¼
	void enterWorkingDir(QTableWidgetItem* item);//�ı乤��Ŀ¼
	void closeFileItem();//�ر�ѡ����ļ��б�item
	
	void displayError(QAbstractSocket::SocketError socketError);//��ʾ���ӹ����еĴ���
	void connectOrDisconnect();//������Ӱ�ť�������ӻ��ߣ�����Ͽ����Ӱ�ť�Ͽ����ӣ�
	void login();//���е�¼
	void serverConnected();//�ɹ����������������
	void serverDisconnected();//��������Ͽ�����
	void sendMessage(QString command, QString param = "");//������Ϣ
	void readMessage();//��ȡ�յ�����Ϣ
	void newDataConnect();//�����µ���������
	void readData();//��ȡ�յ�������
	void sendData();//��������
	
private:
	Ui::FTP_client_GUIClass ui;

	void init();//��ʼ����������
	void addTasksListItem();//�������б����item
	void setFilesList();//�������б����item
	void addNewText(QString newtext);//��Ϣ�б�������µ���Ϣ

	QTcpSocket *m_socket = nullptr;//��Ϣ�����׽���
	QTcpSocket *d_socket = nullptr;//���ݴ����׽���
	QTcpServer *d_server = nullptr;//PORT��ʽ���������ź�

	QString command_status = "";//��¼���ڵ���������
	QString next_command = "";//��¼��һ�������������
	QString next_param = "";//��¼��һ������������
	QString connect_status = "";//��¼���ڵ�����������������
	QString files_list_string = "";//�ļ��б��ַ���
	QString dest_IP = "";//PORT��PASV�������IP��ַ
	QString old_filename = "";//���ļ���
	QString working_dir = "";//���ڵĹ���Ŀ¼
	QList<QProgressBar*> progress_bars;//�������б�

	QFile *file = nullptr;//��д��Ŀ���ļ�
	int task_row = 0;//�������ڵ��б���
	int file_row = 0;//�ļ����ڵ��б���
	int dest_port = 0;//PORT��PASV������Ӷ˿�
	int total_size = 0;//�ļ��ܴ�С
	int read_size = 0;//�ļ���ȡ�Ĵ�С

};
