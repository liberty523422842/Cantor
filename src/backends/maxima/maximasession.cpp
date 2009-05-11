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
    Copyright (C) 2009 Alexander Rieder <alexanderrieder@gmail.com>
 */

#include "maximasession.h"
#include "maximaexpression.h"

#include <kdebug.h>
#include <kptyprocess.h>
#include <kptydevice.h>
#include <signal.h>
#include "settings.h"


const QRegExp MaximaSession::MaximaPrompt=QRegExp("\\(%I[0-9]*\\)"); //Text, maxima outputs, if it's taking new input
const QRegExp MaximaSession::MaximaOutputPrompt=QRegExp("\\(%O[0-9]*\\)"); //Text, maxima outputs, before any output


static QByteArray initCmd="display2d:false;                     \n \
                           inchar:%I;                           \n \
                           outchar:%O;                          \n \
                           print(\"____END_OF_INIT____\");      \n";

MaximaSession::MaximaSession( MathematiK::Backend* backend) : Session(backend)
{
    kDebug();
    m_isInitialized=false;
    m_process=0;
}

MaximaSession::~MaximaSession()
{
    kDebug();
}

void MaximaSession::login()
{
    kDebug()<<"login";
    if (m_process)
        m_process->deleteLater();

    m_process=new KPtyProcess(this);
    m_process->setProgram(MaximaSettings::self()->path().toLocalFile());
    m_process->setOutputChannelMode(KProcess::SeparateChannels);
    m_process->setPtyChannels(KPtyProcess::AllChannels);
    m_process->pty()->setEcho(false);
    //m_process->setUseUtmp(true);

    connect(m_process->pty(), SIGNAL(readyRead()), this, SLOT(readStdOut()));
    connect(m_process, SIGNAL(readyReadStandardError()), this, SLOT(readStdErr()));
    connect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(restartMaxima()));
    m_process->start();
    m_process->pty()->write(initCmd);
}

void MaximaSession::logout()
{
    kDebug()<<"logout";
    disconnect(m_process, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(restartMaxima()));

    if(m_expressionQueue.isEmpty())
        evaluateExpression("quit();");
    else
        interrupt();
    //Give maxima time to clean up
    if(!m_process->waitForFinished(3000))
    {
        m_process->kill();
    }

    m_process->deleteLater();
    m_expressionQueue.clear();
}

MathematiK::Expression* MaximaSession::evaluateExpression(const QString& cmd)
{
    kDebug()<<"evaluating: "<<cmd;
    MaximaExpression* expr=new MaximaExpression(this);
    expr->setCommand(cmd);
    expr->evaluate();

    return expr;
}

void MaximaSession::appendExpressionToQueue(MaximaExpression* expr)
{
    m_expressionQueue.append(expr);

    if(m_expressionQueue.size()==1)
    {
        changeStatus(MathematiK::Session::Running);
        runFirstExpression();
    }
}

void MaximaSession::readStdOut()
{
//    if(!m_process->canReadLine()) return;

    kDebug()<<"reading stdOut";
    QString out=m_process->pty()->readAll();
    kDebug()<<"out: "<<out;


    if(out.contains("____END_OF_INIT____"))
    {
        kDebug()<<"initialized";
        out.remove("____END_OF_INIT____");
        out.remove(MaximaPrompt);
        m_isInitialized=true;
        runFirstExpression();
        changeStatus(MathematiK::Session::Done);
        emit ready();
    }

    kDebug()<<"queuesize: "<<m_expressionQueue.size();
    if(!m_expressionQueue.isEmpty())
    {
        MaximaExpression* expr=m_expressionQueue.first();
        expr->parseOutput(out);
    }
}

void MaximaSession::readStdErr()
{
    kDebug()<<"reading stdErr";
    QString out=m_process->readAllStandardError();
    kDebug()<<"err: "<<out;
    if (!m_expressionQueue.isEmpty())
    {
        MaximaExpression* expr=m_expressionQueue.first();
        expr->parseError(out);
    }
}

void MaximaSession::currentExpressionChangedStatus(MathematiK::Expression::Status status)
{
    if(status!=MathematiK::Expression::Computing) //The session is ready for the next command
    {
        kDebug()<<"expression finished";
        MaximaExpression* expression=m_expressionQueue.first();
        if(expression->needsLatexResult())
        {
            kDebug()<<"asking for tex version";
            m_process->pty()->write("tex(%);\n");
            expression->aboutToReceiveLatex();
        }
        else
        {
            kDebug()<<"running next command";
            m_expressionQueue.removeFirst();
            if(m_expressionQueue.isEmpty())
                changeStatus(MathematiK::Session::Done);
            runFirstExpression();
        }
    }

}

void MaximaSession::runFirstExpression()
{
    if(!m_expressionQueue.isEmpty()&&m_isInitialized)
    {
        MaximaExpression* expr=m_expressionQueue.first();
        connect(expr, SIGNAL(statusChanged(MathematiK::Expression::Status)), this, SLOT(currentExpressionChangedStatus(MathematiK::Expression::Status)));
        QString command=expr->command();

        kDebug()<<"writing "<<command+"\n"<<" to the process";
        //We first send the command, with a $ at the end, and then request a latex version of the last result
        if (!command.endsWith('$'))
        {
            if (!command.endsWith(";"))
                command+=";";

            m_process->pty()->write((command+"\n").toLatin1());

        }
        else
        {
            m_process->pty()->write((command+"\n").toLatin1());
        }
    }
}

void MaximaSession::interrupt()
{
    if(!m_expressionQueue.isEmpty())
        m_expressionQueue.first()->interrupt();
    m_expressionQueue.clear();
    changeStatus(MathematiK::Session::Done);
}

void MaximaSession::sendSignalToProcess(int signal)
{
    kDebug()<<"sending signal....."<<signal;
    kill(m_process->pid(), signal);
}

void MaximaSession::sendInputToProcess(const QString& input)
{
    kDebug()<<"WARNING: use this method only if you know what you're doing. Use evaluateExpression to run commands";
    kDebug()<<"running "<<input;
    m_process->pty()->write(input.toLatin1());
}

MathematiK::Expression* MaximaSession::contextHelp(const QString& command)
{
/*    bool t=isTypesettingEnabled();
    if(t)
        setTypesettingEnabled(false);

    MathematiK::Expression* e=evaluateExpression("dir("+command+")");

    if(t)
        setTypesettingEnabled(true);

    return e;
*/
    return 0;
}

void MaximaSession::restartMaxima()
{
    kDebug()<<"restarting maxima";
    //remove the command that caused maxima to crash (to avoid infinite loops)
    m_expressionQueue.removeFirst();
    login();
}

#include "maximasession.moc"