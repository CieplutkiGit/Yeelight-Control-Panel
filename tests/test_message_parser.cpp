#include "../src/core/protocol/YeelightMessageParser.h"

#include <QSignalSpy>
#include <QtTest>

class MessageParserTest final : public QObject {
    Q_OBJECT

private slots:
    void parsesResponsesAndErrors();
    void parsesNotificationsAndMultipleLines();
    void preservesPartialMessages();
    void recoversAfterMalformedInput();
};

void MessageParserTest::parsesResponsesAndErrors() {
    YeelightMessageParser parser;
    QSignalSpy responses(&parser, &YeelightMessageParser::responseReceived);
    QSignalSpy errors(&parser, &YeelightMessageParser::commandFailed);
    parser.feed("{\"id\":1,\"result\":[\"ok\"]}\r\n");
    parser.feed("{\"id\":2,\"error\":{\"code\":-1,\"message\":\"bad\"}}\r\n");
    QCOMPARE(responses.size(), 1);
    QCOMPARE(responses.at(0).at(0).toInt(), 1);
    QCOMPARE(errors.size(), 1);
    QCOMPARE(errors.at(0).at(1).toInt(), -1);
}

void MessageParserTest::parsesNotificationsAndMultipleLines() {
    YeelightMessageParser parser;
    QSignalSpy properties(&parser, &YeelightMessageParser::propertiesChanged);
    parser.feed("\r\n{\"method\":\"props\",\"params\":{\"power\":\"on\"}}\r\n"
                "{\"method\":\"props\",\"params\":{\"bright\":\"50\"}}\r\n");
    QCOMPARE(properties.size(), 2);
}

void MessageParserTest::preservesPartialMessages() {
    YeelightMessageParser parser;
    QSignalSpy responses(&parser, &YeelightMessageParser::responseReceived);
    parser.feed("{\"id\":3,");
    parser.feed("\"result\":[");
    parser.feed("\"ok\"]}\r\n");
    QCOMPARE(responses.size(), 1);
}

void MessageParserTest::recoversAfterMalformedInput() {
    YeelightMessageParser parser;
    QSignalSpy protocolErrors(&parser, &YeelightMessageParser::protocolError);
    QSignalSpy responses(&parser, &YeelightMessageParser::responseReceived);
    parser.feed("not-json\r\n{\"id\":4,\"result\":[\"ok\"]}\r\n"
                "{\"method\":\"unknown\",\"params\":{}}\r\n");
    QCOMPARE(protocolErrors.size(), 2);
    QCOMPARE(responses.size(), 1);
}

QTEST_MAIN(MessageParserTest)
#include "test_message_parser.moc"

