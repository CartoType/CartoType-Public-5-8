#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mapchildwindow.h"
#include "util.h"
#include "styledialog.h"
#include "ui_styledialog.h"

#include <QFileDialog>
#include <QMessageBox>

#include <stdio.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_ui(new Ui::MainWindow),
    m_map_form(nullptr),
    m_app_path(QCoreApplication::applicationDirPath()),
    m_style_dialog_map_form(nullptr)
    {
    m_ui->setupUi(this);

    m_print_separator = m_ui->menuFile->addSeparator();
    m_recent_file_separator = m_ui->menuFile->addSeparator();
    for (int i = 0; i < KMaxRecentFiles; i++)
        {
        m_recent_file_action[i] = new QAction(this);
        m_recent_file_action[i]->setVisible(false);
        connect(m_recent_file_action[i],SIGNAL(triggered()),this,SLOT(OpenRecentFileTriggered()));
        m_ui->menuFile->addAction(m_recent_file_action[i]);
        }

    // Set the app icon. On Windows and Mac that's done in other ways (see the end of the .pro file).
#if !defined(Q_OS_WIN32) && !defined(Q_OS_MAC)
    QIcon icon(":/CT_Arrow-RGB-256x256.png");
    QApplication::setWindowIcon(icon);
#endif

    // Set up the CartoType framework.
    if (m_app_path.length() == 0 || m_app_path[m_app_path.length() - 1] != '/')
        m_app_path += '/';

    // Strip the path down to a string ending just before /src/apps/; if no such pattern is found, use the app path
    m_cartotype_source_path = m_app_path;
    int n = m_cartotype_source_path.lastIndexOf("/src/apps/");
    if (n != -1)
        m_cartotype_source_path.truncate(n + 1);
    if (m_cartotype_source_path.length() == 0 || m_cartotype_source_path[m_cartotype_source_path.length() - 1] != '/')
        m_cartotype_source_path += '/';
    InitCartoType();

    // Disable menu items not used when there is no map window.
    EnableMenuItems();

    // Update the recent file list and remove nonexistent files.
    UpdateRecentFiles(true);

    // If there's a command line argument not starting with '-', load it as a map.
    bool map_loaded = false;
    int arg_count = QCoreApplication::arguments().size();
    for (int i = 1; i < arg_count; i++)
        {
        QString p = QCoreApplication::arguments().at(i);
        if (p[0] != '-')
            {
            LoadMap(p);
            map_loaded = true;
            }
        }

    // If the main window geometry, and the geometry and states of the child windows, have been saved, restore those settings.
    QSettings settings;
    if (!map_loaded)
        {
        settings.beginGroup("mainWindow");
        QByteArray main_window_geometry = settings.value("geometry").toByteArray();
        settings.endGroup();
        if (main_window_geometry.isEmpty())
            setWindowState(Qt::WindowMaximized);
        else
            restoreGeometry(main_window_geometry);

        // Open the saved map windows.
        int saved_map_window_count = settings.beginReadArray("mapWindows");
        for (int index = 0; index < saved_map_window_count; index++)
            {
            settings.setArrayIndex(index);
            QByteArray geometry = settings.value("geometry").toByteArray();
            QString filename = settings.value("fileName").toString();
            QString view_state = settings.value("viewState").toString();
            LoadMap(filename,&geometry,&view_state);
            map_loaded = true;
            }
        settings.endArray();
        }

    // Load the first file from the recent file list, or if there aren't any, show a welcome message.
    QStringList files = settings.value("recentFileList").toStringList();
    if (!map_loaded)
        {
        if (files.size() > 0)
            LoadMap(*files.begin());
        else
            QMessageBox::about(this,"Welcome to CartoType Maps","The <b>CartoType Maps</b> app allows you to view maps, calculate routes, find places, and more.<br/><br/>"
                               "Please open a map (a .ctm1 file). Some sample maps were provided with this application. "
                               "Right-click on the map to set the start or end of a route, add a pushpin or see what is there.<br/><br/>"
                               "See <a href='http://www.cartotype.com'>cartotype.com</a> for information about creating maps, using the API, and licensing CartoType for your application.");
        }

    // Read the custom route profile from the settings.
    QString profile = settings.value("customRouteProfile").toString();
    if (!profile.isEmpty())
        {
        QByteArray profile_utf8 = profile.toUtf8();
        CartoType::TMemoryInputStream input((const uint8_t*)profile_utf8.data(),profile_utf8.length());
        CartoType::TRouteProfile p;
        CartoType::TResult error = p.ReadFromXml(input);
        if (!error)
            m_custom_route_profile = p;
        }

    // Read the custom style sheet from the settings.
    QString style = settings.value("customStyleSheet").toString();
    if (!style.isEmpty())
        m_custom_style_sheet = style.toStdString();
    }

