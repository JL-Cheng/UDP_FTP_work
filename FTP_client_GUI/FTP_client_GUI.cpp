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
	ui.login_button->setEnabled(false);

	//设置窗口属性
	this->setFixedSize(700, 900);
	this->setWindowOpacity(1);
	this->setWindowTitle("FTP client");

	ui.IP_lineEdit->setFocus();

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
	connect(d_server, &QTcpServer::newConnection, this, &FTP_client_GUI::newDataConnect);

	//创建按钮状态转换的信号槽
	connect(ui.IP_lineEdit, &QLineEdit::textChanged,
		this, &FTP_client_GUI::enableConnectButton);
	connect(ui.port_lineEdit, &QLineEdit::textChanged,
		this, &FTP_client_GUI::enableConnectButton);

	connect(ui.username_lineEdit, &QLineEdit::textChanged,
		this, &FTP_client_GUI::enableLoginButton);
	connect(ui.password_lineEdit, &QLineEdit::textChanged,
		this, &FTP_client_GUI::enableLoginButton);

	//创建点击按钮的信号槽
	connect(ui.connect_button, &QPushButton::clicked,
		this, &FTP_client_GUI::connectOrDisconnect);
	connect(ui.login_button, &QPushButton::clicked,
		this, &FTP_client_GUI::login);

	//创建文件列表鼠标事件
	connect(ui.files_list, &QWidget::customContextMenuRequested,
		this, &FTP_client_GUI::showFileActionsMenu);
	connect(ui.files_list, &QTableWidget::itemSelectionChanged,
		this, &FTP_client_GUI::closeFileItem);
	connect(ui.files_list, &QTableWidget::itemDoubleClicked,
		this, &FTP_client_GUI::enterWorkingDir);

	//创建任务列表鼠标事件
	connect(ui.tasks_list, &QWidget::customContextMenuRequested,
		this, &FTP_client_GUI::showTaskActionsMenu);

	//创建帮助菜单动作
	connect(ui.act_help, &QAction::triggered, this, &FTP_client_GUI::showHelp);
}

void FTP_client_GUI::enableConnectButton()
{
	ui.connect_button->setEnabled(!ui.IP_lineEdit->text().isEmpty() &&
		!ui.port_lineEdit->text().isEmpty());
}

void FTP_client_GUI::enableLoginButton()
{
	ui.login_button->setEnabled(!ui.username_lineEdit->text().isEmpty() &&
		!ui.password_lineEdit->text().isEmpty());
}

void FTP_client_GUI::showFileActionsMenu(QPoint pos)
{

	QModelIndex index = ui.files_list->indexAt(pos);
	QMenu *menu = new QMenu(ui.files_list);

	QAction *act_refresh = new QAction("刷新", ui.files_list);
	QAction *act_upload = new QAction("上传", ui.files_list);
	QAction *act_download = new QAction("下载", ui.files_list);
	QAction *act_rename = new QAction("重命名", ui.files_list);
	QAction *act_back = new QAction("返回上级目录", ui.files_list);
	QAction *act_makedir = new QAction("创建文件夹", ui.files_list);
	QAction *act_removedir = new QAction("删除文件夹", ui.files_list);

	connect(act_refresh, &QAction::triggered, this, &FTP_client_GUI::refreshFilesList);
	connect(act_upload, &QAction::triggered, this, &FTP_client_GUI::uploadFile);
	connect(act_download, &QAction::triggered, this, &FTP_client_GUI::downloadFile);
	connect(act_rename, &QAction::triggered, this, &FTP_client_GUI::renameFile);
	connect(act_back, &QAction::triggered, this, &FTP_client_GUI::backWorkingDir);
	connect(act_makedir, &QAction::triggered, this, &FTP_client_GUI::makeWorkingDir);
	connect(act_removedir, &QAction::triggered, this, &FTP_client_GUI::removeWorkingDir);

	if (d_socket->state() != 0)
	{
		act_refresh->setEnabled(false);
		act_upload->setEnabled(false);
		act_download->setEnabled(false);
		act_rename->setEnabled(false);
		act_back->setEnabled(false);
		act_makedir->setEnabled(false);
		act_removedir->setEnabled(false);
	}

	if (index.row() >= 0)
	{
		file_row = index.row();
		QString type = ui.files_list->item(file_row, 2)->text();
		if (type == "文件夹")
		{
			menu->addAction(act_refresh);
			menu->addAction(act_upload);
			menu->addAction(act_back);
			menu->addAction(act_makedir);
			menu->addAction(act_removedir);
		}
		else
		{
			menu->addAction(act_refresh);
			menu->addAction(act_upload);
			menu->addAction(act_download);
			menu->addAction(act_rename);
			menu->addAction(act_back);
			menu->addAction(act_makedir);
		}
	}
	else
	{
		menu->addAction(act_refresh);
		menu->addAction(act_upload);
		menu->addAction(act_back);
		menu->addAction(act_makedir);
	}

	menu->move(cursor().pos());
	menu->show();
}

