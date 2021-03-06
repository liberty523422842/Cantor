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
    Copyright (C) 2015 Minh Ngo <minh@fedoraproject.org>
 */

#ifndef _PYTHON2SESSION_H
#define _PYTHON2SESSION_H

#include "../python/pythonsession.h"

struct _object;
typedef _object PyObject;

class Python2Session : public PythonSession
{
  public:
    Python2Session(Cantor::Backend* backend);

    void login() Q_DECL_OVERRIDE;

    bool integratePlots() const Q_DECL_OVERRIDE;
    QStringList autorunScripts() const Q_DECL_OVERRIDE;

  private:
    void runPythonCommand(const QString& command) const Q_DECL_OVERRIDE;
    QString getOutput() const Q_DECL_OVERRIDE;
    QString getError() const Q_DECL_OVERRIDE;

    QString pyObjectToQString(PyObject* obj) const;

  private:
    PyObject *m_pModule;
};

#endif
