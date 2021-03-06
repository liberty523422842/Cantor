/*
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *    Boston, MA  02110-1301, USA.
 *
 *    ---
 *    Copyright (C) 2016 Ivan Lakhtanov <ivan.lakhtanov@gmail.com>
 */
#include "testjulia.h"

#include "session.h"
#include "backend.h"
#include "expression.h"
#include "result.h"
#include "textresult.h"
#include "imageresult.h"
#include "defaultvariablemodel.h"
#include "completionobject.h"

QString TestJulia::backendName()
{
    return QLatin1String("julia");
}

void TestJulia::testOneLine()
{
    Cantor::Expression *e = evalExp(QLatin1String("2 + 3"));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QCOMPARE(e->result()->data().toString(), QLatin1String("5"));
    QVERIFY(e->errorMessage().isEmpty());
}

void TestJulia::testOneLineWithPrint()
{
    Cantor::Expression *e = evalExp(QLatin1String("print(2 + 3)"));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QCOMPARE(e->result()->data().toString(), QLatin1String("5"));
    QVERIFY(e->errorMessage().isEmpty());
}

void TestJulia::testException()
{
    Cantor::Expression *e = evalExp(QLatin1String("sqrt(-1)"));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Error);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QVERIFY(
        !e->errorMessage().isEmpty()
        && e->errorMessage().contains(QLatin1String(
            "sqrt will only return a complex result if called with a "
            "complex argument. Try sqrt(complex(x))."
        ))
    );
}

void TestJulia::testSyntaxError()
{
    Cantor::Expression *e = evalExp(QLatin1String("for i = 1:10\nprint(i)"));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Error);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QVERIFY(
        !e->errorMessage().isEmpty()
        && e->errorMessage().contains(QLatin1String(
            "syntax: incomplete: \"for\" at none:1 requires end"
        ))
    );
}

void TestJulia::testMultilineCode()
{
    Cantor::Expression *e = evalExp(QLatin1String(
        "q = 0; # comment\n"
        "# sdfsdf\n"
        "for i = 1:10\n"
        "    q += i\n"
        "end\n"
        "print(q)"
    ));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QCOMPARE(e->result()->data().toString(), QLatin1String("55"));
    QVERIFY(e->errorMessage().isEmpty());
}

void TestJulia::testPartialResultOnException()
{
    Cantor::Expression *e = evalExp(QLatin1String(
        "for i = 1:5\n"
        "    print(i)\n"
        "end\n"
        "sqrt(-1)\n"
    ));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Error);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QCOMPARE(e->result()->data().toString(), QLatin1String("12345"));
    QVERIFY(
        !e->errorMessage().isEmpty()
        && e->errorMessage().contains(QLatin1String(
            "sqrt will only return a complex result if called with a "
            "complex argument. Try sqrt(complex(x))."
        ))
    );
}

void TestJulia::testInlinePlot()
{
    Cantor::Expression *e = evalExp(QLatin1String(
        "import GR\n"
        "GR.plot(linspace(-1, 1), sin(linspace(-1, 1)))\n"
    ));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Done);
    QVERIFY(e->result()->type() == Cantor::ImageResult::Type);
}

void TestJulia::testInlinePlotWithExceptionAndPartialResult()
{
    Cantor::Expression *e = evalExp(QLatin1String(
        "import GR\n"
        "print(\"gonna plot\")\n"
        "sqrt(-1)\n"
        "GR.plot(linspace(-1, 1), sin(linspace(-1, 1)))\n"
    ));
    QVERIFY(e != nullptr);
    QCOMPARE(e->status(), Cantor::Expression::Error);
    QVERIFY(e->result()->type() == Cantor::TextResult::Type);
    QCOMPARE(e->result()->data().toString(), QLatin1String("gonna plot"));
    QVERIFY(
         !e->errorMessage().isEmpty()
        && e->errorMessage().contains(QLatin1String(
            "sqrt will only return a complex result if called with a "
            "complex argument. Try sqrt(complex(x))."
        ))
    );
}