void FTP_client_GUI::refreshFilesList()
{
	if (m_socket->state() != 3)//未成功连接
	{
		this->addNewText(tr("The connection was not established.\n"));
		return;
	}

	this->next_command = "LIST";
	this->next_param = "";
	QString command = "PORT";
	QString param = m_socket->localAddress().toString().replace(".", ",") + ",123,62";

	this->sendMessage(command, param);
}

void FTP_client_GUI::uploadFile()
{
	if (m_socket->state() != 3)//未成功连接
	{
		this->addNewText(tr("The connection was not established.\n"));
		return;
	}

	QString filename = QFileDialog::getOpenFileName(NULL, "选择上传文件", ".");

	this->next_command = "STOR";
	this->next_param = filename;
	QString command = "PORT";
	QString param = m_socket->localAddress().toString().replace(".", ",") + ",123,56";

	this->sendMessage(command, param);

}

void FTP_client_GUI::downloadFile()
{
	if (m_socket->state() != 3)//未成功连接
	{
		this->addNewText(tr("The connection was not established.\n"));
		return;
	}

	QString filename = ui.files_list->item(file_row, 0)->text();
	QString type = ui.files_list->item(file_row, 2)->text();

	if (type != "文件夹")
	{
		this->next_command = "RETR";
		this->next_param = filename;
		QString command = "PORT";
		QString param = m_socket->localAddress().toString().replace(".", ",") + ",123,56";

		this->sendMessage(command, param);
	}
	else
	{
		this->addNewText(tr("Folder can not be downloaded.\n"));
		return;
	}
}

void FTP_client_GUI::renameFile()
{
	if (m_socket->state() != 3)//未成功连接
	{
		this->addNewText(tr("The connection was not established.\n"));
		return;
	}

	//获得当前节点并获取编辑名称
	QTableWidgetItem *selected_file_item = ui.files_list->item(file_row, 0);
	ui.files_list->setCurrentCell(file_row, 0);
	ui.files_list->openPersistentEditor(selected_file_item);
	ui.files_list->editItem(selected_file_item);

	QString type = ui.files_list->item(file_row, 2)->text();

	if (type != "文件夹")
	{
		this->next_command = "RNTO";
		old_filename = ui.files_list->item(file_row, 0)->text();
	}
	else
	{
		this->addNewText(tr("Folder can not be renamed.\n"));
		return;
	}
}

void FTP_client_GUI::enterWorkingDir(QTableWidgetItem * item)
{
	if (m_socket->state() != 3)//未成功连接
	{
		this->addNewText(tr("The connection was not established.\n"));
		return;
	}

	int row = item->row();
	QString dirname = ui.files_list->item(row, 0)->text();
	QString type = ui.files_list->item(row, 2)->text();
	if (type == "文件夹")
	{
		this->working_dir += dirname + "/";
		this->sendMessage("CWD", this->working_dir);
	}
	else
	{
		this->addNewText(tr("File can not be entered.\n"));
		return;
	}

}

