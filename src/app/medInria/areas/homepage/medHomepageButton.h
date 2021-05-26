/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2018. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <QtGui>
#include <QtWidgets>
#include <medCoreExport.h>

class medHomepagePushButton : public QPushButton
{
    Q_OBJECT   
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected)
    
public:
    medHomepagePushButton ( QWidget * parent = nullptr );

    void setIdentifier(const QString & id);
    QString identifier ();

    bool selected() const;
    bool isSelected() const;

public slots:
    void onClick();
    void setSelected(bool value);

signals:
    /**
     * @brief is emitted when the button is clicked.
     * @param the identifier, or if not present the displayed text.append
     */
    void clicked ( QString text );

private:
    QString id;
    bool m_selected;
};

class medHomepageButton : public QToolButton
{
    Q_OBJECT
public:
    medHomepageButton ( QWidget * parent = nullptr );

    void setIdentifier(const QString & id);
    QString identifier ();
public slots:
    void onClick();

signals:
    /**
     * @brief is emitted when the button is clicked.
     * @param the identifier, or if not present the displayed text.append
     */
    void clicked ( QString text );

private:
    QString id;

};


