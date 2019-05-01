// FindDialog.cpp : implementation file
//

#include "stdafx.h"
#include "CartoTypeDemo.h"
#include "FindDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFindTextDialog::CFindTextDialog(CartoType::CFramework& aFramework,const CartoType::MString& aText,bool aPrefix,bool aFuzzy,CWnd* pParent /*=NULL*/) :
CDialog(CFindTextDialog::IDD,pParent),
iPrefix(aPrefix),
iFuzzy(aFuzzy),
iFramework(aFramework)
    {
    SetString(iFindText,aText);
    iFindParam.iMaxObjectCount = 64;
    iFindParam.iAttributes = "$,name:*,ref,alt_name,int_name,addr:housename,pco";
    iFindParam.iCondition = "OsmType!='bsp'"; // exclude bus stops
    iFindParam.iStringMatchMethod = CartoType::TStringMatchMethod(CartoType::TStringMatchMethodFlag::Prefix |
                                                                  CartoType::TStringMatchMethodFlag::FoldAccents |
                                                                  CartoType::TStringMatchMethodFlag::FoldCase |
                                                                  CartoType::TStringMatchMethodFlag::IgnoreNonAlphanumerics |
                                                                  CartoType::TStringMatchMethodFlag::Fast);
    CartoType::TRectFP view;
    iFramework.GetView(view,CartoType::TCoordType::Map);
    iFindParam.iLocation = CartoType::CGeometry(view,CartoType::TCoordType::Map);
    }

CartoType::CMapObjectArray CFindTextDialog::FoundObjectArray()
    {
    if (iListBoxIndex >= 0 && size_t(iListBoxIndex) <= iMapObjectGroupArray.size())
        return std::move(iMapObjectGroupArray[iListBoxIndex].iMapObjectArray);

    CartoType::CMapObjectArray a;
    for (auto& p : iMapObjectGroupArray)
        {
        for (auto& o : p.iMapObjectArray)
            a.push_back(std::move(o));
        }
    return a;
    }

void CFindTextDialog::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFindTextDialog)
    DDX_Text(pDX,IDC_FIND_TEXT,iFindText);
    DDX_Check(pDX,IDC_FIND_PREFIX,iPrefix);
    DDX_Check(pDX,IDC_FIND_FUZZY,iFuzzy);
    //}}AFX_DATA_MAP
    }

BEGIN_MESSAGE_MAP(CFindTextDialog,CDialog)
    ON_CBN_EDITCHANGE(IDC_FIND_TEXT,OnEditChange)
    ON_CBN_DBLCLK(IDC_FIND_TEXT,OnComboBoxDoubleClick)
    ON_CBN_SELCHANGE(IDC_FIND_TEXT,OnComboBoxSelChange)
END_MESSAGE_MAP()

BOOL CFindTextDialog::OnInitDialog()
    {
    CComboBox* cb = (CComboBox*)GetDlgItem(IDC_FIND_TEXT);
    cb->SetHorizontalExtent(400);
    UpdateData(0);
    PopulateComboBox();
    return true;
    }

void CFindTextDialog::OnEditChange()
    {
    PopulateComboBox();
    }

void CFindTextDialog::OnComboBoxDoubleClick()
    {
    UpdateMatch();
    EndDialog(IDOK);
    }

void CFindTextDialog::OnComboBoxSelChange()
    {
    UpdateMatch();
    }

