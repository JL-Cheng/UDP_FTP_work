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
	void enableConnectButton();//可以点击连接按钮进行连接
	void enableLoginButton();//可以点击登录按钮
	void enableSendButton();//可以点击发送按钮发送数据

	void showFileActionsMenu(QPoint pos);//右键点击显示菜单
	void refreshFilesList();//刷新文件列表
	void uploadFile();//下载文件
	void downloadFile();//下载文件
	void renameFile();//重命名文件
	void makeWorkingDir();//创建目录
	void removeWorkingDir();//删除目录
	void backWorkingDir();//返回上一级目录
	void enterWorkingDir(QTableWidgetItem* item);//改变工作目录
	void closeFileItem();//关闭选择的文件列表item
	
	void displayError(QAbstractSocket::SocketError socketError);//显示连接过程中的错误
	void connectOrDisconnect();//点击连接按钮进行连接或者（点击断开连接按钮断开连接）
	void login();//进行登录
	void serverConnected();//成功与服务器建立连接
	void serverDisconnected();//与服务器断开连接
	void sendMessage(QString command, QString param = "");//发送信息
	void readMessage();//读取收到的信息
	void newDataConnect();//建立新的数据连接
	void readData();//读取收到的数据
	void sendData();//传输数据
	
private:
	Ui::FTP_client_GUIClass ui;

	void init();//初始化各种设置
	void addTasksListItem();//向任务列表添加item
	void setFilesList();//向任务列表添加item
	void addNewText(QString newtext);//消息列表中添加新的消息

	QTcpSocket *m_socket = nullptr;//信息传输套接字
	QTcpSocket *d_socket = nullptr;//数据传输套接字
	QTcpServer *d_server = nullptr;//PORT方式接受连接信号

	QString command_status = "";//记录现在的命令类型
	QString next_command = "";//记录下一个相关命令类型
	QString next_param = "";//记录下一个相关命令参数
	QString connect_status = "";//记录现在的数据连接类型类型
	QString files_list_string = "";//文件列表字符串
	QString dest_IP = "";//PORT和PASV相关连接IP地址
	QString old_filename = "";//旧文件名
	QString working_dir = "";//现在的工作目录
	QList<QProgressBar*> progress_bars;//进度条列表

	QFile *file = nullptr;//读写的目标文件
	int task_row = 0;//任务所在的列表行
	int file_row = 0;//文件所在的列表行
	int dest_port = 0;//PORT和PASV相关连接端口
	int total_size = 0;//文件总大小
	int read_size = 0;//文件读取的大小

};
