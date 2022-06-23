#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
	, ui(new Ui::MainWindow)
{
	ui->setupUi(this);

	// Инициализируем Таймер и подключим его к слоту,
	// который будет обрабатывать timeout() таймера
	timer = new QTimer();
	connect(timer, SIGNAL(timeout()), this, SLOT(onTimerAlarm()));
	timer->start(POLLING_FREQUENCY); // И запустим таймер
}

MainWindow::~MainWindow()
{
	if (socket) {
		delete socket;
		socket = nullptr;
	}
	if (timer) {
		delete timer;
		timer = nullptr;
	}
	delete ui;
}

void MainWindow::onTimerAlarm() {
	if (socket) {
		std::string request = "ping";
		std::cerr << request << std::endl;
		socket->write(request.c_str(), request.size());

		char buff[BUFFER_SIZE];
		memset(buff, '\0', sizeof(char));
		if (socket->waitForReadyRead(250)) socket->read(buff, BUFFER_SIZE);
		else {
			ActivateButtons(false);
			statusBar()->showMessage("Error: Connection lost.", 3000);
		}
	}
}

void MainWindow::SetBoxStatus(QGroupBox* gb1, QGroupBox* gb2, const QCheckBox* cb) {
	gb1->setEnabled(cb->isChecked());
	gb2->setEnabled(cb->isChecked());
}

void MainWindow::AddRemoveChannel(const bool is_add, const int channel_number,
																	QLabel* voltage_lable, QLabel* status_lable) {
	Channel channel(channel_number, voltage_lable, status_lable);
	if (is_add) {
		Channel channel(channel_number, voltage_lable, status_lable);
		active_channels.insert(channel);
	}
	else {
		const auto it = active_channels.find(channel);
		it->status->setText("");
		it->voltage->setText("0");
		active_channels.erase(it);
	}
}

void MainWindow::on_channel0_checkBox_stateChanged()
{
	AddRemoveChannel(ui->channel0_checkBox->isChecked(), 0, ui->channel0_label_voltage, ui->channel0_status_label);
	SetBoxStatus(ui->channel0_voltage_groupBox, ui->channel0_range_groupBox, ui->channel0_checkBox);
}

void MainWindow::on_channel1_checkBox_stateChanged()
{
	AddRemoveChannel(ui->channel1_checkBox->isChecked(), 1, ui->channel1_label_voltage, ui->channel1_status_label);
	SetBoxStatus(ui->channel1_voltage_groupBox, ui->channel1_range_groupBox, ui->channel1_checkBox);
}

void MainWindow::on_channel2_checkBox_stateChanged()
{
	AddRemoveChannel(ui->channel2_checkBox->isChecked(), 2, ui->channel2_label_voltage, ui->channel2_status_label);
	SetBoxStatus(ui->channel2_voltage_groupBox, ui->channel2_range_groupBox, ui->channel2_checkBox);
}

void MainWindow::on_channel3_checkBox_stateChanged()
{
	AddRemoveChannel(ui->channel3_checkBox->isChecked(), 3, ui->channel3_label_voltage, ui->channel3_status_label);
	SetBoxStatus(ui->channel3_voltage_groupBox, ui->channel3_range_groupBox, ui->channel3_checkBox);
}

bool MainWindow::Connect() {
	socket = new QLocalSocket;
	const QString home_path(getenv("HOME"));
	socket_path = home_path + "/.cache/.server-socket";
	socket->connectToServer(socket_path);
	return socket->isOpen();
}

bool MainWindow::Disconnect() {
	bool result = true;
	if (socket) {
		socket->disconnectFromServer();
		result = !socket->isOpen();
		delete socket;
	}
	return result;
}

void MainWindow::ActivateButtons(const bool is_active) {
	ui->start_measure_pushButton->setEnabled(is_active);
	ui->stop_measure_pushButton->setEnabled(is_active);
	ui->get_status_pushButton->setEnabled(is_active);
	ui->get_result_pushButton->setEnabled(is_active);
	ui->connect_pushButton->setEnabled(!is_active);
	ui->disconnect_pushButton->setEnabled(is_active);
}

void MainWindow::on_connect_pushButton_clicked()
{
	bool is_connect = Connect();
	ActivateButtons(is_connect);
	if (is_connect) statusBar()->showMessage("Connection success.", 3000);
	else statusBar()->showMessage("Error: Server not found.", 3000);
}

void MainWindow::on_disconnect_pushButton_clicked()
{
	bool is_disconnect = Connect();
	ActivateButtons(Disconnect());
	if (is_disconnect) statusBar()->showMessage("Disconnected.", 3000);
}

void MainWindow::on_exit_pushButton_clicked()
{
	Disconnect();
	exit(0);
}

