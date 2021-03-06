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

#include "juliacompletionobject.h"

#include "juliasession.h"
#include "juliakeywords.h"

JuliaCompletionObject::JuliaCompletionObject(
    const QString &command, int index, JuliaSession *session)
    : Cantor::CompletionObject(session)
{
    setLine(command, index);
}

JuliaCompletionObject::~JuliaCompletionObject()
{
}

void JuliaCompletionObject::fetchCompletions()
{
    auto julia_session = dynamic_cast<JuliaSession *>(session());
    julia_session->runJuliaCommand(
        QString::fromLatin1(
            "join("
            "Base.REPL.REPLCompletions.completions(\"%1\", %2)[1],"
            "\"__CANTOR_DELIM__\")"
        ).arg(command()).arg(command().size())
    );

    auto result = julia_session->getOutput();
    result.chop(1);
    result.remove(0, 1);
    setCompletions(result.split(QLatin1String("__CANTOR_DELIM__")));

    emit fetchingDone();
}

bool JuliaCompletionObject::mayIdentifierContain(QChar c) const
{
    return c.isLetter() || c.isDigit() || c == QLatin1Char('_') ||
        c == QLatin1Char('%') || c == QLatin1Char('$') || c == QLatin1Char('.');
}

bool JuliaCompletionObject::mayIdentifierBeginWith(QChar c) const
{
    return c.isLetter() || c == QLatin1Char('_') || c == QLatin1Char('%') ||
        c == QLatin1Char('$');
}
