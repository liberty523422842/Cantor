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
#include "cantor.h"
#include "cantor.moc"

#include <kaction.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kshortcutsdialog.h>
#include <klibloader.h>
#include <kmessagebox.h>
#include <kstandardaction.h>
#include <kstatusbar.h>
#include <kurl.h>
#include <klocale.h>
#include <ktabwidget.h>
#include <kdebug.h>
#include <kconfigdialog.h>
#include <ktextedit.h>
#include <ktextbrowser.h>
#include <kxmlguifactory.h>
#include <knewstuff2/engine.h>
#include <kstandarddirs.h>

#include <QDockWidget>
#include <QApplication>

#include "lib/backend.h"

#include "settings.h"
#include "ui_settings.h"
#include "backendchoosedialog.h"

CantorShell::CantorShell()
    : KParts::MainWindow( )
{
    m_part=0;

    // set the shell's ui resource file
    setXMLFile("cantor_shell.rc");

    // then, setup our actions
    setupActions();

    QDockWidget* dock=new QDockWidget(i18n("Help"), this);
    m_helpView=new KTextEdit(dock);
    m_helpView->setText(i18n("<h1>Cantor</h1>The KDE way to do Mathematics"));
    m_helpView->setTextInteractionFlags(Qt::TextBrowserInteraction);
    dock->setWidget(m_helpView);
    addDockWidget ( Qt::RightDockWidgetArea,  dock );

    createGUI(0);
    bool hasBackend=false;
    foreach(Cantor::Backend* b, Cantor::Backend::availableBackends())
    {
        if(b->isEnabled())
            hasBackend=true;
    }

    if(hasBackend)
    {
        m_tabWidget=new KTabWidget(this);
        m_tabWidget->setCloseButtonEnabled(true);
        setCentralWidget(m_tabWidget);
        connect(m_tabWidget, SIGNAL(currentChanged(int)), this, SLOT(activateWorksheet(int)));
        connect(m_tabWidget, SIGNAL(closeRequest (QWidget *)), this, SLOT(closeTab(QWidget*)));
    }
    else
    {
        KTextBrowser *browser=new KTextBrowser(this);
        QString backendList="<ul>";
        foreach(Cantor::Backend* b, Cantor::Backend::availableBackends())
        {
            if(!b->requirementsFullfilled()) //It's disabled because of misssing dependencies, not because of some other reason(like eg. nullbackend)
                backendList+=QString("<li>%1: <a href=\"%2\">%2</a></li>").arg(b->name(), b->url());
        }
        browser->setHtml(i18n("<h1>No Backend Found</h1>\n"             \
                              "<div>You could try:\n"                   \
                              "  <ul>"                                  \
                              "    <li>Changing the settings in the config dialog;</li>" \
                              "    <li>Installing packages for one of the following programs:</li>" \
                              "     %1 "                                \
                              "  <ul> "                                 \
                              "</div> "
                             , backendList
                             ));
        setCentralWidget(browser);
    }

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();
}

CantorShell::~CantorShell()
{
}

void CantorShell::load(const KUrl& url)
{
    if (!m_part||!m_part->url().isEmpty() || m_part->isModified() )
    {
        addWorksheet("nullbackend");
        m_tabWidget->setCurrentIndex(m_parts.size()-1);
    }
    m_part->openUrl( url );
}

void CantorShell::setupActions()
{
    KStandardAction::openNew(this, SLOT(fileNew()), actionCollection());
    KStandardAction::open(this, SLOT(fileOpen()), actionCollection());

    KStandardAction::close (this,  SLOT(closeTab()),  actionCollection());

    KStandardAction::quit(qApp, SLOT(closeAllWindows()), actionCollection());

    createStandardStatusBarAction();
    //setStandardToolBarMenuEnabled(true);

    //KStandardAction::keyBindings(this, SLOT(optionsConfigureKeys()), actionCollection());
    //KStandardAction::configureToolbars(this, SLOT(optionsConfigureToolbars()), actionCollection());

    KStandardAction::preferences(this, SLOT(showSettings()), actionCollection());
    KStandardAction::keyBindings( guiFactory(),  SLOT( configureShortcuts() ),  actionCollection() );

    KAction* downloadExamples = new KAction(i18n("Download Example Worksheets"), actionCollection());
    downloadExamples->setIcon(KIcon("get-hot-new-stuff"));
    actionCollection()->addAction("file_example_download",  downloadExamples);
    connect(downloadExamples, SIGNAL(triggered()), this,  SLOT(downloadExamples()));

    KAction* openExample =new KAction(i18n("&Open Example"), actionCollection());
    openExample->setIcon(KIcon("document-open"));
    actionCollection()->addAction("file_example_open", openExample);
    connect(openExample, SIGNAL(triggered()), this, SLOT(openExample()));
}

void CantorShell::saveProperties(KConfigGroup & /*config*/)
{
    // the 'config' object points to the session managed
    // config file.  anything you write here will be available
    // later when this app is restored
}

void CantorShell::readProperties(const KConfigGroup & /*config*/)
{
    // the 'config' object points to the session managed
    // config file.  this function is automatically called whenever
    // the app is being restored.  read in here whatever you wrote
    // in 'saveProperties'
}

void CantorShell::fileNew()
{
    addWorksheet();
}

void CantorShell::optionsConfigureKeys()
{
  /*KShortcutsDialog dlg( KKeyChooser::AllActions, KKeyChooser::LetterShortcutsDisallowed, this );
  dlg.insert( actionCollection(), "cantor_shell.rc" );
  dlg.insert( m_part->actionCollection(), "cantor_part.rc" );
  (void) dlg.configure( true );*/
}

