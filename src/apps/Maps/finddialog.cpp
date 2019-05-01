#include "finddialog.h"
#include "ui_finddialog.h"
#include <QKeyEvent>

FindDialog::FindDialog(QWidget* aParent,CartoType::CFramework& aFramework):
    QDialog(aParent),
    m_ui(new Ui::FindDialog),
    m_framework(aFramework),
    m_lock(0)
    {
    m_ui->setupUi(this);
    m_ui->findText->setFocus();
    m_find_param.iMaxObjectCount = 64;
    m_find_param.iAttributes = "$,name:*,ref,alt_name,int_name,addr:housename,pco";
    m_find_param.iCondition = "OsmType!='bsp'"; // exclude bus stops
    m_find_param.iStringMatchMethod = CartoType::TStringMatchMethod(CartoType::TStringMatchMethodFlag::Prefix |
                                                                    CartoType::TStringMatchMethodFlag::FoldAccents |
                                                                    CartoType::TStringMatchMethodFlag::FoldCase |
                                                                    CartoType::TStringMatchMethodFlag::IgnoreNonAlphanumerics |
                                                                    CartoType::TStringMatchMethodFlag::Fast);

    // Find items in or near the view by preference.
    CartoType::TRectFP view;
    m_framework.GetView(view,CartoType::TCoordType::Map);
    m_find_param.iLocation = CartoType::CGeometry(view,CartoType::TCoordType::Map);

    // Install an event filter to intercept up and down arrow events and use them to move between the line editor and the list box.
    m_ui->findText->installEventFilter(this);
    m_ui->findList->installEventFilter(this);

    PopulateList(m_ui->findText->text());
    }

FindDialog::~FindDialog()
    {
    delete m_ui;
    }

CartoType::CMapObjectArray FindDialog::FoundObjectArray()
    {
    if (size_t(m_list_box_index) < m_map_object_group_array.size())
        return std::move(m_map_object_group_array[m_list_box_index].iMapObjectArray);

    CartoType::CMapObjectArray a;
    for (auto& p : m_map_object_group_array)
        {
        for (auto& o : p.iMapObjectArray)
            a.push_back(std::move(o));
        }
    return a;
    }

void FindDialog::on_findText_textChanged(const QString& aText)
    {
    if (m_lock)
        return;
    m_lock++;
    PopulateList(aText);
    UpdateMatch();
    m_lock--;
    }

bool FindDialog::eventFilter(QObject* aWatched,QEvent* aEvent)
    {
    if ((aWatched == m_ui->findText || aWatched == m_ui->findList) &&
        aEvent->type() == QEvent::KeyPress)
        {
        auto key_event = static_cast<const QKeyEvent*>(aEvent);
        if (aWatched == m_ui->findText)
            {
            if (key_event->key() == Qt::Key_Down && m_ui->findList->count() > 0)
                {
                m_ui->findList->setFocus();
                return true;
                }
            }
        else
            {
            if (key_event->key() == Qt::Key_Up)
                {
                QModelIndexList index_list { m_ui->findList->selectionModel()->selectedIndexes() };
                if (!index_list.size() || index_list.cbegin()->row() == 0)
                    {
                    m_ui->findText->setFocus();
                    return true;
                    }
                }
            }
        }
    return false;
    }

void FindDialog::PopulateList(const QString& aText)
    {
    CartoType::CString text;
    text.Set(aText.utf16());
    m_ui->findList->clear();
    if (text.Length() == 0)
        return;

    // Find up to 64 items starting with the current text.
    CartoType::CMapObjectArray object_array;
    m_find_param.iText = text;
    m_framework.Find(m_map_object_group_array,m_find_param);

    // Put them in the list.
    for (const auto& cur_group : m_map_object_group_array)
        {
        QString qs;
        qs.setUtf16(cur_group.iName.Text(),(int)cur_group.iName.Length());
        m_ui->findList->addItem(qs);
        }
    }

void FindDialog::UpdateMatch()
    {
    m_list_box_index = m_ui->findList->currentRow();
    }

void FindDialog::on_findList_currentTextChanged(const QString& aCurrentText)
    {
    if (m_lock)
        return;
    m_lock++;
    m_ui->findText->setText(aCurrentText);
    UpdateMatch();
    m_lock--;
    }
