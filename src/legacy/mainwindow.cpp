#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork/QHostInfo>
#include <QtNetwork>
#include <qbytearray.h>
#include <stdint.h>
#include <QDebug>
#include <QColorDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Setup the user interface
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());
    MainWindow::setWindowTitle("dwie kreski auu");

    // Set the multicast address and port
    mcast_addr = QHostAddress("239.255.255.250");
    udp_port = 1982;

    // Get the local IP address
    QString localHostName = QHostInfo::localHostName();
    QHostInfo info = QHostInfo::fromName(localHostName);
    foreach (QHostAddress address, info.addresses())
    {
        if(address.protocol() == QAbstractSocket::IPv4Protocol)
        {
            ui->label->setText(address.toString());
            local_ip = address.toString();
            qDebug()<<"IP:"<<address.toString();
        }
    }

    // Initialize the network
    udp_socket.close();
    if (false == udp_socket.bind(QHostAddress(local_ip), 0, QUdpSocket::ShareAddress))
    {
        qDebug() << "udp bind failed\n";
        return;
    }
    else
    {
        qDebug() << "udp bind success\n";
    }
    udp_socket.joinMulticastGroup(mcast_addr);
    connect(&udp_socket, SIGNAL(readyRead()), this, SLOT(processPendingDatagrams()));

    // Add a new bulb with IP address, id and port
    //addBulb("192.168.8.144","0",55443);
    //You can add here bulb manually if you have problem with connection
}

// Function to add a new bulb to the list
void MainWindow::addBulb(QString ip, QString id, int port)
{
    // Create a new bulb object
    bulb_t new_bulb(ip.toStdString(), id.toStdString(), port);

    // Check if the bulb is already in the list
    auto it = std::find(bulb.begin(), bulb.end(), new_bulb);
    if (it == bulb.end())
    {
        // Add the bulb to the list
        bulb.push_back(new_bulb);

        // Add the IP address to the comboBox
        QStringList items;
        items << ip;
        ui->comboBox->addItems(items);
    }
    else
    {
        // The bulb is already in the list, do nothing
    }
}

MainWindow::~MainWindow()
{
    // Close the sockets and delete the user interface
    tcp_socket.close();
    udp_socket.close();
    delete ui;
}
void MainWindow::updateColor(const QColor &color)
{
    setRGB(color.red(), color.green(), color.blue());
}


    void MainWindow::setRGB(int red, int green, int blue)
    {
        // Variable init
        int RGB = (red * 65536) + (green * 256) + blue;

        QString command = "{\"id\":1,\"method\":\"set_rgb\",\"params\":[" + QString::number(RGB) + "]}\r\n\r\n\r\n";
        QByteArray bacommand;
        bacommand = command.toLocal8Bit();

        QTcpSocket *socketVar = &tcp_socket;
        socketVar->connectToHost(bulb_ip, tcp_port);
        // Variable init

        if(socketVar->waitForConnected(3000)) // Connected
        {
            qDebug() << "Connected within setRGB"; // Print "Connected within setRGB" to qDebug()

            socketVar->write(bacommand); // Write the command

            if(socketVar->waitForBytesWritten(3000)) // setRGB command written
            {
                qDebug() << "setRGB command written"; // Print "setRGB command written" to qDebug()

            }
            else // setRGB command not written
            {
                qDebug() << "setRGB command not written"; // Print "setRGB command not written" to qDebug()
            }


        }

        else
        {
            qDebug() << "Not connected within setRGB"; // Print "Not connected within setRGB" to qDebug()
        }
    }

