#include "RangeUI.hpp"
#include "ui_RangeUI.h"

#include <cassert>
#include <chrono>

#include <QMessageBox>

#include "StatusMessage.hpp"
#include "StatusResponseMessage.hpp"
#include "ResultsMessage.hpp"
#include "ResultsResponseMessage.hpp"
#include "StartMessage.hpp"
#include "StopMessage.hpp"
#include "SetupMessage.hpp"
#include "ExitMessage.hpp"
#include "TCPMessageBridge.hpp"
#include "MemoryUtils.hpp"

using namespace std;

namespace {

const std::chrono::seconds patienceWithGame(5);

} // end anonymous namespace

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

	connect(ui->btnConnect, &QPushButton::clicked, [this](bool) { connectToSM(); });
	connect(ui->btnSetup, &QPushButton::clicked, [this](bool) { setup(); });
	connect(ui->btnStart, &QPushButton::clicked, [this](bool) { start(); });
	connect(ui->btnStop, &QPushButton::clicked, [this](bool) { stop(); });
	connect(ui->btnStatus, &QPushButton::clicked, [this](bool) { getStatus(); });
	connect(ui->btnResults, &QPushButton::clicked, [this](bool) { getResults(); });
}

RangeUI::~RangeUI()
{
	delete ui;
	closeConnection();
}

void RangeUI::connectToSM()
{
	closeConnection();
	commsThread = async(launch::async, &runTCPMessageClient, ref(toSM), ref(fromSM),
	                                   ui->leEndPoint->text().toStdString());
}

bool RangeUI::ensureConnection()
{
	if (!commsThread.valid()) {
		QMessageBox::critical(this, "Not connected",
		                      "You must connect before performing this operation");
		return false;
	}

	switch (commsThread.wait_for(chrono::seconds(0))) {
		case future_status::deferred:
			assert(0); // We're using async launching, so we shouldn't be deferred
			break;

		// If it's ready that means we've crashed
		case future_status::ready:
			try {
				commsThread.get();
				QMessageBox::critical(this, "Connection closed", "The game disconnected.");
			}
			catch (const exception& e) {
				QMessageBox::critical(this, "Connection error", e.what());
			}
			break;

		case future_status::timeout:
			// All good!
			return true;
	}

	return false;
}

void RangeUI::closeConnection()
{
	if (commsThread.valid()) {
		toSM.prioritySend(unique_ptr<Message>(new ExitMessage(++uid)));
		commsThread.get();
		fromSM.reset();
	}
}

std::unique_ptr<Message> RangeUI::awaitResponse(Message* toSend)
{
	if (!ensureConnection())
		return nullptr;

	toSM.send(std::unique_ptr<Message>(toSend));

	auto msg = fromSM.receive(patienceWithGame);

	if (msg == nullptr) {
		if (ensureConnection())
			QMessageBox::critical(this, "Unresponsive game", "The game is not responding.");
	}

	return msg;
}

void RangeUI::setup()
{
	auto msg = awaitResponse(new SetupMessage(++uid, GameType::POP_UP,
	                         (board_id_t)ui->spnPlayers->value(),
	                         ui->chkTime->isChecked() ? ui->spnTime->value() : -1,
	                         ui->chkScore->isChecked() ? ui->spnScore->value() : -1));

	if (msg == nullptr)
		return;

	ui->txtTerminal->append(fromStd(jWriter.write(msg->toJSON())));

	auto rm = unique_dynamic_cast<ResponseMessage>(std::move(msg));

	if (rm == nullptr)
		QMessageBox::critical(this, "Wrong Message", "Setup got the wrong response.");
}

void RangeUI::start()
{
	auto msg = awaitResponse(new StartMessage(++uid));

	if (msg == nullptr)
		return;

	ui->txtTerminal->append(fromStd(jWriter.write(msg->toJSON())));

	auto rm = unique_dynamic_cast<ResponseMessage>(std::move(msg));

	if (rm == nullptr)
		QMessageBox::critical(this, "Wrong Message", "Start got the wrong response.");
}

void RangeUI::stop()
{
	auto msg = awaitResponse(new StopMessage(++uid));

	if (msg == nullptr)
		return;

	ui->txtTerminal->append(fromStd(jWriter.write(msg->toJSON())));

	auto rm = unique_dynamic_cast<ResponseMessage>(std::move(msg));

	if (rm == nullptr)
		QMessageBox::critical(this, "Wrong Message", "Stop got the wrong response.");
}

void RangeUI::getStatus()
{
	auto msg = awaitResponse(new StatusMessage(++uid));

	if (msg == nullptr)
		return;

	ui->txtTerminal->append(fromStd(jWriter.write(msg->toJSON())));

	auto srm = unique_dynamic_cast<StatusResponseMessage>(std::move(msg));

	if (srm == nullptr)
		QMessageBox::critical(this, "Wrong Message", "Get Status got the wrong response.");
}


void RangeUI::getResults()
{
	auto msg = awaitResponse(new ResultsMessage(++uid));

	if (msg == nullptr)
		return;

	ui->txtTerminal->append(fromStd(jWriter.write(msg->toJSON())));

	auto rrm = unique_dynamic_cast<ResponseMessage>(std::move(msg));

	if (rrm == nullptr)
		QMessageBox::critical(this, "Wrong Message", "Get Results got the wrong response.");
}
