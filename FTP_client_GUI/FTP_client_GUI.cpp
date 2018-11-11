#include "FTP_client_GUI.h"

FTP_client_GUI::FTP_client_GUI(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	init();
}

void FTP_client_GUI::init()
{
	m_socket = new QTcpSocket(this);
	d_socket = new QTcpSocket(this);
	d_server = new QTcpServer(this);

	ui.connect_button->setEnabled(false);
	ui.send_button->setEnabled(false);

	//设置窗口属性
	this->setFixedSize(700, 850);
	this->setWindowOpacity(1);
	this->setWindowTitle("FTP client");

	//设置任务列表显示
	ui.tasks_list->horizontalHeader()->setStretchLastSection(true);
	ui.tasks_list->setColumnWidth(0, 150);
	ui.tasks_list->setColumnWidth(1, 150);
	ui.tasks_list->setColumnWidth(2, 150);
	ui.tasks_list->setContextMenuPolicy(Qt::CustomContextMenu);

	//设置文件列表显示
	ui.files_list->horizontalHeader()->setStretchLastSection(true);
	ui.files_list->setColumnWidth(0, 150);
	ui.files_list->setColumnWidth(1, 150);
	ui.files_list->setColumnWidth(2, 150);
	ui.files_list->setContextMenuPolicy(Qt::CustomContextMenu);

	//创建信息传递套接字的信号槽
	connect(m_socket, &QTcpSocket::connected, this, &FTP_client_GUI::serverConnected);
	connect(m_socket, &QTcpSocket::readyRead, this, &FTP_client_GUI::readMessage);
	connect(m_socket, &QTcpSocket::disconnected, this, &FTP_client_GUI::serverDisconnected);
	connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
		this, &FTP_client_GUI::displayError);
	//创建数据传输套接字信号槽
	connect(d_socket, &QTcpSocket::readyRead, this, &FTP_client_GUI::readData);
	connect(d_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
		this, &FTP_client_GUI::displayError);
	//创建数据传输监听套接字信号槽
	connect(d_server,&QTcpServer::newConnection,this,&FTP_client_GUI::newDataConnect);

	//创建按钮状态转换的信号槽
	connect(ui.IP_lineEdit, &QLineEdit::textChanged,
		this, &FTP_client_GUI::enableConnectButton);
	connect(ui.port_lineEdit, &QLineEdit::textChanged,
		this, &FTP_client_GUI::enableConnectButton);
	connect(ui.command_lineEdit, &QLineEdit::textChanged,
		this, &FTP_client_GUI::enableSendButton);
	//创建点击按钮的信号槽
	connect(ui.connect_button, &QPushButton::clicked,
		this, &FTP_client_GUI::connectOrDisconnect);
	connect(ui.send_button, &QPushButton::clicked,
		this, &FTP_client_GUI::sendMessage);

}

void FTP_client_GUI::enableConnectButton()
{
	ui.connect_button->setEnabled(!ui.IP_lineEdit->text().isEmpty() &&
									!ui.port_lineEdit->text().isEmpty());
}

void FTP_client_GUI::enableSendButton()
{
	ui.send_button->setEnabled(!ui.command_lineEdit->text().isEmpty());
}

void FTP_client_GUI::connectOrDisconnect()
{
	if (!m_socket->state())
	{
		//连接到服务器
		ui.connect_button->setEnabled(false);
		m_socket->connectToHost(ui.IP_lineEdit->text(), ui.port_lineEdit->text().toInt(), QTcpSocket::ReadWrite);

	}
	else
	{
		//断开连接
		int a = m_socket->state();
		m_socket->disconnectFromHost();
	}
}

