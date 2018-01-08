#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QGraphicsScene>
#include <QVector>
#include <QImage>
#include <QRgb>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QModelIndex>
#include <QTextStream>
#include <QMessageBox>
#include <math.h>
#include <QDebug>
#include <QTextStream>
#include <QColorDialog>
#include <QtAlgorithms>

double PI = 3.1415926526;
int sortN = 18;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setFixedSize(this->size());
    //инициализация сцены
    int width = ui->graphicsView->geometry().width();
    int height = ui->graphicsView->geometry().height();
    scene = new QGraphicsScene(QRectF(0, 0, width, height), 0);
    ui->graphicsView->fitInView(QRectF(0, 0, width, height),
                    Qt::KeepAspectRatio);
    ui->graphicsView->setScene(scene);
    //очистка переменных
    kntFileName = "";
    inputPath = "";
    selectedRow = 0;
    selectedColumn = 0;
    nSquares.clear();
    polygonList.clear();
    cntList.clear();
    pdata.clear();
    for (int i=0; i<18; i++)
    {
        priznaks[i] = "";
    }
    for (int i=0; i<21; i++)
    {
        dPriznaks[i] = 0;
    }
    sortN = 18;
    selSquare = 0;
    ui->btnCalculate->setVisible(false);
    selcolor = QColor(255,0,0);
    ui->selColor->setStyleSheet("QLabel { background-color : #ff0000;}");
    alphaSelcolor = QColor(255,255,255);
    ui->selColor_2->setStyleSheet("QLabel { background-color : #ffffff;}");
    QPixmap ascPixmap;
    QPixmap descPixmap;
    ascPixmap.load("asc.png");
    descPixmap.load("desc.png");

    ui->img_asc->setPixmap(ascPixmap);
    ui->img_desc->setPixmap(descPixmap);
    ui->img_asc->setScaledContents(true);
    ui->img_desc->setScaledContents(true);

    ui->spinAlpha->setEnabled(false);
    ui->btnSelColor_2->setEnabled(false);
    connect(ui->graphicsView, SIGNAL (clickEvent(int, int)), this, SLOT(viewClick(int, int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}
//загрузка исходного изображения(ий)
void MainWindow::on_btnLoadImage_clicked()
{
    QString vName = QFileDialog::getOpenFileName();
    //загрузить изображение
    pm.load (vName);
    pPixmapItem = new QGraphicsPixmapItem(pm);
    //добавить на сцену
    //scene->addItem (pPixmapItem);
    scene->addPixmap(pm.scaled(QSize(
                    (int)scene->width(), (int)scene->height()),
                    Qt::KeepAspectRatio, Qt::SmoothTransformation));
    pPixmapItem ->setFlags(QGraphicsItem::ItemIsMovable);
    //взять текущее изображение
    inputImage = pPixmapItem->pixmap().toImage();
    //взять текущий путь
    inputPath = vName;
    //очистить изо контуров
    kntFileName = vName.mid(0,vName.lastIndexOf(".")) + ".dat";
    //загрузка файла контуров
    QFile* file = new QFile(kntFileName);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug()<<tr("Невозможно открыть файл");
        return;
    }
    QTextStream in(file);
    int counter = 0;
    while(!in.atEnd()) {
        int state = 1;
        QString line = in.readLine();
        QStringList fields = line.split(".");
        //nSquares<<fields.at(0);
        QPolygon poly;
        QPolygon cnt;
        bool end = true;
        QList<int> points;
        for(int i = 0; i<fields.length(); i++)
        {
            if(state==1 && end) {
                nSquares<<fields.at(i);
                end = false;
                continue;
            }
            if(fields.at(i)=="C") {
                polygonList<<poly;
                ui->listWidget->addItem(QString::number(counter));
                counter++;
                poly.clear();
                state = 2;
                continue;
            }
            if(fields.at(i)==";") {
                cntList<<cnt;
                cnt.clear();
                state = 1;
                end = true;
                continue;
            }
            if(fields.at(i).contains(";"))
            {
                cntList<<cnt;
                cnt.clear();
                state = 1;
                QString var= fields.at(i);
                nSquares<<var.replace(";","").trimmed();
                end = false;
                continue;
            }
            points<<fields.at(i).toInt();
            if(points.size()==2)
            {
                QPoint p(points.at(0), points.at(1));
                if(state==1)poly.append(p);
                if(state==2)cnt.append(p);
                points.clear();
            }
        }
        //polygonList<<poly;
        //cntList<<cnt;
    }
    file->close();
    //calculate_alpha(false);
    //sort();
    QMessageBox msg;
    msg.setText(tr("Выполнено!"));
    msg.exec();
}

//очистка
void MainWindow::on_btnClear_clicked()
{
    ui->graphicsView->scene()->clear();
    kntFileName = "";
    selectedRow = 0;
    selectedColumn = 0;
    nSquares.clear();
    polygonList.clear();
    cntList.clear();

    for (int i=0; i<18; i++)
    {
        priznaks[i] = "";
    }
    for (int i=0; i<21; i++)
    {
        dPriznaks[i] = 0;
    }
    ui->txtParamValue->setText("");
    selSquare = 0;
    ui->listWidget->clear();
}
//уведичить масштаб
void MainWindow::on_btnIncrease_clicked()
{
    ui->graphicsView->scale(1.1,1.1);
}
//уменьшить масштаб
void MainWindow::on_btnDecrease_clicked()
{
    ui->graphicsView->scale(1/1.1,1/1.1);
}
//сохранить текущие результаты
void MainWindow::on_btnSaveRezults_clicked()
{
    QString saveFileName = QFileDialog::getSaveFileName();
    //сохранение в текстовый файл
    saveTextFile(saveFileName);
}
//сохранение текстового файла
void MainWindow::saveTextFile(QString saveFileName)
{
    QFile file( saveFileName );
    if ( file.open(QIODevice::ReadWrite) )
    {
        QTextStream stream( &file );
        stream << tr("Средняя яркость R: ") <<priznaks[0]<< "\r\n";
        stream << tr("Средняя яркость G: ") <<priznaks[1]<< "\r\n";
        stream << tr("Средняя яркость B: ") <<priznaks[2]<< "\r\n";
        stream << tr("СКО R: ") <<priznaks[3]<< "\r\n";
        stream << tr("СКО G: ") <<priznaks[4]<< "\r\n";
        stream << tr("СКО B: ") <<priznaks[5]<< "\r\n";
        stream << tr("Диапазон R: ") <<priznaks[6]<< "\r\n";
        stream << tr("Диапазон G: ") <<priznaks[7]<< "\r\n";
        stream << tr("Диапазон B: ") <<priznaks[8]<< "\r\n";
        stream << tr("Текстур призн МПС R: ") <<priznaks[9]<< "\r\n";
        stream << tr("Текстур призн МПС G: ") <<priznaks[10]<< "\r\n";
        stream << tr("Текстур призн МПС B: ") <<priznaks[11]<< "\r\n";
        stream << tr("Тек пр длины серий R: ") <<priznaks[12]<< "\r\n";
        stream << tr("Тек пр длины серий G: ") <<priznaks[13]<< "\r\n";
        stream << tr("Тек пр длины серий B: ") <<priznaks[14]<< "\r\n";
        stream << tr("Форма: \r\n\r\n") <<priznaks[15]<< "\r\n";
        stream << tr("Площадь: \r\n\r\n") <<priznaks[16]<< "\r\n";
        stream << tr("Периметр: \r\n\r\n") <<priznaks[17]<< "\r\n";

    }
}
//обновление окна признаков
void MainWindow::updatePrizn()
{
    QString txtValue = "";
    txtValue += tr("Средняя яркость R: ") + priznaks[0] + "\r\n";
    txtValue += tr("Средняя яркость G: ") + priznaks[1] + "\r\n";
    txtValue += tr("Средняя яркость B: ") + priznaks[2] + "\r\n";
    txtValue += tr("СКО R: ") + priznaks[3] + "\r\n";
    txtValue += tr("СКО G: ") + priznaks[4] + "\r\n";
    txtValue += tr("СКО B: ") + priznaks[5] + "\r\n";
    txtValue += tr("Диапазон R: ") + priznaks[6] + "\r\n";
    txtValue += tr("Диапазон G: ") + priznaks[7] + "\r\n";
    txtValue += tr("Диапазон B: ") + priznaks[8] + "\r\n";
    txtValue += tr("Текстур призн МПС R: ") + priznaks[9] + "\r\n";
    txtValue += tr("Текстур призн МПС G: ") + priznaks[10] + "\r\n";
    txtValue += tr("Текстур призн МПС B: ") + priznaks[11] + "\r\n";
    txtValue += tr("Тек пр длины серий R: ") + priznaks[12] + "\r\n";
    txtValue += tr("Тек пр длины серий G: ") + priznaks[13] + "\r\n";
    txtValue += tr("Тек пр длины серий B: ") + priznaks[14] + "\r\n";
    txtValue += tr("Форма: ") + priznaks[15] + "\r\n";
    txtValue += tr("Площадь: ") + priznaks[16] + "\r\n";
    txtValue += tr("Периметр: ") + priznaks[17] + "\r\n";
    ui->txtParamValue->setText(txtValue);
}
//вычисление параметров
void MainWindow::paramsRGB() {
    //компоненты цвета
    int valueR = 0;
    int valueG = 0;
    int valueB = 0;
    //диапазоны
    int minR = 255;
    int maxR = 0;
    int minG = 255;
    int maxG = 0;
    int minB = 255;
    int maxB = 0;
    //длины серий
    double dsR = 0;
    double dsG = 0;
    double dsB = 0;
    //текстурные признаки
    double texR = 0;
    double texG = 0;
    double texB = 0;

    QPolygon poly = polygonList.at(selSquare);
    for (int x=0; x<poly.length(); x++)
    {
        QPoint p = poly.at(x);
        if(p.x()<inputImage.width() && p.y()<inputImage.height())
        {

            //получение компонент
            QRgb color = inputImage.pixel(p.x(),p.y());
            valueR = qRed(color);
            valueG = qGreen(color);
            valueB = qBlue(color);
            dPriznaks[0] += valueR;
            dPriznaks[1] += valueG;
            dPriznaks[2] += valueB;
            //определение диапазонов
            if(valueR<minR)
            {
                minR = valueR;
            }
            if(valueR>maxR)
            {
                maxR = valueR;
            }
            if(valueG<minG)
            {
                minG = valueG;
            }
            if(valueG>maxG)
            {
                maxG = valueG;
            }
            if(valueB<minB)
            {
                minB = valueB;
            }
            if(valueB>maxB)
            {
                maxB = valueB;
            }
            //длины серий (энтропия)
            dsR += (double)valueR*log((double)valueR);
            dsG += (double)valueG*log((double)valueG);
            dsB += (double)valueB*log((double)valueB);
            //текстурный признак - энергия
            texR += pow((double)valueR,2.0f);
            texG += pow((double)valueG,2.0f);
            texB += pow((double)valueB,2.0f);
        }
    }
    //средняя яркость
    dPriznaks[0] = (double)dPriznaks[0] / ((double)poly.length());
    dPriznaks[1] = (double)dPriznaks[1] / ((double)poly.length());
    dPriznaks[2] = (double)dPriznaks[2] / ((double)poly.length());
    //диапазоны
    dPriznaks[6] = minR;
    dPriznaks[7] = maxR;
    dPriznaks[8] = minG;
    dPriznaks[9] = maxG;
    dPriznaks[10] = minB;
    dPriznaks[11] = maxB;
    //текстурные признаки
    dPriznaks[12] = texR / ((double)poly.length());
    dPriznaks[13] = texG / ((double)poly.length());
    dPriznaks[14] = texB / ((double)poly.length());
    //длины серий
    dPriznaks[15] = dsR / ((double)poly.length());
    dPriznaks[16] = dsG / ((double)poly.length());
    dPriznaks[17] = dsB / ((double)poly.length());
}
//вычисление СКО
void MainWindow::SKORGB() {
    int valueR = 0;
    int valueG = 0;
    int valueB = 0;
    double skoR = 0;
    double skoG = 0;
    double skoB = 0;
    QPolygon poly = polygonList.at(selSquare);
    for (int x=0; x<poly.length(); x++)
    {
        QPoint p = poly.at(x);
        if(p.x()<inputImage.width() && p.y()<inputImage.height())
        {
            QRgb color = inputImage.pixel(p.x(),p.y());
            valueR = qRed(color);
            valueG = qGreen(color);
            valueB = qBlue(color);
            skoR += pow(valueR-dPriznaks[0],2);
            skoG += pow(valueR-dPriznaks[1],2);
            skoB += pow(valueR-dPriznaks[2],2);
        }
    }
    //СКО
    dPriznaks[3] = sqrt(skoR/((double)poly.length()));
    dPriznaks[4] = sqrt(skoG/((double)poly.length()));
    dPriznaks[5] = sqrt(skoB/((double)poly.length()));
}

//кнопка выполнить
void MainWindow::on_btnCalculate_clicked()
{
    if(kntFileName.trimmed().length()==0)
    {
        QMessageBox msg2;
        msg2.setText(tr("Не выбран файл контуров!"));
        msg2.exec();
        return;
    }
    if(inputPath.trimmed().length()==0)
    {
        QMessageBox msg2;
        msg2.setText(tr("Не выбран файл изображения!"));
        msg2.exec();
        return;
    }
    //вычисление параметров
    paramsRGB();
    //вычисление СКО
    SKORGB();
    //сохранение признаков
    priznaks[0] = QString::number(dPriznaks[0]);
    priznaks[1] = QString::number(dPriznaks[1]);
    priznaks[2] = QString::number(dPriznaks[2]);
    priznaks[3] = QString::number(dPriznaks[3]);
    priznaks[4] = QString::number(dPriznaks[4]);
    priznaks[5] = QString::number(dPriznaks[5]);
    priznaks[6] = QString::number(dPriznaks[6]) + " - " + QString::number(dPriznaks[7]);
    priznaks[7] = QString::number(dPriznaks[8]) + " - " + QString::number(dPriznaks[9]);
    priznaks[8] = QString::number(dPriznaks[10]) + " - " + QString::number(dPriznaks[11]);
    priznaks[9] = QString::number(dPriznaks[12]);
    priznaks[10] = QString::number(dPriznaks[13]);
    priznaks[11] = QString::number(dPriznaks[14]);
    priznaks[12] = QString::number(dPriznaks[15]);
    priznaks[13] = QString::number(dPriznaks[16]);
    priznaks[14] = QString::number(dPriznaks[17]);
}

void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    selSquare = item->text().toInt();
    calculate(selSquare);
}