void FTP_client_GUI::makeWorkingDir()
{
	if (m_socket->state() != 3)//未成功连接
	{
		this->addNewText(tr("The connection was not established.\n"));
		return;
	}

	file_row = ui.files_list->rowCount();
	ui.files_list->setRowCount(file_row + 1);
	//获得当前节点并获取编辑名称
	QTableWidgetItem *selected_file_item = ui.files_list->item(file_row, 0);
	ui.files_list->setCurrentCell(file_row, 0);
	ui.files_list->openPersistentEditor(selected_file_item);
	ui.files_list->editItem(selected_file_item);

	this->next_command = "MKD";
}

void FTP_client_GUI::removeWorkingDir()
{
	if (m_socket->state() != 3)//未成功连接
	{
		this->addNewText(tr("The connection was not established.\n"));
		return;
	}

	QString dirname = ui.files_list->item(file_row, 0)->text();
	QString type = ui.files_list->item(file_row, 2)->text();

	if (type == "文件夹")
	{
		QString command = "RMD";
		QString param = dirname;
		this->sendMessage(command, param);
	}
	else
	{
		this->addNewText(tr("File can not be removed.\n"));
		return;
	}
}

void FTP_client_GUI::backWorkingDir()
{
	if (m_socket->state() != 3)//未成功连接
	{
		this->addNewText(tr("The connection was not established.\n"));
		return;
	}

	if (this->working_dir == "/")
	{
		this->addNewText(tr("It's in the root directory.\n"));
		return;
	}
	else
	{
		QStringList temp_str_list = this->working_dir.split('/');
		this->working_dir = "/";
		for (int i = 1; i < temp_str_list.length() - 2; i++)
		{
			this->working_dir += temp_str_list[i] + "/";
		}
		this->sendMessage("CWD", this->working_dir);
	}
}

void FTP_client_GUI::closeFileItem()
{
	QTableWidgetItem *selected_file_item = ui.files_list->item(file_row, 0);
	if (selected_file_item != nullptr)
	{
		ui.files_list->closePersistentEditor(selected_file_item);
		if (this->next_command == "RNTO")
		{
			QString new_filename = selected_file_item->text();
			if (new_filename == "")
			{
				this->addNewText(tr("New filename cannot be null.\n"));
				selected_file_item->setText(old_filename);
				this->next_command = "";
			}
			else
			{
				this->next_param = new_filename;
				this->sendMessage("RNFR", old_filename);
			}
			old_filename = "";
		}
		else if (this->next_command == "MKD")
		{
			QString dir_name = selected_file_item->text();
			if (dir_name == "")
			{
				this->addNewText(tr("New dirname cannot be null.\n"));
				ui.files_list->setRowCount(file_row);
			}
			else
			{
				this->sendMessage("MKD", dir_name);
			}
			this->next_command = "";
		}
	}
	return;
}

void FTP_client_GUI::showTaskActionsMenu(QPoint pos)
{

	QModelIndex index = ui.tasks_list->indexAt(pos);
	QMenu *menu = new QMenu(ui.tasks_list);

	QAction *act_continue_download = new QAction("继续下载", ui.tasks_list);
	QAction *act_stop_download = new QAction("停止下载", ui.tasks_list);

	connect(act_continue_download, &QAction::triggered, this, &FTP_client_GUI::continueDownloadTask);
	connect(act_stop_download, &QAction::triggered, this, &FTP_client_GUI::stopDownloadTask);

	if (d_socket->state() != 0)
		act_continue_download->setEnabled(false);
	else
		act_stop_download->setEnabled(false);

	if (index.row() >= 0)
	{
		task_row = index.row();
		QString type = ui.tasks_list->item(task_row, 2)->text();
		if (type == "下载")
		{
			menu->addAction(act_continue_download);
			menu->addAction(act_stop_download);
		}
	}

	menu->move(cursor().pos());
	menu->show();
}

