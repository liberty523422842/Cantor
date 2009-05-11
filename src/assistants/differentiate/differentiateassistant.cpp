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

#include "differentiateassistant.h"

#include <kdialog.h>
#include <kaction.h>
#include <kactioncollection.h>
#include "ui_differentiatedlg.h"
#include "mathematik_macros.h"
#include "backend.h"
#include "extension.h"

DifferentiateAssistant::DifferentiateAssistant(QObject* parent, QList<QVariant> args) : Assistant(parent)
{
    Q_UNUSED(args)
}

DifferentiateAssistant::~DifferentiateAssistant()
{

}

void DifferentiateAssistant::initActions()
{
    setXMLFile("mathematik_differentiate_assistant.rc");
    KAction* differentiate=new KAction(i18n("Differentiate"), actionCollection());
    differentiate->setIcon(KIcon(icon()));
    actionCollection()->addAction("differentiate_assistant", differentiate);
    connect(differentiate, SIGNAL(triggered()), this, SIGNAL(requested()));
}

QStringList DifferentiateAssistant::run(QWidget* parent)
{
    KDialog dlg(parent);
    QWidget widget;
    Ui::DifferentiateAssistantBase base;
    base.setupUi(&widget);
    dlg.setMainWidget(&widget);

    if( dlg.exec())
    {
        QString expression=base.expression->text();
        QString variable=base.variable->text();
        int times=base.times->value();

        MathematiK::CalculusExtension* ext= dynamic_cast<MathematiK::CalculusExtension*>(backend()->extension("CalculusExtension"));

        return QStringList()<<ext->differentiate(expression, variable, times);

    }
    return QStringList();
}

K_EXPORT_MATHEMATIK_PLUGIN(differentiateassistant, DifferentiateAssistant)