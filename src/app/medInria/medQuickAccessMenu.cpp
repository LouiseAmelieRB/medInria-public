/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2019. All rights reserved.
 See LICENSE.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include <medQuickAccessMenu.h>
#include <medWorkspaceFactory.h>
#include <medSettingsManager.h>

#ifdef Q_OS_MAC
Qt::Key OSIndependentControlKey = Qt::Key_Meta;
#else
Qt::Key OSIndependentControlKey = Qt::Key_Control;
#endif

int retrieveDefaultWorkSpace()
{
    int homepageDefaultWorkspaceNumber = 0;
    int iRes = homepageDefaultWorkspaceNumber;

    bool bMatch = false;
    medWorkspaceFactory::Details *poDetail = nullptr;
    QList<medWorkspaceFactory::Details*> oListOfWorkspaceDetails = medWorkspaceFactory::instance()->workspaceDetailsSortedByName(true);
    QVariant oStartupWorkspace = medSettingsManager::instance()->value("startup", "default_starting_area");

    if (oStartupWorkspace.toString() == "Homepage")
    {
        iRes = homepageDefaultWorkspaceNumber;
    }
    else if (oStartupWorkspace.toString() == "Import/export files")
    {
        iRes = 1;
    }
    else if (oStartupWorkspace.toString() == "Composer")
    {
        iRes = 2;
    }
    else
    {
        for (int i = 0; i < oListOfWorkspaceDetails.size() && !bMatch; ++i)
        {
            poDetail = oListOfWorkspaceDetails[i];
            bMatch = poDetail->name == oStartupWorkspace.toString();
            if (bMatch)
            {
                iRes = i+3;
            }
        }
    }

    return iRes;
}

/**
 * Constructor, alt-tab like menu, access through "CTRL-Space"
 */
medQuickAccessMenu::medQuickAccessMenu(QWidget *parent, Qt::WindowFlags f) : QWidget(parent, f)
{
    currentSelected = retrieveDefaultWorkSpace();

    this->createHorizontalQuickAccessMenu();
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground, true);
}

/**
 *  emit the menuHidden() signal when the widget lost the focus
 */
void medQuickAccessMenu::focusOutEvent ( QFocusEvent *event )
{
    QWidget::focusOutEvent ( event );
    emit menuHidden();
}

/**
 * Handles key press events to control selected widget in menu
 */
void medQuickAccessMenu::keyPressEvent ( QKeyEvent *event )
{
    if ((event->key() == Qt::Key_Down)||(event->key() == Qt::Key_Right))
    {
        this->updateCurrentlySelectedRight();
        return;
    }

    if ((event->key() == Qt::Key_Up)||(event->key() == Qt::Key_Left))
    {
        this->updateCurrentlySelectedLeft();
        return;
    }

    if (event->key() == Qt::Key_Return)
    {
        this->switchToCurrentlySelected();
        return;
    }

    if (event->key() == Qt::Key_Escape)
    {
        emit menuHidden();
        return;
    }

    QApplication::sendEvent(this->parentWidget(), event);
}

/**
 * Send back key release event to parent widget that handles key releases
 */
void medQuickAccessMenu::keyReleaseEvent ( QKeyEvent *event )
{
    if (event->key() == OSIndependentControlKey && this->isVisible())
    {
        emit menuHidden();
        switchToCurrentlySelected();
        return;
    }

    if (event->key() == Qt::Key_Space && event->modifiers().testFlag(Qt::ShiftModifier))
    {
        updateCurrentlySelectedLeft();
        return;
    }

    QApplication::sendEvent(this->parentWidget(), event);
}

/**
 * Mouse over implementation to select widget in menu
 */
void medQuickAccessMenu::mouseMoveEvent (QMouseEvent *event)
{
    for (int i = 0; i < buttonsList.size(); ++i)
    {
        QRect widgetRect = buttonsList[i]->geometry();
        if (widgetRect.contains(event->pos()))
        {
            this->mouseSelectWidget(i);
            break;
        }
    }

    QWidget::mouseMoveEvent(event);
}

/**
 * Updates the currently selected item when current workspace was changed from somewhere else
 */
void medQuickAccessMenu::updateSelected (QString workspace)
{
    currentSelected = 0;

    for (int i = 0; i < buttonsList.size(); ++i)
    {
        if (buttonsList[i]->identifier() == workspace)
        {
            currentSelected = i;
            break;
        }
    }
}

/**
 * Actually emits signal to switch to selected workspace
 */