void FTP_client_GUI::continueDownloadTask()
{
	if (m_socket->state() != 3)//未成功连接
	{
		this->addNewText(tr("The connection was not established.\n"));
		return;
	}

	QString filename = ui.tasks_list->item(task_row, 0)->text();
	QFile temp_file(filename);
	if (!temp_file.exists())
	{
		this->addNewText(tr("The file doesn't exist.\n"));
		return;
	}

	continue_recv = true;
	this->sendMessage("REST", QString::number(temp_file.size()));

}

void FTP_client_GUI::stopDownloadTask()
{
	m_socket->write(QByteArray(QString("stop\0").toLatin1()));
	m_socket->flush();
	m_socket->waitForBytesWritten(300);
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
	first_recv = true;
	pack_size = 0;
	continue_recv = false;
	connect_status = "";
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
		this->sendMessage("QUIT");
	}
}

void FTP_client_GUI::login()
{
	if (m_socket->state() != 3)//未成功连接
	{
		this->addNewText(tr("The connection was not established.\n"));
		return;
	}
	QString username = ui.username_lineEdit->text().trimmed();
	QString password = ui.password_lineEdit->text().trimmed();

	this->next_command = "PASS";
	this->next_param = password;
	this->sendMessage("USER", username);
}

void FTP_client_GUI::showHelp()
{
	QString helpText =
		"1 输入正确的ip地址和端口号，点击连接按钮进行连接；\n"\
		"2 输入用户名和密码进行登录；\n"\
		"3 在文件列表中双击文件夹可以进入对应目录；\n"\
		"4 在文件目录中点击右键会出现如下选项：\n"\
		"  4.1 刷新：重新获取当前目录的文件列表；\n"\
		"  4.2 上传：选择文件上传到服务器；\n"\
		"  4.3 返回上级目录：返回到上一级目录；\n"\
		"  4.4 创建文件夹：在当前目录中创建文件夹；\n"\
		"  4.5 刷新：重新获取当前目录的文件列表；\n"\
		"  4.6 下载（只对文件有效）：下载文件到目录；\n"\
		"  4.7 重命名（只对文件有效）：对文件重命名；\n"\
		"  4.8 删除文件夹（只对空文件夹有效）：\n"\
		"         删除选中的文件夹；\n"\
		"5 在任务目录中点击右键会出现如下选项：\n"\
		"  5.1 停止下载：终止文件下载；\n"\
		"  5.2 继续下载：进行断点续传；\n";


	QMessageBox message;
	message.setWindowTitle(tr("帮助"));
	message.setText(helpText);
	message.setIcon(QMessageBox::Information);
	message.addButton(tr("OK懂了~"), QMessageBox::ActionRole);
	message.exec();

}