void FTP_client_GUI::sendMessage()
{
	QString text = ui.message_label->text();
	if (m_socket->state() != 3)//未成功连接
	{
		text += tr("The connection was not established.\n");
		ui.message_label->setText(text);
		return;
	}
	else
	{
		QString command = ui.command_lineEdit->text().trimmed();
		QString param = ui.param_lineEdit->text().trimmed();
		QString send_message = command + " " + param + "\r\n";

		//判断命令类型
		if (command == "PORT")//如果是port指令,获取ip地址和端口号
		{
			QStringList str_list = param.split(',');
			if (str_list.length() != 6)
			{
				text += tr("PORT params are wrong.\n");
				ui.message_label->setText(text);
				return;
			}
			dest_IP = str_list[0] + '.' + str_list[1] + '.' + str_list[2] + '.' + str_list[3];
			dest_port = str_list[4].toInt() * 256 + str_list[5].toInt();
			connect_status = "PORT";
		}
		if (command == "RETR")
		{
			read_size = 0;
			total_size = 0;
			QString file_name = param.split('/').last();
			if (file)
				file->close();
			file = new QFile(file_name);
			if (!file->open(QFile::WriteOnly))
			{
				text += tr("Open local file error.\n");
				ui.message_label->setText(text);
				return;
			}
			if (connect_status == "PORT")
			{
				if (d_server->isListening())
					d_server->close();
				if (!d_server->listen(QHostAddress::Any, dest_port))
				{
					text += tr("'PORT' data server listens error.\n");
					ui.message_label->setText(text);
					return;
				}
			}
			else if (connect_status == "PASV")
			{
				d_socket->connectToHost(dest_IP, dest_port, QTcpSocket::ReadWrite);
				//创建数据传输套接字信号槽
				connect(d_socket, &QTcpSocket::readyRead, this, &FTP_client_GUI::readData);
				connect(d_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
					this, &FTP_client_GUI::displayError);
			}
		}
		if (command == "STOR")
		{
			read_size = 0;
			total_size = 0;
			QString file_name = param;
			if (file)
				file->close();
			file = new QFile(file_name);
			if (!file->open(QFile::ReadOnly))
			{
				text += tr("Open local file error.\n");
				ui.message_label->setText(text);
				return;
			}
			total_size = file->size();
			if (connect_status == "PORT")
			{
				if (d_server->isListening())
					d_server->close();
				if (!d_server->listen(QHostAddress::Any, dest_port))
				{
					text += tr("'PORT' data server listens error.\n");
					ui.message_label->setText(text);
					return;
				}
			}
			else if (connect_status == "PASV")
			{
				d_socket->connectToHost(dest_IP, dest_port, QTcpSocket::ReadWrite);
				//创建数据传输套接字信号槽
				connect(d_socket, &QTcpSocket::readyRead, this, &FTP_client_GUI::readData);
				connect(d_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
					this, &FTP_client_GUI::displayError);
			}
		}
		if (command == "LIST")
		{
			read_size = 0;
			total_size = 0;
			files_list_string = "";
			if (connect_status == "PORT")
			{
				if (d_server->isListening())
					d_server->close();
				if (!d_server->listen(QHostAddress::Any, dest_port))
				{
					text += tr("'PORT' data server listens error.\n");
					ui.message_label->setText(text);
					return;
				}
			}
			else if (connect_status == "PASV")
			{
				d_socket->connectToHost(dest_IP, dest_port, QTcpSocket::ReadWrite);
				//创建数据传输套接字信号槽
				connect(d_socket, &QTcpSocket::readyRead, this, &FTP_client_GUI::readData);
				connect(d_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
					this, &FTP_client_GUI::displayError);
			}
		}
		command_status = command;
		m_socket->write(QByteArray(send_message.toLatin1()));
		m_socket->flush();
		m_socket->waitForBytesWritten(300);
	}
}

void FTP_client_GUI::readMessage()
{
	QString message;
	QString text = ui.message_label->text();
	while (m_socket->bytesAvailable()>0)
	{
		message = QString(m_socket->readLine(1024));
		text += QString(message);
		ui.message_label->setText(text);

		//判断返回信息
		if (command_status == "PASV")
		{
			if (message.split(' ')[0] == "227")
			{
				connect_status = "PASV";
				QStringList str_list = message.split('(')[1].split(')')[0].split(',');
				dest_IP = str_list[0] + '.' + str_list[1] + '.' + str_list[2] + '.' + str_list[3];
				dest_port = str_list[4].toInt() * 256 + str_list[5].toInt();
			}
		}
		else if (command_status == "RETR")
		{
			if (message.split(' ')[0]=="451" || message.split(' ')[0] == "426" || 
				message.split(' ')[0] == "425"|| message.split(' ')[0] == "550")
			{
				if (d_socket->state() != 0)
				{
					d_socket->disconnectFromHost();
					d_socket->close();

				}
				if (file)
				{
					file->close();
					file = nullptr;
				}
				read_size = 0;
				total_size = 0;
				connect_status = "";
			}
			else if (message.split(' ')[0] == "150")
			{
				total_size = message.split('(')[1].split(')')[0].toInt();
				this->addTasksListItem();
			}
		}
		else if (command_status == "STOR")
		{
			if (message.split(' ')[0] == "451" || message.split(' ')[0] == "426" ||
				message.split(' ')[0] == "425" || message.split(' ')[0] == "550" )
			{
				if (d_socket->state() != 0)
				{
					d_socket->disconnectFromHost();
					d_socket->close();

				}
				if (file)
				{
					file->close();
					file = nullptr;
				}
				read_size = 0;
				total_size = 0;
				connect_status = "";
			}
			else if (message.split(' ')[0] == "150")
			{
				this->addTasksListItem();
				this->sendData();
			}
		}
		else if (command_status == "LIST")
		{
			if (message.split(' ')[0] == "451" || message.split(' ')[0] == "426" ||
				message.split(' ')[0] == "425" || message.split(' ')[0] == "550")
			{
				if (d_socket->state() != 0)
				{
					d_socket->disconnectFromHost();
					d_socket->close();

				}
				read_size = 0;
				total_size = 0;
				connect_status = "";
			}
			else if (message.split(' ')[0] == "150")
			{
				total_size = message.split('(')[1].split(')')[0].toInt();
			}
		}
	}
}