void medQuickAccessMenu::switchToCurrentlySelected()
{
    if (currentSelected < 0)
    {
        emit menuHidden();
        return;
    }

    if (currentSelected == 0)
    {
        emit homepageSelected();
        return;
    }

    if (currentSelected == 1)
    {
        emit browserSelected();
        return;
    }

    if (currentSelected == 2)
    {
        emit composerSelected();
        return;
    }

    if (currentSelected >= 3)
    {
        emit workspaceSelected(buttonsList[currentSelected]->identifier());
        return;
    }
}

/**
 * Move selected widget one position to the left
 */
void medQuickAccessMenu::updateCurrentlySelectedLeft()
{
    if (currentSelected <= 0)
    {
        if (currentSelected == 0)
            buttonsList[currentSelected]->setSelected(false);

        currentSelected = buttonsList.size() - 1;
        buttonsList[currentSelected]->setSelected(true);

        return;
    }

    buttonsList[currentSelected]->setSelected(false);
    currentSelected--;
    buttonsList[currentSelected]->setSelected(true);
}

/**
 * Move selected widget one position to the right
 */
void medQuickAccessMenu::updateCurrentlySelectedRight()
{
    if (currentSelected < 0)
    {
        currentSelected = 0;
        buttonsList[currentSelected]->setSelected(true);
        return;
    }

    buttonsList[currentSelected]->setSelected(false);

    currentSelected++;
    if (currentSelected >= buttonsList.size())
        currentSelected = 0;

    buttonsList[currentSelected]->setSelected(true);
}

/**
 * Resets the menu when shown, optionally updates the layout to window size
 */
void medQuickAccessMenu::reset(bool optimizeLayout)
{
    for (int i = 0; i < buttonsList.size(); ++i)
    {
        if (currentSelected == i)
            buttonsList[i]->setSelected(true);
        else
            buttonsList[i]->setSelected(false);
    }

    if ((optimizeLayout)&&(backgroundFrame))
    {
        unsigned int width = ((QWidget*)this->parent())->size().width();
        unsigned int numberOfWidgetsPerLine = floor((width - 40.0) / 180.0);
        unsigned int optimalSizeLayout = ceil((float)buttonsList.size() / numberOfWidgetsPerLine);

        if (backgroundFrame->layout())
            delete backgroundFrame->layout();

        if (optimalSizeLayout > 1)
        {
            QGridLayout *layout = new QGridLayout;
            unsigned int totalAdded = 0;
            for (unsigned int i = 0; i < optimalSizeLayout; ++i)
            {
                for (unsigned int j = 0; j < numberOfWidgetsPerLine; ++j)
                {
                    layout->addWidget(buttonsList[totalAdded], i, j);
                    ++totalAdded;

                    if (totalAdded == (unsigned int)buttonsList.size())
                        break;
                }

                if (totalAdded == (unsigned int)buttonsList.size())
                    break;
            }

            backgroundFrame->setLayout(layout);
            backgroundFrame->setFixedWidth ( 40 + 180 * numberOfWidgetsPerLine );
            backgroundFrame->setFixedHeight ( 130 * optimalSizeLayout );
            this->setFixedWidth ( 40 + 180 * numberOfWidgetsPerLine );
            this->setFixedHeight ( 130 * optimalSizeLayout );
        }
        else
        {
            QHBoxLayout *layout = new QHBoxLayout;
            for (int i = 0; i < buttonsList.size(); ++i)
            {
                layout->addWidget(buttonsList[i]);
            }

            backgroundFrame->setLayout(layout);
            backgroundFrame->setFixedWidth ( 40 + 180 * buttonsList.size() );
            backgroundFrame->setFixedHeight ( 130 );

            this->setFixedWidth ( 40 + 180 * buttonsList.size() );
            this->setFixedHeight ( 130 );
        }
    }
}

/**
 * Mouse over slot to select widget in menu
 */
void medQuickAccessMenu::mouseSelectWidget(unsigned int identifier)
{
    unsigned int newSelection = identifier;

    if (newSelection == (unsigned int)currentSelected)
        return;

    if (currentSelected >= 0)
        buttonsList[currentSelected]->setSelected(false);

    buttonsList[newSelection]->setSelected(true);
    currentSelected = newSelection;
}

/**
 * Horizontal menu layout creation method
 */
