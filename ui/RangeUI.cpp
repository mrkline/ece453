#include "RangeUI.hpp"
#include "ui_RangeUI.h"

using namespace std;



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
}

RangeUI::~RangeUI()
{
	delete ui;
}