//TO DO add timeout to break infiniti loop when no datagram recived
    void MainWindow::processPendingDatagrams()
    {
        while (udp_socket.hasPendingDatagrams())
        { // Check if there are any pending datagrams
            qDebug()<<"udp receive data";
            udp_datagram_recv.resize(udp_socket.pendingDatagramSize()); // Initialize the datagram with the size of the pending datagram
            udp_socket.readDatagram(udp_datagram_recv.data(), udp_datagram_recv.size()); // Read the data
            qDebug()<<udp_datagram_recv.data();

            QByteArray start_str;
            QByteArray end_str;
            QByteArray rtn_str;

            // Extract the bulb IP
            start_str.clear(); end_str.clear(); rtn_str.clear();
            start_str.append("Location: yeelight://");
            end_str.append(":");
            sub_string(start_str, end_str, rtn_str);
            if(rtn_str.isEmpty() == false)
            {
                bulb_ip = rtn_str;
            }

            // Extract the bulb ID
            start_str.clear(); end_str.clear(); rtn_str.clear();
            start_str.append("id: ");
            end_str.append("\r\n");
            sub_string(start_str, end_str, rtn_str);
            if(rtn_str.isEmpty() == false)
            {
                bulb_id_str = rtn_str;
            }

            // Filter out duplicate bulbs
            bulb_t bulb_tmp(bulb_ip.toStdString(), bulb_id_str.toStdString());
            ib = std::find(bulb.begin(), bulb.end(), bulb_tmp);
            if (ib == bulb.end())
            {
                bulb.push_back(bulb_tmp);

                QStringList items;
                QString tmp;
                tmp = bulb_ip;
                items << tmp;
                ui->comboBox->addItems(items);
            }

        }

    }

    int MainWindow::sub_string(QByteArray &start_str, QByteArray &end_str, QByteArray &rtn_str)
    {
        // Extract a string
        QByteArray result;
        int pos1 = -1;
        int pos2 = -1;

        result.clear();
        pos1 = udp_datagram_recv.indexOf(start_str, 0);
        if (pos1 != -1)
        {
            result = udp_datagram_recv.mid(pos1);
            pos1 = start_str.length();
            result = result.mid(pos1);
            pos2 = result.indexOf(end_str);
            result = result.mid(0, pos2);
        }
        rtn_str = result;

        return 0;
    }


void MainWindow::on_pushButton_2_clicked()
{
    QColorDialog* colorDialog = new QColorDialog(this);
    connect(colorDialog, &QColorDialog::currentColorChanged, this, [this](const QColor &color) {
        static QElapsedTimer timer;
        if(timer.isValid() && timer.elapsed() < 250) return;
        timer.start();
        updateColor(color);
    });
    colorDialog->exec();
}


void MainWindow::on_pushButton_3_clicked()
{//"Detection"button
    QByteArray datagram = "M-SEARCH * HTTP/1.1\r\n\
HOST: 239.255.255.250:1982\r\n\
MAN: \"ssdp:discover\"\r\n\
ST: wifi_bulb";

    int ret = udp_socket.writeDatagram(datagram.data(), datagram.size(), mcast_addr, udp_port);
    qDebug()<<"udp write "<<ret<<" bytes";
}

void MainWindow::on_pushButton_clicked()
{//"Connect"button
    tcp_socket.close();//Close last connection
    int device_idx = ui->comboBox->currentIndex();
    if(bulb.size() > 0)
    {
        tcp_socket.connectToHost(QHostAddress(bulb[device_idx].get_ip_str().c_str()), bulb[device_idx].get_port());
    }
    else
    {
        qDebug()<<"Bulb is empty";
    }
}

void MainWindow::on_pushButton_4_clicked()
{//"On/Off"button, Send Control Message
    QByteArray *cmd_str =new QByteArray;
    cmd_str->clear();
    cmd_str->append("{\"id\":");

    int device_idx = ui->comboBox->currentIndex();
    if(bulb.size() > 0)
    {
        cmd_str->append(bulb[device_idx].get_id_str().c_str());
        qDebug() << "combox index  = " << device_idx;

        cmd_str->append(",\"method\":\"toggle\",\"params\":[]}\r\n");
        tcp_socket.write(cmd_str->data());
        qDebug() << cmd_str->data();
    }
    else
    {
        qDebug()<<"Bulb is empty";
    }
}


void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    int pos = ui->horizontalSlider->value();
    QString slider_value = QString("%1").arg(pos) + "%";
    ui->label_4->setText(slider_value);
/* */
    QByteArray *cmd_str =new QByteArray;
    cmd_str->clear();
    cmd_str->append("{\"id\":");

    int device_idx = ui->comboBox->currentIndex();
    if(bulb.size() > 0)
    {
        cmd_str->append(bulb[device_idx].get_id_str().c_str());
        qDebug() << "combox index  = " << device_idx;

        cmd_str->append(",\"method\":\"set_bright\",\"params\":[");
        cmd_str->append(QString("%1").arg(pos).toUtf8());
        cmd_str->append(", \"smooth\", 500]}\r\n");
        tcp_socket.write(cmd_str->data());
        qDebug() << cmd_str->data();
    }
    else
    {
        qDebug()<<"Bulb is empty";
    }

}
