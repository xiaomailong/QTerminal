#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QVariant>
#include <QDockWidget>

#define VERSION    "1.0.0"

#include "NewSession/NewSession.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tabWidget->removeTab(0);
    ui->tabWidget->removeTab(0);
    ui->dockWidget->setWidget(ui->twProject);

    loadSession();    
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadSession()
{
    SesList sl;

    prjfile.Load("qstprj.xml");
    prjfile.GetSessionList(sl);

    for (int i = 0; i < sl.count(); i ++)
    {
        Session s;

        s = sl.at(i);
        addSession(s, false);
    }
}

void MainWindow::about(void)
{
    QMessageBox mbox;

    mbox.about(this, tr("关于"), tr("版本号: " VERSION));
}

void MainWindow::on_new_s_triggered()
{
    NewSession *ns = new NewSession;

    ns->show();
    ns->exec();

    if (ns->result() == 1)
    {
        Session set;

        ns->getSetting(set);

        addSession(set);
    }

    delete ns;
}

void MainWindow::addSession(Session &set, bool save)
{
    QTreeWidgetItem *child;
    QWidget *w;

    child = addSessionProject(set);

    w = addSessionWindow(set, child);
    if (w != NULL)
    {
        QVariant var;

        var.setValue(w);
        ui->tabWidget->addTab(w, set.name);
        child->setData(0, Qt::UserRole, var);

        var.setValue(set.id);
        child->setData(1, Qt::UserRole, var);

        if (save)
        {
            prjfile.AddSession(set);
            prjfile.Save();
        }
    }
}

QTreeWidgetItem* MainWindow::addSessionProject(Session &set)
{
    QTreeWidgetItem *type = new QTreeWidgetItem;
    bool addtype = true;

    for (int i = 0; i < ui->twProject->topLevelItemCount(); i ++)
    {
        QTreeWidgetItem *tmptype = ui->twProject->topLevelItem(i);

        if (tmptype->text(0) == set.type)
        {
            delete type;
            type = tmptype;
            addtype = false;
            break;
        }
    }

    type->setText(0, set.type);
    if (addtype)
    {
        QVariant var(0);

        type->setData(0, Qt::UserRole, var);
        ui->twProject->addTopLevelItem(type);
    }

    QTreeWidgetItem *child = new QTreeWidgetItem;
    child->setText(0, set.name);
    type->addChild(child);

    return child;
}

#include "Serial/SerialTerm.h"
#include "Telnet/TelnetTerm.h"

QWidget* MainWindow::addSessionWindow(Session &set, QTreeWidgetItem *item)
{
    QWidget *w = NULL;

    if (set.type == "串口终端")
    {
        SerialTerm *term = new SerialTerm;

        term->setSettings(set.param, set.id);
        w = term;
    }

    if (set.type == "telnet")
    {
        TelnetTerm *term = new TelnetTerm;

        term->setSettings(set.param);
        w = term;
    }

    return w;
}

void MainWindow::on_twProject_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    QWidget *w;
    QVariant var;

    var = item->data(column, Qt::UserRole);
    if (var == 0)
        return;

    w = var.value<QWidget*>();
    if (ui->tabWidget->indexOf(w) >= 0)
    {
        ui->tabWidget->setCurrentWidget(w);
    }
    else
    {
        ui->tabWidget->addTab(w, item->text(0));
    }
}

void MainWindow::on_twProject_customContextMenuRequested(const QPoint &pos)
{
    QTreeWidgetItem* curItem;

    curItem = ui->twProject->itemAt(pos);
    if (curItem == NULL)
        return;

    QVariant var = curItem->data(0, Qt::UserRole);
    if (var == 0)
        return;

    QMenu *popMenu =new QMenu(this);

    popMenu->addAction(ui->del_s);//往菜单内添加QAction   该action在前面用设计器定义了
    popMenu->exec(QCursor::pos());

    delete popMenu;
}

void MainWindow::on_del_s_triggered()
{
    QTreeWidgetItem* curItem, *top;
    QVariant var;
    QString id;
    QWidget* w;

    curItem = ui->twProject->currentItem();
    if (curItem == NULL)
        return;
    var = curItem->data(0, Qt::UserRole);
    if (var == 0)
        return;

    top = curItem->parent();

    var = curItem->data(1, Qt::UserRole);
    id = var.value<QString>();
    prjfile.DelSession(id);

    var = curItem->data(0, Qt::UserRole);
    w = var.value<QWidget*>();
    top->removeChild(curItem);

    ui->tabWidget->removeTab(ui->tabWidget->indexOf(w));

    delete w;
    delete curItem;
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    ui->tabWidget->removeTab(index);
}
