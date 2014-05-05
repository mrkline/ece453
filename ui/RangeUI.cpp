#include "RangeUI.hpp"
#include "ui_RangeUI.h"

#include <QMessageBox>

#include "StatusMessage.hpp"
#include "StatusResponseMessage.hpp"
#include "ExitMessage.hpp"
#include "TCPMessageBridge.hpp"
#include "MemoryUtils.hpp"

using namespace std;

inline QString fromStd(const std::string& s) { return QString::fromStdString(s); }


RangeUI::RangeUI(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::RangeUI)
{
	ui->setupUi(this);
	// Have the checkboxes for win conditions enable and disable
	// their corresponding spin boxes and labels
	connect(ui->chkTime, &QCheckBox::stateChanged, [this](int checked) {
		ui->spnTime->setEnabled(checked == Qt::Checked);
		ui->lblTime->setEnabled(checked == Qt::Checked);
	});
	connect(ui->chkScore, &QCheckBox::stateChanged, [this](int checked) {
		ui->spnScore->setEnabled(checked == Qt::Checked);
		ui->lblScore->setEnabled(checked == Qt::Checked);
	});

	connect(ui->btnConnect, &QPushButton::clicked, [this](bool) {
		connectToSM();
	});
	connect(ui->btnStatus, &QPushButton::clicked, [this](bool) {
		getStatus();
	});
}

RangeUI::~RangeUI()
{
	delete ui;
	closeConnection();
}

void RangeUI::closeConnection()
{
	if (commsThread != nullptr) {
		toSM.prioritySend(unique_ptr<Message>(new ExitMessage(++uid)));
		commsThread->join();
		fromSM.reset();
		commsThread.reset();
	}
}

void RangeUI::connectToSM()
{
	closeConnection();
	commsThread.reset(new std::thread(&runTCPMessageClient, ref(toSM), ref(fromSM),
	                                  ui->leEndPoint->text().toStdString()));
}

void RangeUI::getStatus()
{
	toSM.send(unique_ptr<Message>(new StatusMessage(++uid)));
	auto msg = fromSM.receive();

	auto srm = unique_dynamic_cast<StatusResponseMessage>(std::move(msg));

	if (srm == nullptr)
		QMessageBox::critical(this, "Wrong Message", "Get Status got the wrong response.");

	ui->txtTerminal->append(fromStd(jWriter.write(srm->toJSON())));
}
