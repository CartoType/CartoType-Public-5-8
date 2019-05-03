#include "mapchildwindow.h"

MapChildWindow::MapChildWindow(QWidget* aParent,MainWindow& aMainWindow,const QString& aMapFileName):
    QMdiSubWindow(aParent),
    m_map_form(nullptr)
    {
    setMinimumSize(250,150);
    m_map_form = new MapForm(this,aMainWindow,aMapFileName);
    setWidget(m_map_form);

    // Set the document icon, but not on the Mac, where it doesn't look that good.
#ifndef Q_OS_MAC
    QIcon icon(":/CT_Arrow-RGB-256x256.png");
    setWindowIcon(icon);
#endif
    }

MapChildWindow::~MapChildWindow()
    {
    m_map_form->~MapForm();
    }

void MapChildWindow::SetView(const CartoType::TViewState& aViewState)
    {
    resize(aViewState.iWidthInPixels,aViewState.iHeightInPixels);
    m_map_form->SetView(aViewState);
    }