MainWindow::~MainWindow()
    {
    delete m_ui;
    }

void MainWindow::closeEvent(QCloseEvent* aEvent)
    {
    bool dirty = false;
    bool accept = false;
    QList<QMdiSubWindow*> sub_window_list = m_ui->mdiArea->subWindowList(QMdiArea::StackingOrder);
    for (auto p : sub_window_list)
        {
        MapChildWindow* w = dynamic_cast<MapChildWindow*>(p);
        if (w->m_map_form && w->m_map_form->WritableDataChanged())
            {
            dirty = true;
            break;
            }
        }

    if (dirty)
        {
        QMessageBox message_box(QMessageBox::Question,"Quit without saving","You have unsaved added data. Close without saving it?",
                                QMessageBox::Yes | QMessageBox::Cancel);
        QMessageBox::StandardButton button = (QMessageBox::StandardButton)message_box.exec();
        if (button == QMessageBox::Yes)
            accept = true;
        }
    else
        accept = true;

    if (accept)
        {
        // Write the geometry of the main window.
        QSettings settings;
        settings.beginGroup("mainWindow");
        settings.setValue("geometry",saveGeometry());
        settings.endGroup();

        // Write the view states of all the map windows.
        settings.beginWriteArray("mapWindows");
        int index = 0;
        for (auto p : sub_window_list)
            {
            MapChildWindow* w = dynamic_cast<MapChildWindow*>(p);
            if (w->m_map_form)
                {
                settings.setArrayIndex(index);
                settings.setValue("geometry",w->saveGeometry());
                settings.setValue("fileName",w->m_map_form->FileName());
                settings.setValue("viewState",w->m_map_form->ViewState());
                index++;
                }
            }
        settings.endArray();

        aEvent->accept();
        }
    else
        aEvent->ignore();
    }

void MainWindow::OpenRecentFileTriggered()
    {
    QAction *action = qobject_cast<QAction*>(sender());
    if (action)
        LoadMap(action->data().toString());
    }

void MainWindow::on_actionOpen_triggered()
    {
    // Open a new document in a child window.
    QString map_filename = QFileDialog::getOpenFileName(this,"Open a map","","CartoType maps (*.ctm1)");
    if (map_filename.length())
        LoadMap(map_filename);
    }

void MainWindow::LoadMap(const QString& aPath,const QByteArray* aWindowGeometry,const QString* aViewState)
    {
    auto SetWindowState = [aWindowGeometry,aViewState](MapChildWindow* aWindow)
        {
        if (aWindowGeometry)
            aWindow->restoreGeometry(*aWindowGeometry);
        if (aViewState)
            {
            CartoType::TViewState v;
            v.ReadFromXml(aViewState->toStdString().c_str());
            aWindow->SetView(v);
            }

        // The following code fixes a bug on Qt on Linux causing maximized windows to be restored in the wrong place.
        if (aWindow->isMaximized())
            {
            aWindow->showNormal();
            aWindow->showMaximized();
            }
        };

    // If this map has already been loaded, just bring it to the front.
    QList<QMdiSubWindow*> sub_window_list = m_ui->mdiArea->subWindowList();
    for (auto p : sub_window_list)
        {
        MapChildWindow* w = dynamic_cast<MapChildWindow*>(p);
        if (w->m_map_form && w->m_map_form->FileName() == aPath)
            {
            w->raise();
            w->showMaximized();
            SetWindowState(w);
            return;
            }
        }

    MapChildWindow* w = new MapChildWindow(m_ui->mdiArea,*this,aPath);
    w->setAttribute(Qt::WA_DeleteOnClose);
    if (w->m_map_form == nullptr || !w->m_map_form->Valid())
        w->close();
    else
        {
        w->showMaximized();
        SetWindowState(w);

        QSettings settings;
        QStringList files = settings.value("recentFileList").toStringList();
        files.removeAll(aPath);
        files.prepend(aPath);
        while (files.size() > KMaxRecentFiles)
            files.removeLast();
        settings.setValue("recentFileList",files);
        settings.sync();

        UpdateRecentFiles();
        }
    }