void MainWindow::on_btnSelColor_clicked()
{
    selcolor = QColorDialog::getColor(Qt::red, this );
    ui->selColor->setStyleSheet("QLabel { background-color : " + selcolor.name() +";}");
}

void MainWindow::on_isAlpha_clicked()
{
    bool isAlpha = ui->isAlpha->isChecked();
    ui->spinAlpha->setEnabled(isAlpha);
    ui->btnSelColor_2->setEnabled(isAlpha);
    if(isAlpha)
    {
        calculate_alpha();
    }
    else
    {
        QImage image = inputImage.copy();
        scene->clear();
        pPixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(image));
        //добавить на сцену
        //scene->addItem (pPixmapItem);
        scene->addPixmap(QPixmap::fromImage(image).scaled(QSize(
                        (int)scene->width(), (int)scene->height()),
                        Qt::KeepAspectRatio, Qt::SmoothTransformation));

        pPixmapItem ->setFlags(QGraphicsItem::ItemIsMovable);
    }
}

void MainWindow::on_spinAlpha_valueChanged(int arg1)
{

}

void MainWindow::on_btnSelColor_2_clicked()
{
    alphaSelcolor = QColorDialog::getColor(Qt::white, this );
    ui->selColor_2->setStyleSheet("QLabel { background-color : " + alphaSelcolor.name() +";}");
}

