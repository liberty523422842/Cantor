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

#ifndef _RSERVER_H
#define _RSERVER_H

#include <QObject>

class Expression
{
  public:
    QString cmd;
    int returnCode;
    bool hasOtherResults;
    QString err_buffer;
    QString std_buffer;
};

class RServer : public QObject
{
  Q_OBJECT
  Q_CLASSINFO("D-Bus Interface", "org.kde.Cantor.R")

  public:
    enum Status { Idle=0, Busy };
    enum ReturnCode { SuccessCode=0, ErrorCode, InterruptedCode};
    RServer( );
    ~RServer();

    void initR();
    void autoload();
    void endR();

    QString requestInput(const QString& prompt);
    void showFiles(const QStringList& files);

  Q_SIGNALS:
    void ready();
    void statusChanged(int status);
    void expressionFinished(int returnCode, const QString& text);
    void completionFinished(const QString& token,const QStringList& options);
    void showFilesNeeded(const QStringList& files);
    void inputRequested(const QString& prompt);

    void requestAnswered();
    void symbolList(const QStringList& variables, const QStringList& values, const QStringList& functions);

  public Q_SLOTS:
    void runCommand(const QString& cmd, bool internal=false);
    void answerRequest(const QString& answer);
    void completeCommand(const QString& cmd); // TODO: comment properly, only takes command from start to cursor
    void listSymbols();

  private:
    void setStatus(Status status);
    void newPlotDevice();
  private:
    bool m_isInitialized;
    bool m_isCompletionAvailable;
    Status m_status;
    QString m_requestCache;
    QString m_tmpDir;
    QString m_curPlotFile;
};

#endif /* _RSERVER_H */
