#include "../src/core/protocol/YeelightDiscoveryParser.h"

#include <QtTest>

class DiscoveryParserTest final : public QObject {
    Q_OBJECT

private:
    static QByteArray response(const QByteArray& headers) {
        return "HTTP/1.1 200 OK\r\n" + headers + "\r\n";
    }

private slots:
    void parsesCompleteResponse();
    void acceptsHeaderVariations();
    void handlesOptionalAndUnknownHeaders();
    void rejectsInvalidLocations();
    void decodesNamesAndInitialState();
};

void DiscoveryParserTest::parsesCompleteResponse() {
    const auto result = YeelightDiscoveryParser::parse(response(
        "Location: yeelight://192.168.1.25:55443\r\n"
        "id: 0x1\r\nmodel: color\r\nfw_ver: 42\r\n"
        "support: set_power set_bright set_rgb\r\n"
        "power: on\r\nbright: 75\r\ncolor_mode: 1\r\n"
        "ct: 4000\r\nrgb: 16711680\r\nhue: 10\r\nsat: 90\r\nname: Desk"
    ));
    QVERIFY(result.success);
    QCOMPARE(result.device.stableId(), QStringLiteral("0x1"));
    QCOMPARE(result.device.port, quint16(55443));
    QVERIFY(result.device.capabilities.supportsRgb());
    QCOMPARE(result.state.power, PowerState::On);
    QCOMPARE(result.state.brightness, 75);
}

void DiscoveryParserTest::acceptsHeaderVariations() {
    const auto result = YeelightDiscoveryParser::parse(response(
        "BrIgHt: 20\r\nSUPPORT:   set_power   set_ct_abx  \r\n"
        "LOCATION:   yeelight://10.0.0.2:55443\r\nid: first\r\nID: second"
    ));
    QVERIFY(result.success);
    QCOMPARE(result.device.id, QStringLiteral("second"));
    QCOMPARE(result.state.brightness, 20);
    QVERIFY(result.device.capabilities.supportsColorTemperature());
}

void DiscoveryParserTest::handlesOptionalAndUnknownHeaders() {
    const auto result = YeelightDiscoveryParser::parse(response(
        "Location: yeelight://127.0.0.1:55443\r\n"
        "support:\r\nX-Unknown: ignored"
    ));
    QVERIFY(result.success);
    QVERIFY(result.device.name.isEmpty());
    QVERIFY(result.device.capabilities.methods.isEmpty());
    QCOMPARE(result.device.stableId(), QStringLiteral("127.0.0.1:55443"));
}

void DiscoveryParserTest::rejectsInvalidLocations() {
    QVERIFY(!YeelightDiscoveryParser::parse(response("id: missing")).success);
    QVERIFY(!YeelightDiscoveryParser::parse(
        response("Location: yeelight://not-an-ip:55443")
    ).success);
    QVERIFY(!YeelightDiscoveryParser::parse(
        response("Location: yeelight://127.0.0.1:99999")
    ).success);
}

void DiscoveryParserTest::decodesNamesAndInitialState() {
    const auto result = YeelightDiscoveryParser::parse(response(
        "Location: yeelight://192.168.1.2:55443\r\n"
        "name: Living%20Room\r\npower: off\r\nbright: 12"
    ));
    QVERIFY(result.success);
    QCOMPARE(result.device.name, QStringLiteral("Living Room"));
    QCOMPARE(result.state.power, PowerState::Off);
    QCOMPARE(result.state.brightness, 12);
}

QTEST_MAIN(DiscoveryParserTest)
#include "test_discovery_parser.moc"