void MainWindow::OnMapFormDestroyed(MapForm* /*aMapForm*/)
    {
    }

void MainWindow::on_actionAbout_CartoType_Maps_triggered()
    {
    QString text = "The <b>CartoType Maps</b> app allows you to view maps, calculate routes, find places, and more.<br/><br/>"
                   "See <a href='http://www.cartotype.com'>cartotype.com</a> for information about creating maps, using the API, and licensing CartoType for your application.<br/><br/><br/>"
                   "Created using CartoType ";
    text += QString(CartoType::Version()) + "." + CartoType::Build() + ".";
    QMessageBox::about(this,"About CartoType Maps",text);
    }

void MainWindow::GetFontPath(CartoType::CString& aPath,const char* aFontName)
    {
    QString font_path(m_app_path + aFontName);
    if (!QFileInfo::exists(font_path))
        {
        font_path = m_app_path + "font/" + aFontName;
        if (!QFileInfo::exists(font_path))
            font_path = m_cartotype_source_path + "font/" + aFontName;
        }
    assert(sizeof(QChar) == sizeof(uint16_t));
    aPath.Set((const uint16_t*)font_path.constData(),font_path.length());
    }

void MainWindow::ShowError(const char* aMessage,int aErrorCode)
    {
    char buffer[1024];
    if (aErrorCode)
        {
        sprintf(buffer,"%s: %s (error code %d)",aMessage,CartoType::ErrorString(aErrorCode),aErrorCode);
        QString text(buffer);
        QMessageBox::critical(this,"CartoType error",text);
        }
    else
        {
        QString text(aMessage);
        QMessageBox::information(this,"Information",text);
        }
    }

CartoType::TResult MainWindow::InitCartoType()
    {
    // Get the path of the default font.
    CartoType::CString font_filename;
    GetFontPath(font_filename,"DejaVuSans.ttf");

    // Create the engine, loading the default font.
    CartoType::TResult error = 0;
    m_engine = CartoType::CFrameworkEngine::New(error,font_filename);
    if (!m_engine || error)
        {
        ShowError("could not create CartoType engine",error);
        return error;
        }

    // Load extra fonts.
    error = LoadFont("DejaVuSans-Bold.ttf");
    if (!error)
        error = LoadFont("DejaVuSerif.ttf");
    if (!error)
        error = LoadFont("DejaVuSerif-Italic.ttf");
    if (!error)
        error = LoadFont("DroidSansFallback.ttf");
    if (!error)
        error = LoadFont("MapkeyIcons.ttf");

    /*
    Find the default style sheet.
    Paths are searched in this order:
    the application directory;
    the subdirectory "style" of the application directory;
    the directory "CartoType/src/style" in the development tree.
    */
    QString style_sheet_name("standard.ctstyle");
    QString style_sheet_path = m_app_path + style_sheet_name;
    if (!QFileInfo::exists(style_sheet_path))
        {
        style_sheet_path = m_app_path + "style/" + style_sheet_name;
        if (!QFileInfo::exists(style_sheet_path))
            style_sheet_path = m_cartotype_source_path + "style/" + style_sheet_name;
        }
    if (!error)
        m_default_style_sheet_path.Set(style_sheet_path.utf16(),style_sheet_path.length());

    // For debugging: show the style sheet path.
    // CartoType::CUtf8String style_utf8(m_default_style_sheet_path);
    // ShowError(style_utf8,0);

    return error;
    }