void MainWindow::calculate(int n)
{
    for (int i=0; i<21; i++)
    {
        dPriznaks[i] = 0;
    }
    QImage image;
    image = inputImage.copy();
    priznaks[15] = tr("Множество овалов");
    dPriznaks[19] = 0;
    dPriznaks[20] = 0;
    QPolygon poly = polygonList.at(n);
    QPolygon cnt = cntList.at(n);

    QImage cutout(image.width(),image.height(), QImage::Format_ARGB32_Premultiplied);
    QColor a1 = alphaSelcolor;
    a1.setAlpha(ui->spinAlpha->value());
    cutout.fill(a1);//transparent
    QPainter p;
    if(ui->isAlpha->isChecked())
    {
        p.begin(&image);
    }
    for (int x=0; x<poly.length(); x++)
    {
        QPoint p = poly.at(x);
        if(p.x()<image.width() && p.y()<image.height())
        {
            QRgb color = qRgb(selcolor.red(),selcolor.green(),selcolor.blue());
            if(ui->showSquare->isChecked())
            {
                image.setPixel(p.x(),p.y(),color);
                cutout.setPixel(p.x(),p.y(),color);
            }
            dPriznaks[19] ++;
        }
    }
    for (int x=0; x<cnt.length(); x++)
    {
        QPoint p = cnt.at(x);
        if(p.x()<image.width() && p.y()<image.height())
        {
            QRgb color = qRgb(selcolor.red(),selcolor.green(),selcolor.blue());
            if(ui->showContour->isChecked())
            {
                image.setPixel(p.x(),p.y(),color);
                cutout.setPixel(p.x(),p.y(),color);
            }
            dPriznaks[20]++;
        }
    }
    if(ui->isAlpha->isChecked())
    {
        p.drawImage(0,0, cutout);
        p.end();
    }

    priznaks[16] = QString::number(dPriznaks[19]);
    priznaks[17] = QString::number(dPriznaks[20]);

    scene->clear();
    pPixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(image));
    //добавить на сцену
    //scene->addItem (pPixmapItem);
    scene->addPixmap(QPixmap::fromImage(image).scaled(QSize(
                    (int)scene->width(), (int)scene->height()),
                    Qt::KeepAspectRatio, Qt::SmoothTransformation));

    pPixmapItem ->setFlags(QGraphicsItem::ItemIsMovable);
    on_btnCalculate_clicked();
    updatePrizn();
}

