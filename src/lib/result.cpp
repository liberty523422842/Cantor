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

#include "result.h"
using namespace Cantor;

#include <QRegExp>
#include <QUrl>

class Cantor::ResultPrivate
{
  public:

};


Result::Result() : d(new ResultPrivate)
{

}

Result::~Result()
{
    delete d;
}

QUrl Result::url()
{
    return QUrl();
}

QString Result::toLatex()
{
    QString html=toHtml();
    //replace linebreaks
    html.replace(QRegExp(QLatin1String("<br/>[\n]")), QLatin1String("\n"));
    //remove all the unknown tags
    html.remove( QRegExp( QLatin1String("<[a-zA-Z\\/][^>]*>") ) );
    return QString::fromLatin1("\\begin{verbatim} %1 \\end{verbatim}").arg(html);
}

void Result::saveAdditionalData(KZip* archive)
{
    Q_UNUSED(archive)
    //Do nothing
}