void medQuickAccessMenu::createHorizontalQuickAccessMenu()
{
    buttonsList.clear();

    backgroundFrame = new QFrame(this);
    backgroundFrame->setStyleSheet("border-radius: 10px;background-color: rgba(200,200,200,150);");
    QHBoxLayout *mainWidgetLayout = new QHBoxLayout;
    mainWidgetLayout->setMargin(0);
    mainWidgetLayout->setSpacing ( 0 );

    QHBoxLayout *shortcutAccessLayout = new QHBoxLayout;

    medHomepagePushButton *smallHomeButton = new medHomepagePushButton ( this );
    smallHomeButton->setFixedHeight ( 100 );
    smallHomeButton->setFixedWidth ( 160 );
    smallHomeButton->setStyleSheet("border-radius: 5px;font-size:12px;color: #ffffff;background-image: url(:pixmaps/home_sc.png) no-repeat;");
    smallHomeButton->setFocusPolicy ( Qt::NoFocus );
    smallHomeButton->setCursor(Qt::PointingHandCursor);
    smallHomeButton->setIdentifier("Homepage");
    smallHomeButton->setText("Homepage");
    smallHomeButton->setSelected(true);
    shortcutAccessLayout->addWidget (smallHomeButton);
    QObject::connect ( smallHomeButton, SIGNAL ( clicked() ), this, SIGNAL ( homepageSelected() ) );
    buttonsList.push_back(smallHomeButton);

    //Setup browser access button
    medHomepagePushButton *smallBrowserButton = new medHomepagePushButton ( this );
    smallBrowserButton->setCursor(Qt::PointingHandCursor);
    smallBrowserButton->setStyleSheet("border-radius: 5px;font-size:12px;color: #ffffff;background-image: url(:pixmaps/database.png) no-repeat;");
    smallBrowserButton->setFixedHeight ( 100 );
    smallBrowserButton->setFixedWidth ( 160 );
    smallBrowserButton->setFocusPolicy ( Qt::NoFocus );
    smallBrowserButton->setText("Import/export files");
    smallBrowserButton->setIdentifier("Browser");
    shortcutAccessLayout->addWidget ( smallBrowserButton );
    QObject::connect ( smallBrowserButton, SIGNAL ( clicked() ), this, SIGNAL ( browserSelected()) );
    buttonsList.push_back(smallBrowserButton);

    //Setup composer access button
    medHomepagePushButton *smallComposerButton = new medHomepagePushButton(this);
    smallComposerButton->setCursor(Qt::PointingHandCursor);
    smallComposerButton->setStyleSheet("border-radius: 5px;font-size:12px;color: #ffffff;background-image: url(:pixmaps/composer_sc.png) no-repeat;");
    smallComposerButton->setFixedHeight(100);
    smallComposerButton->setFixedWidth(160);
    smallComposerButton->setFocusPolicy(Qt::NoFocus);
    smallComposerButton->setText("Composer");
    smallComposerButton->setIdentifier("Composer");
    shortcutAccessLayout->addWidget(smallComposerButton);
    QObject::connect(smallComposerButton, SIGNAL(clicked()), this, SIGNAL(composerSelected()));
    buttonsList.push_back(smallComposerButton);

    QList<medWorkspaceFactory::Details*> workspaceDetails = medWorkspaceFactory::instance()->workspaceDetailsSortedByName();
    unsigned int numActiveWorkspaces = 0;
    for( medWorkspaceFactory::Details* detail : workspaceDetails )
    {
        if (!detail->isActive)
            continue;

        ++numActiveWorkspaces;
        medHomepagePushButton *button = new medHomepagePushButton ( this );
        button->setText ( detail->name );
        button->setFocusPolicy ( Qt::NoFocus );
        button->setCursor(Qt::PointingHandCursor);
        button->setFixedHeight ( 100 );
        button->setFixedWidth ( 160 );
        button->setStyleSheet("border-radius: 5px;font-size:12px;color: #ffffff;background-image: url(:pixmaps/workspace_" + detail->name + ".png) no-repeat;");
        button->setIdentifier(detail->identifier);
        shortcutAccessLayout->addWidget ( button );
        QObject::connect ( button, SIGNAL ( clicked ( QString ) ), this, SIGNAL ( workspaceSelected ( QString ) ) );
        buttonsList.push_back(button);
    }

    backgroundFrame->setLayout(shortcutAccessLayout);
    backgroundFrame->setFixedWidth ( 40 + 180 * ( 2 + numActiveWorkspaces ) );
    backgroundFrame->setFixedHeight ( 240 );
    backgroundFrame->setMouseTracking(true);
    mainWidgetLayout->addWidget(backgroundFrame);
    this->setFixedWidth ( 40 + 180 * ( 2 + numActiveWorkspaces ) );
    this->setFixedHeight ( 240 );
    this->setLayout(mainWidgetLayout);
}