void MainWindow::calculate_alpha(bool isDraw)
{
    pdata.clear();
    QImage image;
    image = inputImage.copy();
    QImage cutout(image.width(),image.height(), QImage::Format_ARGB32_Premultiplied);
    QColor a1 = alphaSelcolor;
    a1.setAlpha(ui->spinAlpha->value());
    cutout.fill(a1);//transparent
    QPainter p;
    p.begin(&image);
    for(int n=0; n<polygonList.length(); n++)
    {
        for (int i=0; i<21; i++)
        {
            dPriznaks[i] = 0;
        }
        priznaks[15] = tr("Множество овалов");
        dPriznaks[19] = 0;
        dPriznaks[20] = 0;
        QPolygon poly = polygonList.at(n);
        QPolygon cnt = cntList.at(n);
        for (int x=0; x<poly.length(); x++)
        {
            QPoint p = poly.at(x);
            if(p.x()<image.width() && p.y()<image.height())
            {
                QRgb color = qRgb(selcolor.red(),selcolor.green(),selcolor.blue());
                if(ui->showSquare->isChecked())
                {
                    image.setPixel(p.x(),p.y(),color);
                    cutout.setPixel(p.x(),p.y(),color);
                }
                dPriznaks[19] ++;
            }
        }
        for (int x=0; x<cnt.length(); x++)
        {
            QPoint p = cnt.at(x);
            if(p.x()<image.width() && p.y()<image.height())
            {
                QRgb color = qRgb(selcolor.red(),selcolor.green(),selcolor.blue());
                if(ui->showContour->isChecked())
                {
                    image.setPixel(p.x(),p.y(),color);
                    cutout.setPixel(p.x(),p.y(),color);
                }
                dPriznaks[20]++;
            }
        }
        selSquare = n;
        on_btnCalculate_clicked();
        //
        filter f1;
        f1.n0 = n;
        f1.name = QString::number(n);
        for(int ii=0; ii<21;ii++)
        {
            f1.dPriznaks[ii] = dPriznaks[ii];
        }
        pdata<<f1;
    }
    p.drawImage(0,0, cutout);
    p.end();
    priznaks[16] = QString::number(dPriznaks[19]);
    priznaks[17] = QString::number(dPriznaks[20]);

    if(isDraw)
    {
        scene->clear();
        pPixmapItem = new QGraphicsPixmapItem(QPixmap::fromImage(image));
        //добавить на сцену
        //scene->addItem (pPixmapItem);
        scene->addPixmap(QPixmap::fromImage(image).scaled(QSize(
                        (int)scene->width(), (int)scene->height()),
                        Qt::KeepAspectRatio, Qt::SmoothTransformation));

        pPixmapItem ->setFlags(QGraphicsItem::ItemIsMovable);
    }
    //updatePrizn();
}