void TestJulia::testAddVariablesFromCode()
{
    evalExp(QLatin1String("a = 0; b = 1; c = 2; d = 3\n"));

    auto variableModel = session()->variableModel();
    QStringList variableNames =
    QString::fromLatin1("a b c d").split(QLatin1String(" "));

    for (int i = 0; i < variableNames.size(); i++) {
        QModelIndexList matchedVariables = variableModel->match(
            variableModel->index(0, 0),
            Qt::DisplayRole,
            QVariant::fromValue(variableNames[i]),
            -1,
            Qt::MatchExactly
        );
        QCOMPARE(matchedVariables.size(), 1);
        auto match = matchedVariables[0];
        auto valueIndex =
            variableModel->index(match.row(), match.column() + 1);
        QVERIFY(
            valueIndex.data(Qt::DisplayRole) ==
            QVariant::fromValue(QString::number(i))
        );
    }
}

void TestJulia::testAddVariablesFromManager()
{
    auto variableModel = dynamic_cast<Cantor::DefaultVariableModel *>(
        session()->variableModel()
    );
    QStringList variableNames =
    QString::fromLatin1("a b c d").split(QLatin1String(" "));

    for (int i = 0; i < variableNames.size(); i++) {
        variableModel->addVariable(variableNames[i], QString::number(i));

        QModelIndexList matchedVariables = variableModel->match(
            variableModel->index(0, 0),
            Qt::DisplayRole,
            QVariant::fromValue(variableNames[i]),
            -1,
            Qt::MatchExactly
        );
        QCOMPARE(matchedVariables.size(), 1);
        auto match = matchedVariables[0];
        auto valueIndex =
            variableModel->index(match.row(), match.column() + 1);
        QVERIFY(
            valueIndex.data(Qt::DisplayRole) ==
            QVariant::fromValue(QString::number(i))
        );
    }
}

void TestJulia::testRemoveVariables()
{
    auto variableModel = dynamic_cast<Cantor::DefaultVariableModel *>(
        session()->variableModel()
    );
    QStringList variableNames =
    QString::fromLatin1("a b c d").split(QLatin1String(" "));

    for (int i = 0; i < variableNames.size(); i++) {
        variableModel->addVariable(variableNames[i], QString::number(i));
    }
    for (int i = 0; i < variableNames.size(); i += 2) {
        variableModel->removeVariable(variableNames[i]);
    }

    for (int i = 0; i < variableNames.size(); i++) {
        QModelIndexList matchedVariables = variableModel->match(
            variableModel->index(0, 0),
            Qt::DisplayRole,
            QVariant::fromValue(variableNames[i]),
            -1,
            Qt::MatchExactly
        );
        if (i % 2 == 0) {
            QVERIFY(matchedVariables.isEmpty());
        } else {
            QCOMPARE(matchedVariables.size(), 1);
            auto match = matchedVariables[0];
            auto valueIndex =
                variableModel->index(match.row(), match.column() + 1);
            QVERIFY(
                valueIndex.data(Qt::DisplayRole) ==
                QVariant::fromValue(QString::number(i))
            );
        }
    }
}

void TestJulia::testAutoCompletion()
{
    auto prefix = QLatin1String("Ba");
    auto completionObject = session()->completionFor(prefix);
    // Give sometime for Qt's singleShot in fetch completions to trigger
    QTest::qWait(500);
    auto completions = completionObject->completions();

    QStringList completionsToCheck;
    completionsToCheck << QLatin1String("Base");
    completionsToCheck << QLatin1String("Base64DecodePipe");
    completionsToCheck << QLatin1String("Base64EncodePipe");

    for (auto completion : completionsToCheck) {
        QVERIFY(completions.contains(completion));
    }

    for (auto completion : completionsToCheck) {
        QVERIFY(completion.startsWith(prefix));
    }
}

QTEST_MAIN(TestJulia)

