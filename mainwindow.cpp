#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtNetwork/QHostInfo>
#include <QtNetwork>
#include <qbytearray.h>
#include <stdint.h>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());
    MainWindow::setWindowTitle("Yeelight Bulb Toggle");
    //ui->horizontalSlider tracking = false

    mcast_addr = "239.255.255.250";//UDP组播IP
    udp_port = 1982;//UDP port

    {//获得本机IP
        QString localHostName = QHostInfo::localHostName();
        QHostInfo info = QHostInfo::fromName(localHostName);
        foreach (QHostAddress address, info.addresses())
        {
            if(address.protocol() == QAbstractSocket::IPv4Protocol)
            {
                ui->label->setText(address.toString());
                local_ip = address.toString();//存储本机IP
                qDebug()<<"IP:"<<address.toString();
            }
        }
    }

    {//初始化网络
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
        connect(&udp_socket, SIGNAL(readyRead()), // 数据流过来触发readyRead()信号
            this, SLOT(processPendingDatagrams()));
    }
}

MainWindow::~MainWindow()
{
    tcp_socket.close();
    udp_socket.close();
    delete ui;
}

void MainWindow::processPendingDatagrams()
{
    while (udp_socket.hasPendingDatagrams()) 
    { // 是否有待处理的信号
        qDebug()<<"udp receive data";
        udp_datagram_recv.resize(udp_socket.pendingDatagramSize()); //以数据包的大小初始化datagram
        udp_socket.readDatagram(udp_datagram_recv.data(), udp_datagram_recv.size()); //读取数据
        qDebug()<<udp_datagram_recv.data();

        QByteArray start_str;
        QByteArray end_str;
        QByteArray rtn_str;

        //提取bulb_ip
        start_str.clear(); end_str.clear(); rtn_str.clear();
        start_str.append("Location: yeelight://");
        end_str.append(":");
        sub_string(start_str, end_str, rtn_str);
        if(rtn_str.isEmpty() == false)
        {
            bulb_ip = rtn_str;
        }

        //提取bulb_id
        start_str.clear(); end_str.clear(); rtn_str.clear();
        start_str.append("id: ");
        end_str.append("\r\n");
        sub_string(start_str, end_str, rtn_str);
        if(rtn_str.isEmpty() == false)
        {
            bulb_id_str = rtn_str;
        }

        //过滤重复探测到的bulb
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
{//提取字符串
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

void MainWindow::on_pushButton_3_clicked()
{//"探测"button
    QByteArray datagram = "M-SEARCH * HTTP/1.1\r\n\
HOST: 239.255.255.250:1982\r\n\
MAN: \"ssdp:discover\"\r\n\
ST: wifi_bulb";

    int ret = udp_socket.writeDatagram(datagram.data(), datagram.size(), mcast_addr, udp_port);
    qDebug()<<"udp write "<<ret<<" bytes";
}

void MainWindow::on_pushButton_clicked()
{//"连接"button
    tcp_socket.close();//关闭上次的连接
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
{//"开/关"button, 发送控制信息
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
		cmd_str->append(QString("%1").arg(pos));
        cmd_str->append(", \"smooth\", 500]}\r\n");
        tcp_socket.write(cmd_str->data());
        qDebug() << cmd_str->data();
    }
    else
    {
        qDebug()<<"Bulb is empty";
    }

}