void MainWindow::on_ascBtn_clicked()
{
    sort();
}

void MainWindow::on_descBtn_clicked()
{
    sort();
}

void MainWindow::on_radioB1_clicked()
{
    sort();
}

void MainWindow::on_radioB2_clicked()
{
    sort();
}

void MainWindow::on_radioB3_clicked()
{
    sort();
}

void MainWindow::on_radioB4_clicked()
{
    sort();
}

void MainWindow::on_radioB5_clicked()
{
    sort();
}

void MainWindow::on_radioB6_clicked()
{
    sort();
}

void MainWindow::on_radioB7_clicked()
{
    sort();
}

void MainWindow::on_radioB8_clicked()
{
    sort();
}

void MainWindow::on_radioB9_clicked()
{
    sort();
}

void MainWindow::on_radioB10_clicked()
{
    sort();
}

void MainWindow::on_radioB11_clicked()
{
    sort();
}

void MainWindow::on_radioB12_clicked()
{
    sort();
}

void MainWindow::on_radioB13_clicked()
{
    sort();
}

void MainWindow::on_radioB14_clicked()
{
    sort();
}

void MainWindow::on_radioB15_clicked()
{
    sort();
}

void MainWindow::on_radioB16_clicked()
{
    sort();
}

void MainWindow::on_radioB17_clicked()
{
    sort();
}