/**
Search for a font file and load it. Paths are searched in this order:
the application directory; the development tree directory; the subdirectory 'font' of the application directory
*/
CartoType::TResult MainWindow::LoadFont(const char* aFontName)
    {
    CartoType::CString filename;
    GetFontPath(filename,aFontName);
    CartoType::TResult error = m_engine->LoadFont(filename);
    if (error)
        {
        std::string text("could not load font file ");
        text += std::string(aFontName);
        ShowError(text.c_str(),error);
        }
    return error;
    }

void MainWindow::on_mdiArea_subWindowActivated(QMdiSubWindow* aSubWindow)
    {
    MapChildWindow* p = dynamic_cast<MapChildWindow*>(aSubWindow);

    if (p)
        m_map_form = p->m_map_form;
    else
        m_map_form = nullptr;

    EnableMenuItems();
    }

void MainWindow::on_actionScale_changed()
    {
    if (m_map_form)
        m_map_form->EnableDrawScale(m_ui->actionScale->isChecked());
    }

void MainWindow::on_actionLegend_changed()
    {
    if (m_map_form)
        m_map_form->EnableDrawLegend(m_ui->actionLegend->isChecked());
    }

void MainWindow::on_actionFind_triggered()
    {
    if (m_map_form)
        m_map_form->Find();
    }

void MainWindow::on_actionFind_Next_triggered()
    {
    if (m_map_form)
        m_map_form->FindNext();
    }

void MainWindow::EnableMenuItems()
    {
    if (m_map_form)
        {
        menuBar()->insertAction(m_ui->menuHelp->menuAction(),m_ui->menuFind->menuAction());
        menuBar()->insertAction(m_ui->menuHelp->menuAction(),m_ui->menuView->menuAction());
        menuBar()->insertAction(m_ui->menuHelp->menuAction(),m_ui->menuRoute->menuAction());
        menuBar()->insertAction(m_ui->menuHelp->menuAction(),m_ui->menuStyle->menuAction());
        m_ui->menuFile->insertAction(m_recent_file_separator,m_ui->actionPrint);
        m_ui->menuFile->insertAction(m_recent_file_separator,m_ui->actionPrint_Preview);
        m_ui->menuFile->insertAction(m_recent_file_separator,m_ui->actionSave_Image_as_PNG);
        m_ui->menuFile->insertAction(m_recent_file_separator,m_ui->actionSave_Added_Data_as_CTMS);
        m_ui->menuFile->insertAction(m_recent_file_separator,m_ui->actionImport_Data_from_CTMS);
        m_ui->menuFile->insertAction(m_recent_file_separator,m_ui->actionImport_Data_from_GPX_file);
        m_ui->actionScale->setChecked(m_map_form->DrawScaleEnabled());
        m_ui->actionRotator->setChecked(m_map_form->DrawRotatorEnabled());
        m_ui->actionLegend->setChecked(m_map_form->DrawLegendEnabled());
        m_ui->actionRange->setChecked(m_map_form->DrawRangeEnabled());
        m_ui->actionPerspective_View->setChecked(m_map_form->Perspective());
        m_ui->action3D_Buildings->setChecked(m_map_form->Draw3DBuildings());
        m_ui->actionMetric_Units->setChecked(m_map_form->MetricUnits());
        m_ui->actionGraphics_Acceleration->setChecked(m_map_form->GraphicsAcceleration());
        m_ui->actionTurn_expanded_router->setChecked(m_map_form->PreferredRouterType() == CartoType::TRouterType::TurnExpandedAStar);
        UpdateSaveAddedData();
        UpdateFindNext();
        UpdateNorthUp();
        UpdateDeleteOrSaveRoute();
        UpdateDeletePushpins();
        UpdateRouteProfile();
        UpdateGoToGridRef();
        UpdateStyleSheet();
        }
    else
        {
        menuBar()->removeAction(m_ui->menuFind->menuAction());
        menuBar()->removeAction(m_ui->menuView->menuAction());
        menuBar()->removeAction(m_ui->menuRoute->menuAction());
        menuBar()->removeAction(m_ui->menuStyle->menuAction());
        m_ui->menuFile->removeAction(m_ui->actionPrint);
        m_ui->menuFile->removeAction(m_ui->actionPrint_Preview);
        m_ui->menuFile->removeAction(m_ui->actionSave_Image_as_PNG);
        m_ui->menuFile->removeAction(m_ui->actionSave_Added_Data_as_CTMS);
        m_ui->menuFile->removeAction(m_ui->actionImport_Data_from_CTMS);
        m_ui->menuFile->removeAction(m_ui->actionImport_Data_from_GPX_file);
        }
    }

