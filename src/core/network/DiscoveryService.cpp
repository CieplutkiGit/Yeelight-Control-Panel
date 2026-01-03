#include "DiscoveryService.h"

#include "../protocol/YeelightDiscoveryParser.h"

#include <QNetworkDatagram>
#include <QNetworkInterface>

namespace {
const QHostAddress MulticastAddress(QStringLiteral("239.255.255.250"));
constexpr quint16 MulticastPort = 1982;
const QByteArray DiscoveryRequest =
    "M-SEARCH * HTTP/1.1\r\n"
    "HOST: 239.255.255.250:1982\r\n"
    "MAN: \"ssdp:discover\"\r\n"
    "ST: wifi_bulb\r\n"
    "\r\n";
}

DiscoveryService::DiscoveryService(QObject* parent)
    : QObject(parent) {
    sessionTimer_.setSingleShot(true);
    connect(&sessionTimer_, &QTimer::timeout, this, &DiscoveryService::stop);
    connect(&socket_, &QUdpSocket::readyRead, this, &DiscoveryService::readPendingDatagrams);
}

void DiscoveryService::start() {
    stop();
    const auto bindMode = QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint;
    if (!socket_.bind(QHostAddress::AnyIPv4, MulticastPort, bindMode)) {
        emit discoveryError(QStringLiteral("Unable to bind the discovery socket."));
        return;
    }

    int joinedInterfaces = 0;
    for (const auto& interface : QNetworkInterface::allInterfaces()) {
        const auto flags = interface.flags();
        if (!flags.testFlag(QNetworkInterface::IsUp)
            || !flags.testFlag(QNetworkInterface::IsRunning)
            || flags.testFlag(QNetworkInterface::IsLoopBack)) {
            continue;
        }
        bool hasIpv4 = false;
        for (const auto& entry : interface.addressEntries()) {
            hasIpv4 = hasIpv4
                || entry.ip().protocol() == QAbstractSocket::IPv4Protocol;
        }
        if (hasIpv4 && socket_.joinMulticastGroup(MulticastAddress, interface)) {
            ++joinedInterfaces;
        }
    }
    if (joinedInterfaces == 0) {
        emit discoveryError(QStringLiteral("No active IPv4 interface could join multicast discovery."));
    }

    discovering_ = true;
    broadcast();
    QTimer::singleShot(500, this, &DiscoveryService::broadcast);
    QTimer::singleShot(1500, this, &DiscoveryService::broadcast);
    sessionTimer_.start(4000);
}

void DiscoveryService::stop() {
    const bool wasDiscovering = discovering_;
    discovering_ = false;
    sessionTimer_.stop();
    socket_.close();
    if (wasDiscovering) {
        emit discoveryFinished();
    }
}

bool DiscoveryService::isDiscovering() const {
    return discovering_;
}

void DiscoveryService::broadcast() {
    if (!discovering_) {
        return;
    }
    const qint64 written = socket_.writeDatagram(
        DiscoveryRequest,
        MulticastAddress,
        MulticastPort
    );
    if (written != static_cast<qint64>(DiscoveryRequest.size())) {
        emit discoveryError(QStringLiteral("Unable to send a discovery broadcast."));
    }
}

void DiscoveryService::readPendingDatagrams() {
    while (socket_.hasPendingDatagrams()) {
        const QNetworkDatagram datagram = socket_.receiveDatagram();
        const auto result = YeelightDiscoveryParser::parse(datagram.data());
        if (result.success) {
            emit deviceDiscovered(result.device, result.state);
        } else {
            emit discoveryError(result.error);
        }
    }
}
