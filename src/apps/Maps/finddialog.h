#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>
#include <cartotype_framework.h>

namespace Ui
    {
    class FindDialog;
    }

class FindDialog: public QDialog
    {
    Q_OBJECT

    public:
    explicit FindDialog(QWidget* aParent,CartoType::CFramework& aFramework);
    ~FindDialog();
    CartoType::CMapObjectArray FoundObjectArray();

    Ui::FindDialog* m_ui;

    private slots:
    void on_findText_textChanged(const QString& aText);
    void on_findList_currentTextChanged(const QString& aCurrentText);

    private:
    void PopulateList(const QString& aText);
    void UpdateMatch();
    bool eventFilter(QObject* aWatched,QEvent* aEvent) override;

    CartoType::CFramework& m_framework;
    CartoType::TFindParam m_find_param;
    CartoType::CMapObjectGroupArray m_map_object_group_array;
    int m_list_box_index = -1;
    int m_lock;
    };

#endif // FINDDIALOG_H
