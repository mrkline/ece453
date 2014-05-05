#include "RangeUI.hpp"
#include "ui_RangeUI.h"

#include <cassert>
#include <chrono>

#include <QMessageBox>

#include "StatusMessage.hpp"
#include "StatusResponseMessage.hpp"
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
				assert(0); // We shouldn't get here
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

void RangeUI::getStatus()
{
	if (!ensureConnection())
		return;

	toSM.send(unique_ptr<Message>(new StatusMessage(++uid)));
	auto msg = fromSM.receive(chrono::seconds(patienceWithGame));

	if (msg == nullptr) {
		// Hmm, we didn't get a message back. Check the connection
		ensureConnection();
		return;
	}

	auto srm = unique_dynamic_cast<StatusResponseMessage>(std::move(msg));

	if (srm == nullptr)
		QMessageBox::critical(this, "Wrong Message", "Get Status got the wrong response.");

	ui->txtTerminal->append(fromStd(jWriter.write(srm->toJSON())));
}