void FTP_client_GUI::readData()
{
	if (command_status == "RETR")
	{
		QString text = ui.message_label->text();
		if (!file)
		{
			text += tr("Something wrong with the file.\n");
			ui.message_label->setText(text);
			d_socket->readAll();
			return;
		}
		else
		{
			QByteArray in_block;

			in_block = d_socket->readAll();
			file->write(in_block);
			file->flush();
			read_size += in_block.size();
			progress_bars[list_row]->setValue(read_size);
			QApplication::processEvents();

			m_socket->write(QByteArray(QString("finish\0").toLatin1()));
			m_socket->flush();
			m_socket->waitForBytesWritten(300);

			if (read_size == total_size)
			{
				// Close connect
				d_socket->flush();
				d_socket->disconnectFromHost();
				d_socket->close();
				file->close();
				file = nullptr;
				read_size = 0;
				total_size = 0;
				connect_status = "";
			}
			return;

		}
	}
	else if (command_status == "LIST")
	{
		QString text = ui.message_label->text();
		QByteArray in_block;
		
		in_block = d_socket->readAll();
		files_list_string += QString(in_block);
		read_size += in_block.size();

		m_socket->write(QByteArray(QString("finish\0").toLatin1()));
		m_socket->flush();
		m_socket->waitForBytesWritten(300);

		if (read_size == total_size)
		{
			text += files_list_string;
			ui.message_label->setText(text);
			this->setFilesList();

			d_socket->flush();
			d_socket->disconnectFromHost();
			d_socket->close();
			read_size = 0;
			total_size = 0;
			connect_status = "";
		}
		return;

	}
}

void FTP_client_GUI::sendData()
{
	QString text = ui.message_label->text();
	if (!file)
	{
		text += tr("Something wrong with the file.\n");
		ui.message_label->setText(text);
		d_socket->readAll();
		return;
	}
	else
	{
		QByteArray out_block;
		
		while (read_size != total_size)
		{
			out_block = file->read(1024);
			d_socket->write(out_block);
			d_socket->flush();
			d_socket->waitForBytesWritten(300);

			read_size += out_block.size();
			progress_bars[list_row]->setValue(read_size);
			QApplication::processEvents();
		}

		// Close connect
		d_socket->flush();
		d_socket->disconnectFromHost();
		d_socket->close();
		file->close();
		file = nullptr;
		read_size = 0;
		total_size = 0;
		connect_status = "";

		return;
	}

}

void FTP_client_GUI::serverConnected()
{
	ui.connect_button->setText("断开连接");
	ui.connect_button->setEnabled(true);
}

void FTP_client_GUI::serverDisconnected()
{
	ui.connect_button->setText("连接");
	ui.connect_button->setEnabled(true);
}

void FTP_client_GUI::newDataConnect()
{
	auto socket_temp = d_server->nextPendingConnection();
	d_socket = socket_temp;

	//创建数据传输套接字信号槽
	connect(d_socket, &QTcpSocket::readyRead, this, &FTP_client_GUI::readData);
	connect(d_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
		this, &FTP_client_GUI::displayError);
}

void FTP_client_GUI::addTasksListItem()
{
	list_row = ui.tasks_list->rowCount();
	ui.tasks_list->setRowCount(list_row + 1);
	ui.tasks_list->setItem(list_row, 0, new QTableWidgetItem(file->fileName()));
	ui.tasks_list->setItem(list_row, 1, new QTableWidgetItem(QString::number(total_size)+"byte"));
	ui.tasks_list->setItem(list_row, 2, new QTableWidgetItem((command_status=="RETR")?"下载":"上传"));
	QProgressBar *bar = new QProgressBar(ui.tasks_list);
	bar->setTextVisible(true);
	bar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	bar->setRange(0, total_size);
	bar->setValue(0);
	ui.tasks_list->setCellWidget(list_row, 3, bar);
	progress_bars.append(bar);
	QApplication::processEvents();
}

void FTP_client_GUI::setFilesList()
{
	QStringList files_list = files_list_string.split('\n');
	int file_row = 0;
	for (int i = 1; i < files_list.length(); i++)
	{
		QStringList file_info_list = files_list[i].split(QRegExp("[\\s]+"));
		file_row = ui.files_list->rowCount();
		ui.files_list->setRowCount(file_row + 1);
		if (file_info_list[0][0] == '-')
		{
			ui.files_list->setItem(file_row, 0, new QTableWidgetItem(QIcon(":/icons/file_icon"), file_info_list[8]));
		}
		else if (file_info_list[0][0] == 'd')
		{
			ui.files_list->setItem(file_row, 0, new QTableWidgetItem(QIcon(":/icons/dir_icon"), file_info_list[8]));
		}
	}
	QApplication::processEvents();
}


void FTP_client_GUI::displayError(QAbstractSocket::SocketError socketError)
{
	QString text = ui.message_label->text();

	switch (socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		break;
	case QAbstractSocket::HostNotFoundError:
		text += tr("The host was not found.\n");
		ui.message_label->setText(text);
		break;
	case QAbstractSocket::ConnectionRefusedError:
		text += tr("The connection was refused by the peer.\n");
		ui.message_label->setText(text);
		break;
	default:
		text += tr("The following error occurred: %1.\n").arg(m_socket->errorString());
		ui.message_label->setText(text);
	}

	ui.connect_button->setEnabled(true);
}