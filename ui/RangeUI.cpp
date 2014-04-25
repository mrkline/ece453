#include "RangeUI.hpp"
#include "ui_RangeUI.h"

using namespace std;

RangeUI::RangeUI(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::RangeUI)
{
	ui->setupUi(this);
}

RangeUI::~RangeUI()
{
	delete ui;
}
