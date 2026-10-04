// SPDX-License-Identifier: GPL-2.0-or-later
#ifndef ELEMENTCLASSIFICATION_H
#define ELEMENTCLASSIFICATION_H

#include "../diagramcontext.h"
#include <QStringList>

/** Optional semicolon-separated classification in elementInformations.
	No physical collection path or element identity is changed.
*/
struct ElementClassification
{
	QStringList standards;
	QStringList countries;
	QStringList tags;
	QStringList categories;

	// Country codes such as BE must be searchable in both collection views.
	static constexpr int minimumQueryLength = 2;

	static QStringList values(const QString &text)
	{
		QStringList result;
		for (const QString &part : text.split(QLatin1Char(';'))) {
			const QString value = part.trimmed();
			if (!value.isEmpty() && !result.contains(value, Qt::CaseInsensitive)) {
				result.append(value);
			}
		}
		return result;
	}

	static ElementClassification fromInformations(const DiagramContext &context)
	{
		return {
			values(context.value(QStringLiteral("standards")).toString()),
			values(context.value(QStringLiteral("countries")).toString()),
			values(context.value(QStringLiteral("tags")).toString()),
			values(context.value(QStringLiteral("categories")).toString())
		};
	}

	/** Keep all existing information searchable, including unknown keys.
		Only classification lists are tokenized; the stored XML is untouched.
	*/
	static QString searchText(const DiagramContext &context, const QString &name)
	{
		QStringList result;
		for (const QString &key : context.keys()) {
			const QString value = context.value(key).toString();
			if (key == QLatin1String("standards") ||
				key == QLatin1String("countries") ||
				key == QLatin1String("tags") ||
				key == QLatin1String("categories")) {
				result.append(values(value).join(QLatin1Char(' ')));
			} else {
				result.append(value);
			}
		}
		result.append(name);
		return result.join(QLatin1Char(' '));
	}
};

#endif // ELEMENTCLASSIFICATION_H