void FTP_client_GUI::sendMessage(QString command, QString param)
{
	QString send_message = command + " " + param + "\r\n";

	//判断命令类型
	if (command == "PORT")//如果是port指令,获取ip地址和端口号
	{
		QStringList str_list = param.split(',');
		if (str_list.length() != 6)
		{
			this->addNewText(tr("PORT params are wrong.\n"));
			return;
		}
		dest_IP = str_list[0] + '.' + str_list[1] + '.' + str_list[2] + '.' + str_list[3];
		dest_port = str_list[4].toInt() * 256 + str_list[5].toInt();
		connect_status = "PORT";
	}
	else if (command == "RETR")
	{
		read_size = 0;
		total_size = 0;
		QString file_name = param.split('/').last();
		if (file)
			file->close();
		file = new QFile(file_name);

		if (continue_recv)
		{
			if (!file->open(QFile::WriteOnly | QIODevice::Append))
			{
				this->addNewText(tr("Open local file error.\n"));
				return;
			}
			read_size = file->size();
			file->seek(file->size());
		}
		else
		{
			if (!file->open(QFile::WriteOnly))
			{
				this->addNewText(tr("Open local file error.\n"));
				return;
			}
		}

		if (connect_status == "PORT")
		{
			if (d_server->isListening())
				d_server->close();
			if (!d_server->listen(QHostAddress::Any, dest_port))
			{
				this->addNewText(tr("'PORT' data server listens error.\n"));
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
	else if (command == "STOR")
	{
		read_size = 0;
		total_size = 0;
		QString file_name = param;
		if (file)
			file->close();
		file = new QFile(file_name);
		if (!file->open(QFile::ReadOnly))
		{
			this->addNewText(tr("Open local file error.\n"));
			return;
		}
		total_size = file->size();
		if (connect_status == "PORT")
		{
			if (d_server->isListening())
				d_server->close();
			if (!d_server->listen(QHostAddress::Any, dest_port))
			{
				this->addNewText(tr("'PORT' data server listens error.\n"));
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
	else if (command == "LIST")
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
				this->addNewText(tr("'PORT' data server listens error.\n"));
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

void FTP_client_GUI::readMessage()
{
	QString message;
	while (m_socket->bytesAvailable()>0)
	{
		message = QString(m_socket->readLine(1024));
		this->addNewText(QString(message));

		//判断返回信息
		if (command_status == "USER")
		{
			if (message.split(' ')[0] == "331")
			{
				this->sendMessage(this->next_command, this->next_param);
			}
			this->next_command = "";
			this->next_param = "";
		}
		else if (command_status == "PASS")
		{
			if (message.split(' ')[0] == "230")
			{
				ui.login_button->setEnabled(false);
				this->refreshFilesList();
			}
		}
		else if (command_status == "PORT")
		{
			if (message.split(' ')[0] == "200")
			{
				this->sendMessage(this->next_command, this->next_param);
			}
			this->next_command = "";
			this->next_param = "";
		}
		else if (command_status == "PASV")
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
			if (message.split(' ')[0] == "451" || message.split(' ')[0] == "426" ||
				message.split(' ')[0] == "425" || message.split(' ')[0] == "550")
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
				first_recv = true;
				pack_size = 0;
				continue_recv = false;
				connect_status = "";
			}
			else if (message.split(' ')[0] == "150")
			{
				total_size = message.split('(')[1].split(')')[0].toInt();
				if (!continue_recv)
				{
					this->addTasksListItem();
				}
			}
		}
		else if (command_status == "STOR")
		{
			if (message.split(' ')[0] == "451" || message.split(' ')[0] == "426" ||
				message.split(' ')[0] == "425" || message.split(' ')[0] == "550")
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
			else if (message.split(' ')[0] == "226")
			{
				this->refreshFilesList();
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
			else if (message.split(' ')[0] == "226")
			{
				this->sendMessage("PWD");
			}
		}
		else if (command_status == "RNFR")
		{
			if (message.split(' ')[0] == "350")
			{
				this->sendMessage(this->next_command, this->next_param);
			}
			this->next_command = "";
			this->next_param = "";
		}
		else if (command_status == "RNTO")
		{
			this->refreshFilesList();
		}
		else if (command_status == "PWD")
		{
			if (message.split(' ')[0] == "257")
			{
				this->working_dir = message.split(':')[1].split('.')[0];
				ui.files_list_label->setText(tr("文件列表（当前目录为%1）：").arg(this->working_dir));
			}
		}
		else if (command_status == "CWD")
		{
			if (message.split(' ')[0] == "250")
			{
				this->refreshFilesList();
			}
		}
		else if (command_status == "MKD")
		{
			if (message.split(' ')[0] == "257")
			{
				this->refreshFilesList();
			}
			else
			{
				ui.files_list->setRowCount(file_row);
			}
		}
		else if (command_status == "RMD")
		{
			if (message.split(' ')[0] == "250")
			{
				this->refreshFilesList();
			}
		}
		else if (command_status == "QUIT")
		{
			if (message.split(' ')[0] == "221")
			{
				m_socket->disconnectFromHost();
				m_socket->close();
			}
		}
		else if (command_status == "REST")
		{
			if (message.split(' ')[0] == "350")
			{
				QString filename = ui.tasks_list->item(task_row, 0)->text();

				this->next_command = "RETR";
				this->next_param = filename;
				QString command = "PORT";
				QString param = m_socket->localAddress().toString().replace(".", ",") + ",123,56";

				this->sendMessage(command, param);
			}
		}
	}
}

void FTP_client_GUI::readData()
{
	if (command_status == "RETR")
	{
		if (!file)
		{
			this->addNewText(tr("Something wrong with the file.\n"));
			d_socket->readAll();
			ui.connect_button->setEnabled(true);
			return;
		}
		else
		{
			QByteArray in_block;
			in_block = d_socket->readAll();

			if (first_recv)
			{
				pack_size = QString(in_block.mid(0, 10)).toInt();
				in_block = in_block.mid(10);
				first_recv = false;
			}

			file->write(in_block);
			file->flush();
			read_size += in_block.size();
			pack_size -= in_block.size();
			progress_bars[task_row]->setValue(read_size);
			QApplication::processEvents();

			if (!pack_size)
			{
				m_socket->write(QByteArray(QString("finish\0").toLatin1()));
				m_socket->flush();
				m_socket->waitForBytesWritten(300);
				first_recv = true;
			}

			if (read_size == total_size)
			{
				// Close connect
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
				first_recv = true;
				pack_size = 0;
				continue_recv = false;
				connect_status = "";
				ui.connect_button->setEnabled(true);
				return;
			}
		}
	}
	else if (command_status == "LIST")
	{
		QByteArray in_block;

		in_block = d_socket->readAll();
		files_list_string += QString(in_block);
		read_size += in_block.size();

		m_socket->write(QByteArray(QString("finish\0").toLatin1()));
		m_socket->flush();
		m_socket->waitForBytesWritten(300);

		if (read_size == total_size)
		{
			this->setFilesList();

			d_socket->flush();
			d_socket->disconnectFromHost();
			d_socket->close();
			read_size = 0;
			total_size = 0;
			connect_status = "";
			ui.connect_button->setEnabled(true);
			return;
		}
	}
}

void FTP_client_GUI::sendData()
{
	if (!file)
	{
		this->addNewText(tr("Something wrong with the file.\n"));
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
			progress_bars[task_row]->setValue(read_size);
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
	ui.login_button->setEnabled(true);
}

void FTP_client_GUI::serverDisconnected()
{
	ui.connect_button->setText("连接");
	ui.connect_button->setEnabled(true);
	ui.login_button->setEnabled(true);
}

void FTP_client_GUI::newDataConnect()
{
	auto socket_temp = d_server->nextPendingConnection();
	d_socket = socket_temp;

	ui.connect_button->setEnabled(false);

	//创建数据传输套接字信号槽
	connect(d_socket, &QTcpSocket::readyRead, this, &FTP_client_GUI::readData);
	connect(d_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
		this, &FTP_client_GUI::displayError);
}

void FTP_client_GUI::addTasksListItem()
{
	task_row = ui.tasks_list->rowCount();
	ui.tasks_list->setRowCount(task_row + 1);
	ui.tasks_list->setItem(task_row, 0, new QTableWidgetItem(file->fileName()));
	if (total_size < 1024)
	{
		ui.tasks_list->setItem(task_row, 1, new QTableWidgetItem(QString::number(total_size) + " byte"));
	}
	else if (total_size > 1024 && total_size < 1024 * 1024)
	{
		ui.tasks_list->setItem(task_row, 1, new QTableWidgetItem(QString::number(total_size / 1024) + " KB"));
	}
	else if (total_size > 1024 * 1024)
	{
		ui.tasks_list->setItem(task_row, 1, new QTableWidgetItem(QString::number(total_size / 1024 / 1024) + " MB"));
	}
	ui.tasks_list->setItem(task_row, 2, new QTableWidgetItem((command_status == "RETR") ? "下载" : "上传"));
	QProgressBar *bar = new QProgressBar(ui.tasks_list);
	bar->setTextVisible(true);
	bar->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	bar->setRange(0, total_size);
	bar->setValue(0);
	ui.tasks_list->setCellWidget(task_row, 3, bar);
	progress_bars.append(bar);

	for (int j = 0; j < 4; j++)
	{
		if (ui.tasks_list->item(task_row, j) != NULL)
		{
			ui.tasks_list->item(task_row, j)->setFlags(ui.tasks_list->item(task_row, j)->flags() & (~Qt::ItemIsEditable));
			ui.tasks_list->item(task_row, j)->setFlags(ui.tasks_list->item(task_row, j)->flags() & (~Qt::ItemIsSelectable));
		}
	}
	QApplication::processEvents();
}

void FTP_client_GUI::setFilesList()
{
	ui.files_list->clearContents();
	ui.files_list->setRowCount(0);
	QStringList files_list = files_list_string.split('\n');
	file_row = 0;
	int file_size = 0;
	for (int i = 1; i < files_list.size(); i++)
	{
		if (!files_list[i].size())
			continue;
		QStringList file_info_list = files_list[i].split(QRegExp("[\\s]+"));
		file_row = ui.files_list->rowCount();
		ui.files_list->setRowCount(file_row + 1);
		if (file_info_list[0][0] == '-')
		{
			ui.files_list->setItem(file_row, 0, new QTableWidgetItem(QIcon(":/icons/file_icon"), file_info_list[8]));
			if (file_info_list[8].contains("."))
			{
				ui.files_list->setItem(file_row, 2, new QTableWidgetItem(file_info_list[8].split('.').last() + " 文件"));
			}
			else
			{
				ui.files_list->setItem(file_row, 2, new QTableWidgetItem("文件"));
			}
			file_size = file_info_list[4].toInt();
			if (file_size < 1024)
			{
				ui.files_list->setItem(file_row, 3, new QTableWidgetItem(QString::number(file_size) + " byte"));
			}
			else if (file_size > 1024 && file_size < 1024 * 1024)
			{
				ui.files_list->setItem(file_row, 3, new QTableWidgetItem(QString::number(file_size / 1024) + " KB"));
			}
			else if (file_size > 1024 * 1024)
			{
				ui.files_list->setItem(file_row, 3, new QTableWidgetItem(QString::number(file_size / 1024 / 1024) + " MB"));
			}

		}
		else if (file_info_list[0][0] == 'd')
		{
			ui.files_list->setItem(file_row, 0, new QTableWidgetItem(QIcon(":/icons/dir_icon"), file_info_list[8]));
			ui.files_list->setItem(file_row, 2, new QTableWidgetItem("文件夹"));
		}
		ui.files_list->setItem(file_row, 1, new QTableWidgetItem(file_info_list[5] + " " + file_info_list[6] + " " + file_info_list[7]));
	}
	for (int i = 0; i<ui.files_list->rowCount(); i++)
	{
		for (int j = 0; j<ui.files_list->columnCount(); j++)
		{
			if (ui.files_list->item(i, j) != NULL)
			{
				ui.files_list->item(i, j)->setFlags(ui.files_list->item(i, j)->flags() & (~Qt::ItemIsEditable));
				ui.files_list->item(i, j)->setFlags(ui.files_list->item(i, j)->flags() & (~Qt::ItemIsSelectable));
			}
		}
	}
	QApplication::processEvents();
}

void FTP_client_GUI::addNewText(QString newtext)
{
	QString text = ui.message_label->text();
	text += newtext;
	ui.message_label->setText(text);
	ui.scrollArea->verticalScrollBar()->setValue(ui.scrollArea->verticalScrollBar()->maximum());
}

void FTP_client_GUI::displayError(QAbstractSocket::SocketError socketError)
{
	switch (socketError)
	{
	case QAbstractSocket::RemoteHostClosedError:
		break;
	case QAbstractSocket::HostNotFoundError:
		this->addNewText(tr("The host was not found.\n"));
		break;
	case QAbstractSocket::ConnectionRefusedError:
		this->addNewText(tr("The connection was refused by the peer.\n"));
		break;
	default:
		this->addNewText(tr("The following error occurred: %1.\n").arg(m_socket->errorString()));
	}

	ui.connect_button->setEnabled(true);
}