void MainWindow::on_radioB18_clicked()
{
    sort();
}

static bool doubleLessThan(const filter &v1, const filter &v2)
{
    double vv1, vv2;
    switch (sortN)
    {
        case 0:
            vv1 = v1.dPriznaks[0];
            vv2 = v2.dPriznaks[0];
        break;
        case 1:
            vv1 = v1.dPriznaks[1];
            vv2 = v2.dPriznaks[1];
        break;
        case 2:
            vv1 = v1.dPriznaks[2];
            vv2 = v2.dPriznaks[2];
        break;
        case 3:
            vv1 = v1.dPriznaks[3];
            vv2 = v2.dPriznaks[3];
        break;
        case 4:
            vv1 = v1.dPriznaks[4];
            vv2 = v2.dPriznaks[4];
        break;
        case 5:
            vv1 = v1.dPriznaks[5];
            vv2 = v2.dPriznaks[5];
        break;
        case 6:
            vv1 = v1.dPriznaks[6];
            vv2 = v2.dPriznaks[6];
        break;
        case 8:
            vv1 = v1.dPriznaks[8];
            vv2 = v2.dPriznaks[8];
        break;
        case 10:
            vv1 = v1.dPriznaks[10];
            vv2 = v2.dPriznaks[10];
        break;
        case 12:
            vv1 = v1.dPriznaks[12];
            vv2 = v2.dPriznaks[12];
        break;
        case 13:
            vv1 = v1.dPriznaks[13];
            vv2 = v2.dPriznaks[13];
        break;
        case 14:
            vv1 = v1.dPriznaks[14];
            vv2 = v2.dPriznaks[14];
        break;
        case 15:
            vv1 = v1.dPriznaks[15];
            vv2 = v2.dPriznaks[15];
        break;
        case 16:
            vv1 = v1.dPriznaks[16];
            vv2 = v2.dPriznaks[16];
        break;
        case 17:
            vv1 = v1.dPriznaks[17];
            vv2 = v2.dPriznaks[17];
        break;
        case 18:
            vv1 = v1.n0;
            vv2 = v2.n0;
        break;
        case 19:
            vv1 = v1.dPriznaks[19];
            vv2 = v2.dPriznaks[19];
        break;
        case 20:
            vv1 = v1.dPriznaks[20];
            vv2 = v2.dPriznaks[20];
        break;
    }
    return vv1 < vv2;
}

