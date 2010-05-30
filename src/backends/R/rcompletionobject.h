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
    Copyright (C) 2010 Oleksiy Protas <elfy.ua@gmail.com>
 */

#ifndef _RCOMPLETIONOBJECT_H
#define _RCOMPLETIONOBJECT_H

#include "completionobject.h"

class RSession;

class RCompletionObject : public Cantor::CompletionObject
{
  Q_OBJECT
  public:
    RCompletionObject( const QString& cmd, RSession* session );
    ~RCompletionObject();

  protected slots:
    void fetchCompletions();
    void receiveCompletions(const QString& token,const QStringList& options);

  signals:
    void requestCompletion(const QString&);
    void goesOutOfScope(Cantor::CompletionObject*);
};

#endif /* _RCOMPLETIONOBJECT_H */
