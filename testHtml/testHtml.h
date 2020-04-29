#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_testHtml.h"

class testHtml : public QMainWindow
{
	Q_OBJECT

public:
	testHtml(QWidget *parent = Q_NULLPTR);

private:
	Ui::testHtmlClass ui;
};