static bool doubleGreaterThan(const filter &v1, const filter &v2)
{
    double vv1, vv2;
    switch (sortN)
    {
        case 0:
            vv1 = v1.dPriznaks[0];
            vv2 = v2.dPriznaks[0];
        break;
        case 1:
            vv1 = v1.dPriznaks[1];
            vv2 = v2.dPriznaks[1];
        break;
        case 2:
            vv1 = v1.dPriznaks[2];
            vv2 = v2.dPriznaks[2];
        break;
        case 3:
            vv1 = v1.dPriznaks[3];
            vv2 = v2.dPriznaks[3];
        break;
        case 4:
            vv1 = v1.dPriznaks[4];
            vv2 = v2.dPriznaks[4];
        break;
        case 5:
            vv1 = v1.dPriznaks[5];
            vv2 = v2.dPriznaks[5];
        break;
        case 6:
            vv1 = v1.dPriznaks[6];
            vv2 = v2.dPriznaks[6];
        break;
        case 8:
            vv1 = v1.dPriznaks[8];
            vv2 = v2.dPriznaks[8];
        break;
        case 10:
            vv1 = v1.dPriznaks[10];
            vv2 = v2.dPriznaks[10];
        break;
        case 12:
            vv1 = v1.dPriznaks[12];
            vv2 = v2.dPriznaks[12];
        break;
        case 13:
            vv1 = v1.dPriznaks[13];
            vv2 = v2.dPriznaks[13];
        break;
        case 14:
            vv1 = v1.dPriznaks[14];
            vv2 = v2.dPriznaks[14];
        break;
        case 15:
            vv1 = v1.dPriznaks[15];
            vv2 = v2.dPriznaks[15];
        break;
        case 16:
            vv1 = v1.dPriznaks[16];
            vv2 = v2.dPriznaks[16];
        break;
        case 17:
            vv1 = v1.dPriznaks[17];
            vv2 = v2.dPriznaks[17];
        break;
        case 18:
            vv1 = v1.n0;
            vv2 = v2.n0;
        break;
        case 19:
            vv1 = v1.dPriznaks[19];
            vv2 = v2.dPriznaks[19];
        break;
        case 20:
            vv1 = v1.dPriznaks[20];
            vv2 = v2.dPriznaks[20];
        break;
    }
    return vv1 > vv2;
}

void MainWindow::sort()
{
    int ascDesc;
    //sortN
    if(ui->radioB1->isChecked())sortN = 18;
    if(ui->radioB1->isChecked())sortN = 0;
    if(ui->radioB2->isChecked())sortN = 1;
    if(ui->radioB3->isChecked())sortN = 2;
    if(ui->radioB4->isChecked())sortN = 3;
    if(ui->radioB5->isChecked())sortN = 4;
    if(ui->radioB6->isChecked())sortN = 5;
    if(ui->radioB7->isChecked())sortN = 6;
    if(ui->radioB8->isChecked())sortN = 8;
    if(ui->radioB9->isChecked())sortN = 10;
    if(ui->radioB10->isChecked())sortN = 12;
    if(ui->radioB11->isChecked())sortN = 13;
    if(ui->radioB12->isChecked())sortN = 14;
    if(ui->radioB13->isChecked())sortN = 15;
    if(ui->radioB14->isChecked())sortN = 16;
    if(ui->radioB15->isChecked())sortN = 17;
    if(ui->radioB16->isChecked())sortN = 18;
    if(ui->radioB17->isChecked())sortN = 19;
    if(ui->radioB18->isChecked())sortN = 20;
    //ascDesc
    if(ui->ascBtn->isChecked())
    {
        ascDesc = 1;
    }
    else
    {
        ascDesc = 2;
    }
    if(pdata.size()==0)return;
    if(ascDesc==1)
    {
        std::sort(pdata.begin(), pdata.end(), doubleLessThan);
    }
    else
    {
        std::sort(pdata.begin(), pdata.end(), doubleGreaterThan);
    }
    ui->listWidget->clear();
    for(int i=0; i<pdata.size(); i++)
    {
        ui->listWidget->addItem(pdata.at(i).name);
    }
}

void MainWindow::on_radioB0_clicked()
{
    sort();
}

void MainWindow::viewClick(int x, int y)
{
    int w = inputImage.width();
    float k = scene->width();
    qDebug()<<"x,y="<<x*w/k<<" "<<y*w/k;
    float xx = (float)x*w/k;
    float yy = (float)y*w/k;
    for(int n=0; n<polygonList.length(); n++)
    {
        QPolygon p = polygonList.at(n);
        QPainterPath path;
        path.addPolygon(p);
        if(path.contains(QPoint(xx,yy))) {
            selSquare = n;
            break;
        }
    }
    if(polygonList.length()>0) {
        calculate(selSquare);
    }
}
