/*
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2
    of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA  02110-1301, USA.

    ---
    Copyright (C) 2011 Filipe Saraiva <filipe@kde.org>
 */

#include "scilabexpression.h"
#include "scilabkeywords.h"

#include <config-cantorlib.h>

#include "textresult.h"
#include "imageresult.h"
#include "helpresult.h"

#include <QDebug>
#include <QDir>
#include <QFile>

#include <KIconLoader>

#include "scilabsession.h"
#include "settings.h"
#include "defaultvariablemodel.h"

typedef Cantor::ImageResult ScilabPlotResult;

ScilabExpression::ScilabExpression( Cantor::Session* session ) : Cantor::Expression(session)
{
    qDebug() << "ScilabExpression construtor";
}

ScilabExpression::~ScilabExpression()
{

}

void ScilabExpression::evaluate()
{
    setStatus(Cantor::Expression::Computing);

    ScilabSession* scilabSession = dynamic_cast<ScilabSession*>(session());

    if((ScilabSettings::integratePlots()) && (command().contains(QLatin1String("plot")))){

        qDebug() << "Preparing export figures property";

        QString exportCommand;

        QStringList commandList = command().split(QLatin1String("\n"));

        for(int count = 0; count < commandList.size(); count++){

            if(commandList.at(count).toLocal8Bit().contains("plot")){

                exportCommand = QString::fromLatin1("\nxs2png(gcf(), 'cantor-export-scilab-figure-%1.png');\ndelete(gcf());").arg(rand());

                commandList[count].append(exportCommand);

                exportCommand.clear();
            }

            qDebug() << "Command " << count << ": " << commandList.at(count).toLocal8Bit().constData();
        }

        QString newCommand = commandList.join(QLatin1String("\n"));
        newCommand.prepend(QLatin1String("clf();\n"));
        newCommand.append(QLatin1String("\n"));

        this->setCommand(newCommand);

        qDebug() << "New Command " << command();

    }

    scilabSession->runExpression(this);
}

void ScilabExpression::parseOutput(QString output)
{
    qDebug() << "output: " << output;

    m_output = output;
    setResult(new Cantor::TextResult(output));

    evalFinished();
    setStatus(Cantor::Expression::Done);
}

void ScilabExpression::parseError(QString error)
{
    qDebug() << "error" << error;

    setErrorMessage(error.replace(QLatin1String("\n"), QLatin1String("<br>")).remove(0, 2)
        .replace(QLatin1String(" "), QLatin1String("&nbsp;")));

    evalFinished();
    setStatus(Cantor::Expression::Error);
}

void ScilabExpression::parsePlotFile(QString filename)
{
    qDebug() << "parsePlotFile";

    qDebug() << "ScilabExpression::parsePlotFile: " << filename;

    setResult(new ScilabPlotResult(QUrl::fromLocalFile(filename)));

    setPlotPending(false);

    if (m_finished){
        qDebug() << "ScilabExpression::parsePlotFile: done";
        setStatus(Done);
    }
}

void ScilabExpression::interrupt()
{
    qDebug()<<"interruptinging command";
    setStatus(Cantor::Expression::Interrupted);
}

void ScilabExpression::evalFinished()
{
    qDebug()<<"evaluation finished";

    foreach (const QString& line, m_output.simplified().split(QLatin1Char('\n'), QString::SkipEmptyParts)){
        if (m_output.contains(QLatin1Char('='))){

            qDebug() << line;

            QStringList parts = line.split(QLatin1Char('='));

            if (parts.size() >= 2){
                Cantor::DefaultVariableModel* model = dynamic_cast<Cantor::DefaultVariableModel*>(session()->variableModel());

                if (model){
                    model->addVariable(parts.first().trimmed(), parts.last().trimmed());
                    ScilabKeywords::instance()->addVariable(parts.first().trimmed());
                }
            }
        }
    }
}

void ScilabExpression::setPlotPending(bool plot)
{
    m_plotPending = plot;
}
