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

#include "maximaextensions.h"
#include <QStringList>

MaximaCASExtension::MaximaCASExtension( QObject* parent) : MathematiK::CASExtension(parent)
{

}

MaximaCASExtension::~MaximaCASExtension()
{

}


QString MaximaCASExtension::solve(const QStringList& equations, const QStringList& variables)
{
    QString eqstr=QString("[%1]").arg(equations.join(","));

    QString variablestr=QString("[%1]").arg(variables.join(","));

    return QString("solve(%1,%2)").arg(eqstr, variablestr);
}

QString MaximaCASExtension::simplify(const QString& expression)
{
    return "simplify("+expression+")";
}

QString MaximaCASExtension::expand(const QString& expression)
{
    return "expand("+expression+")";
}


MaximaCalculusExtension::MaximaCalculusExtension(QObject* parent) : MathematiK::CalculusExtension(parent)
{

}

MaximaCalculusExtension::~MaximaCalculusExtension()
{

}

QString MaximaCalculusExtension::limit(const QString& expression, const QString& variable, const QString& limit)
{
    return QString("limit(%1, %2=%3)").arg(expression, variable, limit);
}

QString MaximaCalculusExtension::differentiate(const QString& function,const QString& variable, int times)
{
    return QString("diff(%1, %2, %3)").arg(function, variable, QString::number(times));
}

QString MaximaCalculusExtension::integrate(const QString& function, const QString& variable)
{
    return QString("integrate(%1, %2)").arg(function, variable);
}

QString MaximaCalculusExtension::integrate(const QString& function,const QString& variable, const QString& left, const QString& right)
{
    return QString("integrate(%1, %2, %3, %4)").arg(function, variable, left, right);
}