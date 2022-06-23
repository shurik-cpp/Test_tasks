#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLabel>
#include <set>
#include <vector>
#include <iostream>

#include "parser.h"

#include <QLocalSocket>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = nullptr);
	~MainWindow();

private slots:

	void on_channel0_checkBox_stateChanged();
	void on_channel1_checkBox_stateChanged();
	void on_channel2_checkBox_stateChanged();
	void on_channel3_checkBox_stateChanged();

	void on_connect_pushButton_clicked();

	void on_disconnect_pushButton_clicked();

	void on_exit_pushButton_clicked();

	void on_get_status_pushButton_clicked();

	void on_get_result_pushButton_clicked();

	void on_channel0_range0_radioButton_clicked();
	void on_channel0_range1_radioButton_clicked();
	void on_channel0_range2_radioButton_clicked();
	void on_channel0_range3_radioButton_clicked();

	void on_channel1_range0_radioButton_clicked();
	void on_channel1_range1_radioButton_clicked();
	void on_channel1_range2_radioButton_clicked();
	void on_channel1_range3_radioButton_clicked();

	void on_channel2_range0_radioButton_clicked();
	void on_channel2_range1_radioButton_clicked();
	void on_channel2_range2_radioButton_clicked();
	void on_channel2_range3_radioButton_clicked();

	void on_channel3_range0_radioButton_clicked();
	void on_channel3_range1_radioButton_clicked();
	void on_channel3_range2_radioButton_clicked();
	void on_channel3_range3_radioButton_clicked();



	void on_start_measure_pushButton_clicked();

	void on_stop_measure_pushButton_clicked();

private:
	Ui::MainWindow *ui;
	const qint64 BUFFER_SIZE = 8192;
	QLocalSocket* socket;
	QString socket_path;

	bool Connect();
	bool Disconnect();
	void ActivateButtons(const bool is_active);
	void SetBoxStatus(QGroupBox* gb1, QGroupBox* gb2, const QCheckBox* cb);
	void AddRemoveChannel(const bool is_add, const int channel_number, QLabel* voltage_lable, QLabel* status_lable);
	void SetRange(const int channel, const int range);

	struct Channel {
		Channel(const int n, QLabel* voltage_lable = nullptr, QLabel* status_lable = nullptr)
			: number(n)
			, channel_name("channel")
			, voltage(voltage_lable)
			, status(status_lable) {
			channel_name += std::to_string(n);
		}
		int number;
		int range = 1;
		std::string channel_name;
		QLabel* voltage;
		QLabel* status;

		bool operator< (const Channel& rhs) const { return this->number < rhs.number; }
	};

	std::set<Channel> active_channels;

};
#endif // MAINWINDOW_H
