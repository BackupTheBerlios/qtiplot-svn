/***************************************************************************
    File                 : Note.h
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Tilman Hoener zu Siederdissen
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net
    Description          : Notes window class

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#ifndef NOTE_H
#define NOTE_H

#include "MdiSubWindow.h"
#include "ScriptEdit.h"
#include "LineNumberDisplay.h"
#include <QTextEdit>

class ScriptingEnv;

/*!\brief Notes window class.
 *
 * \section future Future Plans
 * - Search and replace
 */
class Note: public MdiSubWindow
{
    Q_OBJECT

public:

	Note(ScriptingEnv *env, const QString& label, ApplicationWindow* parent, const QString& name = QString(), Qt::WFlags f=0);
	~Note(){};

	void init(ScriptingEnv *env);
	void setName(const QString& name);

public slots:
	QString saveToString(const QString &info, bool = false);
	void restore(const QStringList&);

	ScriptEdit* editor(){return te;};
	bool autoexec() const { return autoExec; }
	void setAutoexec(bool);
	void modifiedNote();

	// ScriptEdit methods
	QString text() { return te->text(); };
	void setText(const QString &s) { te->setText(s); };
	void print() { te->print(); };
	void exportPDF(const QString& fileName){te->exportPDF(fileName);};
	QString exportASCII(const QString &file=QString::null) { return te->exportASCII(file); };
	QString importASCII(const QString &file=QString::null) { return te->importASCII(file); };
	void execute() { te->execute(); };
	void executeAll() { te->executeAll(); };
	void evaluate() { te->evaluate(); };
	void setDirPath(const QString& path){te->setDirPath(path);};

	//! Enables/Disables the line number display
	void showLineNumbers(bool show = true){d_line_number->setVisible(show);};
	bool hasLineNumbers(){return d_line_number->isVisible();};

 signals:
	void dirPathChanged(const QString& path);

private:
	ScriptEdit *te;
	LineNumberDisplay *d_line_number;
	QWidget *d_frame;
	bool autoExec;
};

#endif