void MainWindow::UpdateRecentFiles(bool aRemoveNonExistentFiles)
    {
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    int file_action_index = 0;
    int file_index = 0;
    while (file_index < files.size())
        {
        QFileInfo file_info(files[file_index]);
        if (aRemoveNonExistentFiles && !file_info.exists())
            {
            files.erase(files.begin() + file_index);
            continue;
            }

        if (file_action_index < KMaxRecentFiles)
            {
            QString text;
            text.setNum(file_action_index + 1);
            text += " ";
            text += file_info.fileName();
            m_recent_file_action[file_action_index]->setText(text);
            m_recent_file_action[file_action_index]->setData(files[file_index]);
            m_recent_file_action[file_action_index]->setVisible(true);
            file_action_index++;
            }

        file_index++;
        }

    while (file_action_index < KMaxRecentFiles)
        m_recent_file_action[file_action_index++]->setVisible(false);

    if (aRemoveNonExistentFiles)
        {
        settings.setValue("recentFileList",files);
        settings.sync();
        }
    }

void MainWindow::on_actionRotator_changed()
    {
    if (m_map_form)
        m_map_form->EnableDrawRotator(m_ui->actionRotator->isChecked());
    }

void MainWindow::on_actionRange_changed()
    {
    if (m_map_form)
        m_map_form->EnableDrawRange(m_ui->actionRange->isChecked());
    }

void MainWindow::on_actionNorth_Up_triggered()
    {
    if (m_map_form)
        m_map_form->SetRotation(0);
    }

void MainWindow::UpdateSaveAddedData()
    {
    if (m_map_form)
        m_ui->actionSave_Added_Data_as_CTMS->setEnabled(m_map_form->HasWritableData());
    }

void MainWindow::UpdateFindNext()
    {
    if (m_map_form)
        m_ui->actionFind_Next->setEnabled(m_map_form->FoundItemCount() > 1);
    }

void MainWindow::UpdateNorthUp()
    {
    if (m_map_form)
        m_ui->actionNorth_Up->setDisabled(m_map_form->Rotation() == 0);
    }

void MainWindow::UpdatePerspective()
    {
    if (m_map_form)
        m_ui->actionPerspective_View->setChecked(m_map_form->Perspective());
    }

void MainWindow::on_actionDelete_Route_triggered()
    {
    if (m_map_form)
        m_map_form->DeleteRoute();
    }

void MainWindow::on_actionDelete_Pushpins_triggered()
    {
    if (m_map_form)
        m_map_form->DeletePushpins();
    }

void MainWindow::UpdateDeleteOrSaveRoute()
    {
    bool has_route = m_map_form && m_map_form->HasRoute();
    m_ui->actionReverse_Route->setEnabled(has_route);
    m_ui->actionDelete_Route->setEnabled(has_route);
    m_ui->actionSave_Route->setEnabled(has_route);
    m_ui->actionSave_Route_Instructions->setEnabled(has_route);
    m_ui->actionSave_Route_as_GPX->setEnabled(has_route);
    m_ui->actionView_Route_Instructions->setEnabled(has_route);
    }

void MainWindow::UpdateDeletePushpins()
    {
    bool has_pushpins = m_map_form && m_map_form->HasPushpins();
    m_ui->actionDelete_Pushpins->setEnabled(has_pushpins);
    }