void CantorShell::optionsConfigureToolbars()
{
    //saveMainWindowSettings(KGlobal::config(), autoSaveGroup());

    // use the standard toolbar editor
    /*KEditToolBar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(applyNewToolbarConfig()));
    dlg.exec();*/
}

void CantorShell::applyNewToolbarConfig()
{
    //applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
}

void CantorShell::fileOpen()
{
    // this slot is called whenever the File->Open menu is selected,
    // the Open shortcut is pressed (usually CTRL+O) or the Open toolbar
    // button is clicked
    KUrl url = KFileDialog::getOpenUrl( KUrl(), i18n("*.mws|Cantor Worksheet"), this );

    if (url.isEmpty() == false)
    {
        // About this function, the style guide (
        // http://developer.kde.org/documentation/standards/kde/style/basics/index.html )
        // says that it should open a new window if the document is _not_
        // in its initial state.  This is what we do here..
        /*if ( m_part->url().isEmpty() && ! m_part->isModified() )
        {
            // we open the file in this window...
            load( url );
        }
        else
        {
            // we open the file in a new window...
            CantorShell* newWin = new CantorShell;
            newWin->load( url );
            newWin->show();
            }*/
        load( url );
    }
}

void CantorShell::addWorksheet()
{
    QPointer<BackendChooseDialog> dlg=new BackendChooseDialog(this);
    if(dlg->exec())
    {
        addWorksheet(dlg->backendName());
        activateWorksheet(m_parts.size()-1);
    }

    delete dlg;
}

void CantorShell::addWorksheet(const QString& backendName)
{
    static int sessionCount=1;

    // this routine will find and load our Part.  it finds the Part by
    // name which is a bad idea usually.. but it's alright in this
    // case since our Part is made for this Shell
    KLibFactory *factory = KLibLoader::self()->factory("libcantorpart");
    if (factory)
    {
        // now that the Part is loaded, we cast it to a Part to get
        // our hands on it
        KParts::ReadWritePart* part = dynamic_cast<KParts::ReadWritePart *>(factory->create(m_tabWidget, "CantorPart", QStringList()<<backendName ));

        if (part)
        {
            connect(part, SIGNAL(showHelp(const QString&)), m_helpView, SLOT(setHtml(const QString&)));
            connect(part, SIGNAL(setCaption(const QString&)), this, SLOT(setTabCaption(const QString&)));
            m_parts.append(part);
            m_tabWidget->addTab(part->widget(), i18n("Session %1", sessionCount++));
        }
        else
        {
            kDebug()<<"error creating part ";
        }

    }
    else
    {
        // if we couldn't find our Part, we exit since the Shell by
        // itself can't do anything useful
        KMessageBox::error(this, i18n("Could not find the Cantor Part."));
        qApp->quit();
        // we return here, cause qApp->quit() only means "exit the
        // next time we enter the event loop...
        return;
    }

}

void CantorShell::activateWorksheet(int index)
{
    m_part=m_parts.value(index);
    if(m_part)
        createGUI(m_part);
    else
        kDebug()<<"selected part doesn't exist";

    m_tabWidget->setCurrentIndex(index);
}

void CantorShell::setTabCaption(const QString& caption)
{
    if (caption.isEmpty()) return;

    KParts::ReadWritePart* part=dynamic_cast<KParts::ReadWritePart*>(sender());
    m_tabWidget->setTabText(m_tabWidget->indexOf(part->widget()), caption);
}

void CantorShell::closeTab(QWidget* widget)
{
    if(widget==0) widget=m_part->widget();
    int index=m_tabWidget->indexOf(widget);

    KParts::ReadWritePart* part=m_parts.takeAt(index);
    m_tabWidget->removeTab(index);

    part->deleteLater();
}

void CantorShell::showSettings()
{
    KConfigDialog *dialog = new KConfigDialog(this,  "settings", Settings::self());
    QWidget *generalSettings = new QWidget;
    Ui::SettingsBase base;
    base.setupUi(generalSettings);
    base.kcfg_DefaultBackend->addItems(Cantor::Backend::listAvailableBackends());

    dialog->addPage(generalSettings, i18n("General"), "preferences-other");
    foreach(Cantor::Backend* backend, Cantor::Backend::availableBackends())
    {
        if (backend->config()) //It has something to configure, so add it to the dialog
            dialog->addPage(backend->settingsWidget(dialog), backend->config(), backend->name(),  backend->icon());
    }

    dialog->show();
}

void CantorShell::downloadExamples()
{
    KNS::Entry::List entries = KNS::Engine::download();
    // list of changed entries
    foreach(KNS::Entry* entry,  entries)
    {
        // care only about installed ones
        if (entry->status() == KNS::Entry::Installed)
        {
            //kDebug()<<"downloaded example "<<entry->name();
        }
    }

    qDeleteAll(entries);
}

void CantorShell::openExample()
{
    QString dir = KStandardDirs::locateLocal("appdata",  "examples");
    if (dir.isEmpty()) return;
    KStandardDirs::makeDir(dir);

    QStringList files=QDir(dir).entryList(QDir::Files);
    QPointer<KDialog> dlg=new KDialog(this);
    QListWidget* list=new QListWidget(dlg);
    foreach(const QString& file, files)
    {
        QString name=file;
        name.remove(QRegExp("-.*\\.hotstuff-access$"));
        list->addItem(name);
    }

    dlg->setMainWidget(list);

    if (dlg->exec()==QDialog::Accepted&&list->currentRow()>=0)
    {
        const QString& selectedFile=files[list->currentRow()];
        KUrl url;
        url.setDirectory(dir);
        url.setFileName(selectedFile);

        kDebug()<<"loading file "<<url;
        load(url);
    }

    delete dlg;
}