#include "MainWindow.h"
#include <QButtonGroup>

MainWindow::MainWindow(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    QButtonGroup* buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui.btnList);
    buttonGroup->addButton(ui.btnAllFun);
    buttonGroup->addButton(ui.btnInscFun);
    buttonGroup->addButton(ui.btnCollFun);
    buttonGroup->addButton(ui.btnAllIt);
    buttonGroup->addButton(ui.btnInscIt);
    buttonGroup->addButton(ui.btnCollIt);
    buttonGroup->setExclusive(true);
}

MainWindow::~MainWindow()
{}