void CFindTextDialog::PopulateComboBox()
    {
    // Get the current text.
    CComboBox* cb = (CComboBox*)GetDlgItem(IDC_FIND_TEXT);
    CString w_text;
    cb->GetWindowText(w_text);
    if (w_text.IsEmpty())
        return;

    iMapObjectGroupArray.clear();
    CartoType::CString text;
    SetString(text,w_text);

    // Find up to 64 items starting with the current text.
    CartoType::TPointOfInterestType poi = CartoType::TPointOfInterestType::None;
    if (text == "airport") poi = CartoType::TPointOfInterestType::Airport;
    else if (text == "bar") poi = CartoType::TPointOfInterestType::Bar;
    else if (text == "beach") poi = CartoType::TPointOfInterestType::Beach;
    else if (text == "bus") poi = CartoType::TPointOfInterestType::BusStation;
    else if (text == "cafe") poi = CartoType::TPointOfInterestType::Cafe;
    else if (text == "camping") poi = CartoType::TPointOfInterestType::Camping;
    else if (text == "fastfood") poi = CartoType::TPointOfInterestType::FastFood;
    else if (text == "fuel") poi = CartoType::TPointOfInterestType::Fuel;
    else if (text == "golf") poi = CartoType::TPointOfInterestType::GolfCourse;
    else if (text == "hospital") poi = CartoType::TPointOfInterestType::Hospital;
    else if (text == "hotel") poi = CartoType::TPointOfInterestType::Hotel;
    else if (text == "pharmacy") poi = CartoType::TPointOfInterestType::Pharmacy;
    else if (text == "police") poi = CartoType::TPointOfInterestType::Police;
    else if (text == "restaurant") poi = CartoType::TPointOfInterestType::Restaurant;
    else if (text == "shops") poi = CartoType::TPointOfInterestType::Shops;
    else if (text == "sport") poi = CartoType::TPointOfInterestType::SportsCenter;
    else if (text == "supermarket") poi = CartoType::TPointOfInterestType::Supermarket;
    else if (text == "swim") poi = CartoType::TPointOfInterestType::SwimmingPool;
    else if (text == "tourism") poi = CartoType::TPointOfInterestType::Tourism;
    else if (text == "train") poi = CartoType::TPointOfInterestType::TrainStation;

    if (poi != CartoType::TPointOfInterestType::None)
        {
        CartoType::TFindNearbyParam param;
        param.iType = poi;
        param.iLocation = iFindParam.iLocation;
        iFramework.Find(iMapObjectGroupArray,param);
        }
    else
        {
        iFindParam.iText = text;
        iFramework.Find(iMapObjectGroupArray,iFindParam);
        }

    // Put them in the combo box.
    for (int i = cb->GetCount(); i >= 0; i--)
        cb->DeleteString(i);

    for (const auto& cur_group : iMapObjectGroupArray)
        {
        SetString(w_text,cur_group.iName);
        cb->AddString(w_text);
        }
    }

void CFindTextDialog::UpdateMatch()
    {
    CComboBox* cb = (CComboBox*)GetDlgItem(IDC_FIND_TEXT);
    iListBoxIndex = cb->GetCurSel();
    }

CFindAddressDialog::CFindAddressDialog(CWnd* pParent /*=NULL*/)
    : CDialog(CFindAddressDialog::IDD,pParent)
    {
    //{{AFX_DATA_INIT(CFindAddressDialog)
    iBuilding = _T("");
    iFeature = _T("");
    iStreet = _T("");
    iSubLocality = _T("");
    iLocality = _T("");
    iSubAdminArea = _T("");
    iAdminArea = _T("");
    iCountry = _T("");
    iPostCode = _T("");
    //}}AFX_DATA_INIT
    }

void CFindAddressDialog::DoDataExchange(CDataExchange* pDX)
    {
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFindAddressDialog)
    DDX_Text(pDX,IDC_FIND_BUILDING,iBuilding);
    DDX_Text(pDX,IDC_FIND_FEATURE,iFeature);
    DDX_Text(pDX,IDC_FIND_STREET,iStreet);
    DDX_Text(pDX,IDC_FIND_SUBLOCALITY,iSubLocality);
    DDX_Text(pDX,IDC_FIND_LOCALITY,iLocality);
    DDX_Text(pDX,IDC_FIND_SUBADMINAREA,iSubAdminArea);
    DDX_Text(pDX,IDC_FIND_ADMINAREA,iAdminArea);
    DDX_Text(pDX,IDC_FIND_COUNTRY,iCountry);
    DDX_Text(pDX,IDC_FIND_POSTCODE,iPostCode);
    //}}AFX_DATA_MAP
    }

BEGIN_MESSAGE_MAP(CFindAddressDialog,CDialog)
    //{{AFX_MSG_MAP(CFindAddressDialog)
    // NOTE: the ClassWizard will add message map macros here
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()