void MainWindow::UpdateRouteProfile()
    {
    if (m_map_form)
        {
        TExtendedRouteProfileType p = m_map_form->RouteProfileType();
        m_ui->actionDrive->setChecked(p == TExtendedRouteProfileType::Drive);
        m_ui->actionCycle->setChecked(p == TExtendedRouteProfileType::Cycle);
        m_ui->actionWalk->setChecked(p == TExtendedRouteProfileType::Walk);
        m_ui->actionHike->setChecked(p == TExtendedRouteProfileType::Hike);
        m_ui->actionCustom_Profile->setChecked(p == TExtendedRouteProfileType::Custom);
        }
    }

void MainWindow::UpdateGoToGridRef()
    {
    if (m_map_form)
        m_ui->actionGo_to_Ordnance_Survey_grid_reference->setEnabled(m_map_form->MapIncludesGreatBritain());
    }

void MainWindow::UpdateStyleSheet()
    {
    if (m_map_form)
        {
        bool custom = m_map_form->UsingCustomStyleSheet();
        m_ui->actionUse_Custom_Style_Sheet->setChecked(custom);
        m_ui->actionChoose_Style_Sheet->setEnabled(!custom);
        m_ui->actionReload_Style_Sheet->setEnabled(!custom);
        m_ui->actionEdit_Custom_Style_Sheet->setEnabled(!m_style_dialog);
        }
    }

void MainWindow::on_actionDrive_triggered()
    {
    if (m_map_form && m_ui->actionDrive->isChecked())
        m_map_form->SetRouteProfileType(TExtendedRouteProfileType::Drive);
    }

void MainWindow::on_actionCycle_triggered()
    {
    if (m_map_form && m_ui->actionCycle->isChecked())
        m_map_form->SetRouteProfileType(TExtendedRouteProfileType::Cycle);
    }

void MainWindow::on_actionWalk_triggered()
    {
    if (m_map_form && m_ui->actionWalk->isChecked())
        m_map_form->SetRouteProfileType(TExtendedRouteProfileType::Walk);
    }

void MainWindow::on_actionHike_triggered()
    {
    if (m_map_form && m_ui->actionHike->isChecked())
        m_map_form->SetRouteProfileType(TExtendedRouteProfileType::Hike);
    }

void MainWindow::on_actionCustom_Profile_triggered()
    {
    if (m_map_form && m_ui->actionCustom_Profile->isChecked())
        m_map_form->SetRouteProfileType(TExtendedRouteProfileType::Custom);
    }

void MainWindow::on_actionTurn_expanded_router_triggered()
    {
    if (m_map_form && m_ui->actionTurn_expanded_router->isChecked())
        m_map_form->SetPreferredRouterType(CartoType::TRouterType::TurnExpandedAStar);
    else
        m_map_form->SetPreferredRouterType(CartoType::TRouterType::Default);
    }

void MainWindow::on_actionSave_Route_Instructions_triggered()
    {
    if (m_map_form)
        m_map_form->SaveRouteInstructions();
    }

void MainWindow::on_actionSave_Route_triggered()
    {
    if (m_map_form)
        m_map_form->SaveRouteAsXml();
    }

void MainWindow::on_actionSave_Route_as_GPX_triggered()
    {
    if (m_map_form)
        m_map_form->SaveRouteAsGpx();
    }

void MainWindow::on_actionLoad_Route_triggered()
    {
    if (m_map_form)
        {
        m_map_form->LoadRouteFromXml();
        UpdateDeleteOrSaveRoute();
        }
    }

void MainWindow::on_actionSave_Image_as_PNG_triggered()
    {
    if (m_map_form)
        m_map_form->SaveImageAsPng();
    }

void MainWindow::on_actionFind_Address_triggered()
    {
    if (m_map_form)
        m_map_form->FindAddress();
    }

void MainWindow::on_actionSave_Added_Data_as_CTMS_triggered()
    {
    if (m_map_form)
        m_map_form->SaveWritableDataAsCtms();
    }

void MainWindow::on_actionImport_Data_from_CTMS_triggered()
    {
    if (m_map_form)
        m_map_form->LoadWritableDataFromCtms();
    }

void MainWindow::on_actionImport_Data_from_GPX_file_triggered()
    {
    if (m_map_form)
        m_map_form->LoadWritableDataFromGpx();
    }

