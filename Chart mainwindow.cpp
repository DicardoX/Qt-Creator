#include "pieview.h"
#include "mainwindow.h"

#include <QtWidgets>

QColor item[17] = { Qt::blue, Qt::red, Qt::green, Qt::yellow, Qt::gray, Qt::cyan, Qt::magenta, Qt::black, Qt::white,
                    Qt::darkGreen, Qt::darkBlue, Qt::darkRed, Qt::darkYellow, Qt::darkGray, Qt::darkCyan, Qt::magenta, Qt::lightGray,
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QMenu *fileMenu = new QMenu(tr("&File"), this);
    QAction *openAction = fileMenu->addAction(tr("&Open..."));
    openAction->setShortcuts(QKeySequence::Open);
    QAction *saveAction = fileMenu->addAction(tr("&Save As..."));
    saveAction->setShortcuts(QKeySequence::SaveAs);
    QAction *quitAction = fileMenu->addAction(tr("E&xit"));
    quitAction->setShortcuts(QKeySequence::Quit);

    setupModel();
    setupViews();

    connect(openAction, &QAction::triggered, this, &MainWindow::openFile);
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveFile);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    menuBar()->addMenu(fileMenu);
    statusBar();

    loadFile(":/Charts/test.csv");

    setWindowTitle(tr("Chart"));
    resize(870, 550);
}

void MainWindow::setupModel()
{
    model = new QStandardItemModel(8, 2, this);
    model->setHeaderData(0, Qt::Horizontal, tr("Country"));
    model->setHeaderData(1, Qt::Horizontal, tr("Population(1k)"));
}

void MainWindow::setupViews()
{
    QSplitter *splitter = new QSplitter;
    QTableView *table = new QTableView;
    pieChart = new PieView;
    splitter->addWidget(table);
    splitter->addWidget(pieChart);
    splitter->setStretchFactor(0, 0);
    splitter->setStretchFactor(1, 1);

    table->setModel(model);
    pieChart->setModel(model);

    QItemSelectionModel *selectionModel = new QItemSelectionModel(model);
    table->setSelectionModel(selectionModel);
    pieChart->setSelectionModel(selectionModel);

    QHeaderView *headerView = table->horizontalHeader();
    headerView->setStretchLastSection(true);

    setCentralWidget(splitter);
}

void MainWindow::openFile()
{
    const QString fileName =
        QFileDialog::getOpenFileName(this, tr("Choose a data file"), "", "*.csv");
    if (!fileName.isEmpty())
        loadFile(fileName);
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return;

    QTextStream stream(&file);

    model->removeRows(0, model->rowCount(QModelIndex()), QModelIndex());

    int row = 0;
    int color_choice = 0;
    bool title_existed = true;
    while (!stream.atEnd()) {
        const QString line = stream.readLine();
        if (!line.isEmpty()) {
            if(title_existed){               // To skip the title line.
                title_existed = false;
                continue;
            }
            model->insertRows(row, 1, QModelIndex());

            const QStringList pieces = line.split(',', QString::SkipEmptyParts);
            if (pieces.size() < 2) continue;

            model->setData(model->index(row, 0, QModelIndex()), pieces.value(0));
            model->setData(model->index(row, 1, QModelIndex()), pieces.value(1));
            model->setData(model->index(row, 0, QModelIndex()), QColor(item[color_choice]), Qt::DecorationRole);
            if(color_choice < 16)
                color_choice++;
            else
                color_choice = 0;
            row++;
        }
    }

    file.close();
    statusBar()->showMessage(tr("Loaded %1").arg(fileName), 2000);
}

void MainWindow::saveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Save file as"), "", "*.csv");

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text))
        return;

    QTextStream stream(&file);
    for (int row = 0; row < model->rowCount(QModelIndex()); ++row) {

        QStringList pieces;

        pieces.append(model->data(model->index(row, 0, QModelIndex()),
                                  Qt::DisplayRole).toString());
        pieces.append(model->data(model->index(row, 1, QModelIndex()),
                                  Qt::DisplayRole).toString());
        pieces.append(model->data(model->index(row, 0, QModelIndex()),
                                  Qt::DecorationRole).toString());

        stream << pieces.join(',') << "\n";
    }

    file.close();
    statusBar()->showMessage(tr("Saved %1").arg(fileName), 2000);
}