void MainWindow::on_get_status_pushButton_clicked()
{
	std::string request = "get_status ";
	int i = 0;
	for (const auto& it : active_channels) {
		if (i++ > 0) request += ", ";
		request += it.channel_name;
	}
	socket->write(request.c_str(), request.size());

	char buff[BUFFER_SIZE];
	memset(buff, '\0', sizeof(char));
	if (socket->waitForReadyRead())	socket->read(buff, BUFFER_SIZE);
	std::vector<QString> data = SplitIntoWords(buff);

	std::cerr << "data.size() == " << data.size() << std::endl;
	i = 0;
	statusBar()->showMessage(data[i++], 3000);
	for (auto& channel : active_channels) {
		channel.status->setText(data[i++]);
	}
	socket->flush();
}

void MainWindow::on_get_result_pushButton_clicked()
{
	std::string request = "get_result ";
	int i = 0;
	for (const auto& it : active_channels) {
		if (i++ > 0) request += ", ";
		request += it.channel_name;
	}
	socket->write(request.c_str(), request.size());

	char buff[BUFFER_SIZE];
	memset(buff, '\0', sizeof(char));
	if (socket->waitForReadyRead())	socket->read(buff, BUFFER_SIZE);
	std::vector<QString> data = SplitIntoWords(buff);

	std::cerr << "data.size() == " << data.size() << std::endl;

	i = 0;
	statusBar()->showMessage(data[i++], 3000);
	for (auto& channel : active_channels) {
		channel.voltage->setText(data[i++]);
	}
	socket->flush();
}

void MainWindow::SetRange(const int channel, const int range) {
	std::string request = "set_range channel" + std::to_string(channel) + ", range" + std::to_string(range);
	socket->write(request.c_str(), request.size());
	Channel ch(channel);
	auto it = active_channels.find(ch);
	if (it != end(active_channels)) {
		auto node = active_channels.extract(it);
		node.value().range = range;
		active_channels.insert(std::move(node));
	}

	char buff[BUFFER_SIZE];
	memset(buff, '\0', sizeof(char));
	if (socket->waitForReadyRead())	socket->read(buff, BUFFER_SIZE);
	std::vector<QString> data = SplitIntoWords(buff);
	statusBar()->showMessage(data[0], 3000);
	socket->flush();
}

void MainWindow::on_channel0_range0_radioButton_clicked() { SetRange(0, 0); }
void MainWindow::on_channel0_range1_radioButton_clicked() {	SetRange(0, 1); }
void MainWindow::on_channel0_range2_radioButton_clicked() {	SetRange(0, 2); }
void MainWindow::on_channel0_range3_radioButton_clicked() {	SetRange(0, 3); }

void MainWindow::on_channel1_range0_radioButton_clicked() {	SetRange(1, 0); }
void MainWindow::on_channel1_range1_radioButton_clicked() {	SetRange(1, 1); }
void MainWindow::on_channel1_range2_radioButton_clicked() {	SetRange(1, 2); }
void MainWindow::on_channel1_range3_radioButton_clicked() {	SetRange(1, 3); }

void MainWindow::on_channel2_range0_radioButton_clicked() {	SetRange(2, 0); }
void MainWindow::on_channel2_range1_radioButton_clicked() {	SetRange(2, 1); }
void MainWindow::on_channel2_range2_radioButton_clicked() {	SetRange(2, 2); }
void MainWindow::on_channel2_range3_radioButton_clicked() {	SetRange(2, 3); }

void MainWindow::on_channel3_range0_radioButton_clicked() {	SetRange(3, 0); }
void MainWindow::on_channel3_range1_radioButton_clicked() {	SetRange(3, 1); }
void MainWindow::on_channel3_range2_radioButton_clicked() {	SetRange(3, 2); }
void MainWindow::on_channel3_range3_radioButton_clicked() {	SetRange(3, 3); }

void MainWindow::on_start_measure_pushButton_clicked()
{
	std::string request = "start_measure ";
	int i = 0;
	for (const auto& it : active_channels) {
		if (i++ > 0) request += ", ";
		request += it.channel_name;
	}
	socket->write(request.c_str(), request.size());

	char buff[BUFFER_SIZE];
	memset(buff, '\0', sizeof(char));
	if (socket->waitForReadyRead())	socket->read(buff, BUFFER_SIZE);
	std::vector<QString> data = SplitIntoWords(buff);

	std::cerr << "data.size() == " << data.size() << std::endl;

	statusBar()->showMessage(data[0], 3000);
	socket->flush();
}

void MainWindow::on_stop_measure_pushButton_clicked()
{
	std::string request = "stop_measure ";
	int i = 0;
	for (const auto& it : active_channels) {
		if (i++ > 0) request += ", ";
		request += it.channel_name;
	}
	socket->write(request.c_str(), request.size());

	char buff[BUFFER_SIZE];
	memset(buff, '\0', sizeof(char));
	if (socket->waitForReadyRead())	socket->read(buff, BUFFER_SIZE);
	std::vector<QString> data = SplitIntoWords(buff);

	std::cerr << "data.size() == " << data.size() << std::endl;

	statusBar()->showMessage(data[0], 3000);
	socket->flush();
}