void MainWindow::on_actionSet_Scale_triggered()
    {
    if (m_map_form)
        m_map_form->SetScale();
    }

void MainWindow::on_actionGo_to_Location_triggered()
    {
    if (m_map_form)
        m_map_form->GoToLocation();
    }

void MainWindow::on_actionGo_to_Ordnance_Survey_grid_reference_triggered()
    {
    if (m_map_form)
        m_map_form->GoToOrdnanceSurveyGridRef();
    }

void MainWindow::on_actionChoose_Style_Sheet_triggered()
    {
    if (m_map_form)
        m_map_form->ChooseStyleSheet();
    }

void MainWindow::on_actionReload_Style_Sheet_triggered()
    {
    if (m_map_form)
        m_map_form->ReloadStyleSheet();
    }

void MainWindow::on_actionEdit_Custom_Style_Sheet_triggered()
    {
    if (!m_map_form)
        return;

    std::string saved_custom_style_sheet { CustomStyleSheet() };
    m_style_dialog_map_form = m_map_form;
    m_style_dialog.reset(new StyleDialog(this,m_map_form->Framework(),saved_custom_style_sheet));

    if (m_style_dialog->HaveError())
        {
        std::string s { "style sheet parse error: " };
        s += m_style_dialog->ErrorMessage() + " at '" + m_style_dialog->ErrorLocation() + "'";
        ShowError(s.c_str(),0);
        }

    connect(m_style_dialog.get(),SIGNAL(ApplyStyleSheet(std::string)),m_map_form,SLOT(ApplyStyleSheet(std::string)));
    connect(m_style_dialog.get(),SIGNAL(done(int)),this,SLOT(FinishEditingCustomStyleSheet(int)));
    m_style_dialog->show();

    // Disable edit-custom-style-sheet menu option.
    UpdateStyleSheet();
    }

void MainWindow::FinishEditingCustomStyleSheet(int aResult)
    {
    if (aResult == QDialog::DialogCode::Accepted)
        m_style_dialog_map_form->ApplyStyleSheet(m_style_dialog->EditedStyleSheetText());
    else
        m_style_dialog_map_form->ApplyStyleSheet(m_style_dialog->OriginalStyleSheetText());
    m_style_dialog_map_form->FinishEditingCustomStyleSheet(aResult);
    m_style_dialog = nullptr;
    m_style_dialog_map_form = nullptr;

    // Enable edit-custom-style-sheet menu option.
    UpdateStyleSheet();
    }

void MainWindow::on_actionLayers_triggered()
    {
    if (m_map_form)
        m_map_form->ChooseLayers();
    }

void MainWindow::on_actionPerspective_View_changed()
    {
    if (m_map_form)
        m_map_form->SetPerspective(m_ui->actionPerspective_View->isChecked());
    }

void MainWindow::on_actionMetric_Units_changed()
    {
    if (m_map_form)
        m_map_form->SetMetricUnits(m_ui->actionMetric_Units->isChecked());
    }

void MainWindow::on_actionGraphics_Acceleration_changed()
    {
    if (m_map_form)
        m_map_form->SetGraphicsAcceleration(m_ui->actionGraphics_Acceleration->isChecked());
    }

void MainWindow::on_action3D_Buildings_changed()
    {
    if (m_map_form)
        m_map_form->SetDraw3DBuildings(m_ui->action3D_Buildings->isChecked());
    }

void MainWindow::on_actionView_Route_Instructions_triggered()
    {
    if (m_map_form)
        m_map_form->ViewRouteInstructions();
    }

void MainWindow::UpdateCustomRouteProfile()
    {
    QSettings settings;
    CartoType::CMemoryOutputStream output;
    m_custom_route_profile.WriteAsXml(output);
    QString s { QString::fromUtf8((const char*)output.Data(),(int)output.Length()) };
    settings.setValue("customRouteProfile",s);
    }

void MainWindow::SetCustomStyleSheet(const std::string& aStyleSheet)
    {
    m_custom_style_sheet = aStyleSheet;
    QSettings settings;
    QString s { m_custom_style_sheet.c_str() };
    settings.setValue("customStyleSheet",s);
    }

