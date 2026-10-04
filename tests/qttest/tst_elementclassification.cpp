// SPDX-License-Identifier: GPL-2.0-or-later
#include <QtTest>
#include <QStandardItemModel>
#include "ElementsCollection/elementclassification.h"
#include "qetapp.h"

QString QETApp::m_interface_language;

class tst_elementclassification : public QObject
{
	Q_OBJECT
	QByteArray m_definition;

	static DiagramContext read(const QByteArray &xml, bool pugiReader)
	{
		DiagramContext context;
		if (pugiReader) {
			pugi::xml_document doc;
			if (!doc.load_buffer(xml.constData(), size_t(xml.size()))) {
				return context;
			}
			context.fromXml(doc.document_element().child("elementInformations"),
					QStringLiteral("elementInformation"));
		} else {
			QDomDocument doc;
			if (!doc.setContent(xml)) {
				return context;
			}
			context.fromXml(doc.documentElement().firstChildElement(
					QStringLiteral("elementInformations")),
					QStringLiteral("elementInformation"));
		}
		return context;
	}

private slots:
	void initTestCase()
	{
		QFile file(QFINDTESTDATA("fixtures/classified_rcd.elmt"));
		QVERIFY(file.open(QIODevice::ReadOnly));
		m_definition = file.readAll();
	}

	void readClassification_data()
	{
		QTest::addColumn<bool>("pugiReader");
		QTest::newRow("collection-pugixml") << true;
		QTest::newRow("project-QDom") << false;
	}

	void readClassification()
	{
		QFETCH(bool, pugiReader);
		const DiagramContext context = read(m_definition, pugiReader);
		const auto metadata = ElementClassification::fromInformations(context);
		QCOMPARE(metadata.standards, (QStringList{"IEC60617", "RGIE"}));
		QCOMPARE(metadata.countries, (QStringList{"BE"}));
		QCOMPARE(metadata.tags,
				(QStringList{"differential", "RCD", "protection", "domestic"}));
		QCOMPARE(metadata.categories,
				(QStringList{"electrical protection", "residential"}));
		QVERIFY(!context.keyMustShow(QStringLiteral("tags")));
	}

	void searchClassification_data()
	{
		QTest::addColumn<QString>("query");
		QTest::addColumn<bool>("found");
		QTest::newRow("tag") << "RCD" << true;
		QTest::newRow("tag-case-insensitive") << "DOMESTIC" << true;
		QTest::newRow("standard") << "IEC60617" << true;
		QTest::newRow("second-standard") << "rgie" << true;
		QTest::newRow("country") << "BE" << true;
		QTest::newRow("category") << "residential" << true;
		QTest::newRow("absent") << "UL489" << false;
	}

	void searchClassification()
	{
		QFETCH(QString, query);
		QFETCH(bool, found);
		QVERIFY(query.size() >= ElementClassification::minimumQueryLength);
		QStandardItemModel model;
		auto *item = new QStandardItem(QStringLiteral("Device"));
		item->setData(ElementClassification::searchText(read(m_definition, true),
				QStringLiteral("Device")), Qt::UserRole + 1);
		model.appendRow(item);
		// Same role and matching flags as tree and ranked collection searches.
		const auto matches = model.match(model.index(0, 0), Qt::UserRole + 1,
				query, -1, Qt::MatchContains | Qt::MatchRecursive);
		QCOMPARE(matches.size(), found ? 1 : 0);
	}

	void legacyDefinition_data() { readClassification_data(); }

	void legacyDefinition()
	{
		QFETCH(bool, pugiReader);
		const auto context = read(
				"<definition><elementInformations>"
				"<elementInformation name='description'>Old device</elementInformation>"
				"<elementInformation name='custom_field'>Custom value</elementInformation>"
				"</elementInformations></definition>", pugiReader);
		const auto metadata = ElementClassification::fromInformations(context);
		QVERIFY(metadata.standards.isEmpty());
		QVERIFY(metadata.countries.isEmpty());
		QVERIFY(metadata.tags.isEmpty());
		QVERIFY(metadata.categories.isEmpty());
		QCOMPARE(context.keys().size(), 2);
		QCOMPARE(ElementClassification::searchText(context, "Device"),
				QStringLiteral("Custom value Old device Device"));
		const auto empty = read("<definition><names/></definition>", pugiReader);
		QCOMPARE(empty.keys().size(), 0);
		QCOMPARE(ElementClassification::searchText(empty, "Device"),
				QStringLiteral("Device"));

		QDomDocument document;
		auto info = document.createElement(QStringLiteral("elementInformations"));
		document.appendChild(info);
		context.toXml(info, QStringLiteral("elementInformation"));
		for (const QString &key : {QStringLiteral("standards"),
				QStringLiteral("countries"), QStringLiteral("tags"),
				QStringLiteral("categories")}) {
			QVERIFY(!document.toString().contains(key));
		}
	}

	void listNormalization()
	{
		QCOMPARE(ElementClassification::values(" RCD ; ; protection;rcd; "),
				(QStringList{"RCD", "protection"}));
	}

	void xmlRoundTrip()
	{
		const auto original = read(m_definition, true);
		QDomDocument document;
		auto root = document.createElement(QStringLiteral("definition"));
		document.appendChild(root);
		auto info = document.createElement(QStringLiteral("elementInformations"));
		root.appendChild(info);
		original.toXml(info, QStringLiteral("elementInformation"));
		const auto restored = read(document.toByteArray(), true);
		QVERIFY(original == restored);
		QCOMPARE(restored.value(QStringLiteral("tags")).toString(),
				QStringLiteral("differential;RCD;protection;domestic"));
	}
};

QTEST_APPLESS_MAIN(tst_elementclassification)
#include "tst_elementclassification.moc"
