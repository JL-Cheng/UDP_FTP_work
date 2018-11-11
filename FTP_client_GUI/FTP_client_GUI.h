//客户端显示界面

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
	void enableConnectButton();//可以点击连接按钮进行连接
	void enableSendButton();//可以点击发送按钮发送数据
	
	void displayError(QAbstractSocket::SocketError socketError);//显示连接过程中的错误
	void connectOrDisconnect();//点击连接按钮进行连接或者（点击断开连接按钮断开连接）
	void serverConnected();//成功与服务器建立连接
	void serverDisconnected();//与服务器断开连接
	void sendMessage();//点击发送按钮发送信息
	void readMessage();//读取收到的信息
	void newDataConnect();//建立新的数据连接
	void readData();//读取收到的数据
	void sendData();//传输数据
	
private:
	Ui::FTP_client_GUIClass ui;

	void init();//初始化各种设置
	void addTaskListItem();//向任务列表添加item

	QTcpSocket *m_socket = nullptr;//信息传输套接字
	QTcpSocket *d_socket = nullptr;//数据传输套接字
	QTcpServer *d_server = nullptr;//PORT方式接受连接信号

	QString command_status = "";//记录现在的命令类型
	QString connect_status = "";//记录现在的数据连接类型类型
	QString dest_IP = "";//PORT和PASV相关连接IP地址
	QList<QProgressBar*> progress_bars;//进度条列表

	QFile *file = nullptr;//读写的目标文件
	int file_row = 0;//文件所在的列表行
	int dest_port = 0;//PORT和PASV相关连接端口
	int total_size = 0;//文件总大小
	int read_size = 0;//文件读取的大小

};