void MainWindow::on_actionEdit_Custom_Route_Profile_triggered()
    {
    if (m_map_form && m_map_form->EditCustomRouteProfile(m_custom_route_profile))
        UpdateCustomRouteProfile();
    }

void MainWindow::on_actionSave_Custom_Route_Profile_triggered()
    {
    std::string path { GetSaveFile(*this,"Save Route Profile","CartoType route profiles","ctprofile") };
    if (path.empty())
        return;
    CartoType::CMemoryOutputStream output;
    CartoType::TResult error = m_custom_route_profile.WriteAsXml(output);
    if (!error)
        {
        FILE* f = fopen(path.c_str(),"w");
        if (f)
            {
            size_t n = fwrite(output.Data(),output.Length(),1,f);
            if (!n)
                error = CartoType::KErrorIo;
            fclose(f);
            }
        else
            error = CartoType::KErrorIo;
        }
    if (error)
        ShowError("failed to save the route to a file",error);
    }

void MainWindow::on_actionLoad_Custom_Route_Profile_triggered()
    {
    QString path = QFileDialog::getOpenFileName(this,"Load Route Profile","","CartoType route files (*.ctprofile)");
    if (!path.length())
        return;
    CartoType::TResult error = 0;
    CartoType::CString filename; filename.Set(path.utf16());
    std::unique_ptr<CartoType::CFileInputStream> input { CartoType::CFileInputStream::New(error,filename) };
    if (!error)
        {
        CartoType::TRouteProfile profile;
        error = profile.ReadFromXml(*input);
        if (!error)
            {
            m_custom_route_profile = profile;
            UpdateCustomRouteProfile();
            }
        }
    if (error)
        ShowError("could not load route profile",error);
    }

void MainWindow::on_actionUse_Custom_Style_Sheet_changed()
    {
    if (m_map_form)
        m_map_form->SetUseCustomStyleSheet(m_ui->actionUse_Custom_Style_Sheet->isChecked());
    }

void MainWindow::on_actionSave_Custom_Style_Sheet_triggered()
    {
    std::string path { GetSaveFile(*this,"Save Custom Style Sheet","CartoType style sheets","ctstyle") };
    if (path.empty())
        return;
    CartoType::TResult error = 0;
    FILE* f = fopen(path.c_str(),"w");
    if (f)
        {
        size_t n = fwrite(m_custom_style_sheet.data(),m_custom_style_sheet.length(),1,f);
        if (!n)
            error = CartoType::KErrorIo;
        fclose(f);
        }
    else
        error = CartoType::KErrorIo;
    if (error)
        ShowError("failed to save the style sheet to a file",error);
    }

void MainWindow::on_actionLoad_Custom_Style_Sheet_triggered()
    {
    QString path = QFileDialog::getOpenFileName(this,"Load Custom Style Sheet","","CartoType style sheets (*.ctstyle *.xml)");
    if (!path.length())
        return;
    CartoType::TResult error = 0;
    CartoType::CString filename; filename.Set(path.utf16());
    std::unique_ptr<CartoType::CFileInputStream> input { CartoType::CFileInputStream::New(error,filename) };
    if (!error)
        {
        std::string buffer;
        while (!input->EndOfStream())
            {
            const uint8_t* p = nullptr;
            size_t length = 0;
            error = input->Read(p,length);
            if (error || !p || length == 0)
                break;
            buffer.append((const char*)p,length);
            }
        if (!error)
            {
            SetCustomStyleSheet(buffer);
            if (m_map_form)
                m_map_form->OnCustomStyleSheetLoaded();
            }
        }
    if (error)
        ShowError("could not load style sheet",error);
    }

void MainWindow::on_actionReverse_Route_triggered()
    {
    if (m_map_form)
        m_map_form->ReverseRoute();
    }

void MainWindow::on_actionPrint_triggered()
    {
    if (m_map_form)
        m_map_form->Print(false);
    }

void MainWindow::on_actionPrint_Preview_triggered()
    {
    if (m_map_form)
        m_map_form->Print(true);
    }
